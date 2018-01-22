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

#ifndef _QE3_H_
#define _QE3_H_

#ifdef _WIN32

// disable data conversion warnings for gl
#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA
#pragma warning(disable : 4800)

#endif

// for interfaces, we require main plugin header included
#include "iplugin.h"
#include "qerplugin.h"
#include "qertypes.h"

#include "missing.h" // temporary stuff, needs to be removed
#include "idatastream.h"
#include "file.h"

#include "qgl.h"

#include <gtk/gtk.h>
#include <math.h>
#include <stdlib.h>

#include <stdio.h>

// this is the version to expect from template projects
// bump this to force a regeneration of the template menus
// this is useful if you update/edit the BSP command lines for instance
#define PROJECT_VERSION 2

//#define MEM_DEBUG
#ifdef MEM_DEBUG

#define malloc( a ) debug_malloc( a, __FILE__, __LINE__ )
#define free( a ) debug_free( a, __FILE__, __LINE__ )

void* debug_malloc( size_t size, const char* file, int line );
void debug_free( void *buf, const char* file, int line );

#endif

#ifdef _DEBUG
//#define DBG_WINDOWPOS
#endif

#ifdef DBG_WINDOWPOS
void CheckWatchit( char *msg );
#endif

// those two files are generated
// if they are missing, you NEED to run makeversion.sh
// NOTE: for win32 users, cygwin installation is REQUIRED to run makeversion.sh
// NOTE TTimo if any of those changes (they might change a lot), then the whole app is rebuilt.
//   very often it's not necessary
#include "version.h"
#include "aboutmsg.h"

// synapse is our utility lib for dynamic shared objects management
#include "synapse.h"

#include "qertypes.h"
#include "cmdlib.h"
#include "mathlib.h"
#include "parse.h"

#include "qedefs.h"
#include "qfiles.h"
#include "textures.h"
#include "brush.h"
//#include "entity.h"
#define USE_ENTITYTABLE_DEFINE
#include "ientity.h"
extern _QEREntityTable __ENTITYTABLENAME;
// wrappers for brush access
#include "ibrush.h"
// wrappers for patch access
#include "ipatch.h"

#include "imodel.h"

#include "imap.h"

#include "iundo.h"

extern _QERPlugMapTable g_MapTable;

//++timo for BP conversion escaping FIXME: remove when mixing two formats!
extern bool g_bCancel_Map_LoadFile;
// used to be #defines, multiple engine support suggests we should go towards dynamic
extern vec_t g_MaxWorldCoord;
extern vec_t g_MinWorldCoord;
extern vec_t g_MaxBrushSize;
/*
   // set to true when we are parsing a terrain entity
   extern bool g_bParseTerrain;
   extern IShader *g_pTerrainShader, *g_pCaulk;
 */
#include "map.h"

#include "select.h"

#include "camera.h"
#include "z.h"

#include "undo.h"
#include "glwidget.h"

// set these before calling CheckParm
extern int myargc;
extern char **myargv;

// our own implementation of Q_int, clamping can be disabled on prefs
vec_t Rad_rint( vec_t in );

double I_FloatTime( void );

void   Error( const char *error, ... );
int    CheckParm( const char *check );
void   ParseCommandLine( char *lpCmdLine );

int    ParseNum( const char *str );

char*  COM_Parse( char *data );
char*  Get_COM_Token();

extern char com_token[1024];
extern qboolean com_eof;

#define MAX_NUM_ARGVS   32
extern int argc;
extern char            *argv[MAX_NUM_ARGVS];

//
// system functions
//
// TTimo NOTE: WINAPI funcs can be accessed by plugins
void    Sys_UpdateStatusBar( void );
void WINAPI Sys_UpdateWindows( int bits );
void    Sys_Beep( void );
void    Sys_ClearPrintf( void );
double  Sys_DoubleTime( void );
void    Sys_GetCursorPos( int *x, int *y );
void    Sys_SetCursorPos( int x, int y );
void    Sys_SetTitle( const char *text );
void WINAPI Sys_BeginWait( void );
void WINAPI Sys_EndWait( void );
void    Sys_Status( const char *psz, int part );
bool    Sys_AltDown();
bool    Sys_ShiftDown();
// will open/close/check the log file based on the following globals:
// g_PrefsDlg.m_bLogConsole g_qeglobals.hLogFile
void    Sys_LogFile( void );

