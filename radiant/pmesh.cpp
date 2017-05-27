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

//
// Preliminary patch stuff
//
//

#include <glib/gi18n.h>

#include "stdafx.h"
#include "gtkmisc.h"

#include "gtkr_list.h"

// externs
extern void MemFile_fprintf( MemStream* pMemFile, const char* pText, ... );
extern face_t *Face_Alloc( void );

void _Write3DMatrix( FILE *f, int y, int x, int z, float *m );
void _Write3DMatrix( MemStream *f, int y, int x, int z, float *m );

void Patch_InitialiseLODPointers( patchMesh_t *p ){
	int i;
	int rowcount = ( ( MAX_PATCH_WIDTH - 1 ) / 2 ) * MAX_PATCH_HEIGHT;
	for ( i = 0; i < rowcount; i++ )
		p->rowLOD[i] = NULL;
	int colcount = ( ( MAX_PATCH_HEIGHT - 1 ) / 2 ) * MAX_PATCH_WIDTH;
	for ( i = 0; i < colcount; i++ )
		p->colLOD[i] = NULL;
}

patchMesh_t* Patch_Alloc(){
	patchMesh_t *pPatch = (patchMesh_t *)malloc( sizeof( patchMesh_t ) );
	pPatch->pShader = NULL;
	pPatch->pSymbiot = NULL; // Hydra: added missing initialiser.
	// spog - initialise patch LOD pointers
	Patch_InitialiseLODPointers( pPatch );
	pPatch->drawLists = NULL;
	pPatch->bDirty = true;
	pPatch->nListID = -1;
	pPatch->bSelected = false;
	pPatch->bOverlay = false;
	pPatch->bDirty = true;
	pPatch->LODUpdated = false;

	int i;
	for ( i = 0; i < ( ( ( MAX_PATCH_WIDTH - 1 ) - 1 ) / 2 ); i++ )
		pPatch->rowDirty[i] = false;
	for ( i = 0; i < ( ( ( MAX_PATCH_HEIGHT - 1 ) - 1 ) / 2 ); i++ )
		pPatch->colDirty[i] = false;

	return pPatch;
}

patchMesh_t* MakeNewPatch(){
	patchMesh_t *pm = reinterpret_cast<patchMesh_t*>( qmalloc( sizeof( patchMesh_t ) ) );

	// spog - initialise patch LOD pointers
	Patch_InitialiseLODPointers( pm );
	pm->drawLists = NULL;
	pm->bDirty = true;

	return pm;
}

// FIXME: this needs to be dynamic
//#define	MAX_PATCH_MESHES	4096
//patchMesh_t		patchMeshes[MAX_PATCH_MESHES];
//int numPatchMeshes = 0;

// used for a save spot
patchMesh_t patchSave;

// Tracks the selected patch for point manipulation/update. FIXME: Need to revert back to a generalized
// brush approach
//--int  g_nSelectedPatch = -1;

// HACK: for tracking which view generated the click
// as we dont want to deselect a point on a same point
// click if it is from a different view
int g_nPatchClickedView = -1;
bool g_bSameView = false;

//typedef enum XFormType { TRANSLATE, SCALE, ROTATE };


// globals
bool g_bPatchShowBounds = true;
bool g_bPatchWireFrame = false;
bool g_bPatchWeld = true;
bool g_bPatchDrillDown = true;
//bool g_bPatchInsertMode = false;
bool g_bPatchBendMode = false;
int g_nPatchBendState = -1;
int g_nPatchInsertState = -1;
int g_nBendOriginIndex = 0;
vec3_t g_vBendOrigin;

bool g_bPatchAxisOnRow = true;
int g_nPatchAxisIndex = 0;
bool g_bPatchLowerEdge = true;

vec3_t g_vCycleCapNormal;
// cycles when we use Patch_CycleCapSelected
VIEWTYPE g_nCycleCapIndex = XY;

// BEND states
enum
{
	BEND_SELECT_ROTATION = 0,
	BEND_SELECT_ORIGIN,
	BEND_SELECT_EDGE,
	BEND_BENDIT,
	BEND_STATE_COUNT
};

const char *g_pBendStateMsg[] =
{
	"Use TAB to cycle through available bend axis. Press ENTER when the desired one is highlighted.",
	"Use TAB to cycle through available rotation axis. This will LOCK around that point. You may also use Shift + Middle Click to select an arbitrary point. Press ENTER when the desired one is highlighted",
	"Use TAB to choose which side to bend. Press ENTER when the desired one is highlighted.",
	"Use the MOUSE to bend the patch. It uses the same ui rules as Free Rotation. Press ENTER to accept the bend, press ESC to abandon it and exit Bend mode",
	""
};

// INSERT states
enum
{
	INSERT_SELECT_EDGE = 0,
	INSERT_STATE_COUNT
};

const char* g_pInsertStateMsg[] =
{
	"Use TAB to cycle through available rows/columns for insertion/deletion. Press INS to insert at the highlight, DEL to remove the pair"
};


float *g_InversePoints[1024];

const float fFullBright = 1.0;
const float fLowerLimit = .50;
const float fDec = .05f;
void _SetColor( face_t* f, float fColor[3] ){
	return;
	fColor[0] = f->d_color[0];
	fColor[1] = f->d_color[1];
	fColor[2] = f->d_color[2];
	qglColor3fv( fColor );
}


void _DecColor( float fColor[3] ){
	return;
	fColor[0] -= fDec;
	fColor[1] -= fDec ;
	fColor[2] -= fDec;
	for ( int i = 0; i < 3; i++ )
	{
		if ( fColor[i] <= fLowerLimit ) {
			fColor[0] = fFullBright;
			fColor[1] = fFullBright;
			fColor[2] = fFullBright;
			break;
		}
	}
	qglColor3fv( fColor );
}

vec_t __VectorNormalize( vec3_t in, vec3_t out ){
	vec_t length, ilength;

	length = sqrt( in[0] * in[0] + in[1] * in[1] + in[2] * in[2] );
	if ( length == 0 ) {
		VectorClear( out );
		return 0;
	}

	ilength = 1.0 / length;
	out[0] = in[0] * ilength;
	out[1] = in[1] * ilength;
	out[2] = in[2] * ilength;

	return length;
}


void Patch_SetType( patchMesh_t *p, int nType ){
	p->type = ( p->type & PATCH_STYLEMASK ) | nType;
}

void Patch_SetStyle( patchMesh_t *p, int nStyle ){
	p->type = ( p->type & PATCH_TYPEMASK ) | nStyle;
}

/*
   ==================
   Patch_MemorySize
   ==================
 */
int Patch_MemorySize( patchMesh_t *p ){
	//	return _msize(p);
	return 0;
}


/*
   ===============
   InterpolateInteriorPoints
   ===============
 */
void InterpolateInteriorPoints( patchMesh_t *p ){
	int i, j, k;
	int next, prev;

	for ( i = 0 ; i < p->width ; i += 2 )
	{

		next = ( i == p->width - 1 ) ? 1 : ( i + 1 ) % p->width;
		prev = ( i == 0 ) ? p->width - 2 : i - 1;

#if 0
		if ( i == 0 ) {
			next = ( i + 1 ) % p->width;
			prev = p->width - 2;              // joined wrap case
		}
		else if ( i == p->width - 1 ) {
			next = 1;
			prev = i - 1;
		}
		else
		{
			next = ( i + 1 ) % p->width;
			prev = i - 1;
		}
#endif

		for ( j = 0 ; j < p->height ; j++ )
		{
			for ( k = 0 ; k < 3 ; k++ )
			{
				p->ctrl[i][j].xyz[k] = ( p->ctrl[next][j].xyz[k] + p->ctrl[prev][j].xyz[k] ) * 0.5;
			}
		}
	}
}

/*
   =================
   MakeMeshNormals

   =================
 */
int neighbors[8][2] = {
	{0,1}, {1,1}, {1,0}, {1,-1}, {0,-1}, {-1,-1}, {-1,0}, {-1,1}
};

void Patch_MeshNormals( patchMesh_t *in ){
	int i, j, k, dist;
	vec3_t normal;
	vec3_t sum;
	int count;
	vec3_t base;
	vec3_t delta;
	int x, y;
	drawVert_t  *dv;
	vec3_t around[8], temp;
	qboolean good[8];
	qboolean wrapWidth, wrapHeight;
	float len;

	wrapWidth = false;
	for ( i = 0 ; i < in->height ; i++ )
	{

		VectorSubtract( in->ctrl[0][i].xyz,
						in->ctrl[in->width - 1][i].xyz, delta );
		len = VectorLength( delta );
		if ( len > 1.0 ) {
			break;
		}
	}
	if ( i == in->height ) {
		wrapWidth = true;
	}

	wrapHeight = false;
	for ( i = 0 ; i < in->width ; i++ )
	{
		VectorSubtract( in->ctrl[i][0].xyz,
						in->ctrl[i][in->height - 1].xyz, delta );
		len = VectorLength( delta );
		if ( len > 1.0 ) {
			break;
		}
	}
	if ( i == in->width ) {
		wrapHeight = true;
	}


	for ( i = 0 ; i < in->width ; i++ )
	{
		for ( j = 0 ; j < in->height ; j++ )
		{
			count = 0;
			//--dv = reinterpret_cast<drawVert_t*>(in.ctrl[j*in.width+i]);
			dv = &in->ctrl[i][j];
			VectorCopy( dv->xyz, base );
			for ( k = 0 ; k < 8 ; k++ )
			{
				VectorClear( around[k] );
				good[k] = false;

				for ( dist = 1 ; dist <= 3 ; dist++ )
				{
					x = i + neighbors[k][0] * dist;
					y = j + neighbors[k][1] * dist;
					if ( wrapWidth ) {
						if ( x < 0 ) {
							x = in->width - 1 + x;
						}
						else if ( x >= in->width ) {
							x = 1 + x - in->width;
						}
					}
					if ( wrapHeight ) {
						if ( y < 0 ) {
							y = in->height - 1 + y;
						}
						else if ( y >= in->height ) {
							y = 1 + y - in->height;
						}
					}

					if ( x < 0 || x >= in->width || y < 0 || y >= in->height ) {
						break;                  // edge of patch
					}
					//--VectorSubtract( in.ctrl[y*in.width+x]->xyz, base, temp );
					VectorSubtract( in->ctrl[x][y].xyz, base, temp );
					if ( __VectorNormalize( temp, temp ) == 0 ) {
						continue;               // degenerate edge, get more dist
					}
					else
					{
						good[k] = true;
						VectorCopy( temp, around[k] );
						break;                  // good edge
					}
				}
			}

			VectorClear( sum );
			for ( k = 0 ; k < 8 ; k++ )
			{
				if ( !good[k] || !good[( k + 1 ) & 7] ) {
					continue;   // didn't get two points
				}
				CrossProduct( around[( k + 1 ) & 7], around[k], normal );
				if ( __VectorNormalize( normal, normal ) == 0 ) {
					continue;
				}
				VectorAdd( normal, sum, sum );
				count++;
			}
			if ( count == 0 ) {
				//printf("bad normal\n");
				count = 1;
				//continue;
			}
			__VectorNormalize( sum, dv->normal );
		}
	}
}




/*
   ==================
   Patch_CalcBounds
   ==================
 */
void Patch_CalcBounds( patchMesh_t *p, vec3_t& vMin, vec3_t& vMax ){
	vMin[0] = vMin[1] = vMin[2] = 99999;
	vMax[0] = vMax[1] = vMax[2] = -99999;

	p->bDirty = true;
	for ( int w = 0; w < p->width; w++ )
	{
		for ( int h = 0; h < p->height; h++ )
		{
			for ( int j = 0; j < 3; j++ )
			{
				float f = p->ctrl[w][h].xyz[j];
				if ( f < vMin[j] ) {
					vMin[j] = f;
				}
				if ( f > vMax[j] ) {
					vMax[j] = f;
				}
			}
		}
	}
}

/*
   ==================
   Brush_RebuildBrush
   ==================
 */
void Brush_RebuildBrush( brush_t *b, vec3_t vMins, vec3_t vMaxs ){
	//
	// Total hack job
	// Rebuilds a brush
	int i, j;
	face_t  *f, *next;
	vec3_t pts[4][2];
	texdef_t texdef;
	// free faces

	for ( j = 0; j < 3; j++ )
	{
		if ( (int)vMins[j] == (int)vMaxs[j] ) {
			vMins[j] -= 4;
			vMaxs[j] += 4;
		}
	}


	for ( f = b->brush_faces ; f ; f = next )
	{
		next = f->next;
		if ( f ) {
			texdef = f->texdef;
		}
		Face_Free( f );
	}

	b->brush_faces = NULL;

	// left the last face so we can use its texdef

	for ( i = 0 ; i < 3 ; i++ )
		if ( vMaxs[i] < vMins[i] ) {
			Error( "Brush_RebuildBrush: backwards" );
		}

	pts[0][0][0] = vMins[0];
	pts[0][0][1] = vMins[1];

	pts[1][0][0] = vMins[0];
	pts[1][0][1] = vMaxs[1];

	pts[2][0][0] = vMaxs[0];
	pts[2][0][1] = vMaxs[1];

	pts[3][0][0] = vMaxs[0];
	pts[3][0][1] = vMins[1];

	for ( i = 0 ; i < 4 ; i++ )
	{
		pts[i][0][2] = vMins[2];
		pts[i][1][0] = pts[i][0][0];
		pts[i][1][1] = pts[i][0][1];
		pts[i][1][2] = vMaxs[2];
	}

	for ( i = 0 ; i < 4 ; i++ )
	{
		f = Face_Alloc();
		f->texdef = texdef;
		f->texdef.flags &= ~SURF_KEEP;
		f->texdef.contents &= ~CONTENTS_KEEP;
//		f->texdef.flags |= SURF_PATCH;
		f->next = b->brush_faces;
		b->brush_faces = f;
		j = ( i + 1 ) % 4;

		VectorCopy( pts[j][1], f->planepts[0] );
		VectorCopy( pts[i][1], f->planepts[1] );
		VectorCopy( pts[i][0], f->planepts[2] );
	}

	f = Face_Alloc();
	f->texdef = texdef;
	f->texdef.flags &= ~SURF_KEEP;
	f->texdef.contents &= ~CONTENTS_KEEP;
//  f->texdef.flags |= SURF_PATCH;
	f->next = b->brush_faces;
	b->brush_faces = f;

	VectorCopy( pts[0][1], f->planepts[0] );
	VectorCopy( pts[1][1], f->planepts[1] );
	VectorCopy( pts[2][1], f->planepts[2] );

	f = Face_Alloc();
	f->texdef = texdef;
	f->texdef.flags &= ~SURF_KEEP;
	f->texdef.contents &= ~CONTENTS_KEEP;
//  f->texdef.flags |= SURF_PATCH;
	f->next = b->brush_faces;
	b->brush_faces = f;

	VectorCopy( pts[2][0], f->planepts[0] );
	VectorCopy( pts[1][0], f->planepts[1] );
	VectorCopy( pts[0][0], f->planepts[2] );

	Brush_Build( b );
}

void WINAPI Patch_Rebuild( patchMesh_t *p ){
	vec3_t vMin, vMax;
	Patch_CalcBounds( p, vMin, vMax );
	Brush_RebuildBrush( p->pSymbiot, vMin, vMax );
	p->bDirty = true;
}

/*
   ==================
   AddBrushForPatch
   ==================
   adds a patch brush and ties it to this patch id
 */
brush_t* AddBrushForPatch( patchMesh_t *pm, bool bLinkToWorld ){
	// find the farthest points in x,y,z
	vec3_t vMin, vMax;
	Patch_CalcBounds( pm, vMin, vMax );

	for ( int j = 0; j < 3; j++ )
	{
		if ( vMin[j] == vMax[j] ) {
			vMin[j] -= 4;
			vMax[j] += 4;
		}
	}

	brush_t *b = Brush_Create( vMin, vMax, &g_qeglobals.d_texturewin.texdef );

	// FIXME: this entire type of linkage needs to be fixed
	b->patchBrush = true;
	b->pPatch = pm;
	pm->pSymbiot = b;
	pm->bSelected = false;
	pm->bOverlay = false;
	pm->bDirty = true;
	pm->nListID = -1;

	if ( bLinkToWorld ) {
		Brush_AddToList( b, &active_brushes );
		Entity_LinkBrush( world_entity, b );
		Brush_Build( b );
	}

	return b;
}

void Patch_SetPointIntensities( int n ){
#if 0
	patchMesh_t *p = patchMeshes[n];
	for ( int i = 0; i < p->width; i++ )
	{
		for ( int j = 0; j < p->height; j++ )
		{

		}
	}
#endif
}

// very approximate widths and heights

/*
   ==================
   Patch_Width
   ==================
 */
float Patch_Width( patchMesh_t *p ){
	float f = 0;
	for ( int i = 0; i < p->width - 1; i++ )
	{
		vec3_t vTemp;
		VectorSubtract( p->ctrl[i][0].xyz, p->ctrl[i + 1][0].xyz, vTemp );
		f += VectorLength( vTemp );
	}
	return f;
}

float Patch_WidthDistanceTo( patchMesh_t *p, int j ){
	float f = 0;
	for ( int i = 0; i < j; i++ )
	{
		vec3_t vTemp;
		VectorSubtract( p->ctrl[i][0].xyz, p->ctrl[i + 1][0].xyz, vTemp );
		f += VectorLength( vTemp );
	}
	return f;
}



/*
   ==================
   Patch_Height
   ==================
 */
float Patch_Height( patchMesh_t *p ){
	float f = 0;
	for ( int i = 0; i < p->height - 1; i++ )
	{
		vec3_t vTemp;
		VectorSubtract( p->ctrl[0][i].xyz, p->ctrl[0][i + 1].xyz, vTemp );
		f += VectorLength( vTemp );
	}
	return f;
}

float Patch_HeightDistanceTo( patchMesh_t *p, int j ){
	float f = 0;
	for ( int i = p->height - 1; i > j; i-- )
	{
		vec3_t vTemp;
		VectorSubtract( p->ctrl[0][i].xyz, p->ctrl[0][i - 1].xyz, vTemp ); // reverse order for T coords
		f += VectorLength( vTemp );
	}
	return f;
}



/*
   ==================
   Patch_Naturalize
   ==================
   texture = TotalTexture * LengthToThisControlPoint / TotalControlPointLength

   dist( this control point to first control point ) / dist ( last control pt to first)
 */
void WINAPI Patch_Naturalize( patchMesh_t *p ){
	int nWidth = (int)( p->d_texture->width * g_PrefsDlg.m_fDefTextureScale );
	int nHeight = (int)( p->d_texture->height * g_PrefsDlg.m_fDefTextureScale );
	float fPWidth = Patch_Width( p );
	float fPHeight = Patch_Height( p );
	float xAccum = 0.0f;

	for ( int i = 0; i < p->width ; i++ )
	{
		float yAccum = 0.0f;
		for ( int j = p->height - 1; j >= 0 ; j-- )
		{
			p->ctrl[i][j].st[0] = ( fPWidth / nWidth ) * xAccum / fPWidth;
			p->ctrl[i][j].st[1] = ( fPHeight / nHeight ) * yAccum / fPHeight;
			yAccum = Patch_HeightDistanceTo( p,j - 1 );
			//p->ctrl[i][j][3] = (fPWidth / nWidth) * (float)i / (p->width - 1);
			//p->ctrl[i][j][4] = (fPHeight/ nHeight) * (float)j / (p->height - 1);
		}
		xAccum = Patch_WidthDistanceTo( p,i + 1 );
	}
	p->bDirty = true;
}

/*
   if (bIBevel)
   {
    VectorCopy(p->ctrl[1][0], p->ctrl[1][1]);
   }

   if (bIEndcap)
   {
    VectorCopy(p->ctrl[3][0], p->ctrl[4][1]);
    VectorCopy(p->ctrl[2][0], p->ctrl[3][1]);
    VectorCopy(p->ctrl[2][0], p->ctrl[2][1]);
    VectorCopy(p->ctrl[2][0], p->ctrl[1][1]);
    VectorCopy(p->ctrl[1][0], p->ctrl[0][1]);
    VectorCopy(p->ctrl[1][0], p->ctrl[0][2]);
    VectorCopy(p->ctrl[1][0], p->ctrl[1][2]);
    VectorCopy(p->ctrl[2][0], p->ctrl[2][2]);
    VectorCopy(p->ctrl[3][0], p->ctrl[3][2]);
    VectorCopy(p->ctrl[3][0], p->ctrl[4][2]);
   }
 */

int Index3By[][2] =
{
	{0,0},
	{1,0},
	{2,0},
	{2,1},
	{2,2},
	{1,2},
	{0,2},
	{0,1},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0}
};

int Index5By[][2] =
{
	{0,0},
	{1,0},
	{2,0},
	{3,0},
	{4,0},
	{4,1},
	{4,2},
	{4,3},
	{4,4},
	{3,4},
	{2,4},
	{1,4},
	{0,4},
	{0,3},
	{0,2},
	{0,1}
};



int Interior3By[][2] =
{
	{1,1}
};

int Interior5By[][2] =
{
	{1,1},
	{2,1},
	{3,1},
	{1,2},
	{2,2},
	{3,2},
	{1,3},
	{2,3},
	{3,3}
};

int Interior3ByCount = sizeof( Interior3By ) / sizeof( int[2] );
int Interior5ByCount = sizeof( Interior5By ) / sizeof( int[2] );

extern int Plane_FromPoints( vec3_t p1, vec3_t p2, vec3_t p3, plane_t *plane );
// the bFaceCycle only means we are going through a patch cycling loop
// then we rely on g_vCycleCapNormal to compute the cap

void Patch_CapTexture( patchMesh_t *p, bool bFaceCycle = false ){
	vec3_t vProjection, vX, vY;
	qtexture_t *texture = p->pShader->getTexture();
	plane_t Plane1, Plane2, Plane3;
	bool bThing = true;

	if ( bFaceCycle ) {
		VectorCopy( g_vCycleCapNormal, vProjection );
	}

	else
	{
		VectorClear( vProjection );

		// find normal for plane from first 3 corner points
		if ( !Plane_FromPoints( p->ctrl[0][0].xyz,p->ctrl[0][p->height - 1].xyz,p->ctrl[p->width - 1][p->height - 1].xyz,&Plane1 ) ) {
			VectorClear( Plane3.normal );
			bThing = false;
		}

		// find normal for plane from next 3 corner points
		if ( !Plane_FromPoints( p->ctrl[p->width - 1][p->height - 1].xyz,p->ctrl[p->width - 1][0].xyz,p->ctrl[0][0].xyz,&Plane2 ) ) {
			if ( bThing ) {
				VectorCopy( Plane1.normal, Plane3.normal );
				Plane3.dist = Plane1.dist;
			}
		}

		else
		{
			if ( bThing ) {
				// find average plane for all 4 corner points
				for ( int n = 0; n <= 2; n++ )
				{
					Plane3.normal[n] = ( Plane1.normal[n] + Plane2.normal[n] ) / 2;
				}
				Plane3.dist = ( Plane1.dist + Plane2.dist ) / 2;
			}
			else
			{
				VectorCopy( Plane2.normal, Plane3.normal );
				Plane3.dist = Plane2.dist;
			}
		}

		// get best axis for projection from average plane
		//Sys_Printf("surface normal1: (%f,%f,%f)\n",Plane1.normal[0],Plane1.normal[1],Plane1.normal[0]);
		//Sys_Printf("surface normal2: (%f,%f,%f)\n",Plane2.normal[0],Plane2.normal[1],Plane2.normal[0]);
		//Sys_Printf("surface normal3: (%f,%f,%f)\n",Plane3.normal[0],Plane3.normal[1],Plane3.normal[0]);
		TextureAxisFromPlane( &Plane3, vX, vY );
	}

	for ( int w = 0; w < p->width; w++ )
	{
		for ( int h = 0; h < p->height; h++ )
		{
			if ( vProjection[2] == 1.0f || ( vX[0] == 1.0f && vY[1] == -1.0f ) ) {
				p->ctrl[w][h].st[0] = p->ctrl[w][h].xyz[0] / ( texture->width * g_PrefsDlg.m_fDefTextureScale );
				p->ctrl[w][h].st[1] = p->ctrl[w][h].xyz[1] / ( texture->height * g_PrefsDlg.m_fDefTextureScale ) * -1;
			}
			else if ( vProjection[0] == 1.0f || ( vX[1] == 1.0f && vY[2] == -1.0f ) ) {
				p->ctrl[w][h].st[0] = p->ctrl[w][h].xyz[1] / ( texture->width * g_PrefsDlg.m_fDefTextureScale );
				p->ctrl[w][h].st[1] = p->ctrl[w][h].xyz[2] / ( texture->height * g_PrefsDlg.m_fDefTextureScale ) * -1;
			}
			else if ( vProjection[1] == 1.0f || ( vX[0] == 1.0f && vY[2] == -1.0f ) ) {
				p->ctrl[w][h].st[0] = p->ctrl[w][h].xyz[0] / ( texture->width * g_PrefsDlg.m_fDefTextureScale );
				p->ctrl[w][h].st[1] = p->ctrl[w][h].xyz[2] / ( texture->height * g_PrefsDlg.m_fDefTextureScale ) * -1;
			}
			//Sys_Printf("(%i,%i) (%f,%f,%f) (%f,%f) %f\n",w,h,
			//	p->ctrl[w][h].xyz[0],p->ctrl[w][h].xyz[1],p->ctrl[w][h].xyz[2],
			//	p->ctrl[w][h].st[0],p->ctrl[w][h].st[1],p->ctrl[w][h].normal);
		}
	}
	// make sure it will rebuild
	p->bDirty = true;
}

void FillPatch( patchMesh_t *p, vec3_t v ){
	for ( int i = 0; i < p->width; i++ )
	{
		for ( int j = 0; j < p->height; j++ )
		{
			VectorCopy( v, p->ctrl[i][j].xyz );
		}
	}
}

