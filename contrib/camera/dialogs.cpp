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

/*
   Camera plugin for GtkRadiant
   Copyright (C) 2002 Splash Damage Ltd.
 */

#include "camera.h"

#include <glib.h>
#include <glib/gi18n.h>

static GSList *g_pEditTypeRadio = NULL;
static GtkWidget *g_pEditModeEditRadioButton = NULL;
GtkWidget *g_pEditModeAddRadioButton = NULL;
static GtkWidget *g_pSecondsEntry = NULL;
static GtkWidget *g_pEventsList = NULL;
static GtkLabel *g_pCurrentTime = NULL;
static GtkLabel *g_pTotalTime = NULL;
static GtkAdjustment *g_pTimeLine = NULL;
static GtkWidget *g_pTrackCamera = NULL;
static GtkWidget *g_pCamName = NULL;

#define EVENT_TEXT_COLUMN (0)
#define EVENT_INDEX_COLUMN (1)


static gint ci_editmode_edit( GtkWidget *widget, gpointer data ){
	g_iEditMode = 0;

	return TRUE;
}

static gint ci_editmode_add( GtkWidget *widget, gpointer data ){
	g_iEditMode = 1;

	return TRUE;
}

/*static gint ci_delete_selected( GtkWidget *widget, gpointer data )
   {
   return TRUE;
   }

   static gint ci_select_all( GtkWidget *widget, gpointer data )
   {
   return TRUE;
   }*/

static gint ci_new( GtkWidget *widget, gpointer data ){
	GtkWidget *dialog, *w, *vbox, *vbox2, *hbox, *frame; //, *name;
	GtkWidget *fixed, *interpolated, *spline, *content_area;
	GSList *targetTypeRadio = NULL;
//	char buf[128];
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	// create the window
	dialog = gtk_dialog_new_with_buttons( _( "New Camera" ), NULL, flags, NULL );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pCameraInspectorWnd ) );
	gtk_window_set_position( GTK_WINDOW( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	// fill the window
	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_widget_show( vbox );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	frame = gtk_frame_new( _( "Type" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), frame, TRUE, TRUE, 0 );
	gtk_widget_show( frame );

	vbox2 = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox2 );
	gtk_container_set_border_width( GTK_CONTAINER( vbox2 ), 5 );
	gtk_widget_show( vbox2 );

	// -------------------------- //

	fixed = gtk_radio_button_new_with_label( targetTypeRadio, _( "Fixed" ) );
	gtk_box_pack_start( GTK_BOX( vbox2 ), fixed, FALSE, FALSE, 3 );
	gtk_widget_show( fixed );
	targetTypeRadio = gtk_radio_button_get_group( GTK_RADIO_BUTTON( fixed ) );

	interpolated = gtk_radio_button_new_with_label( targetTypeRadio, _( "Interpolated" ) );
	gtk_box_pack_start( GTK_BOX( vbox2 ), interpolated, FALSE, FALSE, 3 );
	gtk_widget_show( interpolated );
	targetTypeRadio = gtk_radio_button_get_group( GTK_RADIO_BUTTON( interpolated ) );

	spline = gtk_radio_button_new_with_label( targetTypeRadio, _( "Spline" ) );
	gtk_box_pack_start( GTK_BOX( vbox2 ), spline, FALSE, FALSE, 3 );
	gtk_widget_show( spline );
	targetTypeRadio = gtk_radio_button_get_group( GTK_RADIO_BUTTON( spline ) );

	// -------------------------- //

	w = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 2 );
	gtk_widget_show( w );

	// -------------------------- //

	response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

	if ( response_id == GTK_RESPONSE_OK ) {

		if ( gtk_toggle_button_get_active( (GtkToggleButton*)fixed ) ) {
			DoNewFixedCamera();
		}
		else if ( gtk_toggle_button_get_active( (GtkToggleButton*)interpolated ) ) {
			DoNewInterpolatedCamera();
		}
		else if ( gtk_toggle_button_get_active( (GtkToggleButton*)spline ) ) {
			DoNewSplineCamera();
		}
	}

	gtk_widget_destroy( dialog );

	return TRUE;
}

static gint ci_load( GtkWidget *widget, gpointer data ){
	DoLoadCamera();

	return TRUE;
}

static gint ci_save( GtkWidget *widget, gpointer data ){
	DoSaveCamera();

	return TRUE;
}

static gint ci_unload( GtkWidget *widget, gpointer data ){
	DoUnloadCamera();

	return TRUE;
}

