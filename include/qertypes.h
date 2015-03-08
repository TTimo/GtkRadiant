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

// qertypes.h
//
// common types
// merged from brush.h, etc. for plugin support
//
#ifndef _QERTYPES_H_
#define _QERTYPES_H_

#ifdef _WIN32
#include <wtypes.h>
#endif

#include <GL/gl.h>

#include "str.h"

#ifdef _WIN32
#define PATH_MAX 260
#endif

// HACK glib-2.0
#define NAME_MAX 255

typedef bool qboolean;

#define MAXPOINTS   16

// merged from qedefs.h ------

#define MAX_EDGES   512
#define MAX_POINTS  1024

#define COLOR_TEXTUREBACK     0
#define COLOR_GRIDBACK        1
#define COLOR_GRIDMINOR       2
#define COLOR_GRIDMAJOR       3
#define COLOR_CAMERABACK      4
#define COLOR_ENTITY        5
#define COLOR_GRIDBLOCK     6
#define COLOR_GRIDTEXT      7
#define COLOR_BRUSHES       8
#define COLOR_SELBRUSHES    9
#define COLOR_CLIPPER       10
#define COLOR_VIEWNAME      11
#define COLOR_SELBRUSHES3D  12

#define COLOR_GRIDMINOR_ALT 13
#define COLOR_GRIDMAJOR_ALT 14

#define COLOR_DETAIL        15

#define COLOR_LAST          16

// ----------------------------

typedef float vec_t;
typedef vec_t vec3_t[3];

// turn this on/off to use a static texdef or a memory one
// THIS MUST BE CONSISTENT throughout a whole build of Radiant / modules / plugins
// DO_TEXDEF_ALLOC is more memory efficient, but I suspect it to be wacky on win32 / C runtime etc.
#define DO_TEXDEF_ALLOC 1
#if DO_TEXDEF_ALLOC

class texdef_t
{
private:
char *name;
public:
texdef_t(){
	name = new char[1];
	name[0] = '\0';
	shift[0] = 0.0f;
	shift[1] = 0.0f;
	rotate = 0.0f;
	scale[0] = 1.0f;
	scale[1] = 1.0f;
	contents = 0;
	flags = 0;
	value = 0;
}
texdef_t( const texdef_t& other ){
	name = NULL;
	SetName( other.name );
	shift[0] = other.shift[0];
	shift[1] = other.shift[1];
	rotate = other.rotate;
	scale[0] = other.scale[0];
	scale[1] = other.scale[1];
	contents = other.contents;
	flags = other.flags;
	value = other.value;
}
~texdef_t(){
	if ( name ) {
		delete []name;
		name = (char*)NULL;
	}
}

void SetName( const char *p ){
	if ( name ) {
		delete []name;
		name = NULL;
	}
	if ( p ) {
		name = strcpy( new char[strlen( p ) + 1], p );
	}
	else
	{
		name = new char[1];
		name[0] = '\0';
	}
}

const char * GetName() const {
	return name;
}

// NOTE TTimo when loading prefs as binary, we load a bogus value in texdef..
void DropName(){
	name = NULL;
	SetName( NULL );
}

texdef_t& operator =( const texdef_t& rhs ){
	if ( &rhs != this ) {
		SetName( rhs.name );
		shift[0] = rhs.shift[0];
		shift[1] = rhs.shift[1];
		rotate = rhs.rotate;
		scale[0] = rhs.scale[0];
		scale[1] = rhs.scale[1];
		contents = rhs.contents;
		flags = rhs.flags;
		value = rhs.value;
	}
	return *this;
}
float shift[2];
float rotate;
float scale[2];
int contents;
int flags;
int value;
};

#else

// max length of a vfs texture path
#define QPATH 64
class texdef_t
{
private:
char name[QPATH];
public:
texdef_t() { name[0] = '\0'; }
~texdef_t() {  }

void SetName( const char *p ){
	strncpy( name, p, QPATH );
}

const char * GetName() const {
	return name;
}

// NOTE TTimo when loading prefs as binary, we load a bogus value in texdef..
void DropName(){
	name[0] = '\0';
}

texdef_t& operator =( const texdef_t& rhs ){
	if ( &rhs != this ) {
		SetName( rhs.name );
		shift[0] = rhs.shift[0];
		shift[1] = rhs.shift[1];
		rotate = rhs.rotate;
		scale[0] = rhs.scale[0];
		scale[1] = rhs.scale[1];
		contents = rhs.contents;
		flags = rhs.flags;
		value = rhs.value;
	}
	return *this;
}
float shift[2];
float rotate;
float scale[2];
int contents;
int flags;
int value;
};

