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
// Sprite Model Plugin
//
// Code by Hydra aka Dominic Clifton
//
// Based on MD3Model source code by SPoG
//

#ifndef _PLUGIN_H_
#define _PLUGIN_H_

/*!
   \todo need general notice about lib purpose etc.
   and the external dependencies (such as GLib, STL, mathlib etc.)
 */

/*!
   \todo not sure about what should be used for common data structures, GLib or STL
   I think STL would be better since I intend on using STL in synapse
 */

#include <stdio.h>

#include "synapse.h"
#include "iplugin.h"
#define USE_QERTABLE_DEFINE
#include "qerplugin.h"
#include "imodel.h"
#include "igl.h"
#include "ifilesystem.h"
#include "ishaders.h"

extern _QERFuncTable_1 g_FuncTable;
extern _QERQglTable g_QglTable;
extern _QERShadersTable g_ShadersTable;

#define QERApp_Shader_ForName g_ShadersTable.m_pfnShader_ForName
#define QERApp_Try_Shader_ForName g_ShadersTable.m_pfnTry_Shader_ForName

void LoadSpriteModel( entity_interfaces_t *interfaces, const char *name );

extern CSynapseServer* g_pSynapseServer;

class CSynapseClientModel : public CSynapseClient
{
public:
// CSynapseClient API
bool RequestAPI( APIDescriptor_t *pAPI );
const char* GetInfo();
const char* GetName();

CSynapseClientModel() { }
virtual ~CSynapseClientModel() { }
};


#endif // _PLUGIN_H_
