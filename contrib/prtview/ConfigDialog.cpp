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

// ConfigDialog.cpp : implementation file
//

#include "stdafx.h"
#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
#endif
#include <glib/gi18n.h>

/////////////////////////////////////////////////////////////////////////////
// CConfigDialog dialog

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

// =============================================================================
// Color selection dialog

static int DoColor( GdkRGBA *color ){
	GtkWidget* dialog;
	GdkRGBA defaultcolor;
	gint response_id;
	int result;

	defaultcolor.red = color->red;
	defaultcolor.green = color->green;
	defaultcolor.blue = color->blue;
	defaultcolor.alpha = color->alpha;

	dialog = gtk_color_chooser_dialog_new( NULL, NULL );
	gtk_color_chooser_set_rgba( GTK_COLOR_CHOOSER( dialog ), &defaultcolor );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );
	if ( response_id == GTK_RESPONSE_OK )
    {
		gtk_color_chooser_get_rgba( GTK_COLOR_CHOOSER( dialog ), color );

		result = IDOK;
	} else {
		result = 0;
	}
	gtk_widget_destroy( dialog );
	return result;
}

static void Set2DText( GtkWidget* label ){
	char s[64];

	snprintf( s, sizeof( s ), "Line Width = %6.3f", portals.width_2d * 0.5f );

	gtk_label_set_text( GTK_LABEL( label ), s );
}

static void Set3DText( GtkWidget* label ){
	char s[64];

	snprintf( s, sizeof( s ), "Line Width = %6.3f", portals.width_3d * 0.5f );

	gtk_label_set_text( GTK_LABEL( label ), s );
}

static void Set3DTransText( GtkWidget* label ){
	char s[40];

	snprintf( s, sizeof( s ), "Polygon transparency = %d%%", (int)portals.trans_3d );

	gtk_label_set_text( GTK_LABEL( label ), s );
}

static void SetClipText( GtkWidget* label ){
	char s[40];

	snprintf( s, sizeof( s ), "Cubic clip range = %d", (int)portals.clip_range * 64 );

	gtk_label_set_text( GTK_LABEL( label ), s );
}

static void OnScroll2d( GtkAdjustment *adj, gpointer data ){
	portals.width_2d = gtk_adjustment_get_value( adj );
	Set2DText( GTK_WIDGET( data ) );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_2D );
	}
}

static void OnScroll3d( GtkAdjustment *adj, gpointer data ){
	portals.width_3d = gtk_adjustment_get_value( adj );
	Set3DText( GTK_WIDGET( data ) );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
	}
}

static void OnScrollTrans( GtkAdjustment *adj, gpointer data ){
	portals.trans_3d = gtk_adjustment_get_value( adj );
	Set3DTransText( GTK_WIDGET( data ) );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
	}
}

static void OnScrollClip( GtkAdjustment *adj, gpointer data ){
	portals.clip_range = gtk_adjustment_get_value( adj );
	SetClipText( GTK_WIDGET( data ) );

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
	}
}

static void OnAntiAlias2d( GtkWidget *widget, gpointer data ){
	portals.aa_2d = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( widget ) ) ? true : false;

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_2D );
	}
}

static void OnConfig2d( GtkWidget *widget, gpointer data ){
	portals.show_2d = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( widget ) ) ? true : false;

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
	portals.show_3d = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( widget ) ) ? true : false;

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
	}
}


static void OnAntiAlias3d( GtkWidget *widget, gpointer data ){
	portals.aa_3d = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( widget ) ) ? true : false;

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
	portals.fog = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( widget ) ) ? true : false;

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
	}
}

static void OnSelchangeZbuffer( GtkWidget *widget, gpointer data ){
	portals.zbuffer = gtk_combo_box_get_active( GTK_COMBO_BOX( GTK_COMBO_BOX_TEXT( widget ) ) );

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
	portals.clip = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( widget ) ) ? true : false;

	if ( interfaces_started ) {
		g_FuncTable.m_pfnSysUpdateWindows( UPDATE_3D );
	}
}