static gint ci_apply( GtkWidget *widget, gpointer data ){
	if ( GetCurrentCam() ) {
		const char *str;
		char buf[128];
		bool build = false;

		str = gtk_entry_get_text( GTK_ENTRY( g_pCamName ) );

		if ( str ) {
			GetCurrentCam()->GetCam()->setName( str );
			build = true;
		}

		str = gtk_entry_get_text( GTK_ENTRY( g_pSecondsEntry ) );

		if ( str ) {
			GetCurrentCam()->GetCam()->setBaseTime( atof( str ) );
			build = true;
		}

		if ( build ) {
			GetCurrentCam()->GetCam()->buildCamera();
		}

		sprintf( buf, "%.2f", GetCurrentCam()->GetCam()->getBaseTime() );
		gtk_entry_set_text( GTK_ENTRY( g_pSecondsEntry ), buf );

		sprintf( buf, "%.2f", GetCurrentCam()->GetCam()->getTotalTime() );
		gtk_label_set_text( g_pCurrentTime, "0.00" );
		gtk_label_set_text( g_pTotalTime, buf );

		gtk_adjustment_set_value( g_pTimeLine, 0.f );
		gtk_adjustment_set_upper( g_pTimeLine, GetCurrentCam()->GetCam()->getTotalTime() * 1000 );

		GetCurrentCam()->HasBeenModified();
	}

	return TRUE;
}

static gint ci_preview( GtkWidget *widget, gpointer data ){
	if ( GetCurrentCam() ) {
		g_iPreviewRunning = 1;
		g_FuncTable.m_pfnSysUpdateWindows( W_XY_OVERLAY | W_CAMERA );
	}

	return TRUE;
}

static void ci_show( GtkWidget *widget, gpointer data ){
	// start edit mode
	DoStartEdit( GetCurrentCam() );

	return;
}

static gint ci_close( GtkWidget *widget, gpointer data ){
	gtk_widget_hide( g_pCameraInspectorWnd );

	// exit edit mode
	DoStopEdit();

	return TRUE;
}

static GtkWidget *g_pPathListCombo = NULL;
static GtkLabel *g_pPathType = NULL;

static void RefreshPathListCombo( void ){
	GList *combo_list = (GList*)NULL;
	GList *lst;
	GtkListStore *store;

	if ( !g_pPathListCombo ) {
		return;
	}

	if ( GetCurrentCam() ) {
		combo_list = g_list_append( combo_list, (void *)GetCurrentCam()->GetCam()->getPositionObj()->getName() );
		for ( int i = 0; i < GetCurrentCam()->GetCam()->numTargets(); i++ ) {
			combo_list = g_list_append( combo_list, (void *)GetCurrentCam()->GetCam()->getActiveTarget( i )->getName() );
		}
	}
#if GTK_CHECK_VERSION( 3, 0, 0 )
	gtk_combo_box_text_remove_all( GTK_COMBO_BOX_TEXT( g_pPathListCombo ) );
#else
	store = GTK_LIST_STORE( gtk_combo_box_get_model( GTK_COMBO_BOX( g_pPathListCombo ) ) );
	gtk_list_store_clear( store );
#endif
	for( lst = combo_list; lst != NULL; lst = g_list_next( lst ) )
	{
		gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT( g_pPathListCombo ), (const gchar *)lst->data );
	}
	g_list_free( combo_list );

	gtk_combo_box_set_active( GTK_COMBO_BOX( g_pPathListCombo ), 0 );
}

static gint ci_pathlist_changed( GtkWidget *widget, gpointer data ){
	char *str = gtk_combo_box_text_get_active_text( GTK_COMBO_BOX_TEXT( widget ) );

	if ( !str || !GetCurrentCam() ) {
		return TRUE;
	}

	int i;
	for ( i = 0; i < GetCurrentCam()->GetCam()->numTargets(); i++ ) {
		if ( !strcmp( str, GetCurrentCam()->GetCam()->getActiveTarget( i )->getName() ) ) {
			break;
		}
	}

	if ( i >= 0 && i < GetCurrentCam()->GetCam()->numTargets() ) {
		GetCurrentCam()->GetCam()->setActiveTarget( i );

		g_iActiveTarget = i;
		if ( g_pPathType ) {
			gtk_label_set_text( g_pPathType, GetCurrentCam()->GetCam()->getActiveTarget( g_iActiveTarget )->typeStr() );
		}
	}
	else {
		g_iActiveTarget = -1;
		if ( g_pPathType ) {
			gtk_label_set_text( g_pPathType, GetCurrentCam()->GetCam()->getPositionObj()->typeStr() );
		}
	}

	// start edit mode
	if ( g_pCameraInspectorWnd && gtk_widget_get_visible( g_pCameraInspectorWnd ) ) {
		DoStartEdit( GetCurrentCam() );
	}

	g_free( str );

	return TRUE;
}

