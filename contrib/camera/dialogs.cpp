/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
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
static char *g_cNull = '\0';

static gint ci_editmode_edit( GtkWidget *widget, gpointer data )
{
	g_iEditMode = 0;

	return TRUE;
}

static gint ci_editmode_add( GtkWidget *widget, gpointer data )
{
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

static gint ci_new( GtkWidget *widget, gpointer data )
{
	GtkWidget *window, *w, *vbox, *vbox2, *hbox, *frame; //, *name;
	GtkWidget *fixed, *interpolated, *spline;
	EMessageBoxReturn ret;
  int loop = 1;
	GSList *targetTypeRadio = NULL;
//	char buf[128];

	// create the window
  window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW (window), "New Camera" );
  gtk_signal_connect( GTK_OBJECT (window), "delete_event", GTK_SIGNAL_FUNC( dialog_delete_callback ), NULL );
  gtk_signal_connect( GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC( gtk_widget_destroy ), NULL );
  gtk_window_set_transient_for( GTK_WINDOW( window ), GTK_WINDOW( g_pCameraInspectorWnd ) );

	g_object_set_data (G_OBJECT (window), "loop", &loop);
	g_object_set_data (G_OBJECT (window), "ret", &ret);

	gtk_widget_realize (window);

	// fill the window
	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show( vbox );

  // -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
  gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

  frame = gtk_frame_new( "Type" );
  gtk_box_pack_start( GTK_BOX( hbox ), frame, TRUE, TRUE, 0 );
  gtk_widget_show( frame );

	vbox2 = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox2 );
  gtk_container_set_border_width( GTK_CONTAINER (vbox2), 5 );
	gtk_widget_show( vbox2 );

	// -------------------------- //

	fixed = gtk_radio_button_new_with_label( targetTypeRadio, "Fixed" );
	gtk_box_pack_start( GTK_BOX( vbox2 ), fixed, FALSE, FALSE, 3 );
  gtk_widget_show( fixed );
  targetTypeRadio = gtk_radio_button_group( GTK_RADIO_BUTTON( fixed ) );

	interpolated = gtk_radio_button_new_with_label( targetTypeRadio, "Interpolated" );
	gtk_box_pack_start( GTK_BOX( vbox2 ), interpolated, FALSE, FALSE, 3 );
  gtk_widget_show( interpolated );
  targetTypeRadio = gtk_radio_button_group( GTK_RADIO_BUTTON( interpolated ) );

	spline = gtk_radio_button_new_with_label( targetTypeRadio, "Spline" );
	gtk_box_pack_start( GTK_BOX( vbox2 ), spline, FALSE, FALSE, 3 );
  gtk_widget_show( spline );
  targetTypeRadio = gtk_radio_button_group( GTK_RADIO_BUTTON( spline ) );

  // -------------------------- //

  w = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 2);
  gtk_widget_show (w);

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
  gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_button_new_with_label ("Ok");
	gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDOK));
	gtk_widget_show (w);

	GTK_WIDGET_SET_FLAGS( w, GTK_CAN_DEFAULT );
	gtk_widget_grab_default( w );

	w = gtk_button_new_with_label ("Cancel");
	gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDCANCEL));
	gtk_widget_show (w);
	ret = eIDCANCEL;

	// -------------------------- //

	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	gtk_grab_add (window);

	bool dialogError = TRUE;
	while (dialogError) {
		loop = 1;
		while (loop)
			gtk_main_iteration ();
 
		dialogError = FALSE;

		if( ret == eIDOK ) {
			if( gtk_toggle_button_get_active( (GtkToggleButton*)fixed ) )
				DoNewFixedCamera();
			else if( gtk_toggle_button_get_active( (GtkToggleButton*)interpolated ) )
				DoNewInterpolatedCamera();
			else if( gtk_toggle_button_get_active( (GtkToggleButton*)spline ) )
				DoNewSplineCamera();
		}
	}

	gtk_grab_remove (window);
	gtk_widget_destroy (window);

  return TRUE;
}

static gint ci_load( GtkWidget *widget, gpointer data )
{
  DoLoadCamera();

  return TRUE;
}

static gint ci_save( GtkWidget *widget, gpointer data )
{
  DoSaveCamera();

  return TRUE;
}

static gint ci_unload( GtkWidget *widget, gpointer data )
{
	DoUnloadCamera();

	return TRUE;
}