extern qboolean verbose;
#include "qsysprintf.h"

// NOTE TTimo I split out the GUI-depependant stuff from QEGlobals_t into a seperate struct
typedef struct
{
	// GL widget of the camera view
	// all textures are binded in this context and shared with the others
	GtkWidget *d_glBase;

	GtkWidget *d_main_window; // d_hwndMain
	GtkWidget *d_edit;      // d_hwndEdit
	GtkWidget *d_entity;    // d_hwndEntity
	GtkWidget *d_camera;    // d_hwndCamera;
	GtkWidget *d_texture;   // d_hwndTexture;
	GtkWidget *d_texture_scroll;
	GtkWidget *d_z;         // d_hwndZ;

} QEGlobals_GUI_t;

// usefull portability stuff
//++timo move them somewhere
bool DoesFileExist( const char* pBuff, long& lSize );

char *copystring( char *s );
char *ExpandReletivePath( char *p );

#include "xmlstuff.h"
#include "points.h"

//
// drag.c
//
void Drag_Begin( int x, int y, int buttons, vec3_t xaxis, vec3_t yaxis, vec3_t origin, vec3_t dir, bool sf_camera = false );
void Drag_MouseMoved( int x, int y, int buttons );
void Drag_MouseUp( int nButtons = 0 );

//
// csg.c
//
#define CSG_HOLLOW_MODE_OVERLAP (0)
#define CSG_HOLLOW_MODE_TOUCH (1)
void CSG_MakeHollow( void );
void CSG_MakeHollowMode( int mode );
void CSG_Subtract( void );
void CSG_Merge( void );

//
// vertsel.c
//

void SetupVertexSelection( void );
void SelectEdgeByRay( vec3_t org, vec3_t dir );
void SelectVertexByRay( vec3_t org, vec3_t dir );

void ConnectEntities( void );

extern int update_bits;

extern int screen_width;
extern int screen_height;

char *TranslateString( char *buf );

//
// linux_qe3.cc
//
//void OpenDialog ();
//void SaveAsDialog (bool bRegion);
void ProjectDialog( void );
void MRU_Load();
void MRU_Save();
void MRU_AddWidget( GtkWidget *widget, int pos );
void MRU_AddFile( const char *str );
void MRU_Activate( int index );


void FillBSPMenu( void );
void ClearGSList( GSList *lst );
void FillTextureList( GSList** pArray );
void FillTextureMenu( GSList *texdirs );
void FillTextureDirListWidget( GSList *texdirs );

// profile functions - kind of utility lib
// they are kind of dumb, they expect to get the path to the .ini file or to the prefs directory when called
// load_buffer and save_buffer expect the path only, theyll build a $(pszName).bin file
bool WINAPI profile_save_int( const char *filename, const char *section, const char *key, int value );
bool WINAPI profile_save_float( const char *filename, const char *section, const char *key, float value );
bool WINAPI profile_save_string( const char *filename, const char *section, const char *key, const char *value );
bool profile_save_buffer( const char *rc_path, const char *pszName, void *pvBuf, guint32 lSize );
bool profile_load_buffer( const char *rc_path, const char *pszName, void *pvBuf, guint32 *plSize );
int WINAPI profile_load_int( const char *filename, const char *section, const char *key, int default_value );
float WINAPI profile_load_float( const char *filename, const char *section, const char *key, float default_value );
char* WINAPI profile_load_string( const char *filename, const char *section, const char *key, const char *default_value );
// used in the command map code
bool read_var( const char *filename, const char *section, const char *key, char *value );

//
// entityw.c
//
void FillClassList( void );
bool UpdateEntitySel( eclass_t *pec );
void SetInspectorMode( int iType );
void SetSpawnFlags( void );
void GetSpawnFlags( void );
void SetKeyValuePairs( bool bClearMD3 = false );
extern void BuildGammaTable( float g );
bool GetSelectAllCriteria( CString &strKey, CString &strVal );

// linux_dlg.c

typedef enum {
	BEVEL = 0,
	ENDCAP,
	IBEVEL,
	IENDCAP
} CapDialog;

int DoCapDlg( int *type, bool *b_GroupResul );
int DoBSInputDlg( const char *fields[5], float values[5] );
int DoTextureLayout( float *fx, float *fy );
char* DoNameDlg( const char* title );
char* DoNewProjectDlg();
/*
   text editor, open filename at given line
   opening at line works only for win32 / editpad and builtin Gtk editor

   we only allow one instance of the Gtk editor widget opened at a given time
   if we get called with an existing instance, switch to new file ..
 */
