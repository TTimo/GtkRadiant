/*
   PrtView plugin for GtkRadiant
   Copyright (C) 2001 Geoffrey Dewan, Loki software and qeradiant.com

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// LoadPortalFileDialog.cpp : implementation file
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
#endif

static void dialog_button_callback( GtkWidget *widget, gpointer data ){
	GtkWidget *parent;
	int *loop, *ret;

	parent = gtk_widget_get_toplevel( widget );
	loop = (int*)g_object_get_data( G_OBJECT( parent ), "loop" );
	ret = (int*)g_object_get_data( G_OBJECT( parent ), "ret" );

	*loop = 0;
	*ret = (intptr_t)data;
}

static gint dialog_delete_callback( GtkWidget *widget, GdkEvent* event, gpointer data ){
	int *loop;

	gtk_widget_hide( widget );
	loop = (int*)g_object_get_data( G_OBJECT( widget ), "loop" );
	*loop = 0;

	return TRUE;
}

static void file_sel_callback( GtkWidget *widget, gpointer data ){
	GtkWidget *parent;
	int *loop;
	char **filename;

	parent = gtk_widget_get_toplevel( widget );
	loop = (int*)g_object_get_data( G_OBJECT( parent ), "loop" );
	filename = (char**)g_object_get_data( G_OBJECT( parent ), "filename" );

	*loop = 0;
	if ( (intptr_t)data == IDOK ) {
		*filename = g_strdup( gtk_file_selection_get_filename( GTK_FILE_SELECTION( parent ) ) );
	}
}

static void change_clicked( GtkWidget *widget, gpointer data ){
	GtkWidget* file_sel;
	char* filename = NULL;
	int loop = 1;

	file_sel = gtk_file_selection_new( "Locate portal (.prt) file" );
	g_signal_connect( G_OBJECT( GTK_FILE_SELECTION( file_sel )->ok_button ), "clicked",
						G_CALLBACK( file_sel_callback ), GINT_TO_POINTER( IDOK ) );
	g_signal_connect( G_OBJECT( GTK_FILE_SELECTION( file_sel )->cancel_button ), "clicked",
						G_CALLBACK( file_sel_callback ), GINT_TO_POINTER( IDCANCEL ) );
	g_signal_connect( G_OBJECT( file_sel ), "delete-event",
						G_CALLBACK( dialog_delete_callback ), NULL );
	gtk_file_selection_hide_fileop_buttons( GTK_FILE_SELECTION( file_sel ) );

	g_object_set_data( G_OBJECT( file_sel ), "loop", &loop );
	g_object_set_data( G_OBJECT( file_sel ), "filename", &filename );
	gtk_file_selection_set_filename( GTK_FILE_SELECTION( file_sel ), portals.fn );

	gtk_grab_add( file_sel );
	gtk_widget_show( file_sel );

	while ( loop )
		gtk_main_iteration();

	gtk_grab_remove( file_sel );
	gtk_widget_destroy( file_sel );

	if ( filename != NULL ) {
		strcpy( portals.fn, filename );
		gtk_entry_set_text( GTK_ENTRY( data ), filename );
		g_free( filename );
	}
}

int DoLoadPortalFileDialog(){
	GtkWidget *dlg, *vbox, *hbox, *button, *entry, *check2d, *check3d;
	int loop = 1, ret = IDCANCEL;

	dlg = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW( dlg ), "Load .prt" );
	g_signal_connect( G_OBJECT( dlg ), "delete-event",
						G_CALLBACK( dialog_delete_callback ), NULL );
	g_signal_connect( G_OBJECT( dlg ), "destroy",
						G_CALLBACK( gtk_widget_destroy ), NULL );
	g_object_set_data( G_OBJECT( dlg ), "loop", &loop );
	g_object_set_data( G_OBJECT( dlg ), "ret", &ret );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_widget_show( vbox );
	gtk_container_add( GTK_CONTAINER( dlg ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );

	entry = gtk_entry_new();
	gtk_widget_show( entry );
	gtk_entry_set_editable( GTK_ENTRY( entry ), FALSE );
	gtk_box_pack_start( GTK_BOX( vbox ), entry, FALSE, FALSE, 0 );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_widget_show( hbox );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );

	check3d = gtk_check_button_new_with_label( "Show 3D" );
	gtk_widget_show( check3d );
	gtk_box_pack_start( GTK_BOX( hbox ), check3d, FALSE, FALSE, 0 );

	check2d = gtk_check_button_new_with_label( "Show 2D" );
	gtk_widget_show( check2d );
	gtk_box_pack_start( GTK_BOX( hbox ), check2d, FALSE, FALSE, 0 );

	button = gtk_button_new_with_label( "Change" );
	gtk_widget_show( button );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( change_clicked ), entry );
	gtk_widget_set_usize( button, 60, -2 );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_widget_show( hbox );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );

	button = gtk_button_new_with_label( "Cancel" );
	gtk_widget_show( button );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDCANCEL ) );
	gtk_widget_set_usize( button, 60, -2 );

	button = gtk_button_new_with_label( "OK" );
	gtk_widget_show( button );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDOK ) );
	gtk_widget_set_usize( button, 60, -2 );

	char *fn = g_FuncTable.m_pfnGetMapName();
	strcpy( portals.fn, fn );
	fn = strrchr( portals.fn, '.' );
	if ( fn != NULL ) {
		*fn = '\0';
		strcat( portals.fn, ".prt" );
	}

	gtk_entry_set_text( GTK_ENTRY( entry ), portals.fn );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( check2d ), portals.show_2d );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( check3d ), portals.show_3d );

	gtk_grab_add( dlg );
	gtk_widget_show( dlg );

	while ( loop )
		gtk_main_iteration();

	if ( ret == IDOK ) {
		portals.Purge();

		portals.show_3d = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( check3d ) ) ? true : false;
		portals.show_2d = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( check2d ) ) ? true : false;
	}

	gtk_grab_remove( dlg );
	gtk_widget_destroy( dlg );

	return ret;
}