// temporarily moved function to allow use in Cap() and CapSpecial()
void patchInvert( patchMesh_t *p ){
	drawVert_t vertTemp;
	p->bDirty = true;
	for ( int i = 0 ; i < p->width ; i++ )
	{
		for ( int j = 0; j < p->height / 2; j++ )
		{
			memcpy( &vertTemp, &p->ctrl[i][p->height - 1 - j], sizeof( drawVert_t ) );
			memcpy( &p->ctrl[i][p->height - 1 - j], &p->ctrl[i][j], sizeof( drawVert_t ) );
			memcpy( &p->ctrl[i][j], &vertTemp, sizeof( drawVert_t ) );
		}
	}
}

brush_t* Cap( patchMesh_t *pParent, bool bByColumn, bool bFirst ){
	brush_t *b;
	patchMesh_t *p;
	vec3_t vMin, vMax;
	int i, j;

	bool bSmall = true;
	// make a generic patch
	if ( pParent->width <= 9 ) {
		b = Patch_GenericMesh( 3, 3, 2, false );
	}
	else
	{
		b = Patch_GenericMesh( 5, 5, 2, false );
		bSmall = false;
	}

	if ( !b ) {
		Sys_Printf( "Unable to cap. You may need to ungroup the patch.\n" );
		return NULL;
	}

	p = b->pPatch;
	p->type |= PATCH_CAP;

	vMin[0] = vMin[1] = vMin[2] = 9999;
	vMax[0] = vMax[1] = vMax[2] = -9999;

	// we seam the column edge, FIXME: this might need to be able to seem either edge
	//
	int nSize = ( bByColumn ) ? pParent->width : pParent->height;
	int nIndex = ( bFirst ) ? 0 : ( bByColumn ) ? pParent->height - 1 : pParent->width - 1;

	FillPatch( p, pParent->ctrl[0][nIndex].xyz );

	for ( i = 0; i < nSize; i++ )
	{
		if ( bByColumn ) {
			if ( bSmall ) {
				VectorCopy( pParent->ctrl[i][nIndex].xyz, p->ctrl[Index3By[i][0]][Index3By[i][1]].xyz );
			}
			else
			{
				VectorCopy( pParent->ctrl[i][nIndex].xyz, p->ctrl[Index5By[i][0]][Index5By[i][1]].xyz );
			}
		}
		else
		{
			if ( bSmall ) {
				VectorCopy( pParent->ctrl[nIndex][i].xyz, p->ctrl[Index3By[i][0]][Index3By[i][1]].xyz );
			}
			else
			{
				VectorCopy( pParent->ctrl[nIndex][i].xyz, p->ctrl[Index5By[i][0]][Index5By[i][1]].xyz );
			}
		}

		for ( j = 0; j < 3; j++ )
		{
			float f = ( bSmall ) ? p->ctrl[Index3By[i][0]][Index3By[i][1]].xyz[j] : p->ctrl[Index5By[i][0]][Index5By[i][1]].xyz[j];
			if ( f < vMin[j] ) {
				vMin[j] = f;
			}
			if ( f > vMax[j] ) {
				vMax[j] = f;
			}
		}
	}

	vec3_t vTemp;
	for ( j = 0; j < 3; j++ )
	{
		vTemp[j] = vMin[j] + fabs( ( vMax[j] - vMin[j] ) * 0.5 );
	}
	int nCount = ( bSmall ) ? Interior3ByCount : Interior5ByCount;
	for ( j = 0; j < nCount; j++ )
	{
		if ( bSmall ) {
			VectorCopy( vTemp, p->ctrl[Interior3By[j][0]][Interior3By[j][1]].xyz );
		}
		else
		{
			VectorCopy( vTemp, p->ctrl[Interior5By[j][0]][Interior5By[j][1]].xyz );
		}
	}

	if ( bFirst ) {
		patchInvert( p );
	}
	/*
	   {
	   drawVert_t vertTemp;
	   for (i = 0; i < p->width; i++)
	   {
	    for (j = 0; j < p->height / 2; j++)
	    {
	      memcpy(&vertTemp, &p->ctrl[i][p->height - 1- j], sizeof (drawVert_t));
	      memcpy(&p->ctrl[i][p->height - 1 - j], &p->ctrl[i][j], sizeof(drawVert_t));
	      memcpy(&p->ctrl[i][j], &vertTemp, sizeof(drawVert_t));
	    }
	   }
	   }
	 */

	Patch_Rebuild( p );
	Patch_CapTexture( p );
	return p->pSymbiot;
}

brush_t* CapSpecial( patchMesh_t *pParent, int nType, bool bFirst ){

	brush_t *b;
	patchMesh_t *p;
	vec3_t vMin, vMax, vTemp;
	int i, j;

	if ( nType == IENDCAP ) {
		b = Patch_GenericMesh( 5, 3, 2, false );
	}
	else{
		b = Patch_GenericMesh( 3, 3, 2, false );
	}

	if ( !b ) {
		Sys_Printf( "Unable to cap. Make sure you ungroup before re-capping." );
		return NULL;
	}

	p = b->pPatch;
	p->type |= PATCH_CAP;

	vMin[0] = vMin[1] = vMin[2] = 9999;
	vMax[0] = vMax[1] = vMax[2] = -9999;

	//  int nSize = pParent->width;
	int nIndex = ( bFirst ) ? 0 : pParent->height - 1;

	// parent bounds are used for some things
	Patch_CalcBounds( pParent, vMin, vMax );

	for ( j = 0; j < 3; j++ )
	{
		vTemp[j] = vMin[j] + fabs( ( vMax[j] - vMin[j] ) * 0.5 );
	}

	if ( nType == IBEVEL ) {
		VectorCopy( pParent->ctrl[0][nIndex].xyz, p->ctrl[0][0].xyz );
		VectorCopy( pParent->ctrl[2][nIndex].xyz, p->ctrl[0][2].xyz );
		VectorCopy( pParent->ctrl[1][nIndex].xyz, p->ctrl[0][1].xyz );
		VectorCopy( pParent->ctrl[1][nIndex].xyz, p->ctrl[2][2].xyz );
		VectorCopy( pParent->ctrl[1][nIndex].xyz, p->ctrl[1][0].xyz );
		VectorCopy( pParent->ctrl[1][nIndex].xyz, p->ctrl[1][1].xyz );
		VectorCopy( pParent->ctrl[1][nIndex].xyz, p->ctrl[1][2].xyz );
		VectorCopy( pParent->ctrl[1][nIndex].xyz, p->ctrl[2][0].xyz );
		VectorCopy( pParent->ctrl[1][nIndex].xyz, p->ctrl[2][1].xyz );
	}
	else if ( nType == BEVEL ) {
		vec3_t p1, p2, p3, p4; //, temp, dir;

		VectorCopy( pParent->ctrl[0][nIndex].xyz, p3 );
		VectorCopy( pParent->ctrl[1][nIndex].xyz, p1 );
		VectorCopy( pParent->ctrl[2][nIndex].xyz, p2 );

		//Sys_Printf("CapSpecial() p1: %f %f %f\n",p1[0],p1[1],p1[2]);
		//Sys_Printf("CapSpecial() p2: %f %f %f\n",p2[0],p2[1],p2[2]);
		//Sys_Printf("CapSpecial() p3: %f %f %f\n",p3[0],p3[1],p3[2]);

		VectorSubtract( p2, p1, p4 );
		VectorAdd( p3, p4, p4 );
		// spog - use opposite-point-on-parallelogram to find p4
		/*
		   VectorSubtract(p3, p2, dir);
		   VectorNormalize(dir);
		   VectorSubtract(p1, p2, temp);
		   vec_t dist = _DotProduct(temp, dir);
		   VectorScale(dir, dist, temp);
		   VectorAdd(p2, temp, temp);
		   VectorSubtract(temp, p1, temp);
		   VectorScale(temp, 2, temp);
		   VectorAdd(p1, temp, p4);
		 */

		//Sys_Printf("CapSpecial() p1: %f %f %f\n",p1[0],p1[1],p1[2]);
		//Sys_Printf("CapSpecial() p2: %f %f %f\n",p2[0],p2[1],p2[2]);
		//Sys_Printf("CapSpecial() p3: %f %f %f\n",p3[0],p3[1],p3[2]);
		//Sys_Printf("CapSpecial() p4: %f %f %f\n",p4[0],p4[1],p4[2]);

		VectorCopy( p4, p->ctrl[0][0].xyz );
		VectorCopy( p4, p->ctrl[1][0].xyz );
		VectorCopy( p4, p->ctrl[0][1].xyz );
		VectorCopy( p4, p->ctrl[1][1].xyz );
		VectorCopy( p4, p->ctrl[0][2].xyz );
		VectorCopy( p4, p->ctrl[1][2].xyz );
		VectorCopy( p2, p->ctrl[2][0].xyz );
		VectorCopy( p1, p->ctrl[2][1].xyz );
		VectorCopy( p3, p->ctrl[2][2].xyz );

	}
	else if ( nType == ENDCAP ) {
		VectorAdd( pParent->ctrl[4][nIndex].xyz, pParent->ctrl[0][nIndex].xyz, vTemp );
		VectorScale( vTemp, 0.5, vTemp );
		VectorCopy( pParent->ctrl[0][nIndex].xyz, p->ctrl[0][0].xyz );
		VectorCopy( vTemp, p->ctrl[1][0].xyz );
		VectorCopy( pParent->ctrl[4][nIndex].xyz, p->ctrl[2][0].xyz );

		VectorCopy( pParent->ctrl[2][nIndex].xyz, p->ctrl[0][2].xyz );
		VectorCopy( pParent->ctrl[2][nIndex].xyz, p->ctrl[1][2].xyz );
		VectorCopy( pParent->ctrl[2][nIndex].xyz, p->ctrl[2][2].xyz );
		VectorCopy( pParent->ctrl[2][nIndex].xyz, p->ctrl[1][1].xyz );

		VectorCopy( pParent->ctrl[1][nIndex].xyz, p->ctrl[0][1].xyz );
		VectorCopy( pParent->ctrl[3][nIndex].xyz, p->ctrl[2][1].xyz );
	}
	else
	{
		VectorCopy( pParent->ctrl[4][nIndex].xyz, p->ctrl[0][0].xyz );
		VectorCopy( pParent->ctrl[3][nIndex].xyz, p->ctrl[1][0].xyz );
		VectorCopy( pParent->ctrl[2][nIndex].xyz, p->ctrl[2][0].xyz );
		VectorCopy( pParent->ctrl[1][nIndex].xyz, p->ctrl[3][0].xyz );
		VectorCopy( pParent->ctrl[0][nIndex].xyz, p->ctrl[4][0].xyz );

		VectorCopy( pParent->ctrl[3][nIndex].xyz, p->ctrl[0][1].xyz );
		VectorCopy( pParent->ctrl[3][nIndex].xyz, p->ctrl[1][1].xyz );
		VectorCopy( pParent->ctrl[2][nIndex].xyz, p->ctrl[2][1].xyz );
		VectorCopy( pParent->ctrl[1][nIndex].xyz, p->ctrl[3][1].xyz );
		VectorCopy( pParent->ctrl[1][nIndex].xyz, p->ctrl[4][1].xyz );

		VectorCopy( pParent->ctrl[3][nIndex].xyz, p->ctrl[0][2].xyz );
		VectorCopy( pParent->ctrl[3][nIndex].xyz, p->ctrl[1][2].xyz );
		VectorCopy( pParent->ctrl[2][nIndex].xyz, p->ctrl[2][2].xyz );
		VectorCopy( pParent->ctrl[1][nIndex].xyz, p->ctrl[3][2].xyz );
		VectorCopy( pParent->ctrl[1][nIndex].xyz, p->ctrl[4][2].xyz );
	}


	if ( !bFirst ) {
		drawVert_t vertTemp;
		for ( i = 0; i < p->width; i++ )
		{
			for ( j = 0; j < p->height / 2; j++ )
			{
				memcpy( &vertTemp, &p->ctrl[i][p->height - 1 - j], sizeof( drawVert_t ) );
				memcpy( &p->ctrl[i][p->height - 1 - j], &p->ctrl[i][j], sizeof( drawVert_t ) );
				memcpy( &p->ctrl[i][j], &vertTemp, sizeof( drawVert_t ) );
			}
		}
	}

	//--Patch_CalcBounds(p, vMin, vMax);
	//--Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
	Patch_Rebuild( p );
	Patch_CapTexture( p );
	return p->pSymbiot;
}

void Patch_CapCurrent(){
	patchMesh_t *pParent = NULL;
	brush_t *b[4];
	brush_t *pCap = NULL;
	b[0] = b[1] = b[2] = b[3] = NULL;
	int nIndex = 0;
	bool b_GroupResult = TRUE;

	if ( !QE_SingleBrush( true ) ) {
		Sys_Printf( "Patch_CapCurrent: you must have a single patch selected\n" );
		return;
	}


	for ( brush_t *pb = selected_brushes.next ; pb != NULL && pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			pParent = pb->pPatch;
			// decide which if any ends we are going to cap
			// if any of these compares hit, it is a closed patch and as such
			// the generic capping will work.. if we do not find a closed edge
			// then we need to ask which kind of cap to add
			if ( VectorCompare( pParent->ctrl[0][0].xyz, pParent->ctrl[pParent->width - 1][0].xyz ) ) {
				pCap = Cap( pParent, true, false );
				if ( pCap != NULL ) {
					b[nIndex++] = pCap;
				}
			}
			if ( VectorCompare( pParent->ctrl[0][pParent->height - 1].xyz, pParent->ctrl[pParent->width - 1][pParent->height - 1].xyz ) ) {
				pCap = Cap( pParent, true, true );
				if ( pCap != NULL ) {
					b[nIndex++] = pCap;
				}
			}
			if ( VectorCompare( pParent->ctrl[0][0].xyz, pParent->ctrl[0][pParent->height - 1].xyz ) ) {
				pCap = Cap( pParent, false, false );
				if ( pCap != NULL ) {
					b[nIndex++] = pCap;
				}
			}
			if ( VectorCompare( pParent->ctrl[pParent->width - 1][0].xyz, pParent->ctrl[pParent->width - 1][pParent->height - 1].xyz ) ) {
				pCap = Cap( pParent, false, true );
				if ( pCap != NULL ) {
					b[nIndex++] = pCap;
				}
			}
		}
	}

	if ( pParent ) {
		// if we did not cap anything with the above tests
		if ( nIndex == 0 ) {
			int type;

			if ( DoCapDlg( &type, &b_GroupResult ) == IDOK ) {
				b[nIndex++] = CapSpecial( pParent, type, false );
				b[nIndex++] = CapSpecial( pParent, type, true );
			}
		}

		if ( nIndex > 0 ) {
			while ( nIndex > 0 )
			{
				nIndex--;
				if ( b[nIndex] ) {
					Select_Brush( b[nIndex] );
				}
			}
			// Gef: Added toggle for capped patch func_group
			if ( b_GroupResult ) {
				entity_t *e = Entity_Alloc();
				SetKeyValue( e, "classname", "func_group" );
				SetKeyValue( e, "type", "patchCapped" );
				Select_GroupEntity( e );
				Entity_AddToList( e, &entities );
			}
		}
	}
}

/*
   ===============
   BrushToPatchMesh
   ===============
 */
void Patch_BrushToMesh( bool bCone, bool bBevel, bool bEndcap, bool bSquare, int nHeight ){
	brush_t     *b;
	patchMesh_t *p;
	int i,j;

	if ( !QE_SingleBrush() ) {
		return;
	}

	b = selected_brushes.next;

	p = MakeNewPatch();

	p->d_texture = b->brush_faces->d_texture;
	p->pShader = b->brush_faces->pShader;

	p->height = nHeight;

	p->type = PATCH_CYLINDER;
	if ( bBevel & !bSquare ) {
		p->type = PATCH_BEVEL;
		p->width = 3;
		int nStep = (int)( ( b->maxs[2] - b->mins[2] ) / ( p->height - 1 ) );
		int nStart = (int)( b->mins[2] );
		for ( i = 0; i < p->height; i++ )
		{
			p->ctrl[0][i].xyz[0] =  b->mins[0];
			p->ctrl[0][i].xyz[1] =  b->mins[1];
			p->ctrl[0][i].xyz[2] = nStart;

			p->ctrl[1][i].xyz[0] =  b->maxs[0];
			p->ctrl[1][i].xyz[1] =  b->mins[1];
			p->ctrl[1][i].xyz[2] = nStart;

			p->ctrl[2][i].xyz[0] =  b->maxs[0];
			p->ctrl[2][i].xyz[1] =  b->maxs[1];
			p->ctrl[2][i].xyz[2] = nStart;
			nStart += nStep;
		}
	}
	else if ( bEndcap & !bSquare ) {
		p->type = PATCH_ENDCAP;
		p->width = 5;
		int nStep = (int)( ( b->maxs[2] - b->mins[2] ) / ( p->height - 1 ) );
		int nStart = (int)( b->mins[2] );
		for ( i = 0; i < p->height; i++ )
		{
			p->ctrl[0][i].xyz[0] =  b->mins[0];
			p->ctrl[0][i].xyz[1] =  b->mins[1];
			p->ctrl[0][i].xyz[2] = nStart;

			p->ctrl[1][i].xyz[0] =  b->mins[0];
			p->ctrl[1][i].xyz[1] =  b->maxs[1];
			p->ctrl[1][i].xyz[2] = nStart;

			p->ctrl[2][i].xyz[0] =  b->mins[0] + ( ( b->maxs[0] - b->mins[0] ) * 0.5 );
			p->ctrl[2][i].xyz[1] =  b->maxs[1];
			p->ctrl[2][i].xyz[2] = nStart;

			p->ctrl[3][i].xyz[0] =  b->maxs[0];
			p->ctrl[3][i].xyz[1] =  b->maxs[1];
			p->ctrl[3][i].xyz[2] = nStart;

			p->ctrl[4][i].xyz[0] =  b->maxs[0];
			p->ctrl[4][i].xyz[1] =  b->mins[1];
			p->ctrl[4][i].xyz[2] = nStart;
			nStart += nStep;
		}
	}
	else
	{
		p->width = 9;
		p->ctrl[1][0].xyz[0] =  b->mins[0];
		p->ctrl[1][0].xyz[1] =  b->mins[1];

		p->ctrl[3][0].xyz[0] =  b->maxs[0];
		p->ctrl[3][0].xyz[1] =  b->mins[1];

		p->ctrl[5][0].xyz[0] =  b->maxs[0];
		p->ctrl[5][0].xyz[1] =  b->maxs[1];

		p->ctrl[7][0].xyz[0] =  b->mins[0];
		p->ctrl[7][0].xyz[1] =  b->maxs[1];

		for ( i = 1 ; i < p->width - 1 ; i += 2 )
		{

			p->ctrl[i][0].xyz[2] =  b->mins[2];

			VectorCopy( p->ctrl[i][0].xyz, p->ctrl[i][2].xyz );

			p->ctrl[i][2].xyz[2] =  b->maxs[2];

			p->ctrl[i][1].xyz[0] = ( p->ctrl[i][0].xyz[0] + p->ctrl[i][2].xyz[0] ) * 0.5;
			p->ctrl[i][1].xyz[1] = ( p->ctrl[i][0].xyz[1] + p->ctrl[i][2].xyz[1] ) * 0.5;
			p->ctrl[i][1].xyz[2] = ( p->ctrl[i][0].xyz[2] + p->ctrl[i][2].xyz[2] ) * 0.5;
		}
		InterpolateInteriorPoints( p );

		if ( bSquare ) {
			if ( bBevel || bEndcap ) {
				if ( bBevel ) {
					for ( i = 0; i < p->height; i++ )
					{
						VectorCopy( p->ctrl[1][i].xyz, p->ctrl[2][i].xyz );
						VectorCopy( p->ctrl[7][i].xyz, p->ctrl[6][i].xyz );
					}
				}
				else
				{
					for ( i = 0; i < p->height; i++ )
					{
						VectorCopy( p->ctrl[5][i].xyz, p->ctrl[4][i].xyz );
						VectorCopy( p->ctrl[1][i].xyz, p->ctrl[2][i].xyz );
						VectorCopy( p->ctrl[7][i].xyz, p->ctrl[6][i].xyz );
						VectorCopy( p->ctrl[8][i].xyz, p->ctrl[7][i].xyz );
					}
				}
			}
			else
			{
				for ( i = 0; i < p->width - 1; i++ )
				{
					for ( j = 0; j < p->height; j++ )
					{
						VectorCopy( p->ctrl[i + 1][j].xyz, p->ctrl[i][j].xyz );
					}
				}
				for ( j = 0; j < p->height; j++ )
				{
					VectorCopy( p->ctrl[0][j].xyz, p->ctrl[8][j].xyz );
				}
			}
		}
	}


	Patch_Naturalize( p );

	if ( bCone ) {
		p->type = PATCH_CONE;
		float xc = ( b->maxs[0] + b->mins[0] ) * 0.5;
		float yc = ( b->maxs[1] + b->mins[1] ) * 0.5;

		for ( i = 0 ; i < p->width ; i++ )
		{
			p->ctrl[i][2].xyz[0] = xc;
			p->ctrl[i][2].xyz[1] = yc;
		}
	}

	b = AddBrushForPatch( p );

	Select_Delete();
	Select_Brush( b );

}

/*
   ==================
   Patch_GenericMesh
   ==================
 */
brush_t* Patch_GenericMesh( int nWidth, int nHeight, int nOrientation, bool bDeleteSource, bool bOverride ){
	int i,j;

	if ( nHeight < 3 || nHeight > 15 || nWidth < 3 || nWidth > 15 ) {
		Sys_Printf( "Invalid patch width or height.\n" );
		return NULL;
	}

	if ( !bOverride && !QE_SingleBrush() ) {
		Sys_FPrintf( SYS_ERR, "ERROR: you must have a single brush selected\n" );
		return NULL;
	}

	patchMesh_t* p = MakeNewPatch();
	p->pShader = g_qeglobals.d_texturewin.pShader;
	p->d_texture = g_qeglobals.d_texturewin.pShader->getTexture();

	p->width = nWidth;
	p->height = nHeight;
	p->type = PATCH_GENERIC;

	int nFirst = 0;
	int nSecond = 1;
	if ( nOrientation == 0 ) {
		nFirst = 1;
		nSecond = 2;
	}
	else if ( nOrientation == 1 ) {
		nSecond = 2;
	}

	brush_t *b = selected_brushes.next;
	// set the workzone to this brush, use it later to create the patch points
	UpdateWorkzone_ForBrush( b );

	int xStep = (int)( b->mins[nFirst] );
	float xAdj = fabs( ( b->maxs[nFirst] - b->mins[nFirst] ) / ( nWidth - 1 ) );
	float yAdj = fabs( ( b->maxs[nSecond] - b->mins[nSecond] ) / ( nHeight - 1 ) );

	for ( i = 0; i < nWidth; i++ )
	{
		int yStep = (int)( b->mins[nSecond] );
		for ( j = 0; j < nHeight; j++ )
		{
			p->ctrl[i][j].xyz[nFirst] = xStep;
			p->ctrl[i][j].xyz[nSecond] = yStep;
			// create patch based on workzone
			p->ctrl[i][j].xyz[nOrientation] = g_qeglobals.d_work_max[nOrientation];
			yStep += (int)yAdj;
		}
		xStep += (int)xAdj;
	}

	Patch_Naturalize( p );

	b = AddBrushForPatch( p );
	if ( bDeleteSource ) {
		Select_Delete();
		Select_Brush( b );
	}

	return b;
	//g_qeglobals.d_select_mode = sel_curvepoint;
}

/*
   ==================
   PointInMoveList
   ==================
 */
int PointInMoveList( float *pf ){
	for ( int i = 0; i < g_qeglobals.d_num_move_points; i++ )
	{
		if ( pf == &g_qeglobals.d_move_points[i][0] ) {
			return i;
		}
	}
	return -1;
}

/*
   ==================
   PointValueInMoveList
   ==================
 */
int PointValueInMoveList( vec3_t v ){
	for ( int i = 0; i < g_qeglobals.d_num_move_points; i++ )
	{
		if ( VectorCompare( v, g_qeglobals.d_move_points[i] ) ) {
			return i;
		}
	}
	return -1;
}


/*
   ==================
   RemovePointFromMoveList
   ==================
 */
void RemovePointFromMoveList( vec3_t v ){
	int n;
	while ( ( n = PointValueInMoveList( v ) ) >= 0 )
	{
		for ( int i = n; i < g_qeglobals.d_num_move_points - 1; i++ )
		{
			g_qeglobals.d_move_points[i] = g_qeglobals.d_move_points[i + 1];
		}
		g_qeglobals.d_num_move_points--;
	}
}

/*
   ==================
   ColumnSelected
   ==================
 */
bool ColumnSelected( patchMesh_t* p, int nCol ){
	for ( int i = 0; i < p->height; i++ )
	{
		if ( PointInMoveList( p->ctrl[nCol][i].xyz ) == -1 ) {
			return false;
		}
	}
	return true;
}

/*
   ==================
   AddPoint
   ==================
 */
void AddPoint( patchMesh_t* p, vec3_t v, bool bWeldOrDrill = true ){
	int nDim1 = ( g_pParentWnd->ActiveXY()->GetViewType() == YZ ) ? 1 : 0;
	int nDim2 = ( g_pParentWnd->ActiveXY()->GetViewType() == XY ) ? 1 : 2;
	g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = v;
	if ( ( g_bPatchWeld || g_bPatchDrillDown ) && bWeldOrDrill ) {
		for ( int i = 0 ; i < p->width ; i++ )
		{
			for ( int j = 0 ; j < p->height ; j++ )
			{
				if ( g_bPatchWeld ) {
					if ( VectorCompare( v, p->ctrl[i][j].xyz )
						 && PointInMoveList( p->ctrl[i][j].xyz ) == -1 ) {
						g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = p->ctrl[i][j].xyz;
						continue;
					}
				}
				if ( g_bPatchDrillDown && g_nPatchClickedView != W_CAMERA ) {
					if ( ( fabs( v[nDim1] - p->ctrl[i][j].xyz[nDim1] ) <= EQUAL_EPSILON )
						 && ( fabs( v[nDim2] - p->ctrl[i][j].xyz[nDim2] ) <= EQUAL_EPSILON ) ) {
						if ( PointInMoveList( p->ctrl[i][j].xyz ) == -1 ) {
							g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = p->ctrl[i][j].xyz;
							continue;
						}
					}
				}
			}
		}
	}
}

