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
// implementation of IMessaging specific interface
//

#include "stdafx.h"

CPtrArray l_Listeners[RADIANT_MSGCOUNT];
CPtrArray l_WindowListeners;
CXYWndWrapper l_XYWndWrapper;

// CGtkWindow implementation -------------------------------------

static void button_press( GtkWidget *widget, GdkEventButton *event, gpointer data ){
	IWindowListener *pListen = static_cast<IWindowListener *>( data );
	switch ( event->button )
	{
	case 1:
		pListen->OnLButtonDown( event->state, event->x, event->y ); break;
	case 3:
		pListen->OnRButtonDown( event->state, event->x, event->y ); break;
	}
}

static void button_release( GtkWidget *widget, GdkEventButton *event, gpointer data ){
	IWindowListener *pListen = static_cast<IWindowListener *>( data );
	switch ( event->button )
	{
	case 1:
		pListen->OnLButtonUp( event->state, event->x, event->y ); break;
	case 3:
		pListen->OnRButtonUp( event->state, event->x, event->y ); break;
	}
}

static void motion( GtkWidget *widget, GdkEventMotion *event, gpointer data ){
	IWindowListener *pListen = static_cast<IWindowListener *>( data );
	pListen->OnMouseMove( event->state, event->x, event->y );
}

static gint expose( GtkWidget *widget, GdkEventExpose *event, gpointer data ){
	if ( event->count > 0 ) {
		return TRUE;
	}

	CGtkWindow *pWindow = static_cast<CGtkWindow *>( data );
	pWindow->DoExpose();

	return TRUE;
}

// we use the string versions of the keys for now..
static gint keypress( GtkWidget* widget, GdkEventKey* event, gpointer data ){
	gint ret;

	IWindowListener *pListen = static_cast<IWindowListener *>( data );
	ret = pListen->OnKeyPressed( gdk_keyval_name( event->keyval ) );
	if ( ret ) {
		g_signal_stop_emission_by_name( widget, "key-press-event" );
	}
	return ret;
}

// close_widget is not hooked on the listener but on the CGtkWindow object to handle the closure
static gint close_widget( GtkWidget *widget, GdkEvent* event, gpointer data ){
	CGtkWindow *pWindow = static_cast<CGtkWindow *>( data );
	pWindow->Close();

	return TRUE;
}

void CGtkWindow::DoExpose(){
	gtk_glwidget_make_current( m_pGLWidget );
	if ( m_pListen->Paint() ) {
		gtk_glwidget_swap_buffers( m_pGLWidget );
	}
}

void CGtkWindow::Redraw(){
	gtk_widget_queue_draw( m_pGLWidget );
}

void CGtkWindow::Close(){
	// similar to a destructor, except we warn first
	m_pListen->Close();
	m_pListen->DecRef(); m_pListen = NULL;
	gtk_widget_destroy( m_pWnd ); m_pWnd = NULL;
}

bool CGtkWindow::Show(){
	// check we got everything and are reading to instanciate
	if ( m_nWidthParam == 0 || m_nHeightParam == 0 ) {
		Sys_FPrintf( SYS_ERR, "Height and Width params not set in CGtkWindow::Show\n" );
		return false;
	}
	if ( !m_pListen ) {
		Sys_FPrintf( SYS_ERR, "No listener set in CGtkWindow::Show\n" );
		return false;
	}

	// seems all good, here we go
	m_pWnd = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW( m_pWnd ), m_Name.GetBuffer() );
	gtk_window_set_default_size( GTK_WINDOW( m_pWnd ), m_nWidthParam, m_nHeightParam );
	gtk_widget_show( m_pWnd );

	// GL widget creation
	m_pGLWidget = gtk_glwidget_new( FALSE, g_qeglobals_gui.d_glBase );

	gtk_widget_set_events( m_pGLWidget, GDK_DESTROY | GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK |
						   GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK );

	// Connect signal handlers
	g_signal_connect( G_OBJECT( m_pGLWidget ), "expose-event", G_CALLBACK( expose ), this );
	g_signal_connect( G_OBJECT( m_pGLWidget ), "motion-notify-event",
						G_CALLBACK( motion ), m_pListen );
	g_signal_connect( G_OBJECT( m_pGLWidget ), "button-press-event",
						G_CALLBACK( button_press ), m_pListen );
	g_signal_connect( G_OBJECT( m_pGLWidget ), "button-release-event",
						G_CALLBACK( button_release ), m_pListen );

	g_signal_connect( G_OBJECT( m_pWnd ), "delete-event", G_CALLBACK( close_widget ), this );
	g_signal_connect( G_OBJECT( m_pWnd ), "key-press-event",
						G_CALLBACK( keypress ), m_pListen );

	gtk_widget_show( m_pGLWidget );
	gtk_container_add( GTK_CONTAINER( m_pWnd ), m_pGLWidget );

	return true;
}

