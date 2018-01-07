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

extern GtkWidget *g_pEditModeAddRadioButton;

char* Q_realpath( const char *path, char *resolved_path, size_t size ){
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
	return realpath( path, resolved_path );
#else
	return _fullpath( resolved_path, path, size );
#endif
}

static void DoNewCamera( idCameraPosition::positionType type ){
	CCamera *cam = AllocCam();

	if ( cam ) {
		char buf[128];
		sprintf( buf, "camera%i", cam->GetCamNum() );

		cam->GetCam()->startNewCamera( type );
		cam->GetCam()->setName( buf );
		cam->GetCam()->buildCamera();

		sprintf( buf, "Unsaved Camera %i", cam->GetCamNum() );
		cam->SetFileName( buf, false );

		SetCurrentCam( cam );
		RefreshCamListCombo();

		// Go to editmode Add
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( g_pEditModeAddRadioButton ), TRUE );

		// Show the camera inspector
		DoCameraInspector();

		// Start edit mode (if not initiated by DoCameraInspector)
		if ( !g_bEditOn ) {
			DoStartEdit( GetCurrentCam() );
		}
	}
	else {
		g_FuncTable.m_pfnMessageBox( (GtkWidget *)g_pRadiantWnd, "No free cameras available.", "Create Camera Error", MB_OK, NULL );
	}
}

void DoNewFixedCamera(){
	DoNewCamera( idCameraPosition::FIXED );
}

void DoNewInterpolatedCamera(){
	DoNewCamera( idCameraPosition::INTERPOLATED );
}

void DoNewSplineCamera(){
	DoNewCamera( idCameraPosition::SPLINE );
}

void DoCameraInspector(){
	gtk_widget_show( g_pCameraInspectorWnd );
}

void DoPreviewCamera(){
	if ( GetCurrentCam() ) {
		g_iPreviewRunning = 1;
		g_FuncTable.m_pfnSysUpdateWindows( W_XY_OVERLAY | W_CAMERA );
	}
}

void DoLoadCamera(){
	char basepath[PATH_MAX];

	if ( firstCam && firstCam->HasBeenSaved() ) {
		CAMERA_ExtractFilePath( firstCam->GetFileName(), basepath );
	}
	else{
		strcpy( basepath, g_FuncTable.m_pfnGetGamePath() );
	}

	const gchar *filename = g_FuncTable.m_pfnFileDialog( (GtkWidget *)g_pRadiantWnd, TRUE, "Open Camera File", basepath, "camera", NULL );

	if ( filename ) {
		CCamera *cam = AllocCam();
		char fullpathtofile[PATH_MAX];

		if ( cam ) {
			Q_realpath( filename, fullpathtofile, PATH_MAX );

			// see if this camera file was already loaded
			CCamera *checkCam = firstCam->GetNext(); // not the first one as we just allocated it
			while ( checkCam ) {
				if ( !strcmp( fullpathtofile, checkCam->GetFileName() ) ) {
					char error[PATH_MAX + 64];
					FreeCam( cam );
					sprintf( error, "Camera file \'%s\' is already loaded", fullpathtofile );
					g_FuncTable.m_pfnMessageBox( (GtkWidget *)g_pRadiantWnd, error, "Load error", MB_OK, NULL );
					//g_free( filename );
					return;
				}
				checkCam = checkCam->GetNext();
			}

			if ( loadCamera( cam->GetCamNum(), fullpathtofile ) ) {
				cam->GetCam()->buildCamera();
				cam->SetFileName( filename, true );
				SetCurrentCam( cam );
				RefreshCamListCombo();
				g_FuncTable.m_pfnSysUpdateWindows( W_XY_OVERLAY | W_CAMERA );
			}
			else {
				char error[PATH_MAX + 64];
				FreeCam( cam );
				sprintf( error, "An error occured during the loading of \'%s\'", fullpathtofile );
				g_FuncTable.m_pfnMessageBox( (GtkWidget *)g_pRadiantWnd, error, "Load error", MB_OK, NULL );
			}

			//g_free( filename );
		}
		else {
			g_FuncTable.m_pfnMessageBox( (GtkWidget *)g_pRadiantWnd, "No free camera slots available", "Load error", MB_OK, NULL );
		}
	}
}