/*
   ==================
   SelectRow
   ==================
 */
void SelectRow( patchMesh_t* p, int nRow, bool bMulti ){
	if ( !bMulti ) {
		g_qeglobals.d_num_move_points = 0;
	}
	for ( int i = 0; i < p->width; i++ )
	{
		AddPoint( p, p->ctrl[i][nRow].xyz, false );
	}
	//Sys_Printf("Selected Row %d\n", nRow);
}

/*
   ==================
   SelectColumn
   ==================
 */
void SelectColumn( patchMesh_t* p, int nCol, bool bMulti ){
	if ( !bMulti ) {
		g_qeglobals.d_num_move_points = 0;
	}
	for ( int i = 0; i < p->height; i++ )
	{
		AddPoint( p, p->ctrl[nCol][i].xyz, false );
	}
	//Sys_Printf("Selected Col %d\n", nCol);
}


/*
   ==================
   AddPatchMovePoint
   ==================
 */
void AddPatchMovePoint( vec3_t v, bool bMulti, bool bFull ){
	if ( !g_bSameView && !bMulti && !bFull ) {
		g_bSameView = true;
		//return; // was causing odd behaviour on patch vertex selection
	}

	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			patchMesh_t* p = pb->pPatch;
			for ( int i = 0 ; i < p->width ; i++ )
			{
				for ( int j = 0 ; j < p->height ; j++ )
				{
					if ( VectorCompare( v, p->ctrl[i][j].xyz ) ) {
						if ( PointInMoveList( p->ctrl[i][j].xyz ) == -1 ) {
							if ( bFull ) { // if we want the full row/col this is on
								SelectColumn( p, i, bMulti );
							}
							else
							{
								if ( !bMulti ) {
									g_qeglobals.d_num_move_points = 0;
								}
								AddPoint( p, p->ctrl[i][j].xyz );
								//Sys_Printf("Selected col:row %d:%d\n", i, j);
							}
							//--if (!bMulti)
							return;
						}
						else
						{
							if ( bFull ) {
								if ( ColumnSelected( p, i ) ) {
									SelectRow( p, j, bMulti );
								}
								else
								{
									SelectColumn( p, i, bMulti );
								}
								return;
							}
							//if (!bMulti)
							//{
							//    g_qeglobals.d_num_move_points = 0;
							//    AddPoint(p, p->ctrl[i][j].xyz);
							//}
							if ( bMulti ) { // if (g_bSameView) // this is not having desired effect
								RemovePointFromMoveList( v );
								return;
							}
						}
					}
				}
			}
		}
	}
}

/*
   ==================
   Patch_UpdateSelected
   ==================
 */
void Patch_UpdateSelected( vec3_t vMove ){
	int i; //, j;
	for ( i = 0 ; i < g_qeglobals.d_num_move_points ; i++ )
	{
		VectorAdd( g_qeglobals.d_move_points[i], vMove, g_qeglobals.d_move_points[i] );
		if ( g_qeglobals.d_num_move_points == 1 ) {
		}
	}

	//--patchMesh_t* p = &patchMeshes[g_nSelectedPatch];
	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			patchMesh_t* p = pb->pPatch;

#if 0 //moving to SelectCurvePointByRay
			g_qeglobals.d_numpoints = 0;
			for ( i = 0 ; i < p->width ; i++ )
			{
				for ( j = 0 ; j < p->height ; j++ )
				{
					VectorCopy( p->ctrl[i][j].xyz, g_qeglobals.d_points[g_qeglobals.d_numpoints] );
					if ( g_qeglobals.d_numpoints < MAX_POINTS - 1 ) {
						g_qeglobals.d_numpoints++;
					}
				}
			}
#endif
			vec3_t vMin, vMax;
			Patch_CalcBounds( p, vMin, vMax );
			Brush_RebuildBrush( p->pSymbiot, vMin, vMax );
		}
	}
	//Brush_Free(p->pSymbiot);
	//Select_Brush(AddBrushForPatch(g_nSelectedPatch));
}



/*
   ===============
   SampleSinglePatch
   ===============
 */
void SampleSinglePatch( float ctrl[3][3][5], float u, float v, float out[5] ) {
	float vCtrl[3][5];
	int vPoint;
	int axis;

	// find the control points for the v coordinate
	for ( vPoint = 0 ; vPoint < 3 ; vPoint++ )
	{
		for ( axis = 0 ; axis < 5 ; axis++ )
		{
			float a, b, c;
			float qA, qB, qC;

			a = ctrl[0][vPoint][axis];
			b = ctrl[1][vPoint][axis];
			c = ctrl[2][vPoint][axis];
			qA = a - 2 * b + c;
			qB = 2 * b - 2 * a;
			qC = a;

			vCtrl[vPoint][axis] = qA * u * u + qB * u + qC;
		}
	}

	// interpolate the v value
	for ( axis = 0 ; axis < 5 ; axis++ )
	{
		float a, b, c;
		float qA, qB, qC;

		a = vCtrl[0][axis];
		b = vCtrl[1][axis];
		c = vCtrl[2][axis];
		qA = a - 2 * b + c;
		qB = 2 * b - 2 * a;
		qC = a;

		out[axis] = qA * v * v + qB * v + qC;
	}
}

//spog - Curve LOD stuff starts

float ShadeForNormal( vec3_t normal ){
	float f;

	vec3_t L;
	L[0] = 1.0f;
	L[1] = 1.0f;
	L[2] = 1.0f;


	// quick diffuse shading
	f = DotProduct( L, normal );

	// range 0.5 to 1.0
	f = ( f + 1 ) / 4.0f;
	//if (f < 0.0f) f = 0.0f;

	f += 0.5f;

	return f;
}

void ShadeVertex( drawVert_t &p ){
	p.lightmap[0] = ShadeForNormal( p.normal );
}


void Patch_DrawNormals( patchMesh_t *patch ){
	int row, col;
	vec3_t vNormal;

	qglBegin( GL_LINES );
	for ( col = 0; col < patch->width; col++ )
	{
		for ( row = 0; row < patch->height; row++ )
		{
			VectorAdd( patch->ctrl[col][row].xyz, patch->ctrl[col][row].normal, vNormal );
			qglVertex3fv( patch->ctrl[col][row].xyz );
			qglVertex3fv( vNormal );
		}
	}
	qglEnd();
}


// take an array of three drawVerts, and the addresses of three more drawVerts
// interpolate new XYZST values from the three drawVerts, these are:
// the left sub-control-point, the right sub-control-point and the midpoint of the curve respectively
// store these values in the drawVerts passed to the function
void Patch_CurveSplit( drawVert_t *vCurve[3], drawVert_t &pLeft, drawVert_t &pRight, drawVert_t &pMid, float u ){
	int i;
	//float u = 0.5f;
//	float a, b;
	drawVert_t v1, v2, v3;
//	vec3_t v4;

	for ( i = 0; i < 3; i++ )
	{
		// xyz
		v1.xyz[i] = vCurve[1]->xyz[i] - vCurve[0]->xyz[i];
		v2.xyz[i] = vCurve[2]->xyz[i] - vCurve[1]->xyz[i];
		v1.xyz[i] *= u;
		v2.xyz[i] *= u;
		pLeft.xyz[i] = vCurve[0]->xyz[i] + v1.xyz[i];
		pRight.xyz[i] = vCurve[1]->xyz[i] + v2.xyz[i];

		v3.xyz[i] = pRight.xyz[i] - pLeft.xyz[i];
		v3.xyz[i] *= u;
		pMid.xyz[i] = pLeft.xyz[i] + v3.xyz[i];

		// normal (weighted average) // no, that's b0rked
		//a = 1 / u; // total
		//b = u * a; // component 2
		//a = u - b; // component 1
		//pMid.normal[i] = u * ((vCurve[0]->normal[i] * b) + (vCurve[2]->normal[i] * a));

		if ( i == 2 ) {
			continue;
		}

		// st
		v1.st[i] = vCurve[1]->st[i] - vCurve[0]->st[i];
		v2.st[i] = vCurve[2]->st[i] - vCurve[1]->st[i];
		v1.st[i] *= u;
		v2.st[i] *= u;
		pLeft.st[i] = vCurve[0]->st[i] + v1.st[i];
		pRight.st[i] = vCurve[1]->st[i] + v2.st[i];

		v3.st[i] = pRight.st[i] - pLeft.st[i];
		v3.st[i] *= u;
		pMid.st[i] = pLeft.st[i] + v3.st[i];
	}
}

// take an array of three points, return an index representing the curvature of those three points
// return zero if the curve is a straight line, unless the midpoint is not between the endpoints
float Patch_CurveIndex( vec3_t vCurve[] ){
	vec3_t vTemp, v1, v2, v3, vClear;
//	int i;
	float width, angle;
	float index, dot;

	VectorClear( vClear );

	VectorSubtract( vCurve[2], vCurve[0], vTemp );
	VectorSubtract( vCurve[1], vCurve[0], v1 );
	VectorSubtract( vCurve[2], vCurve[1], v2 );

	if ( VectorCompare( v1, vClear ) || VectorCompare( vTemp, v1 ) ) { // return 0 if 1->2 == 0 or 1->2 == 1->3
		return 0.0f;
	}

	VectorNormalize( v1, v1 );
	VectorNormalize( v2, v2 );
	if ( VectorCompare( v1, v2 ) ) {
		return 0.0f;
	}

	VectorCopy( vTemp, v3 );
	width = VectorNormalize( v3, v3 );

	if ( VectorCompare( v1, v3 ) && VectorCompare( v2, v3 ) ) {
		return 0.0f;
	}

	dot = DotProduct( v1, v2 );

	angle = acos( dot ) / Q_PI;

	index = width * angle;

	return index;
}


// create a new tree root, give it the coordinate values of the drawVert
// return a pointer to the new tree root
BTNode_t *BTree_Create( drawVert_t info ){
	BTNode_t *BTree = new BTNode_t;
	BTree->left = BTree->right = NULL;
	VectorCopy( info.xyz, BTree->info.xyz );
	VectorCopy( info.xyz, BTree->vMid.xyz );
	for ( int i = 0; i < 2; i++ )
	{
		BTree->info.st[i] = info.st[i];
		BTree->vMid.st[i] = info.st[i];
	}
	return BTree;
}

// take ownership of the subtree
// delete the entire subtree
// return a NULL pointer
BTNode_t *BTree_Delete( BTNode_t *pBT ){
	if ( pBT != NULL ) {
		BTree_Delete( pBT->left );
		BTree_Delete( pBT->right );
		delete pBT;
	}
	return NULL;
}

// NOT currently used
BTNode_t *BTree_Clear( BTNode_t *pBT, bool bFirst = true ){
	if ( pBT != NULL ) {
		BTree_Clear( pBT->left, false );
		BTree_Clear( pBT->right, false );
		if ( !bFirst ) {
			delete pBT;
		}
	}
	return pBT;
}

// take a pointer to the last item added to the list (this can also be a NULL pointer)
// take a pointer to the root of a subtree, and the patch points to the left and right of it
// add a new item to the subtree list, and add the subtree and its adjacent points to the new item
// return a pointer to the last item added to the subtree list
BTreeList_t *BTree_AddToList( BTreeList_t *pBTList, BTNode_t *pBT, drawVert_t &pLeft, drawVert_t &pRight ){
	BTreeList_t *newBTList = new BTreeList_t;
	newBTList->next = pBTList;
	newBTList->pBT = pBT;
	VectorCopy( pLeft.xyz, newBTList->vLeft.xyz );
	VectorCopy( pRight.xyz, newBTList->vRight.xyz );
	VectorCopy( pLeft.normal, newBTList->vLeft.normal );
	VectorCopy( pRight.normal, newBTList->vRight.normal );
	for ( int i = 0; i < 2; i++ )
	{
		newBTList->vLeft.st[i] = pLeft.st[i];
		newBTList->vRight.st[i] = pRight.st[i];
	}
	return newBTList;
}

// NOT currently used, subtrees are now stored on the patch
// take ownership of the subtree list
// delete the entire list and the subtrees it points to
// return a NULL pointer
BTreeList_t *BTree_DeleteList( BTreeList_t *pBTList ){
	if ( pBTList != NULL ) {
		BTree_DeleteList( pBTList->next );
		pBTList->pBT = BTree_Delete( pBTList->pBT );
		delete pBTList;
	}
	return NULL;
}

// take ownership of the subtree list
// delete the entire subtree list, but not the subtrees themselves
// return a NULL pointer
BTreeList_t *BTree_DeletePointerList( BTreeList_t *pBTList ){
	if ( pBTList != NULL ) {
		BTree_DeletePointerList( pBTList->next );
		delete pBTList;
	}
	return NULL;
}

// take a pointer to the last item added to the list of subtree lists
// add a subtree list to the list
// return a pointer to the last item added
BTListList_t *BTree_AddListToList( BTListList_t *pBTListList, BTreeList_t *pBTList ){
	BTListList_t *newBTListList = new BTListList_t;
	newBTListList->next = pBTListList;
	newBTListList->list = pBTList;
	return newBTListList;
}


// take ownership of the list of subtree lists
// delete the entire list of lists, but not the subtrees themselves
// return a NULL pointer
BTListList_t *BTree_DeleteListFromList( BTListList_t *pBTListList ){
	if ( pBTListList != NULL ) {
		BTree_DeleteListFromList( pBTListList->next );
		pBTListList->list = BTree_DeletePointerList( pBTListList->list );
		delete pBTListList;
	}
	return NULL;
}

// take a pointer to the last item in the list
// add a NULL linker subtree to the list, setting the "flipped" flag using the left curvepoint normal .. er.. hacky?
BTreeList_t *BTree_AddLinkToList( BTreeList_t *pBTList, bool bFlipped = false ){
	BTreeList_t *linkBTList = new BTreeList_t;
	linkBTList->pBT = NULL;
	linkBTList->next = pBTList;
	linkBTList->vLeft.normal[0] = ( bFlipped ) ? 1.0f : 0.0f;
	return linkBTList;
}


// take an array of three points and the address of a vector
// store midpoint of the bezier curve formed by the three points, in the vector
void Patch_BezierInterpolate( vec3_t vCurve[], vec3_t &pMid ){
	vec3_t vTemp;
	int i;
	VectorSubtract( vCurve[2], vCurve[0], vTemp ); // Start->End
	for ( i = 0; i < 3; i++ )
		vTemp[i] /= 2;
	VectorAdd( vCurve[0], vTemp, vTemp ); // midpoint of Start->End

	VectorSubtract( vTemp, vCurve[1], vTemp ); // Mid->(midpoint of Start->End)
	for ( i = 0; i < 3; i++ )
		vTemp[i] /= 2;
	VectorAdd( vCurve[1], vTemp, pMid ); // midpoint of Mid->(midpoint of Start->End)
}


// take a pointer to the list of subtrees, and a threshold value
// generate REAL surface curvature for the subtree curves, using bezier interpolation
// if any of the real curves has an index greater than the threshold, return true
bool Patch_MostCurvedRow( BTreeList_t *pBTList, int threshold ){
	BTreeList_t *p;
	float index; //, bestindex = 0;
	vec3_t vCurve[3];
	vec3_t vRow[3];
//	int i;

	for ( p = pBTList; p != NULL; p = p->next->next )
	{
		// this row
		VectorCopy( p->vLeft.xyz, vCurve[0] );
		VectorCopy( p->pBT->info.xyz, vCurve[1] );
		VectorCopy( p->vRight.xyz, vCurve[2] );

		index = Patch_CurveIndex( vCurve );
		if ( index > threshold ) {
			return true;
		}

		if ( p->next == NULL ) {
			break;
		}

		if ( p->next->pBT == NULL ) {
			continue;
		}

		VectorCopy( p->vLeft.xyz, vCurve[0] );
		VectorCopy( p->next->vLeft.xyz, vCurve[1] );
		VectorCopy( p->next->next->vLeft.xyz, vCurve[2] );
		Patch_BezierInterpolate( vCurve, vRow[0] );

		VectorCopy( p->pBT->info.xyz, vCurve[0] );
		VectorCopy( p->next->pBT->info.xyz, vCurve[1] );
		VectorCopy( p->next->next->pBT->info.xyz, vCurve[2] );
		Patch_BezierInterpolate( vCurve, vRow[1] );

		VectorCopy( p->vRight.xyz, vCurve[0] );
		VectorCopy( p->next->vRight.xyz, vCurve[1] );
		VectorCopy( p->next->next->vRight.xyz, vCurve[2] );
		Patch_BezierInterpolate( vCurve, vRow[2] );

		index = Patch_CurveIndex( vRow );
		if ( index > threshold ) {
			return true;
		}
	}
	return false;
}


// take a pointer to a list of subtrees.. each subtree in the list is a 3-point bezier curve formed by two endpoints owned by the list, and a midpoint subtree node owned by a patch.
// if any of the subtrees are curved above a threshold, create a left and right subsubtree for each subtree in the list.
// if a NULL linker subtree is found, check for an orientation flip - ie. an inverted LOD-match - and create a NULL subsubtree with the same orientation flip
// this effectively generates trees for multiple patches at the same time.. the subtrees are always owned by their respective patches though
void BTree_ListCurveRecurse( BTreeList_t *pBTList ){
	BTreeList_t *p;
	BTreeList_t *leftBTList, *rightBTList;
	//drawVert_t pLeft, pRight, pMid;
	drawVert_t *vCurve[3];
	int threshold;
	//int i;
	bool bFlipped = false;

	if ( g_PrefsDlg.m_nSubdivisions >= 1 ) {
		threshold = g_PrefsDlg.m_nSubdivisions;
	}
	else{
		threshold = 0;
	}

	leftBTList = rightBTList = NULL;

	if ( Patch_MostCurvedRow( pBTList, threshold ) ) { // split all subtrees in list if any subtree is above threshold
		//Sys_Printf("| ");
		// traverse nodes in list
		for ( p = pBTList; p != NULL; p = p->next )
		{
			if ( p->pBT == NULL ) {
				leftBTList = BTree_AddLinkToList( leftBTList, ( p->vLeft.normal[0] == 1.0f ) );
				rightBTList = BTree_AddLinkToList( rightBTList, ( p->vLeft.normal[0] == 1.0f ) );
				if ( p->vLeft.normal[0] == 1.0f ) {
					bFlipped = ( !bFlipped ) ? true : false;                           // switch bFlipped if true
				}
				continue;
			}

			// create left node for this subtree
			BTNode_t *newLeft = new BTNode_t;
			p->pBT->left = newLeft;
			newLeft->left = newLeft->right = NULL;

			// create right node for this subtree
			BTNode_t *newRight = new BTNode_t;
			p->pBT->right = newRight;
			newRight->left = newRight->right = NULL;

			// split this node
			vCurve[0] = &p->vLeft;
			vCurve[1] = &p->pBT->info;
			vCurve[2] = &p->vRight;
			Patch_CurveSplit( vCurve, newLeft->info, newRight->info, p->pBT->vMid, 0.5 );

			memcpy( &newLeft->vMid, &newLeft->info, sizeof( drawVert_t ) );
			memcpy( &newRight->vMid, &newRight->info, sizeof( drawVert_t ) );


			if ( !bFlipped ) {
				// add new left subtree to left subtree list
				leftBTList = BTree_AddToList( leftBTList, newLeft, p->vLeft, p->pBT->vMid );

				// add new right subtree to right subtree list
				rightBTList = BTree_AddToList( rightBTList, newRight, p->pBT->vMid, p->vRight );
			}
			else
			{
				// add new left subtree to right subtree list
				rightBTList = BTree_AddToList( rightBTList, newLeft, p->vLeft, p->pBT->vMid );

				// add new right subtree to left subtree list
				leftBTList = BTree_AddToList( leftBTList, newRight, p->pBT->vMid, p->vRight );
			}
		}

		// continue tree left
		BTree_ListCurveRecurse( leftBTList );
		leftBTList = BTree_DeletePointerList( leftBTList );

		// continue tree right
		BTree_ListCurveRecurse( rightBTList );
		rightBTList = BTree_DeletePointerList( rightBTList );
	}
}

// take mins and maxs values from two brushes
// return true if they intersect on every axis
bool TouchingAABBs( vec3_t mins1, vec3_t maxs1, vec3_t mins2, vec3_t maxs2 ){
	//bool xyz[3];
	vec3_t v1, v2, p1, p2, T;
	for ( int i = 0; i < 3; i++ )
	{
		v1[i] = maxs1[i] - mins1[i];
		v2[i] = maxs2[i] - mins2[i];
		v1[i] /= 2;
		v2[i] /= 2;
		p1[i] = mins1[i] + v1[i];
		p2[i] = mins2[i] + v2[i];
		// p1 == origin of aabb1
		// p2 == origin of aabb1
		// v1 == displacement of aabb1
		// v1 == displacement of aabb2
		T[i] = p2[i] - p1[i]; // T == vector from aabb1 to aabb2
		if ( fabs( T[i] ) > ( fabs( v1[i] ) + fabs( v2[i] ) ) ) {
			return false;
		}
	}
	return true;
}

// take a pointer to the last item added to pBTList, a pointer to the patch, a row index (start) and a column index
// generate a row of row-curve tree roots, owned by the patch and add the entire column of row-curves to the list, using the row index to decide the order to add
// return a pointer to the last item added to the list
BTreeList_t *Patch_CreateBTListForRows( BTreeList_t *pBTList, patchMesh_t *patch, int start, int col ){
	int row, pos;
	patch->colDirty[( col - 1 ) / 2] = true;

	if ( start == 0 ) {
		for ( row = 0; row < patch->height; row++ )
		{
			pos = ( ( ( col - 1 ) / 2 ) * patch->height ) + row;
			patch->rowLOD[pos] = BTree_Delete( patch->rowLOD[pos] );
			patch->rowLOD[pos] = BTree_Create( patch->ctrl[col][row] );
			pBTList = BTree_AddToList( pBTList, patch->rowLOD[pos], patch->ctrl[col - 1][row], patch->ctrl[col + 1][row] );
		}
	}
	else
	{
		for ( row = patch->height - 1; row >= 0; row-- )
		{
			pos = ( ( ( col - 1 ) / 2 ) * patch->height ) + row;
			patch->rowLOD[pos] = BTree_Delete( patch->rowLOD[pos] );
			patch->rowLOD[pos] = BTree_Create( patch->ctrl[col][row] );
			pBTList = BTree_AddToList( pBTList, patch->rowLOD[pos], patch->ctrl[col - 1][row], patch->ctrl[col + 1][row] );
		}
	}
	return pBTList;
}

// take a pointer to the last item added to pBTList, a pointer to the patch, a row index and a column index (start)
// generate a row of column-curve tree roots, owned by the patch and add the entire row of column-curves to the list, using the column index to decide the order to add
// return a pointer to the last item added to the list
BTreeList_t *Patch_CreateBTListForCols( BTreeList_t *pBTList, patchMesh_t *patch, int row, int start ){
	int col, pos;
	patch->rowDirty[( row - 1 ) / 2] = true;

	if ( start == 0 ) {
		for ( col = 0; col < patch->width; col++ )
		{
			pos = ( ( ( row - 1 ) / 2 ) * patch->width ) + col;
			patch->colLOD[pos] = BTree_Delete( patch->colLOD[pos] );
			patch->colLOD[pos] = BTree_Create( patch->ctrl[col][row] );
			pBTList = BTree_AddToList( pBTList, patch->colLOD[pos], patch->ctrl[col][row - 1], patch->ctrl[col][row + 1] );
		}
	}
	else
	{
		for ( col = patch->width - 1; col >= 0; col-- )
		{
			pos = ( ( ( row - 1 ) / 2 ) * patch->width ) + col;
			patch->colLOD[pos] = BTree_Delete( patch->colLOD[pos] );
			patch->colLOD[pos] = BTree_Create( patch->ctrl[col][row] );
			pBTList = BTree_AddToList( pBTList, patch->colLOD[pos], patch->ctrl[col][row - 1], patch->ctrl[col][row + 1] );
		}

	}
	return pBTList;
}

bool BTree_IsInList( BTreeList_t *pBTList, BTNode_t *pBT ){
	BTreeList_t *p;
	if ( pBTList == NULL ) {
		return false;
	}

	for ( p = pBTList; p != NULL; p = p->next )
	{
		if ( p->pBT != NULL ) {
			if ( p->pBT == pBT ) {
				return true;
			}
		}
	}
	return false;
}

int Patch_DegenCurve( vec3_t &start, vec3_t &mid, vec3_t &end ){
	if ( VectorCompare( start, mid ) || VectorCompare( end, mid ) ) {
		if ( VectorCompare( start, end ) ) {
			return 2;
		}
		else{ return 1; }
	}
	else{ return 0; }
}