static void RefreshEventList( void ){
	int i;
	char buf[128];
	GtkListStore *store;

	store = GTK_LIST_STORE( GTK_TREE_MODEL( gtk_tree_view_get_model( GTK_TREE_VIEW( g_pEventsList ) ) ) );

	// Clear events list
	gtk_list_store_clear( store );

	if ( GetCurrentCam() ) {
		GtkTreeIter iter;

		// Fill events list
		for ( i = 0; i < GetCurrentCam()->GetCam()->numEvents(); i++ ) {
			char rowbuf[3][128], *row[3];
			// FIXME: sort by time?
			sprintf( rowbuf[0], "%li", GetCurrentCam()->GetCam()->getEvent( i )->getTime() );                 row[0] = rowbuf[0];
			strncpy( rowbuf[1], GetCurrentCam()->GetCam()->getEvent( i )->typeStr(), sizeof( rowbuf[0] ) );     row[1] = rowbuf[1];
			strncpy( rowbuf[2], GetCurrentCam()->GetCam()->getEvent( i )->getParam(), sizeof( rowbuf[1] ) );    row[2] = rowbuf[2];

			gtk_list_store_append( store, &iter );
			gtk_list_store_set( store, &iter, EVENT_TEXT_COLUMN, row, EVENT_INDEX_COLUMN, i, -1 );
		}

		// Total duration might have changed
		sprintf( buf, "%.2f", GetCurrentCam()->GetCam()->getTotalTime() );
		gtk_label_set_text( g_pCurrentTime, "0.00" );
		gtk_label_set_text( g_pTotalTime, buf );

		gtk_adjustment_set_value( g_pTimeLine, 0.f );
		gtk_adjustment_set_upper( g_pTimeLine, ( GetCurrentCam()->GetCam()->getTotalTime() * 1000 ) );
	}

}

