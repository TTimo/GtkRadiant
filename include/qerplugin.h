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

// QERadiant PlugIns
//
//

#ifndef __QERPLUGIN_H__
#define __QERPLUGIN_H__

/*!
   \todo this header is intended to be turned into a header for the core editor functionality
   some portability related code should be moved to synapse (such as the GUID stuff)
 */

#include <stdio.h>
#include <string.h>
// TTimo
// ideally the plugin API would be UI toolkit independent, but removing the dependency with GLib seems tricky right now..
#include <glib.h>
#include "qertypes.h"

// FIXME TTimo:
// GUID declaration here should be trashed, it is in synapse.h
#ifdef _WIN32
#include <wtypes.h>
#endif

#define QER_MAX_NAMELEN 1024

#ifndef _WIN32
#include "misc_def.h"
#endif

// the editor will look for plugins in two places, the plugins path
// under the application path, and the path under the basepath as defined
// in the project (.qe4) file.
//
// you can drop any number of new texture, model format DLL's in the standard plugin path
// but only one plugin that overrides map loading/saving, surface dialog, surface flags, etc..
// should be used at one time.. if multiples are loaded then the last one loaded will be the
// active one
//
// type of services the plugin supplies, pass any combo of these flags
// it is assumed the plugin will have a matching function as defined below
// to correlate to the implied functionality
//

#define RADIANT_MAJOR "radiant"

// basics
#define QERPLUG_INIT "QERPlug_Init"
#define QERPLUG_GETNAME "QERPlug_GetName"
#define QERPLUG_GETCOMMANDLIST "QERPlug_GetCommandList"
#define QERPLUG_DISPATCH "QERPlug_Dispatch"
#define QERPLUG_GETFUNCTABLE "QERPlug_GetFuncTable"

// game stuff
#define QERPLUG_GETTEXTUREINFO "QERPlug_GetTextureInfo"   // gets a texture info structure
#define QERPLUG_LOADTEXTURE    "QERPlug_LoadTexture"      // loads a texture, will return an RGBA structure
                                                          // and any surface flags/contents for it
#define QERPLUG_GETSURFACEFLAGS "QERPlug_GetSurfaceFlags" // gets a list of surface/content flag names from a plugin

struct _QERTextureInfo
{
	char m_TextureExtension[QER_MAX_NAMELEN]; // the extension these textures have
	qboolean m_bHiColor;  // if textures are NOT high color, the default
	// palette (as described inthe qe4 file will be used for gamma correction)
	// if they are high color, gamma and shading are computed on the fly
	// based on the rgba data
	//--bool m_bIsShader;   // will probably do q3 shaders this way when i merge
	qboolean m_bWadStyle; // if this is true, the plugin will be presented with the texture path
	// defined in the .qe4 file and is expected to preload all the textures
	qboolean m_bHalfLife; // causes brushes to be saved/parsed without the surface contents/flags/value
};

struct _QERTextureLoad    // returned by a plugin
{
	_QERTextureLoad(){
		memset( reinterpret_cast<void*>( this ), 0, sizeof( _QERTextureLoad ) );
	};

	~_QERTextureLoad(){
		delete []m_pRGBA;
		delete []m_pName;
	};

	void makeSpace( int nSize ){
		m_pRGBA = new unsigned char[nSize + 1];
	};

	void setName( const char* p ){
		m_pName = new char[strlen( p ) + 1];
		strcpy( m_pName, p );
	};


	unsigned char *m_pRGBA; // rgba data (alpha channel is supported and drawn appropriately)
	int m_nWidth;         // width
	int m_nHeight;        // height
	int m_nContents;      // default contents
	int m_nFlags;         // "" flags
	int m_nValue;         // "" value
	char *m_pName;        // name to be referenced in map, build tools, etc.
};

struct _QERModelInfo
{
	char m_ModelExtension[QER_MAX_NAMELEN];
	bool m_bSkinned;
	bool m_bMultipart;
};

struct _QERModelLoad
{
	// vertex and skin data
};


//=========================================
// plugin functions

typedef char* ( WINAPI * PFN_QERPLUG_GETFUNCTABLE )();

// v1.5
//
// Texture loading
// returns a ptr to _QERTextureInfo
typedef void* ( WINAPI * PFN_QERPLUG_GETTEXTUREINFO )();
//
// loads a texture by calling the texture load func in the editor (defined below)
// transparency (for water, fog, lava, etc.. ) can be emulated in the editor
// by passing in appropriate alpha data or by setting the appropriate surface flags
// expected by q2 (which the editor will use.. )
typedef void ( WINAPI * PFN_QERPLUG_LOADTEXTURE )( const char* pFilename );