void DoSaveCamera() {
	char basepath[PATH_MAX];

	if ( !GetCurrentCam() ) {
		return;
	}

	if ( GetCurrentCam()->GetFileName()[0] ) {
		CAMERA_ExtractFilePath( GetCurrentCam()->GetFileName(), basepath );
	}
	else{
		strcpy( basepath, g_FuncTable.m_pfnGetGamePath() );
	}

	const gchar *filename = g_FuncTable.m_pfnFileDialog( (void *)g_pRadiantWnd, FALSE, "Save Camera File", basepath, "camera", NULL );

	if ( filename ) {
		char fullpathtofile[PATH_MAX + 8];

		Q_realpath( filename, fullpathtofile, PATH_MAX );

		// File dialog from windows (and maybe the gtk one from radiant) doesn't handle default extensions properly.
		// Add extension and check again if file exists
		if ( strcmp( fullpathtofile + ( strlen( fullpathtofile ) - 7 ), ".camera" ) ) {
			strcat( fullpathtofile, ".camera" );

			if ( FileExists( fullpathtofile ) ) {
				if ( g_FuncTable.m_pfnMessageBox( (GtkWidget *)g_pRadiantWnd, "File already exists.\nOverwrite?", "Save Camera File", MB_YESNO, NULL ) == IDNO ) {
					return;
				}
			}
		}

		// see if this camera file was already loaded
		CCamera *checkCam = firstCam;
		while ( checkCam ) {
			if ( checkCam == GetCurrentCam() ) {
				checkCam = checkCam->GetNext();
				if ( !checkCam ) { // we only have one camera file opened so no need to check further
					break;
				}
			}
			else if ( !strcmp( fullpathtofile, checkCam->GetFileName() ) ) {
				char error[PATH_MAX + 64];
				sprintf( error, "Camera file \'%s\' is currently loaded by GtkRadiant.\nPlease select a different filename.", fullpathtofile );
				g_FuncTable.m_pfnMessageBox( (GtkWidget *)g_pRadiantWnd, error, "Save error", MB_OK, NULL );
				return;
			}
			checkCam = checkCam->GetNext();
		}

		// FIXME: check for existing directory

		GetCurrentCam()->GetCam()->save( fullpathtofile );
		GetCurrentCam()->SetFileName( fullpathtofile, true );
		RefreshCamListCombo();
	}
}

void DoUnloadCamera() {
	if ( !GetCurrentCam() ) {
		return;
	}

	if ( !GetCurrentCam()->HasBeenSaved() ) {
		char buf[PATH_MAX + 64];
		sprintf( buf, "Do you want to save the changes for camera '%s'?", GetCurrentCam()->GetCam()->getName() );
		if ( g_FuncTable.m_pfnMessageBox( (GtkWidget *)g_pRadiantWnd, buf, "Warning", MB_YESNO, NULL ) == IDYES ) {
			DoSaveCamera();
		}
	}
	else if ( GetCurrentCam()->HasBeenSaved() == 2 ) {
		char buf[PATH_MAX + 64];
		sprintf( buf, "Do you want to save the changes made to camera file '%s'?", GetCurrentCam()->GetFileName() );
		if ( g_FuncTable.m_pfnMessageBox( (GtkWidget *)g_pRadiantWnd, buf, "Warning", MB_YESNO, NULL ) == IDYES ) {
			DoSaveCamera();
		}
	}

	if ( g_pCurrentEditCam ) {
		DoStopEdit();
		g_pCurrentEditCam = NULL;
	}

	FreeCam( GetCurrentCam() );
	SetCurrentCam( NULL );
	RefreshCamListCombo();
}

CCamera *g_pCurrentEditCam = NULL;

void DoStartEdit( CCamera *cam ) {
	if ( g_pCurrentEditCam ) {
		DoStopEdit();
		g_pCurrentEditCam = NULL;
	}

	if ( cam ) {
		g_bEditOn = true;

		if ( !Listener ) {
			Listener = new CListener;
		}

		cam->GetCam()->startEdit( g_iActiveTarget < 0 ? true : false );

		g_pCurrentEditCam = cam;

		g_FuncTable.m_pfnSysUpdateWindows( W_XY_OVERLAY | W_CAMERA );
	}
}

void DoStopEdit( void ) {
	g_bEditOn = false;

	if ( Listener ) {
		delete Listener;
		Listener = NULL;
	}

	if ( g_pCurrentEditCam ) {
		// stop editing on the current cam
		g_pCurrentEditCam->GetCam()->stopEdit();
		g_pCurrentEditCam = NULL;

		g_FuncTable.m_pfnSysUpdateWindows( W_XY_OVERLAY | W_CAMERA );
	}
}