IWindow* WINAPI QERApp_CreateGLWindow(){
	return new CGtkWindow;
}

void WINAPI QERApp_HookWindow( IWindowListener* pListen ){
	l_WindowListeners.Add( pListen );
	pListen->IncRef();
}

void WINAPI QERApp_UnHookWindow( IWindowListener* pListen ){
	for ( int i = 0; i < l_WindowListeners.GetSize(); i++ )
	{
		if ( l_WindowListeners.GetAt( i ) == pListen ) {
			l_WindowListeners.RemoveAt( i );
			pListen->DecRef();
			return;
		}
	}
#ifdef _DEBUG
	Sys_FPrintf( SYS_WRN, "WARNING: IWindowListener not found in QERApp_UnHookWindow\n" );
#endif
}

void DispatchOnMouseMove( guint32 nFlags, int x, int y ){
	for ( int i = 0; i < l_WindowListeners.GetSize(); i++ )
		static_cast<IWindowListener*>( l_WindowListeners.GetAt( i ) )->OnMouseMove( nFlags, x, y );
}

bool DispatchOnLButtonDown( guint32 nFlags, int x, int y ){
	for ( int i = 0; i < l_WindowListeners.GetSize(); i++ )
		if ( static_cast<IWindowListener*>( l_WindowListeners.GetAt( i ) )->OnLButtonDown( nFlags, x, y ) ) {
			return true;
		}
	return false;
}

bool DispatchOnLButtonUp( guint32 nFlags, int x, int y ){
	for ( int i = 0; i < l_WindowListeners.GetSize(); i++ )
		if ( static_cast<IWindowListener*>( l_WindowListeners.GetAt( i ) )->OnLButtonUp( nFlags, x, y ) ) {
			return true;
		}
	return false;
}

void WINAPI QERApp_HookListener( IListener* pListen, int Msg ){
#ifdef _DEBUG
	if ( Msg >= RADIANT_MSGCOUNT ) {
		Sys_FPrintf( SYS_ERR, "ERROR: bad index in QERApp_HookListener\n" );
		return;
	}
#endif
	l_Listeners[Msg].Add( pListen );
	pListen->IncRef();
}

int WINAPI QERApp_UnHookListener( IListener* pListen ){
	int count = 0;
	for ( int i = 0; i < RADIANT_MSGCOUNT; i++ )
		for ( int j = 0; j < l_Listeners[i].GetSize(); j++ )
			if ( l_Listeners[i].GetAt( j ) == pListen ) {
				l_Listeners[i].RemoveAt( j );
				pListen->DecRef();
				count++;
			}
	return count;
}

void DispatchRadiantMsg( int Msg ){
#ifdef _DEBUG
	if ( Msg >= RADIANT_MSGCOUNT ) {
		Sys_FPrintf( SYS_ERR, "ERROR: bad index in DispatchRadiantMsg\n" );
		return;
	}
#endif
	for ( int i = 0; i < l_Listeners[Msg].GetSize(); i++ )
		static_cast<IListener *>( l_Listeners[Msg].GetAt( i ) )->DispatchRadiantMsg( Msg );
}

void CXYWndWrapper::SnapToGrid( int x1, int y1, vec3_t pt ){
	int height = g_pParentWnd->ActiveXY()->GetWidget()->allocation.height;
	g_pParentWnd->ActiveXY()->SnapToPoint( x1, height - 1 - y1, pt );
}

VIEWTYPE CXYWndWrapper::GetViewType( void ){
	return (VIEWTYPE)g_pParentWnd->ActiveXY()->GetViewType();
}

IXYWndWrapper* WINAPI QERApp_GetXYWndWrapper(){
	return &l_XYWndWrapper;
}
