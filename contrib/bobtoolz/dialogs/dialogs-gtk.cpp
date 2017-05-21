/*
   BobToolz plugin for GtkRadiant
   Copyright (C) 2001 Gordon Biggans

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

#include "../StdAfx.h"
#include "dialogs-gtk.h"
#include "../funchandlers.h"
#include "../lists.h"
#include "../misc.h"
#include <glib/gi18n.h>

/*--------------------------------
        Callback Functions
   ---------------------------------*/

typedef struct {
	GtkWidget *cbTexChange;
	GtkWidget *editTexOld, *editTexNew;

	GtkWidget *cbScaleHor, *cbScaleVert;
	GtkWidget *spinScaleHor, *spinScaleVert;

	GtkWidget *cbShiftHor, *cbShiftVert;
	GtkWidget *spinShiftHor, *spinShiftVert;

	GtkWidget *cbRotation;
	GtkWidget *spinRotation;
}dlg_texReset_t;

dlg_texReset_t dlgTexReset;

extern GtkWidget *g_pRadiantWnd;

void Update_TextureReseter();

static void dialog_button_callback_texreset_update( GtkWidget *widget, gpointer data ){
	Update_TextureReseter();
}

void Update_TextureReseter(){
	gboolean check;

	check = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( dlgTexReset.cbTexChange ) );
	gtk_editable_set_editable( GTK_EDITABLE( dlgTexReset.editTexNew ), check );
	gtk_editable_set_editable( GTK_EDITABLE( dlgTexReset.editTexOld ), check );

	check = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( dlgTexReset.cbScaleHor ) );
	gtk_editable_set_editable( GTK_EDITABLE( dlgTexReset.spinScaleHor ), check );

	check = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( dlgTexReset.cbScaleVert ) );
	gtk_editable_set_editable( GTK_EDITABLE( dlgTexReset.spinScaleVert ), check );

	check = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( dlgTexReset.cbShiftHor ) );
	gtk_editable_set_editable( GTK_EDITABLE( dlgTexReset.spinShiftHor ), check );

	check = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( dlgTexReset.cbShiftVert ) );
	gtk_editable_set_editable( GTK_EDITABLE( dlgTexReset.spinShiftVert ), check );

	check = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( dlgTexReset.cbRotation ) );
	gtk_editable_set_editable( GTK_EDITABLE( dlgTexReset.spinRotation ), check );
}

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

static void dialog_button_callback_settex( GtkWidget *widget, gpointer data ){
	gchar *text;
	TwinWidget* tw = (TwinWidget*)data;

	GtkEntry* entry = GTK_ENTRY( tw->one );
	GtkComboBoxText* combo = GTK_COMBO_BOX_TEXT( tw->two );

	text = gtk_combo_box_text_get_active_text( combo );
	gtk_entry_set_text( entry, text );
	g_free( text );
}

/*--------------------------------
    Data validation Routines
   ---------------------------------*/

bool ValidateTextFloat( const char* pData, const char* error_title, float* value ){
	if ( pData ) {
		float testNum = (float)atof( pData );

		if ( ( testNum == 0.0f ) && strcmp( pData, "0" ) ) {
			DoMessageBox( _( "Please Enter A Floating Point Number" ), error_title, MB_OK );
			return FALSE;
		}
		else
		{
			*value = testNum;
			return TRUE;
		}
	}

	DoMessageBox( _( "Please Enter A Floating Point Number" ), error_title, MB_OK );
	return FALSE;
}

bool ValidateTextFloatRange( const char* pData, float min, float max, const char* error_title, float* value ){
	char error_buffer[256];
	sprintf( error_buffer, _( "Please Enter A Floating Point Number Between %.3f and %.3f" ), min, max );

	if ( pData ) {
		float testNum = (float)atof( pData );

		if ( ( testNum < min ) || ( testNum > max ) ) {
			DoMessageBox( error_buffer, error_title, MB_OK );
			return FALSE;
		}
		else
		{
			*value = testNum;
			return TRUE;
		}
	}

	DoMessageBox( error_buffer, error_title, MB_OK );
	return FALSE;
}

bool ValidateTextIntRange( const char* pData, int min, int max, const char* error_title, int* value ){
	char error_buffer[256];
	sprintf( error_buffer, _( "Please Enter An Integer Between %i and %i" ), min, max );

	if ( pData ) {
		int testNum = atoi( pData );

		if ( ( testNum < min ) || ( testNum > max ) ) {
			DoMessageBox( error_buffer, error_title, MB_OK );
			return FALSE;
		}
		else
		{
			*value = testNum;
			return TRUE;
		}
	}

	DoMessageBox( error_buffer, error_title, MB_OK );
	return FALSE;
}

bool ValidateTextInt( const char* pData, const char* error_title, int* value ){
	if ( pData ) {
		int testNum = atoi( pData );

		if ( ( testNum == 0 ) && strcmp( pData, "0" ) ) {
			DoMessageBox( _( "Please Enter An Integer" ), error_title, MB_OK );
			return FALSE;
		}
		else
		{
			*value = testNum;
			return TRUE;
		}
	}

	DoMessageBox( _( "Please Enter An Integer" ), error_title, MB_OK );
	return FALSE;
}

/*--------------------------------
        Modal Dialog Boxes
   ---------------------------------*/

/*

   Major clean up of variable names etc required, excluding Mars's ones,
   which are nicely done :)

 */

int DoMessageBox( const char* lpText, const char* lpCaption, guint32 uType ){
	GtkWidget *dialog, *w, *vbox, *hbox, *content_area;
	gint response_id;
	int mode, ret;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( lpCaption, NULL, flags, NULL );	
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pRadiantWnd ) );
	gtk_window_set_position( GTK_WINDOW( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
	gtk_container_set_border_width( GTK_CONTAINER( dialog ), 10 );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_vbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_widget_show( vbox );

	w = gtk_label_new( lpText );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 2 );
	gtk_label_set_justify( GTK_LABEL( w ), GTK_JUSTIFY_LEFT );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	w = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 2 );
	gtk_widget_show( w );

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 2 );
	gtk_widget_show( hbox );

	mode = ( uType & MB_TYPEMASK );
	if ( mode == MB_OK ) {
		w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
		ret = IDOK;
	}
	else if ( mode ==  MB_OKCANCEL ) {
		w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
		w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );
		ret = IDCANCEL;
	}
	else if ( mode == MB_YESNOCANCEL ) {
		w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Yes" ), GTK_RESPONSE_YES );
		w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "No" ), GTK_RESPONSE_NO );
		w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );
		ret = IDCANCEL;
	}
	else /* if (mode == MB_YESNO) */
	{
		w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Yes" ), GTK_RESPONSE_YES );
		w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "No" ), GTK_RESPONSE_NO );
		ret = IDNO;
	}


	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	switch( response_id )
	{
	case GTK_RESPONSE_OK:
		ret = IDOK;
		break;
	case GTK_RESPONSE_CANCEL:
		ret = IDCANCEL;
		break;
	case GTK_RESPONSE_YES:
		ret = IDYES;
		break;
	case GTK_RESPONSE_NO:
		ret = IDNO;
		break;
	}

	gtk_widget_destroy( dialog );

	return ret;
}