static gint ci_apply( GtkWidget *widget, gpointer data )
{
	if( GetCurrentCam() ) {
		const char *str;
		char buf[128];
		bool build = false;

		str = gtk_entry_get_text( GTK_ENTRY(g_pCamName) );

		if( str ) {
			GetCurrentCam()->GetCam()->setName( str );
			build = true;
		}

		str = gtk_entry_get_text( GTK_ENTRY(g_pSecondsEntry) );
		
		if( str ) {
			GetCurrentCam()->GetCam()->setBaseTime( atof( str ) );
			build = true;
		}

		if( build ) {
			GetCurrentCam()->GetCam()->buildCamera();
		}

		sprintf( buf, "%.2f", GetCurrentCam()->GetCam()->getBaseTime() );
		gtk_entry_set_text( GTK_ENTRY(g_pSecondsEntry), buf );

		sprintf( buf, "%.2f", GetCurrentCam()->GetCam()->getTotalTime() );
		gtk_label_set_text( g_pCurrentTime, "0.00" );
		gtk_label_set_text( g_pTotalTime, buf );

		gtk_adjustment_set_value( g_pTimeLine, 0.f );
		g_pTimeLine->upper = GetCurrentCam()->GetCam()->getTotalTime() * 1000;

		GetCurrentCam()->HasBeenModified();
	}

  return TRUE;
}

static gint ci_preview( GtkWidget *widget, gpointer data )
{
	if( GetCurrentCam() ) {
		g_iPreviewRunning = 1;
		g_FuncTable.m_pfnSysUpdateWindows( W_XY_OVERLAY | W_CAMERA );
	}

  return TRUE;
}

static gint ci_expose( GtkWidget *widget, gpointer data )
{
  // start edit mode
  DoStartEdit( GetCurrentCam() );

  return FALSE;
}

static gint ci_close( GtkWidget *widget, gpointer data )
{
  gtk_widget_hide( g_pCameraInspectorWnd );

  // exit edit mode
  DoStopEdit();

  return TRUE;
}

static GtkWidget *g_pPathListCombo = NULL;
static GtkLabel *g_pPathType = NULL;

static void RefreshPathListCombo( void )
{
  if( !g_pPathListCombo )
    return;

  GList *combo_list = (GList*)NULL;

  if( GetCurrentCam() ) {
    combo_list = g_list_append( combo_list, (void *)GetCurrentCam()->GetCam()->getPositionObj()->getName() );
    for( int i = 0; i < GetCurrentCam()->GetCam()->numTargets(); i++ ) {
      combo_list = g_list_append( combo_list, (void *)GetCurrentCam()->GetCam()->getActiveTarget( i )->getName() );
    }
  } else {
		// add one empty string make gtk be quiet
		combo_list = g_list_append( combo_list, (gpointer)g_cNull );
  }

  gtk_combo_set_popdown_strings( GTK_COMBO( g_pPathListCombo ), combo_list );
  g_list_free( combo_list );
}

static gint ci_pathlist_changed( GtkWidget *widget, gpointer data )
{
  const char *str = gtk_entry_get_text( GTK_ENTRY(widget) );

  if( !str || !GetCurrentCam() )
    return TRUE;

	int i;
  for( i = 0; i < GetCurrentCam()->GetCam()->numTargets(); i++ ) {
    if( !strcmp( str, GetCurrentCam()->GetCam()->getActiveTarget( i )->getName() ) )
      break;
  }

  if( i >= 0 && i < GetCurrentCam()->GetCam()->numTargets() ) {
    GetCurrentCam()->GetCam()->setActiveTarget( i );

    g_iActiveTarget = i;
		if( g_pPathType )
			gtk_label_set_text( g_pPathType, GetCurrentCam()->GetCam()->getActiveTarget(g_iActiveTarget)->typeStr() );
  } else {
    g_iActiveTarget = -1;
		if( g_pPathType )
			gtk_label_set_text( g_pPathType, GetCurrentCam()->GetCam()->getPositionObj()->typeStr() );
  }

  // start edit mode
  if( g_pCameraInspectorWnd && GTK_WIDGET_VISIBLE( g_pCameraInspectorWnd ) )
    DoStartEdit( GetCurrentCam() );

  return TRUE;
}

static void RefreshEventList( void )
{
	int i;
	char buf[128];

	// Clear events list
	gtk_clist_freeze( GTK_CLIST(g_pEventsList) );
	gtk_clist_clear( GTK_CLIST(g_pEventsList) );

	if( GetCurrentCam() ) {
		// Fill events list
		for( i = 0; i < GetCurrentCam()->GetCam()->numEvents(); i++ ) {
			char rowbuf[3][128], *row[3];
			// FIXME: sort by time?
			sprintf( rowbuf[0], "%li", GetCurrentCam()->GetCam()->getEvent(i)->getTime() );                 row[0] = rowbuf[0];
			strncpy( rowbuf[1], GetCurrentCam()->GetCam()->getEvent(i)->typeStr(), sizeof(rowbuf[0]) );     row[1] = rowbuf[1];
			strncpy( rowbuf[2], GetCurrentCam()->GetCam()->getEvent(i)->getParam(), sizeof(rowbuf[1]) );    row[2] = rowbuf[2];
			gtk_clist_append( GTK_CLIST(g_pEventsList), row );
		}

		// Total duration might have changed
		sprintf( buf, "%.2f", GetCurrentCam()->GetCam()->getTotalTime() );
		gtk_label_set_text( g_pCurrentTime, "0.00" );
		gtk_label_set_text( g_pTotalTime, buf );

		gtk_adjustment_set_value( g_pTimeLine, 0.f );
		g_pTimeLine->upper = ( GetCurrentCam()->GetCam()->getTotalTime() * 1000 );
	}

	gtk_clist_thaw( GTK_CLIST(g_pEventsList) );
}