// v1.6
typedef void* ( WINAPI * PFN_QERPLUG_GETSURFACEFLAGS )();

// v1.7
// if exists in plugin, gets called between INIT and GETCOMMANDLIST
// the plugin can register the EClasses he wants to handle
//++timo TODO: this has got to move into the table, and be requested by QERPlug_RequestInterface
//++timo FIXME: the LPVOID parameter must be casted to an IEpair interface
#define QERPLUG_REGISTERPLUGINENTITIES "QERPlug_RegisterPluginEntities"
typedef void ( WINAPI * PFN_QERPLUG_REGISTERPLUGINENTITIES )( void* );

// if exists in plugin, gets called between INIT and GETCOMMANDLIST
// the plugin can Init all it needs for surface properties
#define QERPLUG_INITSURFACEPROPERTIES "QERPlug_InitSurfaceProperties"
typedef void ( WINAPI * PFN_QERPLUG_INITSURFACEPROPERTIES )();

// if Radiant needs to use a particular set of commands, it can request the plugin to fill a func table
// this is similar to PFN_QERAPP_REQUESTINTERFACE
#define QERPLUG_REQUESTINTERFACE "QERPlug_RequestInterface"
typedef int ( WINAPI * PFN_QERPLUG_REQUESTINTERFACE )( REFGUID refGUID, void* pInterface, const char *version_name );

// Load an image file
typedef void ( *PFN_QERAPP_LOADIMAGE )( const char *name, unsigned char **pic, int *width, int *height );

// TTimo FIXME: the logic for this is in synapse now

// MODULES specific:
// if it exports this entry point, will be considered as a module
// a module is a plugin that provides some REQUIRED interfaces to Radiant, such as the shader module
// Radiant will call QERPLUG_LISTINTERFACES to get a list of the interfaces a given plugin implements
// then it will call PFN_QERPLUG_REQUESTINTERFACE to actually get them

// following leo's code .. looks ok to use a string to identify the various versions of a same interface
// obviously it would be handy to have the same string naming for the interfaces.
// best way would be to have the names come in when you list the interfaces
// NOTE: we might have a problem with the order in which the interfaces are filled in
//   there's some kind of dependency graph, the shader module expects to find the VFS ready etc.
typedef struct moduleentry_s {
	const GUID *interface_GUID;
	const char* interface_name;
	const char* version_name;
} moduleentry_t;

#define QERPLUG_LISTINTERFACES "QERPlug_ListInterfaces"
#define MAX_QERPLUG_INTERFACES 10
typedef int ( WINAPI * PFN_QERPLUG_LISTINTERFACES )( moduleentry_t table[MAX_QERPLUG_INTERFACES] );

// ========================================
// GTK+ helper functions

// NOTE: parent can be NULL in all functions but it's best to set them

// simple Message Box, see above for the 'type' flags
// toolkit-independent, cast parent ot a GtkWidget*
typedef gint ( WINAPI * PFN_QERAPP_MESSAGEBOX )( void *parent, const char* text,
												 const char* caption, guint32 type, const char *URL );

// file and directory selection functions return NULL if the user hits cancel
// or a gchar* string that must be g_free'd by the user
// - 'title' is the dialog title (can be NULL)
// - 'path' is used to set the initial directory (can be NULL)
// - 'pattern': the first pattern is for the win32 mode, then comes the Gtk pattern list, see Radiant source for samples
// TTimo 04/01/2001 toolkit-independant, cast parent to a GtkWidget*
typedef const gchar* ( *PFN_QERAPP_FILEDIALOG )( void *parent, gboolean open, const char* title,
												 const char* path, const char* pattern, const char *baseSubDir );
typedef gchar* ( WINAPI * PFN_QERAPP_DIRDIALOG )( void *parent, const char* title,
												  const char* path );

// return true if the user closed the dialog with 'Ok'
// 'color' is used to set the initial value and store the selected value
typedef bool ( WINAPI * PFN_QERAPP_COLORDIALOG )( void *parent, float *color,
												  const char* title );

// load a .bmp file and store the results in 'gdkpixmap' and 'mask'
// returns TRUE on success but even if it fails, it creates an empty pixmap
// NOTE: 'filename' is relative to <radiant_path>/plugins/bitmaps/
// TTimo 04/01/2001 toolkit-independant, cast gkpixmap to GdkPixmap and mask to GdkBitmap
typedef bool ( WINAPI * PFN_QERAPP_LOADBITMAP )( const char* filename, void **gdkpixmap, void **mask );

// ========================================
// read/write preferences file

// use this function to get the directory where the preferences file are stored
typedef const char* ( WINAPI * PFN_QERAPP_PROFILE_GETDIR )();

