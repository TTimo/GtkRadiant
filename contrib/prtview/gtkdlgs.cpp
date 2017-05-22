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

//
// PrtView dialogs done with GTK+
//

#include <stdio.h>
#include "stdafx.h"

// =============================================================================
// Static functions

static void dialog_button_callback( GtkWidget *widget, gpointer data ){
	GtkWidget *parent;
	int *loop, *ret;

	parent = gtk_widget_get_toplevel( widget );
	loop = (int*)g_object_get_data( G_OBJECT( parent ), "loop" );
	ret = (int*)g_object_get_data( G_OBJECT( parent ), "ret" );

	*loop = 0;
	*ret = (int)((intptr_t)data);
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
	if ( (int)((intptr_t)data) == IDOK ) {
		*filename = g_strdup( gtk_file_selection_get_filename( GTK_FILE_SELECTION( parent ) ) );
	}
}

static void change_clicked( GtkWidget *widget, gpointer data ){
	GtkWidget* file_sel;
	char* filename = NULL;
	int loop = 1;

	file_sel = gtk_file_selection_new( _( "Locate portal (.prt) file" ) );
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

// =============================================================================
// LoadPortalFile dialog

int DoLoadPortalFileDialog(){
	GtkWidget *dlg, *vbox, *hbox, *button, *entry, *check2d, *check3d;
	int loop = 1, ret = IDCANCEL;

	dlg = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW( dlg ), _( "Load .prt" ) );
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

	check3d = gtk_check_button_new_with_label( _( "Show 3D" ) );
	gtk_widget_show( check3d );
	gtk_box_pack_start( GTK_BOX( hbox ), check3d, FALSE, FALSE, 0 );

	check2d = gtk_check_button_new_with_label( _( "Show 2D" ) );
	gtk_widget_show( check2d );
	gtk_box_pack_start( GTK_BOX( hbox ), check2d, FALSE, FALSE, 0 );

	button = gtk_button_new_with_label( _( "Change" ) );
	gtk_widget_show( button );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( change_clicked ), entry );
	gtk_widget_set_size_request( button, 60, -2 );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_widget_show( hbox );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );

	button = gtk_button_new_with_label( _( "Cancel" ) );
	gtk_widget_show( button );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDCANCEL ) );
	gtk_widget_set_size_request( button, 60, -2 );

	button = gtk_button_new_with_label( _( "OK" ) );
	gtk_widget_show( button );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDOK ) );
	gtk_widget_set_size_request( button, 60, -2 );

	char *fn = g_IBSPTable.m_pfnGetMapName();
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

		portals.show_3d = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( check3d ) );
		portals.show_2d = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( check2d ) );
	}

	gtk_grab_remove( dlg );
	gtk_widget_destroy( dlg );

	return ret;
}

// =============================================================================
// About dialog

void DoAboutDlg(){
	GtkWidget *dlg, *hbox, *vbox, *button, *label;
	int loop = 1, ret = IDCANCEL;

	dlg = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW( dlg ), _( "About Portal Viewer" ) );
	g_signal_connect( G_OBJECT( dlg ), "delete-event",
						G_CALLBACK( dialog_delete_callback ), NULL );
	g_signal_connect( G_OBJECT( dlg ), "destroy",
						G_CALLBACK( gtk_widget_destroy ), NULL );
	g_object_set_data( G_OBJECT( dlg ), "loop", &loop );
	g_object_set_data( G_OBJECT( dlg ), "ret", &ret );

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_widget_show( hbox );
	gtk_container_add( GTK_CONTAINER( dlg ), hbox );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 10 );

	label = gtk_label_new( "Version 1.000\n\n"
						   "Gtk port by Leonardo Zide\nleo@lokigames.com\n\n"
						   "Written by Geoffrey DeWan\ngdewan@prairienet.org" );
	gtk_widget_show( label );
	gtk_box_pack_start( GTK_BOX( hbox ), label, TRUE, TRUE, 0 );
	gtk_label_set_justify( GTK_LABEL( label ), GTK_JUSTIFY_LEFT );

	vbox = gtk_vbox_new( FALSE, 0 );
	gtk_widget_show( vbox );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox, FALSE, FALSE, 0 );

	button = gtk_button_new_with_label( _( "OK" ) );
	gtk_widget_show( button );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDOK ) );
	gtk_widget_set_size_request( button, 60, -2 );

	gtk_grab_add( dlg );
	gtk_widget_show( dlg );

	while ( loop )
		gtk_main_iteration();

	gtk_grab_remove( dlg );
	gtk_widget_destroy( dlg );
}