int DoIntersectBox( IntersectRS* rs ){
	GtkWidget *window, *w, *vbox, *hbox;
	GtkWidget *radio1, *radio2;
	GtkWidget *check1, *check2;
	int ret, loop = 1;

	window = gtk_window_new( GTK_WINDOW_TOPLEVEL );

	g_signal_connect( G_OBJECT( window ), "delete-event", G_CALLBACK( dialog_delete_callback ), NULL );
	g_signal_connect( G_OBJECT( window ), "destroy", G_CALLBACK( gtk_widget_destroy ), NULL );

	gtk_window_set_title( GTK_WINDOW( window ), _( "Intersect" ) );
	gtk_container_set_border_width( GTK_CONTAINER( window ), 10 );
	gtk_window_set_transient_for( GTK_WINDOW( window ), GTK_WINDOW( g_pRadiantWnd ) );
	gtk_window_set_position( GTK_WINDOW( window ), GTK_WIN_POS_CENTER_ON_PARENT );

	g_object_set_data( G_OBJECT( window ), "loop", &loop );
	g_object_set_data( G_OBJECT( window ), "ret", &ret );

	gtk_widget_realize( window );



	vbox = gtk_vbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( window ), vbox );
	gtk_widget_show( vbox );

	// ---- vbox ----

	radio1 = gtk_radio_button_new_with_label( NULL, _( "Use Whole Map" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), radio1, FALSE, FALSE, 2 );
	gtk_widget_show( radio1 );

	radio2 = gtk_radio_button_new_with_label( gtk_radio_button_get_group( GTK_RADIO_BUTTON( radio1 ) ), _( "Use Selected Brushes" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), radio2, FALSE, FALSE, 2 );
	gtk_widget_show( radio2 );

	w = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 2 );
	gtk_widget_show( w );

	check1 = gtk_check_button_new_with_label( _( "Include Detail Brushes" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), check1, FALSE, FALSE, 0 );
	gtk_widget_show( check1 );

	check2 = gtk_check_button_new_with_label( _( "Select Duplicate Brushes Only" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), check2, FALSE, FALSE, 0 );
	gtk_widget_show( check2 );

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 2 );
	gtk_widget_show( hbox );

	// ---- hbox ---- ok/cancel buttons

	w = gtk_button_new_with_label( _( "Ok" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDOK ) );

	gtk_widget_set_can_default( w, TRUE );
	gtk_widget_grab_default( w );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( _( "Cancel" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDCANCEL ) );
	gtk_widget_show( w );
	ret = IDCANCEL;

	// ---- /hbox ----

	// ---- /vbox ----

	gtk_widget_show( window );
	gtk_grab_add( window );

	while ( loop )
		gtk_main_iteration();

	if ( gtk_toggle_button_get_active( (GtkToggleButton*)radio1 ) ) {
		rs->nBrushOptions = BRUSH_OPT_WHOLE_MAP;
	}
	else if ( gtk_toggle_button_get_active( (GtkToggleButton*)radio2 ) ) {
		rs->nBrushOptions = BRUSH_OPT_SELECTED;
	}

	rs->bUseDetail = gtk_toggle_button_get_active( (GtkToggleButton*)check1 ) ? true : false;
	rs->bDuplicateOnly = gtk_toggle_button_get_active( (GtkToggleButton*)check2 ) ? true : false;

	gtk_grab_remove( window );
	gtk_widget_destroy( window );

	return ret;
}

int DoPolygonBox( PolygonRS* rs ){
	GtkWidget *dialog, *w, *vbox, *hbox, *vbox2, *hbox2;
	GtkSizeGroup *label_group;
	GtkWidget *check1, *check2, *check3;
	GtkWidget *text1, *text2;
	GtkWidget *sides_label, *width_label, *content_area;
	gint response_id;
	int ret;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Polygon Builder" ), NULL, flags, NULL );	
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pRadiantWnd ) );
	gtk_window_set_position( GTK_WINDOW( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
	gtk_container_set_border_width( GTK_CONTAINER( dialog ), 5 );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );


	vbox = gtk_vbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_widget_show( vbox );

	// ---- vbox ----

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 2 );
	gtk_widget_show( hbox );

	// ---- hbox ----


	vbox2 = gtk_vbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox2, FALSE, FALSE, 2 );
	gtk_widget_show( vbox2 );

	// ---- vbox2 ----

	hbox2 = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox2 ), hbox2, FALSE, FALSE, 2 );
	gtk_widget_show( hbox2 );

	// ---- hbox2 ----

	sides_label = w = gtk_label_new( _( "Number Of Sides" ) );
	gtk_box_pack_start( GTK_BOX( hbox2 ), w, FALSE, FALSE, 2 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	text1 = gtk_entry_new();
	gtk_entry_set_alignment( GTK_ENTRY( text1 ), 1.0 ); //right
	gtk_entry_set_max_length( GTK_ENTRY( text1 ), 256 );
	gtk_entry_set_text( GTK_ENTRY( text1 ), "3" );
	gtk_box_pack_start( GTK_BOX( hbox2 ), text1, FALSE, FALSE, 2 );
	gtk_widget_show( text1 );

	// ---- /hbox2 ----

	hbox2 = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox2 ), hbox2, FALSE, FALSE, 2 );
	gtk_widget_show( hbox2 );

	// ---- hbox2 ----

	width_label = w = gtk_label_new( _( "Border Width" ) );
	gtk_box_pack_start( GTK_BOX( hbox2 ), w, FALSE, FALSE, 2 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	text2 = gtk_entry_new();
	gtk_entry_set_alignment( GTK_ENTRY( text2 ), 1.0 ); //right
	gtk_entry_set_max_length( GTK_ENTRY( text2 ), 256 );
	gtk_entry_set_text( GTK_ENTRY( text2 ), "8" );
	gtk_box_pack_start( GTK_BOX( hbox2 ), text2, FALSE, FALSE, 2 );
	gtk_widget_show( text2 );

	// ---- /hbox2 ----

	// ---- /vbox2 ----

	label_group = gtk_size_group_new( GTK_SIZE_GROUP_HORIZONTAL );
	gtk_size_group_add_widget( label_group, sides_label );
	gtk_size_group_add_widget( label_group, width_label );
	g_object_unref( label_group );


	vbox2 = gtk_vbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox2, FALSE, FALSE, 2 );
	gtk_widget_show( vbox2 );

	// ---- vbox2 ----

	check1 = gtk_check_button_new_with_label( _( "Use Border" ) );
	gtk_box_pack_start( GTK_BOX( vbox2 ), check1, FALSE, FALSE, 0 );
	gtk_widget_show( check1 );


	check2 = gtk_check_button_new_with_label( _( "Inverse Polygon" ) );
	gtk_box_pack_start( GTK_BOX( vbox2 ), check2, FALSE, FALSE, 0 );
	gtk_widget_show( check2 );


	check3 = gtk_check_button_new_with_label( _( "Align Top Edge" ) );
	gtk_box_pack_start( GTK_BOX( vbox2 ), check3, FALSE, FALSE, 0 );
	gtk_widget_show( check3 );

	// ---- /vbox2 ----

	// ---- /hbox ----

	// ---- /vbox ----


	bool dialogError = TRUE;
	while ( dialogError )
	{
		response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

		dialogError = FALSE;

		if ( response_id == GTK_RESPONSE_OK ) {
			rs->bUseBorder = gtk_toggle_button_get_active( (GtkToggleButton*)check1 ) ? true : false;
			rs->bInverse = gtk_toggle_button_get_active( (GtkToggleButton*)check2 ) ? true : false;
			rs->bAlignTop = gtk_toggle_button_get_active( (GtkToggleButton*)check3 ) ? true : false;

			if ( !ValidateTextIntRange( gtk_entry_get_text( GTK_ENTRY( text1 ) ), 3, 32, _( "Number Of Sides" ), &rs->nSides ) ) {
				dialogError = TRUE;
			}

			if ( rs->bUseBorder ) {
				if ( !ValidateTextIntRange( gtk_entry_get_text( GTK_ENTRY( text2 ) ), 8, 256, _( "Border Width" ), &rs->nBorderWidth ) ) {
					dialogError = TRUE;
				}
			}
		}
	}

	if( response_id == GTK_RESPONSE_OK ) {
		ret = IDOK;
	} else {
		ret = IDCANCEL;
	}

	gtk_widget_destroy( dialog );

	return ret;
}

