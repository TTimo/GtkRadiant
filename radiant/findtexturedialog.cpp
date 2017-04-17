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

//
// Find/Replace textures dialogs
//
// Leonardo Zide (leo@lokigames.com)
//

#include "stdafx.h"
#include "findtexturedialog.h"
#include <glib/gi18n.h>

FindTextureDialog g_TexFindDlg;
FindTextureDialog& g_dlgFind = g_TexFindDlg;
static bool g_bFindActive = true;

static void OnApply( GtkWidget *widget, gpointer data ){
	g_dlgFind.UpdateData( TRUE );
	FindReplaceTextures( g_dlgFind.m_strFind, g_dlgFind.m_strReplace,
						 g_dlgFind.m_bSelectedOnly, g_dlgFind.m_bForce, FALSE );
}

static void OnFind( GtkWidget *widget, gpointer data ){
	g_dlgFind.UpdateData( TRUE );
	FindReplaceTextures( g_dlgFind.m_strFind, g_dlgFind.m_strReplace,
						 g_dlgFind.m_bSelectedOnly, FALSE, TRUE );
}

static void OnOK( GtkWidget *widget, gpointer data ){
	g_dlgFind.UpdateData( TRUE );
	FindReplaceTextures( g_dlgFind.m_strFind, g_dlgFind.m_strReplace,
						 g_dlgFind.m_bSelectedOnly, g_dlgFind.m_bForce, FALSE );
	g_dlgFind.HideDlg();
}

static void OnClose( GtkWidget *widget, gpointer data ){
	g_dlgFind.HideDlg();
}

static void popup_selected( GtkWidget *widget, gpointer data ){
	const gchar *str;

	str = gtk_label_get_text( GTK_LABEL( gtk_bin_get_child( GTK_BIN( widget ) ) ) );
	gtk_entry_set_text( GTK_ENTRY( data ), str );
}

static void find_clicked( GtkWidget *widget, gpointer data ){
	GtkWidget *menu, *item;
	menu = gtk_menu_new();

	for ( int i = 0; i < QERApp_GetActiveShaderCount(); i++ )
	{
		IShader *pShader = QERApp_ActiveShader_ForIndex( i );

		item = gtk_menu_item_new_with_label( pShader->getName() );
		g_signal_connect( G_OBJECT( item ), "activate", G_CALLBACK( popup_selected ), data );
		gtk_widget_show( item );
		gtk_menu_shell_append( GTK_MENU_SHELL( menu ), item );
	}

	gtk_menu_popup( GTK_MENU( menu ), NULL, NULL, NULL, NULL, 1, GDK_CURRENT_TIME );
}

static gint find_focus_in( GtkWidget *widget, GdkEventFocus *event, gpointer data ){
	g_bFindActive = true;
	return FALSE;
}

static gint replace_focus_in( GtkWidget *widget, GdkEventFocus *event, gpointer data ){
	g_bFindActive = false;
	return FALSE;
}

// =============================================================================
// FindTextureDialog class

FindTextureDialog::FindTextureDialog (){
	m_bSelectedOnly = FALSE;
	m_strFind = "";
	m_strReplace = "";
	m_bForce = FALSE;
	m_bLive = TRUE;
}

FindTextureDialog::~FindTextureDialog (){
}