// take a pointer to the last item added to the list, and a pointer to a patch (this patch is the owner of the three drawverts)
// take the addresses of three drawVerts, and compare them with the edges of all patches that touch the patch
// if they match an edge, add the tree roots for that section of the matched patch to the list, and recurse for the opposite edge of that patch section. Also, set the matched patch Dirty, so that its drawlists will be rebuilt
// return a pointer to the last item added
BTreeList_t *Patch_FindLODMatches( patchMesh_t *patch, BTreeList_t *pBTList, drawVert_t &pMid, drawVert_t &pLeft, drawVert_t &pRight ){
	brush_t *pb, *brushlist;
	int row, col, i; //, pos;
	vec3_t vTemp, v1, v2; //, vClear;
	bool bAlreadyAdded;

	//Sys_Printf("Patch_FindLODMatches: called\n");

	if ( VectorCompare( pMid.xyz, pLeft.xyz ) && VectorCompare( pMid.xyz, pRight.xyz ) ) {
		return pBTList;
	}

	//VectorClear(vClear);
	VectorSubtract( pRight.xyz, pLeft.xyz, vTemp );
	VectorSubtract( pMid.xyz, pLeft.xyz, v1 );
	VectorSubtract( pRight.xyz, pMid.xyz, v2 );

	//if (VectorCompare(v1, vClear) || VectorCompare(vTemp, v1)) // return null if 1->2 == 0 or 1->2 == 1->3
	//	return pBTList;

	VectorNormalize( v1, v1 );
	VectorNormalize( v2, v2 );
	if ( VectorCompare( v1, v2 ) ) {
		return pBTList;
	}

	VectorNormalize( vTemp, vTemp );
	if ( VectorCompare( v1, vTemp ) && VectorCompare( v2, vTemp ) ) {
		return pBTList;
	}

	brushlist = &active_brushes;
	for ( i = 0; i < 2; i++ )
	{
		for ( pb = brushlist->next; pb != brushlist; pb = pb->next )
		{
			if ( !pb->patchBrush || pb->pPatch == patch ) {
				continue;
			}

			// ignore this patch if its AABB does not touch the subject patch
			if ( !TouchingAABBs( patch->pSymbiot->maxs, patch->pSymbiot->mins, pb->maxs, pb->mins ) ) {
				continue;
			}

			// all columns of curves
			for ( col = 1; col < pb->pPatch->width; col += 2 )
			{
				if ( pb->pPatch->colDirty[( col - 1 ) / 2] ) {
					continue;
				}

				bAlreadyAdded = false;

				// top and bottom curves of this column
				for ( row = 0; row < pb->pPatch->height; row += pb->pPatch->height - 1 )
				{
					if ( bAlreadyAdded ) {
						continue;
					}
					//if (!BTree_IsInList(pBTList, pb->pPatch->rowLOD[(((col-1)/2)*patch->height)+row]))
					//  continue;
					// ignore this curve if it shares no mid ctrl point with the test curve
					if ( !VectorCompare( pb->pPatch->ctrl[col][row].xyz, pMid.xyz ) ) {
						continue;
					}
					// ignore this curve if it is degenerate
					if ( VectorCompare( pb->pPatch->ctrl[col][row].xyz, pb->pPatch->ctrl[col - 1][row].xyz ) || VectorCompare( pb->pPatch->ctrl[col][row].xyz, pb->pPatch->ctrl[col + 1][row].xyz ) ) {
						continue;
					}
					// if curve matches the test curve directly
					if ( VectorCompare( pb->pPatch->ctrl[col - 1][row].xyz, pLeft.xyz ) && VectorCompare( pb->pPatch->ctrl[col + 1][row].xyz, pRight.xyz ) ) {
						// add a blank link as separator
						pBTList = BTree_AddLinkToList( pBTList );
						// add this entire column, if top, top-to-bottom, else bottom to top
						pBTList = Patch_CreateBTListForRows( pBTList, pb->pPatch, row, col );
						// continue checking from last curve added to list
						pBTList = Patch_FindLODMatches( pb->pPatch, pBTList, pBTList->pBT->info, pBTList->vLeft, pBTList->vRight );
						// set flag
						pb->pPatch->LODUpdated = true;
						bAlreadyAdded = true;
					}
					// if curve matches test curve but flipped
					else if ( VectorCompare( pb->pPatch->ctrl[col - 1][row].xyz, pRight.xyz ) && VectorCompare( pb->pPatch->ctrl[col + 1][row].xyz, pLeft.xyz ) ) {
						pBTList = BTree_AddLinkToList( pBTList, true ); // flip
						pBTList = Patch_CreateBTListForRows( pBTList, pb->pPatch, row, col );
						pBTList = Patch_FindLODMatches( pb->pPatch, pBTList, pBTList->pBT->info, pBTList->vLeft, pBTList->vRight );
						pb->pPatch->LODUpdated = true;
						bAlreadyAdded = true;
					}
				}
			}

			// all rows of curves
			for ( row = 1; row < pb->pPatch->height; row += 2 )
			{
				if ( pb->pPatch->rowDirty[( row - 1 ) / 2] ) {
					continue;
				}

				bAlreadyAdded = false;

				for ( col = 0; col < pb->pPatch->width; col += pb->pPatch->width - 1 )
				{
					if ( bAlreadyAdded ) {
						continue;
					}
					//if (BTree_IsInList(pBTList, pb->pPatch->colLOD[(((row-1)/2)*patch->width)+col]))
					//  continue;
					if ( !VectorCompare( pb->pPatch->ctrl[col][row].xyz, pMid.xyz ) ) {
						continue;
					}
					if ( VectorCompare( pb->pPatch->ctrl[col][row].xyz, pb->pPatch->ctrl[col][row - 1].xyz ) || VectorCompare( pb->pPatch->ctrl[col][row].xyz, pb->pPatch->ctrl[col][row + 1].xyz ) ) {
						continue;
					}
					if ( VectorCompare( pb->pPatch->ctrl[col][row - 1].xyz, pLeft.xyz ) && VectorCompare( pb->pPatch->ctrl[col][row + 1].xyz, pRight.xyz ) ) {
						pBTList = BTree_AddLinkToList( pBTList );
						pBTList = Patch_CreateBTListForCols( pBTList, pb->pPatch, row, col );
						pBTList = Patch_FindLODMatches( pb->pPatch, pBTList, pBTList->pBT->info, pBTList->vLeft, pBTList->vRight );
						pb->pPatch->LODUpdated = true;
						bAlreadyAdded = true;
					}
					else if ( VectorCompare( pb->pPatch->ctrl[col][row - 1].xyz, pRight.xyz ) && VectorCompare( pb->pPatch->ctrl[col][row + 1].xyz, pLeft.xyz ) ) {
						pBTList = BTree_AddLinkToList( pBTList, true ); // flip
						pBTList = Patch_CreateBTListForCols( pBTList, pb->pPatch, row, col );
						pBTList = Patch_FindLODMatches( pb->pPatch, pBTList, pBTList->pBT->info, pBTList->vLeft, pBTList->vRight );
						pb->pPatch->LODUpdated = true;
						bAlreadyAdded = true;
					}
				}
			}
		}
		brushlist = &selected_brushes;
	}
	return pBTList;
}

// take a pointer to a patch
// create tree roots for all the rows and columns of curves in the patch, the patch takes ownership of these new tree roots
// generate lists of pointers to all the trees in all the patches in the map which need to match the LOD of trees owned by this patch
// store all the lists in a list of lists
// recursively generate the rest of every tree in each list in the list
void Patch_CreateLODTrees( patchMesh_t *patch ){
	BTreeList_t *pBTList;
	int col, row, pos; //, rowcount, colcount;
	BTListList_t *pLists;

	//Sys_Printf("Patch_CreateMatchedLODTrees: called\n");

	BTListList_t *LODLists;
	LODLists = NULL;

	pBTList = NULL;

	patch->bDirty = false;
	patch->LODUpdated = true;

	for ( col = 1; col < patch->width; col += 2 )
	{
		if ( patch->colDirty[( col - 1 ) / 2] ) {
			continue;
		}
		else{patch->colDirty[( col - 1 ) / 2] = true; }

		// create list for rows of current patch
		for ( row = 0; row < patch->height; row++ )
		{
			pos = ( ( ( col - 1 ) / 2 ) * patch->height ) + row;
			patch->rowLOD[pos] = BTree_Delete( patch->rowLOD[pos] );
			patch->rowLOD[pos] = BTree_Create( patch->ctrl[col][row] );
			pBTList = BTree_AddToList( pBTList, patch->rowLOD[pos], patch->ctrl[col - 1][row], patch->ctrl[col + 1][row] );
		}

		//create connection list for first row
		pBTList = Patch_FindLODMatches( patch, pBTList, patch->ctrl[col][0], patch->ctrl[col - 1][0], patch->ctrl[col + 1][0] );
		//create connection list for last row
		pBTList = Patch_FindLODMatches( patch, pBTList, patch->ctrl[col][row - 1], patch->ctrl[col - 1][row - 1], patch->ctrl[col + 1][row - 1] );

		LODLists = BTree_AddListToList( LODLists, pBTList );
		pBTList = NULL;
	}

	pBTList = NULL;
	for ( row = 1; row < patch->height; row += 2 )
	{
		if ( patch->rowDirty[( row - 1 ) / 2] ) {
			continue;
		}
		else{patch->rowDirty[( row - 1 ) / 2] = true; }

		// create list for cols of current patch
		for ( col = 0; col < patch->width; col++ )
		{
			pos = ( ( ( row - 1 ) / 2 ) * patch->width ) + col;
			patch->colLOD[pos] = BTree_Delete( patch->colLOD[pos] );
			patch->colLOD[pos] = BTree_Create( patch->ctrl[col][row] );
			pBTList = BTree_AddToList( pBTList, patch->colLOD[pos], patch->ctrl[col][row - 1], patch->ctrl[col][row + 1] );
		}

		//create connection list for first col
		pBTList = Patch_FindLODMatches( patch, pBTList, patch->ctrl[0][row], patch->ctrl[0][row - 1], patch->ctrl[0][row + 1] );
		//create connection list for last col
		pBTList = Patch_FindLODMatches( patch, pBTList, patch->ctrl[col - 1][row], patch->ctrl[col - 1][row - 1], patch->ctrl[col - 1][row + 1] );

		LODLists = BTree_AddListToList( LODLists, pBTList );
		pBTList = NULL;
	}

	for ( pLists = LODLists; pLists != NULL; pLists = pLists->next )
		BTree_ListCurveRecurse( pLists->list );
	LODLists = BTree_DeleteListFromList( LODLists );
}

int Patch_GetCVTangent( vec3_t &v1, vec3_t &p1, vec3_t &p2, vec3_t &p3 ){
	if ( VectorCompare( p1, p2 ) ) {
		if ( VectorCompare( p1, p3 ) ) {
			return 2;
		}
		else{VectorSubtract( p3, p1, v1 ); }
		return 1;
	}
	else{VectorSubtract( p2, p1, v1 ); }
	return 0;
}

void Patch_CVNormal( vec3_t ctrl[3][3], vec3_t &normal ){
	vec3_t v1, v2, vTemp1, vTemp2;
	int a, b;

	a = Patch_GetCVTangent( v1, ctrl[0][0], ctrl[1][0], ctrl[2][0] );
	b = Patch_GetCVTangent( v2, ctrl[0][0], ctrl[0][1], ctrl[0][2] );

	//Sys_Printf("p1: (%1.1f %1.1f %1.1f) p2: (%1.1f %1.1f %1.1f) p2: (%1.1f %1.1f %1.1f)\n",
	//	ctrl[0][0][0], ctrl[0][0][1], ctrl[0][0][2], ctrl[0][2][0], ctrl[0][2][1], ctrl[0][2][2], ctrl[2][0][0], ctrl[2][0][1], ctrl[2][0][2]);

	v1[0] = v1[1] = v1[2] = v2[0] = v2[1] = v2[2] = 0;

	if ( a == 2 ) {
		a = Patch_GetCVTangent( v1, ctrl[0][0], ctrl[1][1], ctrl[1][2] );
	}
	if ( b == 2 ) {
		b = Patch_GetCVTangent( v2, ctrl[0][0], ctrl[1][1], ctrl[2][1] );
	}

	if ( a == 2 ) {
		a = Patch_GetCVTangent( v1, ctrl[0][0], ctrl[2][1], ctrl[2][2] );
	}
	if ( b == 2 ) {
		b = Patch_GetCVTangent( v2, ctrl[0][0], ctrl[1][2], ctrl[2][2] );
	}

	CrossProduct( v1, v2, normal );


	if ( normal[0] == 0.0f && normal[1] == 0.0f && normal[2] == 0.0f ) {
		// more degenerate cases
		vec3_t pMid;
		vec3_t vCurve[3];
		/*
		   if (VectorCompare(ctrl[0][0], ctrl[2][0])) // endcap left
		   {
		   if (VectorCompare(ctrl[0][2], ctrl[1][2]))
		   {
		    VectorSubtract(ctrl[2][2], ctrl[0][0], v2);
		   }
		   else if (VectorCompare(ctrl[1][2], ctrl[2][2]))
		   {
		    VectorSubtract(ctrl[0][2], ctrl[0][0], v2);
		   }
		   else
		   a = Patch_DegenCurve(ctrl[0][2], ctrl[1][2], ctrl[2][2]);
		   if (a == 0)
		   {
		    VectorCopy(ctrl[0][2], vCurve[0]);
		    VectorCopy(ctrl[1][2], vCurve[1]);
		    VectorCopy(ctrl[2][2], vCurve[2]);
		    Patch_BezierInterpolate(vCurve, pMid);
		          VectorSubtract(pMid, ctrl[0][0], v1);
		   }


		    }
		   else if (VectorCompare(ctrl[0][0], ctrl[0][2])) // endcap right
		    {

		   if (VectorCompare(ctrl[2][0], ctrl[2][1]))
		   {
		    VectorSubtract(ctrl[2][2], ctrl[0][0], v2);
		   }
		   else if (VectorCompare(ctrl[2][1], ctrl[2][2]))
		   {
		    VectorSubtract(ctrl[2][0], ctrl[0][0], v2);
		   }
		   else

		   b = Patch_DegenCurve(ctrl[2][0], ctrl[2][1], ctrl[2][2]);
		   if (b == 0)
		   {
		        VectorCopy(ctrl[2][0], vCurve[0]);
		    VectorCopy(ctrl[2][1], vCurve[1]);
		    VectorCopy(ctrl[2][2], vCurve[2]);
		    Patch_BezierInterpolate(vCurve, pMid);
		          VectorSubtract(pMid, ctrl[0][0], v2);
		   }

		    }
		 */
		if ( VectorCompare( ctrl[0][0], ctrl[2][0] ) ) { // bottom degen
			Patch_GetCVTangent( v1, ctrl[0][0], ctrl[2][1], ctrl[2][2] );
		}
		else if ( VectorCompare( ctrl[0][0], ctrl[0][2] ) ) { // left degen
			Patch_GetCVTangent( v2, ctrl[0][0], ctrl[1][2], ctrl[2][2] );
		}
		else if ( VectorCompare( ctrl[0][2], ctrl[2][2] ) ) { // top degen
			VectorSubtract( ctrl[2][0], ctrl[0][0], v1 );
		}
		else if ( VectorCompare( ctrl[2][0], ctrl[2][2] ) ) { // right degen
			VectorSubtract( ctrl[0][2], ctrl[0][0], v2 );
		}
		else // tangents parallel
		{
			VectorCopy( v1, vTemp1 );
			VectorCopy( v2, vTemp2 );
			VectorNormalize( vTemp1, vTemp1 );
			VectorNormalize( vTemp2, vTemp2 );
			if ( VectorCompare( vTemp1, vTemp2 ) ) { // parallel same way
				VectorSubtract( ctrl[2][0], ctrl[0][0], vTemp1 );
				VectorNormalize( vTemp1, vTemp1 );
				if ( VectorCompare( vTemp1, vTemp2 ) ) {
					VectorSubtract( ctrl[0][2], ctrl[0][0], v2 );
				}
				else
				{
					VectorCopy( vTemp1, v1 );
				}
			}
			else // parallel opposite way
			{
				VectorCopy( ctrl[2][0], vCurve[0] );
				VectorCopy( ctrl[1][1], vCurve[1] );
				VectorCopy( ctrl[0][2], vCurve[2] );
				Patch_BezierInterpolate( vCurve, pMid );
				VectorSubtract( pMid, ctrl[0][0], v2 );
			}
		}

		CrossProduct( v1, v2, normal );
	}
}

void Patch_CalcCVNormals( patchMesh_t *patch ){
	int row, col, i, j, n;
	vec3_t ctrl[3][3];
	vec3_t normals[4];

	for ( col = 0; col < patch->width; col += 2 )
	{
		for ( row = 0; row < patch->height; row += 2 )
		{
			n = 0;
			if ( col + 1 != patch->width && row + 1 != patch->height ) {
				for ( i = 0; i < 3; i++ )
					for ( j = 0; j < 3; j++ )
						VectorCopy( patch->ctrl[col + i][row + j].xyz, ctrl[i][j] );

				Patch_CVNormal( ctrl, normals[n] );
				VectorNormalize( normals[n], normals[n] );
				n++;
			}

			if ( col - 1 >= 0 && row - 1 >= 0 ) {
				for ( i = 0; i < 3; i++ )
					for ( j = 0; j < 3; j++ )
						VectorCopy( patch->ctrl[col - i][row - j].xyz, ctrl[i][j] );

				Patch_CVNormal( ctrl, normals[n] );
				VectorNormalize( normals[n], normals[n] );
				n++;
			}
			if ( col - 1 >= 0 && row + 1 != patch->height ) {
				for ( i = 0; i < 3; i++ )
					for ( j = 0; j < 3; j++ )
						VectorCopy( patch->ctrl[col - i][row + j].xyz, ctrl[j][i] );

				Patch_CVNormal( ctrl, normals[n] );
				VectorNormalize( normals[n], normals[n] );
				n++;
			}
			if ( col + 1 != patch->width && row - 1 >= 0 ) {
				for ( i = 0; i < 3; i++ )
					for ( j = 0; j < 3; j++ )
						VectorCopy( patch->ctrl[col + i][row - j].xyz, ctrl[j][i] );

				Patch_CVNormal( ctrl, normals[n] );
				VectorNormalize( normals[n], normals[n] );
				n++;
			}

			for ( i = 0; i < 3; i++ )
			{
				if ( n == 1 ) {
					patch->ctrl[col][row].normal[i] = normals[0][i];
				}
				if ( n == 2 ) {
					patch->ctrl[col][row].normal[i] = ( normals[0][i] + normals[1][i] ) / n;
				}
				//if (n == 3) patch->ctrl[col][row].normal[i] = (normals[0][i] + normals[1][i] + normals[2][i]) / n;
				if ( n == 4 ) {
					patch->ctrl[col][row].normal[i] = ( normals[0][i] + normals[1][i] + normals[2][i] + normals[3][i] ) / n;
				}
			}
			VectorNormalize( patch->ctrl[col][row].normal, patch->ctrl[col][row].normal );
			//if (!g_PrefsDlg.m_bGLLighting)
			//	ShadeVertex(patch->ctrl[col][row]);
		}
	}
}


void BTree_SetNormals( BTNode_t *pBT, vec3_t &normal ){
	if ( pBT != NULL ) {
		if ( pBT->left != NULL && pBT->right != NULL ) {
			VectorCopy( normal, pBT->vMid.normal );
			//if (!g_PrefsDlg.m_bGLLighting)
			//	ShadeVertex(pBT->vMid);
		}
		BTree_SetNormals( pBT->left, normal );
		BTree_SetNormals( pBT->right, normal );
	}
}


void NormalFromPoints( vec3_t p1, vec3_t p2, vec3_t p3, vec3_t &normal, bool flip = false ){
	vec3_t v1, v2;

	if ( flip ) {
		VectorSubtract( p2, p3, v1 ); //p3->p2
		VectorSubtract( p1, p2, v2 ); //p2->p1
	}
	else
	{
		VectorSubtract( p2, p1, v1 ); //p1->p2
		VectorSubtract( p3, p2, v2 ); //p2->p3
	}
	CrossProduct( v1, v2, normal );
}


void BTree_GenerateNormals( BTNode_t *pBTMid, BTNode_t *pBTLeft, BTNode_t *pBTRight, bool avg, bool flat, bool nomid, bool noleft, bool noright, /*bool endcap, vec3_t &n1, vec3_t &n2,*/ bool flip ){
	if ( pBTMid != NULL ) {
		if ( pBTMid->left != NULL && pBTMid->right != NULL ) {
			vec3_t normal;

			if ( noleft ) { // left curve is degenerate
				if ( nomid ) { // mid curve is degenerate
					NormalFromPoints( pBTRight->right->info.xyz, pBTRight->vMid.xyz, pBTMid->vMid.xyz, normal, flip );
					NormalFromPoints( pBTRight->right->info.xyz, pBTRight->vMid.xyz, pBTMid->vMid.xyz, pBTRight->vMid.normal, flip );
				}
				//else if (endcap)
				//{
				//  VectorCopy(n1, normal);
				//  NormalFromPoints(pBTRight->right->info.xyz, pBTRight->vMid.xyz, pBTMid->vMid.xyz, pBTRight->vMid.normal, flip);
				//}
				else
				{
					NormalFromPoints( pBTMid->left->info.xyz, pBTLeft->vMid.xyz, pBTMid->vMid.xyz, normal, flip );
					NormalFromPoints( pBTRight->right->info.xyz, pBTRight->vMid.xyz, pBTMid->vMid.xyz, pBTRight->vMid.normal, flip );
				}
			}
			else if ( noright ) { // right curve is degenerate
				if ( nomid ) { // mid curve is degenerate
					NormalFromPoints( pBTLeft->left->info.xyz, pBTLeft->vMid.xyz, pBTMid->vMid.xyz, normal, flip );
					NormalFromPoints( pBTLeft->left->info.xyz, pBTLeft->vMid.xyz, pBTMid->vMid.xyz, pBTRight->vMid.normal, flip );
				}
				//else if (endcap)
				//{
				//  NormalFromPoints(pBTLeft->left->info.xyz, pBTLeft->vMid.xyz, pBTMid->vMid.xyz, normal, flip);
				//  VectorCopy(n2, pBTRight->vMid.normal);
				//}
				else
				{
					NormalFromPoints( pBTLeft->left->info.xyz, pBTLeft->vMid.xyz, pBTMid->vMid.xyz, normal, flip );
					NormalFromPoints( pBTMid->right->info.xyz, pBTRight->vMid.xyz, pBTMid->vMid.xyz, pBTRight->vMid.normal, flip );
				}
			}
			else
			{
				if ( flat ) { // all curves are semi-degenerate (flat) or degenerate
					NormalFromPoints( pBTLeft->left->info.xyz, pBTLeft->vMid.xyz, pBTRight->vMid.xyz, normal, flip );
					NormalFromPoints( pBTRight->right->info.xyz, pBTRight->vMid.xyz, pBTLeft->vMid.xyz, pBTRight->vMid.normal, flip );
				}
				else
				{
					NormalFromPoints( pBTLeft->left->info.xyz, pBTLeft->vMid.xyz, pBTMid->vMid.xyz, normal, flip );
					NormalFromPoints( pBTRight->right->info.xyz, pBTRight->vMid.xyz, pBTMid->vMid.xyz, pBTRight->vMid.normal, flip );
				}
			}

			VectorNormalize( normal, normal );
			if ( avg ) {
				for ( int i = 0; i < 3; i++ )
					pBTLeft->vMid.normal[i] = ( normal[i] + pBTLeft->vMid.normal[i] ) / 2.0f;
			}
			else{VectorCopy( normal, pBTLeft->vMid.normal ); }

			VectorNormalize( pBTLeft->vMid.normal, pBTLeft->vMid.normal );
			VectorNormalize( pBTRight->vMid.normal, pBTRight->vMid.normal );

		}
		BTree_GenerateNormals( pBTMid->left, pBTLeft->left, pBTRight->left, avg, flat, nomid, noleft, noright, /*endcap, n1, n2,*/ flip );
		BTree_GenerateNormals( pBTMid->right, pBTLeft->right, pBTRight->right, avg, flat, nomid, noleft, noright, /*endcap, n1, n2,*/ flip );
	}
}



void Patch_GenerateLODNormals( patchMesh_t *patch ){
	int col, row, rowpos, colpos, i;
	BTNode_t *tree[2][3];
	int degen[2][3];
	bool rowAvg, colAvg;

	for ( col = 0; col + 2 < patch->width; col += 2 )
	{
		for ( row = 0; row + 2 < patch->height; row += 2 )
		{
			if ( !patch->colDirty[col / 2] && !patch->rowDirty[row / 2] ) {
				continue;
			}

			rowpos = ( ( col / 2 ) * patch->height ) + row;
			colpos = ( ( row / 2 ) * patch->width ) + col;

			if ( row == 0 ) {
				rowAvg = false;
			}
			else{ rowAvg = true; }
			if ( col == 0 ) {
				colAvg = false;
			}
			else{ colAvg = true; }

			for ( i = 0; i < 3; i++ )
			{
				tree[0][i] = patch->rowLOD[rowpos + i];
				tree[1][i] = patch->colLOD[colpos + i];

				degen[0][i] = Patch_DegenCurve( patch->ctrl[col][row + i].xyz, patch->ctrl[col + 1][row + i].xyz, patch->ctrl[col + 2][row + i].xyz );
				degen[1][i] = Patch_DegenCurve( patch->ctrl[col + i][row].xyz, patch->ctrl[col + i][row + 1].xyz, patch->ctrl[col + i][row + 2].xyz );
			}

			BTree_GenerateNormals( tree[0][1], tree[0][0], tree[0][2], rowAvg, ( degen[1][0] && degen[1][1] && degen[1][2] ), degen[0][1] == 2, degen[0][0] == 2, degen[0][2] == 2, /*degen[1][1], patch->ctrl[col][row].normal, patch->ctrl[col][row+2].normal,*/ false );
			BTree_GenerateNormals( tree[1][1], tree[1][0], tree[1][2], colAvg, ( degen[0][0] && degen[0][1] && degen[0][2] ), degen[1][1] == 2, degen[1][0] == 2, degen[1][2] == 2, /*degen[0][1], patch->ctrl[col][row].normal, patch->ctrl[col+2][row].normal,*/ true );
		}
	}
}


void Patch_ClearLODFlags( patchMesh_t *p ){
	int i;

	for ( i = 0; i < ( p->width - 1 ) / 2; i++ )
		p->colDirty[i] = false;

	for ( i = 0; i < ( p->height - 1 ) / 2; i++ )
		p->rowDirty[i] = false;
}

