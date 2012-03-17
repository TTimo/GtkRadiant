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

#ifndef _IMAGEWAL_H_
#define _IMAGEWAL_H_

#include "synapse.h"
#include "qerplugin.h"
#include "ifilesystem.h"
#include "iimage.h"

#ifdef __linux__

typedef void* HMODULE;
typedef void* LPVOID;
typedef char* LPCSTR;

#endif // __linux__

extern _QERFuncTable_1 g_FuncTable;
extern _QERFileSystemTable g_FileSystemTable;
void LoadWAL( const char *filename, unsigned char **pic, int *width, int *height );

#define Error g_FuncTable.m_pfnError
#define Sys_Printf g_FuncTable.m_pfnSysPrintf
#define vfsLoadFile g_FileSystemTable.m_pfnLoadFile
#define vfsFreeFile g_FileSystemTable.m_pfnFreeFile

class CSynapseClientImage : public CSynapseClient
{
public:
// CSynapseClient API
bool RequestAPI( APIDescriptor_t *pAPI );
const char* GetInfo();
bool OnActivate();
const char* GetName() { return "image"; }

CSynapseClientImage() { }
virtual ~CSynapseClientImage() { }
};

#endif // _IMAGEWAL_H_
