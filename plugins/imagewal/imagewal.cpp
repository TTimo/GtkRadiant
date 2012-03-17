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

#include <stdio.h>
#include "imagewal.h"

// =============================================================================
// global tables

_QERFuncTable_1 g_FuncTable; // Radiant function table
_QERFileSystemTable g_FileSystemTable;

// =============================================================================
// SYNAPSE

CSynapseServer* g_pSynapseServer = NULL;
CSynapseClientImage g_SynapseClient;

static const XMLConfigEntry_t entries[] =
{
	{ VFS_MAJOR, SYN_REQUIRE, sizeof( _QERFileSystemTable ), &g_FileSystemTable },
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

	g_SynapseClient.AddAPI( IMAGE_MAJOR, "wal", sizeof( _QERPlugImageTable ) );
	g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( _QERFuncTable_1 ), SYN_REQUIRE, &g_FuncTable );

	if ( !g_SynapseClient.ConfigXML( pServer, NULL, entries ) ) {
		return NULL;
	}

	return &g_SynapseClient;
}

bool CSynapseClientImage::RequestAPI( APIDescriptor_t *pAPI ){
	if ( !strcmp( pAPI->major_name, IMAGE_MAJOR ) ) {
		_QERPlugImageTable* pTable = static_cast<_QERPlugImageTable*>( pAPI->mpTable );
		if ( !strcmp( pAPI->minor_name, "wal" ) ) {
			pTable->m_pfnLoadImage = &LoadWAL;
			return true;
		}
	}

	Syn_Printf( "ERROR: RequestAPI( '%s' ) not found in '%s'\n", pAPI->major_name, GetInfo() );
	return false;
}

bool CSynapseClientImage::OnActivate() {
	if ( !g_FileSystemTable.m_nSize ) {
		Syn_Printf( "ERROR: VFS_MAJOR table was not initialized before OnActivate in '%s' - incomplete synapse.config?\n", GetInfo() );
		return false;
	}
	return true;
}

#include "version.h"

const char* CSynapseClientImage::GetInfo(){
	return "WAL formats module built " __DATE__ " " RADIANT_VERSION;
}