// 'filename' is the absolute path
typedef bool ( WINAPI * PFN_QERAPP_PROFILE_SAVEINT )( const char *filename, const char *section,
													  const char *key, int value );
typedef bool ( WINAPI * PFN_QERAPP_PROFILE_SAVESTR )( const char *filename, const char *section,
													  const char *key, const char *value );
typedef int ( WINAPI * PFN_QERAPP_PROFILE_LOADINT )( const char *filename, const char *section,
													 const char *key, int default_value );
typedef char* ( WINAPI * PFN_QERAPP_PROFILE_LOADSTR )( const char *filename, const char *section,
													   const char *key, const char *default_value );

//=========================================
// editor functions

// There are 3 potential brush handle lists
// 1. the list that contains brushes a plugin creates using CreateBrushHandle
// 2. the selected brush list (brushes the user has selected)
// 3. the active brush list (brushes in the map that are not selected)
//
// In general, the same things can be done to brush handles (face manip, delete brushhandle, etc.. ) in each
// list. There are a few exceptions.
// 1. You cannot commit a selected or active brush handle to the map. This is because it is already in the map.
// 2. You cannot bind brush handles from the selected or active brush list to an entity. As of v1.0 of the plugins
// the only way for a plugin to create entities is to create a brush handles (or a list of handles) and then bind
// them to an entity. This will commit the brush(s) and/or the entities to the map as well.
//
// To use the active or selected brush lists, you must first allocate them (which returns a count) and then
// release them when you are finish manipulating brushes in one of those lists.

// v1.80
#define QERAPP_GETDISPATCHPARAMS "QERApp_GetDispatchParams"

struct _QERPointData
{
	int m_nCount;
	vec3_t *m_pVectors;
};

struct _QERFaceData
{
	char m_TextureName[QER_MAX_NAMELEN];
	int m_nContents;
	int m_nFlags;
	int m_nValue;
	float m_fShift[2];
	float m_fRotate;
	float m_fScale[2];
	vec3_t m_v1, m_v2, m_v3;
	// brush primitive additions
	qboolean m_bBPrimit;
	brushprimit_texdef_t brushprimit_texdef;
};

typedef void ( WINAPI * PFN_QERAPP_CREATEBRUSH )( vec3_t vMin, vec3_t vMax );

typedef void* ( WINAPI * PFN_QERAPP_CREATEBRUSHHANDLE )();
typedef void ( WINAPI * PFN_QERAPP_DELETEBRUSHHANDLE )( void* pv );
typedef void ( WINAPI * PFN_QERAPP_COMMITBRUSHHANDLETOMAP )( void* pv );
typedef void ( WINAPI * PFN_QERAPP_ADDFACE )( void* pv, vec3_t v1, vec3_t v2, vec3_t v3 );

typedef void ( WINAPI * PFN_QERAPP_ADDFACEDATA )( void* pv, _QERFaceData *pData );
typedef int ( WINAPI * PFN_QERAPP_GETFACECOUNT )( void* pv );
typedef _QERFaceData* ( WINAPI * PFN_QERAPP_GETFACEDATA )( void* pv, int nFaceIndex );
typedef void ( WINAPI * PFN_QERAPP_SETFACEDATA )( void* pv, int nFaceIndex, _QERFaceData *pData );
typedef void ( WINAPI * PFN_QERAPP_DELETEFACE )( void* pv, int nFaceIndex );
typedef void ( WINAPI * PFN_QERAPP_TEXTUREBRUSH )( void* pv, char* pName );
typedef void ( WINAPI * PFN_QERAPP_BUILDBRUSH )( void* pv );        // PGM
typedef void ( WINAPI * PFN_QERAPP_SELECTBRUSH )( void* pv );       // PGM
typedef void ( WINAPI * PFN_QERAPP_DESELECTBRUSH )( void* pv );     // PGM
typedef void ( WINAPI * PFN_QERAPP_DESELECTALLBRUSHES )();            // PGM

typedef void ( WINAPI * PFN_QERAPP_DELETESELECTION )();
typedef void ( WINAPI * PFN_QERAPP_GETPOINTS )( int nMax, _QERPointData *pData, char* pMsg );

typedef int ( WINAPI * PFN_QERAPP_SELECTEDBRUSHCOUNT )();
typedef int ( WINAPI * PFN_QERAPP_ALLOCATESELECTEDBRUSHHANDLES )();
typedef void ( WINAPI * PFN_QERAPP_RELEASESELECTEDBRUSHHANDLES )();
typedef void* ( WINAPI * PFN_QERAPP_GETSELECTEDBRUSHHANDLE )( int nIndex );