void DoTextEditor( const char* filename, int cursorpos );
int DoLightIntensityDlg( int *intensity );

void DoMapInfo();
void DoEntityList();
void DoGamma();
void DoFind();
void DoRotateDlg();
void DoSides( bool bCone = false, bool bSphere = false, bool bTorus = false );
void DoAbout();
void DoSnapTToGrid( float hscale = 0.0f, float vscale = 0.0f );
void DoSurface();
void ToggleSurface(); // will show/hide depending on the current state
void DoNewPatchDlg();
void DoThickenDlg();
void DoCommandListDlg();
void DoScaleDlg();;
void DoTextureListDlg();
void DoScriptsDlg();

// QE function declarations
void     QE_CheckAutoSave( void );
void WINAPI QE_ConvertDOSToUnixName( char *dst, const char *src );
void     QE_CountBrushesAndUpdateStatusBar( void );
void WINAPI QE_CheckOpenGLForErrors( void );
void     QE_ExpandBspString( char *bspaction, GPtrArray & out, char *mapname );
// initialise the VFS from current project settings
void     QE_InitVFS();
// do all initialisations that should happen after a project is load (during startup or project change)
void     QE_Init( void );
qboolean QE_KeyDown( int key, int nFlags = 0 );
// does some sanity checks on the project entity, such as removing ending filename seperators from paths
// (this usually gets propagated to the actual project file since most of the time we save right after calling the check)
void     QE_CheckProjectEntity();
// peek in a project file to retrieve the version key
int		QE_GetTemplateVersionForProject( const char * projectfile );
// this will load a new project entity in memory, and potentially process it from a template
// NOTE TTimo calling QE_LoadProject won't take care of the various initialisation that are performed depending on the project settings
//   you should then call QE_Init for that
#define PROJECT_TEMPLATE_NAME "default_project.proj"
#define PROJECT_USER_NAME "user_project.proj"
#define PROJECT_FILETYPE "proj"
qboolean QE_LoadProject( const char *projectfile );
qboolean QE_SingleBrush( bool bQuiet = false );


// sys stuff
void Sys_MarkMapModified( void );

#if 0 // no longer used

// QE Win32 function declarations
#ifdef _WIN32
int WINAPI QEW_SetupPixelFormat( HDC hDC, qboolean zbuffer );
void QEW_StopGL( HWND hWnd, HGLRC hGLRC, HDC hDC );
#endif

#endif

// extern declarations
extern QEGlobals_t g_qeglobals;
extern QEGlobals_GUI_t g_qeglobals_gui;

qboolean IsBrushSelected( brush_t* bSel );

// curve brushes

void Curve_MakeCurvedBrush( qboolean negative, qboolean top, qboolean bottom,
							qboolean s1, qboolean s2, qboolean s3, qboolean s4 );

void Curve_Invert( void );

void Curve_AddFakePlanes( brush_t *B );
void Curve_StripFakePlanes( brush_t *B );
void Curve_BuildPoints( brush_t *b );
void Curve_XYDraw( brush_t *b );
void Curve_CameraDraw( brush_t *b );

void Curve_WriteFile( char *name );


