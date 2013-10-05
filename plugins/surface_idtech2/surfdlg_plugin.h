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

#ifndef _SURFDLG_PLUGIN_H_
#define _SURFDLG_PLUGIN_H_

#ifdef __linux__

typedef void* HMODULE;
typedef void* LPVOID;
typedef char* LPCSTR;

#endif // __linux__

#include "qerplugin.h"
#include "synapse.h"
#include "iselectedface.h"
#include "iundo.h"
#include "ishaders.h"
#include "mathlib.h"
#include "missing.h"
#include "idata.h"

#include "isurfaceplugin.h"

#define SURFACEDIALOG_MINOR "idtech2"
#define SHADERS_MINOR "idtech2"

class SurfaceDialog : public IPluginTexdef
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
};

extern _QERFuncTable_1 g_FuncTable;
extern _QERUndoTable g_UndoTable;
extern _QERAppSurfaceTable g_AppSurfaceTable;
extern _QERSelectedFaceTable g_SelectedFaceTable;
extern _QERShadersTable g_ShadersTable;
extern _QERAppShadersTable g_AppShadersTable;
extern _QERAppDataTable g_AppDataTable;

#define GetSelectedFaceCount g_SelectedFaceTable.m_pfnGetSelectedFaceCount

#define Undo_Undo g_UndoTable.m_pfnUndo_Undo
#define Undo_GetUndoId g_UndoTable.m_pfnUndo_GetUndoId

#define Sys_Printf g_FuncTable.m_pfnSysPrintf
#define Sys_FPrintf g_FuncTable.m_pfnSysFPrintf
#define Sys_UpdateWindows g_FuncTable.m_pfnSysUpdateWindows

#define Select_FitTexture g_AppSurfaceTable.m_pfnSelect_FitTexture
#define Get_SI_Inc g_AppSurfaceTable.m_pfnQERApp_QeglobalsSavedinfo_SIInc
#define GridSize g_AppSurfaceTable.m_pfnQeglobalsGetGridSize
#define FaceList_FitTexture g_AppSurfaceTable.m_pfnFaceList_FitTexture
#define GetMainWindow g_AppSurfaceTable.m_pfnGetMainWindow
#define GetSelectedFaceCountfromBrushes g_AppSurfaceTable.m_pfnGetSelectedFaceCountfromBrushes
#define GetSelFacesTexdef g_AppSurfaceTable.m_pfnGetSelFacesTexdef
#define SetTexdef_FaceList g_AppSurfaceTable.m_pfnSetTexdef_FaceList
#define SetWinPos_from_Prefs g_AppSurfaceTable.m_pfnSetWinPos_From_Prefs

#define Texturewin g_AppShadersTable.m_pfnQeglobalsTexturewin

#endif // _SURFDLG_PLUGIN_H_