void DoConfigDialog(){
	GtkWidget *dialog, *hbox, *vbox, *vbox2, *button, *table, *frame;
	GtkWidget *lw3slider, *lw3label, *lw2slider, *lw2label, *zlist;
	GtkWidget *aa2check, *aa3check, *depthcheck, *linescheck, *polyscheck;
	GtkWidget *transslider, *translabel, *clipslider, *cliplabel;
	GtkWidget *show2check, *show3check, *portalcheck;
	GtkWidget *content_area, *color_button, *depth_button;
	gint response_id;
	GtkSizeGroup *button_group;
	GList *combo_list;
	GList *lst;
	GtkAdjustment *adj;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Portal Viewer Configuration" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	frame = gtk_frame_new( _( "3D View" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, TRUE, TRUE, 0 );
	gtk_widget_show( frame );

	vbox2 = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox2 );
	gtk_container_set_border_width( GTK_CONTAINER( vbox2 ), 5 );
	gtk_widget_show( vbox2 );

	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_box_pack_start( GTK_BOX( vbox2 ), hbox, TRUE, TRUE, 0 );
	gtk_widget_show( hbox );

	adj = gtk_adjustment_new( portals.width_3d, 2, 40, 1, 1, 1 );
	lw3slider = gtk_scale_new( GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT( adj ) );
	gtk_box_pack_start( GTK_BOX( hbox ), lw3slider, TRUE, TRUE, 0 );
	gtk_scale_set_draw_value( GTK_SCALE( lw3slider ), FALSE );
	gtk_widget_show( lw3slider );

	lw3label = gtk_label_new( "" );
	gtk_box_pack_start( GTK_BOX( hbox ), lw3label, FALSE, TRUE, 0 );
	gtk_widget_show( lw3label );
	g_signal_connect( adj, "value_changed", G_CALLBACK( OnScroll3d ), lw3label );

	table = gtk_grid_new();
	gtk_box_pack_start( GTK_BOX( vbox2 ), table, TRUE, TRUE, 0 );
	gtk_grid_set_row_spacing( GTK_GRID( table ), 5 );
	gtk_grid_set_column_spacing( GTK_GRID( table ), 5 );
	gtk_widget_show( table );

	color_button = button = gtk_button_new_with_label( _( "Color" ) );
	gtk_grid_attach( GTK_GRID( table ), button, 0, 0, 1, 1 );
	gtk_widget_show( button );
	g_signal_connect( button, "clicked", G_CALLBACK( OnColor3d ), NULL );

	depth_button = button = gtk_button_new_with_label( _( "Depth Color" ) );
	gtk_grid_attach( GTK_GRID( table ), button, 0, 1, 1, 1 );
	gtk_widget_show( button );
	g_signal_connect( button, "clicked", G_CALLBACK( OnColorFog ), NULL );

	aa3check = gtk_check_button_new_with_label( "Anti-Alias (May not work on some video cards)" );
	gtk_grid_attach( GTK_GRID( table ), aa3check, 1, 0, 3, 1 );
	gtk_widget_show( aa3check );
	g_signal_connect( aa3check, "toggled", G_CALLBACK( OnAntiAlias3d ), NULL );

	depthcheck = gtk_check_button_new_with_label( _( "Depth Cue" ) );
	gtk_grid_attach( GTK_GRID( table ), depthcheck, 1, 1, 1, 1 );
	gtk_widget_show( depthcheck );
	g_signal_connect( depthcheck, "toggled", G_CALLBACK( OnFog ), NULL );

	linescheck = gtk_check_button_new_with_label( _( "Lines" ) );
	gtk_grid_attach( GTK_GRID( table ), linescheck, 2, 1, 1, 1 );
	gtk_widget_show( linescheck );
	g_signal_connect( linescheck, "toggled", G_CALLBACK( OnLines ), NULL );

	polyscheck = gtk_check_button_new_with_label( _( "Polygons" ) );
	gtk_grid_attach( GTK_GRID( table ), polyscheck, 3, 1, 1, 1 );
	gtk_widget_show( polyscheck );
	g_signal_connect( polyscheck, "toggled", G_CALLBACK( OnPoly ), NULL );

	combo_list = NULL;
	combo_list = g_list_append( combo_list, (void *)_( "Z-Buffer Test and Write (recommended for solid or no polygons)" ) );
	combo_list = g_list_append( combo_list, (void *)_( "Z-Buffer Test Only (recommended for transparent polygons)" ) );
	combo_list = g_list_append( combo_list, (void *)_( "Z-Buffer Off" ) );

	zlist = gtk_combo_box_text_new();
	for ( lst = combo_list; lst != NULL; lst = g_list_next( lst ) )
	{
		gtk_combo_box_text_append( GTK_COMBO_BOX_TEXT( zlist ), (const char *)lst->data, (const char *)lst->data );
	}
	g_list_free( combo_list );
	combo_list = NULL;

	gtk_widget_show( zlist );
	gtk_box_pack_start( GTK_BOX( vbox2 ), zlist, TRUE, FALSE, 0 );

	g_signal_connect( zlist, "changed", G_CALLBACK( OnSelchangeZbuffer ), NULL );

	table = gtk_grid_new();
	gtk_container_add( GTK_CONTAINER( vbox2 ), table );
	gtk_grid_set_row_spacing( GTK_GRID( table ), 5 );
	gtk_grid_set_column_spacing( GTK_GRID( table ), 5 );
	gtk_widget_show( table );

	adj = gtk_adjustment_new( portals.trans_3d, 0, 100, 1, 1, 1 );
	transslider = gtk_scale_new( GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT( adj ) );
	gtk_grid_attach( GTK_GRID( table ), transslider, 0, 0, 1, 1 );
	gtk_widget_set_hexpand( transslider, TRUE );
	gtk_widget_show( transslider );
	gtk_scale_set_draw_value( GTK_SCALE( transslider ), FALSE );

	translabel = gtk_label_new( "" );
	gtk_grid_attach( GTK_GRID( table ), translabel, 1, 0, 1, 1 );
	gtk_widget_set_halign( translabel, GTK_ALIGN_START );
	gtk_widget_show( translabel );
	g_signal_connect( adj, "value_changed", G_CALLBACK( OnScrollTrans ), translabel );

	adj = gtk_adjustment_new( portals.clip_range, 1, 128, 1, 1, 1 );
	clipslider = gtk_scale_new( GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT( adj ) );
	gtk_grid_attach( GTK_GRID( table ), clipslider, 0, 1, 1, 1 );
	gtk_widget_set_hexpand( clipslider, TRUE );
	gtk_widget_show( clipslider );
	gtk_scale_set_draw_value( GTK_SCALE( clipslider ), FALSE );

	cliplabel = gtk_label_new( "" );
	gtk_grid_attach( GTK_GRID( table ), cliplabel, 1, 1, 1, 1 );
	gtk_widget_set_halign( cliplabel, GTK_ALIGN_START );
	gtk_widget_show( cliplabel );
	g_signal_connect( adj, "value_changed", G_CALLBACK( OnScrollClip ), cliplabel );

	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_widget_show( hbox );
	gtk_box_pack_start( GTK_BOX( vbox2 ), hbox, TRUE, FALSE, 0 );

	show3check = gtk_check_button_new_with_label( _( "Show" ) );
	gtk_widget_show( show3check );
	gtk_box_pack_start( GTK_BOX( hbox ), show3check, TRUE, TRUE, 0 );
	g_signal_connect( show3check, "toggled", G_CALLBACK( OnConfig3d ), NULL );

	portalcheck = gtk_check_button_new_with_label( _( "Portal cubic clipper" ) );
	gtk_widget_show( portalcheck );
	gtk_box_pack_start( GTK_BOX( hbox ), portalcheck, TRUE, TRUE, 0 );
	g_signal_connect( portalcheck, "toggled", G_CALLBACK( OnClip ), NULL );

	frame = gtk_frame_new( _( "2D View" ) );
	gtk_widget_show( frame );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, TRUE, TRUE, 0 );

	vbox2 = gtk_box_new( GTK_ORIENTATION_VERTICAL, 5 );
	gtk_widget_show( vbox2 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox2 );
	gtk_container_set_border_width( GTK_CONTAINER( vbox2 ), 5 );

	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_widget_show( hbox );
	gtk_box_pack_start( GTK_BOX( vbox2 ), hbox, TRUE, FALSE, 0 );

	adj = gtk_adjustment_new( portals.width_2d, 2, 40, 1, 1, 1 );
	lw2slider = gtk_scale_new( GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT( adj ) );
	gtk_widget_show( lw2slider );
	gtk_box_pack_start( GTK_BOX( hbox ), lw2slider, TRUE, TRUE, 0 );
	gtk_scale_set_draw_value( GTK_SCALE( lw2slider ), FALSE );

	lw2label = gtk_label_new( "" );
	gtk_widget_show( lw2label );
	gtk_box_pack_start( GTK_BOX( hbox ), lw2label, FALSE, TRUE, 0 );
	g_signal_connect( adj, "value_changed", G_CALLBACK( OnScroll2d ), lw2label );

	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_box_pack_start( GTK_BOX( vbox2 ), hbox, TRUE, FALSE, 0 );
	gtk_widget_show( hbox );

	button = gtk_button_new_with_label( _( "Color" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );
	gtk_widget_show( button );
	g_signal_connect( button, "clicked", G_CALLBACK( OnColor2d ), NULL );

	button_group = gtk_size_group_new( GTK_SIZE_GROUP_BOTH );
	gtk_size_group_add_widget( button_group, color_button );
	gtk_size_group_add_widget( button_group, depth_button );
	gtk_size_group_add_widget( button_group, button );
	g_object_unref( button_group );

	aa2check = gtk_check_button_new_with_label( _( "Anti-Alias (May not work on some video cards)" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), aa2check, TRUE, TRUE, 0 );
	gtk_widget_show( aa2check );
	g_signal_connect( aa2check, "toggled", G_CALLBACK( OnAntiAlias2d ), NULL );

	hbox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 5 );
	gtk_box_pack_start( GTK_BOX( vbox2 ), hbox, TRUE, FALSE, 0 );
	gtk_widget_show( hbox );

	show2check = gtk_check_button_new_with_label( _( "Show" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), show2check, FALSE, FALSE, 0 );
	gtk_widget_show( show2check );
	g_signal_connect( show2check, "toggled", G_CALLBACK( OnConfig2d ), NULL );

	// initialize dialog
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( show2check ), portals.show_2d );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( aa2check ), portals.aa_2d );
	Set2DText( lw2label );

	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( show3check ), portals.show_3d );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( depthcheck ), portals.fog );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( polyscheck ), portals.polygons );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( linescheck ), portals.lines );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( aa3check ), portals.aa_3d );
	gtk_combo_box_set_active( GTK_COMBO_BOX( GTK_COMBO_BOX_TEXT( zlist ) ), portals.zbuffer );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( portalcheck ), portals.clip );

	Set3DText( lw3label );
	Set3DTransText( translabel );
	SetClipText( cliplabel );

	gtk_widget_show( dialog );

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	gtk_widget_destroy( dialog );
}