// reset the lodDirty flags owned by all patches in the map
// create new LOD trees for all dirty patches, matched with all other patches in the map
void Patch_LODMatchAll(){
	brush_t *pb, *brushlist;
	int i;

	// create LOD tree roots and LOD tree lists for all patches that are dirty

	brushlist = &active_brushes;
	for ( i = 0; i < 2; i++ )
	{
		for ( pb = brushlist->next; pb && ( pb != brushlist ); pb = pb->next )
		{
			// create lod for selected patches when patches are filtered
			if ( pb->bFiltered && ( pb->patchBrush && !pb->pPatch->bSelected ) ) {
				continue;
			}
			if ( !pb->patchBrush ) {
				continue;
			}
			if ( !pb->pPatch->bDirty ) {
				continue;
			}

			Patch_CalcCVNormals( pb->pPatch );
			Patch_CreateLODTrees( pb->pPatch );
		}
		brushlist = &selected_brushes;
	}

	brushlist = &active_brushes;
	for ( i = 0; i < 2; i++ )
	{
		for ( pb = brushlist->next; pb && ( pb != brushlist ); pb = pb->next )
		{
			if ( !pb->patchBrush ) {
				continue;
			}

			if ( pb->pPatch->LODUpdated ) {
				Patch_GenerateLODNormals( pb->pPatch );
			}

			Patch_ClearLODFlags( pb->pPatch );
		}
		brushlist = &selected_brushes;
	}

}

void Vertex_TransformTexture( drawVert_t *pVert, float fx, float fy, transformtype xform ){
	switch ( xform )
	{
	case TRANSLATE:
		pVert->st[0] += fx;
		pVert->st[1] += fy;
		break;
	case SCALE:
		pVert->st[0] *= fx;
		pVert->st[1] *= fy;
		break;
	case ROTATE:
		float x = pVert->st[0];
		float y = pVert->st[1];
		pVert->st[0] = x * fx - y * fy;
		pVert->st[1] = y * fx + x * fy;
	}
}

void BTree_TransformTexture( BTNode_t *pBT, float fx, float fy, transformtype xform ){
	if ( pBT != NULL ) { // PreOrder traversal
		Vertex_TransformTexture( &pBT->info, fx, fy, xform );
		Vertex_TransformTexture( &pBT->vMid, fx, fy, xform );
		BTree_TransformTexture( pBT->left, fx, fy, xform );
		BTree_TransformTexture( pBT->right, fx, fy, xform );
	}
}

void Patch_TransformLODTexture( patchMesh_t *p, float fx, float fy, transformtype xform ){
	int col, row;

	for ( col = 1; col < p->width; col += 2 )
		for ( row = 0; row < p->height; row++ )
			BTree_TransformTexture( p->rowLOD[( ( ( col - 1 ) / 2 ) * p->height ) + row], fx, fy, xform );

	for ( row = 1; row < p->height; row += 2 )
		for ( col = 0; col < p->width; col++ )
			BTree_TransformTexture( p->colLOD[( ( ( row - 1 ) / 2 ) * p->width ) + col], fx, fy, xform );
}

void Patch_AddBTreeToDrawListInOrder( list<drawVert_t> *drawList, BTNode_t *pBT ){
	if ( pBT != NULL ) { //traverse InOrder
		Patch_AddBTreeToDrawListInOrder( drawList, pBT->left );
		if ( pBT->left != NULL && pBT->right != NULL ) {
			drawList->push_back( pBT->vMid );
		}
		Patch_AddBTreeToDrawListInOrder( drawList, pBT->right );
	}
}

void Patch_InterpolateListFromRowBT( list<drawVert_t> *drawList, BTNode_t *rowBT, BTNode_t *rowBTLeft, drawVert_t *vCurve[], float u, float n, float v ){
	if ( rowBT != NULL ) {
		Patch_InterpolateListFromRowBT( drawList, rowBT->left, rowBTLeft->left, vCurve, u - n, n * 0.5f, v );
		if ( rowBT->left != NULL && rowBT->right != NULL ) {
			vec3_t v1, v2;
			drawVert_t newVert, vTemp1, vTemp2;
			Patch_CurveSplit( vCurve, vTemp1, vTemp2, newVert, u );
			for ( int i = 0; i < 3; i++ )
			{
				v1[i] = rowBT->vMid.xyz[i] - rowBTLeft->vMid.xyz[i]; // left -> mid
				v1[i] = rowBTLeft->vMid.xyz[i] + ( v1[i] * v );
				v1[i] = newVert.xyz[i] - v1[i];
			}
			VectorSubtract( vTemp1.xyz, newVert.xyz, v2 );
			CrossProduct( v1, v2, newVert.normal );
			VectorNormalize( newVert.normal, newVert.normal );
			//if (!g_PrefsDlg.m_bGLLighting)
			//	ShadeVertex(newVert);
			drawList->push_back( newVert );
		}
		Patch_InterpolateListFromRowBT( drawList, rowBT->right, rowBTLeft->right, vCurve, u + n, n * 0.5f, v );
	}
}

void Patch_TraverseColBTInOrder( list<list<drawVert_t>*>::iterator& iter, BTNode_t *colBTLeft, BTNode_t *colBT, BTNode_t *colBTRight, BTNode_t *rowBT, BTNode_t *rowBTLeft, float v, float n ){
	if ( colBT != NULL ) {
		//traverse subtree In Order
		Patch_TraverseColBTInOrder( iter, colBTLeft->left, colBT->left, colBTRight->left, rowBT, rowBTLeft, v - n, n * 0.5f );
		if ( colBT->left != NULL && colBT->right != NULL ) {
			drawVert_t *vCurve[3];
			vCurve[0] = &colBTLeft->vMid;
			vCurve[1] = &colBT->vMid;
			vCurve[2] = &colBTRight->vMid;
			Patch_InterpolateListFromRowBT( ( *iter ), rowBT, rowBTLeft, vCurve, 0.5f, 0.25f, v );

			( *iter )->push_back( colBTRight->vMid );
			iter++;
		}
		Patch_TraverseColBTInOrder( iter, colBTLeft->right, colBT->right, colBTRight->right, rowBT, rowBTLeft, v + n, n * 0.5f );
	}
}


void Patch_StartDrawLists( list<list<drawVert_t>*> *drawLists, BTNode_t *colBT ){
	if ( colBT != NULL ) {
		//traverse subtree In Order
		Patch_StartDrawLists( drawLists, colBT->left );
		if ( colBT->left != NULL && colBT->right != NULL ) {
			list<drawVert_t> *newList = new list<drawVert_t>;
			drawLists->push_back( newList ); // add empty list to back
			drawLists->back()->push_back( colBT->vMid );
		}
		Patch_StartDrawLists( drawLists, colBT->right );
	}
}

typedef list<drawVert_t> drawList_t;
typedef list<list<drawVert_t>*> drawLists_t;

void Patch_CreateDrawLists( patchMesh_t *patch ){
	int col, row, colpos, rowpos;

	drawLists_t *drawLists = new drawLists_t;

	drawLists_t::iterator iter1, iter2;

	for ( row = 0; row < patch->height; row += 2 )
	{
		colpos = ( row / 2 ) * patch->width;
		drawList_t *newList = new drawList_t;
		drawLists->push_back( newList ); // add a new empty list to back
		drawLists->back()->push_back( patch->ctrl[0][row] ); // fill list at back

		if ( row + 1 == patch->height ) {
			continue;
		}
		Patch_StartDrawLists( drawLists, patch->colLOD[colpos] );
	}

	iter1 = drawLists->begin();
	for ( row = 0; row < patch->height; row += 2 )
	{
		iter2 = iter1;
		for ( col = 0; col + 1 < patch->width; col += 2 )
		{
			iter1 = iter2;
			colpos = ( ( row / 2 ) * patch->width ) + col;
			rowpos = ( ( col / 2 ) * patch->height ) + row;

			Patch_AddBTreeToDrawListInOrder( ( *iter1 ), patch->rowLOD[rowpos] );
			( *iter1 )->push_back( patch->ctrl[col + 2][row] );

			if ( row + 1 == patch->height ) {
				continue;
			}

			iter1++;

			Patch_TraverseColBTInOrder( iter1, patch->colLOD[colpos], patch->colLOD[colpos + 1], patch->colLOD[colpos + 2], patch->rowLOD[rowpos + 1], patch->rowLOD[rowpos], 0.5, 0.25 );
		}
	}

	patch->drawLists = drawLists;
}


void Patch_DeleteDrawLists( patchMesh_t *patch ){
	drawLists_t *drawLists;
	drawLists_t::iterator iter;

	if ( patch->drawLists == NULL ) {
		return;
	}

	drawLists = (drawLists_t *)patch->drawLists;

	for ( iter = drawLists->begin(); iter != drawLists->end(); iter++ )
	{
		delete ( *iter );
	}

	delete drawLists;
	patch->drawLists = NULL;
}


void Patch_DrawLODPatchMesh( patchMesh_t *patch ){
	drawLists_t *drawLists;

	drawLists_t::iterator iterLists, iterListsNext;
	drawList_t::iterator iterList, iterListNext;

	//int nGLState = g_pParentWnd->GetCamera()->Camera()->draw_glstate;

	if ( patch->drawLists == NULL ) {
		return;
	}

	drawLists = (drawLists_t *)patch->drawLists;

	iterListsNext = drawLists->begin();
	iterListsNext++;
	for ( iterLists = drawLists->begin(); iterLists != drawLists->end() && iterListsNext != drawLists->end(); iterLists++, iterListsNext++ )
	{
		// traverse two drawlists at once to draw a strip
		//if (nGLState & DRAW_GL_LINE)
		qglBegin( GL_QUAD_STRIP );
		//else
		//  qglBegin(GL_TRIANGLE_STRIP);
		for ( iterList = ( *iterLists )->begin(), iterListNext = ( *iterListsNext )->begin(); iterList != ( *iterLists )->end() && iterListNext != ( *iterListsNext )->end(); iterList++, iterListNext++ )
		{
			//if (g_PrefsDlg.m_bGLLighting)
			qglNormal3fv( ( *iterList ).normal );
			//else if (bShade && !g_PrefsDlg.m_bDisplayLists)
			//	qglColor3f((*iterList).lightmap[0], (*iterList).lightmap[0], (*iterList).lightmap[0]);

			qglTexCoord2fv( ( *iterList ).st );
			qglVertex3fv( ( *iterList ).xyz );

			//if (g_PrefsDlg.m_bGLLighting)
			qglNormal3fv( ( *iterListNext ).normal );
			//else if (bShade && !g_PrefsDlg.m_bDisplayLists)
			//	qglColor3f((*iterListNext).lightmap[0], (*iterListNext).lightmap[0], (*iterListNext).lightmap[0]);

			qglTexCoord2fv( ( *iterListNext ).st );
			qglVertex3fv( ( *iterListNext ).xyz );
		}
		qglEnd();
	}
/*
   #ifdef _DEBUG
   vec3_t vNormal;
   for (iterLists=drawLists->begin(); iterLists != drawLists->end(); iterLists++)
   {
        qglBegin (GL_LINES); // draw normals
        //qglColor3f(1,1,1);
        for (iterList=(*iterLists)->begin(); iterList != (*iterLists)->end(); iterList++)
        {
            VectorAdd((*iterList).xyz, (*iterList).normal, vNormal);
            qglVertex3fv ((*iterList).xyz);
            qglVertex3fv (vNormal);
        }
        qglEnd ();
   }

    Patch_DrawNormals(patch);

   #endif
 */
}

/*
   // fast memory-efficient ray-triangle intersection - MollerTrumbore97

   #define EPSILON 0.000001
   #define CROSS(dest,v1,v2) {dest[0]=v1[1]*v2[2]-v1[2]*v2[1];dest[1]=v1[2]*v2[0]-v1[0]*v2[2];dest[2]=v1[0]*v2[1]-v1[1]*v2[0];}
   #define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
   #define SUB(dest,v1,v2) {dest[0]=v1[0]-v2[0];dest[1]=v1[1]-v2[1];dest[2]=v1[2]-v2[2];}

   int intersect_triangle(float orig[3], float dir[3],
                   float vert0[3], float vert1[3], float vert2[3],
                   double *t, double *u, double *v)
   {
   double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
   double det,inv_det;

   // find vectors for two edges sharing vert0
   SUB(edge1, vert1, vert0);
   SUB(edge2, vert2, vert0);

   // begin calculating determinant - also used to calculate U parameter
   CROSS(pvec, dir, edge2);

   // if determinant is near zero, ray lies in plane of triangle
   det = DOT(edge1, pvec);

   #ifdef TEST_CULL           // define TEST_CULL if culling is desired
   if (det < EPSILON)
      return 0;

   // calculate distance from vert0 to ray origin
   SUB(tvec, orig, vert0);

   // calculate U parameter and test bounds
   *u = DOT(tvec, pvec);
   if (*u < 0.0 || *u > det)
      return 0;

   // prepare to test V parameter
   CROSS(qvec, tvec, edge1);

    // calculate V parameter and test bounds
   *v = DOT(dir, qvec);
   if (*v < 0.0 || *u + *v > det)
      return 0;

   // calculate t, scale parameters, ray intersects triangle
   *t = DOT(edge2, qvec);
   inv_det = 1.0 / det;
   *t *= inv_det;
   *u *= inv_det;
   *v *= inv_det;
   #else                    // the non-culling branch
   if (det > -EPSILON && det < EPSILON)
     return 0;
   inv_det = 1.0 / det;

   // calculate distance from vert0 to ray origin
   SUB(tvec, orig, vert0);

   // calculate U parameter and test bounds
   *u = DOT(tvec, pvec) * inv_det;
   if (*u < 0.0 || *u > 1.0)
     return 0;

   // prepare to test V parameter
   CROSS(qvec, tvec, edge1);

   // calculate V parameter and test bounds
   *v = DOT(dir, qvec) * inv_det;
   if (*v < 0.0 || *u + *v > 1.0)
     return 0;

   // calculate t, ray intersects triangle
   *t = DOT(edge2, qvec) * inv_det;
   #endif
   return 1;
   }
 */

int Triangle_Ray( float orig[3], float dir[3], bool bCullBack,
				  float vert0[3], float vert1[3], float vert2[3],
				  double *t, double *u, double *v ){
	float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	double det,inv_det;

	/* find vectors for two edges sharing vert0 */
	VectorSubtract( vert1, vert0, edge1 );
	VectorSubtract( vert2, vert0, edge2 );

	/* begin calculating determinant - also used to calculate U parameter */
	CrossProduct( dir, edge2, pvec );

	/* if determinant is near zero, ray lies in plane of triangle */
	det = DotProduct( edge1, pvec );

	if ( bCullBack ) {
		if ( det < 0.000001 ) {
			return 0;
		}

		// calculate distance from vert0 to ray origin
		VectorSubtract( orig, vert0, tvec );

		// calculate U parameter and test bounds
		*u = DotProduct( tvec, pvec );
		if ( *u < 0.0 || *u > det ) {
			return 0;
		}

		// prepare to test V parameter
		CrossProduct( tvec, edge1, qvec );

		// calculate V parameter and test bounds
		*v = DotProduct( dir, qvec );
		if ( *v < 0.0 || *u + *v > det ) {
			return 0;
		}

		// calculate t, scale parameters, ray intersects triangle
		*t = DotProduct( edge2, qvec );
		inv_det = 1.0 / det;
		*t *= inv_det;
		*u *= inv_det;
		*v *= inv_det;
	}
	else
	{
		/* the non-culling branch */
		if ( det > -0.000001 && det < 0.000001 ) {
			return 0;
		}
		inv_det = 1.0 / det;

		/* calculate distance from vert0 to ray origin */
		VectorSubtract( orig, vert0, tvec );

		/* calculate U parameter and test bounds */
		*u = DotProduct( tvec, pvec ) * inv_det;
		if ( *u < 0.0 || *u > 1.0 ) {
			return 0;
		}

		/* prepare to test V parameter */
		CrossProduct( tvec, edge1, qvec );

		/* calculate V parameter and test bounds */
		*v = DotProduct( dir, qvec ) * inv_det;
		if ( *v < 0.0 || *u + *v > 1.0 ) {
			return 0;
		}

		/* calculate t, ray intersects triangle */
		*t = DotProduct( edge2, qvec ) * inv_det;
	}
	return 1;
}

bool Patch_Ray( patchMesh_t *patch, vec3_t origin, vec3_t dir, double *t, double *u, double *v ){
	drawLists_t *drawLists;

	drawLists_t::iterator iterLists, iterListsNext;
	drawList_t::iterator i1, i2, i3, i4;

//  vec3_t tris[2][3];
	bool bIntersect = false;
	float tBest = FLT_MAX;

	if ( patch->drawLists == NULL ) {
		return false;
	}

	drawLists = (drawLists_t *)patch->drawLists;

	iterListsNext = drawLists->begin();
	iterListsNext++;
	for ( iterLists = drawLists->begin(); iterLists != drawLists->end() && iterListsNext != drawLists->end(); iterLists++, iterListsNext++ )
	{
		// traverse two drawlists at once with two iterators each to triangulate
		i1 = i3 = ( *iterLists )->begin();
		i2 = i4 = ( *iterListsNext )->begin();
		i3++;
		i4++;
		while ( i3 != ( *iterLists )->end() && i4 != ( *iterListsNext )->end() )
		{
			if ( Triangle_Ray( origin, dir, false, ( *i1 ).xyz, ( *i2 ).xyz, ( *i3 ).xyz, t, u, v ) ) {
				bIntersect = true;
				if ( *t < tBest ) {
					tBest = *t;
				}
			}
			if ( Triangle_Ray( origin, dir, false, ( *i3 ).xyz, ( *i4 ).xyz, ( *i2 ).xyz, t, u, v ) ) {
				bIntersect = true;
				if ( *t < tBest ) {
					tBest = *t;
				}
			}
			i1++;
			i2++;
			i3++;
			i4++;
		}
	}
	if ( bIntersect ) {
		*t = tBest;
		return true;
	}
	else
	{
		*t = 0;
		return false;
	}
}

// spog - curve LOD stuff ends

/*
   =================
   DrawPatchMesh
   =================
 */
void DrawPatchMesh( patchMesh_t *pm ){
	if ( g_PrefsDlg.m_bDisplayLists ) {
		if ( pm->bDirty || pm->nListID <= 0 || pm->LODUpdated ) {
			if ( pm->nListID <= 0 ) {
				pm->nListID = qglGenLists( 1 );
			}
			if ( pm->nListID > 0 ) {
				qglNewList( pm->nListID, GL_COMPILE_AND_EXECUTE );
			}

			Patch_DeleteDrawLists( pm );
			Patch_CreateDrawLists( pm );

			Patch_DrawLODPatchMesh( pm );

			if ( pm->nListID > 0 ) {
				qglEndList();
			}

			pm->bDirty = false;
			pm->LODUpdated = false;
		}
		else
		{
			qglCallList( pm->nListID );
		}
	}
	else
	{
		if ( pm->bDirty || pm->LODUpdated ) {
			Patch_DeleteDrawLists( pm );
			Patch_CreateDrawLists( pm );
			pm->bDirty = false;
			pm->LODUpdated = false;
		}
		Patch_DrawLODPatchMesh( pm );
	}
}

/*
   =================
   DrawPatchControls
   =================
 */
void DrawPatchControls( patchMesh_t *pm ){
	int i, j;
	bool bSelectedPoints[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT];

	bool bOverlay = pm->bOverlay;

	// bending
	if ( g_bPatchBendMode ) {
		qglPointSize( 6 );
		if ( g_bPatchAxisOnRow ) {
			qglColor3f( 1, 0, 1 );
			qglBegin( GL_POINTS );
			for ( i = 0; i < pm->width; i++ )
			{
				qglVertex3fv( pm->ctrl[i][g_nPatchAxisIndex].xyz );
			}
			qglEnd();

			if ( g_nPatchBendState == BEND_SELECT_EDGE || g_nPatchBendState == BEND_BENDIT || g_nPatchBendState == BEND_SELECT_ORIGIN ) {
				qglColor3f( 0, 0, 1 );
				qglBegin( GL_POINTS );
				if ( g_nPatchBendState == BEND_SELECT_ORIGIN ) {
					qglVertex3fv( g_vBendOrigin );
				}
				else
				{
					for ( i = 0; i < pm->width; i++ )
					{
						if ( g_bPatchLowerEdge ) {
							for ( j = 0; j < g_nPatchAxisIndex; j++ )
								qglVertex3fv( pm->ctrl[i][j].xyz );
						}
						else
						{
							for ( j = pm->height - 1; j > g_nPatchAxisIndex; j-- )
								qglVertex3fv( pm->ctrl[i][j].xyz );
						}
					}
				}
				qglEnd();
			}
		}
		else
		{
			qglColor3f( 1, 0, 1 );
			qglBegin( GL_POINTS );
			for ( i = 0; i < pm->height; i++ )
			{
				qglVertex3fv( pm->ctrl[g_nPatchAxisIndex][i].xyz );
			}
			qglEnd();

			if ( g_nPatchBendState == BEND_SELECT_EDGE || g_nPatchBendState == BEND_BENDIT || g_nPatchBendState == BEND_SELECT_ORIGIN ) {
				qglColor3f( 0, 0, 1 );
				qglBegin( GL_POINTS );
				for ( i = 0; i < pm->height; i++ )
				{
					if ( g_nPatchBendState == BEND_SELECT_ORIGIN ) {
						qglVertex3fv( pm->ctrl[g_nBendOriginIndex][i].xyz );
					}
					else
					{
						if ( g_bPatchLowerEdge ) {
							for ( j = 0; j < g_nPatchAxisIndex; j++ )
								qglVertex3fv( pm->ctrl[j][i].xyz );
						}
						else
						{
							for ( j = pm->width - 1; j > g_nPatchAxisIndex; j-- )
								qglVertex3fv( pm->ctrl[j][i].xyz );
						}
					}
				}
				qglEnd();
			}
		}
	}
	else
	{
		//qglDisable(GL_TEXTURE_2D); // stops point colours being multiplied by texture colour..
		//draw CV lattice - could be made optional
		//qglDisable( GL_CULL_FACE );
		//    qglPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		qglEnable( GL_POLYGON_OFFSET_LINE );
		if ( g_PrefsDlg.m_bNoStipple == FALSE ) {
			qglDisable( GL_LINE_STIPPLE );
		}
		qglLineWidth( 1 );
		qglColor3f( 1.0f, 0.75f, 0.0f );
		for ( i = 0 ; i + 1 < pm->width ; i++ )
		{
			qglBegin( GL_QUAD_STRIP );
			for ( j = 0 ; j < pm->height ; j++ )
			{
				qglVertex3fv( pm->ctrl[i][j].xyz );
				qglVertex3fv( pm->ctrl[i + 1][j].xyz );
			}
			qglEnd();
		}
		qglDisable( GL_POLYGON_OFFSET_LINE );
		//if (g_PrefsDlg.m_bNoStipple == FALSE)
		//  qglEnable (GL_LINE_STIPPLE);

		// draw selection handles
		qglPointSize( 6 );
		qglBegin( GL_POINTS );
		for ( i = 0 ; i < pm->width ; i++ )
		{
			for ( j = 0 ; j < pm->height ; j++ )
			{
				if ( PointInMoveList( pm->ctrl[i][j].xyz ) != -1 ) {
					bSelectedPoints[i][j] = true;
				}
				else
				{
					bSelectedPoints[i][j] = false;
					if ( i & 0x01 || j & 0x01 ) {
						qglColor3f( 1, 0, 1 );
					}
					else{
						qglColor3f( 0, 1, 0 );
					}
					qglVertex3fv( pm->ctrl[i][j].xyz );
				}
			}
		}
		qglColor3f( 0, 0, 1 );
		for ( i = 0 ; i < pm->width ; i++ )
		{
			for ( j = 0 ; j < pm->height ; j++ )
			{
				if ( bSelectedPoints[i][j] ) {
					qglVertex3fv( pm->ctrl[i][j].xyz );
				}
			}
		}
		qglEnd();
	}
	if ( bOverlay ) {
		qglPointSize( 6 );
		qglBegin( GL_POINTS );
		for ( i = 0 ; i < pm->width ; i++ )
		{
			for ( j = 0 ; j < pm->height ; j++ )
			{
				if ( i & 0x01 || j & 0x01 ) {
					qglColor3f( 1, 0, 1 );
				}
				else{
					qglColor3f( 0, 1, 0 );
				}
				qglVertex3fv( pm->ctrl[i][j].xyz );
			}
		}
		qglEnd();
	}
}

/*
   ==================
   Patch_DrawXY
   ==================
 */
void Patch_DrawXY( patchMesh_t *pm ){
	qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	if ( pm->bSelected ) {
		qglColor3fv( g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES] );
		if ( g_PrefsDlg.m_bNoStipple == FALSE ) {
			qglEnable( GL_LINE_STIPPLE );
		}
		qglLineWidth( 2 );
	}

	DrawPatchMesh( pm );

	if ( ( pm->bSelected && ( g_qeglobals.d_select_mode == sel_curvepoint
							  || g_qeglobals.d_select_mode == sel_area
							  || g_bPatchBendMode ) )
		 || pm->bOverlay ) {
		DrawPatchControls( pm );
	}
}

/*
   ==================
   Patch_DrawCam
   ==================
 */
void Patch_DrawCam( patchMesh_t *pm ){
	qglPushAttrib( GL_ALL_ATTRIB_BITS ); // save the current state

	if ( g_bPatchWireFrame ) {
		qglDisable( GL_CULL_FACE );
		qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		qglDisable( GL_TEXTURE_2D );
		if ( g_PrefsDlg.m_bGLLighting ) {
			qglDisable( GL_LIGHTING );
		}

		DrawPatchMesh( pm );

		//if (g_PrefsDlg.m_bGLLighting)
		//  qglEnable(GL_LIGHTING);
		//qglEnable( GL_CULL_FACE );
	}
	else
	{
		qglDisable( GL_CULL_FACE );
		qglBindTexture( GL_TEXTURE_2D, pm->d_texture->texture_number );
		qglPolygonMode( GL_FRONT, GL_FILL );
		qglPolygonMode( GL_BACK, GL_LINE );

		if ( pm->pShader->getTrans() < 1.0f ) {
			qglEnable( GL_BLEND );
			qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			qglColor4f( pm->d_texture->color[0], pm->d_texture->color[1], pm->d_texture->color[2], pm->pShader->getTrans() );
		}

		DrawPatchMesh( pm ); // both sides
	}

	qglPopAttrib(); // restore saved state
}

void ConvexHullForSection( float section[2][4][7] ) {
}

void BrushesForSection( float section[2][4][7] ) {
}

