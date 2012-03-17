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

typedef struct
{
	float x, y, z;
} _3DSPoint_t;

typedef struct
{
	short a, b, c;
	short flags;
} _3DSFace_t;

typedef struct
{
	float s, t;
} _3DSTexVert_t;

typedef struct
{
	char name[100];
	short numFaces;
	short *pFaces;
} _3DSMeshMaterialGroup_t;

typedef struct
{
	char name[80];

	char texture[100];
	char specular[100];
	char reflection[100];
	char bump[100];
	char opacity[100];
} _3DSMaterial_t;

typedef struct
{
	short numFaces, numPoints, numTexVerts;
	int numMeshMaterialGroups;

	_3DSPoint_t *pPoints;
	_3DSFace_t  *pFaces;
	_3DSTexVert_t *pTexVerts;

	_3DSMeshMaterialGroup_t *pMeshMaterialGroups;
} _3DSTriObject_t;

typedef struct
{
	char name[100];

	int numTriObjects;
	_3DSTriObject_t *pTriObjects;
} _3DSNamedObject_t;

typedef struct
{
	int numNamedObjects;
	int numMaterials;

	_3DSNamedObject_t   *pNamedObjects;
	_3DSMaterial_t      *pMaterials;

} _3DSEditChunk_t;

typedef struct
{
	_3DSEditChunk_t editChunk;
} _3DS_t;

#define _3DS_CHUNK_NULL                     0x0000
#define _3DS_CHUNK_UNKNOWN0                 0x0001
#define _3DS_CHUNK_M3D_VERSION              0x0002
#define _3DS_CHUNK_M3D_KFVERSION            0x0005
#define _3DS_CHUNK_COLOR_F                  0x0010
#define _3DS_CHUNK_COLOR_24                 0x0011
#define _3DS_CHUNK_LIN_COLOR_24             0x0012
#define _3DS_CHUNK_LIN_COLOR_F              0x0013
#define _3DS_CHUNK_INT_PERCENTAGE           0x0030
#define _3DS_CHUNK_FLOAT_PERCENT            0x0031
#define _3DS_CHUNK_MASTER_SCALE             0x0100
#define _3DS_CHUNK_CHUNK_TYPE               0x0995
#define _3DS_CHUNK_CHUNK_UNIQUE             0x0996
#define _3DS_CHUNK_NOT_CHUNK                0x0997
#define _3DS_CHUNK_CONTAINER                0x0998
#define _3DS_CHUNK_IS_CHUNK                 0x0999
#define _3DS_CHUNK_C_SXP_SELFI_MASKDATA     0x0c3c

#define _3DS_CHUNK_BITMAP                   0x1100
#define _3DS_CHUNK_USE_BITMAP               0x1101
#define _3DS_CHUNK_SOLID_BGND               0x1200
#define _3DS_CHUNK_USE_SOLID_BGND           0x1201

#define _3DS_CHUNK_EDIT                     0x3d3d
#define _3DS_CHUNK_MESH_VERSION             0x3d3e

#define _3DS_CHUNK_NAMED_OBJECT             0x4000
#define _3DS_CHUNK_NAMED_TRI_OBJECT         0x4100
#define _3DS_CHUNK_POINT_ARRAY              0x4110
#define _3DS_CHUNK_POINT_FLAG_ARRAY         0x4111
#define _3DS_CHUNK_FACE_ARRAY               0x4120
#define _3DS_CHUNK_MSH_MAT_GROUP            0x4130
#define _3DS_CHUNK_TEX_VERTS                0x4140
#define _3DS_CHUNK_SMOOTH_GROUP             0x4150
#define _3DS_CHUNK_MESH_MATRIX              0x4160
#define _3DS_CHUNK_MAGIC                    0x4d4d

#define _3DS_CHUNK_MAT_NAME                 0xa000
#define _3DS_CHUNK_TEXMAP                   0xa200
#define _3DS_CHUNK_SPECMAP                  0xa204
#define _3DS_CHUNK_OPACMAP                  0xa210
#define _3DS_CHUNK_REFLMAP                  0xa220
#define _3DS_CHUNK_BUMPMAP                  0xa230
#define _3DS_CHUNK_MAT_MAPNAME              0xa300
#define _3DS_CHUNK_MAT_LIST                 0xafff

#define _3DS_CHUNK_KEYFRAME_DATA            0xb000

void _3DS_LoadPolysets( const char *filename, polyset_t **ppPSET, int *numpsets, qboolean verbose );