static gint ci_rename( GtkWidget *widget, gpointer data )
{
	GtkWidget *window, *w, *vbox, *hbox, *name;
	EMessageBoxReturn ret;
  int loop = 1;

	if( !GetCurrentCam() )
		return TRUE;

	// create the window
  window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW (window), "Rename Path" );
  gtk_signal_connect( GTK_OBJECT (window), "delete_event", GTK_SIGNAL_FUNC( dialog_delete_callback ), NULL );
  gtk_signal_connect( GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC( gtk_widget_destroy ), NULL );
  gtk_window_set_transient_for( GTK_WINDOW( window ), GTK_WINDOW( g_pCameraInspectorWnd ) );

	g_object_set_data (G_OBJECT (window), "loop", &loop);
	g_object_set_data (G_OBJECT (window), "ret", &ret);

	gtk_widget_realize ( window );

	// fill the window
	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show( vbox );

  // -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
  gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

  w = gtk_label_new( "Name:" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

  name = gtk_entry_new();
  gtk_box_pack_start( GTK_BOX( hbox ), name, FALSE, FALSE, 0 );
  gtk_widget_show( name );

	if( g_iActiveTarget < 0 )
		gtk_entry_set_text( GTK_ENTRY(name), GetCurrentCam()->GetCam()->getPositionObj()->getName() );
	else
		gtk_entry_set_text( GTK_ENTRY(name), GetCurrentCam()->GetCam()->getActiveTarget(g_iActiveTarget)->getName() );

  // -------------------------- //

  w = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 2);
  gtk_widget_show (w);

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
  gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_button_new_with_label ("Ok");
	gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDOK));
	gtk_widget_show (w);

	GTK_WIDGET_SET_FLAGS( w, GTK_CAN_DEFAULT );
	gtk_widget_grab_default( w );

	w = gtk_button_new_with_label ("Cancel");
	gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDCANCEL));
	gtk_widget_show (w);
	ret = eIDCANCEL;

	// -------------------------- //

	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	gtk_grab_add (window);

	bool dialogError = TRUE;
	while (dialogError) {
		loop = 1;
		while (loop)
			gtk_main_iteration ();
 
		dialogError = FALSE;

		if( ret == eIDOK ) {
			const char *str = gtk_entry_get_text( GTK_ENTRY(name) );

			if( str && str[0] ) {
				// Update the path
				if( g_iActiveTarget < 0 )
					GetCurrentCam()->GetCam()->getPositionObj()->setName( str );
				else
					GetCurrentCam()->GetCam()->getActiveTarget(g_iActiveTarget)->setName( str );

				GetCurrentCam()->GetCam()->buildCamera();

				// Rebuild the listbox
				RefreshPathListCombo();
			} else {
				dialogError = TRUE;
			}
		}
	}

	gtk_grab_remove (window);
	gtk_widget_destroy (window);

  return TRUE;
}

