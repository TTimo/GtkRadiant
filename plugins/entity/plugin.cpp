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
// Model Plugin
//

#include "plugin.h"
#include "entity.h"
#include "entity_entitymodel.h"
#include "light.h"

// =============================================================================
// Globals

// function tables
_QERFuncTable_1 g_FuncTable;
_QERQglTable g_QglTable;
_QERBrushTable __BRUSHTABLENAME;
_QERUndoTable __UNDOTABLENAME;
_EClassManagerTable __ECLASSMANAGERTABLENAME;

// =============================================================================
// SYNAPSE

class CSynapseClientEntity : public CSynapseClient
{
public:
// CSynapseClient API
bool RequestAPI( APIDescriptor_t *pAPI );
const char* GetInfo();

CSynapseClientEntity() { }
virtual ~CSynapseClientEntity() { }
};


CSynapseServer* g_pSynapseServer = NULL;
CSynapseClientEntity g_SynapseClient;

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

	g_SynapseClient.AddAPI( ENTITY_MAJOR, NULL, sizeof( _QEREntityTable ) );
	g_SynapseClient.AddAPI( RADIANT_MAJOR, NULL, sizeof( g_FuncTable ), SYN_REQUIRE, &g_FuncTable );
	g_SynapseClient.AddAPI( QGL_MAJOR, NULL, sizeof( g_QglTable ), SYN_REQUIRE, &g_QglTable );
	g_SynapseClient.AddAPI( BRUSH_MAJOR, NULL, sizeof( __BRUSHTABLENAME ), SYN_REQUIRE, &__BRUSHTABLENAME );
	g_SynapseClient.AddAPI( UNDO_MAJOR, NULL, sizeof( __UNDOTABLENAME ), SYN_REQUIRE, &__UNDOTABLENAME );
	g_SynapseClient.AddAPI( ECLASSMANAGER_MAJOR, NULL, sizeof( __ECLASSMANAGERTABLENAME ), SYN_REQUIRE, &__ECLASSMANAGERTABLENAME );

	return &g_SynapseClient;
}

bool CSynapseClientEntity::RequestAPI( APIDescriptor_t *pAPI ){
	if ( !strcmp( pAPI->major_name, ENTITY_MAJOR ) ) {
		_QEREntityTable* pTable = static_cast<_QEREntityTable*>( pAPI->mpTable );
		pTable->m_pfnEntity_Alloc = &Entity_Alloc;
		pTable->m_pfnEntity_Free = &Entity_Free;
		pTable->m_pfnEntity_Clone = &Entity_Clone;
		pTable->m_pfnSetKeyValue = &SetKeyValue;
		pTable->m_pfnDeleteKey = &DeleteKey;
		pTable->m_pfnValueForKey = &ValueForKey;
		pTable->m_pfnFloatForKey = &FloatForKey;
		pTable->m_pfnIntForKey = &IntForKey;
		pTable->m_pfnGetVectorForKey = &GetVectorForKey;
		pTable->m_pfnEntity_AddToList = &Entity_AddToList;
		pTable->m_pfnEntity_RemoveFromList = &Entity_RemoveFromList;
		pTable->m_pfnEntity_LinkBrush = &Entity_LinkBrush;
		pTable->m_pfnEntity_UnlinkBrush = &Entity_UnlinkBrush;
		pTable->m_pfnDrawLight = &DrawLight;
		pTable->m_pfnEntity_MemorySize = &Entity_MemorySize;
		pTable->m_pfnAllocateEpair = &Entity_AllocateEpair;
		pTable->m_pfnGetEntityKeyValList = &Entity_GetKeyValList;
		pTable->m_pfnSetEntityKeyValList = &Entity_SetKeyValList;

		return true;
	}

	Syn_Printf( "ERROR: RequestAPI( '%s' ) not found in '%s'\n", pAPI->major_name, GetInfo() );
	return false;
}

#include "version.h"

const char* CSynapseClientEntity::GetInfo(){
	return "Entity module built " __DATE__ " " RADIANT_VERSION;
}