// mars
// for stair builder stuck as close as i could to the MFC version
// obviously feel free to change it at will :)
int DoBuildStairsBox( BuildStairsRS* rs ){
	// i made widgets for just about everything ... i think that's what i need to do  dunno tho
	GtkWidget   *window, *w, *vbox, *hbox;
	GtkWidget   *textStairHeight, *textRiserTex, *textMainTex;
	GtkWidget   *radioNorth, *radioSouth, *radioEast, *radioWest;   // i'm guessing we can't just abuse 'w' for these if we're getting a value
	GtkWidget   *radioOldStyle, *radioBobStyle, *radioCornerStyle;
	GtkWidget   *checkUseDetail;
	GSList      *radioDirection, *radioStyle;
	int ret, loop;

	loop = 1;

	const char    *text = _( "Please set a value in the boxes below and press 'OK' to build the stairs" );

	window = gtk_window_new( GTK_WINDOW_TOPLEVEL );

	g_signal_connect( G_OBJECT( window ), "delete-event", G_CALLBACK( dialog_delete_callback ), NULL );
	g_signal_connect( G_OBJECT( window ), "destroy", G_CALLBACK( gtk_widget_destroy ), NULL );

	gtk_window_set_title( GTK_WINDOW( window ), _( "Stair Builder" ) );
	gtk_container_set_border_width( GTK_CONTAINER( window ), 10 );
	gtk_window_set_transient_for( GTK_WINDOW( window ), GTK_WINDOW( g_pRadiantWnd ) );
	gtk_window_set_position( GTK_WINDOW( window ), GTK_WIN_POS_CENTER_ON_PARENT );

	g_object_set_data( G_OBJECT( window ), "loop", &loop );
	g_object_set_data( G_OBJECT( window ), "ret", &ret );

	gtk_widget_realize( window );

	// new vbox
	vbox = gtk_vbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( window ), vbox );
	gtk_widget_show( vbox );

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( vbox ), hbox );
	gtk_widget_show( hbox );

	// dunno if you want this text or not ...
	w = gtk_label_new( text );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 ); // not entirely sure on all the parameters / what they do ...
	gtk_widget_show( w );

	w = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

	// ------------------------- // indenting == good way of keeping track of lines :)

	// new hbox
	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	textStairHeight = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( textStairHeight ), 256 );
	gtk_box_pack_start( GTK_BOX( hbox ), textStairHeight, FALSE, FALSE, 1 );
	gtk_widget_show( textStairHeight );

	w = gtk_label_new( _( "Stair Height" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 1 );
	gtk_widget_show( w );

	// ------------------------- //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_label_new( _( "Direction:" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 5 );
	gtk_widget_show( w );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	// radio buttons confuse me ...
	// but this _looks_ right

	// djbob: actually it looks very nice :), slightly better than the way i did it
	// edit: actually it doesn't work :P, you must pass the last radio item each time, ugh

	radioNorth = gtk_radio_button_new_with_label( NULL, _( "North" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), radioNorth, FALSE, FALSE, 3 );
	gtk_widget_show( radioNorth );

	radioDirection = gtk_radio_button_get_group( GTK_RADIO_BUTTON( radioNorth ) );

	radioSouth = gtk_radio_button_new_with_label( radioDirection, _( "South" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), radioSouth, FALSE, FALSE, 2 );
	gtk_widget_show( radioSouth );

	radioDirection = gtk_radio_button_get_group( GTK_RADIO_BUTTON( radioSouth ) );

	radioEast = gtk_radio_button_new_with_label( radioDirection, _( "East" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), radioEast, FALSE, FALSE, 1 );
	gtk_widget_show( radioEast );

	radioDirection = gtk_radio_button_get_group( GTK_RADIO_BUTTON( radioEast ) );

	radioWest = gtk_radio_button_new_with_label( radioDirection, _( "West" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), radioWest, FALSE, FALSE, 0 );
	gtk_widget_show( radioWest );

	// --------------------------- //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_label_new( _( "Style:" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 5 );
	gtk_widget_show( w );

	// --------------------------- //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	radioOldStyle = gtk_radio_button_new_with_label( NULL, _( "Original" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), radioOldStyle, FALSE, FALSE, 0 );
	gtk_widget_show( radioOldStyle );

	radioStyle = gtk_radio_button_get_group( GTK_RADIO_BUTTON( radioOldStyle ) );

	radioBobStyle = gtk_radio_button_new_with_label( radioStyle, _( "Bob's Style" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), radioBobStyle, FALSE, FALSE, 0 );
	gtk_widget_show( radioBobStyle );

	radioStyle = gtk_radio_button_get_group( GTK_RADIO_BUTTON( radioBobStyle ) );

	radioCornerStyle = gtk_radio_button_new_with_label( radioStyle, _( "Corner Style" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), radioCornerStyle, FALSE, FALSE, 0 );
	gtk_widget_show( radioCornerStyle );

	// err, the q3r has an if or something so you need bob style checked before this
	// is "ungreyed out" but you'll need to do that, as i suck :)

	// djbob: er.... yeah um, im not at all sure how i'm gonna sort this
	// djbob: think we need some button callback functions or smuffin
	// FIXME: actually get around to doing what i suggested!!!!

	checkUseDetail = gtk_check_button_new_with_label( _( "Use Detail Brushes" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), checkUseDetail, FALSE, FALSE, 0 );
	gtk_widget_show( checkUseDetail );

	// --------------------------- //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	textMainTex = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( textMainTex ), 512 );
	gtk_entry_set_text( GTK_ENTRY( textMainTex ), rs->mainTexture );
	gtk_box_pack_start( GTK_BOX( hbox ), textMainTex, FALSE, FALSE, 0 );
	gtk_widget_show( textMainTex );

	w = gtk_label_new( _( "Main Texture" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 1 );
	gtk_widget_show( w );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	textRiserTex = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( textRiserTex ), 512 );
	gtk_box_pack_start( GTK_BOX( hbox ), textRiserTex, FALSE, FALSE, 0 );
	gtk_widget_show( textRiserTex );

	w = gtk_label_new( _( "Riser Texture" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 1 );
	gtk_widget_show( w );

	// -------------------------- //
	w = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_button_new_with_label( _( "OK" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDOK ) );
	gtk_widget_set_can_default( w, TRUE );
	gtk_widget_grab_default( w );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( _( "Cancel" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDCANCEL ) );
	gtk_widget_show( w );

	ret = IDCANCEL;

// +djbob: need our "little" modal loop mars :P
	gtk_widget_show( window );
	gtk_grab_add( window );

	bool dialogError = TRUE;
	while ( dialogError )
	{
		loop = 1;
		while ( loop )
			gtk_main_iteration();

		dialogError = FALSE;

		if ( ret == IDOK ) {
			rs->bUseDetail = gtk_toggle_button_get_active( (GtkToggleButton*)checkUseDetail ) ? true : false;

			strcpy( rs->riserTexture, gtk_entry_get_text( GTK_ENTRY( textRiserTex ) ) );
			strcpy( rs->mainTexture, gtk_entry_get_text( GTK_ENTRY( textMainTex ) ) );

			if ( gtk_toggle_button_get_active( (GtkToggleButton*)radioNorth ) ) {
				rs->direction = MOVE_NORTH;
			}
			else if ( gtk_toggle_button_get_active( (GtkToggleButton*)radioSouth ) ) {
				rs->direction = MOVE_SOUTH;
			}
			else if ( gtk_toggle_button_get_active( (GtkToggleButton*)radioEast ) ) {
				rs->direction = MOVE_EAST;
			}
			else if ( gtk_toggle_button_get_active( (GtkToggleButton*)radioWest ) ) {
				rs->direction = MOVE_WEST;
			}

			if ( !ValidateTextInt( gtk_entry_get_text( GTK_ENTRY( textStairHeight ) ), _( "Stair Height" ), &rs->stairHeight ) ) {
				dialogError = TRUE;
			}

			if ( gtk_toggle_button_get_active( (GtkToggleButton*)radioOldStyle ) ) {
				rs->style = STYLE_ORIGINAL;
			}
			else if ( gtk_toggle_button_get_active( (GtkToggleButton*)radioBobStyle ) ) {
				rs->style = STYLE_BOB;
			}
			else if ( gtk_toggle_button_get_active( (GtkToggleButton*)radioCornerStyle ) ) {
				rs->style = STYLE_CORNER;
			}
		}
	}

	gtk_grab_remove( window );
	gtk_widget_destroy( window );

	return ret;
// -djbob

	// there we go, all done ... on my end at least, not bad for a night's work
}

int DoDoorsBox( DoorRS* rs ){
	GtkWidget   *window, *hbox, *vbox, *w;
	GtkWidget   *textFrontBackTex, *textTrimTex;
	GtkWidget   *checkScaleMainH, *checkScaleMainV, *checkScaleTrimH, *checkScaleTrimV;
	GtkWidget   *comboMain, *comboTrim;
	GtkWidget   *buttonSetMain, *buttonSetTrim;
	GtkWidget   *radioNS, *radioEW;
	GSList      *radioOrientation;
	GList		*lst;
	TwinWidget tw1, tw2;
	int ret, loop;

	loop = 1;

	window = gtk_window_new( GTK_WINDOW_TOPLEVEL );

	g_signal_connect( G_OBJECT( window ), "delete-event", G_CALLBACK( dialog_delete_callback ), NULL );
	g_signal_connect( G_OBJECT( window ), "destroy", G_CALLBACK( gtk_widget_destroy ), NULL );

	gtk_window_set_title( GTK_WINDOW( window ), _( "Door Builder" ) );
	gtk_container_set_border_width( GTK_CONTAINER( window ), 10 );
	gtk_window_set_transient_for( GTK_WINDOW( window ), GTK_WINDOW( g_pRadiantWnd ) );
	gtk_window_set_position( GTK_WINDOW( window ), GTK_WIN_POS_CENTER_ON_PARENT );

	g_object_set_data( G_OBJECT( window ), "loop", &loop );
	g_object_set_data( G_OBJECT( window ), "ret", &ret );

	gtk_widget_realize( window );

	char buffer[256];
	GList       *listMainTextures = NULL;
	GList       *listTrimTextures = NULL;
	LoadGList( GetFilename( buffer, "plugins/bt/door-tex.txt" ), &listMainTextures );
	LoadGList( GetFilename( buffer, "plugins/bt/door-tex-trim.txt" ), &listTrimTextures );

	vbox = gtk_vbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( window ), vbox );
	gtk_widget_show( vbox );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	textFrontBackTex = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( textFrontBackTex ), 512 );
	gtk_entry_set_text( GTK_ENTRY( textFrontBackTex ), rs->mainTexture );
	gtk_box_pack_start( GTK_BOX( hbox ), textFrontBackTex, FALSE, FALSE, 0 );
	gtk_widget_show( textFrontBackTex );

	w = gtk_label_new( _( "Door Front/Back Texture" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

	// ------------------------ //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	textTrimTex = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( textTrimTex ), 512 );
	gtk_box_pack_start( GTK_BOX( hbox ), textTrimTex, FALSE, FALSE, 0 );
	gtk_widget_show( textTrimTex );

	w = gtk_label_new( _( "Door Trim Texture" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

	// ----------------------- //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	// sp: horizontally ????
	// djbob: yes mars, u can spell :]
	checkScaleMainH = gtk_check_button_new_with_label( _( "Scale Main Texture Horizontally" ) );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( checkScaleMainH ), TRUE );
	gtk_box_pack_start( GTK_BOX( hbox ), checkScaleMainH, FALSE, FALSE, 0 );
	gtk_widget_show( checkScaleMainH );

	checkScaleTrimH = gtk_check_button_new_with_label( _( "Scale Trim Texture Horizontally" ) );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( checkScaleTrimH ), TRUE );
	gtk_box_pack_start( GTK_BOX( hbox ), checkScaleTrimH, FALSE, FALSE, 0 );
	gtk_widget_show( checkScaleTrimH );

	// ---------------------- //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	checkScaleMainV = gtk_check_button_new_with_label( _( "Scale Main Texture Vertically" ) );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( checkScaleMainV ), TRUE );
	gtk_box_pack_start( GTK_BOX( hbox ), checkScaleMainV, FALSE, FALSE, 0 );
	gtk_widget_show( checkScaleMainV );

	checkScaleTrimV = gtk_check_button_new_with_label( _( "Scale Trim Texture Vertically" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), checkScaleTrimV, FALSE, FALSE, 0 );
	gtk_widget_show( checkScaleTrimV );

	// --------------------- //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	// djbob: lists added

	comboMain = gtk_combo_box_text_new();
	gtk_box_pack_start( GTK_BOX( hbox ), comboMain, FALSE, FALSE, 0 );
	for ( lst = listMainTextures; lst != NULL; lst = g_list_next( lst ) )
	{
		gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT( comboMain ), (const char *)lst->data );
	}
	gtk_widget_show( comboMain );

	tw1.one = textFrontBackTex;
	tw1.two = comboMain;

	buttonSetMain = gtk_button_new_with_label( _( "Set As Main Texture" ) );
	g_signal_connect( G_OBJECT( buttonSetMain ), "clicked", G_CALLBACK( dialog_button_callback_settex ), &tw1 );
	gtk_box_pack_start( GTK_BOX( hbox ), buttonSetMain, FALSE, FALSE, 0 );
	gtk_widget_show( buttonSetMain );

	// ------------------- //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	comboTrim = gtk_combo_box_text_new();
	gtk_box_pack_start( GTK_BOX( hbox ), comboTrim, FALSE, FALSE, 0 );
	for ( lst = listTrimTextures; lst != NULL; lst = g_list_next( lst ) )
	{
		gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT( comboTrim ), (const char *)lst->data );
	}
	gtk_widget_show( comboTrim );

	tw2.one = textTrimTex;
	tw2.two = comboTrim;

	buttonSetTrim = gtk_button_new_with_label( _( "Set As Trim Texture" ) );
	g_signal_connect( G_OBJECT( buttonSetTrim ), "clicked", G_CALLBACK( dialog_button_callback_settex ), &tw2 );
	gtk_box_pack_start( GTK_BOX( hbox ), buttonSetTrim, FALSE, FALSE, 0 );
	gtk_widget_show( buttonSetTrim );

	// ------------------ //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_label_new( _( "Orientation" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

	// argh more radio buttons!
	radioNS = gtk_radio_button_new_with_label( NULL, _( "North - South" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), radioNS, FALSE, FALSE, 0 );
	gtk_widget_show( radioNS );

	radioOrientation = gtk_radio_button_get_group( GTK_RADIO_BUTTON( radioNS ) );

	radioEW = gtk_radio_button_new_with_label( radioOrientation, _( "East - West" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), radioEW, FALSE, FALSE, 0 );
	gtk_widget_show( radioEW );

	// ----------------- //

	w = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

	// ----------------- //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_button_new_with_label( _( "OK" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDOK ) );
	gtk_widget_set_can_default( w, TRUE );
	gtk_widget_grab_default( w );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( _( "Cancel" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDCANCEL ) );
	gtk_widget_show( w );
	ret = IDCANCEL;

	// ----------------- //