// patch stuff
patchMesh_t *Patch_Alloc();
patchMesh_t* MakeNewPatch();
brush_t* AddBrushForPatch( patchMesh_t *pm, bool bLinkToWorld = true );
brush_t* Patch_GenericMesh( int nWidth, int nHeight, int nOrientation = 2, bool bDeleteSource = true, bool bOverride = false );
//void Patch_ReadFile (char *name);
//void Patch_WriteFile (char *name);
void Patch_BuildPoints( brush_t *b );
void Patch_Move( patchMesh_t *p, const vec3_t vMove, bool bRebuild = false );
//++timo had to add a default value for bSnap (see Patch_ApplyMatrix call from Select_ApplyMatrix in select.cpp)
void Patch_ApplyMatrix( patchMesh_t *p, const vec3_t vOrigin, const vec3_t vMatrix[3], bool bSnap = false );
void Patch_EditPatch();
void Patch_Deselect();
void Patch_Deselect( patchMesh_t *p );
void Patch_Delete( patchMesh_t *p );
int  Patch_MemorySize( patchMesh_t *p );
void Patch_Select( patchMesh_t *p );
void Patch_Scale( patchMesh_t *p, const vec3_t vOrigin, const vec3_t vAmt, bool bRebuilt = true );
void Patch_Cleanup();
void Patch_SetView( int n );
void Patch_SetTexture( patchMesh_t *p, texdef_t *tex_def, IPluginTexdef* pPlugTexdef = NULL );
void Patch_BrushToMesh( bool bCone = false, bool bBevel = false, bool bEndcap = false, bool bSquare = false, int nHeight = 3 );
bool Patch_DragScale( patchMesh_t *p, vec3_t vAmt, vec3_t vMove );
//void Patch_ReadBuffer(char* pBuff, bool bSelect = false);
//void Patch_WriteFile (MemStream* pMemFile);
void Patch_UpdateSelected( vec3_t vMove );
//brush_t* Patch_Parse(bool bOld);
//void Patch_Write (patchMesh_t *p, FILE *f);
//void Patch_Write (patchMesh_t *p, MemStream *file);
//void Patch_AdjustColumns(patchMesh_t *p, int nCols);
//void Patch_AdjustRows(patchMesh_t *p, int nRows);
void Patch_AdjustSelected( bool bInsert, bool bColumn, bool bFlag );
patchMesh_t* Patch_Duplicate( patchMesh_t *pFrom );
void Patch_RotateTexture( patchMesh_t *p, float fAngle );
void Patch_ScaleTexture( patchMesh_t *p, float fx, float fy, bool bFixup = true );
// shift of some pixel amount
void Patch_ShiftTexture( patchMesh_t *p, float fx, float fy );
// shift of ST increments
void Patch_ShiftTextureST( patchMesh_t *p, float fx, float fy );
void Patch_DrawCam( patchMesh_t *p );
void Patch_DrawXY( patchMesh_t *p );
void Patch_InsertColumn( patchMesh_t *p, bool bAdd );
void Patch_InsertRow( patchMesh_t *p, bool bAdd );
void Patch_RemoveRow( patchMesh_t *p, bool bFirst );
void Patch_RemoveColumn( patchMesh_t *p, bool bFirst );
void Patch_ToggleInverted();
void Patch_Restore( patchMesh_t *p );
void Patch_Save( patchMesh_t *p );
void Patch_SetTextureInfo( texdef_t* pt );
void Patch_NaturalTexturing();
void Patch_ResetTexturing( float fx, float fy );
void Patch_FitTexturing();
void Patch_BendToggle();
//void Patch_StartInsDel();
void Patch_BendHandleTAB();
void Patch_BendHandleENTER();
void Patch_SelectBendNormal();
void Patch_SelectBendAxis();
bool OnlyPatchesSelected();
bool AnyPatchesSelected();
patchMesh_t* SinglePatchSelected();
void Patch_CapCurrent();
void Patch_DisperseRows();
void Patch_DisperseIntermediateRows();
void Patch_DisperseIntermediateColumns();
void Patch_CycleCapSelected();
void Patch_NaturalizeSelected( bool bCap = false ); //, bool bCycleCap = false);
void Patch_SelectAreaPoints( bool bMulti );
void Patch_InvertTexture( bool bY );
void patchInvert( patchMesh_t *p );
//void Patch_InsDelToggle();
//void Patch_InsDelHandleTAB();
//void Patch_InsDelHandleENTER();
void Patch_SetOverlays();
void Patch_ClearOverlays();
void Patch_Thicken( int nAmount, bool bSeam, qboolean bGroupResult );
void Patch_Transpose();
void Patch_Freeze();
void Patch_UnFreeze( bool bAll );
const char* Patch_GetTextureName();
void Patch_FindReplaceTexture( brush_t *pb, const char *pFind, const char *pReplace, bool bForce );
void Patch_SnapToGrid( patchMesh_t *p );
extern bool g_bPatchShowBounds;
extern bool g_bPatchWireFrame;
extern bool g_bPatchWeld;
extern bool g_bPatchDrillDown;
//extern bool g_bPatchInsertMode;
extern bool g_bPatchBendMode;
extern vec3_t g_vBendOrigin;
//void Patch_FromTriangle(vec5_t vx, vec5_t vy, vec5_t vz);
const char* Patch_GetKeyValue( patchMesh_t *p, const char *pKey );
void Patch_SetEpair( patchMesh_t *p, const char *pKey, const char *pValue );
void Patch_LODMatchAll();
void Patch_CalcBounds( patchMesh_t *p, vec3_t& vMin, vec3_t& vMax );




