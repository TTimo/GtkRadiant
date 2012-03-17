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
// InfoDialog - non-modal, not derived from Dialog
//
// Leonardo Zide (leo@lokigames.com)
//

#include "stdafx.h"
#include <gtk/gtk.h>
//#include "qe3.h"

GtkWidget *g_dlgInfo;
GtkWidget *s_pEdit;

// =============================================================================
// Global functions

void ShowInfoDialog( const char* pText ){
	if ( g_dlgInfo == NULL ) {
		GtkWidget *dlg, *scr, *text;

		g_dlgInfo = dlg = gtk_window_new( GTK_WINDOW_TOPLEVEL );
		gtk_window_set_title( GTK_WINDOW( dlg ), "Information" );
		gtk_signal_connect( GTK_OBJECT( dlg ), "delete_event",
							GTK_SIGNAL_FUNC( gtk_widget_hide ), NULL );
		gtk_signal_connect( GTK_OBJECT( dlg ), "destroy",
							GTK_SIGNAL_FUNC( gtk_widget_destroy ), NULL );
		gtk_window_set_default_size( GTK_WINDOW( dlg ), 300, 150 );

		scr = gtk_scrolled_window_new( NULL, NULL );
		gtk_widget_show( scr );
		gtk_container_add( GTK_CONTAINER( dlg ), scr );
		gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( scr ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
		gtk_container_set_border_width( GTK_CONTAINER( scr ), 5 );
		gtk_scrolled_window_set_shadow_type( GTK_SCROLLED_WINDOW( scr ), GTK_SHADOW_IN );

		s_pEdit = text = gtk_text_view_new();
		gtk_container_add( GTK_CONTAINER( scr ), text );
		gtk_widget_show( text );
		gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW( text ), GTK_WRAP_WORD );
	}

	GtkTextBuffer* buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( s_pEdit ) );
	gtk_text_buffer_set_text( buffer, pText, -1 );
	gtk_widget_show( g_dlgInfo );
}

void HideInfoDialog(){
	if ( g_dlgInfo ) {
		gtk_widget_hide( g_dlgInfo );
	}
}
