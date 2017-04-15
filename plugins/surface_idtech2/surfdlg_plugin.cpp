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

#include <stdio.h>
#include "surfdlg_plugin.h"
#include "surfacedialog.h"

#include "synapse.h"

void *g_pMainWidget;
const char* QERPlug_Init( void* hApp, void *pWidget );

_QERFuncTable_1 g_FuncTable;
_QERUndoTable g_UndoTable;
_QERAppSurfaceTable g_AppSurfaceTable;
_QERSelectedFaceTable g_SelectedFaceTable;
_QERShadersTable g_ShadersTable;
_QERAppShadersTable g_AppShadersTable;
_QERAppDataTable g_AppDataTable;

class CSynapseClient_SurfDLG : public CSynapseClient
{
public:
// CSynapseClient API
bool RequestAPI( APIDescriptor_t *pAPI );
const char* GetInfo();

CSynapseClient_SurfDLG() { }
virtual ~CSynapseClient_SurfDLG() { }
};

CSynapseServer* g_pSynapseServer = NULL;
CSynapseClient_SurfDLG g_SynapseClient;

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

	g_SynapseClient.AddAPI( SURFACEDIALOG_MAJOR, SURFACEDIALOG_MINOR, sizeof( _QERPlugSurfaceTable ) );

	g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( _QERFuncTable_1 ), SYN_REQUIRE, &g_FuncTable );
	g_SynapseClient.AddAPI( UNDO_MAJOR, NULL, sizeof( _QERUndoTable ), SYN_REQUIRE, &g_UndoTable );
	g_SynapseClient.AddAPI( APPSURFACEDIALOG_MAJOR, NULL, sizeof( _QERAppSurfaceTable ), SYN_REQUIRE, &g_AppSurfaceTable );
	g_SynapseClient.AddAPI( SELECTEDFACE_MAJOR, NULL, sizeof( _QERSelectedFaceTable ), SYN_REQUIRE, &g_SelectedFaceTable );
	g_SynapseClient.AddAPI( SHADERS_MAJOR, SHADERS_MINOR, sizeof( _QERShadersTable ), SYN_REQUIRE, &g_ShadersTable );
	g_SynapseClient.AddAPI( APPSHADERS_MAJOR, NULL, sizeof( _QERAppShadersTable ), SYN_REQUIRE, &g_AppShadersTable );
	g_SynapseClient.AddAPI( DATA_MAJOR, NULL, sizeof( _QERAppDataTable ), SYN_REQUIRE, &g_AppDataTable );

	return &g_SynapseClient;
}

bool CSynapseClient_SurfDLG::RequestAPI( APIDescriptor_t *pAPI ) {

	if ( !strcmp( pAPI->major_name, SURFACEDIALOG_MAJOR ) ) {
		if ( !strcmp( pAPI->minor_name, SURFACEDIALOG_MINOR ) ) {
			_QERPlugSurfaceTable* pSurfaceTable = static_cast<_QERPlugSurfaceTable*>( pAPI->mpTable );
			pSurfaceTable->m_pfnToggleSurface = &ToggleSurface;
			pSurfaceTable->m_pfnDoSurface = &DoSurface;
			pSurfaceTable->m_pfnUpdateSurfaceDialog = &UpdateSurfaceDialog;
			pSurfaceTable->m_pfnSurfaceDlgFitAll = &SurfaceDlgFitAll;
			pSurfaceTable->m_pfnGet_SI_Module_Widget = &Get_SI_Module_Widget;
			return true;
		}
	}

	Syn_Printf( "ERROR: RequestAPI( '%s' ) not found in '%s'\n", pAPI->major_name, GetInfo() );
	return false;
}

#include "version.h"

const char *CSynapseClient_SurfDLG::GetInfo(){
	return "Surface Dialog (idTech2) module built " __DATE__ " " RADIANT_VERSION;
}

const char* QERPlug_Init( void* hApp, void *pWidget ){

	g_pMainWidget = pWidget;

	return "idTech2 Surface Dialog for Radiant";
}
