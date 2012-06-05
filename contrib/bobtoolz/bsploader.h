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

#define LUMP_ENTITIES       0
#define LUMP_SHADERS        1
#define LUMP_PLANES         2
#define LUMP_NODES          3
#define LUMP_LEAFS          4
#define LUMP_LEAFSURFACES   5
#define LUMP_LEAFBRUSHES    6
#define LUMP_MODELS         7
#define LUMP_BRUSHES        8
#define LUMP_BRUSHSIDES     9
#define LUMP_DRAWVERTS      10
#define LUMP_DRAWINDEXES    11
#define LUMP_FOGS           12
#define LUMP_SURFACES       13
#define LUMP_LIGHTMAPS      14
#define LUMP_LIGHTGRID      15
#define LUMP_VISIBILITY     16
#define HEADER_LUMPS        17

typedef struct {
	int fileofs, filelen;
} lump_t;

typedef struct {
	int ident;
	int version;

	lump_t lumps[HEADER_LUMPS];
} dheader_t;

typedef struct {
	float normal[3];
	float dist;
} dplane_t;

typedef struct {
	int planeNum;
	int children[2];            // negative numbers are -(leafs+1), not nodes
	int mins[3];                // for frustom culling
	int maxs[3];
} dnode_t;

typedef struct {
	int cluster;                    // -1 = opaque cluster (do I still store these?)
	int area;

	int mins[3];                    // for frustum culling
	int maxs[3];

	int firstLeafSurface;
	int numLeafSurfaces;

	int firstLeafBrush;
	int numLeafBrushes;
} dleaf_t;

typedef struct {
	vec3_t xyz;
	float st[2];
	float lightmap[2];
	vec3_t normal;
	byte color[4];
} qdrawVert_t;

typedef struct {
	int shaderNum;
	int fogNum;
	int surfaceType;

	int firstVert;
	int numVerts;

	int firstIndex;
	int numIndexes;

	int lightmapNum;
	int lightmapX, lightmapY;
	int lightmapWidth, lightmapHeight;

	vec3_t lightmapOrigin;
	vec3_t lightmapVecs[3];         // for patches, [0] and [1] are lodbounds

	int patchWidth;
	int patchHeight;
} dsurface_t;

typedef struct {
	int planeNum;                   // positive plane side faces out of the leaf
	int shaderNum;
} dbrushside_t;

typedef struct {
	int firstSide;
	int numSides;
	int shaderNum;              // the shader that determines the contents flags
} dbrush_t;

typedef enum {
	MST_BAD,
	MST_PLANAR,
	MST_PATCH,
	MST_TRIANGLE_SOUP,
	MST_FLARE
} mapSurfaceType_t;

#define MAX_MAP_VISIBILITY  0x200000
#define MAX_MAP_NODES       0x20000
#define MAX_MAP_PLANES      0x20000
#define MAX_MAP_LEAFS       0x20000

extern int numVisBytes;
extern int numleafs;
extern int numplanes;
extern int numnodes;
extern int numDrawVerts;
extern int numDrawSurfaces;
extern int numleafsurfaces;
extern int numbrushes;
extern int numbrushsides;
extern int numleafbrushes;

extern dnode_t         *dnodes;
extern dplane_t        *dplanes;
extern dleaf_t         *dleafs;
extern byte            *visBytes;
extern qdrawVert_t     *drawVerts;
extern dsurface_t      *drawSurfaces;
extern int             *dleafsurfaces;
extern dbrush_t        *dbrushes;
extern dbrushside_t    *dbrushsides;
extern int             *dleafbrushes;

qboolean LoadBSPFile( const char *filename );
void FreeBSPData();
