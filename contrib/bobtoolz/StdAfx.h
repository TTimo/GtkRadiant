/*
BobToolz plugin for GtkRadiant
Copyright (C) 2001 Gordon Biggans

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

#ifndef __STDAFX_BOBTOOLZ__
#define __STDAFX_BOBTOOLZ__

#define BOBTOOLZ_MINOR "bobtoolz"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#include "time.h"

#define NAME_MAX 255

#if defined (__linux__) || defined (__APPLE__)

#include <GL/glx.h>

typedef void* HMODULE;
typedef void* LPVOID;
typedef char* LPCSTR;
//typedef int 	bool;


#define WINAPI
#define APIENTRY

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID
{
  unsigned long  Data1;
  unsigned short Data2;
  unsigned short Data3;
  unsigned char  Data4[8];
} GUID;

#define stricmp strcasecmp

#endif
 
#if defined(__cplusplus)
#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID             const GUID &
#endif // !_REFGUID_DEFINED
#endif

typedef struct tagRECT
{
    long    left;
    long    top;
    long    right;
    long    bottom;
} RECT, *PRECT, *LPRECT;

typedef uint UINT;

#endif // __linux__

#include "mathlib.h"
#include <string.h>
#include "qertypes.h"
#include <stdio.h>

#define USE_SCENEGRAPHTABLE_DEFINE
#include "iscenegraph.h"

#define USE_QERTABLE_DEFINE
#include "qerplugin.h"
extern	_QERFuncTable_1 __QERTABLENAME;

#define USE_ENTITYTABLE_DEFINE
#include "ientity.h"
extern _QEREntityTable __ENTITYTABLENAME;

#define USE_BRUSHTABLE_DEFINE
#include "ibrush.h"
extern  _QERBrushTable __BRUSHTABLENAME;

#define USE_PATCHTABLE_DEFINE
#include "ipatch.h"
extern  _QERPatchTable __PATCHTABLENAME;

#define USE_SHADERSTABLE_DEFINE
#include "ishaders.h"
extern _QERShadersTable __SHADERSTABLENAME;

#define USE_QGLTABLE_DEFINE
#include "igl.h"
extern	_QERQglTable __QGLTABLENAME;

#include "ibspfrontend.h"
extern	_QERAppBSPFrontendTable	g_BSPTable;

#include "iui.h"
extern	_QERUITable		          g_MessageTable;

#define USE_RENDERTABLE_DEFINE
#include "irender.h"

#define USE_SELECTIONTABLE_DEFINE
#include "iselection.h"

#include "itoolbar.h"


#include "iplugin.h"

#define MAX_ROUND_ERROR	0.05

#include "itexdef.h"

struct _QERFaceData
{
  vec3_t m_p0;
  vec3_t m_p1;
  vec3_t m_p2;
  texdef_t m_texdef;
};

#endif