//+djbob
	gtk_widget_show( window );
	gtk_grab_add( window );

	while ( loop )
		gtk_main_iteration();

	strcpy( rs->mainTexture, gtk_entry_get_text( GTK_ENTRY( textFrontBackTex ) ) );
	strcpy( rs->trimTexture, gtk_entry_get_text( GTK_ENTRY( textTrimTex ) ) );

	rs->bScaleMainH = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( checkScaleMainH ) ) ? true : false;
	rs->bScaleMainV = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( checkScaleMainV ) ) ? true : false;
	rs->bScaleTrimH = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( checkScaleTrimH ) ) ? true : false;
	rs->bScaleTrimV = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( checkScaleTrimV ) ) ? true : false;

	if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( radioNS ) ) ) {
		rs->nOrientation = DIRECTION_NS;
	}
	else if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( radioEW ) ) ) {
		rs->nOrientation = DIRECTION_EW;
	}

	gtk_grab_remove( window );
	gtk_widget_destroy( window );

	return ret;
//-djbob
}

int DoPathPlotterBox( PathPlotterRS* rs ){
	GtkWidget *window, *w, *vbox, *hbox;

	GtkWidget *text1, *text2, *text3;
	GtkWidget *check1, *check2;

	int ret, loop = 1;

	window = gtk_window_new( GTK_WINDOW_TOPLEVEL );

	g_signal_connect( G_OBJECT( window ), "delete-event", G_CALLBACK( dialog_delete_callback ), NULL );
	g_signal_connect( G_OBJECT( window ), "destroy", G_CALLBACK( gtk_widget_destroy ), NULL );

	gtk_window_set_title( GTK_WINDOW( window ), _( "Texture Reset" ) );
	gtk_container_set_border_width( GTK_CONTAINER( window ), 10 );
	gtk_window_set_transient_for( GTK_WINDOW( window ), GTK_WINDOW( g_pRadiantWnd ) );
	gtk_window_set_position( GTK_WINDOW( window ), GTK_WIN_POS_CENTER_ON_PARENT );

	g_object_set_data( G_OBJECT( window ), "loop", &loop );
	g_object_set_data( G_OBJECT( window ), "ret", &ret );

	gtk_widget_realize( window );



	vbox = gtk_vbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( window ), vbox );
	gtk_widget_show( vbox );

	// ---- vbox ----

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 2 );
	gtk_widget_show( hbox );

	// ---- hbox ----

	text1 = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( text1 ), 256 );
	gtk_entry_set_text( GTK_ENTRY( text1 ), "25" );
	gtk_box_pack_start( GTK_BOX( hbox ), text1, FALSE, FALSE, 2 );
	gtk_widget_show( text1 );

	w = gtk_label_new( _( "Number Of Points" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 2 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	// ---- /hbox ----

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 2 );
	gtk_widget_show( hbox );

	// ---- hbox ----

	text2 = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( text2 ), 256 );
	gtk_entry_set_text( GTK_ENTRY( text2 ), "3" );
	gtk_box_pack_start( GTK_BOX( hbox ), text2, FALSE, FALSE, 2 );
	gtk_widget_show( text2 );

	w = gtk_label_new( _( "Multipler" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 2 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	// ---- /hbox ----

	w = gtk_label_new( _( "Path Distance = dist(start -> apex) * multiplier" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 2 );
	gtk_widget_show( hbox );

	// ---- hbox ----

	text3 = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( text3 ), 256 );
	gtk_entry_set_text( GTK_ENTRY( text3 ), "-800" );
	gtk_box_pack_start( GTK_BOX( hbox ), text3, FALSE, FALSE, 2 );
	gtk_widget_show( text3 );

	w = gtk_label_new( _( "Gravity" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 2 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	// ---- /hbox ----

	w = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

	check1 = gtk_check_button_new_with_label( _( "No Dynamic Update" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), check1, FALSE, FALSE, 0 );
	gtk_widget_show( check1 );

	check2 = gtk_check_button_new_with_label( _( "Show Bounding Lines" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), check2, FALSE, FALSE, 0 );
	gtk_widget_show( check2 );

	// ---- /vbox ----


	// ----------------- //

	w = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

	// ----------------- //

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_button_new_with_label( _( "Enable" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDYES ) );
	gtk_widget_show( w );

	gtk_widget_set_can_default( w, TRUE );
	gtk_widget_grab_default( w );

	w = gtk_button_new_with_label( _( "Disable" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDNO ) );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( _( "Cancel" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDCANCEL ) );
	gtk_widget_show( w );

	ret = IDCANCEL;

	// ----------------- //

	gtk_widget_show( window );
	gtk_grab_add( window );

	bool dialogError = TRUE;
	while ( dialogError )
	{
		loop = 1;
		while ( loop )
			gtk_main_iteration();

		dialogError = FALSE;

		if ( ret == IDYES ) {
			if ( !ValidateTextIntRange( gtk_entry_get_text( GTK_ENTRY( text1 ) ), 1, 200, _( "Number Of Points" ), &rs->nPoints ) ) {
				dialogError = TRUE;
			}

			if ( !ValidateTextFloatRange( gtk_entry_get_text( GTK_ENTRY( text2 ) ), 1.0f, 10.0f, _( "Multiplier" ), &rs->fMultiplier ) ) {
				dialogError = TRUE;
			}

			if ( !ValidateTextFloatRange( gtk_entry_get_text( GTK_ENTRY( text3 ) ), -10000.0f, -1.0f, _( "Gravity" ), &rs->fGravity ) ) {
				dialogError = TRUE;
			}

			rs->bNoUpdate = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( check1 ) ) ? true : false;
			rs->bShowExtra = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( check2 ) ) ? true : false;
		}
	}

	gtk_grab_remove( window );
	gtk_widget_destroy( window );

	return ret;
}

int DoCTFColourChangeBox(){
	GtkWidget *window, *w, *vbox, *hbox;
	int ret, loop = 1;

	window = gtk_window_new( GTK_WINDOW_TOPLEVEL );

	g_signal_connect( G_OBJECT( window ), "delete-event", G_CALLBACK( dialog_delete_callback ), NULL );
	g_signal_connect( G_OBJECT( window ), "destroy", G_CALLBACK( gtk_widget_destroy ), NULL );

	gtk_window_set_title( GTK_WINDOW( window ), _( "CTF Colour Changer" ) );
	gtk_container_set_border_width( GTK_CONTAINER( window ), 10 );
	gtk_window_set_transient_for( GTK_WINDOW( window ), GTK_WINDOW( g_pRadiantWnd ) );
	gtk_window_set_position( GTK_WINDOW( window ), GTK_WIN_POS_CENTER_ON_PARENT );

	g_object_set_data( G_OBJECT( window ), "loop", &loop );
	g_object_set_data( G_OBJECT( window ), "ret", &ret );

	gtk_widget_realize( window );



	vbox = gtk_vbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( window ), vbox );
	gtk_widget_show( vbox );

	// ---- vbox ----

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, TRUE, TRUE, 0 );
	gtk_widget_show( hbox );

	// ---- hbox ---- ok/cancel buttons

	w = gtk_button_new_with_label( _( "Red->Blue" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDOK ) );

	gtk_widget_set_can_default( w, TRUE );
	gtk_widget_grab_default( w );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( _( "Blue->Red" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDYES ) );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( _( "Cancel" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDCANCEL ) );
	gtk_widget_show( w );
	ret = IDCANCEL;

	// ---- /hbox ----

	// ---- /vbox ----

	gtk_widget_show( window );
	gtk_grab_add( window );

	while ( loop )
		gtk_main_iteration();

	gtk_grab_remove( window );
	gtk_widget_destroy( window );

	return ret;
}

