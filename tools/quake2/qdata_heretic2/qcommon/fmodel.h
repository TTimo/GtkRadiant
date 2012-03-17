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


/*
   ========================================================================

   .FM triangle flexible model file format

   ========================================================================
 */

#ifndef __FMODEL_HEADER
#define __FMODEL_HEADER

#include "bspfile.h"

//typedef unsigned char         byte;
//typedef int	qboolean;
//typedef float vec3_t[3];

#define MAX_FM_TRIANGLES    2048
#define MAX_FM_VERTS        2048
#define MAX_FM_FRAMES       2048
#define MAX_FM_SKINS        64
#define MAX_FM_SKINNAME     64
#define MAX_FM_MESH_NODES   16      // also defined in game/qshared.h


#define DTRIVERTX_V0   0
#define DTRIVERTX_V1   1
#define DTRIVERTX_V2   2
#define DTRIVERTX_LNI  3
#define DTRIVERTX_SIZE 4

#define SKINPAGE_WIDTH 640
#define SKINPAGE_HEIGHT 480

#define ENCODED_WIDTH_X 92
#define ENCODED_WIDTH_Y 475
#define ENCODED_HEIGHT_X 128
#define ENCODED_HEIGHT_Y 475

#define SCALE_ADJUST_FACTOR 0.96

#define INFO_HEIGHT 5
#define INFO_Y ( SKINPAGE_HEIGHT - INFO_HEIGHT )

extern byte     *BasePalette;
extern byte     *BasePixels,*TransPixels;
extern int BaseWidth, BaseHeight, TransWidth, TransHeight;
extern int ScaleWidth, ScaleHeight;

int ExtractNumber( byte *pic, int x, int y );
void DrawTextChar( int x, int y, char *text );
void DrawLine( int x1, int y1, int x2, int y2 );

// the glcmd format:
// a positive integer starts a tristrip command, followed by that many
// vertex structures.
// a negative integer starts a trifan command, followed by -x vertexes
// a zero indicates the end of the command list.
// a vertex consists of a floating point s, a floating point t,
// and an integer vertex index.


// Initial Header
#define FM_HEADER_NAME  "header"
#define FM_HEADER_VER   2

typedef struct
{
	int skinwidth;
	int skinheight;
	int framesize;              // byte size of each frame

	int num_skins;
	int num_xyz;
	int num_st;                 // greater than num_xyz for seams
	int num_tris;
	int num_glcmds;             // dwords in strip/fan command list
	int num_frames;
	int num_mesh_nodes;
} fmheader_t;


// Skin Header
#define FM_SKIN_NAME    "skin"
#define FM_SKIN_VER     1


// ST Coord Header
#define FM_ST_NAME      "st coord"
#define FM_ST_VER       1

typedef struct
{
	short s;
	short t;
} fmstvert_t;


// Tri Header
#define FM_TRI_NAME     "tris"
#define FM_TRI_VER      1

typedef struct
{
	short index_xyz[3];
	short index_st[3];
} fmtriangle_t;


// Frame Header
#define FM_FRAME_NAME   "frames"
#define FM_FRAME_VER    1

// Frame for compression, just the names
#define FM_SHORT_FRAME_NAME "short frames"
#define FM_SHORT_FRAME_VER  1

// Normals for compressed frames
#define FM_NORMAL_NAME  "normals"
#define FM_NORMAL_VER   1

// Compressed Frame Data
#define FM_COMP_NAME    "comp data"
#define FM_COMP_VER 1

// GL Cmds Header
#define FM_GLCMDS_NAME  "glcmds"
#define FM_GLCMDS_VER   1


// Mesh Nodes Header
#define FM_MESH_NAME    "mesh nodes"
#define FM_MESH_VER     3

// Skeleton Header
#define FM_SKELETON_NAME "skeleton"
#define FM_SKELETON_VER 1

// References Header
#define FM_REFERENCES_NAME "references"
#define FM_REFERENCES_VER   1

typedef struct
{

	union
	{

		byte tris[MAX_FM_TRIANGLES >> 3];

		struct {
			short   *triIndicies;
			int num_tris;
		};

	};

	byte verts[MAX_FM_VERTS >> 3];
	short start_glcmds, num_glcmds;
} fmmeshnode_t;

//=================================================================

// Frame info
typedef struct
{
	byte v[3];              // scaled byte to fit in frame mins/maxs
	byte lightnormalindex;
} fmtrivertx_t;

typedef struct
{
	float scale[3];                 // multiply byte verts by this
	float translate[3];             // then add this
	char name[16];                  // frame name from grabbing
	fmtrivertx_t verts[1];          // variable sized
} fmaliasframe_t;


#endif // #define __FMODEL_HEADER