typedef int ( WINAPI * PFN_QERAPP_ACTIVEBRUSHCOUNT )();
typedef int ( WINAPI * PFN_QERAPP_ALLOCATEACTIVEBRUSHHANDLES )();
typedef void ( WINAPI * PFN_QERAPP_RELEASEACTIVEBRUSHHANDLES )();
typedef void* ( WINAPI * PFN_QERAPP_GETACTIVEBRUSHHANDLE )( int nIndex );

typedef int ( WINAPI * PFN_QERAPP_TEXTURECOUNT )();
typedef char* ( WINAPI * PFN_QERAPP_GETTEXTURE )( int nIndex );
typedef char* ( WINAPI * PFN_QERAPP_GETCURRENTTEXTURE )();
typedef void ( WINAPI * PFN_QERAPP_SETCURRENTTEXTURE )( char* pName );

typedef void ( WINAPI * PFN_QERAPP_REGISTERMAPLOAD )( void* vp );
typedef void ( WINAPI * PFN_QERAPP_REGISTERMAPSAVE )( void* vp );

typedef int ( WINAPI * PFN_QERAPP_GETECLASSCOUNT )();
typedef char* ( WINAPI * PFN_QERAPP_GETECLASS )( int nIndex );

typedef void ( WINAPI * PFN_QERAPP_RESETPLUGINS )();
//--typedef int (WINAPI* PFN_QERAPP_GETENTITYCOUNT)();

/*!
   \fn LoadTextureRGBA
   \param pPixels is the raw RGBA pixel data (24bits, 8 bit depth)
   \param nWidth image width
   \param nHeight image height
   this will work from the RGBA data and create a GL texture (accessed through a GL bind number)
   it takes care of creating the mipmapping levels too
 */
typedef qtexture_t* ( *PFN_QERAPP_LOADTEXTURERGBA )( unsigned char* pPixels, int nWidth, int nHeight );

//--typedef LPCSTR (WINAPI* PFN_QERAPP_GETENTITY)(int nIndex);

// v1.70
typedef int ( WINAPI * PFN_QERAPP_GETENTITYCOUNT )();
typedef void* ( WINAPI * PFN_QERAPP_GETENTITYHANDLE )( int nIndex );
// FIXME: those two are fairly outdated, you get the epairs
//   but you don't have a clean epair read/write query
//   and you rely on the C structs directly, which might go away soon
//   ok now, stop using, it's bad for your karma (see iepairs.h instead)
typedef epair_t* ( WINAPI * PFN_QERAPP_ALLOCATEEPAIR )( const char*, const char* );
typedef int ( WINAPI * PFN_QERAPP_ALLOCATEENTITYBRUSHHANDLES )( void* vp );
typedef void ( WINAPI * PFN_QERAPP_RELEASEENTITYBRUSHHANDLES )();
typedef void* ( WINAPI * PFN_QERAPP_GETENTITYBRUSHHANDLE )( int nIndex );
typedef void* ( WINAPI * PFN_QERAPP_CREATEENTITYHANDLE )();
typedef void ( WINAPI * PFN_QERAPP_COMMITBRUSHHANDLETOENTITY )( void* vpBrush, void* vpEntity );
typedef void ( WINAPI * PFN_QERAPP_COMMITENTITYHANDLETOMAP )( void* vp );
typedef void ( WINAPI * PFN_QERAPP_SETSCREENUPDATE )( int bScreenUpdate );
// this one uses window flags defined in qertypes.h
typedef void ( WINAPI * PFN_QERAPP_SYSUPDATEWINDOWS )( int bits );
//++timo remove this one
typedef void ( WINAPI * PFN_QERAPP_BUILDBRUSH2 )( void* vp, int bConvert );

// v1.80
typedef void ( WINAPI * PFN_QERAPP_GETDISPATCHPARAMS )( vec3_t vMin, vec3_t vMax, bool *bSingleBrush );

typedef int ( WINAPI * PFN_QERAPP_REQUESTINTERFACE )( REFGUID, void* );
// use this one for errors, Radiant will stop after the "edit preferences" dialog
typedef void ( WINAPI * PFN_QERAPP_ERROR )( const char* pMsg, ... );
// use to gain read access to the project epairs
// FIXME: removed, accessed through QERPlug_RegisterPluginEntities with the IEpair interface
// typedef void (WINAPI* PFN_QERAPP_GETPROJECTEPAIR)(epair_t **);
// used to allocate and read a buffer
//++timo NOTE: perhaps this would need moving to some kind of dedicated interface
typedef int ( WINAPI * PFN_QERAPP_LOADFILE )( const char *pLocation, void ** buffer );
typedef char* ( WINAPI * PFN_QERAPP_EXPANDRELETIVEPATH )( char * );
typedef void ( WINAPI * PFN_QERAPP_QECONVERTDOSTOUNIXNAME )( char *dst, const char *src );
typedef int ( WINAPI * PFN_QERAPP_HASSHADER )( const char * );
typedef int ( WINAPI * PFN_QERAPP_TEXTURELOADSKIN )( char *pName, int *pnWidth, int *pnHeight );
// retrieves the path to the engine from the preferences dialog box
typedef const char* ( WINAPI * PFN_QERAPP_GETGAMEPATH )();
// retrieves full Radiant path
typedef const char* ( WINAPI * PFN_QERAPP_GETQERPATH )();
// retieves .game name of current active game
typedef const char* ( WINAPI * PFN_QERAPP_GETGAMEFILE )();