static gint ci_add_target( GtkWidget *widget, gpointer data )
{
	GtkWidget *window, *w, *vbox, *vbox2, *hbox, *frame, *name;
	GtkWidget *fixed, *interpolated, *spline;
	EMessageBoxReturn ret;
  int loop = 1;
	GSList *targetTypeRadio = NULL;
	char buf[128];

	if( !GetCurrentCam() )
		return TRUE;

	// create the window
  window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW (window), "Add Target" );
  gtk_signal_connect( GTK_OBJECT (window), "delete_event", GTK_SIGNAL_FUNC( dialog_delete_callback ), NULL );
  gtk_signal_connect( GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC( gtk_widget_destroy ), NULL );
  gtk_window_set_transient_for( GTK_WINDOW( window ), GTK_WINDOW( g_pCameraInspectorWnd ) );

	g_object_set_data (G_OBJECT (window), "loop", &loop);
	g_object_set_data (G_OBJECT (window), "ret", &ret);

	gtk_widget_realize (window);

	// fill the window
	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show( vbox );

  // -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
  gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

  w = gtk_label_new( "Name:" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

  name = gtk_entry_new();
  gtk_box_pack_start( GTK_BOX( hbox ), name, TRUE, TRUE, 0 );
  gtk_widget_show( name );

	sprintf( buf, "target%i", GetCurrentCam()->GetCam()->numTargets() + 1 );
	gtk_entry_set_text( GTK_ENTRY(name), buf );

  // -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
  gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

  frame = gtk_frame_new( "Type" );
  gtk_box_pack_start( GTK_BOX( hbox ), frame, TRUE, TRUE, 0 );
  gtk_widget_show( frame );

	vbox2 = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox2 );
  gtk_container_set_border_width( GTK_CONTAINER (vbox2), 5 );
	gtk_widget_show( vbox2 );

	// -------------------------- //

	fixed = gtk_radio_button_new_with_label( targetTypeRadio, "Fixed" );
	gtk_box_pack_start( GTK_BOX( vbox2 ), fixed, FALSE, FALSE, 3 );
  gtk_widget_show( fixed );
  targetTypeRadio = gtk_radio_button_group( GTK_RADIO_BUTTON( fixed ) );

	interpolated = gtk_radio_button_new_with_label( targetTypeRadio, "Interpolated" );
	gtk_box_pack_start( GTK_BOX( vbox2 ), interpolated, FALSE, FALSE, 3 );
  gtk_widget_show( interpolated );
  targetTypeRadio = gtk_radio_button_group( GTK_RADIO_BUTTON( interpolated ) );

	spline = gtk_radio_button_new_with_label( targetTypeRadio, "Spline" );
	gtk_box_pack_start( GTK_BOX( vbox2 ), spline, FALSE, FALSE, 3 );
  gtk_widget_show( spline );
  targetTypeRadio = gtk_radio_button_group( GTK_RADIO_BUTTON( spline ) );

  // -------------------------- //

  w = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 2);
  gtk_widget_show (w);

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
  gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_button_new_with_label ("Ok");
	gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDOK));
	gtk_widget_show (w);

	GTK_WIDGET_SET_FLAGS( w, GTK_CAN_DEFAULT );
	gtk_widget_grab_default( w );

	w = gtk_button_new_with_label ("Cancel");
	gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDCANCEL));
	gtk_widget_show (w);
	ret = eIDCANCEL;

	// -------------------------- //

	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	gtk_grab_add (window);

	bool dialogError = TRUE;
	while (dialogError) {
		loop = 1;
		while (loop)
			gtk_main_iteration ();
 
		dialogError = FALSE;

		if( ret == eIDOK ) {
			const char *str = gtk_entry_get_text( GTK_ENTRY(name) );

			if( str && str[0] ) {
				int type;
				GList *li;

				if( gtk_toggle_button_get_active( (GtkToggleButton*)fixed ) )
					type = 0;
				else if( gtk_toggle_button_get_active( (GtkToggleButton*)interpolated ) )
					type = 1;
				else if( gtk_toggle_button_get_active( (GtkToggleButton*)spline ) )
					type = 2;

				// Add the target
				GetCurrentCam()->GetCam()->addTarget( str, static_cast<idCameraPosition::positionType>(type) );

				// Rebuild the listbox
				RefreshPathListCombo();

				// Select the last item in the listbox
				li = g_list_last( GTK_LIST(GTK_COMBO(g_pPathListCombo)->list)->children );
				gtk_list_select_child( GTK_LIST(GTK_COMBO(g_pPathListCombo)->list), GTK_WIDGET (li->data) );

				// If this was the first one, refresh the event list
				if( GetCurrentCam()->GetCam()->numTargets() == 1 ) {
					RefreshEventList();
				}

				// Go to editmode Add
				gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_pEditModeAddRadioButton), TRUE );

			} else {
				dialogError = TRUE;
			}
		}
	}

	gtk_grab_remove (window);
	gtk_widget_destroy (window);

  return TRUE;
}

static GtkWidget *g_pCamListCombo = NULL;
static GtkLabel *g_pCamType = NULL;

