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

// AboutDialog.cpp : implementation file
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDialog dialog

void DoAboutDlg( GtkWidget *parent ){
	GtkWidget *dialog, *vbox, *label, *content_area;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "About Portal Viewer" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( parent) );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	label = gtk_label_new( "Version 1.000\n\n"
						   "Gtk port by Leonardo Zide\nleo@lokigames.com\n\n"
						   "Written by Geoffrey DeWan\ngdewan@prairienet.org\n\n"
						   "Built against GtkRadiant " RADIANT_VERSION "\n"
						   __DATE__
						   );
	gtk_box_pack_start( GTK_BOX( vbox ), label, TRUE, TRUE, 0 );
	gtk_label_set_justify( GTK_LABEL( label ), GTK_JUSTIFY_LEFT );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_widget_show( label );


	gtk_dialog_run( GTK_DIALOG( dialog ) );

	gtk_widget_destroy( dialog );
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDialog message handlers