// =============================================================================
// Config dialog

static int DoColor( COLORREF *c ){
	GtkWidget* dlg;
	double clr[3];
	int loop = 1, ret = IDCANCEL;

	clr[0] = ( (double)GetRValue( *c ) ) / 255.0;
	clr[1] = ( (double)GetGValue( *c ) ) / 255.0;
	clr[2] = ( (double)GetBValue( *c ) ) / 255.0;

	dlg = gtk_color_selection_dialog_new( _( "Choose Color" ) );
	gtk_color_selection_set_color( GTK_COLOR_SELECTION( GTK_COLOR_SELECTION_DIALOG( dlg )->colorsel ), clr );
	g_signal_connect( G_OBJECT( dlg ), "delete-event",
						G_CALLBACK( dialog_delete_callback ), NULL );
	g_signal_connect( G_OBJECT( dlg ), "destroy",
						G_CALLBACK( gtk_widget_destroy ), NULL );
	g_signal_connect( G_OBJECT( GTK_COLOR_SELECTION_DIALOG( dlg )->ok_button ), "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDOK ) );
	g_signal_connect( G_OBJECT( GTK_COLOR_SELECTION_DIALOG( dlg )->cancel_button ), "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDCANCEL ) );
	g_object_set_data( G_OBJECT( dlg ), "loop", &loop );
	g_object_set_data( G_OBJECT( dlg ), "ret", &ret );

	gtk_widget_show( dlg );
	gtk_grab_add( dlg );

	while ( loop )
		gtk_main_iteration();

	gtk_color_selection_get_color( GTK_COLOR_SELECTION( GTK_COLOR_SELECTION_DIALOG( dlg )->colorsel ), clr );

	gtk_grab_remove( dlg );
	gtk_widget_destroy( dlg );

	if ( ret == IDOK ) {
		*c = RGB( clr[0] * 255, clr[1] * 255, clr[2] * 255 );
	}

	return ret;
}

static void Set2DText( GtkWidget* label ){
	char s[40];

	sprintf( s, "Line Width = %6.3f", portals.width_2d * 0.5f );

	gtk_label_set_text( GTK_LABEL( label ), s );
}

static void Set3DText( GtkWidget* label ){
	char s[40];

	sprintf( s, "Line Width = %6.3f", portals.width_3d * 0.5f );

	gtk_label_set_text( GTK_LABEL( label ), s );
}

static void Set3DTransText( GtkWidget* label ){
	char s[40];

	sprintf( s, "Polygon transparency = %d%%", (int)portals.trans_3d );

	gtk_label_set_text( GTK_LABEL( label ), s );
}

static void SetClipText( GtkWidget* label ){
	char s[40];

	sprintf( s, "Cubic clip range = %d", (int)portals.clip_range * 64 );

	gtk_label_set_text( GTK_LABEL( label ), s );
}

static void OnScroll2d( GtkAdjustment *adj, gpointer data ){
	portals.width_2d = adj->value;
	Set2DText( GTK_WIDGET( data ) );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_2D );
	}
}

static void OnScroll3d( GtkAdjustment *adj, gpointer data ){
	portals.width_3d = adj->value;
	Set3DText( GTK_WIDGET( data ) );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
	}
}

static void OnScrollTrans( GtkAdjustment *adj, gpointer data ){
	portals.trans_3d = adj->value;
	Set3DTransText( GTK_WIDGET( data ) );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
	}
}

static void OnScrollClip( GtkAdjustment *adj, gpointer data ){
	portals.clip_range = adj->value;
	SetClipText( GTK_WIDGET( data ) );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
	}
}

static void OnAntiAlias2d( GtkWidget *widget, gpointer data ){
	portals.aa_2d = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( widget ) );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_2D );
	}
}

static void OnConfig2d( GtkWidget *widget, gpointer data ){
	portals.show_2d = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( widget ) );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_2D );
	}
}