void RefreshCamListCombo( void )
{
  if( !g_pCamListCombo )
    return;

  GList *combo_list = (GList*)NULL;
  CCamera *combo_cam = firstCam;
	if( combo_cam ) {
		while( combo_cam ) {
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
	}else {
		// add one empty string make gtk be quiet
		combo_list = g_list_append( combo_list, (gpointer)g_cNull );
	}
  gtk_combo_set_popdown_strings( GTK_COMBO( g_pCamListCombo ), combo_list );
  g_list_free( combo_list );

  // select our current entry in the list
  if( GetCurrentCam() ) {
    // stop editing on the current cam
    //GetCurrentCam()->GetCam()->stopEdit();	// FIXME: this crashed on creating new cameras, why is it here?

    GList *li = GTK_LIST( GTK_COMBO(g_pCamListCombo)->list)->children;
    combo_cam = firstCam;
    while( li && combo_cam ) {
      if( combo_cam == GetCurrentCam() ) {
        gtk_list_select_child( GTK_LIST( GTK_COMBO(g_pCamListCombo)->list ), GTK_WIDGET( li->data ) );
        break;
      }
      li = li->next;
      combo_cam = combo_cam->GetNext();
    }
  }

  RefreshPathListCombo();
}

static gint ci_camlist_changed( GtkWidget *widget, gpointer data )
{
  const char *str = gtk_entry_get_text( GTK_ENTRY(widget) );

  CCamera *combo_cam = firstCam;
  while( str && combo_cam ) {
    //if( !strcmp( str, combo_cam->GetCam()->getName() ) )
		//if( combo_cam->HasBeenSaved() ) {
			if( !strcmp( str, combo_cam->GetFileName() ) )
				break;
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

	if( g_pCamType ) {
		if( GetCurrentCam() ) {
			// Fill in our widgets fields for this camera
			char buf[128];

			// Set Name
			gtk_entry_set_text( GTK_ENTRY(g_pCamName), GetCurrentCam()->GetCam()->getName() );

			// Set type
			gtk_label_set_text( g_pCamType, GetCurrentCam()->GetCam()->getPositionObj()->typeStr() );

			// Set duration
			sprintf( buf, "%.2f", GetCurrentCam()->GetCam()->getBaseTime() );
			gtk_entry_set_text( GTK_ENTRY(g_pSecondsEntry), buf );

			sprintf( buf, "%.2f", GetCurrentCam()->GetCam()->getTotalTime() );
			gtk_label_set_text( g_pCurrentTime, "0.00" );
			gtk_label_set_text( g_pTotalTime, buf );

			gtk_adjustment_set_value( g_pTimeLine, 0.f );
			g_pTimeLine->upper = GetCurrentCam()->GetCam()->getTotalTime() * 1000;
		} else {
			// Set Name
			gtk_entry_set_text( GTK_ENTRY(g_pCamName), "" );

			// Set type
			gtk_label_set_text( g_pCamType, "" );

			// Set duration
			gtk_entry_set_text( GTK_ENTRY(g_pSecondsEntry), "30.00" );

			gtk_label_set_text( g_pCurrentTime, "0.00" );
			gtk_label_set_text( g_pTotalTime, "30.00" );

			gtk_adjustment_set_value( g_pTimeLine, 0.f );
			g_pTimeLine->upper = 30000;
		}

		// Refresh event list
		RefreshEventList();
	}

  RefreshPathListCombo();

  // start edit mode
  g_iActiveTarget = -1;
  if( g_pCameraInspectorWnd && GTK_WIDGET_VISIBLE( g_pCameraInspectorWnd ) )
    DoStartEdit( GetCurrentCam() );

  return TRUE;
}

enum camEventType {
	EVENT_NA = 0x00,
	EVENT_WAIT,				// 
	EVENT_TARGETWAIT,	// 
	EVENT_SPEED,			// 
	EVENT_TARGET,			// char(name)
	EVENT_SNAPTARGET,	// 
	EVENT_FOV,				// int(time), int(targetfov)
	EVENT_CMD,				// 
	EVENT_TRIGGER,		// 
	EVENT_STOP,				// 
	EVENT_CAMERA,			// 
	EVENT_FADEOUT,		// int(time)
	EVENT_FADEIN,			// int(time)
	EVENT_FEATHER,		// 
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
	"n/a",
	"Wait",
	"Target wait",
	"Speed",
	"Change Target <string:name>",
	"Snap Target",
	"FOV <int:duration> <int:targetfov>",
	"Run Script",
	"Trigger",
	"Stop",
  "Change to Camera <string:camera> (or <int:cameranum> <string:camera>",
	"Fade Out <int:duration>",
	"Fade In <int:duration>",
	"Feather"
};

static gint ci_add( GtkWidget *widget, gpointer data )
{
	GtkWidget *window, *w, *vbox, *vbox2, *hbox, *frame, *parameters;
	GtkWidget *eventWidget[EVENT_COUNT];
	EMessageBoxReturn ret;
  int i, loop = 1;
	GSList *eventTypeRadio = NULL;
//	char buf[128];

	if( !GetCurrentCam() )
		return TRUE;

	// create the window
  window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW (window), "Add Event" );
  gtk_signal_connect( GTK_OBJECT (window), "delete_event", GTK_SIGNAL_FUNC( dialog_delete_callback ), NULL );
  gtk_signal_connect( GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC( gtk_widget_destroy ), NULL );
  gtk_window_set_transient_for( GTK_WINDOW( window ), GTK_WINDOW( g_pCameraInspectorWnd ) );

	g_object_set_data (G_OBJECT (window), "loop", &loop);
	g_object_set_data (G_OBJECT (window), "ret", &ret);

	gtk_widget_realize (window);

	// fill the window
	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_widget_show( vbox );

  // -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
  gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

  frame = gtk_frame_new( "Type" );
  gtk_box_pack_start( GTK_BOX( hbox ), frame, TRUE, TRUE, 0 );
  gtk_widget_show( frame );

	vbox2 = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox2 );
  gtk_container_set_border_width( GTK_CONTAINER (vbox2), 5 );
	gtk_widget_show( vbox2 );

	// -------------------------- //

	for( i = 1; i < EVENT_COUNT; i++ ) {
		eventWidget[i] = gtk_radio_button_new_with_label( eventTypeRadio, camEventStr[i] );
		gtk_box_pack_start( GTK_BOX( vbox2 ), eventWidget[i], FALSE, FALSE, 3 );
		gtk_widget_show( eventWidget[i] );
		eventTypeRadio = gtk_radio_button_group( GTK_RADIO_BUTTON( eventWidget[i] ) );
    if( camEventFlags[i][1] == false )
      gtk_widget_set_sensitive (eventWidget[i], FALSE);
	}

  // -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
  gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

  w = gtk_label_new( "Parameters:" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

  parameters = gtk_entry_new();
  gtk_box_pack_start( GTK_BOX( hbox ), parameters, TRUE, TRUE, 0 );
  gtk_widget_show( parameters );

  // -------------------------- //

  w = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, FALSE, 2);
  gtk_widget_show (w);

	// -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
  gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_button_new_with_label ("Ok");
	gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDOK));
	gtk_widget_show (w);

	GTK_WIDGET_SET_FLAGS( w, GTK_CAN_DEFAULT );
	gtk_widget_grab_default( w );

	w = gtk_button_new_with_label ("Cancel");
	gtk_box_pack_start (GTK_BOX (hbox), w, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (w), "clicked", GTK_SIGNAL_FUNC (dialog_button_callback), GINT_TO_POINTER (eIDCANCEL));
	gtk_widget_show (w);
	ret = eIDCANCEL;

	// -------------------------- //

	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_show (window);
	gtk_grab_add (window);

	bool dialogError = TRUE;
	while (dialogError) {
		loop = 1;
		while (loop)
			gtk_main_iteration ();
 
		dialogError = FALSE;

		if( ret == eIDOK ) {
			const char *str = gtk_entry_get_text( GTK_ENTRY(parameters) );

			if( !camEventFlags[i][0] || ( str && str[0] ) ) {
				int type = 0;
//				GList *li;

				for( type = 1; type < EVENT_COUNT; type++ ) {
					if( gtk_toggle_button_get_active( (GtkToggleButton*)eventWidget[type] ) )
						break;
				}

				// Add the event
				GetCurrentCam()->GetCam()->addEvent( static_cast<idCameraEvent::eventType>(type), str, (long)(g_pTimeLine->value) );

				// Refresh event list
				RefreshEventList();
			} else {
				dialogError = TRUE;
			}
		}
	}

	gtk_grab_remove (window);
	gtk_widget_destroy (window);

  return TRUE;
}

