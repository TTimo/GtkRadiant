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

#define VC_EXTRALEAN

#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#define BOBTOOLZ_MINOR "bobtoolz"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#include "time.h"

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )

// Necessary for proper boolean type declaration
#include "qertypes.h"

#include <GL/glx.h>

typedef void* HMODULE;
typedef void* LPVOID;
typedef char* LPCSTR;
//typedef int   bool;

#define MB_OK                       0x00000000L
#define MB_OKCANCEL                 0x00000001L
#define MB_ABORTRETRYIGNORE         0x00000002L
#define MB_YESNOCANCEL              0x00000003L
#define MB_YESNO                    0x00000004L
#define MB_RETRYCANCEL              0x00000005L


#define MB_ICONHAND                 0x00000010L
#define MB_ICONQUESTION             0x00000020L
#define MB_ICONEXCLAMATION          0x00000030L
#define MB_ICONASTERISK             0x00000040L

#define MB_USERICON                 0x00000080L
#define MB_ICONWARNING              MB_ICONEXCLAMATION
#define MB_ICONERROR                MB_ICONHAND
#define MB_ICONINFORMATION          MB_ICONASTERISK
#define MB_ICONSTOP                 MB_ICONHAND

#define MB_TYPEMASK                 0x0000000FL
#define MB_ICONMASK                 0x000000F0L
#define MB_DEFMASK                  0x00000F00L
#define MB_MODEMASK                 0x00003000L
#define MB_MISCMASK                 0x0000C000L

#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7

#define WINAPI
#ifndef APIENTRY
	#define APIENTRY
#endif

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID
{
	unsigned long Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char Data4[8];
} GUID;

#define stricmp strcasecmp

#endif

#if defined( __cplusplus )
#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID             const GUID &
#endif // !_REFGUID_DEFINED
#endif

typedef struct tagRECT
{
	long left;
	long top;
	long right;
	long bottom;
} RECT, *PRECT, *LPRECT;

typedef uint UINT;

#endif // defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )

#include "synapse.h"
#include "iplugin.h"
#define USE_QERTABLE_DEFINE

#include "missing.h" // temporary stuff, needs to be removed

#include "str.h"
#include "qertypes.h"
#include "qerplugin.h"
#include "idata.h"
#include "ibrush.h"
#include "iselectedface.h"
#include "ishaders.h"
#include "ibspfrontend.h"
#include "iui.h"
#include "igl.h"
#include "itoolbar.h"
#include "ientity.h"

#include "mathlib.h"

extern _QERFuncTable_1 g_FuncTable;
extern _QERAppDataTable g_AppDataTable;
extern _QERBrushTable g_BrushTable;
extern _QERSelectedFaceTable g_SelectedFaceTable;
extern _QERShadersTable g_ShadersTable;
extern _QERQglTable g_QglTable;
extern _QERUITable g_MessageTable;
extern _QEREntityTable g_EntityTable;


#define MAX_ROUND_ERROR 0.05

#include "gtkr_list.h"

#endif
