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
// Shaders Plugin
//

#include "plugin.h"

// =============================================================================
// Globals

// function tables
_QERFuncTable_1 g_FuncTable;
_QERAppDataTable g_DataTable;
_QERQglTable g_QglTable;
_QERAppShadersTable g_ShadersTable;
_QERFileSystemTable g_VFSTable;
_QERScripLibTable g_ScripLibTable;
_QERBrushTable g_BrushTable;

// static bool g_bInterfaceInitDone = false;

// =============================================================================
// SYNAPSE

CSynapseServer* g_pSynapseServer = NULL;
CSynapseClientShaders g_SynapseClient;

static const XMLConfigEntry_t entries[] =
{
	{ SHADERS_MAJOR, SYN_PROVIDE, sizeof( _QERShadersTable ), NULL },
	{ VFS_MAJOR, SYN_REQUIRE, sizeof( g_VFSTable ), &g_VFSTable },
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

	if ( !g_SynapseClient.ConfigXML( pServer, NULL, entries ) ) {
		return NULL;
	}

	g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( _QERFuncTable_1 ), SYN_REQUIRE, &g_FuncTable );
	g_SynapseClient.AddAPI( DATA_MAJOR, NULL, sizeof( _QERAppDataTable ), SYN_REQUIRE, &g_DataTable );
	g_SynapseClient.AddAPI( QGL_MAJOR, NULL, sizeof( _QERQglTable ), SYN_REQUIRE, &g_QglTable );
	g_SynapseClient.AddAPI( APPSHADERS_MAJOR, NULL, sizeof( _QERAppShadersTable ), SYN_REQUIRE, &g_ShadersTable );
	g_SynapseClient.AddAPI( SCRIPLIB_MAJOR, NULL, sizeof( _QERScripLibTable ), SYN_REQUIRE, &g_ScripLibTable );
	g_SynapseClient.AddAPI( BRUSH_MAJOR, NULL, sizeof( _QERBrushTable ), SYN_REQUIRE, &g_BrushTable );

	return &g_SynapseClient;
}

#include "version.h"

const char* CSynapseClientShaders::GetInfo(){
	return "Q3/Half-Life shaders module built " __DATE__ " " RADIANT_VERSION;
}

const char* CSynapseClientShaders::GetName(){
	return "shaders";
}