static gint ci_rename( GtkWidget *widget, gpointer data ){
	GtkWidget *dialog, *w, *vbox, *hbox, *name, *content_area;
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	if ( !GetCurrentCam() ) {
		return TRUE;
	}

	dialog = gtk_dialog_new_with_buttons( _( "Rename Path" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pCameraInspectorWnd ) );
	gtk_window_set_position( GTK_WINDOW( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_widget_show( vbox );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_label_new( _( "Name:" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

	name = gtk_entry_new();
	gtk_box_pack_start( GTK_BOX( hbox ), name, FALSE, FALSE, 0 );
	gtk_widget_show( name );

	if ( g_iActiveTarget < 0 ) {
		gtk_entry_set_text( GTK_ENTRY( name ), GetCurrentCam()->GetCam()->getPositionObj()->getName() );
	}
	else{
		gtk_entry_set_text( GTK_ENTRY( name ), GetCurrentCam()->GetCam()->getActiveTarget( g_iActiveTarget )->getName() );
	}

	// -------------------------- //

	w = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 2 );
	gtk_widget_show( w );

	// -------------------------- //

	bool dialogError = TRUE;
	while ( dialogError ) 
	{
		response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

		dialogError = FALSE;

		if( response_id == GTK_RESPONSE_OK ) {

			const char *str = gtk_entry_get_text( GTK_ENTRY( name ) );

			if ( str && str[0] ) {
				// Update the path
				if ( g_iActiveTarget < 0 ) {
					GetCurrentCam()->GetCam()->getPositionObj()->setName( str );
				}
				else{
					GetCurrentCam()->GetCam()->getActiveTarget( g_iActiveTarget )->setName( str );
				}

				GetCurrentCam()->GetCam()->buildCamera();

				// Rebuild the listbox
				RefreshPathListCombo();
			}
			else {
				dialogError = TRUE;
			}
		}
	}

	gtk_widget_destroy( dialog );

	return TRUE;
}

static gint ci_add_target( GtkWidget *widget, gpointer data ){
	GtkWidget *dialog, *w, *vbox, *vbox2, *hbox, *frame, *name;
	GtkWidget *fixed, *interpolated, *spline, *content_area;
	GSList *targetTypeRadio = NULL;
	char buf[128];
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	if ( !GetCurrentCam() ) {
		return TRUE;
	}

	dialog = gtk_dialog_new_with_buttons( _( "Add Target" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pCameraInspectorWnd ) );
	gtk_window_set_position( GTK_WINDOW( dialog ),GTK_WIN_POS_CENTER );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );


	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_widget_show( vbox );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_label_new( _( "Name:" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	name = gtk_entry_new();
	gtk_box_pack_start( GTK_BOX( hbox ), name, TRUE, TRUE, 0 );
	gtk_widget_show( name );

	sprintf( buf, "target%i", GetCurrentCam()->GetCam()->numTargets() + 1 );
	gtk_entry_set_text( GTK_ENTRY( name ), buf );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	frame = gtk_frame_new( _( "Type" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), frame, TRUE, TRUE, 0 );
	gtk_widget_show( frame );

	vbox2 = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox2 );
	gtk_container_set_border_width( GTK_CONTAINER( vbox2 ), 5 );
	gtk_widget_show( vbox2 );

	// -------------------------- //

	fixed = gtk_radio_button_new_with_label( targetTypeRadio, _( "Fixed" ) );
	gtk_box_pack_start( GTK_BOX( vbox2 ), fixed, FALSE, FALSE, 3 );
	gtk_widget_show( fixed );
	targetTypeRadio = gtk_radio_button_get_group( GTK_RADIO_BUTTON( fixed ) );

	interpolated = gtk_radio_button_new_with_label( targetTypeRadio, _( "Interpolated" ) );
	gtk_box_pack_start( GTK_BOX( vbox2 ), interpolated, FALSE, FALSE, 3 );
	gtk_widget_show( interpolated );
	targetTypeRadio = gtk_radio_button_get_group( GTK_RADIO_BUTTON( interpolated ) );

	spline = gtk_radio_button_new_with_label( targetTypeRadio, _( "Spline" ) );
	gtk_box_pack_start( GTK_BOX( vbox2 ), spline, FALSE, FALSE, 3 );
	gtk_widget_show( spline );
	targetTypeRadio = gtk_radio_button_get_group( GTK_RADIO_BUTTON( spline ) );

	// -------------------------- //

	w = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 2 );
	gtk_widget_show( w );

	// -------------------------- //


	bool dialogError = TRUE;
	while ( dialogError ) 
	{
		response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

		dialogError = FALSE;

		if( response_id == GTK_RESPONSE_OK ) {

			const char *str = gtk_entry_get_text( GTK_ENTRY( name ) );

			if ( str && str[0] ) {
				int type;
				GList *li, *children;

				if ( gtk_toggle_button_get_active( (GtkToggleButton*)fixed ) ) {
					type = 0;
				}
				else if ( gtk_toggle_button_get_active( (GtkToggleButton*)interpolated ) ) {
					type = 1;
				} else {
					assert( gtk_toggle_button_get_active( (GtkToggleButton*)spline ) );
					type = 2;
				}

				// Add the target
				GetCurrentCam()->GetCam()->addTarget( str, static_cast<idCameraPosition::positionType>( type ) );

				// Rebuild the listbox
				RefreshPathListCombo();

				children = gtk_container_get_children( GTK_CONTAINER( g_pPathListCombo ) );
				// Select the last item in the listbox
				li = g_list_last( children );
				if ( li ) {
					gtk_combo_box_set_active( GTK_COMBO_BOX( g_pPathListCombo ), g_list_index( children, GTK_WIDGET( li->data ) ) );
				}
				if ( children ) {
					g_list_free( children );
				}

				// If this was the first one, refresh the event list
				if ( GetCurrentCam()->GetCam()->numTargets() == 1 ) {
					RefreshEventList();
				}

				// Go to editmode Add
				gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( g_pEditModeAddRadioButton ), TRUE );

			}
			else {
				dialogError = TRUE;
			}
		}
	}

	gtk_widget_destroy( dialog );

	return TRUE;
}

static GtkWidget *g_pCamListCombo = NULL;
static GtkLabel *g_pCamType = NULL;

void RefreshCamListCombo( void ){
	if ( !g_pCamListCombo ) {
		return;
	}

	GList *combo_list = (GList*)NULL;
	GList *lst;
	CCamera *combo_cam = firstCam;
	if ( combo_cam ) {
		while ( combo_cam ) {
			//combo_list = g_list_append( combo_list, (void *)combo_cam->GetCam()->getName() );
			//if( combo_cam->HasBeenSaved() ) {
			combo_list = g_list_append( combo_list, (void *)combo_cam->GetFileName() );
			/*} else {
			    char buf[128];
			    sprintf( buf, "Unsaved Camera %i", combo_cam->GetCamNum() );
			    combo_list = g_list_append( combo_list, (void *)buf );

			    //combo_list = g_list_append( combo_list, (void *)combo_cam->GetCam()->getName() );	// FIXME: this requires camera.dll to create unique names for new cams
			   }*/
			combo_cam = combo_cam->GetNext();
		}
	}
	for( lst = combo_list; lst != NULL; lst = g_list_next( lst ) )
	{
		gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT( g_pCamListCombo ), (const gchar *)lst->data );
	}
	g_list_free( combo_list );

	// select our current entry in the list
	if ( GetCurrentCam() ) {
		GList *li, *children;

		// stop editing on the current cam
		//GetCurrentCam()->GetCam()->stopEdit();	// FIXME: this crashed on creating new cameras, why is it here?

		li = children = gtk_container_get_children( GTK_CONTAINER( g_pCamListCombo ) );
		combo_cam = firstCam;
		while ( li && combo_cam ) {
			if ( combo_cam == GetCurrentCam() ) {
				gtk_combo_box_set_active( GTK_COMBO_BOX( g_pCamListCombo ), g_list_index( li, GTK_WIDGET( li->data ) ) );
				break;
			}
			li = li->next;
			combo_cam = combo_cam->GetNext();
		}
		if ( children ) {
			g_list_free( children );
		}
	}

	RefreshPathListCombo();
}