static void OnColor2d( GtkWidget *widget, gpointer data ){
	if ( DoColor( &portals.color_2d ) == IDOK ) {
		portals.FixColors();

		if ( interfaces_started ) {
			g_FuncTable.m_pfnSysUpdateWindows( UPDATE_2D );
		}
	}
}

static void OnConfig3d( GtkWidget *widget, gpointer data ){
	portals.show_3d = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( widget ) );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
	}
}


static void OnAntiAlias3d( GtkWidget *widget, gpointer data ){
	portals.aa_3d = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( widget ) );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
	}
}

static void OnColor3d( GtkWidget *widget, gpointer data ){
	if ( DoColor( &portals.color_3d ) == IDOK ) {
		portals.FixColors();

		if ( interfaces_started ) {
			g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
		}
	}
}

static void OnColorFog( GtkWidget *widget, gpointer data ){
	if ( DoColor( &portals.color_fog ) == IDOK ) {
		portals.FixColors();

		if ( interfaces_started ) {
			g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
		}
	}
}

static void OnFog( GtkWidget *widget, gpointer data ){
	portals.fog = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( widget ) );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
	}
}

static void OnSelchangeZbuffer( GtkWidget *widget, gpointer data ){
	portals.zbuffer = GPOINTER_TO_INT( data );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
	}
}

static void OnPoly( GtkWidget *widget, gpointer data ){
	portals.polygons = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( widget ) );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
	}
}

static void OnLines( GtkWidget *widget, gpointer data ){
	portals.lines = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( widget ) );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
	}
}

static void OnClip( GtkWidget *widget, gpointer data ){
	portals.clip = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( widget ) );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
	}
}

