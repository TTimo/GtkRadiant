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

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
//
//

#ifndef __ISURFACEPLUGIN_H_
#define __ISURFACEPLUGIN_H_

typedef struct _GtkWidget GtkWidget;
typedef struct _GtkWindow GtkWindow;

#define SURFACEDIALOG_MAJOR "surfdialog"

// there's a void* in each qtexture_t, must be casted to a IPluginTexdef*
// there's a void* in each face_t, must be casted to a IPluginTexdef*
// NOTE: IPluginTexdef stores a pointer to the qtexture_t or face_t it's stored in
// members of IPluginTexdef often access the qtexture_t or face_t they are connected to

// Write texdef needs a function pointer, because Radiant either writes into a FILE or a CMemFile
typedef void ( *PFN_QERAPP_MAPPRINTF )( char *text, ... );

class IPluginTexdef
{
public:
virtual ~IPluginTexdef() { }
// Increment the number of references to this object
virtual void IncRef() = 0;
// Decrement the reference count
virtual void DecRef() = 0;
};



// Nurail: For SI module
class texdef_to_face_t
{
public:
texdef_to_face_t* next;
brush_t *brush;     // Brush faces belong to (for Undo)
face_t *face;       // Face of Texdef
texdef_t texdef;    // Working texdef
texdef_t orig_texdef;    // Original, for baselining changes
brushprimit_texdef_t orig_bp_texdef;   // Original, for undo
};


typedef void ( *PFN_QERPLUG_DOSURFACE )();
typedef void ( *PFN_QERPLUG_TOGGLESURFACE )();
typedef void ( *PFN_QERPLUG_UPDATESURFACEDIALOG )();
typedef void ( *PFN_QERPLUG_SURFACEDLGFITALL )();
typedef GtkWidget* ( *PFN_GET_SI_MODULE_WIDGET )();

struct _QERPlugSurfaceTable
{
	int m_nSize;
	PFN_QERPLUG_TOGGLESURFACE m_pfnToggleSurface;
	PFN_QERPLUG_DOSURFACE m_pfnDoSurface;
	PFN_QERPLUG_UPDATESURFACEDIALOG m_pfnUpdateSurfaceDialog;
	PFN_QERPLUG_SURFACEDLGFITALL m_pfnSurfaceDlgFitAll;
	PFN_GET_SI_MODULE_WIDGET m_pfnGet_SI_Module_Widget;
};

// this one is used by the plugin to access some Radiant stuff

#define APPSURFACEDIALOG_MAJOR "appsurfdialog"

// {42BAE4C0-9787-11d3-8EF3-0000E8E8657B}
static const GUID QERAppSurfaceTable_GUID =
{ 0x42bae4c0, 0x9787, 0x11d3, { 0x8e, 0xf3, 0x0, 0x0, 0xe8, 0xe8, 0x65, 0x7b } };

typedef bool ( *PFN_PATCHESSELECTED )();
// retrieve g_qeglobals.texturewin_t
//++timo FIXME: this should move in a dedicated table for all g_qeglobals stuff
typedef texturewin_t* ( *PFN_QEGLOBALSTEXTUREWIN )();
// look for the first selected patch mesh
//++timo FIXME: this is a convenient func since there's no way to scan patches ( yet )
typedef patchMesh_t* ( *PFN_GETSELECTEDPATCH )();
//++timo FIXME: this one in particular is a hack
typedef void ( *PFN_GETTWOSELECTEDPATCH )( patchMesh_t **p1, patchMesh_t **p2 );