// group stuff
// group_t are loaded / saved through "group_info" entities
// they hold epairs for group settings and additionnal access info (tree nodes)
typedef struct group_s
{
	struct group_s *next;
	epair_t *epairs;
#if 0 //! Deprecated in gtk 2.x.
	GtkCTreeNode *itemOwner;
#endif
} group_t;

// NOTES: grouping only enabled in brush primitives mode
// grouping works by naming brushes and setting display properties
// the group hierarchy is not related with the map hierarchy (entity list, brushes etc.)
// brushes with no group are under the "world" node (default for all brushes)
// void Group_GetListFromWorld(CStringArray *pArray);
void Group_RemoveListFromWorld();
// void Group_SetListToWorld(CStringArray *pArray);
// void Group_BuildTree(CTreeCtrl *pTree);
// void Group_DecomposeTree(CTreeCtrl *pTree);
// save group_t as "classname" "group_info" things
void Group_Save( FILE *f );
// clean the brushes ownerItem, clean the treeview and rebuild everything
// is usually called when loading a new map, but may be called anytime
void Group_Init();
void Group_Add( entity_t *e );

// remove a brush from it's current group, will erase the "group" epair if any, and delete the tree control node
void Group_RemoveBrush( brush_t *b );
void Group_AddToWorld( brush_t *b );
// will remove brush of it's current group if any, and will add it wherever needed according to it's "group" key
void Group_AddToProperGroup( brush_t *b );
void Group_AddToSelected( brush_t *b );
// allocate a new group, set name
group_t* Group_Alloc( const char *name );
// we use entities to store information about the groups
// these entities are not linked into the world, and they have no brushes
// only loaded / saved in map file
group_t* Group_ForName( const char *name );

// TTimo
// new brush primitive stuff

#ifdef _DEBUG
//#define DBG_BP
#endif

// get the relative axes of the current texturing
void BrushPrimit_GetRelativeAxes( face_t *f, vec3_t vecS, vec3_t vecT );
// brush primitive stuff
void ComputeAxisBase( vec3_t normal, vec3_t texS, vec3_t texT );
void FaceToBrushPrimitFace( face_t *f );
void BrushPrimitFaceToFace( face_t *f );
void EmitBrushPrimitTextureCoordinates( face_t *, winding_t * );
// EmitTextureCoordinates, is old code used for brush to brush primitive conversion
void EmitTextureCoordinates( float *xyzst, qtexture_t *q, face_t *f );
void Face_TexdefFromTextureCoordinates( float *xyzst1, float *xyzst2, float *xyzst3, qtexture_t *q, face_t *f );
//void BrushPrimit_Parse(brush_t *);
// compute a fake shift scale rot representation from the texture matrix
void TexMatToFakeTexCoords( vec_t texMat[2][3], float shift[2], float *rot, float scale[2] );
void FakeTexCoordsToTexMat( float shift[2], float rot, float scale[2], vec_t texMat[2][3] );
void ConvertTexMatWithQTexture( vec_t texMat1[2][3], qtexture_t * qtex1, vec_t texMat2[2][3], qtexture_t * qtex2 );
// NOTE: this is a wrapper over the vec_t mat[2][3] version
void ConvertTexMatWithQTexture( brushprimit_texdef_t *texMat1, qtexture_t *qtex1, brushprimit_texdef_t *texMat2, qtexture_t *qtex2 );
// texture locking
void ShiftTextureGeometric_BrushPrimit( face_t *f, vec3_t delta );
void ShiftTextureRelative_BrushPrimit( face_t *f, float x, float y );
void RotateFaceTexture_BrushPrimit( face_t *f, int nAxis, float fDeg, vec3_t vOrigin );
// used in CCamWnd::ShiftTexture_BrushPrimit
void ComputeBest2DVector( vec3_t v, vec3_t X, vec3_t Y, int &x, int &y );
void Face_FitTexture_BrushPrimit( face_t *face, vec3_t minx, vec3_t maxs, int nHeight, int nWidth );
// lock textures on a random transformation
void ApplyMatrix_BrushPrimit( face_t * f, vec3_t matrix[3], vec3_t origin );
// low level functions .. put in mathlib?
#define BPMatCopy( a,b ) {b[0][0] = a[0][0]; b[0][1] = a[0][1]; b[0][2] = a[0][2]; b[1][0] = a[1][0]; b[1][1] = a[1][1]; b[1][2] = a[1][2]; }
// apply a scale transformation to the BP matrix
#define BPMatScale( m,sS,sT ) {m[0][0] *= sS; m[1][0] *= sS; m[0][1] *= sT; m[1][1] *= sT; }
// apply a translation transformation to a BP matrix
#define BPMatTranslate( m,s,t ) {m[0][2] += m[0][0] * s + m[0][1] * t; m[1][2] += m[1][0] * s + m[1][1] * t; }
// 2D homogeneous matrix product C = A*B
void BPMatMul( vec_t A[2][3], vec_t B[2][3], vec_t C[2][3] );
// apply a rotation (degrees)
void BPMatRotate( vec_t A[2][3], float theta );
#ifdef _DEBUG
void BPMatDump( vec_t A[2][3] );
#endif
// GL matrix product
void GLMatMul( vec_t M[4][4], vec_t A[4], vec_t B[4] );
qboolean IsBrushPrimitMode();
//
// eclass.cpp
//
#include "ieclass.h"