#endif

// forward declare
class IShader;

// Timo
// new brush primitive texdef
typedef struct brushprimit_texdef_s
{
	vec_t coords[2][3];
} brushprimit_texdef_t;

// this structure is used in Radiant to reflect the state of the texture window
// it gives information on current shader and various flags
class texturewin_t
{
public:
texturewin_t(){
}
~texturewin_t(){
}
int width, height;
int originy;
// add brushprimit_texdef_t for brush primitive coordinates storage
brushprimit_texdef_t brushprimit_texdef;
int m_nTotalHeight;
// surface plugin, must be casted to a IPluginTexdef*
void* pTexdef;
texdef_t texdef;
// shader
// NOTE: never NULL, initialized in Texture_Init
// NOTE: the reference name of the shader is texdef.name (see QERApp_ReloadShaders for an example)
IShader *pShader;
};

#define QER_TRANS     0x00000001
#define QER_NOCARVE   0x00000002
#define QER_NODRAW    0x00000004
#define QER_NONSOLID  0x00000008
#define QER_WATER     0x00000010
#define QER_LAVA      0x00000020
#define QER_FOG       0x00000040
#define QER_ALPHAFUNC 0x00000080
#define QER_CULL      0x00000100


// describes a GL texture that Radiant uses to represent a shader
// NOTE: all qtexture_t are stored in a main list at g_qeglobals.d_qtextures
// shaders have reference couting, but qtexture_t don't (they're way too deep into Radiant)
typedef struct qtexture_s
{
	struct    qtexture_s *next;
	// name of the texture file (the physical image file we are using)
	// NOTE: used for lookup, must be unique .. vfs path of the texture, lowercase, NO FILE EXTENSION
	// ex textures/gothic_wall/iron
	// NOTE: the "textures/" prefix might seem unnecessary .. but it's better to stick to the vfs name
	char name[64];
	int width,  height;
	GLuint texture_number;      // gl bind number (the qtexture_t are usually loaded and binded by the shaders module)
	vec3_t color;                   // for flat shade mode
	qboolean inuse;               // true = is present on the level (for the texture browser interface)
} qtexture_t;

// NOTE: don't trust this definition!
// you should read float points[..][5]
// see NewWinding definition
// WARNING: don't touch anything to this struct unless you looked into winding.cpp and WINDING_SIZE(pt)
#define MAX_POINTS_ON_WINDING 64
typedef struct
{
	int numpoints;
	int maxpoints;
	float points[8][5];             // variable sized
} winding_t;

typedef struct
{
	vec3_t normal;
	double dist;
	int type;
} plane_t;

// pShader is a shortcut to the shader
// it's only up-to-date after a Brush_Build call
// to initialize the pShader, use QERApp_Shader_ForName(texdef.name)
typedef struct face_s
{
	struct face_s           *next;
	struct face_s           *prev;
	struct face_s           *original;      //used for vertex movement
	vec3_t planepts[3];
	texdef_t texdef;
	plane_t plane;

	// Nurail: Face Undo
	int undoId;
	int redoId;

	winding_t               *face_winding;

	vec3_t d_color;
	vec_t d_shade;
	// calls through here have indirections (pure virtual)
	// it would be good if the rendering loop would avoid scanning there (for the GL binding number for example)
	IShader                 *pShader;
	//++timo FIXME: remove!
	qtexture_t              *d_texture;

	// Timo new brush primit texdef
	brushprimit_texdef_t brushprimit_texdef;

	// cast this one to an IPluginTexdef if you are using it
	// NOTE: casting can be done with a GETPLUGINTEXDEF defined in isurfaceplugin.h
	// TODO: if the __ISURFACEPLUGIN_H_ header is used, use a union { void *pData; IPluginTexdef *pPluginTexdef } kind of thing ?
	void                    *pData;
} face_t;

