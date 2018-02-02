
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

#include "plugin.h"

// toolbar implementation

class CFlushReloadSelectedToolbarButton : public IToolbarButton
{
public:
virtual const char* getImage() const {
	return "model_reload_entity.bmp";
}
virtual const char* getText() const {
	return "Reload";
}
virtual const char* getTooltip() const {
	return "Flush & Reload Selected Model";
}
virtual void activate() const {
	DoFlushReloadSelected();
}
virtual EType getType() const {
	return eButton;
}
};

CFlushReloadSelectedToolbarButton g_flushreloadselected;

unsigned int ToolbarButtonCount(){
	return 1;
}

const IToolbarButton* GetToolbarButton( unsigned int index ){
	return &g_flushreloadselected;
}

// =============================================================================
// Pico utility functions

#include "picomodel.h"

void PicoPrintFunc( int level, const char *str ){
	if ( str == NULL ) {
		return;
	}
	switch ( level )
	{
	case PICO_NORMAL:
		Sys_Printf( "%s\n", str );
		break;

	case PICO_VERBOSE:
		Sys_FPrintf( SYS_VRB, "%s\n", str );
		break;

	case PICO_WARNING:
		Sys_FPrintf( SYS_WRN, "WARNING: %s\n", str );
		break;

	case PICO_ERROR:
		Sys_FPrintf( SYS_VRB, "ERROR: %s\n", str );
		break;

	case PICO_FATAL:
		Sys_FPrintf( SYS_ERR, "ERROR: %s\n", str );
		break;
	}
}

void PicoLoadFileFunc( const char *name, byte **buffer, int *bufSize ){
	*bufSize = vfsLoadFile( name, (void**) buffer, 0 );
}

void PicoFreeFileFunc( void* file ){
	vfsFreeFile( file );
}

static void initialise(){
	PicoInit();
	PicoSetMallocFunc( malloc );
	PicoSetFreeFunc( free );
	PicoSetPrintFunc( PicoPrintFunc );
	PicoSetLoadFileFunc( PicoLoadFileFunc );
	PicoSetFreeFileFunc( PicoFreeFileFunc );
}

static void add_model_apis( CSynapseClient& client ){
	const picoModule_t** modules = PicoModuleList( NULL );
	while ( *modules != NULL )
	{
		const picoModule_t* module = *modules++;
		if ( module->canload && module->load ) {
			for ( unsigned int j = 0; module->defaultExts[j] != NULL; j++ )
				client.AddAPI( MODEL_MAJOR, module->defaultExts[j], sizeof( _QERPlugModelTable ) );
		}
	}
}

static bool model_is_supported( const char* extension ){
	const picoModule_t** modules = PicoModuleList( NULL );
	while ( *modules != NULL )
	{
		const picoModule_t* module = *modules++;
		if ( module->canload && module->load ) {
			for ( unsigned int j = 0; module->defaultExts[j] != NULL; j++ )
				if ( strcmp( extension, module->defaultExts[j] ) == 0 ) {
					return true;
				}
		}
	}
	return false;
}

void init_filetypes(){
	const picoModule_t **modules = PicoModuleList( NULL );
	while ( *modules != NULL )
	{
		const picoModule_t* module = *modules++;
		if ( module->canload && module->load ) {
			for ( char*const* ext = module->defaultExts; *ext != NULL; ++ext )
			{
				char buf[16];
				buf[0] = '*';
				buf[1] = '.';
				strcpy( buf + 2, *ext );
				GetFileTypeRegistry()->addType( MODEL_MAJOR, filetype_t( module->displayName, buf ) );
			}
		}
	}
}

// plugin implementation

static const char *PLUGIN_NAME = "Model loading module";
static const char *PLUGIN_COMMANDS = "About;-;Flush & Reload Models;Flush & Reload Selected";
static const char *PLUGIN_ABOUT = "Model Module v1.0 for GtkRadiant\nby Arnout van Meer (rr2do2@splashdamage.com)\n\nBased on the MD3Model Module by SPoG\nPicoModel Library Copyright (c) 2002, Randy Reddig & seaw0lf\n\nSupported models:\n";

