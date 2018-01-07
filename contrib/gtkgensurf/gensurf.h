/*
   GenSurf plugin for GtkRadiant
   Copyright (C) 2001 David Hyde, Loki software and qeradiant.com

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

#ifndef _GENSURF_H_
#define _GENSURF_H_

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "qerplugin.h"
//#include "qertypes.h"

#include "igl.h"
#include "iui_gtk.h"
#include "ientity.h"

#include "gendlgs.h"

#define PLUGIN
#define Q3RADIANT

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
template <class T>
inline T min( T x, T y ) { return ( x < y ) ? x : y; }
template <class T>
inline T max( T x, T y ) { return ( x > y ) ? x : y; }

typedef struct { long x, y; } POINT;
typedef struct { long left, top, right, bottom; } RECT;
#endif
inline bool PtInRect( RECT *rc, POINT pt ){
	if ( pt.x < rc->left ) {
		return false;
	}
	if ( pt.x > rc->right ) {
		return false;
	}
	if ( pt.y < rc->bottom ) {
		return false;
	}
	if ( pt.y > rc->top ) {
		return false;
	}
	return true;
}

#define NUMGAMES 7

#define CONTENTS_SOLID  0x00000001
#define CONTENTS_DETAIL 0x08000000  // brushes to be added after vis leafs
#define CONTENTS_LADDER 0x20000000
#define SURF_HINT       0x100   // make a primary bsp splitter
#define SURF_SKIP       0x200   // completely ignore, allowing non-closed brushes
#define HINT_OFFSET 96

#define PI 3.14159265358979224
#define RadiansToDegrees( a ) ( floor( a * 57.2957795 - 0.5 ) + 1. )
#define DegreesToRadians( a ) ( a / 57.2957795 )

#define BOGUS_RANGE 65536
#define DotProduct( x,y ) ( x[0] * y[0] + x[1] * y[1] + x[2] * y[2] )
#define VectorAdd( a,b,c ) {c[0] = a[0] + b[0]; c[1] = a[1] + b[1]; c[2] = a[2] + b[2]; }
#define VectorClear( x ) {x[0] = x[1] = x[2] = 0; }
#define VectorCopy( a,b ) {b[0] = a[0]; b[1] = a[1]; b[2] = a[2]; }
#define VectorScale( a,b,c ) {c[0] = b * a[0]; c[1] = b * a[1]; c[2] = b * a[2]; }
#define VectorSubtract( a,b,c ) {c[0] = a[0] - b[0]; c[1] = a[1] - b[1]; c[2] = a[2] - b[2]; }
#define XYZVectorSubtract( a,b,c ) {c[0] = (float)a[0] - (float)b[0]; c[1] = (float)a[1] - (float)b[1]; c[2] = (float)a[2] - (float)b[2]; }
#define side( u1,v1,u2,v2,u3,v3 ) ( v3 - v1 ) * ( u2 - u1 ) - ( u3 - u1 ) * ( v2 - v1 )

#define QUAKE2    0
#define HALFLIFE  1
#define SIN       2
#define HERETIC2  3
#define KINGPIN   4
#define GENESIS3D 5
#define QUAKE3    6

#define MAX_FACES_PER_BRUSH 6
#define SLIVER_ANGLE DegreesToRadians( 20 )
#define MAX_NODES ( MAX_ROWS + 1 ) * ( MAX_ROWS + 1 )
#define MAX_TRIS  (MAX_ROWS)*( MAX_ROWS )

typedef float vec;
typedef vec vec3[3];
typedef vec vec2[2];

typedef struct
{
	vec3 v[3];
	char texture[64];
	float Shift[2];
	float Rotate;
	float Scale[2];
	int Contents;
	int Surface;
	int Value;
} FACE;

typedef struct
{
	vec3 normal;
	vec dist;
} PLANE;

typedef struct
{
	int numpoints;
	vec3 p[4];          // variable sized
} MY_WINDING;

typedef struct
{
	int Number;
	int NumFaces;
	FACE face[MAX_FACES_PER_BRUSH];
} BRUSH;

typedef struct tagXYZ
{
	int fixed;
	int done;
	double p[3];
	double pp[3];    // these used only for general 3D projection (not isometric)
	double fixed_value;
	double range;
	double rate;
} XYZ;

// Q2 PAK file structures
typedef struct
{
	char id[4]; // Should be 'PACK'
	int dstart; // Offest in the file to the directory
	int dsize;  // Size in bytes of the directory, same as num_items*64
} pak_header_t;

typedef struct
{
	char name[56]; // The name of the item, normal C string
	int start; // Offset in .pak file to start of item
	int size; // Size of item in bytes
} pak_item_t;

// SiN .SIN structures
#define SINPAKHEADER        ( ( 'K' << 24 ) + ( 'A' << 16 ) + ( 'P' << 8 ) + 'S' )
#define MAX_PAK_FILENAME_LENGTH 120

typedef struct
{
	char name[MAX_PAK_FILENAME_LENGTH];
	int filepos, filelen;
} dpackfile_t;

typedef struct
{
	int ident;          // == IDPAKHEADER
	int dirofs;
	int dirlen;
} dpackheader_t;

// Half-Life WAD file structures
typedef struct
{
	char identification[4];             // should be WAD2 or 2DAW
	int numlumps;
	int infotableofs;
} wadinfo_t;

typedef struct
{
	int filepos;
	int disksize;
	int size;                           // uncompressed
	char type;
	char compression;
	char pad1, pad2;
	char name[16];                      // must be null terminated
} lumpinfo_t;

typedef struct
{
	int signature;
	short version;
	short bitflag;
	short compression_method;
	short modfiletime;
	short modfiledate;
	int crc;
	int compressed_size;
	int uncompressed_size;
	short filename_size;
	short extra_size;
} zipheader_t;

typedef struct
{
	double x[2];
	double y[2];
	double z[2];
} bounding_box;

typedef struct
{
	float p[3];
	int used;
	int tri;
	float error;
	int fixed;
} NODE;

typedef struct
{
	int v[3];
	int n[3];      // indices of neighboring triangles
	PLANE plane;
	int flag;
	float min[3];
	float max[3];
} TRI;

//--------------- bitmap.c -----------------------------
bool OpenBitmap();
void GenerateBitmapMapping();
//--------------- face.c -------------------------------
void PlaneFromPoints( float *, float *, float *, PLANE * );
void CrossProduct( vec3 v1, vec3 v2, vec3 cross );
vec VectorNormalize( vec3 in, vec3 out );
//--------------- gendlg.c -----------------------------
GtkWidget* create_main_dialog();
void About( GtkWidget *parent );
//--------------- genmap.c -----------------------------
double AtLeast( double,double );
bool CanEdit( int, int );
void CloseFuncGroup();
bool FixedPoint( int,int );
void GenerateMap();
void GenerateXYZ();
double LessThan( double,double );
void MakeBrush( BRUSH * );
double MoreThan( double,double );
double Nearest( double,double );
double NoMoreThan( double,double );
void OpenFuncGroup();
void PlasmaCloud();
int PlayerStartZ( double,double );
void SubdividePlasma( int,int,int,int );
bool ValidSurface();
void XYZtoV( XYZ *, vec3 * );
void MakePatch( patchMesh_t * );
double CalculateSnapValue( double value );

//---------------- gensurf.c ---------------------------
bool GenSurfInit();
void ReadIniFile( const char * );
void WriteIniFile( const char * );
void OpenSetup( GtkWidget*,int );
void SaveSetup( GtkWidget* );
//---------------- heretic.c ---------------------------
int GetDefSurfaceProps( char * );
//---------------- view.c ------------------------------
void CreateViewWindow();
void DrawGrid( RECT );
void DrawPreview( RECT );
void evaluate();
void GetScaleFactor( RECT );
void project( XYZ * );
void Scale( RECT,XYZ,POINT * );
void ShowPreview();
void UpdatePreview( bool );

//---------------- plugin.c -----------------------------
void UseFaceBounds();

extern _QERFuncTable_1 g_FuncTable;
extern _QERQglTable g_GLTable;
extern _QERUIGtkTable g_UIGtkTable;
extern _QEREntityTable g_EntityTable;
//#define MAX_ROWS 64
#define MAX_ROWS 128

#define PLANE_XY0 0
#define PLANE_XY1 1
#define PLANE_YZ0 2
#define PLANE_XZ0 3
#define PLANE_YZ1 4
#define PLANE_XZ1 5

#define WAVE_COS_SIN    0
#define WAVE_HCYLINDER  1
#define WAVE_VCYLINDER  2
#define WAVE_BITMAP     3
#define WAVE_ROUGH_ONLY 4
#define WAVE_FORMULA    5
#define WAVE_FIRST      WAVE_COS_SIN
#define WAVE_LAST       WAVE_FORMULA
#define DLG_WAVE_LAST   DLG_WAVE_01 + WAVE_LAST - WAVE_FIRST

#define MSG_VERTEX_SELECTED WM_USER + 1

typedef struct tagMYBITMAP
{
	char name[NAME_MAX];
	char defpath[NAME_MAX];
	double black_value;
	double white_value;
	int width, height;
	unsigned char* colors;
} MYBITMAP;

typedef struct tagELEMENT {
	int i;
	int j;
} ELEMENT;

extern char gszAppDir[NAME_MAX];
extern char gszCaption[64];
extern char gszHelpFile[NAME_MAX];
extern char gszIni[NAME_MAX];
extern char gszMapFile[NAME_MAX];
extern char gszVersion[64];
extern double Amplitude;
extern double Roughness;
extern double TexOffset[2];
extern double TexScale[2];
extern double WaveLength;
extern double Hll, Hur, Vll, Vur;
extern double Z00, Z01, Z10, Z11;
extern double yaw, pitch, roll;
extern ELEMENT Vertex[( MAX_ROWS + 1 ) * ( MAX_ROWS + 1 )];
extern int AddHints;
extern int ArghRad2;
extern int AutoOverwrite;
extern int Decimate;
extern int FileAppend;
extern int FixBorders;
extern int HideBackFaces;
extern int NH, NV;
extern int NumVerticesSelected;
extern int Plane;
extern int Preview;
extern int RandomSeed;
extern int Skybox;
extern int UseDetail;
extern int UseLadder;
extern int VertexMode;
extern int vid_x, vid_y;
extern int WaveType;
extern int gNumNodes;
extern int gNumTris;
extern int view_x, view_y;
extern int view_cx, view_cy;
extern int UsePatches;
extern int SlantAngle;
extern int GimpHints;
extern int Antialiasing;           // ^Fishman - Antializing for the preview window.
extern int AddTerrainKey;           // ^Fishman - Add terrain key to func_group.
extern int SnapToGrid;           // Hydra : snap to grid
extern int SP;       // ^Fishman - Snap to grid.

/*extern HCURSOR   ghCursorCurrent;
   extern HCURSOR   ghCursorDefault;
   extern HCURSOR   ghCursorVertex;
   extern HINSTANCE ghInst;*/
extern GtkWidget *g_pRadiantWnd;
extern GtkWidget *g_pWnd;
/*extern HWND      ghwndAngles;
   extern HWND      ghwndFix;
 */extern GtkWidget     *g_pWndPreview;
extern GtkWidget *g_pPreviewWidget;
extern MYBITMAP gbmp;
extern NODE      *gNode;
extern TRI       *gTri;
extern XYZ xyz[MAX_ROWS + 1][MAX_ROWS + 1];

extern int Game;
extern bounding_box PlayerBox[NUMGAMES];
//extern char      gszOutputDir[NUMGAMES][NAME_MAX];
extern char Texture[NUMGAMES][3][64];
//extern char      gszTextureDir[NUMGAMES][NAME_MAX];
extern char GameName[NUMGAMES][16];
//extern char      pakfile[NUMGAMES][NAME_MAX];
//extern char      lastpakfile[NUMGAMES][NAME_MAX];
//extern int       UsePak[NUMGAMES];
//extern char      GameDir[NUMGAMES][NAME_MAX];
//extern char      ExcelFunc[1024];

#endif // _GENSURF_H_