// patches in/out
// NOTE: this is a bit different from the brushes in/out, no LPVOID handles this time
// use int indexes instead
// if you call AllocateActivePatchHandles, you'll be playing with active patches
// AllocateSelectedPatcheHandles for selected stuff
// a call to CreatePatchHandle will move you to a seperate index table
typedef int ( WINAPI * PFN_QERAPP_ALLOCATEACTIVEPATCHHANDLES )();
typedef int ( WINAPI * PFN_QERAPP_ALLOCATESELECTEDPATCHHANDLES )();
typedef void ( WINAPI * PFN_QERAPP_RELEASEPATCHHANDLES )();
typedef patchMesh_t*    ( WINAPI * PFN_QERAPP_GETPATCHDATA )( int );
typedef patchMesh_t*    ( WINAPI * PFN_QERAPP_GETPATCHHANDLE )( int );
typedef void ( WINAPI * PFN_QERAPP_DELETEPATCH )( int );
typedef int ( WINAPI * PFN_QERAPP_CREATEPATCHHANDLE )();
// when commiting, only a few patchMesh_t members are relevant:
//  int	width, height;		// in control points, not patches
//  int   contents, flags, value, type;
//  drawVert_t ctrl[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT];
// once you have commited the index is still available, if the patch handle was allocated by you
//   then you can re-use the index to commit other patches .. otherwise you can change existing patches
// NOTE: the handle thing for plugin-allocated patches is a bit silly (nobody's perfect)
// TODO: change current behaviour to an index = 0 to tell Radiant to allocate, other indexes to existing patches
// patch is selected after a commit
// you can add an optional texture / shader name .. if NULL will use the current texture
typedef void ( WINAPI * PFN_QERAPP_COMMITPATCHHANDLETOMAP )( int, patchMesh_t* pMesh, char *texName );
typedef void ( WINAPI * PFN_QERAPP_COMMITPATCHHANDLETOENTITY )( int, patchMesh_t* pMesh, char *texName, void* vpEntity );

// console output
#define SYS_VRB 0 ///< verbose support (on/off)
#define SYS_STD 1 ///< standard print level - this is the default
#define SYS_WRN 2 ///< warnings
#define SYS_ERR 3 ///< error
#define SYS_NOCON 4 ///< no console, only print to the file (useful whenever Sys_Printf and output IS the problem)
typedef void ( WINAPI * PFN_QERAPP_SYSPRINTF )( const char *text, ... );
typedef void ( WINAPI * PFN_QERAPP_SYSFPRINTF )( int flag, const char *text, ... );

typedef void ( WINAPI * PFN_QERAPP_SYSBEGINWAIT )();
typedef void ( WINAPI * PFN_QERAPP_SYSENDWAIT )();

typedef void ( *PFN_QERAPP_SYSBEEP )();

typedef void ( *PFN_QERAPP_SYSSTATUS )( const char *psz, int part );

// core map functionality
typedef void ( *PFN_QERAPP_MAPNEW )();
typedef void ( *PFN_QERAPP_MAPFREE )();
typedef void ( *PFN_QERAPP_MAPBUILDBRUSHDATA )();
typedef qboolean ( *PFN_QERAPP_MAPISBRUSHFILTERED )( brush_t * );
typedef void ( *PFN_QERAPP_MAPSTARTPOSITION )();
typedef void ( *PFN_QERAPP_MAPREGIONOFF )();
//typedef void      (* PFN_QERAPP_SAVEASDIALOG)               (bool bRegion);
typedef void ( *PFN_QERAPP_SETBUILDWINDINGSNOTEXBUILD )( bool );
typedef void ( *PFN_QERAPP_POINTFILECLEAR )();

typedef void ( *PFN_QERAPP_SYSSETTITLE )( const char *text );

typedef void ( *PFN_QERAPP_CSGMAKEHOLLOW )();
typedef void ( *PFN_QERAPP_CSGMAKEHOLLOWMODE )( int mode );

typedef void ( *PFN_QERAPP_REGIONSPAWNPOINT )( FILE *f );