void *g_pMainWidget = NULL;

extern "C" const char* QERPlug_Init( void *hApp, void* pMainWidget ){
	g_pMainWidget = pMainWidget;
	init_filetypes();
	return (char *) PLUGIN_NAME;
}

extern "C" const char* QERPlug_GetName(){
	return (char *) PLUGIN_NAME;
}

extern "C" const char* QERPlug_GetCommandList(){
	return (char *) PLUGIN_COMMANDS;
}

extern "C" void QERPlug_Dispatch( const char *p, vec3_t vMin, vec3_t vMax, bool bSingleBrush ){
	if ( !strcmp( p, "Flush & Reload Selected" ) ) {
		DoFlushReloadSelected();
	}
	else if ( !strcmp( p, "Flush & Reload Models" ) ) {
		DoFlushReloadAll();
	}
	else if ( !strcmp( p, "About" ) ) {
		const picoModule_t** modules = PicoModuleList( NULL );
		char about_buf[1024];
		strncpy( about_buf, PLUGIN_ABOUT, sizeof( about_buf ) - 1 );
		while ( *modules != NULL ) {
			const picoModule_t* module = *modules++;
			strncat( about_buf, module->displayName, sizeof( about_buf ) - 1 );
			strncat( about_buf, " (", sizeof( about_buf ) - 1 );
			strncat( about_buf, module->defaultExts[0], sizeof( about_buf ) - 1 );
			strncat( about_buf, ")\n\t", sizeof( about_buf ) - 1 );
			strncat( about_buf, module->copyright, sizeof( about_buf ) - 1 );
			strncat( about_buf, "\n", sizeof( about_buf ) - 1 );
		}
		g_FuncTable.m_pfnMessageBox( g_pMainWidget, about_buf, "About", MB_OK, NULL );
	}
}


void DoFlushReloadSelected() {
}

void DoFlushReloadAll() {
	GetModelCache()->RefreshAll();
}

// =============================================================================

// function tables
_QERFuncTable_1 g_FuncTable;
_QERQglTable g_QglTable;
_QERShadersTable g_ShadersTable;
_QERFileSystemTable g_FileSystemTable;

// =============================================================================
// SYNAPSE

CSynapseServer* g_pSynapseServer = NULL;
CSynapseClientModel g_SynapseClient;

static const XMLConfigEntry_t entries[] =
{ { SHADERS_MAJOR, SYN_REQUIRE, sizeof( g_ShadersTable ), &g_ShadersTable },
  { VFS_MAJOR, SYN_REQUIRE, sizeof( g_FileSystemTable ), &g_FileSystemTable },
  { NULL, SYN_UNKNOWN, 0, NULL } };

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

	initialise();

	add_model_apis( g_SynapseClient );
	g_SynapseClient.AddAPI( TOOLBAR_MAJOR, "model", sizeof( _QERPlugToolbarTable ) );
	g_SynapseClient.AddAPI( PLUGIN_MAJOR, "model", sizeof( _QERPluginTable ) );

	g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( g_FuncTable ), SYN_REQUIRE, &g_FuncTable );
	g_SynapseClient.AddAPI( QGL_MAJOR, NULL, sizeof( g_QglTable ), SYN_REQUIRE, &g_QglTable );

	if ( !g_SynapseClient.ConfigXML( pServer, NULL, entries ) ) {
		return NULL;
	}

	return &g_SynapseClient;
}

bool CSynapseClientModel::RequestAPI( APIDescriptor_t *pAPI ){
	if ( !strcmp( pAPI->major_name, MODEL_MAJOR ) ) {
		_QERPlugModelTable* pTable = static_cast<_QERPlugModelTable*>( pAPI->mpTable );

		if ( model_is_supported( pAPI->minor_name ) ) {
			pTable->m_pfnLoadModel = &LoadModel;
			return true;
		}
	}
	else if ( !strcmp( pAPI->major_name, TOOLBAR_MAJOR ) ) {
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

const char* CSynapseClientModel::GetInfo(){
	return "picomodel loader module built " __DATE__ " " RADIANT_VERSION;
}

const char* CSynapseClientModel::GetName(){
	return "model";
}