typedef struct {
	vec3_t xyz;
	float sideST[2];
	float capST[2];
} curveVertex_t;

typedef struct {
	curveVertex_t v[2];
} sideVertex_t;


#define MIN_PATCH_WIDTH     3
#define MIN_PATCH_HEIGHT    3

#define MAX_PATCH_WIDTH     16
#define MAX_PATCH_HEIGHT    16

// patch type info
// type in lower 16 bits, flags in upper
// endcaps directly follow this patch in the list

// types
#define PATCH_GENERIC     0x00000000    // generic flat patch
#define PATCH_CYLINDER    0x00000001    // cylinder
#define PATCH_BEVEL       0x00000002    // bevel
#define PATCH_ENDCAP      0x00000004    // endcap
#define PATCH_HEMISPHERE  0x00000008    // hemisphere
#define PATCH_CONE        0x00000010    // cone
#define PATCH_TRIANGLE    0x00000020    // simple tri, assumes 3x3 patch

// behaviour styles
#define PATCH_CAP         0x00001000    // flat patch applied as a cap
#define PATCH_SEAM        0x00002000    // flat patch applied as a seam
#define PATCH_THICK       0x00004000    // patch applied as a thick portion

// styles
#define PATCH_BEZIER      0x00000000    // default bezier
#define PATCH_BSPLINE     0x10000000    // bspline

#define PATCH_TYPEMASK     0x00000fff    //
#define PATCH_BTYPEMASK    0x0000f000    //
#define PATCH_STYLEMASK    0xffff0000    //

typedef struct {
	vec3_t xyz;
	float st[2];
	float lightmap[2];
	vec3_t normal;
} drawVert_t;

// spog - used for patch LOD trees

struct BTNode_t
{
	BTNode_t *left, *right;
	drawVert_t info;
	drawVert_t vMid;
};

struct BTreeList_t
{
	BTreeList_t *next;
	BTNode_t *pBT;
	drawVert_t vLeft, vRight;
};

struct BTListList_t
{
	BTListList_t *next;
	BTreeList_t *list;
};

// used in brush primitive AND entities
typedef struct epair_s
{
	struct epair_s    *next;
	char  *key;
	char  *value;
} epair_t;

struct brush_s;
typedef struct brush_s brush_t;

typedef struct {
	int width, height;      // in control points, not patches
	int contents, flags, value, type;
	qtexture_t  *d_texture;
	IShader     *pShader;
	drawVert_t ctrl[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT];
	brush_t *pSymbiot;
	qboolean bSelected;
	qboolean bOverlay;
	qboolean bDirty;
	int nListID;
	epair_t *epairs;
	// cast this one to an IPluginTexdef if you are using it
	// NOTE: casting can be done with a GETPLUGINTEXDEF defined in isurfaceplugin.h
	// TODO: if the __ISURFACEPLUGIN_H_ header is used, use a union { void *pData; IPluginTexdef *pPluginTexdef } kind of thing ?
	void                  *pData;
	// spog - curve LOD binary trees and lists
	BTNode_t *rowLOD[( ( MAX_PATCH_WIDTH - 1 ) / 2 ) * MAX_PATCH_HEIGHT]; // = ((MAX_PATCH_WIDTH-1)/2) * MAX_PATCH_HEIGHT
	BTNode_t *colLOD[( ( MAX_PATCH_HEIGHT - 1 ) / 2 ) * MAX_PATCH_WIDTH]; // = ((MAX_PATCH_HEIGHT-1)/2) * MAX_PATCH_WIDTH
	bool rowDirty[( ( MAX_PATCH_WIDTH - 1 ) - 1 ) / 2];
	bool colDirty[( ( MAX_PATCH_HEIGHT - 1 ) - 1 ) / 2];
	bool LODUpdated;
	void *drawLists; // pointer to std::list
} patchMesh_t;