static gint ci_del( GtkWidget *widget, gpointer data )
{
	// TODO: add support to splines lib
	if(	GetCurrentCam() && GTK_CLIST(g_pEventsList)->focus_row >= 0 ) {
		GetCurrentCam()->GetCam()->removeEvent( GTK_CLIST(g_pEventsList)->focus_row );
		// Refresh event list
		RefreshEventList();
	}

  return TRUE;
}

static gint ci_timeline_changed( GtkAdjustment *adjustment )
{
	char buf[128];

	sprintf( buf, "%.2f", adjustment->value / 1000.f );
	gtk_label_set_text( g_pCurrentTime, buf );

	// FIXME: this will never work completely perfect. Startcamera calls buildcamera, which sets all events to 'nottriggered'.
	// So if you have a wait at the end of the path, this will go to nontriggered immediately when you go over it and the camera
	// will have no idea where on the track it should be.
	if( GetCurrentCam() && gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(g_pTrackCamera) ) ) {
		float fov;
    vec3_t origin = { 0.0f, 0.0f, 0.0f }, dir = { 0.0f, 0.0f, 0.0f}, angles;

    GetCurrentCam()->GetCam()->startCamera( 0 );

		GetCurrentCam()->GetCam()->getCameraInfo( (long)(adjustment->value), &origin[0], &dir[0], &fov );
		VectorSet( angles, asin (dir[2])*180/3.14159, atan2 (dir[1], dir[0])*180/3.14159, 0 );
		g_CameraTable.m_pfnSetCamera( origin, angles );	
	}	

	return TRUE;
}

