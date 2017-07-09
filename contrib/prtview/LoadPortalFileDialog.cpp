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




static void change_clicked( GtkWidget *widget, gpointer data ){
	GtkWidget *file_sel, *parent;
	gint response_id;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;

	parent = GTK_WIDGET( data );
	file_sel = gtk_file_chooser_dialog_new( _( "Locate portal (.prt) file" ), NULL, action, NULL, (char*)NULL );
	gtk_window_set_transient_for( GTK_WINDOW( file_sel ), GTK_WINDOW( parent ) );
	gtk_dialog_add_button( GTK_DIALOG( file_sel ), _( "_Open" ), GTK_RESPONSE_ACCEPT );
	gtk_dialog_add_button( GTK_DIALOG( file_sel ), _( "_Cancel" ), GTK_RESPONSE_CANCEL );

	response_id = gtk_dialog_run( GTK_DIALOG( file_sel ) );
	if( response_id == GTK_RESPONSE_ACCEPT )
	{
		gchar *filename;
		GtkFileChooser *chooser;

		chooser = GTK_FILE_CHOOSER( file_sel );
		filename = gtk_file_chooser_get_filename( chooser );

		if ( filename != NULL ) {
			strcpy( portals.fn, filename );
			gtk_entry_set_text( GTK_ENTRY( data ), filename );
			g_free( filename );
		}
	}

	gtk_widget_destroy( file_sel );
}

int DoLoadPortalFileDialog( GtkWidget *parent ){
	GtkWidget *dialog, *vbox, *hbox, *button, *entry, *check2d, *check3d;
	GtkWidget *content_area, *ok_button, *cancel_button, *change_button;
	gint response_id;
	int ret;
	GtkSizeGroup *size_group;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Load .prt" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( parent ) );

	ok_button = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	cancel_button = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	entry = gtk_entry_new();
	gtk_editable_set_editable( GTK_EDITABLE( entry ), FALSE );
	gtk_box_pack_start( GTK_BOX( vbox ), entry, FALSE, FALSE, 0 );
	gtk_widget_show( entry );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	check3d = gtk_check_button_new_with_label( _( "Show 3D" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), check3d, FALSE, FALSE, 0 );
	gtk_widget_show( check3d );

	check2d = gtk_check_button_new_with_label( _( "Show 2D" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), check2d, FALSE, FALSE, 0 );
	gtk_widget_show( check2d );

	change_button = button = gtk_button_new_with_label( _( "Change" ) );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( change_clicked ), parent );
	gtk_widget_show( button );

	size_group = gtk_size_group_new( GTK_SIZE_GROUP_BOTH );
	gtk_size_group_add_widget( size_group, ok_button );
	gtk_size_group_add_widget( size_group, cancel_button );
	gtk_size_group_add_widget( size_group, change_button );
	g_object_unref( size_group );

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


	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == GTK_RESPONSE_OK ) {

		portals.Purge();

		portals.show_3d = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( check3d ) ) ? true : false;
		portals.show_2d = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( check2d ) ) ? true : false;

		ret = IDOK;
	} else {
		ret = IDCANCEL;
	}

	gtk_widget_destroy( dialog );

	return ret;
}