typedef struct brush_s
{
	struct brush_s    *prev, *next; // links in active/selected
	struct brush_s    *oprev, *onext; // links in entity
	struct entity_s   *owner;
	vec3_t mins, maxs;
	face_t                *brush_faces;

	qboolean bModelFailed;
	//
	// curve brush extensions
	// all are derived from brush_faces
	qboolean patchBrush;
	qboolean hiddenBrush;

	//int nPatchID;

	patchMesh_t *pPatch;
	struct entity_s *pUndoOwner;

	int undoId;                     //undo ID
	int redoId;                     //redo ID
	int ownerId;                //entityId of the owner entity for undo

	// TTimo: this is not legal, we are not supposed to put UI toolkit dependant stuff in the interfaces
	// NOTE: the grouping stuff never worked, there is embryonary code everywhere though
	int numberId;
	void* itemOwner; // GtkCTreeNode* ?

	// brush primitive only
	epair_t *epairs;

	// brush filtered toggle
	bool bFiltered;
	bool bCamCulled;
	bool bBrushDef;
} brush_t;

#define MAX_FLAGS   16

typedef struct vertmodel_t
{
	float v[3];
	float st[2];
	float normal[3];
} vertmodel;

typedef struct triindex_t
{
	int indexes[3];
} triindex;

// TTimo: NOTE: we don't have dedicated stuff to copy/allocate/delete this structure like we do for entity_t and brush_t
//   could be necessary, I'm adding GString *strSkin that needs to be copied around
// TTimo 04/01/2001 removing the GString* for toolkit-independent interfaces .. cast it ..
typedef struct entitymodel_t
{
	struct entitymodel_t *pNext;
	int nTriCount;
	//trimodel *pTriList;
	//md3Triangle_t *pTriList;
	triindex *pTriList;
	vertmodel *pVertList;
	int numVerts;
	int nTextureBind;
	void *strSkin; // toolkit-independent .. cast to a GString*
	int nSkinWidth;
	int nSkinHeight;
	int nModelPosition;
} entitymodel;

// eclass show flags

#define     ECLASS_LIGHT      0x00000001
#define     ECLASS_ANGLE      0x00000002
#define     ECLASS_PATH       0x00000004
#define     ECLASS_MISCMODEL  0x00000008

#ifdef USEPLUGINENTITIES
#define     ECLASS_PLUGINENTITY 0x00000010
#endif // USEPLUGINENTITIES

typedef struct eclass_s
{
	struct eclass_s *next;
	char    *name;
	qboolean fixedsize;
	qboolean unknown;           // wasn't found in source
	vec3_t mins, maxs;
	vec3_t color;
	texdef_t texdef;
	char    *comments;
	char flagnames[MAX_FLAGS][32];

	entitymodel *model;
	char  *modelpath;
	//++timo NOTE: I don't know what this is used for exactly. But don't trust it for the real skin paths on models (screws up with long/short path names)
	//++hydra NOTE: this, hopefully, will be used to use specific shaders on the bounding boxes of the eclass instead of a color.
	char  *skinpath;
	int nFrame;
	unsigned int nShowFlags;

	void* hPlug;
} eclass_t;

extern eclass_t    *eclass;

/*
** window bits
*/
#define W_CAMERA          0x0001
#define W_XY                0x0002
#define W_XY_OVERLAY    0x0004
#define W_Z                 0x0008
#define W_TEXTURE         0x0010
#define W_Z_OVERLAY     0x0020
#define W_CONSOLE         0x0040
#define W_ENTITY          0x0080
#define W_CAMERA_IFON 0x0100
#define W_XZ          0x0200  //--| only used for patch vertex manip stuff
#define W_YZ          0x0400  //--|
#define W_GROUP       0x0800
#define W_MEDIA       0x1000
#define W_ALL           0xFFFFFFFF

// used in some Drawing routines
enum VIEWTYPE {YZ, XZ, XY};
const char g_AxisName[3] = { 'X', 'Y', 'Z' };