static gint ci_camlist_changed( GtkWidget *widget, gpointer data ){
	char *str = gtk_combo_box_text_get_active_text( GTK_COMBO_BOX_TEXT( widget ) );

	CCamera *combo_cam = firstCam;
	while ( str && combo_cam ) {
		//if( !strcmp( str, combo_cam->GetCam()->getName() ) )
		//if( combo_cam->HasBeenSaved() ) {
		if ( !strcmp( str, combo_cam->GetFileName() ) ) {
			break;
		}
		/*} else {
		    char buf[128];
		    sprintf( buf, "Unsaved Camera %i", combo_cam->GetCamNum() );
		    if( !strcmp( str, buf ) )
		    //if( !strcmp( str, combo_cam->GetCam()->getName() ) )
		        break;
		   }*/

		combo_cam = combo_cam->GetNext();
	}

	SetCurrentCam( combo_cam );

	if ( g_pCamType ) {
		if ( GetCurrentCam() ) {
			// Fill in our widgets fields for this camera
			char buf[128];

			// Set Name
			gtk_entry_set_text( GTK_ENTRY( g_pCamName ), GetCurrentCam()->GetCam()->getName() );

			// Set type
			gtk_label_set_text( g_pCamType, GetCurrentCam()->GetCam()->getPositionObj()->typeStr() );

			// Set duration
			sprintf( buf, "%.2f", GetCurrentCam()->GetCam()->getBaseTime() );
			gtk_entry_set_text( GTK_ENTRY( g_pSecondsEntry ), buf );

			sprintf( buf, "%.2f", GetCurrentCam()->GetCam()->getTotalTime() );
			gtk_label_set_text( g_pCurrentTime, "0.00" );
			gtk_label_set_text( g_pTotalTime, buf );

			gtk_adjustment_set_value( g_pTimeLine, 0.f );
			gtk_adjustment_set_upper( g_pTimeLine, GetCurrentCam()->GetCam()->getTotalTime() * 1000 );
		}
		else {
			// Set Name
			gtk_entry_set_text( GTK_ENTRY( g_pCamName ), "" );

			// Set type
			gtk_label_set_text( g_pCamType, "" );

			// Set duration
			gtk_entry_set_text( GTK_ENTRY( g_pSecondsEntry ), "30.00" );

			gtk_label_set_text( g_pCurrentTime, "0.00" );
			gtk_label_set_text( g_pTotalTime, "30.00" );

			gtk_adjustment_set_value( g_pTimeLine, 0.f );
			gtk_adjustment_set_upper( g_pTimeLine, 30000 );
		}

		// Refresh event list
		RefreshEventList();
	}

	RefreshPathListCombo();

	// start edit mode
	g_iActiveTarget = -1;
	if ( g_pCameraInspectorWnd && gtk_widget_get_visible( g_pCameraInspectorWnd ) ) {
		DoStartEdit( GetCurrentCam() );
	}

	g_free( str );

	return TRUE;
}

enum camEventType {
	EVENT_NA = 0x00,
	EVENT_WAIT,             //
	EVENT_TARGETWAIT,   //
	EVENT_SPEED,            //
	EVENT_TARGET,           // char(name)
	EVENT_SNAPTARGET,   //
	EVENT_FOV,              // int(time), int(targetfov)
	EVENT_CMD,              //
	EVENT_TRIGGER,      //
	EVENT_STOP,             //
	EVENT_CAMERA,           //
	EVENT_FADEOUT,      // int(time)
	EVENT_FADEIN,           // int(time)
	EVENT_FEATHER,      //
	EVENT_COUNT
};

// { requires parameters, enabled }
const bool camEventFlags[][2] = {
	{ false, false },
	{ false, true },
	{ false, false },
	{ false, false },
	{ true, true },
	{ false, false },
	{ true, true },
	{ false, false },
	{ false, false },
	{ false, true },
	{ true, true },
	{ true, true },
	{ true, true },
	{ false, true },
};

const char *camEventStr[] = {
	N_( "n/a" ),
	N_( "Wait" ),
	N_( "Target wait" ),
	N_( "Speed" ),
	N_( "Change Target <string:name>" ),
	N_( "Snap Target" ),
	N_( "FOV <int:duration> <int:targetfov>" ),
	N_( "Run Script" ),
	N_( "Trigger" ),
	N_( "Stop" ),
	N_( "Change to Camera <string:camera> (or <int:cameranum> <string:camera>" ),
	N_( "Fade Out <int:duration>" ),
	N_( "Fade In <int:duration>" ),
	N_( "Feather" )
};

