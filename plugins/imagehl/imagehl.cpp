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
// HalfLife WAD format image loading plugin
//
// hydra - hydra@hydras-world.com
//

#include <stdio.h>
#include "imagehl.h"
#include "lbmlib.h"

// =============================================================================
// static variables

_QERFuncTable_1 g_FuncTable; // Radiant function table
_QERFileSystemTable g_FileSystemTable;

// =============================================================================
// SYNAPSE

CSynapseServer* g_pSynapseServer = NULL;
CSynapseClientImageHL g_SynapseClient;

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

#ifdef USE_HLW
	g_SynapseClient.AddAPI( IMAGE_MAJOR, "hlw", sizeof( _QERPlugImageTable ) );
#endif
#ifdef USE_MIP
	g_SynapseClient.AddAPI( IMAGE_MAJOR, "mip", sizeof( _QERPlugImageTable ) );
#endif
#ifdef USE_IDSP
	g_SynapseClient.AddAPI( IMAGE_MAJOR, "spr", sizeof( _QERPlugImageTable ) );
#endif
	// this "wad" needs to be "*" for the VFS, we don't care what VFS we have, as long as we have one.
	g_SynapseClient.AddAPI( VFS_MAJOR, "*", sizeof( _QERFileSystemTable ), SYN_REQUIRE, &g_FileSystemTable ); // wad, typically
	g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( _QERFuncTable_1 ), SYN_REQUIRE, &g_FuncTable );

	return &g_SynapseClient;
}

bool CSynapseClientImageHL::RequestAPI( APIDescriptor_t *pAPI ){
	if ( !strcmp( pAPI->major_name, "image" ) ) {
		_QERPlugImageTable* pTable = static_cast<_QERPlugImageTable*>( pAPI->mpTable );

		pTable->m_pfnLoadImage = &LoadImage;
		return true;
	}

	Syn_Printf( "ERROR: RequestAPI( '%s' ) not found in '%s'\n", pAPI->major_name, GetInfo() );
	return false;
}

#include "version.h"

const char* CSynapseClientImageHL::GetInfo(){
	return "imagehl formats module built " __DATE__ " " RADIANT_VERSION;
}