/*!
   access to a portable GetTickCount
 */
typedef unsigned long ( *PFN_QERAPP_GETTICKCOUNT )();

class IModelCache
{
public:
virtual ~IModelCache() { }
virtual entity_interfaces_t *GetByID( const char *id, const char* version ) = 0;
virtual void DeleteByID( const char *id, const char* version ) = 0;
virtual void RefreshAll() = 0;
};

typedef IModelCache* ( *PFN_GETMODELCACHE )();

class IFileTypeList
{
public:
virtual ~IFileTypeList() { }
virtual void addType( filetype_t type ) = 0;
};

class IFileTypeRegistry
{
public:
virtual ~IFileTypeRegistry() { }
virtual void addType( const char* key, filetype_t type ) = 0;
virtual void getTypeList( const char* key, IFileTypeList* typelist ) = 0;
private:
};

typedef IFileTypeRegistry* ( *PFN_GETFILETYPEREGISTRY )();

typedef const char* ( *PFN_QERAPP_READPROJECTKEY )( const char* key );

typedef char* ( *PFN_GETMAPFILENAME )();

typedef bfilter_t* ( *PFN_QERPLUG_FILTERADD )( int type, int bmask, const char *str, int exclude );

typedef void ( *PFN_QERPLUG_FILTERACTIVATE )( void );

// Plugins need to declare one of these and implement the getfunctable as described above
struct _QERFuncTable_1
{
	int m_nSize;
	PFN_QERAPP_CREATEBRUSH m_pfnCreateBrush;
	PFN_QERAPP_CREATEBRUSHHANDLE m_pfnCreateBrushHandle;
	PFN_QERAPP_DELETEBRUSHHANDLE m_pfnDeleteBrushHandle;
	PFN_QERAPP_COMMITBRUSHHANDLETOMAP m_pfnCommitBrushHandle;
	PFN_QERAPP_ADDFACE m_pfnAddFace;
	PFN_QERAPP_ADDFACEDATA m_pfnAddFaceData;
	PFN_QERAPP_GETFACEDATA m_pfnGetFaceData;
	PFN_QERAPP_GETFACECOUNT m_pfnGetFaceCount;
	PFN_QERAPP_SETFACEDATA m_pfnSetFaceData;
	PFN_QERAPP_DELETEFACE m_pfnDeleteFace;
	PFN_QERAPP_TEXTUREBRUSH m_pfnTextureBrush;
	PFN_QERAPP_BUILDBRUSH m_pfnBuildBrush;                          // PGM
	PFN_QERAPP_SELECTBRUSH m_pfnSelectBrush;                        // PGM
	PFN_QERAPP_DESELECTBRUSH m_pfnDeselectBrush;                    // PGM
	PFN_QERAPP_DESELECTALLBRUSHES m_pfnDeselectAllBrushes;          // PGM

	PFN_QERAPP_DELETESELECTION m_pfnDeleteSelection;
	PFN_QERAPP_GETPOINTS m_pfnGetPoints;

	PFN_QERAPP_SELECTEDBRUSHCOUNT m_pfnSelectedBrushCount;
	PFN_QERAPP_ALLOCATESELECTEDBRUSHHANDLES m_pfnAllocateSelectedBrushHandles;
	PFN_QERAPP_RELEASESELECTEDBRUSHHANDLES m_pfnReleaseSelectedBrushHandles;
	PFN_QERAPP_GETSELECTEDBRUSHHANDLE m_pfnGetSelectedBrushHandle;

	PFN_QERAPP_ACTIVEBRUSHCOUNT m_pfnActiveBrushCount;
	PFN_QERAPP_ALLOCATEACTIVEBRUSHHANDLES m_pfnAllocateActiveBrushHandles;
	PFN_QERAPP_RELEASEACTIVEBRUSHHANDLES m_pfnReleaseActiveBrushHandles;
	PFN_QERAPP_GETACTIVEBRUSHHANDLE m_pfnGetActiveBrushHandle;

	PFN_QERAPP_TEXTURECOUNT m_pfnTextureCount;
	PFN_QERAPP_GETTEXTURE m_pfnGetTexture;
	PFN_QERAPP_GETCURRENTTEXTURE m_pfnGetCurrentTexture;
	PFN_QERAPP_SETCURRENTTEXTURE m_pfnSetCurrentTexture;