static gint ci_add( GtkWidget *widget, gpointer data ){
	GtkWidget *dialog, *w, *vbox, *vbox2, *hbox, *frame, *parameters;
	GtkWidget *eventWidget[EVENT_COUNT], *content_area;
	int i;
	GSList *eventTypeRadio = NULL;
//	char buf[128];
	gint response_id;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	if ( !GetCurrentCam() ) {
		return TRUE;
	}

	dialog = gtk_dialog_new_with_buttons( _( "Add Event" ), NULL, flags, NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pCameraInspectorWnd ) );
	gtk_window_set_position( GTK_WINDOW( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );

	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "Cancel" ), GTK_RESPONSE_CANCEL );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	// fill the window
	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( content_area ), vbox );
	gtk_widget_show( vbox );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	frame = gtk_frame_new( _( "Type" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), frame, TRUE, TRUE, 0 );
	gtk_widget_show( frame );

	vbox2 = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox2 );
	gtk_container_set_border_width( GTK_CONTAINER( vbox2 ), 5 );
	gtk_widget_show( vbox2 );

	// -------------------------- //

	for ( i = 1; i < EVENT_COUNT; i++ ) {
		eventWidget[i] = gtk_radio_button_new_with_label( eventTypeRadio, camEventStr[i] );
		gtk_box_pack_start( GTK_BOX( vbox2 ), eventWidget[i], FALSE, FALSE, 3 );
		gtk_widget_show( eventWidget[i] );
		eventTypeRadio = gtk_radio_button_get_group( GTK_RADIO_BUTTON( eventWidget[i] ) );
		if ( camEventFlags[i][1] == false ) {
			gtk_widget_set_sensitive( eventWidget[i], FALSE );
		}
	}

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_label_new( _( "Parameters:" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

	parameters = gtk_entry_new();
	gtk_box_pack_start( GTK_BOX( hbox ), parameters, TRUE, TRUE, 0 );
	gtk_widget_show( parameters );

	// -------------------------- //

	w = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 2 );
	gtk_widget_show( w );

	// -------------------------- //


	bool dialogError = TRUE;
	while ( dialogError ) 
	{
		response_id = gtk_dialog_run( GTK_DIALOG( dialog ) );

		dialogError = FALSE;

		if( response_id == GTK_RESPONSE_OK ) {

			const char *str = gtk_entry_get_text( GTK_ENTRY( parameters ) );

			if ( !camEventFlags[i][0] || ( str && str[0] ) ) {
				int type = 0;
//				GList *li;

				for ( type = 1; type < EVENT_COUNT; type++ ) {
					if ( gtk_toggle_button_get_active( (GtkToggleButton*)eventWidget[type] ) ) {
						break;
					}
				}

				// Add the event
				GetCurrentCam()->GetCam()->addEvent( static_cast<idCameraEvent::eventType>( type ), str, (long)( gtk_adjustment_get_value( g_pTimeLine ) ) );

				// Refresh event list
				RefreshEventList();
			}
			else {
				dialogError = TRUE;
			}
		}
	}

	gtk_widget_destroy( dialog );

	return TRUE;
}

static gint ci_del( GtkWidget *widget, gpointer data ){
	GtkTreeIter iter;
	GtkTreeModel *model;
	guint *index;
	GtkTreeSelection *select;

	// TODO: add support to splines lib
	select = gtk_tree_view_get_selection( GTK_TREE_VIEW( g_pEventsList ) );
	if( gtk_tree_selection_get_selected( select, &model, &iter ) )
	{
		gtk_tree_model_get( model, &iter, EVENT_INDEX_COLUMN, &index, -1 );

		GetCurrentCam()->GetCam()->removeEvent( *index );
		// Refresh event list
		RefreshEventList();

		g_free( index );
	}
	return TRUE;
}

static gint ci_timeline_changed( GtkAdjustment *adjustment ){
	char buf[128];

	sprintf( buf, "%.2f", gtk_adjustment_get_value( adjustment ) / 1000.f );
	gtk_label_set_text( g_pCurrentTime, buf );

	// FIXME: this will never work completely perfect. Startcamera calls buildcamera, which sets all events to 'nottriggered'.
	// So if you have a wait at the end of the path, this will go to nontriggered immediately when you go over it and the camera
	// will have no idea where on the track it should be.
	if ( GetCurrentCam() && gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( g_pTrackCamera ) ) ) {
		float fov;
		vec3_t origin = { 0.0f, 0.0f, 0.0f }, dir = { 0.0f, 0.0f, 0.0f}, angles;

		GetCurrentCam()->GetCam()->startCamera( 0 );

		GetCurrentCam()->GetCam()->getCameraInfo( (long)( gtk_adjustment_get_value( adjustment ) ), &origin[0], &dir[0], &fov );
		VectorSet( angles, asin( dir[2] ) * 180 / 3.14159, atan2( dir[1], dir[0] ) * 180 / 3.14159, 0 );
		g_CameraTable.m_pfnSetCamera( origin, angles );
	}

	return TRUE;
}

