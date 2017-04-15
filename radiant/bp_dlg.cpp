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
// DESCRIPTION
//
// custom Gtk dialog for brush primitives load/save

#include "stdafx.h"
#include <glib/gi18n.h>


// ret: 0 = abort, 1 = load and convert, 2 = changed project settings, load and don't convert
// the user might decide to switch the BP mode in project settings
// status: 0 = loading regular, got conflict 1 = loading BP, got conflict
int BP_MessageBox( int status ){
	GtkWidget *dialog, *w, *vbox, *hbox, *content_area;
	GtkAccelGroup *accel;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Current map format is incompatible" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pParentWnd->m_pWidget ) );
	gtk_container_set_border_width( GTK_CONTAINER( dialog ), 10 );

	accel = gtk_accel_group_new();
	gtk_window_add_accel_group( GTK_WINDOW( dialog ), accel );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Convert" ), 1 );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Change default" ), 2 );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Abort load" ), 0 );
	
	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_vbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_widget_show( vbox );

	if ( status == 0 ) {
		w = gtk_label_new( _( "This map was saved using brush primitives format\n"
							  "and your project settings use the standard format.\n"
							  "Do you want to convert the map, change default format or abort?\n"
							  "NOTE: due to limitations of the standard format, "
							  "some texture alignments may be lost after conversion." ) );
	}
	else
	{
		w = gtk_label_new( _( "This map was saved using standard format\n"
							  "and your project settings use the new \"brush primitives\" format.\n"
							  "Do you want to convert the map, change default format or abort?\n"
							  "NOTE: Next versions of Radiant will allow mixing the two formats"
							  "in the same maps for a smooth transition." ) );
	}
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 2 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_label_set_justify( GTK_LABEL( w ), GTK_JUSTIFY_LEFT );
	gtk_widget_show( w );

	w = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 2 );
	gtk_widget_show( w );

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 2 );
	gtk_widget_show( hbox );


	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if( response_id == 2 ) {
		// change project settings
		if ( status == 0 ) {
			g_qeglobals.m_bBrushPrimitMode = TRUE;
		}
		else{
			g_qeglobals.m_bBrushPrimitMode = FALSE;
		}
		SetKeyValue( g_qeglobals.d_project_entity, "brush_primit", ( g_qeglobals.m_bBrushPrimitMode ? "1" : "0" ) );
	}
	if( response_id < 0 ) {
		response_id = 0; //default abort
	}
	gtk_widget_destroy( dialog );

	return response_id;
}