void DoConfigDialog(){
	GtkWidget *dlg, *hbox, *vbox, *vbox2, *button, *table, *frame;
	GtkWidget *lw3slider, *lw3label, *lw2slider, *lw2label, *zlist, *menu, *item;
	GtkWidget *aa2check, *aa3check, *depthcheck, *linescheck, *polyscheck;
	GtkWidget *transslider, *translabel, *clipslider, *cliplabel;
	GtkWidget *show2check, *show3check, *portalcheck;
	int loop = 1, ret = IDCANCEL;
	GtkObject *adj;

	dlg = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW( dlg ), _( "Portal Viewer Configuration" ) );
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

	frame = gtk_frame_new( _( "3D View" ) );
	gtk_widget_show( frame );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, TRUE, TRUE, 0 );

	vbox2 = gtk_vbox_new( FALSE, 5 );
	gtk_widget_show( vbox2 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox2 );
	gtk_container_set_border_width( GTK_CONTAINER( vbox2 ), 5 );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_widget_show( hbox );
	gtk_box_pack_start( GTK_BOX( vbox2 ), hbox, TRUE, TRUE, 0 );

	adj = gtk_adjustment_new( portals.width_3d, 2, 40, 1, 1, 1 );
	lw3slider = gtk_hscale_new( GTK_ADJUSTMENT( adj ) );
	gtk_widget_show( lw3slider );
	gtk_box_pack_start( GTK_BOX( hbox ), lw3slider, TRUE, TRUE, 0 );
	gtk_scale_set_draw_value( GTK_SCALE( lw3slider ), FALSE );

	lw3label = gtk_label_new( "" );
	gtk_widget_show( lw3label );
	gtk_box_pack_start( GTK_BOX( hbox ), lw3label, FALSE, TRUE, 0 );
	g_signal_connect( adj, "value-changed", G_CALLBACK( OnScroll3d ), lw3label );

	table = gtk_table_new( 2, 4, FALSE );
	gtk_widget_show( table );
	gtk_box_pack_start( GTK_BOX( vbox2 ), table, TRUE, TRUE, 0 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );

	button = gtk_button_new_with_label( _( "Color" ) );
	gtk_widget_show( button );
	gtk_table_attach( GTK_TABLE( table ), button, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( OnColor3d ), NULL );

	button = gtk_button_new_with_label( _( "Depth Color" ) );
	gtk_widget_show( button );
	gtk_table_attach( GTK_TABLE( table ), button, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( OnColorFog ), NULL );

	aa3check = gtk_check_button_new_with_label( _( "Anti-Alias (May not work on some video cards)" ) );
	gtk_widget_show( aa3check );
	gtk_table_attach( GTK_TABLE( table ), aa3check, 1, 4, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	g_signal_connect( G_OBJECT( aa3check ), "toggled", G_CALLBACK( OnAntiAlias3d ), NULL );

	depthcheck = gtk_check_button_new_with_label( _( "Depth Cue" ) );
	gtk_widget_show( depthcheck );
	gtk_table_attach( GTK_TABLE( table ), depthcheck, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	g_signal_connect( G_OBJECT( depthcheck ), "toggled", G_CALLBACK( OnFog ), NULL );

	linescheck = gtk_check_button_new_with_label( _( "Lines" ) );
	gtk_widget_show( linescheck );
	gtk_table_attach( GTK_TABLE( table ), linescheck, 2, 3, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	g_signal_connect( G_OBJECT( linescheck ), "toggled", G_CALLBACK( OnLines ), NULL );

	polyscheck = gtk_check_button_new_with_label( _( "Polygons" ) );
	gtk_widget_show( polyscheck );
	gtk_table_attach( GTK_TABLE( table ), polyscheck, 3, 4, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	g_signal_connect( G_OBJECT( polyscheck ), "toggled", G_CALLBACK( OnPoly ), NULL );

	zlist = gtk_option_menu_new();
	gtk_widget_show( zlist );
	gtk_box_pack_start( GTK_BOX( vbox2 ), zlist, TRUE, FALSE, 0 );

	menu = gtk_menu_new();
	gtk_widget_show( menu );
	gtk_option_menu_set_menu( GTK_OPTION_MENU( zlist ), menu );

	item = gtk_menu_item_new_with_label( _( "Z-Buffer Test and Write (recommended for solid or no polygons)" ) );
	gtk_widget_show( item );
	g_signal_connect( G_OBJECT( item ), "activate",
						G_CALLBACK( OnSelchangeZbuffer ), GINT_TO_POINTER( 0 ) );
	gtk_menu_append( GTK_MENU( menu ), item );

	item = gtk_menu_item_new_with_label( _( "Z-Buffer Test Only (recommended for transparent polygons)" ) );
	gtk_widget_show( item );
	g_signal_connect( G_OBJECT( item ), "activate",
						G_CALLBACK( OnSelchangeZbuffer ), GINT_TO_POINTER( 1 ) );
	gtk_menu_append( GTK_MENU( menu ), item );

	item = gtk_menu_item_new_with_label( _( "Z-Buffer Off" ) );
	gtk_widget_show( item );
	g_signal_connect( G_OBJECT( item ), "activate",
						G_CALLBACK( OnSelchangeZbuffer ), GINT_TO_POINTER( 2 ) );
	gtk_menu_append( GTK_MENU( menu ), item );

	table = gtk_table_new( 2, 2, FALSE );
	gtk_widget_show( table );
	gtk_box_pack_start( GTK_BOX( vbox2 ), table, TRUE, TRUE, 0 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );

	adj = gtk_adjustment_new( portals.trans_3d, 0, 100, 1, 1, 1 );
	transslider = gtk_hscale_new( GTK_ADJUSTMENT( adj ) );
	gtk_widget_show( transslider );
	gtk_table_attach( GTK_TABLE( table ), transslider, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_scale_set_draw_value( GTK_SCALE( transslider ), FALSE );

	translabel = gtk_label_new( "" );
	gtk_widget_show( translabel );
	gtk_table_attach( GTK_TABLE( table ), translabel, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( translabel ), 0.0, 0.0 );
	g_signal_connect( adj, "value-changed", G_CALLBACK( OnScrollTrans ), translabel );

	adj = gtk_adjustment_new( portals.clip_range, 1, 128, 1, 1, 1 );
	clipslider = gtk_hscale_new( GTK_ADJUSTMENT( adj ) );
	gtk_widget_show( clipslider );
	gtk_table_attach( GTK_TABLE( table ), clipslider, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_scale_set_draw_value( GTK_SCALE( clipslider ), FALSE );

	cliplabel = gtk_label_new( "" );
	gtk_widget_show( cliplabel );
	gtk_table_attach( GTK_TABLE( table ), cliplabel, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( cliplabel ), 0.0, 0.0 );
	g_signal_connect( adj, "value-changed", G_CALLBACK( OnScrollClip ), cliplabel );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_set_homogeneous( GTK_BOX( hbox ), TRUE );
	gtk_widget_show( hbox );
	gtk_box_pack_start( GTK_BOX( vbox2 ), hbox, TRUE, FALSE, 0 );

	show3check = gtk_check_button_new_with_label( _( "Show" ) );
	gtk_widget_show( show3check );
	gtk_box_pack_start( GTK_BOX( hbox ), show3check, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( show3check ), "toggled", G_CALLBACK( OnConfig3d ), NULL );

	portalcheck = gtk_check_button_new_with_label( _( "Portal cubic clipper" ) );
	gtk_widget_show( portalcheck );
	gtk_box_pack_start( GTK_BOX( hbox ), portalcheck, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( portalcheck ), "toggled", G_CALLBACK( OnClip ), NULL );

	frame = gtk_frame_new( _( "2D View" ) );
	gtk_widget_show( frame );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, TRUE, TRUE, 0 );

	vbox2 = gtk_vbox_new( FALSE, 5 );
	gtk_widget_show( vbox2 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox2 );
	gtk_container_set_border_width( GTK_CONTAINER( vbox2 ), 5 );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_widget_show( hbox );
	gtk_box_pack_start( GTK_BOX( vbox2 ), hbox, TRUE, FALSE, 0 );

	adj = gtk_adjustment_new( portals.width_2d, 2, 40, 1, 1, 1 );
	lw2slider = gtk_hscale_new( GTK_ADJUSTMENT( adj ) );
	gtk_widget_show( lw2slider );
	gtk_box_pack_start( GTK_BOX( hbox ), lw2slider, TRUE, TRUE, 0 );
	gtk_scale_set_draw_value( GTK_SCALE( lw2slider ), FALSE );

	lw2label = gtk_label_new( "" );
	gtk_widget_show( lw2label );
	gtk_box_pack_start( GTK_BOX( hbox ), lw2label, FALSE, TRUE, 0 );
	g_signal_connect( adj, "value-changed", G_CALLBACK( OnScroll2d ), lw2label );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_widget_show( hbox );
	gtk_box_pack_start( GTK_BOX( vbox2 ), hbox, TRUE, FALSE, 0 );

	button = gtk_button_new_with_label( _( "Color" ) );
	gtk_widget_show( button );
	gtk_box_pack_start( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( OnColor2d ), NULL );
	gtk_widget_set_size_request( button, 60, -2 );

	aa2check = gtk_check_button_new_with_label( _( "Anti-Alias (May not work on some video cards)" ) );
	gtk_widget_show( aa2check );
	gtk_box_pack_start( GTK_BOX( hbox ), aa2check, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( aa2check ), "toggled", G_CALLBACK( OnAntiAlias2d ), NULL );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_widget_show( hbox );
	gtk_box_pack_start( GTK_BOX( vbox2 ), hbox, TRUE, FALSE, 0 );

	show2check = gtk_check_button_new_with_label( _( "Show" ) );
	gtk_widget_show( show2check );
	gtk_box_pack_start( GTK_BOX( hbox ), show2check, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( show2check ), "toggled", G_CALLBACK( OnConfig2d ), NULL );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_widget_show( hbox );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );

	button = gtk_button_new_with_label( _( "OK" ) );
	gtk_widget_show( button );
	gtk_box_pack_end( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( button ), "clicked",
						G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDOK ) );
	gtk_widget_set_size_request( button, 60, -2 );

	// initialize dialog
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( show2check ), portals.show_2d );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( aa2check ), portals.aa_2d );
	Set2DText( lw2label );

	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( show3check ), portals.show_3d );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( depthcheck ), portals.fog );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( polyscheck ), portals.polygons );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( linescheck ), portals.lines );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( aa3check ), portals.aa_3d );
	gtk_option_menu_set_history( GTK_OPTION_MENU( zlist ), portals.zbuffer );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( portalcheck ), portals.clip );

	Set3DText( lw3label );
	Set3DTransText( translabel );
	SetClipText( cliplabel );

	gtk_grab_add( dlg );
	gtk_widget_show( dlg );

	while ( loop )
		gtk_main_iteration();

	gtk_grab_remove( dlg );
	gtk_widget_destroy( dlg );
}