GtkWidget *CreateCameraInspectorDialog( void ){
	GtkWidget *dialog, *w, *vbox, *hbox, *table, *frame;
	GtkWidget *content_area;
	GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_dialog_new_with_buttons( _( "Camera Inspector" ), NULL, flags, NULL );
	w = gtk_dialog_add_button( GTK_DIALOG( dialog ), _( "OK" ), GTK_RESPONSE_OK );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( ci_close ), NULL );

	g_signal_connect( dialog, "delete_event", G_CALLBACK( ci_close ), NULL );
	g_signal_connect( dialog, "show", G_CALLBACK( ci_show ), NULL );

	gtk_window_set_transient_for( GTK_WINDOW( dialog ), GTK_WINDOW( g_pCameraInspectorWnd ) );

	content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

	table = gtk_table_new( 3, 2, FALSE );
	gtk_container_add( GTK_CONTAINER( content_area ), table );
	gtk_container_set_border_width( GTK_CONTAINER( table ), 5 );
	gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
	gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );
	gtk_widget_show( table );

	// the properties column
	// -------------------------- //

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_table_attach( GTK_TABLE( table ), vbox, 0, 1, 0, 1,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
	gtk_widget_show( vbox );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_label_new( _( "File:" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	g_pCamListCombo = gtk_combo_box_text_new();
	gtk_box_pack_start( GTK_BOX( hbox ), g_pCamListCombo, TRUE, TRUE, 0 );
	gtk_widget_show( g_pCamListCombo );
	g_signal_connect( G_OBJECT( GTK_COMBO_BOX( g_pCamListCombo ) ), "changed", G_CALLBACK( ci_camlist_changed ), NULL );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_label_new( _( "Name:" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	g_pCamName = gtk_entry_new();
	gtk_box_pack_start( GTK_BOX( hbox ), g_pCamName, FALSE, FALSE, 0 );
	gtk_widget_show( g_pCamName );

	w = gtk_label_new( _( "Type:" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	w = gtk_label_new( "" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );
	g_pCamType = GTK_LABEL( w );

	RefreshCamListCombo();


	// -------------------------- //

	frame = gtk_frame_new( _( "Path and Target editing" ) );
	gtk_table_attach( GTK_TABLE( table ), frame, 0, 1, 1, 2,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
	gtk_widget_show( frame );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_label_new( _( "Edit:" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	g_pPathListCombo = gtk_combo_box_text_new();
	gtk_box_pack_start( GTK_BOX( hbox ), g_pPathListCombo, TRUE, TRUE, 0 );
	gtk_widget_show( g_pPathListCombo );
	g_signal_connect( G_OBJECT( GTK_COMBO_BOX( g_pPathListCombo ) ), "changed", G_CALLBACK( ci_pathlist_changed ), NULL );

	RefreshPathListCombo();


	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	g_pEditModeEditRadioButton = gtk_radio_button_new_with_label( g_pEditTypeRadio, _( "Edit Points" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), g_pEditModeEditRadioButton, FALSE, FALSE, 3 );
	gtk_widget_show( g_pEditModeEditRadioButton );
	g_pEditTypeRadio = gtk_radio_button_get_group( GTK_RADIO_BUTTON( g_pEditModeEditRadioButton ) );

	g_signal_connect( G_OBJECT( g_pEditModeEditRadioButton ), "clicked", G_CALLBACK( ci_editmode_edit ), NULL );

	g_pEditModeAddRadioButton = gtk_radio_button_new_with_label( g_pEditTypeRadio, _( "Add Points" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), g_pEditModeAddRadioButton, FALSE, FALSE, 3 );
	gtk_widget_show( g_pEditModeAddRadioButton );
	g_pEditTypeRadio = gtk_radio_button_get_group( GTK_RADIO_BUTTON( g_pEditModeAddRadioButton ) );

	g_signal_connect( G_OBJECT( g_pEditModeAddRadioButton ), "clicked", G_CALLBACK( ci_editmode_add ), NULL );

	// see if we should use a different default
	if ( g_iEditMode == 1 ) {
		// Go to editmode Add
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( g_pEditModeAddRadioButton ), TRUE );
	}

	w = gtk_label_new( _( "Type:" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	w = gtk_label_new( "" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );
	g_pPathType = GTK_LABEL( w );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_button_new_with_label( _( "Rename..." ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, TRUE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( ci_rename ), NULL );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( _( "Add Target..." ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, TRUE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( ci_add_target ), NULL );
	gtk_widget_show( w );

	// not available in splines library
	/*w = gtk_button_new_with_label( _( "Delete Selected" ) );
	   gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, TRUE, 0);
	   g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( ci_delete_selected ), NULL );
	   gtk_widget_show( w );

	   w = gtk_button_new_with_label( _( "Select All" ) );
	   gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, TRUE, 0);
	   g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( ci_select_all ), NULL );
	   gtk_widget_show( w );*/

	// -------------------------- //

	frame = gtk_frame_new( _( "Time" ) );
	gtk_table_attach( GTK_TABLE( table ), frame, 0, 1, 2, 3,
					  (GtkAttachOptions) ( GTK_EXPAND | GTK_FILL ),
					  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
	gtk_widget_show( frame );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox );
	gtk_container_set_border_width( GTK_CONTAINER( vbox ), 5 );
	gtk_widget_show( vbox );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_label_new( _( "Length (seconds):" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	g_pSecondsEntry = gtk_entry_new();
	gtk_box_pack_start( GTK_BOX( hbox ), g_pSecondsEntry, FALSE, FALSE, 0 );
	gtk_entry_set_alignment( GTK_ENTRY( g_pSecondsEntry ), 1.0 ); //right
	gtk_widget_show( g_pSecondsEntry );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_label_new( _( "Current Time:" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	w = gtk_label_new( _( "0.00" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );
	g_pCurrentTime = GTK_LABEL( w );

	w = gtk_label_new( _( " of " ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

	w = gtk_label_new( _( "0.00" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );
	g_pTotalTime = GTK_LABEL( w );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	g_pTimeLine = GTK_ADJUSTMENT( gtk_adjustment_new( 0, 0, 30000, 100, 250, 0 ) );
	g_signal_connect( G_OBJECT( g_pTimeLine ), "value-changed", G_CALLBACK( ci_timeline_changed ), NULL );
	w = gtk_hscale_new( g_pTimeLine );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	gtk_widget_show( w );
	gtk_scale_set_draw_value( GTK_SCALE( w ), FALSE );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	g_pTrackCamera = gtk_check_button_new_with_label( _( "Track Camera" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), g_pTrackCamera, FALSE, FALSE, 0 );
	gtk_widget_show( g_pTrackCamera );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_label_new( _( "Events:" ) );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_misc_set_alignment( GTK_MISC( w ), 0.0, 0.5 );
	gtk_widget_show( w );

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_scrolled_window_new( NULL, NULL );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( w ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
	gtk_widget_show( w );


	store = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_UINT ); //2 data columns

	g_pEventsList = gtk_tree_view_new_with_model( GTK_TREE_MODEL( store ) );
	g_object_unref( G_OBJECT( store ) );

	renderer = gtk_cell_renderer_text_new();
	//1 view column with the events
	column = gtk_tree_view_column_new_with_attributes( "events", renderer, "text", EVENT_TEXT_COLUMN, (char*)NULL );
	gtk_tree_view_append_column( GTK_TREE_VIEW( g_pEventsList ), column );

	gtk_tree_view_set_headers_visible( GTK_TREE_VIEW( g_pEventsList ), FALSE );

	gtk_container_add( GTK_CONTAINER( w ), g_pEventsList );
	//g_signal_connect( G_OBJECT(g_pEventsList), "select_row", G_CALLBACK (proplist_select_row), NULL);
	gtk_tree_selection_set_mode( gtk_tree_view_get_selection( GTK_TREE_VIEW( g_pEventsList ) ), GTK_SELECTION_BROWSE );
	gtk_widget_show( g_pEventsList );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox, FALSE, FALSE, 0 );
	gtk_widget_show( vbox );

	w = gtk_button_new_with_label( _( "Add..." ) );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( ci_add ), NULL );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( _( "Del" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( ci_del ), NULL );
	gtk_widget_show( w );

	// -------------------------- //
	// the buttons column
	// -------------------------- //

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_table_attach( GTK_TABLE( table ), vbox, 1, 2, 0, 1,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
	gtk_widget_show( vbox );

	w = gtk_button_new_with_label( _( "New..." ) );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( ci_new ), NULL );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( _( "Load..." ) );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( ci_load ), NULL );
	gtk_widget_show( w );

	// -------------------------- //

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_table_attach( GTK_TABLE( table ), vbox, 1, 2, 1, 2,
					  (GtkAttachOptions) ( GTK_FILL ),
					  (GtkAttachOptions) ( GTK_FILL ), 0, 0 );
	gtk_widget_show( vbox );

	w = gtk_button_new_with_label( _( "Save..." ) );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( ci_save ), NULL );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( _( "Unload" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( ci_unload ), NULL );
	gtk_widget_show( w );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, TRUE, TRUE, 0 );
	gtk_widget_show( hbox );

	w = gtk_button_new_with_label( _( "Apply" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( ci_apply ), NULL );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( _( "Preview" ) );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0 );
	g_signal_connect( G_OBJECT( w ), "clicked", G_CALLBACK( ci_preview ), NULL );
	gtk_widget_show( w );

	// -------------------------- //

	return dialog;
}