// leo FIXME: hacks uglier than the ones above
typedef void ( *PFN_TEXMATTOFAKETEXCOORDS )( vec_t texMat[2][3], float shift[2], float *rot, float scale[2] );
typedef void ( *PFN_CONVERTTEXMATWITHQTEXTURE )( brushprimit_texdef_t *texMat1, qtexture_t *qtex1, brushprimit_texdef_t *texMat2, qtexture_t *qtex2 );
typedef void ( *PFN_FAKETEXCOORDSTOTEXMAT )( float shift[2], float rot, float scale[2], vec_t texMat[2][3] );
typedef void ( *PFN_PATCH_RESETTEXTURING )( float fx, float fy );
typedef void ( *PFN_PATCH_FITTEXTURING )();
typedef void ( *PFN_PATCH_NATURALIZESELECTED )( bool bCap );
typedef const char* ( *PFN_PATCH_GETTEXTURENAME )();
typedef qboolean ( *PFN_QE_SINGLEBRUSH )( bool bQuiet );
typedef qboolean ( *PFN_ISBRUSHPRIMITMODE )();
typedef void ( *PFN_SELECT_FITTEXTURE )( int nHeight, int nWidth );
typedef void ( *PFN_COMPUTEAXISBASE )( vec3_t normal,vec3_t texS,vec3_t texT );
typedef void ( *PFN_BPMATMUL )( vec_t A[2][3], vec_t B[2][3], vec_t C[2][3] );
typedef void ( *PFN_EMITBRUSHPRIMITTEXCOORDS )( face_t * f, winding_t * w );
typedef texdef_t* ( *PFN_QEGLOBALSSAVEDINFO_SIINC )();
typedef float ( *PFN_QEGLOBALSGETGRIDSIZE )();
typedef void ( *PFN_FACELIST_FITTEXTURE )( texdef_to_face_t* texdef_face_list, int nHeight, int nWidth );
typedef GtkWindow* ( *PFN_GETMAINWINDOW )();
typedef void ( *PFN_SETWINPOS_FROM_PREFS )( GtkWidget *win );
typedef int ( *PFN_GETSELECTEDFACECOUNT_BRUSH )();
typedef void ( *PFN_GETSELFACESTEXDEF )( texdef_to_face_t * );
typedef void ( *PFN_SETTEXDEF_FACELIST )( texdef_to_face_t* texdef_face_list, bool b_SetUndoPoint, bool bFit_to_Scale );
typedef void ( *PFN_SETACTIVEINRADIANT )();


struct _QERAppSurfaceTable
{
	int m_nSize;
	PFN_PATCHESSELECTED m_pfnOnlyPatchesSelected;
	PFN_PATCHESSELECTED m_pfnAnyPatchesSelected;
	PFN_GETSELECTEDPATCH m_pfnGetSelectedPatch;
	PFN_GETTWOSELECTEDPATCH m_pfnGetTwoSelectedPatch;
	PFN_TEXMATTOFAKETEXCOORDS m_pfnTexMatToFakeTexCoords;
	PFN_CONVERTTEXMATWITHQTEXTURE m_pfnConvertTexMatWithQTexture;
	PFN_FAKETEXCOORDSTOTEXMAT m_pfnFakeTexCoordsToTexMat;
	PFN_PATCH_RESETTEXTURING m_pfnPatch_ResetTexturing;
	PFN_PATCH_FITTEXTURING m_pfnPatch_FitTexturing;
	PFN_PATCH_NATURALIZESELECTED m_pfnPatch_NaturalizeSelected;
	PFN_PATCH_GETTEXTURENAME m_pfnPatch_GetTextureName;
	PFN_QE_SINGLEBRUSH m_pfnQE_SingleBrush;
	PFN_ISBRUSHPRIMITMODE m_pfnIsBrushPrimitMode;
	PFN_COMPUTEAXISBASE m_pfnComputeAxisBase;
	PFN_BPMATMUL m_pfnBPMatMul;
	PFN_EMITBRUSHPRIMITTEXCOORDS m_pfnEmitBrushPrimitTextureCoordinates;
	PFN_QEGLOBALSTEXTUREWIN m_pfnQeglobalsTexturewin;
	PFN_SELECT_FITTEXTURE m_pfnSelect_FitTexture;
	PFN_QEGLOBALSSAVEDINFO_SIINC m_pfnQERApp_QeglobalsSavedinfo_SIInc;
	PFN_QEGLOBALSGETGRIDSIZE m_pfnQeglobalsGetGridSize;
	PFN_FACELIST_FITTEXTURE m_pfnFaceList_FitTexture;
	PFN_GETMAINWINDOW m_pfnGetMainWindow;
	PFN_SETWINPOS_FROM_PREFS m_pfnSetWinPos_From_Prefs;
	PFN_GETSELECTEDFACECOUNT_BRUSH m_pfnGetSelectedFaceCountfromBrushes;
	PFN_GETSELFACESTEXDEF m_pfnGetSelFacesTexdef;
	PFN_SETTEXDEF_FACELIST m_pfnSetTexdef_FaceList;
};

#endif