void FindTextureDialog::BuildDialog(){
	GtkWidget *dlg, *vbox, *hbox, *table, *label;
	GtkWidget *button, *check, *entry, *arrow;

	dlg = m_pWidget;
	gtk_window_set_title( GTK_WINDOW( dlg ), _( "Find / Replace Texture(s)" ) );
	gtk_window_set_transient_for( GTK_WINDOW( dlg ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( dlg ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 5 );
	gtk_widget_show( hbox );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox, TRUE, TRUE, 5 );
	gtk_widget_show( vbox );

	table = gtk_table_new( 2, 3, FALSE );
	gtk_box_pack_start( GTK_BOX( vbox ), table, TRUE, TRUE, 0 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_widget_show( table );

	label = gtk_label_new( _( "Find:" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	label = gtk_label_new( _( "Replace:" ) );
	gtk_table_attach( GTK_TABLE( table ), label, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );

	entry = gtk_entry_new();
	gtk_table_attach( GTK_TABLE( table ), entry, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( entry );
	g_signal_connect( G_OBJECT( entry ), "focus-in-event",
						G_CALLBACK( find_focus_in ), NULL );
	AddDialogData( entry, &m_strFind, DLG_ENTRY_TEXT );

	button = gtk_button_new();
	gtk_table_attach( GTK_TABLE( table ), button, 2, 3, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( find_clicked ), entry );

	arrow = gtk_arrow_new( GTK_ARROW_DOWN, GTK_SHADOW_OUT );
	gtk_container_add( GTK_CONTAINER( button ), arrow );
	gtk_widget_show( arrow );

	entry = gtk_entry_new();
	gtk_table_attach( GTK_TABLE( table ), entry, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( entry );
	g_signal_connect( G_OBJECT( entry ), "focus-in-event",
						G_CALLBACK( replace_focus_in ), NULL );
	AddDialogData( entry, &m_strReplace, DLG_ENTRY_TEXT );

	button = gtk_button_new();
	gtk_table_attach( GTK_TABLE( table ), button, 2, 3, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
	gtk_widget_show( button );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( find_clicked ), entry );

	arrow = gtk_arrow_new( GTK_ARROW_DOWN, GTK_SHADOW_OUT );
	gtk_container_add( GTK_CONTAINER( button ), arrow );
	gtk_widget_show( arrow );

	check = gtk_check_button_new_with_label( _( "Use selected brushes only" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), check, TRUE, TRUE, 0 );
	gtk_widget_show( check );
	AddDialogData( check, &m_bSelectedOnly, DLG_CHECK_BOOL );

	check = gtk_check_button_new_with_label( _( "Replace everywhere (selected/active), don't test against Find" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), check, TRUE, TRUE, 0 );
	gtk_widget_show( check );
	AddDialogData( check, &m_bForce, DLG_CHECK_BOOL );

	check = gtk_check_button_new_with_label( _( "Live updates from Texture/Camera windows" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), check, TRUE, TRUE, 0 );
	gtk_widget_show( check );
	AddDialogData( check, &m_bLive, DLG_CHECK_BOOL );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox, TRUE, TRUE, 0 );
	gtk_widget_show( vbox );

	button = gtk_button_new_with_label( _( "Find" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( OnFind ), NULL );
	gtk_widget_set_size_request( button, 60, -1 );
	gtk_widget_show( button );

	button = gtk_button_new_with_label( _( "OK" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( OnOK ), NULL );
	gtk_widget_set_size_request( button, 60, -1 );
	gtk_widget_show( button );

	button = gtk_button_new_with_label( _( "Apply" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( OnApply ), NULL );
	gtk_widget_set_size_request( button, 60, -1 );
	gtk_widget_show( button );

	button = gtk_button_new_with_label( _( "Close" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( OnClose ), NULL );
	gtk_widget_set_size_request( button, 60, -1 );
	gtk_widget_show( dlg );
}

void FindTextureDialog::updateTextures( const char *p ){
	if ( isOpen() ) {
		if ( g_bFindActive ) {
			setFindStr( p );
		}
		else
		{
			setReplaceStr( p );
		}
	}
}

bool FindTextureDialog::isOpen(){
	return ( g_dlgFind.m_pWidget == NULL || gtk_widget_get_visible( g_dlgFind.m_pWidget ) == FALSE ) ? false : true;
}

void FindTextureDialog::setFindStr( const char * p ){
	g_dlgFind.UpdateData( TRUE );
	if ( g_dlgFind.m_bLive ) {
		g_dlgFind.m_strFind = p;
		g_dlgFind.UpdateData( FALSE );
	}
}

void FindTextureDialog::setReplaceStr( const char * p ){
	g_dlgFind.UpdateData( TRUE );
	if ( g_dlgFind.m_bLive ) {
		g_dlgFind.m_strReplace = p;
		g_dlgFind.UpdateData( FALSE );
	}
}

void FindTextureDialog::show(){
	g_dlgFind.ShowDlg();
}
