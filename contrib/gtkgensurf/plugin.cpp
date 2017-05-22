/*
   GenSurf plugin for GtkRadiant
   Copyright (C) 2001 David Hyde, Loki software and qeradiant.com

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "gensurf.h"

// Global plugin FuncTable
_QERFuncTable_1 g_FuncTable;
_QERQglTable g_GLTable;
_QERUIGtkTable g_UIGtkTable;
_QEREntityTable g_EntityTable;
bool SingleBrushSelected;
bool g_bInitDone;

#include "iplugin.h"

const char* QERPlug_Init( void* hApp, void* pMainWidget ){
	g_pRadiantWnd = (GtkWidget*)pMainWidget;

	return "GenSurf for Q3Radiant";
}

const char* QERPlug_GetName(){
	return "GtkGenSurf";
}

const char* QERPlug_GetCommandList(){
	return "About;-;Wall facing 270...;Wall facing 180...;Wall facing 90...;Wall facing 0...;"
		   "Ceiling...;Ground surface...";
}

// vMin/vMax provide the bounds of the selection, they are zero if there is no selection
// if there is a selection, bSingleBrush will be true if a single brush is selected
// if so, typical plugin behaviour (such as primitive creation) would use the bounds as
// a rule to create the primitive, then delete the selection
void QERPlug_Dispatch( const char *p, vec3_t vMin, vec3_t vMax, bool bSingleBrush ){
	bool Generate = false;

	if ( !g_bInitDone ) {
		if ( GenSurfInit() ) {
			g_bInitDone = true;
		}
	}

	if ( !strcmp( p, "Ground surface..." ) ) {
		SingleBrushSelected = bSingleBrush;
		Plane = PLANE_XY0;
		if ( SingleBrushSelected ) {
			Hll = vMin[0];
			Vll = vMin[1];
			Hur = vMax[0];
			Vur = vMax[1];
			Z00 = Z01 = Z10 = Z11 = vMax[2];
		}
		Generate = true;
	}
	else if ( !strcmp( p, "Ceiling..." ) ) {
		SingleBrushSelected = bSingleBrush;
		Plane = PLANE_XY1;
		if ( SingleBrushSelected ) {
			Hll = vMin[0];
			Vll = vMin[1];
			Hur = vMax[0];
			Vur = vMax[1];
			Z00 = Z01 = Z10 = Z11 = vMin[2];
		}
		Generate = true;
	}
	else if ( !strcmp( p, "Wall facing 0..." ) ) {
		SingleBrushSelected = bSingleBrush;
		Plane = PLANE_YZ0;
		if ( SingleBrushSelected ) {
			Hll = vMin[1];
			Vll = vMin[2];
			Hur = vMax[1];
			Vur = vMax[2];
			Z00 = Z01 = Z10 = Z11 = vMax[0];
		}
		Generate = true;
	}
	else if ( !strcmp( p, "Wall facing 90..." ) ) {
		SingleBrushSelected = bSingleBrush;
		Plane = PLANE_XZ0;
		if ( SingleBrushSelected ) {
			Hll = vMin[0];
			Vll = vMin[2];
			Hur = vMax[0];
			Vur = vMax[2];
			Z00 = Z01 = Z10 = Z11 = vMax[1];
		}
		Generate = true;
	}
	else if ( !strcmp( p, "Wall facing 180..." ) ) {
		SingleBrushSelected = bSingleBrush;
		Plane = PLANE_YZ1;
		if ( SingleBrushSelected ) {
			Hll = vMin[1];
			Vll = vMin[2];
			Hur = vMax[1];
			Vur = vMax[2];
			Z00 = Z01 = Z10 = Z11 = vMin[0];
		}
		Generate = true;
	}
	else if ( !strcmp( p, "Wall facing 270..." ) ) {
		SingleBrushSelected = bSingleBrush;
		Plane = PLANE_XZ1;
		if ( SingleBrushSelected ) {
			Hll = vMin[0];
			Vll = vMin[2];
			Hur = vMax[0];
			Vur = vMax[2];
			Z00 = Z01 = Z10 = Z11 = vMin[1];
		}
		Generate = true;
	}
	else if ( !strcmp( p,"About" ) ) {
		About( g_pRadiantWnd );
	}

	if ( Generate ) {
		if ( SingleBrushSelected ) {
			UseFaceBounds();
		}

		gtk_widget_show( g_pWnd );
	}
}

extern "C" LPVOID WINAPI QERPlug_GetFuncTable(){
	return &g_FuncTable;
}

// =============================================================================
// SYNAPSE

#include "synapse.h"

class GenSurfSynapseClient : public CSynapseClient
{
public:
// CSynapseClient API
bool RequestAPI( APIDescriptor_t *pAPI );
const char* GetInfo();

GenSurfSynapseClient() { }
virtual ~GenSurfSynapseClient() { }
};

CSynapseServer* g_pSynapseServer = NULL;
GenSurfSynapseClient g_SynapseClient;

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

	g_SynapseClient.AddAPI( PLUGIN_MAJOR, "gtkgensurf", sizeof( _QERPluginTable ) );

	g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( _QERFuncTable_1 ), SYN_REQUIRE, &g_FuncTable );
	g_SynapseClient.AddAPI( UIGTK_MAJOR, NULL, sizeof( _QERUIGtkTable ), SYN_REQUIRE, &g_UIGtkTable );
	g_SynapseClient.AddAPI( QGL_MAJOR, NULL, sizeof( _QERQglTable ), SYN_REQUIRE, &g_GLTable );
	g_SynapseClient.AddAPI( ENTITY_MAJOR, NULL, sizeof( _QEREntityTable ), SYN_REQUIRE, &g_EntityTable );

	return &g_SynapseClient;
}

bool GenSurfSynapseClient::RequestAPI( APIDescriptor_t *pAPI ){
	if ( !strcmp( pAPI->major_name, PLUGIN_MAJOR ) ) {
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

const char* GenSurfSynapseClient::GetInfo(){
	return "GtkGenSurf - built " __DATE__ " " RADIANT_VERSION;
}
