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

#if defined( _WIN32 )
  // required for static linking libxml on Windows
  #define LIBXML_STATIC
#endif
#include "libxml/parser.h"

#include "synapse.h"
#define USE_QERTABLE_DEFINE
#include "qerplugin.h"
#include "imap.h"
#include "ishaders.h"
#define USE_ENTITYTABLE_DEFINE
#include "ientity.h"
#define USE_BRUSHTABLE_DEFINE
#include "ibrush.h"
#define USE_PATCHTABLE_DEFINE
#include "ipatch.h"

extern _QERFuncTable_1 g_FuncTable;
extern _QERShadersTable g_ShadersTable;
extern _QEREntityTable g_EntityTable;
extern _QERBrushTable g_BrushTable;
extern _QERPatchTable g_PatchTable;

#define Error g_FuncTable.m_pfnError
#define QERApp_Shader_ForName g_ShadersTable.m_pfnShader_ForName

extern void *g_pRadiantWnd;

void Map_Read( IDataStream *in, CPtrArray *map );
void Map_Write( CPtrArray *map, IDataStream *out );

extern CSynapseServer* g_pSynapseServer;

class CSynapseClientXMap : public CSynapseClient
{
public:
// CSynapseClient API
bool RequestAPI( APIDescriptor_t *pAPI );
const char* GetInfo();
const char* GetName();   ///< required for XML runtime config

CSynapseClientXMap() { }
virtual ~CSynapseClientXMap() { }
};

#endif // _PLUGIN_H_