/*
   ================
   Patch_BuildPoints
   ================
 */
void Patch_BuildPoints( brush_t *b ){
	face_t      *f;
	b->patchBrush = false;
	for ( f = b->brush_faces ; f ; f = f->next )
	{
		if ( f->texdef.flags & SURF_PATCH ) {
			b->patchBrush = true;
			//vec3_t vMin, vMax;
			//Patch_CalcBounds(&patchMeshes[b->nPatchID], vMin, vMax);
			//VectorCopy(vMin, b->mins);
			//VectorCopy(vMax, b->maxs);
			break;
		}
	}
}

/*
   ==================
   Patch_Move
   ==================
 */
void Patch_Move( patchMesh_t *pm, const vec3_t vMove, bool bRebuild ){
	pm->bDirty = true;
	for ( int w = 0; w < pm->width; w++ )
	{
		for ( int h = 0; h < pm->height; h++ )
		{
			VectorAdd( pm->ctrl[w][h].xyz, vMove, pm->ctrl[w][h].xyz );
		}
	}
	// bRebuild is never true
	if ( bRebuild ) {
		vec3_t vMin, vMax;
		Patch_CalcBounds( pm, vMin, vMax );
		//Brush_RebuildBrush(patchMeshes[n].pSymbiot, vMin, vMax);
	}
	UpdatePatchInspector();

}

/*
   ==================
   Patch_ApplyMatrix
   ==================
 */
void Patch_ApplyMatrix( patchMesh_t *p, const vec3_t vOrigin, const vec3_t vMatrix[3], bool bSnap ){
	vec3_t vTemp;

	for ( int w = 0; w < p->width; w++ )
	{
		for ( int h = 0; h < p->height; h++ )
		{
			if ( ( ( g_qeglobals.d_select_mode == sel_curvepoint && g_qeglobals.d_num_move_points != 0 ) || g_bPatchBendMode )
				 && PointInMoveList( p->ctrl[w][h].xyz ) == -1 ) { // snap selected points only, if selected
				continue;
			}
			VectorSubtract( p->ctrl[w][h].xyz, vOrigin, vTemp );
			for ( int j = 0; j < 3; j++ )
			{
				p->ctrl[w][h].xyz[j] = DotProduct( vTemp, vMatrix[j] ) + vOrigin[j];
				if ( bSnap ) {
					p->ctrl[w][h].xyz[j] = floor( p->ctrl[w][h].xyz[j] + 0.5 );
				}
			}
		}
	}
	vec3_t vMin, vMax;
	Patch_CalcBounds( p, vMin, vMax );
	Brush_RebuildBrush( p->pSymbiot, vMin, vMax );
}

/*
   ==================
   Patch_EditPatch
   ==================
 */
void Patch_EditPatch(){
	//--patchMesh_t* p = &patchMeshes[n];
	g_qeglobals.d_numpoints = 0;
	g_qeglobals.d_num_move_points = 0;

	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			patchMesh_t* p = pb->pPatch;
			for ( int i = 0 ; i < p->width ; i++ )
			{
				for ( int j = 0 ; j < p->height ; j++ )
				{
					VectorCopy( p->ctrl[i][j].xyz, g_qeglobals.d_points[g_qeglobals.d_numpoints] );
					if ( g_qeglobals.d_numpoints < MAX_POINTS - 1 ) {
						g_qeglobals.d_numpoints++;
					}
				}
			}
		}
	}
	g_qeglobals.d_select_mode = sel_curvepoint;
	//--g_nSelectedPatch = n;
}



/*
   ==================
   Patch_Deselect
   ==================
 */
//FIXME: need all sorts of asserts throughout a lot of this crap
void Patch_Deselect(){
	//--g_nSelectedPatch = -1;
	g_qeglobals.d_select_mode = sel_brush;

	for ( brush_t *b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
	{
		if ( b->patchBrush ) {
			b->pPatch->bSelected = false;
		}
	}

	//for (int i = 0; i < numPatchMeshes; i++)
	//  patchMeshes[i].bSelected = false;

	if ( g_bPatchBendMode ) {
		Patch_BendToggle();
	}
//  if (g_bPatchInsertMode)
//    Patch_InsDelToggle();
}


/*
   ==================
   Patch_Select
   ==================
 */
void Patch_Select( patchMesh_t *p ){
	// maintained for point manip.. which i need to fix as this
	// is pf error prone
	//--g_nSelectedPatch = n;
	p->bSelected = true;
}


/*
   ==================
   Patch_Deselect
   ==================
 */
void Patch_Deselect( patchMesh_t *p ){
	p->bSelected = false;
}


/*
   ==================
   Patch_Delete
   ==================
 */
extern BTNode_t *BTree_Delete( BTNode_t *pBT );
extern BTListList_t *BTree_DeleteListFromList( BTListList_t *pBTListList );

void Patch_Delete( patchMesh_t *p ){
	if ( p->pSymbiot ) { // Hydra - added a check to prevent access violations.
		p->pSymbiot->pPatch = NULL;
		p->pSymbiot->patchBrush = false;
	}

	// spog - free dynamically allocated memory used by LODs
	int rowcount = ( ( MAX_PATCH_WIDTH - 1 ) / 2 ) * MAX_PATCH_HEIGHT;
	int colcount = ( ( MAX_PATCH_HEIGHT - 1 ) / 2 ) * MAX_PATCH_WIDTH;
	int i;
	for ( i = 0; i < rowcount; i++ )
		p->rowLOD[i] = BTree_Delete( p->rowLOD[i] );
	for ( i = 0; i < colcount; i++ )
		p->colLOD[i] = BTree_Delete( p->colLOD[i] );

	// delete display list associated with patch
	if ( p->nListID != -1 ) {
		qglDeleteLists( p->nListID, 1 ); // list#, number of lists

	}
	// delete LOD drawLists
	Patch_DeleteDrawLists( p );


	free( p );
	p = NULL;


	UpdatePatchInspector();
}


/*
   ==================
   Patch_Scale
   ==================
 */
void Patch_Scale( patchMesh_t *p, const vec3_t vOrigin, const vec3_t vAmt, bool bRebuild ){

	for ( int w = 0; w < p->width; w++ )
	{
		for ( int h = 0; h < p->height; h++ )
		{
			if ( g_qeglobals.d_select_mode == sel_curvepoint && PointInMoveList( p->ctrl[w][h].xyz ) == -1 ) {
				continue;
			}
			for ( int i = 0 ; i < 3 ; i++ )
			{
				p->ctrl[w][h].xyz[i] -= vOrigin[i];
				p->ctrl[w][h].xyz[i] *= vAmt[i];
				p->ctrl[w][h].xyz[i] += vOrigin[i];
			}
		}
	}
	if ( bRebuild ) {
		vec3_t vMin, vMax;
		Patch_CalcBounds( p, vMin, vMax );
		Brush_RebuildBrush( p->pSymbiot, vMin, vMax );
	}
	UpdatePatchInspector();
}


/*
   ==================
   Patch_SetView
   ==================
 */
void Patch_SetView( int n ){
	g_bSameView = ( n == g_nPatchClickedView );
	g_nPatchClickedView = n;
}


/*
   ==================
   Patch_SetTexture
   ==================
 */
// FIXME: need array validation throughout
void Patch_SetTexture( patchMesh_t *p, texdef_t *tex_def, IPluginTexdef* pPlugTexdef ){
	// NOTE: I don't know for sure if this happens
	if ( p->pShader ) {
		p->pShader->DecRef();
	}
	p->pShader = QERApp_Shader_ForName( tex_def->GetName() );
	p->pShader->IncRef();
	p->d_texture = p->pShader->getTexture();

	UpdatePatchInspector();
}


/*
   ==================
   Patch_DragScale
   ==================
 */
bool Patch_DragScale( patchMesh_t *p, vec3_t vAmt, vec3_t vMove ){
	vec3_t vMin, vMax, vScale, vTemp, vMid;
	int i;

	Patch_CalcBounds( p, vMin, vMax );

	VectorSubtract( vMax, vMin, vTemp );

	// if we are scaling in the same dimension the patch has no depth
	for ( i = 0; i < 3; i++ )
	{
		if ( vTemp[i] == 0 && vMove[i] != 0 ) {
			//Patch_Move(n, vMove, true);
			return false;
		}
	}

	for ( i = 0 ; i < 3 ; i++ )
		vMid[i] = ( vMin[i] + ( ( vMax[i] - vMin[i] ) / 2 ) );

	for ( i = 0; i < 3; i++ )
	{
		if ( vAmt[i] != 0 ) {
			vScale[i] = 1.0 + vAmt[i] / vTemp[i];
		}
		else
		{
			vScale[i] = 1.0;
		}
	}

	Patch_Scale( p, vMid, vScale, false );

	VectorSubtract( vMax, vMin, vTemp );

	Patch_CalcBounds( p, vMin, vMax );

	VectorSubtract( vMax, vMin, vMid );

	VectorSubtract( vMid, vTemp, vTemp );

	VectorScale( vTemp, 0.5, vTemp );

	// abs of both should always be equal
	if ( !VectorCompare( vMove, vAmt ) ) {
		for ( i = 0; i < 3; i++ )
		{
			if ( vMove[i] != vAmt[i] ) {
				vTemp[i] = -( vTemp[i] );
			}
		}
	}

	Patch_Move( p, vTemp );
	return true;
}

/*
   ==================
   Patch_InsertColumn
   ==================
 */
void Patch_InsertColumn( patchMesh_t *p, bool bAdd ){
	int w, h, i, width;
	vec3_t vTemp;
	float stTemp[2];

	if ( p->width + 2 >= MAX_PATCH_WIDTH ) {
		return;
	}

	w = 1;
	// check for selected column points
	for ( h = 0; h < p->height; h++ )
	{
		for ( w = 1; w < p->width; w += 2 )
			if ( PointInMoveList( p->ctrl[w][h].xyz ) != -1 ) {
				break;
			}
		if ( w < p->width ) {
			break;
		}
		for ( w = 0; w < p->width; w += 2 )
			if ( PointInMoveList( p->ctrl[w][h].xyz ) != -1 ) {
				break;
			}
		if ( w < p->width ) {
			break;
		}
	}

	if ( w >= p->width ) {
		if ( bAdd ) {
			w = p->width - 1;
		}
		else{w = 2; }
	}
	else if ( w == 0 ) {
		w = 2;
	}
	else if ( w % 2 ) {
		w++;
	}

	// add columns at w
	for ( h = 0; h < p->height; h++ )
	{
		for ( width = p->width - 1; width > w; width-- )
			memcpy( &p->ctrl[width + 2][h],&p->ctrl[width][h], sizeof( drawVert_t ) );

		// set two new column points
		memcpy( &p->ctrl[w + 2][h],&p->ctrl[w][h], sizeof( drawVert_t ) );
		memcpy( &p->ctrl[w + 1][h],&p->ctrl[w - 1][h], sizeof( drawVert_t ) );

		for ( i = 0; i < 3; i++ ) // xyz
		{
			vTemp[i] = p->ctrl[w][h].xyz[i] - p->ctrl[w - 1][h].xyz[i];
			p->ctrl[w + 1][h].xyz[i] = p->ctrl[w + 1][h].xyz[i] + ( vTemp[i] / 2 );

			vTemp[i] = p->ctrl[w - 2][h].xyz[i] - p->ctrl[w - 1][h].xyz[i];
			p->ctrl[w - 1][h].xyz[i] = p->ctrl[w - 1][h].xyz[i] + ( vTemp[i] / 2 );

			vTemp[i] = p->ctrl[w + 1][h].xyz[i] - p->ctrl[w - 1][h].xyz[i];
			p->ctrl[w][h].xyz[i] = p->ctrl[w - 1][h].xyz[i] + ( vTemp[i] / 2 );
		}
		for ( i = 0; i < 2; i++ ) // st
		{
			stTemp[i] = p->ctrl[w][h].st[i] - p->ctrl[w - 1][h].st[i];
			p->ctrl[w + 1][h].st[i] = p->ctrl[w + 1][h].st[i] + ( stTemp[i] / 2 );

			stTemp[i] = p->ctrl[w - 2][h].st[i] - p->ctrl[w - 1][h].st[i];
			p->ctrl[w - 1][h].st[i] = p->ctrl[w - 1][h].st[i] + ( stTemp[i] / 2 );

			stTemp[i] = p->ctrl[w + 1][h].st[i] - p->ctrl[w - 1][h].st[i];
			p->ctrl[w][h].st[i] = p->ctrl[w - 1][h].st[i] + ( stTemp[i] / 2 );
		}
	}

	p->width += 2;
	// deselect all points to keep things neat
	if ( g_qeglobals.d_select_mode == sel_curvepoint ) {
		Patch_EditPatch();
	}

	UpdatePatchInspector();
}

/*
   ==================
   Patch_InsertRow
   ==================
 */

void Patch_InsertRow( patchMesh_t *p, bool bAdd ){
	int h, w, i, height;
	vec3_t vTemp;
	float stTemp[2];

	if ( p->height + 2 >= MAX_PATCH_HEIGHT ) {
		return;
	}

	h = 1;
	// check for selected row points
	for ( w = 0; w < p->width; w++ )
	{
		for ( h = 1; h < p->height; h += 2 )
			if ( PointInMoveList( p->ctrl[w][h].xyz ) != -1 ) {
				break;
			}
		if ( h < p->height ) {
			break;
		}
		for ( h = 0; h < p->height; h += 2 )
			if ( PointInMoveList( p->ctrl[w][h].xyz ) != -1 ) {
				break;
			}
		if ( h < p->height ) {
			break;
		}
	}
	if ( h >= p->height ) {
		if ( bAdd ) {
			h = p->height - 1;
		}
		else{h = 2; }
	}
	else if ( h == 0 ) {
		h = 2;
	}
	else if ( h % 2 ) {
		h++;
	}

	// add rows at h
	for ( w = 0; w < p->width; w++ )
	{
		for ( height = p->height - 1; height > h; height-- )
			memcpy( &p->ctrl[w][height + 2],&p->ctrl[w][height], sizeof( drawVert_t ) );

		// set two new row points
		memcpy( &p->ctrl[w][h + 2],&p->ctrl[w][h], sizeof( drawVert_t ) );
		memcpy( &p->ctrl[w][h + 1],&p->ctrl[w][h - 1], sizeof( drawVert_t ) );

		for ( i = 0; i < 3; i++ ) // xyz
		{
			vTemp[i] = p->ctrl[w][h].xyz[i] - p->ctrl[w][h - 1].xyz[i];
			p->ctrl[w][h + 1].xyz[i] = p->ctrl[w][h + 1].xyz[i] + ( vTemp[i] / 2 );

			vTemp[i] = p->ctrl[w][h - 2].xyz[i] - p->ctrl[w][h - 1].xyz[i];
			p->ctrl[w][h - 1].xyz[i] = p->ctrl[w][h - 1].xyz[i] + ( vTemp[i] / 2 );

			vTemp[i] = p->ctrl[w][h + 1].xyz[i] - p->ctrl[w][h - 1].xyz[i];
			p->ctrl[w][h].xyz[i] = p->ctrl[w][h - 1].xyz[i] + ( vTemp[i] / 2 );
		}
		for ( i = 0; i < 2; i++ ) // st
		{
			stTemp[i] = p->ctrl[w][h].st[i] - p->ctrl[w][h - 1].st[i];
			p->ctrl[w][h + 1].st[i] = p->ctrl[w][h + 1].st[i] + ( stTemp[i] / 2 );

			stTemp[i] = p->ctrl[w][h - 2].st[i] - p->ctrl[w][h - 1].st[i];
			p->ctrl[w][h - 1].st[i] = p->ctrl[w][h - 1].st[i] + ( stTemp[i] / 2 );

			stTemp[i] = p->ctrl[w][h + 1].st[i] - p->ctrl[w][h - 1].st[i];
			p->ctrl[w][h].st[i] = p->ctrl[w][h - 1].st[i] + ( stTemp[i] / 2 );
		}
	}

	p->height += 2;
	// deselect all points to keep things neat
	if ( g_qeglobals.d_select_mode == sel_curvepoint ) {
		Patch_EditPatch();
	}

	UpdatePatchInspector();
}

/*
   ==================
   Patch_RemoveRow
   ==================
 */
void Patch_RemoveRow( patchMesh_t *p, bool bFirst ){
	int w, h, i, height;
	vec3_t vTemp;
	float stTemp[2];
	bool bExtrapolate = true;

	if ( p->height <= MIN_PATCH_HEIGHT ) {
		return;
	}

	h = 0;
	for ( w = 0; w < p->width; w++ )
	{
		for ( h = 0; h < p->height; h += 2 )
			if ( PointInMoveList( p->ctrl[w][h].xyz ) != -1 ) {
				break;
			}
		if ( h < p->height ) {
			break;
		}
		for ( h = 1; h < p->height; h += 2 )
			if ( PointInMoveList( p->ctrl[w][h].xyz ) != -1 ) {
				break;
			}
		if ( h < p->height ) {
			break;
		}
	}

	if ( h >= p->height ) {
		bExtrapolate = false;
		if ( bFirst ) {
			h = p->height - 3;
		}
		else{h = 2; }
	}
	else if ( h <= 0 ) {
		h = 2;
	}
	else if ( h > p->height - 3 ) {
		h = p->height - 3;
	}
	else if ( h % 2 ) {
		h++;
	}

	p->height -= 2;

	for ( w = 0; w < p->width; w++ )
	{
		if ( bExtrapolate ) {
			for ( i = 0; i < 3; i++ ) // xyz
			{
				vTemp[i] = p->ctrl[w][h + 2].xyz[i] - p->ctrl[w][h - 2].xyz[i];
				p->ctrl[w][h - 1].xyz[i] = p->ctrl[w][h - 2].xyz[i] + ( vTemp[i] / 2 );

				vTemp[i] = p->ctrl[w][h].xyz[i] - p->ctrl[w][h - 1].xyz[i];
				p->ctrl[w][h - 1].xyz[i] = p->ctrl[w][h - 1].xyz[i] + ( vTemp[i] * 2 );
			}

			for ( i = 0; i < 2; i++ ) // st
			{
				stTemp[i] = p->ctrl[w][h + 2].st[i] - p->ctrl[w][h - 2].st[i];
				p->ctrl[w][h - 1].st[i] = p->ctrl[w][h - 2].st[i] + ( stTemp[i] / 2 );

				stTemp[i] = p->ctrl[w][h].st[i] - p->ctrl[w][h - 1].st[i];
				p->ctrl[w][h - 1].st[i] = p->ctrl[w][h - 1].st[i] + ( stTemp[i] * 2 );
			}
		}
		else
		{
			if ( !bFirst ) {
				continue;
			}
			else{h = 0; }
		}
		for ( height = h; height < p->height; height++ )
			memcpy( &p->ctrl[w][height], &p->ctrl[w][height + 2], sizeof( drawVert_t ) );
	}
	// deselect all points to keep things neat
	if ( g_qeglobals.d_select_mode == sel_curvepoint ) {
		Patch_EditPatch();
	}

	UpdatePatchInspector();
}

/*
   ==================
   Patch_RemoveColumn
   ==================
 */
void Patch_RemoveColumn( patchMesh_t *p, bool bFirst ){
	int w, h, i, width;
	vec3_t vTemp;
	float stTemp[2];
	bool bExtrapolate = true;

	if ( p->width <= MIN_PATCH_WIDTH ) {
		return;
	}

	w = 0;
	for ( h = 0; h < p->height; h++ )
	{
		for ( w = 0; w < p->width; w += 2 )
			if ( PointInMoveList( p->ctrl[w][h].xyz ) != -1 ) {
				break;
			}
		if ( w < p->width ) {
			break;
		}
		for ( w = 1; w < p->width; w += 2 )
			if ( PointInMoveList( p->ctrl[w][h].xyz ) != -1 ) {
				break;
			}
		if ( w < p->width ) {
			break;
		}
	}

	if ( w >= p->width ) {
		bExtrapolate = false;
		if ( bFirst ) {
			w = p->width - 3;
		}
		else{w = 2; }
	}
	else if ( w <= 0 ) {
		w = 2;
	}
	else if ( w > p->width - 3 ) {
		w = p->width - 3;
	}
	else if ( w % 2 ) {
		w++;
	}

	p->width -= 2;

	for ( h = 0; h < p->height; h++ )
	{
		if ( bExtrapolate ) {
			for ( i = 0; i < 3; i++ ) // xyz
			{
				vTemp[i] = p->ctrl[w + 2][h].xyz[i] - p->ctrl[w - 2][h].xyz[i];
				p->ctrl[w - 1][h].xyz[i] = p->ctrl[w - 2][h].xyz[i] + ( vTemp[i] / 2 );

				vTemp[i] = p->ctrl[w][h].xyz[i] - p->ctrl[w - 1][h].xyz[i];
				p->ctrl[w - 1][h].xyz[i] = p->ctrl[w - 1][h].xyz[i] + ( vTemp[i] * 2 );
			}

			for ( i = 0; i < 2; i++ ) // st
			{
				stTemp[i] = p->ctrl[w + 2][h].st[i] - p->ctrl[w - 2][h].st[i];
				p->ctrl[w - 1][h].st[i] = p->ctrl[w - 2][h].st[i] + ( stTemp[i] / 2 );

				stTemp[i] = p->ctrl[w][h].st[i] - p->ctrl[w - 1][h].st[i];
				p->ctrl[w - 1][h].st[i] = p->ctrl[w - 1][h].st[i] + ( stTemp[i] * 2 );
			}
		}
		else
		{
			if ( !bFirst ) {
				continue;
			}
			else{w = 0; }
		}

		for ( width = w; width < p->width; width++ )
			memcpy( &p->ctrl[width][h], &p->ctrl[width + 2][h], sizeof( drawVert_t ) );
	}
	// deselect all points to keep things neat
	if ( g_qeglobals.d_select_mode == sel_curvepoint ) {
		Patch_EditPatch();
	}

	UpdatePatchInspector();
}

/*
   ==================
   Patch_AdjustColumns
   ==================
 */
/*
   void Patch_AdjustColumns(patchMesh_t *p, int nCols)
   {
   vec3_t vTemp, vTemp2;
   int i, w, h;

   if (nCols & 0x01 || p->width + nCols < 3 || p->width + nCols > MAX_PATCH_WIDTH)
    return;

   // add in column adjustment
   p->width += nCols;

   for (h = 0; h < p->height; h++)
   {
    // for each column, we need to evenly disperse p->width number
    // of points across the old bounds

    // calc total distance to interpolate
    VectorSubtract(p->ctrl[p->width - 1 - nCols][h].xyz, p->ctrl[0][h].xyz, vTemp);

    // amount per cycle
    for (i = 0; i < 3; i ++)
    {
      vTemp2[i] = vTemp[i] / (p->width - 1);
    }

    // move along
    for (w = 0; w < p->width-1; w++)
    {
      VectorAdd(p->ctrl[w][h].xyz, vTemp2, p->ctrl[w+1][h].xyz);
    }

   }
    for ( w = 0 ; w < p->width ; w++ )
   {
        for ( h = 0 ; h < p->height ; h++ )
    {
            p->ctrl[w][h].st[0] = 4 * (float)w / (p->width - 1);
            p->ctrl[w][h].st[1] = 4 * (float)h / (p->height - 1);
        }
    }
   UpdatePatchInspector();
   }
 */

/*
   ==================
   Patch_AdjustRows
   ==================
 */
/*
   void Patch_AdjustRows(patchMesh_t *p, int nRows)
   {
   vec3_t vTemp, vTemp2;
   int i, w, h;

   if (nRows & 0x01 || p->height + nRows < 3 || p->height + nRows > MAX_PATCH_HEIGHT)
    return;

   // add in column adjustment
   p->height += nRows;

   for (w = 0; w < p->width; w++)
   {
    // for each row, we need to evenly disperse p->height number
    // of points across the old bounds

    // calc total distance to interpolate
    VectorSubtract(p->ctrl[w][p->height - 1 - nRows].xyz, p->ctrl[w][0].xyz, vTemp);

    //vTemp[0] = vTemp[1] = vTemp[2] = 0;
    //for (h = 0; h < p->height - nRows; h ++)
    //{
    //  VectorAdd(vTemp, p->ctrl[w][h], vTemp);
    //}

    // amount per cycle
    for (i = 0; i < 3; i ++)
    {
      vTemp2[i] = vTemp[i] / (p->height - 1);
    }

    // move along
    for (h = 0; h < p->height-1; h++)
    {
      VectorAdd(p->ctrl[w][h].xyz, vTemp2, p->ctrl[w][h+1].xyz);
    }

   }
    for ( w = 0 ; w < p->width ; w++ )
   {
        for ( h = 0 ; h < p->height ; h++ )
    {
            p->ctrl[w][h].st[0] = 4 * (float)w / (p->width - 1);
            p->ctrl[w][h].st[1] = 4 * (float)h / (p->height - 1);
        }
    }
   UpdatePatchInspector();
   }
 */

/*
   ==================
   Patch_DisperseRows
   ==================
 */

void Patch_DisperseRows(){
	vec3_t vTemp, vTemp2;
	int i, w, h;


	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			patchMesh_t *p = pb->pPatch;
			Patch_Rebuild( p );
			for ( w = 0; w < p->width; w++ )
			{
				// for each row, we need to evenly disperse p->height number
				// of points across the old bounds

				// calc total distance to interpolate
				VectorSubtract( p->ctrl[w][p->height - 1].xyz, p->ctrl[w][0].xyz, vTemp );

				//vTemp[0] = vTemp[1] = vTemp[2] = 0;
				//for (h = 0; h < p->height - nRows; h ++)
				//{
				//  VectorAdd(vTemp, p->ctrl[w][h], vTemp);
				//}

				// amount per cycle
				for ( i = 0; i < 3; i++ )
				{
					vTemp2[i] = vTemp[i] / ( p->height - 1 );
				}

				// move along
				for ( h = 0; h < p->height - 1; h++ )
				{
					VectorAdd( p->ctrl[w][h].xyz, vTemp2, p->ctrl[w][h + 1].xyz );
				}
				Patch_Naturalize( p );

			}
		}
	}
	UpdatePatchInspector();
}

