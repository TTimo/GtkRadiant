/*
   Copyright (c) 2001, Loki software, inc.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of Loki software nor the names of its contributors may be used
   to endorse or promote products derived from this software without specific prior
   written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT,INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// Image loading plugin
//
// Leonardo Zide (leo@lokigames.com)
//

#include <stdio.h>
#include "imagem8.h"

// =============================================================================
// global tables

_QERFuncTable_1 g_FuncTable; // Radiant function table
_QERFileSystemTable g_FileSystemTable;

// =============================================================================
// SYNAPSE

CSynapseServer* g_pSynapseServer = NULL;
CSynapseClientImage g_SynapseClient;

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

	g_SynapseClient.AddAPI( IMAGE_MAJOR, "m8", sizeof( _QERPlugImageTable ) );
	g_SynapseClient.AddAPI( IMAGE_MAJOR, "m32", sizeof( _QERPlugImageTable ) );
	g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( _QERFuncTable_1 ), SYN_REQUIRE, &g_FuncTable );

	Syn_Printf( "Dynamic APIs for client '%s'\n", g_SynapseClient.GetInfo() );
	if ( !g_pSynapseServer->SelectClientConfig( g_SynapseClient.GetName() ) ) {
		Syn_Printf( "ERROR: Failed to select synapse client config in '%s'\n", g_SynapseClient.GetInfo() );
		return NULL;
	}
	char *api, *minor;
	while ( g_pSynapseServer->GetNextConfig( &api, &minor ) )
	{
		Syn_Printf( "api: '%s' minor: '%s'\n", api, minor );
		if ( !strcmp( api, VFS_MAJOR ) ) {
			g_SynapseClient.AddAPI( VFS_MAJOR, minor, sizeof( _QERFileSystemTable ), SYN_REQUIRE, &g_FileSystemTable );
		}
		else
		{
			Syn_Printf( "WARNING: unknown API node '%s' in synapse config from module '%s'\n", api, g_SynapseClient.GetInfo() );
		}
	}
	return &g_SynapseClient;
}

bool CSynapseClientImage::RequestAPI( APIDescriptor_t *pAPI ){
	if ( !strcmp( pAPI->major_name, IMAGE_MAJOR ) ) {
		_QERPlugImageTable* pTable = static_cast<_QERPlugImageTable*>( pAPI->mpTable );
		if ( !strcmp( pAPI->minor_name, "m8" ) ) {
			pTable->m_pfnLoadImage = &LoadM8;
			return true;
		}
		if ( !strcmp( pAPI->minor_name, "m32" ) ) {
			pTable->m_pfnLoadImage = &LoadM32;
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
	return "M8 M32 formats module built " __DATE__ " " RADIANT_VERSION;
}
