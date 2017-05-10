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

// Render view
CRenderer         *Renderer = NULL;

// Interaction
CListener         *Listener = NULL;

// plugin name
static const char *PLUGIN_NAME = "Camera";

// commands in the menu
static const char *PLUGIN_COMMANDS = "About,-,Load Camera...,-,Preview Camera,-,Camera Inspector...,-,New Spline Camera...,New Interpolated Camera...,New Fixed Camera...";

// globals
GtkWidget *g_pRadiantWnd = NULL;
GtkWidget *g_pCameraInspectorWnd = NULL;
CCamera   *firstCam = NULL;       // double linked list
CCamera   *firstFreeCam = NULL;   // single linked list
CCamera   *currentCam = NULL;     // single item
bool g_bEditOn = false;
int g_iEditMode = 0;                    // 0: editting points 1: adding points
int g_iActiveTarget = -1;
int g_iPreviewRunning = 0;              // 0: no preview 1: start preview 2: preview in progress

static const char *PLUGIN_ABOUT = "Camera v1.0 for GtkRadiant\n"
								  "by Arnout van Meer (rr2do2@splashdamage.com)\n\n"
								  "This product contains software technology\n"
								  "from id Software, Inc. ('id Technology').\n"
								  "id Technology (c) 2001, 2002 id Software, Inc.";


#include "iplugin.h"

const char* QERPlug_Init( void* hApp, void* pMainWidget ){
	g_pRadiantWnd = (GtkWidget*)pMainWidget;

	// initialize cams
	for ( int i = 0; i < MAX_CAMERAS; i++ ) {
		if ( i == 0 ) {
			firstFreeCam = new CCamera( i );
			firstCam = firstFreeCam;
		}
		else {
			firstCam->SetNext( new CCamera( i ) );
			firstCam = firstCam->GetNext();
		}
	}
	firstCam = NULL;

	if ( !Renderer ) {
		Renderer = new CRenderer;
	}

	if ( g_pCameraInspectorWnd == NULL ) {
		g_pCameraInspectorWnd = CreateCameraInspectorDialog();
	}

	InitIglToQgl( &g_QglTable );

	GetFileTypeRegistry()->addType( "camera", filetype_t( "Camera file", "*.camera" ) );

	return "Camera for GtkRadiant";
}

const char* QERPlug_GetName(){
	return PLUGIN_NAME;
}

const char* QERPlug_GetCommandList(){
	return PLUGIN_COMMANDS;
}

void QERPlug_Dispatch( const char* p, float* vMin, float* vMax, bool bSingleBrush ){
	if ( !strcmp( p, "New Fixed Camera" ) ) {
		DoNewFixedCamera();
	}
	else if ( !strcmp( p, "New Interpolated Camera" ) ) {
		DoNewInterpolatedCamera();
	}
	else if ( !strcmp( p, "New Spline Camera" ) ) {
		DoNewSplineCamera();
	}
	else if ( !strcmp( p, "Camera Inspector..." ) ) {
		DoCameraInspector();
	}
	else if ( !strcmp( p, "Preview Camera" ) ) {
		DoPreviewCamera();
	}
	else if ( !strcmp( p, "Load Camera..." ) ) {
		DoLoadCamera();
	}
	else if ( !strcmp( p, "About" ) ) {
		g_FuncTable.m_pfnMessageBox( (GtkWidget *)g_pRadiantWnd, PLUGIN_ABOUT, "About", MB_OK, NULL );
	}
}


// toolbar

#include "itoolbar.h"

unsigned int ToolbarButtonCount(){
	return 1;
}

class CameraInspectorButton : public IToolbarButton
{
public:
virtual const char* getImage() const {
	return "camera_insp.bmp";
}
virtual const char* getText() const {
	return "Inspector";
}
virtual const char* getTooltip() const {
	return "Camera Inspector";
}
virtual void activate() const {
	DoCameraInspector();
}
virtual EType getType() const {
	return eButton;
}
};

CameraInspectorButton g_camerainspectorbutton;

const IToolbarButton* GetToolbarButton( unsigned int index ){
	return &g_camerainspectorbutton;
}


_QERFuncTable_1 g_FuncTable;
_QERQglTable g_QglTable;
_QERUITable g_UITable;
_QERCameraTable g_CameraTable;

// =============================================================================
// SYNAPSE

#include "synapse.h"

class CameraSynapseClient : public CSynapseClient
{
public:
// CSynapseClient API
bool RequestAPI( APIDescriptor_t *pAPI );
const char* GetInfo();

CameraSynapseClient() { }
virtual ~CameraSynapseClient() { }
};