/*
   ==================
   Patch_DisperseIntermediateRows
   ==================
 */

void Patch_DisperseIntermediateRows(){
	vec3_t vTemp, vTemp2;
	int i, w, h;


	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			patchMesh_t *p = pb->pPatch;
			Patch_Rebuild( p );
			for ( w = 0; w < p->width; w++ )
			{
				// move along
				for ( h = 0; h < p->height; h += 2 )
				{
					// calc distance to interpolate
					VectorSubtract( p->ctrl[w][h + 2].xyz, p->ctrl[w][h].xyz, vTemp );

					// halve distance
					for ( i = 0; i < 3; i++ )
					{
						vTemp2[i] = vTemp[i] / 2;
					}

					// move control points
					VectorAdd( p->ctrl[w][h].xyz, vTemp2, p->ctrl[w][h + 1].xyz );
				}
			}
		}
	}
	UpdatePatchInspector();
}

/*
   ==================
   Patch_DisperseIntermediateColumns
   ==================
 */
void Patch_DisperseIntermediateColumns(){
	vec3_t vTemp, vTemp2;
	int i, w, h;


	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			patchMesh_t *p = pb->pPatch;
			Patch_Rebuild( p );
			for ( h = 0; h < p->height; h++ )
			{
				// move along
				for ( w = 0; w < p->width; w += 2 )
				{
					// calc distance to interpolate
					VectorSubtract( p->ctrl[w + 2][h].xyz, p->ctrl[w][h].xyz, vTemp );

					// halve distance
					for ( i = 0; i < 3; i++ )
					{
						vTemp2[i] = vTemp[i] / 2;
					}

					// move control points
					VectorAdd( p->ctrl[w][h].xyz, vTemp2, p->ctrl[w + 1][h].xyz );
				}
			}
		}
	}
	UpdatePatchInspector();
}



/*
   ==================
   Patch_AdjustSelected
   ==================
 */
void Patch_AdjustSelected( bool bInsert, bool bColumn, bool bFlag ){
	bool bUpdate = false;
	for ( brush_t* pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			if ( bInsert ) {
				if ( bColumn ) {
					Patch_InsertColumn( pb->pPatch, bFlag );
				}
				else
				{
					Patch_InsertRow( pb->pPatch, bFlag );
				}
			}
			else
			{
				if ( bColumn ) {
					Patch_RemoveColumn( pb->pPatch, bFlag );
				}
				else
				{
					Patch_RemoveRow( pb->pPatch, bFlag );
				}
			}
			bUpdate = true;
			vec3_t vMin, vMax;
			patchMesh_t *p = pb->pPatch;
			Patch_CalcBounds( p, vMin, vMax );
			Brush_RebuildBrush( p->pSymbiot, vMin, vMax );
			pb->pPatch->bDirty = true; // rebuild LOD trees and their normals
		}
	}
	if ( bUpdate ) {
		Sys_UpdateWindows( W_ALL );
	}
}


/*
   ==================
   Patch_AdjustSelectedRowCols
   ==================
 */
/*
   void Patch_AdjustSelectedRowCols(int nRows, int nCols)
   {
    for (brush_t* pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next)
    {
    if (pb->patchBrush)
    {
      Patch_InsertColumn(pb->pPatch, false);
      if (nRows != 0)
      {
        Patch_AdjustRows(pb->pPatch, nRows);
      }

      if (nCols != 0)
      {
        Patch_AdjustColumns(pb->pPatch, nCols);
      }
        }
   }
   UpdatePatchInspector();
   }
 */

/*
   =================
   CheckName
   temporary stuff, detect potential problems when saving the texture name
   will correct the patch on the fly if problem detected
   =================
 */
/*!
   \todo performance issue with CheckName calls
   don't call this too much, only when absolutely necessary
   strategies that call here too much are known to be slow
   patch 84 to bug 253 adds an additionnal check for textures/
 */
void CheckName( patchMesh_t *p, char *pname ){
	if ( strncmp( p->pShader->getName(), "textures/", 9 ) != 0 ) {
		p->pShader = QERApp_Shader_ForName( SHADER_NOT_FOUND );
	}

	// some manage to get long filename textures (with spaces) in their maps
	if ( strchr( p->pShader->getName(), ' ' ) ) {
		char Msg1[1024];
		sprintf( Msg1, _( "Can't save texture with spaces in name. Rename %s\nNOTE: This message may popup several times .. once for each buggy face detected." ), p->pShader->getName() );
		Sys_Printf( "%s\n", Msg1 );
		gtk_MessageBox( g_pParentWnd->m_pWidget, Msg1, _( "Error saving map" ), MB_OK );
		strcpy( pname, SHADER_NOT_FOUND );
		p->pShader = QERApp_Shader_ForName( SHADER_NOT_FOUND );
		p->d_texture = p->pShader->getTexture();
		return;
	}
	strcpy( pname, p->pShader->getName() + 9 ); // remove "textures/"
}

/*
   ==================
   Patch_Write
   ==================
 */
void Patch_Write( patchMesh_t *p, MemStream *file ){
	char pname[1024];

	MemFile_fprintf( file, " {\n  patchDef2\n  {\n" );

	CheckName( p, pname );
	MemFile_fprintf( file, "   %s\n", pname );
	MemFile_fprintf( file, "   ( %i %i %i %i %i ) \n", p->width, p->height, p->contents, p->flags, p->value );


	float ctrl[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT][5];

	int w, h;
	for ( w = 0; w < p->width; w++ )
	{
		for ( h = 0; h < p->height; h++ )
		{
			ctrl[w][h][0] = p->ctrl[w][h].xyz[0];
			ctrl[w][h][1] = p->ctrl[w][h].xyz[1];
			ctrl[w][h][2] = p->ctrl[w][h].xyz[2];
			ctrl[w][h][3] = p->ctrl[w][h].st[0];
			ctrl[w][h][4] = p->ctrl[w][h].st[1];
		}
	}

	_Write3DMatrix( file, p->width, p->height, 5, reinterpret_cast<float*>( &ctrl ) );

	if ( g_qeglobals.m_bBrushPrimitMode ) {
		if ( p->epairs ) {
			for ( epair_t *ep = p->epairs ; ep ; ep = ep->next )
			{
				MemFile_fprintf( file, "\"%s\" \"%s\"\n", ep->key, ep->value );
			}
		}
	}

	MemFile_fprintf( file, "  }\n }\n" );
}

void Patch_Write( patchMesh_t *p, FILE *file ){
	char pname[1024];

	fprintf( file, " {\n  patchDef2\n  {\n" );
	{
		CheckName( p, pname );
		fprintf( file, "   %s\n", pname );
		fprintf( file, "   ( %i %i %i %i %i ) \n", p->width, p->height, p->contents, p->flags, p->value );
	}

	float ctrl[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT][5];

	int w, h;
	for ( w = 0; w < p->width; w++ )
	{
		for ( h = 0; h < p->height; h++ )
		{
			ctrl[w][h][0] = p->ctrl[w][h].xyz[0];
			ctrl[w][h][1] = p->ctrl[w][h].xyz[1];
			ctrl[w][h][2] = p->ctrl[w][h].xyz[2];
			ctrl[w][h][3] = p->ctrl[w][h].st[0];
			ctrl[w][h][4] = p->ctrl[w][h].st[1];
		}
	}

	_Write3DMatrix( file, p->width, p->height, 5, reinterpret_cast<float*>( &ctrl ) );

	if ( g_qeglobals.m_bBrushPrimitMode ) {
		if ( p->epairs ) {
			for ( epair_t *ep = p->epairs ; ep ; ep = ep->next )
			{
				fprintf( file, "\"%s\" \"%s\"\n", ep->key, ep->value );
			}
		}
	}

	fprintf( file, "  }\n }\n" );
}


/*
   ==================
   Patch_RotateTexture
   ==================
 */
void Patch_RotateTexture( patchMesh_t *p, float fAngle ){
	p->bDirty = true;
	float c = cos( fAngle * Q_PI / 180 );
	float s = sin( fAngle * Q_PI / 180 );

	Patch_TransformLODTexture( p, c, s, ROTATE );

	for ( int w = 0; w < p->width; w++ )
	{
		for ( int h = 0; h < p->height; h++ )
		{
			//if (g_qeglobals.d_select_mode == sel_curvepoint && PointInMoveList(p->ctrl[w][h].xyz) == -1)
			//  continue;

			float x = p->ctrl[w][h].st[0];
			float y = p->ctrl[w][h].st[1];
			p->ctrl[w][h].st[0] = x * c - y * s;
			p->ctrl[w][h].st[1] = y * c + x * s;
		}
	}
}


/*
   ==================
   Patch_ScaleTexture
   ==================
 */
void Patch_ScaleTexture( patchMesh_t *p, float fx, float fy, bool bFixup ){
	// FIXME:
	// this hack turns scales into 1.1 or 0.9
	if ( bFixup ) {
		fx = ( fx == 0 ) ? 1.0 : ( fx > 0 ) ? 0.9 : 1.10;
		fy = ( fy == 0 ) ? 1.0 : ( fy > 0 ) ? 0.9 : 1.10;
	}
	else
	{
		if ( fx == 0 ) {
			fx = 1.0;
		}
		if ( fy == 0 ) {
			fy = 1.0;
		}
	}

	for ( int w = 0; w < p->width; w++ )
	{
		for ( int h = 0; h < p->height; h++ )
		{
			if ( g_qeglobals.d_select_mode == sel_curvepoint && PointInMoveList( p->ctrl[w][h].xyz ) == -1 ) {
				continue;
			}

			p->ctrl[w][h].st[0] *= fx;
			p->ctrl[w][h].st[1] *= fy;
		}
	}
	if ( g_qeglobals.d_select_mode == sel_curvepoint ) {
		p->bDirty = true;
		Patch_LODMatchAll();
	}
	else
	{
		Patch_TransformLODTexture( p, fx, fy, SCALE );
		p->LODUpdated = true;
	}
}


/*
   ==================
   Patch_ShiftTexture
   shift a texture given a pixel count
   ==================
 */
void Patch_ShiftTexture( patchMesh_t *p, float fx, float fy ){
	qtexture_t *pTex;
	pTex = p->pShader->getTexture();
	fx = -1 * fx / pTex->width;
	fy = fy / pTex->height;
	Patch_ShiftTextureST( p, fx, fy );
}

/*
   ====================
   Patch_ShiftTextureST
   shift a patch texture given an ST increment
   ====================
 */
void Patch_ShiftTextureST( patchMesh_t *p, float fx, float fy ){
#ifdef _DEBUG
	// NOTE: when called by Patch_ShiftTexture this warning may be bogus
	if ( ( ABS( fx ) >= 1 ) || ( ABS( fy ) >= 1 ) ) {
		Sys_FPrintf( SYS_WRN, "WARNING: increments exceed 1 in Patch_ShiftTextureST\n" );
	}
#endif
	for ( int w = 0; w < p->width; w++ )
	{
		for ( int h = 0; h < p->height; h++ )
		{
			if ( g_qeglobals.d_select_mode == sel_curvepoint && PointInMoveList( p->ctrl[w][h].xyz ) == -1 ) {
				continue;
			}

			p->ctrl[w][h].st[0] += fx;
			p->ctrl[w][h].st[1] += fy;
		}
	}
	if ( g_qeglobals.d_select_mode == sel_curvepoint ) {
		p->bDirty = true;
		Patch_LODMatchAll();
	}
	else
	{
		Patch_TransformLODTexture( p, fx, fy, TRANSLATE );
		p->LODUpdated = true;
	}
}

/*
   ==================
   Patch_ToggleInverted
   ==================
 */
void Patch_ToggleInverted(){
	bool bUpdate = false;

	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			bUpdate = true;
			patchInvert( pb->pPatch );
		}
	}

	if ( bUpdate ) {
		Sys_UpdateWindows( W_ALL );
	}
	UpdatePatchInspector();
}

/*
   ==================
   Patch_ToggleInverted
   ==================
 */
void Patch_InvertTexture( bool bY ){
	bool bUpdate = false;

	float fTemp[2];
	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			bUpdate = true;
			patchMesh_t *p = pb->pPatch;
			p->bDirty = true;
			if ( bY ) {
				for ( int i = 0 ; i < p->height ; i++ )
				{
					for ( int j = 0; j < p->width / 2; j++ )
					{
						memcpy( fTemp, &p->ctrl[p->width - 1 - j][i].st[0], sizeof( float[2] ) );
						memcpy( &p->ctrl[p->width - 1 - j][i].st[0], &p->ctrl[j][i].st[0], sizeof( float[2] ) );
						memcpy( &p->ctrl[j][i].st[0], fTemp, sizeof( float[2] ) );
					}
				}
			}
			else
			{
				for ( int i = 0 ; i < p->width ; i++ )
				{
					for ( int j = 0; j < p->height / 2; j++ )
					{
						memcpy( fTemp, &p->ctrl[i][p->height - 1 - j].st[0], sizeof( float[2] ) );
						memcpy( &p->ctrl[i][p->height - 1 - j].st[0], &p->ctrl[i][j].st[0], sizeof( float[2] ) );
						memcpy( &p->ctrl[i][j].st[0], fTemp, sizeof( float[2] ) );
					}
				}
			}
		}
	}

	if ( bUpdate ) {
		Sys_UpdateWindows( W_ALL );
	}
	UpdatePatchInspector();
}




/*
   ==================
   Patch_Save
   ==================
   Saves patch ctrl info (originally to deal with a
   cancel in the surface dialog
 */
void Patch_Save( patchMesh_t *p ){
	patchSave.width = p->width;
	patchSave.height = p->height;
	memcpy( patchSave.ctrl, p->ctrl, sizeof( p->ctrl ) );
}


/*
   ==================
   Patch_Restore
   ==================
 */
void Patch_Restore( patchMesh_t *p ){
	p->width = patchSave.width;
	p->height = patchSave.height;
	memcpy( p->ctrl, patchSave.ctrl, sizeof( p->ctrl ) );
}

void Patch_ResetTexturing( float fx, float fy ){
	for ( brush_t* pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			patchMesh_t *p = pb->pPatch;
			p->bDirty = true;
			for ( int i = 0 ; i < p->width ; i++ )
			{
				for ( int j = 0 ; j < p->height ; j++ )
				{
					p->ctrl[i][j].st[0] = fx * (float)i / ( p->width - 1 );
					p->ctrl[i][j].st[1] = 1 - fy * (float)j / ( p->height - 1 );
				}
			}
		}
	}
}

// NOTE TTimo stub!
void Patch_FitTexturing(){
	Patch_ResetTexturing( 1.0f, 1.0f );
}

void Patch_SetTextureInfo( texdef_t *pt ){
	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			if ( pt->rotate ) {
				Patch_RotateTexture( pb->pPatch, pt->rotate );
			}

			if ( pt->shift[0] || pt->shift[1] ) {
				Patch_ShiftTexture( pb->pPatch, pt->shift[0], pt->shift[1] );
			}

			if ( pt->scale[0] || pt->scale[1] ) {
				Patch_ScaleTexture( pb->pPatch, pt->scale[0], pt->scale[1], false );
			}

			patchMesh_t *p = pb->pPatch;
			p->contents = pt->contents;
			p->flags = pt->flags;
			p->value = pt->value;
		}
	}
}

bool OnlyPatchesSelected(){
	if ( g_ptrSelectedFaces.GetSize() > 0 || selected_brushes.next == &selected_brushes ) {
		return false;
	}
	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( !pb->patchBrush ) {
			return false;
		}
	}
	return true;
}

bool AnyPatchesSelected(){
	if ( g_ptrSelectedFaces.GetSize() > 0  || selected_brushes.next == &selected_brushes ) {
		return false;
	}
	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			return true;
		}
	}
	return false;
}

patchMesh_t* SinglePatchSelected(){
	if ( selected_brushes.next->patchBrush ) {
		return selected_brushes.next->pPatch;
	}
	return NULL;
}

void Patch_BendToggle(){
	if ( g_bPatchBendMode ) {
		g_bPatchBendMode = false;
		HideInfoDialog();
		g_pParentWnd->UpdatePatchToolbarButtons() ;
		return;
	}

	brush_t* b = selected_brushes.next;

	if ( !QE_SingleBrush( true ) || !b->patchBrush ) {
		Sys_Printf( "Patch_BendToggle: you must have a single patch selected\n" );
		return;
	}

	Patch_Save( b->pPatch );
	g_bPatchBendMode = true;
	g_nPatchBendState = BEND_SELECT_ROTATION;
	g_bPatchAxisOnRow = true;
	g_nPatchAxisIndex = 1;
	ShowInfoDialog( g_pBendStateMsg[BEND_SELECT_ROTATION] );
}

void Patch_BendHandleTAB(){
	if ( !g_bPatchBendMode ) {
		return;
	}

	brush_t* b = selected_brushes.next;
	if ( !QE_SingleBrush() || !b->patchBrush ) {
		Patch_BendToggle();
		Sys_Printf( "No patch to bend!" );
		return;
	}

	patchMesh_t *p = b->pPatch;

	bool bShift = Sys_ShiftDown();

	if ( g_nPatchBendState == BEND_SELECT_ROTATION ) {
		// only able to deal with odd numbered rows/cols
		g_nPatchAxisIndex += ( bShift ) ? -2 : 2;
		if ( g_bPatchAxisOnRow ) {
			if ( ( bShift ) ? g_nPatchAxisIndex <= 0 : g_nPatchAxisIndex >= p->height ) {
				g_bPatchAxisOnRow = false;
				g_nPatchAxisIndex = ( bShift ) ? p->width - 1 : 1;
			}
		}
		else
		{
			if ( ( bShift ) ? g_nPatchAxisIndex <= 0 : g_nPatchAxisIndex >= p->width ) {
				g_bPatchAxisOnRow = true;
				g_nPatchAxisIndex = ( bShift ) ? p->height - 1 : 1;
			}
		}
	}
	else
	if ( g_nPatchBendState == BEND_SELECT_ORIGIN ) {
		g_nBendOriginIndex += ( bShift ) ? -1 : 1;
		if ( g_bPatchAxisOnRow ) {
			if ( bShift ) {
				if ( g_nBendOriginIndex < 0 ) {
					g_nBendOriginIndex = p->width - 1;
				}
			}
			else
			{
				if ( g_nBendOriginIndex > p->width - 1 ) {
					g_nBendOriginIndex = 0;
				}
			}
			VectorCopy( p->ctrl[g_nBendOriginIndex][g_nPatchAxisIndex].xyz, g_vBendOrigin );
		}
		else
		{
			if ( bShift ) {
				if ( g_nBendOriginIndex < 0 ) {
					g_nBendOriginIndex = p->height - 1;
				}
			}
			else
			{
				if ( g_nBendOriginIndex > p->height - 1 ) {
					g_nBendOriginIndex = 0;
				}
			}
			VectorCopy( p->ctrl[g_nPatchAxisIndex][g_nBendOriginIndex].xyz, g_vBendOrigin );
		}
	}
	else
	if ( g_nPatchBendState == BEND_SELECT_EDGE ) {
		g_bPatchLowerEdge ^= 1;
	}
	Sys_UpdateWindows( W_ALL );
}

void Patch_BendHandleENTER(){
	if ( !g_bPatchBendMode ) {
		return;
	}

	if ( g_nPatchBendState  < BEND_BENDIT ) {
		g_nPatchBendState++;
		ShowInfoDialog( g_pBendStateMsg[g_nPatchBendState] );
		if ( g_nPatchBendState == BEND_SELECT_ORIGIN ) {
			g_vBendOrigin[0] = g_vBendOrigin[1] = g_vBendOrigin[2] = 0;
			g_nBendOriginIndex = 0;
			Patch_BendHandleTAB();
		}
		else
		if ( g_nPatchBendState == BEND_SELECT_EDGE ) {
			g_bPatchLowerEdge = true;
		}
		else
		if ( g_nPatchBendState == BEND_BENDIT ) {
			// basically we go into rotation mode, set the axis to the center of the
		}
	}
	else
	{
		// done
		Patch_BendToggle();
	}
	Sys_UpdateWindows( W_ALL );

}


void Patch_BendHandleESC(){
	if ( !g_bPatchBendMode ) {
		return;
	}
	Patch_BendToggle();
	brush_t* b = selected_brushes.next;
	if ( QE_SingleBrush() && b->patchBrush ) {
		Patch_Restore( b->pPatch );
	}
	Sys_UpdateWindows( W_ALL );
}

void Patch_SetBendRotateOrigin( patchMesh_t *p ){
#if 1
	int nType = g_pParentWnd->ActiveXY()->GetViewType();
	int nDim3 = ( nType == XY ) ? 2 : ( nType == YZ ) ? 0 : 1;

	g_vBendOrigin[nDim3] = 0;
	VectorCopy( g_vBendOrigin, g_pParentWnd->ActiveXY()->RotateOrigin() );
	return;
#else
	int nDim1 = ( g_pParentWnd->ActiveXY()->GetViewType() == YZ ) ? 1 : 0;
	int nDim2 = ( g_pParentWnd->ActiveXY()->GetViewType() == XY ) ? 1 : 2;

	float fxLo, fyLo, fxHi, fyHi;
	fxLo = fyLo = 9999;
	fxHi = fyHi = -9999;

	if ( g_bPatchAxisOnRow ) {
		for ( int i = 0; i < p->width; i++ )
		{
			if ( p->ctrl[i][g_nPatchAxisIndex].xyz[nDim1] < fxLo ) {
				fxLo = p->ctrl[i][g_nPatchAxisIndex].xyz[nDim1];
			}

			if ( p->ctrl[i][g_nPatchAxisIndex].xyz[nDim1] > fxHi ) {
				fxHi = p->ctrl[i][g_nPatchAxisIndex].xyz[nDim1];
			}

			if ( p->ctrl[i][g_nPatchAxisIndex].xyz[nDim2] < fyLo ) {
				fyLo = p->ctrl[i][g_nPatchAxisIndex].xyz[nDim2];
			}

			if ( p->ctrl[i][g_nPatchAxisIndex].xyz[nDim2] > fyHi ) {
				fyHi = p->ctrl[i][g_nPatchAxisIndex].xyz[nDim2];
			}
		}
	}
	else
	{
		for ( int i = 0; i < p->height; i++ )
		{
			if ( p->ctrl[g_nPatchAxisIndex][i].xyz[nDim1] < fxLo ) {
				fxLo = p->ctrl[g_nPatchAxisIndex][i].xyz[nDim1];
			}

			if ( p->ctrl[g_nPatchAxisIndex][i].xyz[nDim1] > fxHi ) {
				fxHi = p->ctrl[g_nPatchAxisIndex][i].xyz[nDim1];
			}

			if ( p->ctrl[g_nPatchAxisIndex][i].xyz[nDim2] < fyLo ) {
				fyLo = p->ctrl[g_nPatchAxisIndex][i].xyz[nDim2];
			}

			if ( p->ctrl[g_nPatchAxisIndex][i].xyz[nDim2] > fyHi ) {
				fyHi = p->ctrl[g_nPatchAxisIndex][i].xyz[nDim2];
			}
		}
	}

	g_pParentWnd->ActiveXY()->RotateOrigin()[0] = g_pParentWnd->ActiveXY()->RotateOrigin()[1] = g_pParentWnd->ActiveXY()->RotateOrigin()[2] = 0.0;
	g_pParentWnd->ActiveXY()->RotateOrigin()[nDim1] = ( fxLo + fxHi ) * 0.5;
	g_pParentWnd->ActiveXY()->RotateOrigin()[nDim2] = ( fyLo + fyHi ) * 0.5;
#endif
}

// also sets the rotational origin
void Patch_SelectBendAxis(){
	brush_t* b = selected_brushes.next;
	if ( !QE_SingleBrush() || !b->patchBrush ) {
		// should not ever happen
		Patch_BendToggle();
		return;
	}

	patchMesh_t *p = b->pPatch;
	if ( g_bPatchAxisOnRow ) {
		SelectRow( p, g_nPatchAxisIndex, false );
	}
	else
	{
		SelectColumn( p, g_nPatchAxisIndex, false );
	}

	//FIXME: this only needs to be set once...
	Patch_SetBendRotateOrigin( p );

}

void Patch_SelectBendNormal(){
	brush_t* b = selected_brushes.next;
	if ( !QE_SingleBrush() || !b->patchBrush ) {
		// should not ever happen
		Patch_BendToggle();
		return;
	}

	patchMesh_t *p = b->pPatch;

	g_qeglobals.d_num_move_points = 0;
	if ( g_bPatchAxisOnRow ) {
		if ( g_bPatchLowerEdge ) {
			for ( int j = 0; j < g_nPatchAxisIndex; j++ )
				SelectRow( p, j, true );
		}
		else
		{
			for ( int j = p->height - 1; j > g_nPatchAxisIndex; j-- )
				SelectRow( p, j, true );
		}
	}
	else
	{
		if ( g_bPatchLowerEdge ) {
			for ( int j = 0; j < g_nPatchAxisIndex; j++ )
				SelectColumn( p, j, true );
		}
		else
		{
			for ( int j = p->width - 1; j > g_nPatchAxisIndex; j-- )
				SelectColumn( p, j, true );
		}
	}
	Patch_SetBendRotateOrigin( p );
}