	PFN_QERAPP_GETECLASSCOUNT m_pfnGetEClassCount;
	PFN_QERAPP_GETECLASS m_pfnGetEClass;
	PFN_QERAPP_RESETPLUGINS m_pfnResetPlugins;
	// v1.00 ends here
	// v1.50 starts here
	PFN_QERAPP_LOADTEXTURERGBA m_pfnLoadTextureRGBA;
	// v1.50 ends here
	// v1.70 starts here
	PFN_QERAPP_GETENTITYCOUNT m_pfnGetEntityCount;
	PFN_QERAPP_GETENTITYHANDLE m_pfnGetEntityHandle;
	PFN_QERAPP_ALLOCATEENTITYBRUSHHANDLES m_pfnAllocateEntityBrushHandles;
	PFN_QERAPP_RELEASEENTITYBRUSHHANDLES m_pfnReleaseEntityBrushHandles;
	PFN_QERAPP_GETENTITYBRUSHHANDLE m_pfnGetEntityBrushHandle;
	PFN_QERAPP_CREATEENTITYHANDLE m_pfnCreateEntityHandle;
	PFN_QERAPP_COMMITBRUSHHANDLETOENTITY m_pfnCommitBrushHandleToEntity;
	PFN_QERAPP_COMMITENTITYHANDLETOMAP m_pfnCommitEntityHandleToMap;
	PFN_QERAPP_ALLOCATEEPAIR m_pfnAllocateEpair;
	PFN_QERAPP_SETSCREENUPDATE m_pfnSetScreenUpdate;
	PFN_QERAPP_BUILDBRUSH2 m_pfnBuildBrush2;
	// v1.70 ends here
	// v1.80 starts here
	PFN_QERAPP_GETDISPATCHPARAMS m_pfnGetDispatchParams;

	// plugins can request additional interfaces
	PFN_QERAPP_REQUESTINTERFACE m_pfnRequestInterface;
	PFN_QERAPP_ERROR m_pfnError;
	// loading a file into a buffer
	PFN_QERAPP_LOADFILE m_pfnLoadFile;
	PFN_QERAPP_EXPANDRELETIVEPATH m_pfnExpandReletivePath;
	PFN_QERAPP_QECONVERTDOSTOUNIXNAME m_pfnQE_ConvertDOSToUnixName;
	PFN_QERAPP_HASSHADER m_pfnHasShader;
	PFN_QERAPP_TEXTURELOADSKIN m_pfnTexture_LoadSkin;
	PFN_QERAPP_GETGAMEPATH m_pfnGetGamePath;
	PFN_QERAPP_GETQERPATH m_pfnGetQERPath;
	PFN_QERAPP_GETGAMEFILE m_pfnGetGameFile;
	// patches in / out
	PFN_QERAPP_ALLOCATEACTIVEPATCHHANDLES m_pfnAllocateActivePatchHandles;
	PFN_QERAPP_ALLOCATESELECTEDPATCHHANDLES m_pfnAllocateSelectedPatchHandles;
	PFN_QERAPP_RELEASEPATCHHANDLES m_pfnReleasePatchHandles;
	PFN_QERAPP_GETPATCHDATA m_pfnGetPatchData;
	PFN_QERAPP_GETPATCHHANDLE m_pfnGetPatchHandle;
	PFN_QERAPP_DELETEPATCH m_pfnDeletePatch;
	PFN_QERAPP_CREATEPATCHHANDLE m_pfnCreatePatchHandle;
	PFN_QERAPP_COMMITPATCHHANDLETOMAP m_pfnCommitPatchHandleToMap;
	PFN_QERAPP_COMMITPATCHHANDLETOENTITY m_pfnCommitPatchHandleToEntity;

	PFN_QERAPP_LOADIMAGE m_pfnLoadImage;

	// GTK+ functions
	PFN_QERAPP_MESSAGEBOX m_pfnMessageBox;
	PFN_QERAPP_FILEDIALOG m_pfnFileDialog;
	PFN_QERAPP_DIRDIALOG m_pfnDirDialog;
	PFN_QERAPP_COLORDIALOG m_pfnColorDialog;
	PFN_QERAPP_LOADBITMAP m_pfnLoadBitmap;

	// Profile functions
	PFN_QERAPP_PROFILE_GETDIR m_pfnProfileGetDirectory;
	PFN_QERAPP_PROFILE_SAVEINT m_pfnProfileSaveInt;
	PFN_QERAPP_PROFILE_SAVESTR m_pfnProfileSaveString;
	PFN_QERAPP_PROFILE_LOADINT m_pfnProfileLoadInt;
	PFN_QERAPP_PROFILE_LOADSTR m_pfnProfileLoadString;