CSynapseServer* g_pSynapseServer = NULL;
CameraSynapseClient g_SynapseClient;

#if __GNUC__ >= 4
#pragma GCC visibility push(default)
#endif
extern "C" CSynapseClient * SYNAPSE_DLL_EXPORT Synapse_EnumerateInterfaces( const char *version, CSynapseServer *pServer ) {
#if __GNUC__ >= 4
#pragma GCC visibility pop
#endif
	if ( strcmp( version, SYNAPSE_VERSION ) ) {
		Syn_Printf( "ERROR: synapse API version mismatch: should be '" SYNAPSE_VERSION "', got '%s'\n", version );
		return NULL;
	}
	g_pSynapseServer = pServer;
	g_pSynapseServer->IncRef();
	Set_Syn_Printf( g_pSynapseServer->Get_Syn_Printf() );

	g_SynapseClient.AddAPI( TOOLBAR_MAJOR, "camera", sizeof( _QERPlugToolbarTable ) );
	g_SynapseClient.AddAPI( PLUGIN_MAJOR, "camera", sizeof( _QERPluginTable ) );

	g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( _QERFuncTable_1 ), SYN_REQUIRE, &g_FuncTable );
	g_SynapseClient.AddAPI( UI_MAJOR, NULL, sizeof( _QERUITable ), SYN_REQUIRE, &g_UITable );
	g_SynapseClient.AddAPI( QGL_MAJOR, NULL, sizeof( _QERQglTable ), SYN_REQUIRE, &g_QglTable );
	g_SynapseClient.AddAPI( CAMERA_MAJOR, NULL, sizeof( _QERCameraTable ), SYN_REQUIRE, &g_CameraTable );

	return &g_SynapseClient;
}

bool CameraSynapseClient::RequestAPI( APIDescriptor_t *pAPI ){
	if ( !strcmp( pAPI->major_name, TOOLBAR_MAJOR ) ) {
		_QERPlugToolbarTable* pTable = static_cast<_QERPlugToolbarTable*>( pAPI->mpTable );

		pTable->m_pfnToolbarButtonCount = &ToolbarButtonCount;
		pTable->m_pfnGetToolbarButton = &GetToolbarButton;
		return true;
	}
	else if ( !strcmp( pAPI->major_name, PLUGIN_MAJOR ) ) {
		_QERPluginTable* pTable = static_cast<_QERPluginTable*>( pAPI->mpTable );

		pTable->m_pfnQERPlug_Init = QERPlug_Init;
		pTable->m_pfnQERPlug_GetName = QERPlug_GetName;
		pTable->m_pfnQERPlug_GetCommandList = QERPlug_GetCommandList;
		pTable->m_pfnQERPlug_Dispatch = QERPlug_Dispatch;
		return true;
	}

	Syn_Printf( "ERROR: RequestAPI( '%s' ) not found in '%s'\n", pAPI->major_name, GetInfo() );
	return false;
}

#include "version.h"

const char* CameraSynapseClient::GetInfo(){
	return "Camera plugin v1.0 - Arnout van Meer - built " __DATE__ " " RADIANT_VERSION;
}



//
// CCamera
//
CCamera *AllocCam() {
	if ( !firstFreeCam ) {
		return( NULL );
	}

	CCamera *cam = firstFreeCam;
	firstFreeCam = firstFreeCam->GetNext();
	cam->Init();
	if ( firstCam ) {
		cam->SetNext( firstCam );
		firstCam->SetPrev( cam );
	}
	firstCam = cam;

	return( cam );
}

void FreeCam( CCamera *cam ) {
	if ( cam->GetPrev() ) {
		if ( cam->GetNext() ) {
			cam->GetPrev()->SetNext( cam->GetNext() );
			cam->GetNext()->SetPrev( cam->GetPrev() );
		}
		else {
			cam->GetPrev()->SetNext( NULL );
		}
	}
	else if ( cam->GetNext() ) {
		cam->GetNext()->SetPrev( NULL );
		firstCam = cam->GetNext();
	}
	else {
		firstCam = NULL;
	}

	cam->GetCam()->clear();
	cam->Init();

	if ( firstFreeCam ) {
		cam->SetNext( firstFreeCam );
	}
	firstFreeCam = cam;
}

void SetCurrentCam( CCamera *cam ) {
	currentCam = cam;
}

CCamera *GetCurrentCam() {
	return( currentCam );
}