GtkWidget *CreateCameraInspectorDialog( void )
{
  GtkWidget	*window, *w, *vbox, *hbox, *table, *frame;

  // create the window
  window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW (window), "Camera Inspector" );
  gtk_signal_connect( GTK_OBJECT (window), "delete_event", GTK_SIGNAL_FUNC( ci_close ), NULL );
  gtk_signal_connect( GTK_OBJECT (window), "expose_event", GTK_SIGNAL_FUNC( ci_expose ), NULL );
	//  gtk_signal_connect( GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC( gtk_widget_destroy ), NULL );
  gtk_window_set_transient_for( GTK_WINDOW( window ), GTK_WINDOW( g_pRadiantWnd ) );

  // don't use show, as you don't want to have it displayed on startup ;-)
  gtk_widget_realize( window );

  // fill the window

  // the table
  // -------------------------- //

  table = gtk_table_new( 3, 2, FALSE );
  gtk_widget_show( table );
  gtk_container_add( GTK_CONTAINER( window ), table );
  gtk_container_set_border_width( GTK_CONTAINER( table ), 5 );
  gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
  gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );

  // the properties column
	// -------------------------- //

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_widget_show( vbox );
  gtk_table_attach( GTK_TABLE( table ), vbox, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0 );

  // -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
  gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

  w = gtk_label_new( "File:" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

  g_pCamListCombo = gtk_combo_new();
  gtk_box_pack_start (GTK_BOX( hbox ), g_pCamListCombo, TRUE, TRUE, 0);
  gtk_widget_show( g_pCamListCombo );

  // -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
  gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_label_new( "Name:" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

  g_pCamName = gtk_entry_new();
  gtk_box_pack_start( GTK_BOX( hbox ), g_pCamName, FALSE, FALSE, 0 );
  gtk_widget_show( g_pCamName );	

  w = gtk_label_new( "Type: " );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

  w = gtk_label_new( "" );
  gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
  gtk_widget_show( w );
	g_pCamType = GTK_LABEL( w );

  RefreshCamListCombo();

  gtk_entry_set_editable( GTK_ENTRY( GTK_COMBO(g_pCamListCombo)->entry ), FALSE );
  gtk_signal_connect( GTK_OBJECT(GTK_COMBO(g_pCamListCombo)->entry), "changed", GTK_SIGNAL_FUNC( ci_camlist_changed ), NULL );

  // -------------------------- //

  frame = gtk_frame_new( "Path and Target editing" );
  gtk_widget_show( frame );
  gtk_table_attach( GTK_TABLE( table ), frame, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0 );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox );
  gtk_container_set_border_width( GTK_CONTAINER (vbox), 5 );
	gtk_widget_show( vbox );

  // -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

  w = gtk_label_new( "Edit:" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

  g_pPathListCombo = gtk_combo_new();
  gtk_box_pack_start (GTK_BOX( hbox ), g_pPathListCombo, TRUE, TRUE, 0);
  gtk_widget_show( g_pPathListCombo );

  RefreshPathListCombo();

	gtk_entry_set_editable( GTK_ENTRY( GTK_COMBO(g_pPathListCombo)->entry ), FALSE );
  gtk_signal_connect( GTK_OBJECT(GTK_COMBO(g_pPathListCombo)->entry), "changed", GTK_SIGNAL_FUNC( ci_pathlist_changed ), NULL );

  // -------------------------- //

  hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	g_pEditModeEditRadioButton = gtk_radio_button_new_with_label( g_pEditTypeRadio, "Edit Points" );
	gtk_box_pack_start( GTK_BOX( hbox ), g_pEditModeEditRadioButton, FALSE, FALSE, 3 );
  gtk_widget_show( g_pEditModeEditRadioButton );
  g_pEditTypeRadio = gtk_radio_button_group( GTK_RADIO_BUTTON( g_pEditModeEditRadioButton ) );

	gtk_signal_connect( GTK_OBJECT( g_pEditModeEditRadioButton ), "clicked", GTK_SIGNAL_FUNC( ci_editmode_edit ), NULL );

	g_pEditModeAddRadioButton = gtk_radio_button_new_with_label( g_pEditTypeRadio, "Add Points" );
	gtk_box_pack_start( GTK_BOX( hbox ), g_pEditModeAddRadioButton, FALSE, FALSE, 3 );
  gtk_widget_show( g_pEditModeAddRadioButton );
  g_pEditTypeRadio = gtk_radio_button_group( GTK_RADIO_BUTTON( g_pEditModeAddRadioButton ) );

	gtk_signal_connect( GTK_OBJECT( g_pEditModeAddRadioButton ), "clicked", GTK_SIGNAL_FUNC( ci_editmode_add ), NULL );

	// see if we should use a different default
	if( g_iEditMode == 1 ) {
		// Go to editmode Add
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(g_pEditModeAddRadioButton), TRUE );
	}

  w = gtk_label_new( "Type: " );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

  w = gtk_label_new( "" );
  gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
  gtk_widget_show( w );
	g_pPathType = GTK_LABEL( w );

  // -------------------------- //

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	w = gtk_button_new_with_label( "Rename..." );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, TRUE, 0);
	gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( ci_rename ), NULL );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( "Add Target..." );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, TRUE, 0);
	gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( ci_add_target ), NULL );
	gtk_widget_show( w );

	// not available in splines library
	/*w = gtk_button_new_with_label( "Delete Selected" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, TRUE, 0);
	gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( ci_delete_selected ), NULL );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( "Select All" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, TRUE, 0);
	gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( ci_select_all ), NULL );
	gtk_widget_show( w );*/

  // -------------------------- //

  frame = gtk_frame_new( "Time" );
  gtk_widget_show( frame );
  gtk_table_attach( GTK_TABLE( table ), frame, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0 );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( frame ), vbox );
  gtk_container_set_border_width( GTK_CONTAINER (vbox), 5 );
	gtk_widget_show( vbox );

  // -------------------------- //

  hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

  w = gtk_label_new( "Length (seconds):" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

  g_pSecondsEntry = gtk_entry_new();
  gtk_box_pack_start( GTK_BOX( hbox ), g_pSecondsEntry, FALSE, FALSE, 0 );
  gtk_widget_show( g_pSecondsEntry );

  // -------------------------- //

  hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

  w = gtk_label_new( "Current Time: " );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

  w = gtk_label_new( "0.00" );
  gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
  gtk_widget_show( w );
	g_pCurrentTime = GTK_LABEL( w );

  w = gtk_label_new( " of " );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

  w = gtk_label_new( "0.00" );
  gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
  gtk_widget_show( w );
	g_pTotalTime = GTK_LABEL( w );

  // -------------------------- //

  hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

	g_pTimeLine = GTK_ADJUSTMENT( gtk_adjustment_new( 0, 0, 30000, 100, 250, 0 ) );
	gtk_signal_connect( GTK_OBJECT(g_pTimeLine), "value_changed", GTK_SIGNAL_FUNC( ci_timeline_changed ), NULL );
  w = gtk_hscale_new( g_pTimeLine );
  gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
  gtk_widget_show( w );
  gtk_scale_set_draw_value( GTK_SCALE( w ), FALSE );

  // -------------------------- //

  hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

  g_pTrackCamera = gtk_check_button_new_with_label( "Track Camera" );
  gtk_box_pack_start( GTK_BOX( hbox ), g_pTrackCamera, FALSE, FALSE, 0 );
  gtk_widget_show( g_pTrackCamera );

  // -------------------------- //

  hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

  w = gtk_label_new( "Events:" );
	gtk_box_pack_start( GTK_BOX( hbox ), w, FALSE, FALSE, 0 );
	gtk_widget_show( w );

  // -------------------------- //

  hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );
	gtk_widget_show( hbox );

  w = gtk_scrolled_window_new( NULL, NULL );
  gtk_widget_set_usize( w, 0, 150 );
  gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( w ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start( GTK_BOX( hbox ), w, TRUE, TRUE, 0 );
  gtk_widget_show( w );

  g_pEventsList = gtk_clist_new( 3 );
  gtk_container_add( GTK_CONTAINER(w), g_pEventsList);
  //gtk_signal_connect( GTK_OBJECT(g_pEventsList), "select_row", GTK_SIGNAL_FUNC (proplist_select_row), NULL);
  gtk_clist_set_selection_mode( GTK_CLIST(g_pEventsList), GTK_SELECTION_BROWSE );
  gtk_clist_column_titles_hide( GTK_CLIST(g_pEventsList) );
  gtk_clist_set_column_auto_resize( GTK_CLIST(g_pEventsList), 0, TRUE );
  gtk_clist_set_column_auto_resize( GTK_CLIST(g_pEventsList), 1, TRUE );
	gtk_clist_set_column_auto_resize( GTK_CLIST(g_pEventsList), 2, TRUE );
	gtk_widget_show( g_pEventsList );

  vbox = gtk_vbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox, FALSE, FALSE, 0 );
	gtk_widget_show( vbox );

	w = gtk_button_new_with_label( "Add..." );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0);
	gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( ci_add ), NULL );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( "Del" );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0);
	gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( ci_del ), NULL );
	gtk_widget_show( w );

  // -------------------------- //

  /*/
   |
   |
   |
  */

  // the buttons column
  // -------------------------- //

  vbox = gtk_vbox_new( FALSE, 5 );
	gtk_widget_show( vbox );
  gtk_table_attach( GTK_TABLE( table ), vbox, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0 );

	w = gtk_button_new_with_label( "New..." );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0);
	gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( ci_new ), NULL );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( "Load..." );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0);
	gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( ci_load ), NULL );
	gtk_widget_show( w );

  // -------------------------- //

  vbox = gtk_vbox_new( FALSE, 5 );
	gtk_widget_show( vbox );
  gtk_table_attach( GTK_TABLE( table ), vbox, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0 );

	w = gtk_button_new_with_label( "Save..." );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0);
	gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( ci_save ), NULL );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( "Unload" );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0);
	gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( ci_unload ), NULL );
	gtk_widget_show( w );
  
  hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, TRUE, TRUE, 0 );
	gtk_widget_show( hbox );

  w = gtk_button_new_with_label( "Apply" );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0);
	gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( ci_apply ), NULL );
	gtk_widget_show( w );

	w = gtk_button_new_with_label( "Preview" );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0);
	gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( ci_preview ), NULL );
	gtk_widget_show( w );

  // -------------------------- //

 	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_widget_show( vbox );
  gtk_table_attach( GTK_TABLE( table ), vbox, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0 );

	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, TRUE, TRUE, 0 );
	gtk_widget_show( hbox );

	w = gtk_button_new_with_label( "Close" );
	gtk_box_pack_start( GTK_BOX( vbox ), w, FALSE, FALSE, 0);
	gtk_signal_connect( GTK_OBJECT( w ), "clicked", GTK_SIGNAL_FUNC( ci_close ), NULL );
	GTK_WIDGET_SET_FLAGS( w, GTK_CAN_DEFAULT );
	gtk_widget_grab_default( w );
	gtk_widget_show( w );

  // -------------------------- //

  return window;
}
