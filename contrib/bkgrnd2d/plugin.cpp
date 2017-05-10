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
// 2d background Plugin
//
// Code by reyalP aka Reed Mideke
//
// Based on
//

/*
    Overview
    ========
    This little plugin allows you to display an image in the background of the
    gtkradiant XY window.

    Version History
    ===============

    v0.1
      - Initial version.
    v0.2
      - three views, dialog box, toolbar
    v0.25
      - tooltips, follow gtkradiant coding conventions

    How ?
    -----
     - textures 'n widgets 'n stuff.
 */

//#include "plugin.h"
//TODO we just poke the objects directly
#include "bkgrnd2d.h"
#include "dialog.h"

#define CMD_SEP "-"
#define CMD_CONFIG "Configure..."
#define CMD_ABOUT "About"
// =============================================================================
// Globals

// function tables
_QERFuncTable_1 g_FuncTable;
_QERQglTable g_QglTable;
_QERFileSystemTable g_FileSystemTable;
_QEREntityTable g_EntityTable;
_QERAppDataTable g_DataTable;

// for the file load dialog
void *g_pMainWidget = NULL;

// =============================================================================
// plugin implementation

static const char *PLUGIN_NAME = "2d window background plugin";

//backwards for some reason
static const char *PLUGIN_COMMANDS = CMD_ABOUT ";"
									 CMD_SEP ";"
									 CMD_CONFIG
;

static const char *PLUGIN_ABOUT = "2d window background v0.25\n\n"
								  "By reyalP (hellsownpuppy@yahoo.com)";




void DoBkgrndToggleXY();
void DoBkgrndToggleXZ();
void DoBkgrndToggleYZ();

#define NUM_TOOLBAR_BUTTONS 4
struct toolbar_button_info_s
{
	const char *image;
	const char *text;
	const char *tip;
	void ( *func )();
	IToolbarButton::EType type;
};

struct toolbar_button_info_s toolbar_buttons[NUM_TOOLBAR_BUTTONS] =
{
	{
		"bkgrnd2d_xy_toggle.bmp",
		"xy background",
		"Toggle xy background image",
		DoBkgrndToggleXY,
		IToolbarButton::eToggleButton
	},
	{
		"bkgrnd2d_xz_toggle.bmp",
		"xz background",
		"Toggle xz background image",
		DoBkgrndToggleXZ,
		IToolbarButton::eToggleButton
	},
	{
		"bkgrnd2d_yz_toggle.bmp",
		"yz background",
		"Toggle yz background image",
		DoBkgrndToggleYZ,
		IToolbarButton::eToggleButton
	},
	{
		"bkgrnd2d_conf.bmp",
		"Configure",
		"Configure background images",
		ShowBackgroundDialog,
		IToolbarButton::eButton
	},
};

class Bkgrnd2dButton : public IToolbarButton
{
public:
const toolbar_button_info_s *bi;
virtual const char* getImage() const {
	return bi->image;
}
virtual const char* getText() const {
	return bi->text;
}
virtual const char* getTooltip() const {
	return bi->tip;
}
virtual void activate() const {
	bi->func();
	return ;
}
virtual EType getType() const {
	return bi->type;
}
};

Bkgrnd2dButton g_bkgrnd2dbuttons[NUM_TOOLBAR_BUTTONS];

unsigned int ToolbarButtonCount(){
	return NUM_TOOLBAR_BUTTONS;
}

const IToolbarButton* GetToolbarButton( unsigned int index ){
	g_bkgrnd2dbuttons[index].bi = &toolbar_buttons[index];
	return &g_bkgrnd2dbuttons[index];
}

extern "C" const char* QERPlug_Init( void *hApp, void* pMainWidget ){
	g_pMainWidget = pMainWidget;

	InitBackgroundDialog();
	render.Register();

	GetFileTypeRegistry()->addType( FILETYPE_KEY, filetype_t( "all files", "*.*" ) );
	GetFileTypeRegistry()->addType( FILETYPE_KEY, filetype_t( "jpeg files", "*.jpg" ) );
	GetFileTypeRegistry()->addType( FILETYPE_KEY, filetype_t( "targa files", "*.tga" ) );
	return (char *) PLUGIN_NAME;
}

extern "C" const char* QERPlug_GetName(){
	return (char *) PLUGIN_NAME;
}

extern "C" const char* QERPlug_GetCommandList(){
	return (char *) PLUGIN_COMMANDS;
}

extern "C" void QERPlug_Dispatch( const char *p, vec3_t vMin, vec3_t vMax, bool bSingleBrush ){
	Sys_Printf( MSG_PREFIX "Command \"%s\"\n",p );
	if ( !strcmp( p, CMD_ABOUT ) ) {
		g_FuncTable.m_pfnMessageBox( g_pMainWidget, PLUGIN_ABOUT, "About", MB_OK, NULL );
	}
	else if ( !strcmp( p,CMD_CONFIG ) ) {
		ShowBackgroundDialog();
	}
}