/*!
   \todo those are at the eclass manager level, but some documentation about what they do will be helpful
 */
extern qboolean parsing_single;
extern qboolean eclass_found;
extern eclass_t *eclass_e;
extern eclass_t *g_md3Cache;

/*!
   eclass manager API
 */
void Eclass_InsertAlphabetized( eclass_t *e );
eclass_t** Get_EClass_E();
void Set_Eclass_Found( qboolean );
qboolean Get_Parsing_Single();

// .def loading, builtin module
#include "eclass_def.h"
extern CSynapseBuiltinClientDef eclass_def;

/*!
   global table to .def entity class description
   this is a builtin module, even if we rely on fgd, we still use this one in cases such as entities not found etc.
 */
extern _EClassTable g_EClassDefTable;

/*!
   support for one additional/optional entity format
 */
extern bool g_bHaveEClassExt;
extern _EClassTable g_EClassExtTable;


#include "iplugin.h"
// for interfaces, we require main plugin header included
#include "qerplugin.h"

//
// SurfaceDlg.cpp and surface properties plugin
//
//++timo some patch in/out stuff is in there, needs to be moved out in a dedicated interface
#include "isurfaceplugin.h"
#include "surfaceplugin.h"
void WINAPI Patch_Rebuild( patchMesh_t *p );
#include "isurfaceplugin.h"
extern _QERPlugSurfaceTable g_SurfaceTable;
void SurfaceDlgFitAll();

//
// OpenGL interface
//
#include "igl.h"

GtkWidget * WINAPI QERApp_GetQeglobalsGLWidget();
void WINAPI QERApp_HookGL2DWindow( IGL2DWindow* pGLW );
void WINAPI QERApp_UnHookGL2DWindow( IGL2DWindow* pGLW );
void WINAPI QERApp_HookGL3DWindow( IGL3DWindow* pGLW );
void WINAPI QERApp_UnHookGL3DWindow( IGL3DWindow* pGLW );
void Draw2DPluginEntities( VIEWTYPE vt );
void Draw3DPluginEntities();

//
// IShaders interface
//
#define USE_SHADERSTABLE_DEFINE
#include "ishaders.h"
extern _QERShadersTable g_ShadersTable;

//
// ISelectedFace interface
//
#include "iselectedface.h"
int WINAPI QERApp_GetSelectedFaceCount();
// NOTE: it's the brush corresponding to the selected face below!
brush_t* WINAPI QERApp_GetSelectedFaceBrush( int iface );
face_t* WINAPI QERApp_GetSelectedFace( int iface );
int WINAPI QERApp_GetFaceInfo( int iface, _QERFaceData *pFaceData, winding_t *pWinding );
int WINAPI QERApp_SetFaceInfo( int iface, _QERFaceData *pFaceData );
int WINAPI QERApp_ISelectedFace_GetTextureNumber( int iface );
void WINAPI QERApp_GetTextureSize( int iface, int Size[2] );

//
// IEpairs interface
//
//#include "iepairs.h"
//#include "epairswrapper.h"

//
// IImage interface
//
#include "iimage.h"

//
// IFileSystem interface
//
#define USE_VFSTABLE_DEFINE
#include "ifilesystem.h"

extern _QERFileSystemTable g_FileSystemTable;

//
// TexWnd.cpp
//
extern qboolean g_bShowAllShaders;