int DoResetTextureBox( ResetTextureRS* rs ){
	Str texSelected;
	gint response_id;
	GtkSizeGroup *label_group;
	GtkWidget *dialog, *w, *vbox, *hbox, *frame, *table, *content_area;
	GtkWidget *new_label, *old_label, *hscale_label, *vscale_label;
	GtkWidget *hshift_label, *vshift_label, *rvalue_label;
	int ret;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Texture Reset" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pRadiantWnd ) );
	gtk_window_set_position( GTK_WINDOW( dialog ), GTK_WIN_POS_CENTER );
	gtk_container_set_border_width( GTK_CONTAINER( dialog ), 5 );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Use Selected Brushes" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Use All Brushes" ), GTK_RESPONSE_YES );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );


	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_box_set_homogeneous( GTK_BOX( vbox ), TRUE );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_widget_show( vbox );

	// ---- vbox ----

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 2 );
	gtk_widget_show( hbox );

	// ---- hbox ----

	w = gtk_label_new( _( "Currently Selected Face:   " ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 2 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	texSelected = "";
	if ( g_SelectedFaceTable.m_pfnGetSelectedFaceCount() == 1 ) {
		texSelected += GetCurrentTexture();
	}

	w = gtk_label_new( texSelected );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 2 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	// ---- /hbox ----

	frame = gtk_frame_new( _( "Reset Texture Names" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, FALSE, TRUE, 0 );
	gtk_widget_show( frame );

	table = gtk_table_new( 2, 3, FALSE );

	gtk_container_add( GTK_CONTAINER( frame ), table );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_container_set_border_width( GTK_CONTAINER( table ), 5 );
	gtk_widget_show( table );

	// ---- frame ----

	dlgTexReset.cbTexChange = gtk_check_button_new_with_label( _( "Enabled" ) );
	g_signal_connect( G_OBJECT( dlgTexReset.cbTexChange ), "toggled", G_CALLBACK( dialog_button_callback_texreset_update ), NULL );
	gtk_table_attach( GTK_TABLE( table ), dlgTexReset.cbTexChange, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( dlgTexReset.cbTexChange );

	new_label = w = gtk_label_new( _( "Old Name: " ) );
	gtk_table_attach( GTK_TABLE( table ), w, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	dlgTexReset.editTexOld = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( dlgTexReset.editTexOld ), 256 );
	gtk_entry_set_text( GTK_ENTRY( dlgTexReset.editTexOld ), rs->textureName );
	gtk_table_attach( GTK_TABLE( table ), dlgTexReset.editTexOld, 2, 3, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( dlgTexReset.editTexOld );

	old_label = w = gtk_label_new( _( "New Name: " ) );
	gtk_table_attach( GTK_TABLE( table ), w, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	dlgTexReset.editTexNew = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( dlgTexReset.editTexNew ), 256 );
	gtk_entry_set_text( GTK_ENTRY( dlgTexReset.editTexNew ), rs->textureName );
	gtk_table_attach( GTK_TABLE( table ), dlgTexReset.editTexNew, 2, 3, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( dlgTexReset.editTexNew );

	// ---- /frame ----

	frame = gtk_frame_new( _( "Reset Scales" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, FALSE, TRUE, 0 );
	gtk_widget_show( frame );

	table = gtk_table_new( 2, 3, FALSE );

	gtk_container_add( GTK_CONTAINER( frame ), table );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_container_set_border_width( GTK_CONTAINER( table ), 5 );
	gtk_widget_show( table );

	// ---- frame ----

	dlgTexReset.cbScaleHor = gtk_check_button_new_with_label( _( "Enabled" ) );
	g_signal_connect( G_OBJECT( dlgTexReset.cbScaleHor ), "toggled", G_CALLBACK( dialog_button_callback_texreset_update ), NULL );
	gtk_table_attach( GTK_TABLE( table ), dlgTexReset.cbScaleHor, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( dlgTexReset.cbScaleHor );

	hscale_label = w = gtk_label_new( _( "New Horizontal Scale: " ) );
	gtk_table_attach( GTK_TABLE( table ), w, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );


	dlgTexReset.spinScaleHor = gtk_spin_button_new( GTK_ADJUSTMENT( gtk_adjustment_new( 0.5, 0, 65535, 0.1, 1, 0 ) ), 0.1, 1 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( dlgTexReset.spinScaleHor ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( dlgTexReset.spinScaleHor ), 1.0 ); //right align numbers
	gtk_table_attach( GTK_TABLE( table ), dlgTexReset.spinScaleHor, 2, 3, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( dlgTexReset.spinScaleHor );


	dlgTexReset.cbScaleVert = gtk_check_button_new_with_label( _( "Enabled" ) );
	g_signal_connect( G_OBJECT( dlgTexReset.cbScaleVert ), "toggled", G_CALLBACK( dialog_button_callback_texreset_update ), NULL );
	gtk_table_attach( GTK_TABLE( table ), dlgTexReset.cbScaleVert, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( dlgTexReset.cbScaleVert );

	vscale_label = w = gtk_label_new( _( "New Vertical Scale: " ) );
	gtk_table_attach( GTK_TABLE( table ), w, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	dlgTexReset.spinScaleVert = gtk_spin_button_new( GTK_ADJUSTMENT( gtk_adjustment_new( 0.5, 0, 65535, 0.1, 1, 0 ) ), 0.1, 1 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( dlgTexReset.spinScaleVert ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( dlgTexReset.spinScaleVert ), 1.0 ); //right align numbers
	gtk_table_attach( GTK_TABLE( table ), dlgTexReset.spinScaleVert, 2, 3, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( dlgTexReset.spinScaleVert );

	// ---- /frame ----

	frame = gtk_frame_new( _( "Reset Shift" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, FALSE, TRUE, 0 );
	gtk_widget_show( frame );

	table = gtk_table_new( 2, 3, FALSE );

	gtk_container_add( GTK_CONTAINER( frame ), table );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_container_set_border_width( GTK_CONTAINER( table ), 5 );
	gtk_widget_show( table );

	// ---- frame ----

	dlgTexReset.cbShiftHor = gtk_check_button_new_with_label( _( "Enabled" ) );
	g_signal_connect( G_OBJECT( dlgTexReset.cbShiftHor ), "toggled", G_CALLBACK( dialog_button_callback_texreset_update ), NULL );
	gtk_table_attach( GTK_TABLE( table ), dlgTexReset.cbShiftHor, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( dlgTexReset.cbShiftHor );

	hshift_label = w = gtk_label_new( _( "New Horizontal Shift: " ) );
	gtk_table_attach( GTK_TABLE( table ), w, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	dlgTexReset.spinShiftHor = gtk_spin_button_new( GTK_ADJUSTMENT( gtk_adjustment_new( 0, 0, 65535, 0.1, 1, 0 ) ), 0.1, 1 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( dlgTexReset.spinShiftHor ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( dlgTexReset.spinShiftHor ), 1.0 ); //right align numbers
	gtk_table_attach( GTK_TABLE( table ), dlgTexReset.spinShiftHor, 2, 3, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( dlgTexReset.spinShiftHor );


	dlgTexReset.cbShiftVert = gtk_check_button_new_with_label( _( "Enabled" ) );
	g_signal_connect( G_OBJECT( dlgTexReset.cbShiftVert ), "toggled", G_CALLBACK( dialog_button_callback_texreset_update ), NULL );
	gtk_table_attach( GTK_TABLE( table ), dlgTexReset.cbShiftVert, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( dlgTexReset.cbShiftVert );

	vshift_label = w = gtk_label_new( _( "New Vertical Shift: " ) );
	gtk_table_attach( GTK_TABLE( table ), w, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	dlgTexReset.spinShiftVert = gtk_spin_button_new( GTK_ADJUSTMENT( gtk_adjustment_new( 0, 0, 65535, 0.1, 1, 0 ) ), 0.1, 1 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( dlgTexReset.spinShiftVert ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( dlgTexReset.spinShiftVert ), 1.0 ); //right align numbers
	gtk_table_attach( GTK_TABLE( table ), dlgTexReset.spinShiftVert, 2, 3, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( dlgTexReset.spinShiftVert );

	// ---- /frame ----

	frame = gtk_frame_new( _( "Reset Rotation" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, FALSE, TRUE, 0 );
	gtk_widget_show( frame );

	table = gtk_table_new( 1, 3, FALSE );
	gtk_container_add( GTK_CONTAINER( frame ), table );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_container_set_border_width( GTK_CONTAINER( table ), 5 );
	gtk_widget_show( table );

	// ---- frame ----

	dlgTexReset.cbRotation = gtk_check_button_new_with_label( _( "Enabled" ) );
	gtk_table_attach( GTK_TABLE( table ), dlgTexReset.cbRotation, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( dlgTexReset.cbRotation );

	rvalue_label = w = gtk_label_new( _( "New Rotation Value: " ) );
	gtk_table_attach( GTK_TABLE( table ), w, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	dlgTexReset.spinRotation = gtk_spin_button_new( GTK_ADJUSTMENT( gtk_adjustment_new( 0, -360, 360, 1, 90, 0 ) ), 1, 1 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( dlgTexReset.spinRotation ), TRUE );
	gtk_entry_set_alignment( GTK_ENTRY( dlgTexReset.spinRotation ), 1.0 ); //right align numbers
	gtk_table_attach( GTK_TABLE( table ), dlgTexReset.spinRotation, 2, 3, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( dlgTexReset.spinRotation );

	// ---- /frame ----

	// ---- /vbox ----

	label_group = gtk_size_group_new( GTK_SIZE_GROUP_HORIZONTAL );
	gtk_size_group_add_widget( label_group, new_label );
	gtk_size_group_add_widget( label_group, old_label );
	gtk_size_group_add_widget( label_group, hscale_label );
	gtk_size_group_add_widget( label_group, vscale_label );
	gtk_size_group_add_widget( label_group, hshift_label );
	gtk_size_group_add_widget( label_group, vshift_label );
	gtk_size_group_add_widget( label_group, rvalue_label );
	g_object_unref( label_group );


	Update_TextureReseter();

	ret = IDCANCEL;
	bool dialogError = TRUE;
	while ( dialogError )
	{
		response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

		dialogError = FALSE;

		if ( response_id != GTK_RESPONSE_CANCEL ) {
			rs->bResetRotation = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( dlgTexReset.cbRotation ) );
			if ( rs->bResetRotation ) {
				rs->rotation = gtk_spin_button_get_value( GTK_SPIN_BUTTON( dlgTexReset.spinRotation ) );
			}

			rs->bResetScale[0] = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( dlgTexReset.cbScaleHor ) );
			if ( rs->bResetScale[0] ) {
				rs->fScale[0] = gtk_spin_button_get_value( GTK_SPIN_BUTTON( dlgTexReset.spinScaleHor ) );
			}

			rs->bResetScale[1] = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( dlgTexReset.cbScaleVert ) );
			if ( rs->bResetScale[1] ) {
				rs->fScale[1] = gtk_spin_button_get_value( GTK_SPIN_BUTTON( dlgTexReset.spinScaleVert ) );
			}

			rs->bResetShift[0] = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( dlgTexReset.cbShiftHor ) );
			if ( rs->bResetShift[0] ) {
				rs->fShift[0] = gtk_spin_button_get_value( GTK_SPIN_BUTTON( dlgTexReset.spinShiftHor ) );
			}

			rs->bResetShift[1] = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( dlgTexReset.cbShiftVert ) );
			if ( rs->bResetShift[1] ) {
				rs->fShift[1] = gtk_spin_button_get_value( GTK_SPIN_BUTTON( dlgTexReset.spinShiftVert ) );
			}

			rs->bResetTextureName = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( dlgTexReset.cbTexChange ) );
			if ( rs->bResetTextureName ) {
				strcpy( rs->textureName,     gtk_entry_get_text( GTK_ENTRY( dlgTexReset.editTexOld ) ) );
				strcpy( rs->newTextureName,  gtk_entry_get_text( GTK_ENTRY( dlgTexReset.editTexNew ) ) );
			}
		}
	}

	switch( response_id ) {
	case GTK_RESPONSE_OK:
		ret = IDOK;
		break;
	case GTK_RESPONSE_CANCEL:
		ret = IDCANCEL;
		break;
	case GTK_RESPONSE_YES:
		ret = IDYES;
		break;
	}

	gtk_widget_destroy( dialog );

	return ret;
}

int DoTrainThingBox( TrainThingRS* rs ){
	Str texSelected;

	GtkWidget *window, *w, *vbox, *hbox, *frame, *table;

	GtkWidget *radiusX, *radiusY;
	GtkWidget *angleStart, *angleEnd;
	GtkWidget *heightStart, *heightEnd;
	GtkWidget *numPoints;

	int ret, loop = 1;

	window = gtk_window_new( GTK_WINDOW_TOPLEVEL );

	g_signal_connect( G_OBJECT( window ), "delete-event", G_CALLBACK( dialog_delete_callback ), NULL );
	g_signal_connect( G_OBJECT( window ), "destroy", G_CALLBACK( gtk_widget_destroy ), NULL );

	gtk_window_set_title( GTK_WINDOW( window ), _( "Train Thing" ) );
	gtk_container_set_border_width( GTK_CONTAINER( window ), 10 );
	gtk_window_set_transient_for( GTK_WINDOW( window ), GTK_WINDOW( g_pRadiantWnd ) );
	gtk_window_set_position( GTK_WINDOW( window ), GTK_WIN_POS_CENTER_ON_PARENT );

	g_object_set_data( G_OBJECT( window ), "loop", &loop );
	g_object_set_data( G_OBJECT( window ), "ret", &ret );

	gtk_widget_realize( window );

	vbox = gtk_vbox_new( FALSE, 10 );
	gtk_container_add( GTK_CONTAINER( window ), vbox );
	gtk_widget_show( vbox );

	// ---- vbox ----

	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 2 );
	gtk_widget_show( hbox );

	// ---- /hbox ----

	frame = gtk_frame_new( _( "Radii" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, FALSE, TRUE, 0 );
	gtk_widget_show( frame );

	table = gtk_table_new( 2, 3, TRUE );
	gtk_widget_show( table );
	gtk_container_add( GTK_CONTAINER( frame ), table );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_container_set_border_width( GTK_CONTAINER( table ), 5 );

	// ---- frame ----

	w = gtk_label_new( _( "X: " ) );
	gtk_table_attach( GTK_TABLE( table ), w, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	radiusX = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( radiusX ), 256 );
	gtk_entry_set_text( GTK_ENTRY( radiusX ), "100" );
	gtk_table_attach( GTK_TABLE( table ), radiusX, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( radiusX );



	w = gtk_label_new( _( "Y: " ) );
	gtk_table_attach( GTK_TABLE( table ), w, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	radiusY = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( radiusY ), 256 );
	gtk_entry_set_text( GTK_ENTRY( radiusY ), "100" );
	gtk_table_attach( GTK_TABLE( table ), radiusY, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( radiusY );



	frame = gtk_frame_new( _( "Angles" ) );
	gtk_widget_show( frame );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, FALSE, TRUE, 0 );

	table = gtk_table_new( 2, 3, TRUE );
	gtk_container_add( GTK_CONTAINER( frame ), table );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_container_set_border_width( GTK_CONTAINER( table ), 5 );
	gtk_widget_show( table );

	// ---- frame ----

	w = gtk_label_new( _( "Start: " ) );
	gtk_table_attach( GTK_TABLE( table ), w, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	angleStart = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( angleStart ), 256 );
	gtk_entry_set_text( GTK_ENTRY( angleStart ), "0" );
	gtk_table_attach( GTK_TABLE( table ), angleStart, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( angleStart );



	w = gtk_label_new( _( "End: " ) );
	gtk_table_attach( GTK_TABLE( table ), w, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	angleEnd = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( angleEnd ), 256 );
	gtk_entry_set_text( GTK_ENTRY( angleEnd ), "90" );
	gtk_table_attach( GTK_TABLE( table ), angleEnd, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( angleEnd );


	frame = gtk_frame_new( _( "Height" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, FALSE, TRUE, 0 );
	gtk_widget_show( frame );

	table = gtk_table_new( 2, 3, TRUE );
	gtk_container_add( GTK_CONTAINER( frame ), table );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_container_set_border_width( GTK_CONTAINER( table ), 5 );
	gtk_widget_show( table );

	// ---- frame ----

	w = gtk_label_new( _( "Start: " ) );
	gtk_table_attach( GTK_TABLE( table ), w, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	heightStart = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( heightStart ), 256 );
	gtk_entry_set_text( GTK_ENTRY( heightStart ), "0" );
	gtk_table_attach( GTK_TABLE( table ), heightStart, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( heightStart );



	w = gtk_label_new( _( "End: " ) );
	gtk_table_attach( GTK_TABLE( table ), w, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	heightEnd = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( heightEnd ), 256 );
	gtk_entry_set_text( GTK_ENTRY( heightEnd ), "0" );
	gtk_table_attach( GTK_TABLE( table ), heightEnd, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( heightEnd );



	frame = gtk_frame_new( _( "Points" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, FALSE, TRUE, 0 );
	gtk_widget_show( frame );

	table = gtk_table_new( 2, 3, TRUE );
	gtk_container_add( GTK_CONTAINER( frame ), table );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_container_set_border_width( GTK_CONTAINER( table ), 5 );
	gtk_widget_show( table );

	// ---- frame ----

	w = gtk_label_new( _( "Number: " ) );
	gtk_table_attach( GTK_TABLE( table ), w, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	numPoints = gtk_entry_new();
	gtk_entry_set_max_length( GTK_ENTRY( numPoints ), 256 );
	gtk_entry_set_text( GTK_ENTRY( numPoints ), "0" );
	gtk_table_attach( GTK_TABLE( table ), numPoints, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( 0 ), 0, 0 );
	gtk_widget_show( numPoints );


	hbox = gtk_hbox_new( FALSE, 10 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 2 );
	gtk_widget_show( hbox );

	// ---- hbox ----

	w = gtk_button_new_with_label( _( "Ok" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDOK ) );

	gtk_widget_set_can_default( w, TRUE );
	gtk_widget_grab_default( w );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( _( "Cancel" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( dialog_button_callback ), GINT_TO_POINTER( IDCANCEL ) );
	gtk_widget_show( w );
	ret = IDCANCEL;

	// ---- /hbox ----



	gtk_widget_show( window );
	gtk_grab_add( window );

	bool dialogError = TRUE;
	while ( dialogError )
	{
		loop = 1;
		while ( loop )
			gtk_main_iteration();

		dialogError = FALSE;

		if ( ret != IDCANCEL ) {
			if ( !ValidateTextFloat( gtk_entry_get_text( GTK_ENTRY( radiusX ) ), _( "Radius (X)" ), &rs->fRadiusX ) ) {
				dialogError = TRUE;
			}

			if ( !ValidateTextFloat( gtk_entry_get_text( GTK_ENTRY( radiusY ) ), _( "Radius (Y)" ), &rs->fRadiusY ) ) {
				dialogError = TRUE;
			}

			if ( !ValidateTextFloat( gtk_entry_get_text( GTK_ENTRY( angleStart ) ), _( "Angle (Start)" ), &rs->fStartAngle ) ) {
				dialogError = TRUE;
			}

			if ( !ValidateTextFloat( gtk_entry_get_text( GTK_ENTRY( angleEnd ) ), _( "Angle (End)" ), &rs->fEndAngle ) ) {
				dialogError = TRUE;
			}

			if ( !ValidateTextFloat( gtk_entry_get_text( GTK_ENTRY( heightStart ) ), _( "Height (Start)" ), &rs->fStartHeight ) ) {
				dialogError = TRUE;
			}

			if ( !ValidateTextFloat( gtk_entry_get_text( GTK_ENTRY( heightEnd ) ), _( "Height (End)" ), &rs->fEndHeight ) ) {
				dialogError = TRUE;
			}

			if ( !ValidateTextInt( gtk_entry_get_text( GTK_ENTRY( numPoints ) ), _( "Num Points" ), &rs->iNumPoints ) ) {
				dialogError = TRUE;
			}
		}
	}

	gtk_grab_remove( window );
	gtk_widget_destroy( window );

	return ret;
}