/*
   void Patch_InsDelToggle()
   {
   if (g_bPatchInsertMode)
   {
    g_bPatchInsertMode = false;
    HideInfoDialog();
    g_pParentWnd->UpdatePatchToolbarButtons() ;
    return;
   }

    brush_t* b = selected_brushes.next;

   if (!QE_SingleBrush(true) || !b->patchBrush)
   {
    Sys_Printf("Patch_InsDelToggle: you must have a single patch selected\n");
        return;
   }

   Patch_Save(b->pPatch);
    g_bPatchInsertMode = true;
   g_nPatchInsertState = INSERT_SELECT_EDGE;
   g_bPatchAxisOnRow = true;
   g_nPatchAxisIndex = 0;
   ShowInfoDialog(g_pInsertStateMsg[INSERT_SELECT_EDGE]);

   }

   void Patch_InsDelESC()
   {
   if (!g_bPatchInsertMode)
   {
    return;
   }
   Patch_InsDelToggle();
   Sys_UpdateWindows(W_ALL);
   }


   void Patch_InsDelHandleENTER()
   {
   }

   void Patch_InsDelHandleTAB()
   {
   if (!g_bPatchInsertMode)
   {
    Patch_InsDelToggle();
    return;
   }

    brush_t* b = selected_brushes.next;
   if (!QE_SingleBrush() || !b->patchBrush)
   {
    Patch_BendToggle();
    Sys_Printf("No patch to bend!");
        return;
   }

   patchMesh_t *p = b->pPatch;

   // only able to deal with odd numbered rows/cols
   g_nPatchAxisIndex += 2;
   if (g_bPatchAxisOnRow)
   {
    if (g_nPatchAxisIndex >= p->height-1)
    {
      g_bPatchAxisOnRow = false;
      g_nPatchAxisIndex = 0;
    }
   }
   else
   {
    if (g_nPatchAxisIndex >= p->width-1)
    {
      g_bPatchAxisOnRow = true;
      g_nPatchAxisIndex = 0;
    }
   }
   Sys_UpdateWindows(W_ALL);
   }
 */


void _Write1DMatrix( FILE *f, int x, float *m ) {
	int i;

	fprintf( f, "( " );
	for ( i = 0 ; i < x ; i++ ) {
		if ( m[i] == (int)m[i] ) {
			fprintf( f, "%i ", (int)m[i] );
		}
		else {
			fprintf( f, "%f ", m[i] );
		}
	}
	fprintf( f, ")" );
}

void _Write2DMatrix( FILE *f, int y, int x, float *m ) {
	int i;

	fprintf( f, "( " );
	for ( i = 0 ; i < y ; i++ ) {
		_Write1DMatrix( f, x, m + i * x );
		fprintf( f, " " );
	}
	fprintf( f, ")\n" );
}


void _Write3DMatrix( FILE *f, int z, int y, int x, float *m ) {
	int i;

	fprintf( f, "(\n" );
	for ( i = 0 ; i < z ; i++ ) {
		_Write2DMatrix( f, y, x, m + i * ( x * MAX_PATCH_HEIGHT ) );
	}
	fprintf( f, ")\n" );
}

void _Write1DMatrix( MemStream *f, int x, float *m ) {
	int i;

	MemFile_fprintf( f, "( " );
	for ( i = 0 ; i < x ; i++ ) {
		if ( m[i] == (int)m[i] ) {
			MemFile_fprintf( f, "%i ", (int)m[i] );
		}
		else {
			MemFile_fprintf( f, "%f ", m[i] );
		}
	}
	MemFile_fprintf( f, ")" );
}

void _Write2DMatrix( MemStream *f, int y, int x, float *m ) {
	int i;

	MemFile_fprintf( f, "( " );
	for ( i = 0 ; i < y ; i++ ) {
		_Write1DMatrix( f, x, m + i * x );
		MemFile_fprintf( f, " " );
	}
	MemFile_fprintf( f, ")\n" );
}


void _Write3DMatrix( MemStream *f, int z, int y, int x, float *m ) {
	int i;

	MemFile_fprintf( f, "(\n" );
	for ( i = 0 ; i < z ; i++ ) {
		_Write2DMatrix( f, y, x, m + i * ( x * MAX_PATCH_HEIGHT ) );
	}
	MemFile_fprintf( f, ")\n" );
}

// NOTE: why the hell is this called Naturalize?
// we dispatch either to Patch+Naturalize or Patch_CapTexture..
void Patch_NaturalizeSelected( bool bCap ){
	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			if ( bCap ) {
				Patch_CapTexture( pb->pPatch ); //, bCycleCap);
			}
			else{
				Patch_Naturalize( pb->pPatch );
			}
		}
	}
}

// go through the selected patches and call Patch_CapTexture
// deal with cycling
void Patch_CycleCapSelected(){
	// compute the g_vCycleCapNormal according to g_nCycleCapIndex
	VectorClear( g_vCycleCapNormal );
	g_vCycleCapNormal[g_nCycleCapIndex] = 1.0f; // cf VIEWTYPE defintion: enum VIEWTYPE {YZ, XZ, XY};
	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			Patch_CapTexture( pb->pPatch, true );
		}
	}
	switch ( g_nCycleCapIndex )
	{
	case YZ:
		g_nCycleCapIndex = XZ;
		break;
	case XZ:
		g_nCycleCapIndex = XY;
		break;
	case XY:
		g_nCycleCapIndex = YZ;
		break;
	}
}

bool within( vec3_t vTest, vec3_t vTL, vec3_t vBR ){
	int nDim1 = ( g_pParentWnd->ActiveXY()->GetViewType() == YZ ) ? 1 : 0;
	int nDim2 = ( g_pParentWnd->ActiveXY()->GetViewType() == XY ) ? 1 : 2;
	if ( ( vTest[nDim1] > vTL[nDim1] && vTest[nDim1] < vBR[nDim1] ) ||
		 ( vTest[nDim1] < vTL[nDim1] && vTest[nDim1] > vBR[nDim1] ) ) {
		if ( ( vTest[nDim2] > vTL[nDim2] && vTest[nDim2] < vBR[nDim2] ) ||
			 ( vTest[nDim2] < vTL[nDim2] && vTest[nDim2] > vBR[nDim2] ) ) {
			return true;
		}
	}
	return false;
}


void Patch_SelectAreaPoints( bool bMulti ){
	if ( !bMulti ) {
		g_qeglobals.d_num_move_points = 0;
	}

	if ( g_nPatchClickedView == W_CAMERA ) {
		// Clip against a pyramid
		// Create our 5 normals (that are pointing to the inside)
		camera_t *m_pCamera = g_pParentWnd->GetCamWnd()->Camera();
		vec3_t norm[5];
		float r[2], u[2], hh, hw;
		int idx;
		vec_t corners[2][2];
		vec3_t ray[4];
		vec3_t check;

		VectorCopy( m_pCamera->vpn, norm[0] ); // only points in front of the camera

		// get our rectangle
		corners[0][0] = MIN( g_qeglobals.d_vAreaTL[0], g_qeglobals.d_vAreaBR[0] );
		corners[0][1] = MAX( g_qeglobals.d_vAreaTL[1], g_qeglobals.d_vAreaBR[1] );
		corners[1][0] = MAX( g_qeglobals.d_vAreaTL[0], g_qeglobals.d_vAreaBR[0] );
		corners[1][1] = MIN( g_qeglobals.d_vAreaTL[1], g_qeglobals.d_vAreaBR[1] );

		// calculate our four ray vectors
		hh = m_pCamera->height / 2;
		hw = m_pCamera->width / 2;
		u[0] = (float)( corners[0][1] - hh ) / ( hw );
		r[0] = (float)( corners[0][0] - hw ) / ( hw );
		u[1] = (float)( corners[1][1] - hh ) / ( hw );
		r[1] = (float)( corners[1][0] - hw ) / ( hw );

		for ( idx = 0 ; idx < 3; idx++ )
			ray[0][idx] = m_pCamera->vpn[idx] + m_pCamera->vright[idx] * r[0] + m_pCamera->vup[idx] * u[0];
		for ( idx = 0 ; idx < 3; idx++ )
			ray[1][idx] = m_pCamera->vpn[idx] + m_pCamera->vright[idx] * r[1] + m_pCamera->vup[idx] * u[0];
		for ( idx = 0 ; idx < 3; idx++ )
			ray[2][idx] = m_pCamera->vpn[idx] + m_pCamera->vright[idx] * r[1] + m_pCamera->vup[idx] * u[1];
		for ( idx = 0 ; idx < 3; idx++ )
			ray[3][idx] = m_pCamera->vpn[idx] + m_pCamera->vright[idx] * r[0] + m_pCamera->vup[idx] * u[1];

		// Create our four other directions from these
		CrossProduct( ray[0], ray[1], norm[1] ); VectorNormalize( norm[1], norm[1] );
		CrossProduct( ray[1], ray[2], norm[2] ); VectorNormalize( norm[2], norm[2] );
		CrossProduct( ray[2], ray[3], norm[3] ); VectorNormalize( norm[3], norm[3] );
		CrossProduct( ray[3], ray[0], norm[4] ); VectorNormalize( norm[4], norm[4] );

		// 3D clipping
		for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
		{
			if ( pb->patchBrush ) {
				patchMesh_t *p = pb->pPatch;
				for ( int i = 0; i < p->width; i++ )
				{
					for ( int j = 0; j < p->height; j++ )
					{
						VectorSubtract( m_pCamera->origin, p->ctrl[i][j].xyz, check );
						VectorNormalize( check, check );
						for ( idx = 0 ; idx < 5; idx++ )
						{
							if ( DotProduct( check, norm[idx] ) >= 0 ) {
								break;
							}
						}
						if ( idx == 5 ) { // all test were good
							if ( bMulti && PointInMoveList( p->ctrl[i][j].xyz ) != -1 ) {
								RemovePointFromMoveList( p->ctrl[i][j].xyz );
							}
							else{
								g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = p->ctrl[i][j].xyz;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		// Simple 2D clipping
		for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
		{
			if ( pb->patchBrush ) {
				patchMesh_t *p = pb->pPatch;
				for ( int i = 0; i < p->width; i++ )
				{
					for ( int j = 0; j < p->height; j++ )
					{
						if ( within( p->ctrl[i][j].xyz, g_qeglobals.d_vAreaTL, g_qeglobals.d_vAreaBR ) ) {
							if ( bMulti && PointInMoveList( p->ctrl[i][j].xyz ) != -1 ) {
								RemovePointFromMoveList( p->ctrl[i][j].xyz );
							}
							else{
								g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = p->ctrl[i][j].xyz;
							}
						}
					}
				}
			}
		}
	}

	g_nPatchClickedView = -1;
}

// TTimo: return the shader name for a patch
const char* Patch_GetTextureName(){
	brush_t* b = selected_brushes.next;
	if ( b->patchBrush ) {
		patchMesh_t *p = b->pPatch;
		return p->pShader->getName();
	}
	return "";
}

patchMesh_t* Patch_Duplicate( patchMesh_t *pFrom ){
	patchMesh_t* p = MakeNewPatch();
	memcpy( p, pFrom, sizeof( patchMesh_t ) );

	// spog - initialise patch LOD pointers (again)
	Patch_InitialiseLODPointers( p );
	p->drawLists = NULL;

	p->bSelected = false;
	p->bDirty = true;
	p->bOverlay = false;
	p->nListID = -1;
	AddBrushForPatch( p );

	return p;
}


void Patch_Thicken( int nAmount, bool bSeam, qboolean bGroupResult ){
	int i, j, h, w;
	brush_t *b;
	patchMesh_t *pSeam;
	vec3_t vMin, vMax;
	CPtrArray brushes;

	nAmount = -nAmount;


	if ( !QE_SingleBrush() ) {
		Sys_Printf( "Cannot thicken multiple patches. Please select a single patch.\n" );
		return;
	}

	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			patchMesh_t *p = pb->pPatch;
			Patch_MeshNormals( p );
			patchMesh_t *pNew = Patch_Duplicate( p );
			for ( i = 0; i < p->width; i++ )
			{
				for ( j = 0; j < p->height; j++ )
				{
					VectorMA( p->ctrl[i][j].xyz, nAmount, p->ctrl[i][j].normal, pNew->ctrl[i][j].xyz );
				}
			}

			Patch_Rebuild( pNew );
			pNew->type |= PATCH_THICK;
			brushes.Add( pNew->pSymbiot );

			if ( bSeam ) {

				// FIXME: this should detect if any edges of the patch are closed and act appropriately
				//
				if ( !( p->type & PATCH_CYLINDER ) ) {
					b = Patch_GenericMesh( 3, p->height, 2, false, true );
					pSeam = b->pPatch;
					pSeam->type |= PATCH_SEAM;
					for ( i = 0; i < p->height; i++ )
					{
						VectorCopy( p->ctrl[0][i].xyz, pSeam->ctrl[0][i].xyz );
						VectorCopy( pNew->ctrl[0][i].xyz, pSeam->ctrl[2][i].xyz );
						VectorAdd( pSeam->ctrl[0][i].xyz, pSeam->ctrl[2][i].xyz, pSeam->ctrl[1][i].xyz );
						VectorScale( pSeam->ctrl[1][i].xyz, 0.5, pSeam->ctrl[1][i].xyz );
					}


					Patch_CalcBounds( pSeam, vMin, vMax );
					Brush_RebuildBrush( pSeam->pSymbiot, vMin, vMax );
					//--Patch_CapTexture(pSeam);
					Patch_Naturalize( pSeam );
					patchInvert( pSeam );
					brushes.Add( b );

					w = p->width - 1;
					b = Patch_GenericMesh( 3, p->height, 2, false, true );
					pSeam = b->pPatch;
					pSeam->type |= PATCH_SEAM;
					for ( i = 0; i < p->height; i++ )
					{
						VectorCopy( p->ctrl[w][i].xyz, pSeam->ctrl[0][i].xyz );
						VectorCopy( pNew->ctrl[w][i].xyz, pSeam->ctrl[2][i].xyz );
						VectorAdd( pSeam->ctrl[0][i].xyz, pSeam->ctrl[2][i].xyz, pSeam->ctrl[1][i].xyz );
						VectorScale( pSeam->ctrl[1][i].xyz, 0.5, pSeam->ctrl[1][i].xyz );
					}
					Patch_CalcBounds( pSeam, vMin, vMax );
					Brush_RebuildBrush( pSeam->pSymbiot, vMin, vMax );
					//--Patch_CapTexture(pSeam);
					Patch_Naturalize( pSeam );
					brushes.Add( b );
				}

				//--{
				// otherwise we will add one per end
				b = Patch_GenericMesh( p->width, 3, 2, false, true );
				pSeam = b->pPatch;
				pSeam->type |= PATCH_SEAM;
				for ( i = 0; i < p->width; i++ )
				{
					VectorCopy( p->ctrl[i][0].xyz, pSeam->ctrl[i][0].xyz );
					VectorCopy( pNew->ctrl[i][0].xyz, pSeam->ctrl[i][2].xyz );
					VectorAdd( pSeam->ctrl[i][0].xyz, pSeam->ctrl[i][2].xyz, pSeam->ctrl[i][1].xyz );
					VectorScale( pSeam->ctrl[i][1].xyz, 0.5, pSeam->ctrl[i][1].xyz );
				}


				Patch_CalcBounds( pSeam, vMin, vMax );
				Brush_RebuildBrush( pSeam->pSymbiot, vMin, vMax );
				//--Patch_CapTexture(pSeam);
				Patch_Naturalize( pSeam );
				patchInvert( pSeam );
				brushes.Add( b );

				h = p->height - 1;
				b = Patch_GenericMesh( p->width, 3, 2, false, true );
				pSeam = b->pPatch;
				pSeam->type |= PATCH_SEAM;
				for ( i = 0; i < p->width; i++ )
				{
					VectorCopy( p->ctrl[i][h].xyz, pSeam->ctrl[i][0].xyz );
					VectorCopy( pNew->ctrl[i][h].xyz, pSeam->ctrl[i][2].xyz );
					VectorAdd( pSeam->ctrl[i][0].xyz, pSeam->ctrl[i][2].xyz, pSeam->ctrl[i][1].xyz );
					VectorScale( pSeam->ctrl[i][1].xyz, 0.5, pSeam->ctrl[i][1].xyz );
				}
				Patch_CalcBounds( pSeam, vMin, vMax );
				Brush_RebuildBrush( pSeam->pSymbiot, vMin, vMax );
				//--Patch_CapTexture(pSeam);
				Patch_Naturalize( pSeam );
				brushes.Add( b );

			}
			patchInvert( pNew );
		}
	}

	for ( i = 0; i < brushes.GetSize(); i++ )
	{
		Select_Brush( reinterpret_cast<brush_t*>( brushes.GetAt( i ) ) );
	}

	if ( bGroupResult ) {
		entity_t *e = Entity_Alloc();
		SetKeyValue( e, "classname", "func_group" );
		SetKeyValue( e, "type", "patchThick" );
		Select_GroupEntity( e );
		Entity_AddToList( e, &entities );
	}

	UpdatePatchInspector();
}


/*
   lets get another list together as far as necessities..

   *snapping stuff to the grid (i will only snap movements by the mouse to the grid.. snapping the rotational bend stuff will fubar everything)

   capping bevels/endcaps

   hot keys

   texture fix for caps

   clear clipboard

   *region fix

   *surface dialog

 */

void Patch_SetOverlays(){
	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			pb->pPatch->bOverlay = true;
		}
	}
}



void Patch_ClearOverlays(){
	brush_t *pb;
	for ( pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			pb->pPatch->bOverlay = false;
		}
	}

	for ( pb = active_brushes.next ; pb != &active_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			pb->pPatch->bOverlay = false;
		}
	}

}

// FIXME: spog - er, someone forgot to finish their patch point freezing feature?
// freezes selected vertices
void Patch_Freeze(){
	brush_t *pb;
	for ( pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			pb->pPatch->bOverlay = false;
		}
	}

	for ( pb = active_brushes.next ; pb != &active_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			pb->pPatch->bOverlay = false;
		}
	}

}

void Patch_UnFreeze( bool bAll ){
}

void Patch_Transpose(){
	int i, j, w;
	drawVert_t dv;
	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			patchMesh_t *p = pb->pPatch;

			if ( p->width > p->height ) {
				for ( i = 0 ; i < p->height ; i++ )
				{
					for ( j = i + 1 ; j < p->width ; j++ )
					{
						if ( j < p->height ) {
							// swap the value
							memcpy( &dv,&p->ctrl[j][i],sizeof( drawVert_t ) );
							memcpy( &p->ctrl[j][i],&p->ctrl[i][j], sizeof( drawVert_t ) );
							memcpy( &p->ctrl[i][j],&dv, sizeof( drawVert_t ) );
						}
						else
						{
							// just copy
							memcpy( &p->ctrl[i][j],&p->ctrl[j][i], sizeof( drawVert_t ) );
						}
					}
				}
			}
			else
			{
				for ( i = 0 ; i < p->width ; i++ )
				{
					for ( j = i + 1 ; j < p->height ; j++ )
					{
						if ( j < p->width ) {
							// swap the value
							memcpy( &dv,&p->ctrl[i][j], sizeof( drawVert_t ) );
							memcpy( &p->ctrl[i][j],&p->ctrl[j][i], sizeof( drawVert_t ) );
							memcpy( &p->ctrl[j][i],&dv, sizeof( drawVert_t ) );
						}
						else
						{
							// just copy
							memcpy( &p->ctrl[j][i],&p->ctrl[i][j], sizeof( drawVert_t ) );
						}
					}
				}
			}

			w = p->width;
			p->width = p->height;
			p->height = w;
			patchInvert( p );
			Patch_Rebuild( p );
		}
	}
}



void Patch_SnapToGrid( patchMesh_t *p ){
	int i,j,k;

	// if patch points selected, snap only selected points
	if ( g_qeglobals.d_select_mode == sel_curvepoint && g_qeglobals.d_num_move_points != 0 ) {
		for ( i = 0; i < g_qeglobals.d_num_move_points; i++ )
			for ( j = 0; j < 3; j++ )
				g_qeglobals.d_move_points[i][j] = floor( g_qeglobals.d_move_points[i][j] / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;
	}

	// else snap all patch points
	else{
		for ( i = 0; i < p->width; i++ )
			for ( j = 0; j < p->height; j++ )
				for ( k = 0; k < 3; k++ )
					p->ctrl[i][j].xyz[k] = floor( p->ctrl[i][j].xyz[k] / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;
	}

	vec3_t vMin, vMax;
	Patch_CalcBounds( p, vMin, vMax );
	Brush_RebuildBrush( p->pSymbiot, vMin, vMax );
}


void Patch_FindReplaceTexture( brush_t *pb, const char *pFind, const char *pReplace, bool bForce ){
	if ( pb->patchBrush ) {
		patchMesh_t *p = pb->pPatch;
		if ( bForce || strcmpi( p->pShader->getName(), pFind ) == 0 ) {
			p->pShader->DecRef();
			p->pShader = QERApp_Shader_ForName( pReplace );
			p->d_texture = p->pShader->getTexture();
		}
	}
}

/* uncomment if necessary, currently not used
   void Patch_FromTriangle(vec5_t vx, vec5_t vy, vec5_t vz)
   {
   patchMesh_t* p = MakeNewPatch();
   p->pShader = g_qeglobals.d_texturewin.pShader;
   p->d_texture = g_qeglobals.d_texturewin.pShader->getTexture();
   p->width = 3;
   p->height = 3;
   p->type = PATCH_TRIANGLE;

   // 0 0 goes to x
   // 0 1 goes to x
   // 0 2 goes to x

   // 1 0 goes to mid of x and z
   // 1 1 goes to mid of x y and z
   // 1 2 goes to mid of x and y

   // 2 0 goes to z
   // 2 1 goes to mid of y and z
   // 2 2 goes to y

   vec5_t vMidXZ;
   vec5_t vMidXY;
   vec5_t vMidYZ;
   int j;

   for (j = 0; j < 3; j++)
   {
    _Vector5Add(vx, vz, vMidXZ);
    _Vector5Scale(vMidXZ, 0.5, vMidXZ);
    //vMidXZ[j] = vx[j] + abs((vx[j] - vz[j]) * 0.5);
   }

   for (j = 0; j < 3; j++)
   {
    _Vector5Add(vx, vy, vMidXY);
    _Vector5Scale(vMidXY, 0.5, vMidXY);
    //vMidXY[j] = vx[j] + abs((vx[j] - vy[j]) * 0.5);
   }

   for (j = 0; j < 3; j++)
   {
    _Vector5Add(vy, vz, vMidYZ);
    _Vector5Scale(vMidYZ, 0.5, vMidYZ);
    //vMidYZ[j] = vy[j] + abs((vy[j] - vz[j]) * 0.5);
   }

   _Vector53Copy(vx, p->ctrl[0][0].xyz);
   _Vector53Copy(vx, p->ctrl[0][1].xyz);
   _Vector53Copy(vx, p->ctrl[0][2].xyz);
   p->ctrl[0][0].st[0] = vx[3];
   p->ctrl[0][0].st[1] = vx[4];
   p->ctrl[0][1].st[0] = vx[3];
   p->ctrl[0][1].st[1] = vx[4];
   p->ctrl[0][2].st[0] = vx[3];
   p->ctrl[0][2].st[1] = vx[4];

   _Vector53Copy(vMidXY, p->ctrl[1][0].xyz);
   _Vector53Copy(vx, p->ctrl[1][1].xyz);
   _Vector53Copy(vMidXZ, p->ctrl[1][2].xyz);
   p->ctrl[1][0].st[0] = vMidXY[3];
   p->ctrl[1][0].st[1] = vMidXY[4];
   p->ctrl[1][1].st[0] = vx[3];
   p->ctrl[1][1].st[1] = vx[4];
   p->ctrl[1][2].st[0] = vMidXZ[3];
   p->ctrl[1][2].st[1] = vMidXZ[4];

   _Vector53Copy(vy, p->ctrl[2][0].xyz);
   _Vector53Copy(vMidYZ, p->ctrl[2][1].xyz);
   _Vector53Copy(vz, p->ctrl[2][2].xyz);
   p->ctrl[2][0].st[0] = vy[3];
   p->ctrl[2][0].st[1] = vy[4];
   p->ctrl[2][1].st[0] = vMidYZ[3];
   p->ctrl[2][1].st[1] = vMidYZ[4];
   p->ctrl[2][2].st[0] = vz[3];
   p->ctrl[2][2].st[1] = vz[4];


   //Patch_Naturalize(p);

   //  brush_t *b =
   AddBrushForPatch(p);

   }
 */

#ifdef ENABLE_GROUPS
/*
   ==============
   Patch_SetEpair
   sets an epair for the given patch
   ==============
 */
void Patch_SetEpair( patchMesh_t *p, const char *pKey, const char *pValue ){
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		SetKeyValue( p->epairs, pKey, pValue );
	}
}

/*
   =================
   Patch_GetKeyValue
   =================
 */
const char* Patch_GetKeyValue( patchMesh_t *p, const char *pKey ){
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		return ValueForKey( p->epairs, pKey );
	}
	return "";
}
#endif


//Real nitpicky, but could you make CTRL-S save the current map with the current name? (ie: File/Save)
/*
   Feature addition.
   When reading in textures, please check for the presence of a file called "textures.link" or something, which contains one line such as;

   g:\quake3\baseq3\textures\common

   So that, when I'm reading in, lets say, my \eerie directory, it goes through and adds my textures to the palette, along with everything in common.

   Don't forget to add "Finer texture alignment" to the list. I'd like to be able to move in 0.1 increments using the Shift-Arrow Keys.

   No. Sometimes textures are drawn the wrong way on patches. We'd like the ability to flip a texture. Like the way X/Y scale -1 used to worked.

   1) Easier way of deleting rows, columns
   2) Fine tuning of textures on patches (X/Y shifts other than with the surface dialog)
   2) Patch matrix transposition

   1) Actually, bump texture flipping on patches to the top of the list of things to do.
   2) When you select a patch, and hit S, it should read in the selected patch texture. Should not work if you multiselect patches and hit S
   3) Brandon has a wierd anomoly. He fine-tunes a patch with caps. It looks fine when the patch is selected, but as soon as he escapes out, it reverts to it's pre-tuned state. When he selects the patch again, it looks tuned


   *1) Flipping textures on patches
   *2) When you select a patch, and hit S, it should read in the selected patch texture. Should not work if you multiselect patches and hit S
   3) Easier way of deleting rows columns
   *4) Thick Curves
   5) Patch matrix transposition
   6) Inverted cylinder capping
   *7) bugs
   *8) curve speed

   Have a new feature request. "Compute Bounding Box" for mapobjects (md3 files). This would be used for misc_mapobject (essentially, drop in 3DS Max models into our maps)

   Ok, Feature Request. Load and draw MD3's in the Camera view with proper bounding boxes. This should be off misc_model

   Feature Addition: View/Hide Hint Brushes -- This should be a specific case.
 */