// dynamically allocated string
class string_t
{
public:
inline string_t(){
	copy( "" );
}
inline string_t( const string_t& other ){
	copy( other.m_string );
}
inline string_t( const char* string ){
	copy( string );
}
inline ~string_t(){
	destroy();
}
inline const string_t& operator=( const string_t& other ){
	destroy();
	copy( other.m_string );
	return *this;
}
inline const string_t& operator=( const char* string ){
	destroy();
	copy( string );
	return *this;
}
inline bool operator<( const string_t& other ) const {
	return compare( other ) < 0;
}
inline bool operator>( const string_t& other ) const {
	return compare( other ) > 0;
}
inline bool operator==( const string_t& other ) const {
	return compare( other ) == 0;
}
inline bool operator!=( const string_t& other ) const {
	return compare( other ) != 0;
}
inline const char* c_str() const {
	return m_string;
}
private:
inline void copy( const char* string ){
	m_string = new char[strlen( string ) + 1];
	strcpy( m_string, string );
}
inline void destroy(){
	delete[] m_string;
}
inline int compare( const string_t& other ) const {
	return strcmp( m_string, other.m_string );
}

char* m_string;
};

class filetype_t
{
public:
filetype_t()
	: name( "" ), pattern( "" )
{}
filetype_t( const char* _name, const char* _pattern )
	: name( _name ), pattern( _pattern )
{}
const char* name;
const char* pattern;
};


/*
** Outline bits
*/
#define OUTLINE_ZBUF  0x01  // zbuffered outline
#define OUTLINE_BSEL  0x02  // selection overlay

#ifdef USEPLUGINENTITIES
// forward declare this one
class IPluginEntity;
#endif // USEPLUGINENTITIES

// MODEL

class IRender;
class ISelect;
class IEdit;

// NOTE TTimo about ~entity_interfaces_t
// using constructors / destructors on C structs is bad practice
struct entity_interfaces_t
{
	IRender *pRender;
	ISelect *pSelect;
	IEdit *pEdit;
};
// MODEL END

typedef struct entity_s
{
	struct entity_s   *prev, *next;

	/*!
	   \todo can use a brushes list, or the blind data below
	   for now, blind data should be interpreted as CPtrArray*, only use in the IMAP API
	 */
	brush_t brushes;                        // head/tail of list
	void *pData;

	int undoId, redoId, entityId;           // used for undo/redo
	vec3_t origin;
	eclass_t  *eclass;
	epair_t       *epairs;
	entity_interfaces_t model;
#ifdef USEPLUGINENTITIES
	IPluginEntity *pPlugEnt;
#endif // USEPLUGINENTITIES

	// this is cam code addition?
	vec3_t color;

	// Arnout: HACK-ish and change for 1.3 (in 1.3 we have a blind data pointer according to TTimo)
	float fLightEnvelope1[3];
	float fLightEnvelope2[2];
} entity_t;

typedef struct
{
	int p1, p2;
	face_t   *f1, *f2;
} pedge_t;

// window system independent camera view code
// NOTE TTimo taken from xy.h
typedef struct
{
	int width, height;

	qboolean timing;

	vec3_t origin;   // at center of window
	float scale;

	float topclip, bottomclip;

	qboolean d_dirty;
} xy_t;

// spog - struct used for nodes in filters list
struct bfilter_t //c++ style
{
	bfilter_t   *next;
	int attribute;          // 1=brush->face->pShader->getName()
	                        // 2=brush->pPatch->pShader->getFlags()
	                        // 3=brush->owner->eclass->name
	                        // 4=brush->owner->eclass->nShowFlags
	                        // 5=brush->face->texdef.flags (q2)
	                        // 6=brush->face->texdef.contents (q2)
	int mask;
	const char  *string;
	bool active;
};

// djbob: no longer any need to add only to end, versioning removed, it is no longer saved as binary
// IMPORTANT: whenever you update this struct, you need to add the relevant load/save code
// preferences.cpp LoadPref / SavePref
typedef struct
{
	int iTexMenu;           // nearest, linear, etc
	float fGamma;         // gamma for textures
	vec3_t colors[COLOR_LAST];
	int exclude;
	int include;
	texdef_t m_SIIncrement; // increments for the surface inspector
	texdef_t m_PIIncrement; // increments for the patch inspector
	vec3_t AxisColors[3];   // colors used for X, Y Z axis
	                        // these are in the View > Show menu with Show coordinates
	qboolean show_names;
	qboolean show_coordinates;
	qboolean show_angles;
	qboolean show_outline;
	qboolean show_axis;
	qboolean bNoSelectedOutlines;
	bfilter_t *filters; // FIXME spog - might be better in another location?
	int iSelectedOutlinesStyle;
} SavedInfo_t;

