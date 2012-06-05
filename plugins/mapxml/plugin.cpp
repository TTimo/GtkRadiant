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

// =============================================================================
// Globals

// function tables
_QERFuncTable_1 g_FuncTable;
_QERShadersTable g_ShadersTable;
_QEREntityTable g_EntityTable;
_QERBrushTable g_BrushTable;
_QERPatchTable g_PatchTable;

// =============================================================================
// SYNAPSE

CSynapseServer* g_pSynapseServer = NULL;
CSynapseClientXMap g_SynapseClient;

static const XMLConfigEntry_t entries[] =
{
	{ SHADERS_MAJOR, SYN_REQUIRE, sizeof( g_ShadersTable ), &g_ShadersTable },
	{ NULL, SYN_UNKNOWN, 0, NULL }
};

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

	g_SynapseClient.AddAPI( MAP_MAJOR, "mapxml", sizeof( _QERPlugMapTable ) );
	g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( _QERFuncTable_1 ), SYN_REQUIRE, &g_FuncTable );
	g_SynapseClient.AddAPI( ENTITY_MAJOR, NULL, sizeof( g_EntityTable ), SYN_REQUIRE, &g_EntityTable );
	g_SynapseClient.AddAPI( BRUSH_MAJOR, NULL, sizeof( g_BrushTable ), SYN_REQUIRE, &g_BrushTable );
	g_SynapseClient.AddAPI( PATCH_MAJOR, NULL, sizeof( g_PatchTable ), SYN_REQUIRE, &g_PatchTable );

	if ( !g_SynapseClient.ConfigXML( pServer, NULL, entries ) ) {
		return NULL;
	}

	return &g_SynapseClient;
}

bool CSynapseClientXMap::RequestAPI( APIDescriptor_t *pAPI ){
	if ( !strcmp( pAPI->major_name, MAP_MAJOR ) ) {
		_QERPlugMapTable* pTable = static_cast<_QERPlugMapTable*>( pAPI->mpTable );
		pTable->m_pfnMap_Read = &Map_Read;
		pTable->m_pfnMap_Write = &Map_Write;

		return true;
	}

	Syn_Printf( "ERROR: RequestAPI( '%s' ) not found in '%s'\n", pAPI->major_name, GetInfo() );
	return false;
}

#include "version.h"

const char* CSynapseClientXMap::GetInfo(){
	return "XMAP module built " __DATE__ " " RADIANT_VERSION;
}

const char* CSynapseClientXMap::GetName(){
	return "xmap";
}