//
// texwindow.cpp
//
#define     MAX_TEXTUREDIRS 1024

extern CPtrArray g_lstSkinCache;
qtexture_t *QERApp_LoadTextureRGBA( unsigned char* pPixels, int nWidth, int nHeight );

//
// IScripLib interface
// GetToken, UnGetToken, etc.
#include "iscriplib.h"
extern FILE *g_File;
void WINAPI QERApp_MapPrintf_FILE( char *text, ... );

//
// ISurfacePlugin interface
//
void QERApp_GetTwoSelectedPatch( patchMesh_t **p1, patchMesh_t **p2 );

//
// IBSPFrontend interface
//
#include "ibspfrontend.h"
extern _QERPlugBSPFrontendTable g_BSPFrontendTable;
extern GSList *g_BSPFrontendCommands;

//
// IToolbar
//
#include "itoolbar.h"

//
// IMessaging interface
#include "iui.h"
#include "iui_gtk.h"
#include "ui.h"
IWindow * WINAPI QERApp_CreateGLWindow();
void WINAPI QERApp_HookWindow( IWindowListener* pListen );
void WINAPI QERApp_UnHookWindow( IWindowListener* pListen );
IXYWndWrapper* WINAPI QERApp_GetXYWndWrapper();
void WINAPI QERApp_HookListener( IListener* pListen, int Msg );
int WINAPI QERApp_UnHookListener( IListener* pListen );
void DispatchRadiantMsg( int Msg );
// dispatch for IWindowListener entities
void DispatchOnMouseMove( guint32 nFlags, int x, int y );
bool DispatchOnLButtonDown( guint32 nFlags, int x, int y );
bool DispatchOnLButtonUp( guint32 nFlags, int x, int y );

//
// IData interface
//
#include "idata.h"

//
// ICamera interface
//
#include "icamera.h"

// Some declarations that were in stdafx.h

// main.cpp
extern gint try_destroy_splash( gpointer );

#include "mainframe.h"
#include "preferences.h"
#include "findtexturedialog.h"
#include "surfacedialog.h"
#include "patchdialog.h"

class MainFrame;
class ClipPoint;

extern MainFrame* g_pParentWnd;
extern CString g_strAppPath;
extern CString g_strDTDPath;
extern CString g_pidFile;
extern CString g_pidGameFile;
extern CString g_strBitmapsPath;
extern CString g_strPluginsDir;
extern CString g_strModulesDir;

extern CGameDescription *g_pGameDescription;
extern CString g_strGameToolsPath;
extern CString g_strExecutablesPath;

extern CString g_strTempPath;
extern PrefsDlg& g_PrefsDlg;
extern FindTextureDialog& g_dlgFind;
extern SurfaceDlg g_dlgSurface;
extern PatchDialog g_PatchDialog;

extern int g_bIgnoreCommands;

void HideInfoDialog();
void ShowInfoDialog( const char* pText );

// externs
//extern void HandleCommand (GtkWidget *widget, gpointer data, bool keydown);
extern gint HandleCommand( GtkWidget *widget, gpointer data );
extern void AddSlash( CString& );
extern void DLLBuildDone();
extern void CleanUpEntities();
extern void FindReplace( CString& strContents, const char* pTag, const char* pValue );
extern void CheckBspProcess();
extern void QE_CountBrushesAndUpdateStatusBar();
extern void QE_CheckAutoSave();
extern qtexture_t   *current_texture;
extern void SaveWithRegion( char *name ); // save the current map, sets the map name in the name buffer (deals with regioning)
extern void RunBsp( char *command );
extern void Map_Snapshot();
extern void WXY_Print();
extern void AddProp( void );
extern qboolean DoColor( int iIndex );
extern entity_t *edit_entity;
extern int inspector_mode;
extern bool g_bRotateMode;
extern bool g_bClipMode;
extern bool g_bScaleMode;
extern int g_nScaleHow;
extern bool g_bPathMode;
extern void RunScript( char* pBuffer );
extern bool ExtractPath_and_Filename( const char* pPath, CString& strPath, CString& strFilename );
extern void Select_Scale( float x, float y, float z );
extern void Select_RotateTexture( int amt );
extern void Select_ScaleTexture( float x, float y );
extern void Select_ShiftTexture( int x, int y );
extern void FindReplaceTextures( const char* pFind, const char* pReplace, bool bSelected, bool bForce, bool bSelectMatchingFaces );
/*!
   \fn DoProjectSettings shows the dialog for per-game configurable settings by the user
   typically this sets up things like mod editing configuration
   those a per-project, not the same thing as preferences which are global to the game conf
   this is still being worked on, as we have several issues with how things are configured
   we also have a number of behaviours defined in the .game, which can't be edited graphically
   we dump those properties to the console when the project settings dialog shows up
 */
