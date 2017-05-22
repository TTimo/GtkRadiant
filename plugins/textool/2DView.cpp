/*
   Copyright (C) 1999-2007 id Software, Inc. and contributors.
   For a list of contributors, see the accompanying CONTRIBUTORS file.

   This file is part of GtkRadiant.

   GtkRadiant is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   GtkRadiant is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GtkRadiant; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
// a class to provide basic services for 2D view of a world
// window <-> local 2D space transforms
// snap to grid
// TODO: this one can be placed under an interface, and provided to the editor as a service

#include "StdAfx.h"

static void view_ZoomIn( GtkWidget* widget, gpointer data ){
	( (C2DView*)data )->ZoomIn();
}

static void view_ZoomOut( GtkWidget* widget, gpointer data ){
	( (C2DView*)data )->ZoomOut();
}

void C2DView::PreparePaint(){
	g_QglTable.m_pfn_qglClearColor( 0, 0, 0, 0 );
	g_QglTable.m_pfn_qglViewport( 0, 0, m_rect.right, m_rect.bottom );
	g_QglTable.m_pfn_qglMatrixMode( GL_PROJECTION );
	g_QglTable.m_pfn_qglLoadIdentity();
	g_QglTable.m_pfn_qglOrtho( m_Mins[0], m_Maxs[0], m_Maxs[1], m_Mins[1], -1, 1 );
}

void C2DView::SpaceForWindow( float c[2], int x, int y ){
	c[0] = ( (float)( x ) ) / ( (float)( m_rect.right - m_rect.left ) ) * ( m_Maxs[0] - m_Mins[0] ) + m_Mins[0];
	c[1] = ( (float)( y ) ) / ( (float)( m_rect.bottom - m_rect.top ) ) * ( m_Maxs[1] - m_Mins[1] ) + m_Mins[1];
}

void C2DView::GridForWindow( float c[2], int x, int y ){
	SpaceForWindow( c, x, y );
	if ( !m_bDoGrid ) {
		return;
	}
	c[0] /= m_GridStep[0];
	c[1] /= m_GridStep[1];
	c[0] = (float)floor( c[0] + 0.5f );
	c[1] = (float)floor( c[1] + 0.5f );
	c[0] *= m_GridStep[0];
	c[1] *= m_GridStep[1];
}

void C2DView::WindowForSpace( int &x, int &y, const float c[2] ){
	x = m_rect.left + (int)( ( (float)( m_rect.right - m_rect.left ) ) * ( c[0] - m_Mins[0] ) / ( m_Maxs[0] - m_Mins[0] ) );
	y = m_rect.top + (int)( ( (float)( m_rect.bottom - m_rect.top ) ) * ( c[1] - m_Mins[1] ) / ( m_Maxs[1] - m_Mins[1] ) );
}

qboolean C2DView::DoesSelect( int x, int y, float c[2] ){
	int xc,yc;
	WindowForSpace( xc, yc, c );
	if ( abs( xc - x ) <= 3 && abs( yc - y ) <= 3 ) {
		return true;
	}
	return false;
}

void C2DView::ZoomIn(){
	m_Mins[0] = 0.5f * ( m_Mins[0] - m_Center[0] ) + m_Center[0];
	m_Mins[1] = 0.5f * ( m_Mins[1] - m_Center[1] ) + m_Center[1];
	m_Maxs[0] = 0.5f * ( m_Maxs[0] - m_Center[0] ) + m_Center[0];
	m_Maxs[1] = 0.5f * ( m_Maxs[1] - m_Center[1] ) + m_Center[1];
	g_pToolWnd->Redraw();
}

void C2DView::ZoomOut(){
	m_Mins[0] = 2.0f * ( m_Mins[0] - m_Center[0] ) + m_Center[0];
	m_Mins[1] = 2.0f * ( m_Mins[1] - m_Center[1] ) + m_Center[1];
	m_Maxs[0] = 2.0f * ( m_Maxs[0] - m_Center[0] ) + m_Center[0];
	m_Maxs[1] = 2.0f * ( m_Maxs[1] - m_Center[1] ) + m_Center[1];
	g_pToolWnd->Redraw();
}

bool C2DView::OnRButtonDown( int x, int y ){
	if ( ViewState == View_Idle ) {
		m_xPosMove = x; // horizontal position of cursor
		m_yPosMove = y; // vertical position of cursor
		// store
		m_MinsMove[0] = m_Mins[0]; m_MinsMove[1] = m_Mins[1];
		m_MaxsMove[0] = m_Maxs[0]; m_MaxsMove[1] = m_Maxs[1];
		ViewState = View_Move;
		// set popup to true
		m_bPopup = true;
		return true;
	}
	return false;
}

bool C2DView::OnRButtonUp( int x, int y ){
	if ( ViewState == View_Move ) {
		// maybe it's time for popup menu
		if ( m_bPopup ) {
			GtkWidget *menu, *item;

			menu = gtk_menu_new();

			item = gtk_menu_item_new_with_label( _( "Validate (RETURN)" ) );
			g_signal_connect( G_OBJECT( item ), "activate", G_CALLBACK( Textool_Validate ), NULL );
			gtk_widget_show( item );
			gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item );

			item = gtk_menu_item_new_with_label( _( "Zoom in (INSERT)" ) );
			g_signal_connect( G_OBJECT( item ), "activate", G_CALLBACK( view_ZoomIn ), this );
			gtk_widget_show( item );
			gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item );

			item = gtk_menu_item_new_with_label( _( "Zoom out (DELETE)" ) );
			g_signal_connect( G_OBJECT( item ), "activate", G_CALLBACK( view_ZoomOut ), this );
			gtk_widget_show( item );
			gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item );

			item = gtk_menu_item_new_with_label( _( "Cancel (ESC)" ) );
			g_signal_connect( G_OBJECT( item ), "activate", G_CALLBACK( Textool_Cancel ), NULL );
			gtk_widget_show( item );
			gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item );

			gtk_menu_popup( GTK_MENU( menu ), NULL, NULL, NULL, NULL, 1, GDK_CURRENT_TIME );
		}

		// back to Idle mode
		ViewState = View_Idle;
		return true;
	}
	return false;
}

bool C2DView::OnMouseMove( int xPos, int yPos ){
	if ( ViewState == View_Move ) {
		float V[2];
		// V is the offset
		V[0] = ( (float)( xPos - m_xPosMove ) ) * ( m_MaxsMove[0] - m_MinsMove[0] ) / ( (float)( m_rect.left - m_rect.right ) );
		V[1] = ( (float)( yPos - m_yPosMove ) ) * ( m_MaxsMove[1] - m_MinsMove[1] ) / ( (float)( m_rect.top - m_rect.bottom ) );
		// update m_Mins m_Maxs and m_Center
		m_Mins[0] = m_MinsMove[0] + V[0];
		m_Mins[1] = m_MinsMove[1] + V[1];
		m_Maxs[0] = m_MaxsMove[0] + V[0];
		m_Maxs[1] = m_MaxsMove[1] + V[1];
		m_Center[0] = 0.5f * ( m_Mins[0] + m_Maxs[0] );
		m_Center[1] = 0.5f * ( m_Mins[1] + m_Maxs[1] );
		// no popup menu if we moved
		m_bPopup = false;
		// send a repaint message
		g_pToolWnd->Redraw();
		return true;
	}
	return false;
}

bool C2DView::OnKeyDown( char *s ){
	if ( ViewState == View_Idle ) {
		if ( !strcmp( s,"Insert" ) ) {
			ZoomOut();
			return true;
		}
		if ( !strcmp( s,"Delete" ) ) {
			ZoomIn();
			return true;
		}
	}
	return false;
}
