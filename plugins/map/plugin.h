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

#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include <stdlib.h>
#include <stdio.h>

#include "synapse.h"

#define USE_QERTABLE_DEFINE
#include "qerplugin.h"
#define USE_VFSTABLE_DEFINE
#define USE_SCRIPLIBTABLE_DEFINE
#include "iscriplib.h"
#include "imap.h"
#include "ishaders.h"
#define USE_ENTITYTABLE_DEFINE
#include "ientity.h"
#define USE_BRUSHTABLE_DEFINE
#include "ibrush.h"
#define USE_PATCHTABLE_DEFINE
#include "ipatch.h"
#define USE_VFSTABLE_DEFINE
#include "ifilesystem.h"

extern _QERFuncTable_1 g_FuncTable;
extern _QERScripLibTable g_ScripLibTable;
extern _QERShadersTable g_ShadersTable;
extern _QEREntityTable __ENTITYTABLENAME;
extern _QERBrushTable g_BrushTable;
extern _QERPatchTable g_PatchTable;
extern _QERFileSystemTable g_FileSystemTable;

#define Error g_FuncTable.m_pfnError
#define QERApp_Shader_ForName g_ShadersTable.m_pfnShader_ForName

// a bunch of globals to the module
extern void *g_pRadiantWnd;

#define MAPVERSION_Q3 1
#define MAPVERSION_HL 2
#define MAPVERSION_Q2 3
//#define MAPVERSION_Q1

#define MAP_NOERROR 0
#define MAP_ABORTED 1
#define MAP_WRONGVERSION 2

void Map_ReadQ3( IDataStream *in, CPtrArray *map );
void Map_WriteQ3( CPtrArray *map, IDataStream *out );
void Map_ReadHL( IDataStream *in, CPtrArray *map );
void Map_WriteHL( CPtrArray *map, IDataStream *out );
void Map_ReadQ2( IDataStream *in, CPtrArray *map );
void Map_WriteQ2( CPtrArray *map, IDataStream *out );

extern CSynapseServer* g_pSynapseServer;

class CSynapseClientMap : public CSynapseClient
{
bool mbMapHL;
public:
// CSynapseClient API
bool RequestAPI( APIDescriptor_t *pAPI );
const char* GetInfo();
const char* GetName();   ///< required for runtime configuration
bool OnActivate();

CSynapseClientMap() { mbMapHL = false; }
virtual ~CSynapseClientMap() { }
};

#endif // _PLUGIN_H_