//TODO these three suck
void DoBkgrndToggleXY(){
	Sys_Printf( MSG_PREFIX "DoBkgrndToggleXY\n" );
	// always toggle, since the buttons do
	backgroundXY.m_bActive = ( backgroundXY.m_bActive ) ? false : true;
	// if we don't have image or extents, and we activated,
	// bring up the dialog with the corresponding page
	// would be better to hide or grey out button, but we can't
	if ( backgroundXY.m_bActive && !backgroundXY.Valid() ) {
		ShowBackgroundDialogPG( 0 );
	}
	else{
		g_FuncTable.m_pfnSysUpdateWindows( W_XY );
	}
}

void DoBkgrndToggleXZ(){
	Sys_Printf( MSG_PREFIX "DoBkgrndToggleXZ\n" );
	backgroundXZ.m_bActive = ( backgroundXZ.m_bActive ) ? false : true;
	if ( backgroundXZ.m_bActive && !backgroundXZ.Valid() ) {
		ShowBackgroundDialogPG( 1 );
	}
	else{
		g_FuncTable.m_pfnSysUpdateWindows( W_XY );
	}
}

void DoBkgrndToggleYZ(){
	Sys_Printf( MSG_PREFIX "DoBkgrndToggleYZ\n" );
	backgroundYZ.m_bActive = ( backgroundYZ.m_bActive ) ? false : true;
	if ( backgroundYZ.m_bActive && !backgroundYZ.Valid() ) {
		ShowBackgroundDialogPG( 2 );
	}
	else{
		g_FuncTable.m_pfnSysUpdateWindows( W_XY );
	}
}

// =============================================================================
// SYNAPSE

CSynapseServer* g_pSynapseServer = NULL;
CSynapseClientBkgrnd2d g_SynapseClient;

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

	g_SynapseClient.AddAPI( TOOLBAR_MAJOR, BKGRND2D_MINOR, sizeof( _QERPlugToolbarTable ) );
	g_SynapseClient.AddAPI( PLUGIN_MAJOR, BKGRND2D_MINOR, sizeof( _QERPluginTable ) );

	g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( g_FuncTable ), SYN_REQUIRE, &g_FuncTable );
	g_SynapseClient.AddAPI( QGL_MAJOR, NULL, sizeof( g_QglTable ), SYN_REQUIRE, &g_QglTable );
// TODO is this the right way to ask for 'whichever VFS we have loaded' ? Seems to work
// for misc filename functions
	g_SynapseClient.AddAPI( VFS_MAJOR, "*", sizeof( g_FileSystemTable ), SYN_REQUIRE, &g_FileSystemTable );
// get worldspawn
	g_SynapseClient.AddAPI( ENTITY_MAJOR, NULL, sizeof( g_EntityTable ), SYN_REQUIRE, &g_EntityTable );
// selected brushes
	g_SynapseClient.AddAPI( DATA_MAJOR, NULL, sizeof( g_DataTable ), SYN_REQUIRE, &g_DataTable );

	return &g_SynapseClient;
}

bool CSynapseClientBkgrnd2d::RequestAPI( APIDescriptor_t *pAPI ){
	if ( !strcmp( pAPI->major_name, PLUGIN_MAJOR ) ) {
		_QERPluginTable* pTable = static_cast<_QERPluginTable*>( pAPI->mpTable );

		pTable->m_pfnQERPlug_Init = QERPlug_Init;
		pTable->m_pfnQERPlug_GetName = QERPlug_GetName;
		pTable->m_pfnQERPlug_GetCommandList = QERPlug_GetCommandList;
		pTable->m_pfnQERPlug_Dispatch = QERPlug_Dispatch;
		return true;
	}
	if ( !strcmp( pAPI->major_name, TOOLBAR_MAJOR ) ) {
		_QERPlugToolbarTable* pTable = static_cast<_QERPlugToolbarTable*>( pAPI->mpTable );

		pTable->m_pfnToolbarButtonCount = &ToolbarButtonCount;
		pTable->m_pfnGetToolbarButton = &GetToolbarButton;
		return true;
	}

	Syn_Printf( "ERROR: RequestAPI( '%s' ) not found in '%s'\n", pAPI->major_name, GetInfo() );
	return false;
}

#include "version.h"

const char* CSynapseClientBkgrnd2d::GetInfo(){
	return "2d Background plugin built " __DATE__ " " RADIANT_VERSION;
}

const char* CSynapseClientBkgrnd2d::GetName(){
	return "bkgrnd2d";
}