	// Sys_ functions
	PFN_QERAPP_SYSUPDATEWINDOWS m_pfnSysUpdateWindows;
	PFN_QERAPP_SYSBEEP m_pfnSysBeep;
	PFN_QERAPP_SYSPRINTF m_pfnSysPrintf;
	PFN_QERAPP_SYSFPRINTF m_pfnSysFPrintf;
	PFN_QERAPP_SYSBEGINWAIT m_pfnSysBeginWait;
	PFN_QERAPP_SYSENDWAIT m_pfnSysEndWait;
	PFN_QERAPP_SYSSETTITLE m_pfnSys_SetTitle;
	PFN_QERAPP_SYSSTATUS m_pfnSys_Status;

	// some core functionality on the map
	PFN_QERAPP_MAPNEW m_pfnMapNew;
	PFN_QERAPP_MAPFREE m_pfnMapFree;
	PFN_QERAPP_MAPBUILDBRUSHDATA m_pfnMapBuildBrushData;
	PFN_QERAPP_MAPISBRUSHFILTERED m_pfnMap_IsBrushFiltered;
	PFN_QERAPP_MAPSTARTPOSITION m_pfnMapStartPosition;
	PFN_QERAPP_MAPREGIONOFF m_pfnMapRegionOff;
	PFN_QERAPP_SETBUILDWINDINGSNOTEXBUILD m_pfnSetBuildWindingsNoTexBuild;
	PFN_QERAPP_POINTFILECLEAR m_pfnPointFileClear;

	PFN_QERAPP_CSGMAKEHOLLOW m_pfnCSG_MakeHollow;
	PFN_QERAPP_CSGMAKEHOLLOWMODE m_pfnCSG_MakeHollowMode;

	PFN_QERAPP_REGIONSPAWNPOINT m_pfnRegionSpawnPoint;
	PFN_QERAPP_GETTICKCOUNT m_pfnQGetTickCount;
	PFN_GETMODELCACHE m_pfnGetModelCache;
	PFN_GETFILETYPEREGISTRY m_pfnGetFileTypeRegistry;

	PFN_QERAPP_READPROJECTKEY m_pfnReadProjectKey;

	PFN_QERPLUG_FILTERACTIVATE m_pfnFiltersActivate;
	PFN_QERPLUG_FILTERADD m_pfnFilterAdd;

	// digibob from the old _QERAppBSPFrontendTable table
	PFN_GETMAPFILENAME m_pfnGetMapName;
};

// macros to access those faster in plugins
#ifdef USE_QERTABLE_DEFINE
#ifndef __QERTABLENAME
#define __QERTABLENAME g_FuncTable
#endif
#define CSG_MakeHollow __QERTABLENAME.m_pfnCSG_MakeHollow
#define CSG_MakeHollowMode __QERTABLENAME.m_pfnCSG_MakeHollowMode
#define Sys_Beep __QERTABLENAME.m_pfnSysBeep
#define Sys_Printf __QERTABLENAME.m_pfnSysPrintf
#define Sys_FPrintf __QERTABLENAME.m_pfnSysFPrintf
#define Sys_BeginWait __QERTABLENAME.m_pfnSysBeginWait
#define Sys_EndWait __QERTABLENAME.m_pfnSysEndWait
#define Sys_UpdateWindows __QERTABLENAME.m_pfnSysUpdateWindows
#define Sys_SetTitle __QERTABLENAME.m_pfnSys_SetTitle
#define Sys_Status __QERTABLENAME.m_pfnSys_Status
#define Select_Deselect __QERTABLENAME.m_pfnDeselectAllBrushes
#define Map_New __QERTABLENAME.m_pfnMapNew
#define Map_Free __QERTABLENAME.m_pfnMapFree
#define Map_IsBrushFiltered __QERTABLENAME.m_pfnMap_IsBrushFiltered
#define Map_BuildBrushData __QERTABLENAME.m_pfnMapBuildBrushData
#define Map_StartPosition __QERTABLENAME.m_pfnMapStartPosition
#define Map_RegionOff __QERTABLENAME.m_pfnMapRegionOff
#define QE_ConvertDOSToUnixName __QERTABLENAME.m_pfnQE_ConvertDOSToUnixName
#define SetBuildWindingsNoTexBuild __QERTABLENAME.m_pfnSetBuildWindingsNoTexBuild
//#define SaveAsDialog __QERTABLENAME.m_pfnSaveAsDialog
#define Pointfile_Clear __QERTABLENAME.m_pfnPointFileClear
#define SetScreenUpdate __QERTABLENAME.m_pfnSetScreenUpdate
#define Region_SpawnPoint __QERTABLENAME.m_pfnRegionSpawnPoint
#define QGetTickCount __QERTABLENAME.m_pfnGetTickCount
#define GetModelCache __QERTABLENAME.m_pfnGetModelCache
#define GetFileTypeRegistry __QERTABLENAME.m_pfnGetFileTypeRegistry
#else
IFileTypeRegistry* GetFileTypeRegistry();
#endif

#endif
