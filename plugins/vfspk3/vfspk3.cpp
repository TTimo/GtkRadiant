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
// Quake 3 Virtual FileSystem - reads files from different dirs and inside pak files
//
// Leonardo Zide (leo@lokigames.com)
//

#ifdef _WIN32
#include <wtypes.h>
#endif

#include <stdio.h>
#include <errno.h>
#include "vfspk3.h"
#include "vfs.h"

// =============================================================================
// SYNAPSE

_QERFuncTable_1 g_FuncTable;

CSynapseServer* g_pSynapseServer = NULL;
CSynapseClientVFS g_SynapseClient;

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

	for ( int i = 0; pak_ext_list[i] != NULL ; i++ ) {
		// ".pk3" -> "pk3"
		g_SynapseClient.AddAPI( VFS_MAJOR, pak_ext_list[i] + sizeof('.'), sizeof( _QERFileSystemTable ) );
	}

	g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( _QERFuncTable_1 ), SYN_REQUIRE, &g_FuncTable );

	return &g_SynapseClient;
}

bool CSynapseClientVFS::RequestAPI( APIDescriptor_t *pAPI ){
	if ( !strcmp( pAPI->major_name, VFS_MAJOR ) ) {
		_QERFileSystemTable* pTable = static_cast<_QERFileSystemTable*>( pAPI->mpTable );
		pTable->m_pfnInitDirectory = &vfsInitDirectory;
		pTable->m_pfnShutdown = &vfsShutdown;
		pTable->m_pfnFreeFile = &vfsFreeFile;
		pTable->m_pfnGetDirList = &vfsGetDirList;
		pTable->m_pfnGetFileList = &vfsGetFileList;
		pTable->m_pfnClearFileDirList = &vfsClearFileDirList;
		pTable->m_pfnGetFileCount = &vfsGetFileCount;
		pTable->m_pfnLoadFile = &vfsLoadFile;
		pTable->m_pfnLoadFullPathFile = &vfsLoadFullPathFile;
		pTable->m_pfnExtractRelativePath = &vfsExtractRelativePath;
		pTable->m_pfnGetFullPath = &vfsGetFullPath;
		pTable->m_pfnBasePromptPath = &vfsBasePromptPath;
		return true;
	}

	Syn_Printf( "ERROR: RequestAPI( '%s' ) not found in '%s'\n", pAPI->major_name, GetInfo() );
	return false;
}

#include "version.h"

const char* CSynapseClientVFS::GetInfo(){
	return "PK3 VFS module built " __DATE__ " " RADIANT_VERSION;
}