extern void DoProjectSettings();
extern qboolean region_active;
extern void Brush_Print( brush_t* b );
extern void Texture_ShowStartupShaders();
extern void Map_ImportFile( char *filename );
extern void Map_SaveSelected( char* pFilename );
extern void UpdateSurfaceDialog();
extern void Select_GetTrueMid( vec3_t mid );
extern bool g_bSwitch;
extern brush_t g_brFrontSplits;
extern brush_t g_brBackSplits;
extern ClipPoint g_Clip1;
extern ClipPoint g_Clip2;
extern brush_t* g_pSplitList;
extern ClipPoint g_PathPoints[256];
extern void AcquirePath( int nCount, PFNPathCallback* pFunc );
extern bool g_bScreenUpdates;
extern SCommandInfo g_Commands[];
extern int g_nCommandCount;
extern SKeyInfo g_Keys[];
extern int g_nKeyCount;
extern int inspector_mode;
extern char *bsp_commands[256];
extern void RunScriptByName( char*, bool );
extern void DoNewColor( int* i1, int* i2, int* i3 );
extern void UpdateSurfaceDialog();
extern void CSG_SplitBrushByFace( brush_t *in, face_t *f, brush_t **front, brush_t **back );
extern z_t z;
extern void Select_Scale( float x, float y, float z );
extern void TextureAxisFromPlane( plane_t *pln, vec3_t xv, vec3_t yv );
extern qboolean QE_SaveProject( const char* pProjectFile );
extern void RunTools( char* pCommandLine, GtkWidget* hwnd, const char* pPAKFile );
extern void Clamp( float& f, int nClamp );
extern void MemFile_fprintf( MemStream* pMemFile, const char* pText, ... );
extern qboolean ConfirmModified( void );
extern void DoPatchInspector();
extern void TogglePatchInspector();
void UpdatePatchInspector();
extern int BuildShortPathName( const char* pPath, char* pBuffer, int nBufferLen );
extern int g_nBrushId;

// defined in gtkdlgs.cpp, we might want to move declaration and implementatin with other Select_ stuff..
// NOTE: there's also a Select_Brush(brush_t *b) function.. unrelated
extern void SelectBrush( int entitynum, int brushnum );

// bp_dlg.cpp
// ret: 0 = abort, 1 = load and convert, 2 = changed project settings, load and don't convert
// the user might decide to switch the BP mode in project settings
// status: 0 = loading regular, got conflict 1 = loading BP, got conflict
extern int BP_MessageBox( int status );

// main.cpp
extern gint try_destroy_splash( gpointer );

// SPoG
// targetname.cpp
void Entity_Connect( entity_t *e1, entity_t *e2 );
int GetUniqueTargetId( int iHint );

// xywindow.cpp
void CreateEntityFromName( const char* name, const vec3_t origin );

// eclass.cpp
/*!
   \brief initialization of the eclass manager
   general guidelines about eclass.cpp implementation:
   - we don't support unlimited number of eclass file formats together
   currently limited to two
   support for .def is builtin to the core, but you may choose not to activate it
   - search and load of the files:
   the manager is in charge of scanning for eclass definition files to load
   there is a general configuration setting for games which use a different set of
    entities between single player mode and multiplayer mode (TODO: the code doing
    this needs to be abstracted some more)
   - duplicate files / multiple files:
   if two files with the same name exist (for instance in the basegame, and in fs_game)
   then only the first one found in the scan order is loaded
   if several files are found, they are all loaded
   this allows mods to either replace or extend the list of eclass
   - if eclass_singleload prop is used in the .game, then there is no multiple files check
   the first file found is loaded, and there is no further search for the given extension
   (this was an addition for HL support)
 */
void Eclass_Init();
eclass_t *Eclass_ForName( const char *name, qboolean has_brushes );
eclass_t * EClass_Create( const char *name, float col1, float col2, float col3, const vec3_t *mins, const vec3_t *maxs, const char *comments );

#endif // _QE3_H_