typedef enum
{
	sel_brush,
	sel_brush_on,
	sel_brush_off,
	// sel_sticky_brush,
	// sel_face,
	sel_vertex,
	sel_edge,
	sel_singlevertex,
	sel_curvepoint,
	sel_area,
	sel_areatall,
	sel_facets_on,
	sel_facets_off,
} select_t;

// most of the QE globals are stored in this structure
typedef struct
{
	qboolean d_showgrid;
	float d_gridsize;
	qboolean d_bSmallGrid; // we use this flag to hack our way into editing of <1 grids

	int d_num_entities;

	entity_t *d_project_entity;

	// defines the boundaries of the current work area
	// is used to guess brushes and drop points third coordinate when creating from 2D view
	vec3_t d_work_min,d_work_max;
	// not stored in registry, default is off
	qboolean d_show_work;

	vec3_t d_points[MAX_POINTS];
	int d_numpoints;
	pedge_t d_edges[MAX_EDGES];
	int d_numedges;

	int d_num_move_points;
	float        *d_move_points[4096];

	qtexture_t   *d_qtextures;
	// used to speedup access, specially in QERApp_Try_Texture_ForName
	// must always be kept up-to-date with d_qtextures*
	//++timo FIXME at some point in the future it would even be better to remove d_qtextures and use this instead
	GHashTable *d_qtexmap;

	texturewin_t d_texturewin;

	int d_pointfile_display_list;

	xy_t d_xyOld;

	SavedInfo_t d_savedinfo;

	int d_workcount;

	// connect entities uses the last two brushes selected
	int d_select_count;
	brush_t      *d_select_order[2];
	vec3_t d_select_translate;        // for dragging w/o making new display lists
	select_t d_select_mode;

	int d_parsed_brushes;

	qboolean show_blocks;
	int blockSize;

	// NOTE TTimo
	// a lot of this data should be in a property bag and available to the other modules through an API
	// this is generated from game configuration and the project settings, and should be still be part of it

	// tells if we are internally using brush primitive (texture coordinates and map format)
	// this is a shortcut for IntForKey( g_qeglobals.d_project_entity, "brush_primit" )
	// NOTE: must keep the two ones in sync
	bool m_bBrushPrimitMode;

	/*!
	   win32: engine full path.
	   unix: user home full path + engine dir.
	 */
	Str m_strHomeGame;
	/*!
	   cache for m_strHomeGame + mod subdirectory.
	 */
	Str m_strHomeMaps;

	// used while importing brush data from file or memory buffer
	// tells if conversion between map format and internal preferences ( m_bBrushPrimitMode ) is needed
	qboolean bNeedConvert;
	qboolean bOldBrushes;
	qboolean bPrimitBrushes;

	vec3_t d_vAreaTL;
	vec3_t d_vAreaBR;

	// tells if we are using .INI files for prefs instead of registry
	qboolean use_ini;
	// even in .INI mode we use the registry for all void* prefs
	char use_ini_registry[64];
	// disabled all INI / registry read write .. used when shutting down after registry cleanup
	qboolean disable_ini;

	// tells we are using a BSP frontend plugin
	qboolean bBSPFrontendPlugin;

	// handle to the console log file
	// we use low level I/O to get rid of buffering and have everything on file if we crash
	int hLogFile;

	qboolean bTextureCompressionSupported; // is texture compression supported by hardware?
	GLint texture_components;

	// temporary values that should be initialised only once at run-time
	// there are too many uneccessary calls to Sys_QGL_ExtensionSupported
	// NOTE TTimo: those are unused atm (set right, but not used)
	bool m_bOpenGLCompressionSupported;
	bool m_bS3CompressionSupported;

	// set to true after OpenGL has been initialized and extensions have been tested
	bool m_bOpenGLReady;

	// set this to true and any new brushes will be detail by default (else they are structural)
	bool m_bMakeDetail;
} QEGlobals_t;

#endif // _QERTYPES_H_
