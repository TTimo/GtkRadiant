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

// stdafx.h
// precompiled headers

// standard headers
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )

// Necessary for proper boolean type declaration
#include "qertypes.h"

typedef void* HMODULE;
typedef void* LPVOID;
typedef char* LPCSTR;

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

typedef struct tagRECT
{
	long left;
	long top;
	long right;
	long bottom;
} RECT, *PRECT, *LPRECT;

#endif // defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )

// plugin
// FIXME TTimo: drop this
extern "C" void Sys_Printf( char *text, ... );

#include "synapse.h"
#include "iplugin.h"
#include "qerplugin.h"
#include "mathlib.h"
#include "igl.h"
#include "iselectedface.h"
#include "isurfaceplugin.h"
#include "iui.h"

// internals
// the implementation of a IWindowListener interface to use with the native UI
// TODO: move in it's own set of files?
// NOTE: I'm not too sure about the bool flags being any use.. they are supposed to tell if we handle the event or not
class CWindowListener : public IWindowListener
{
int refCount;
public:
// Increment the number of references to this object
void IncRef() { refCount++; }
// Decrement the reference count
void DecRef() {
	if ( --refCount <= 0 ) {
		delete this;
	}
}
// IWindowListener ---------------------------------------
bool OnLButtonDown( guint32 nFlags, double x, double y );
bool OnMButtonDown( guint32 nFlags, double x, double y ) { return false; }
bool OnRButtonDown( guint32 nFlags, double x, double y );
bool OnLButtonUp( guint32 nFlags, double x, double y );
bool OnMButtonUp( guint32 nFlags, double x, double y ) { return false; }
bool OnRButtonUp( guint32 nFlags, double x, double y );
bool OnMouseMove( guint32 nFlags, double x, double y );
bool OnKeyPressed( char *s );
bool Paint();
void Close();
};

#include "2DView.h"
typedef struct
{
	float data[MAX_POINTS_ON_WINDING][2];
} CtrlPts_t;
#include "ControlPointsManager.h"

extern _QERQglTable g_QglTable;
extern _QERFuncTable_1 g_FuncTable;
// prefs globals
// NOTE: these are used by the CControlPointsManager classes, not very C++ish
extern bool g_bPrefsUpdateCameraView;
extern _QERSelectedFaceTable g_SelectedFaceTable;
extern _QERFaceData g_CancelFaceData;

#define Sys_Printf g_FuncTable.m_pfnSysPrintf
#define Sys_FPrintf g_FuncTable.m_pfnSysFPrintf

// call to validate the current changes into the editor
extern void Textool_Validate();
extern void Textool_Cancel();

class CSynapseClientTexTool : public CSynapseClient
{
public:
// CSynapseClient API
bool RequestAPI( APIDescriptor_t *pAPI );
const char* GetInfo();

CSynapseClientTexTool() { }
virtual ~CSynapseClientTexTool() { }
};

extern IWindow *g_pToolWnd;
