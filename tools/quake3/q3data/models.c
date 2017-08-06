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

#include <assert.h>
#include "q3data.h"

//=================================================================

static void OrderSurfaces( void );
static void LoadBase( const char *filename );
static int  LoadModelFile( const char *filename, polyset_t **ppsets, int *pnumpolysets );

#define MAX_SURFACE_TRIS    ( SHADER_MAX_INDEXES / 3 )
#define MAX_SURFACE_VERTS   SHADER_MAX_VERTEXES

#define MD3_TYPE_UNKNOWN 0
#define MD3_TYPE_BASE3DS 1
#define MD3_TYPE_SPRITE  2
#define MD3_TYPE_ASE     3

#define MAX_ANIM_FRAMES     512
#define MAX_ANIM_SURFACES   32

typedef struct
{
	polyset_t *frames;
	int numFrames;
} SurfaceAnimation_t;

typedef struct
{
	polyset_t *surfaces[MAX_ANIM_SURFACES];
	int numSurfaces;
} ObjectAnimationFrame_t;

typedef struct {
	vec3_t xyz;
	vec3_t normal;
	vec3_t color;
	float st[2];
	int index;
} baseVertex_t;

typedef struct {
	baseVertex_t v[3];
} baseTriangle_t;

//================================================================

typedef struct
{
	md3Surface_t header;
	md3Shader_t shaders[MD3_MAX_SHADERS];
	// all verts (xyz_normal)
	float   *verts[MD3_MAX_FRAMES];

	baseTriangle_t baseTriangles[MD3_MAX_TRIANGLES];

	// the triangles will be sorted so that they form long generalized tristrips
	int orderedTriangles[MD3_MAX_TRIANGLES][3];
	int lodTriangles[MD3_MAX_TRIANGLES][3];
	baseVertex_t baseVertexes[MD3_MAX_VERTS];

} md3SurfaceData_t;

typedef struct
{
	int skinwidth, skinheight;

	md3SurfaceData_t surfData[MD3_MAX_SURFACES];

	md3Tag_t tags[MD3_MAX_FRAMES][MD3_MAX_TAGS];
	md3Frame_t frames[MD3_MAX_FRAMES];

	md3Header_t model;
	float scale_up;                 // set by $scale
	vec3_t adjust;                  // set by $origin
	vec3_t aseAdjust;
	int fixedwidth, fixedheight;            // set by $skinsize

	int maxSurfaceTris;

	int lowerSkipFrameStart, lowerSkipFrameEnd;
	int maxUpperFrames;
	int maxHeadFrames;
	int currentLod;
	float lodBias;

	int type;               // MD3_TYPE_BASE, MD3_TYPE_OLDBASE, MD3_TYPE_ASE, or MD3_TYPE_SPRITE

} q3data;

q3data g_data;

// the command list holds counts, the count * 3 xyz, st, normal indexes
// that are valid for every frame
char g_cddir[1024];
char g_modelname[1024];

//==============================================================

/*
   ===============
   ClearModel
   ===============
 */
void ClearModel( void ){
	int i;

	g_data.type = MD3_TYPE_UNKNOWN;

	for ( i = 0; i < MD3_MAX_SURFACES; i++ )
	{
		memset( &g_data.surfData[i].header, 0, sizeof( g_data.surfData[i].header ) );
		memset( &g_data.surfData[i].shaders, 0, sizeof( g_data.surfData[i].shaders ) );
		memset( &g_data.surfData[i].verts, 0, sizeof( g_data.surfData[i].verts ) );
	}

	memset( g_data.tags, 0, sizeof( g_data.tags ) );

	for ( i = 0; i < g_data.model.numSurfaces; i++ )
	{
		int j;

		for ( j = 0; j < g_data.surfData[i].header.numShaders; j++ )
		{
			memset( &g_data.surfData[i].shaders[j], 0, sizeof( g_data.surfData[i].shaders[j] ) );
		}
	}
	memset( &g_data.model, 0, sizeof( g_data.model ) );
	memset( g_cddir, 0, sizeof( g_cddir ) );

	g_modelname[0] = 0;
	g_data.scale_up = 1.0;
	memset( &g_data.model, 0, sizeof( g_data.model ) );
	VectorCopy( vec3_origin, g_data.adjust );
	g_data.fixedwidth = g_data.fixedheight = 0;
	g_skipmodel = qfalse;
}

/*
** void WriteModelSurface( FILE *modelouthandle, md3SurfaceData_t *pSurfData )
**
** This routine assumes that the file position has been adjusted
** properly prior to entry to point at the beginning of the surface.
**
** Since surface header information is completely relative, we can't
** just randomly seek to an arbitrary surface location right now.  Is
** this something we should add?
*/
void WriteModelSurface( FILE *modelouthandle, md3SurfaceData_t *pSurfData ){
	md3Surface_t    *pSurf = &pSurfData->header;
	md3Shader_t     *pShader = pSurfData->shaders;
	baseVertex_t    *pBaseVertex = pSurfData->baseVertexes;
	float           **verts = pSurfData->verts;

	short xyznormals[MD3_MAX_VERTS][4];

	float base_st[MD3_MAX_VERTS][2];
	md3Surface_t surftemp;

	int f, i, j, k;

	if ( strstr( pSurf->name, "tag_" ) == pSurf->name ) {
		return;
	}

	//
	// write out the header
	//
	surftemp = *pSurf;
	surftemp.ident = LittleLong( MD3_IDENT );
	surftemp.flags = LittleLong( pSurf->flags );
	surftemp.numFrames = LittleLong( pSurf->numFrames );
	surftemp.numShaders = LittleLong( pSurf->numShaders );

	surftemp.ofsShaders = LittleLong( pSurf->ofsShaders );

	surftemp.ofsTriangles = LittleLong( pSurf->ofsTriangles );
	surftemp.numTriangles = LittleLong( pSurf->numTriangles );

	surftemp.ofsSt = LittleLong( pSurf->ofsSt );
	surftemp.ofsXyzNormals = LittleLong( pSurf->ofsXyzNormals );
	surftemp.ofsEnd = LittleLong( pSurf->ofsEnd );

	SafeWrite( modelouthandle, &surftemp, sizeof( surftemp ) );

	if ( g_verbose ) {
		printf( "surface '%s'\n", pSurf->name );
		printf( "...num shaders: %d\n", pSurf->numShaders );
	}

	//
	// write out shaders
	//
	for ( i = 0; i < pSurf->numShaders; i++ )
	{
		md3Shader_t shadertemp;

		if ( g_verbose ) {
			printf( "......'%s'\n", pShader[i].name );
		}

		shadertemp = pShader[i];
		shadertemp.shaderIndex = LittleLong( shadertemp.shaderIndex );
		SafeWrite( modelouthandle, &shadertemp, sizeof( shadertemp ) );
	}

	//
	// write out the triangles
	//
	for ( i = 0 ; i < pSurf->numTriangles ; i++ )
	{
		for ( j = 0 ; j < 3 ; j++ )
		{
			int ivalue = LittleLong( pSurfData->orderedTriangles[i][j] );
			pSurfData->orderedTriangles[i][j] = ivalue;
		}
	}

	SafeWrite( modelouthandle, pSurfData->orderedTriangles, pSurf->numTriangles * sizeof( g_data.surfData[0].orderedTriangles[0] ) );

	if ( g_verbose ) {
		printf( "\n...num verts: %d\n", pSurf->numVerts );
		printf( "...TEX COORDINATES\n" );
	}

	//
	// write out the texture coordinates
	//
	for ( i = 0; i < pSurf->numVerts ; i++ ) {
		base_st[i][0] = LittleFloat( pBaseVertex[i].st[0] );
		base_st[i][1] = LittleFloat( pBaseVertex[i].st[1] );
		if ( g_verbose ) {
			printf( "......%d: %f,%f\n", i, base_st[i][0], base_st[i][1] );
		}
	}
	SafeWrite( modelouthandle, base_st, pSurf->numVerts * sizeof( base_st[0] ) );

	//
	// write the xyz_normal
	//
	if ( g_verbose ) {
		printf( "...XYZNORMALS\n" );
	}
	for ( f = 0; f < g_data.model.numFrames; f++ )
	{
		for ( j = 0 ; j < pSurf->numVerts; j++ )
		{
			short value;

			for ( k = 0 ; k < 3 ; k++ )
			{
				value = ( short ) ( verts[f][j * 6 + k] / MD3_XYZ_SCALE );
				xyznormals[j][k] = LittleShort( value );
			}
			NormalToLatLong( &verts[f][j * 6 + 3], (byte *)&xyznormals[j][3] );
		}
		SafeWrite( modelouthandle, xyznormals, pSurf->numVerts * sizeof( short ) * 4 );
	}
}

/*
** void WriteModelFile( FILE *modelouthandle )
**
** CHUNK			SIZE
** header			sizeof( md3Header_t )
** frames			sizeof( md3Frame_t ) * numFrames
** tags				sizeof( md3Tag_t ) * numFrames * numTags
** surfaces			surfaceSum
*/
void WriteModelFile( FILE *modelouthandle ){
	int f;
	int i, j;
	md3Header_t modeltemp;
	long surfaceSum = 0;
	int numRealSurfaces = 0;
	int numFrames = g_data.model.numFrames;

	// compute offsets for all surfaces, sum their total size
	for ( i = 0; i < g_data.model.numSurfaces; i++ )
	{
		if ( strstr( g_data.surfData[i].header.name, "tag_" ) != g_data.surfData[i].header.name ) {
			md3Surface_t *psurf = &g_data.surfData[i].header;

			if ( psurf->numTriangles == 0 || psurf->numVerts == 0 ) {
				continue;
			}

			//
			// the triangle and vertex split threshold is controlled by a parameter
			// to $base, a la $base blah.3ds 1900, where "1900" determines the number
			// of triangles to split on
			//
			else if ( psurf->numVerts > MAX_SURFACE_VERTS ) {
				Error( "too many vertices\n" );
			}

			psurf->numFrames = numFrames;

			psurf->ofsShaders = sizeof( md3Surface_t );

			if ( psurf->numTriangles > MAX_SURFACE_TRIS  ) {
				Error( "too many faces\n" );
			}

			psurf->ofsTriangles = psurf->ofsShaders + psurf->numShaders * sizeof( md3Shader_t );

			psurf->ofsSt = psurf->ofsTriangles + psurf->numTriangles * sizeof( md3Triangle_t );
			psurf->ofsXyzNormals = psurf->ofsSt + psurf->numVerts * sizeof( md3St_t );
			psurf->ofsEnd = psurf->ofsXyzNormals + psurf->numFrames * psurf->numVerts * ( sizeof( short ) * 4 );

			surfaceSum += psurf->ofsEnd;

			numRealSurfaces++;
		}
	}

	g_data.model.ident = MD3_IDENT;
	g_data.model.version = MD3_VERSION;

	g_data.model.ofsFrames = sizeof( md3Header_t );
	g_data.model.ofsTags = g_data.model.ofsFrames + numFrames * sizeof( md3Frame_t );
	g_data.model.ofsSurfaces = g_data.model.ofsTags + numFrames * g_data.model.numTags * sizeof( md3Tag_t );
	g_data.model.ofsEnd = g_data.model.ofsSurfaces + surfaceSum;

	//
	// write out the model header
	//
	modeltemp = g_data.model;
	modeltemp.ident = LittleLong( modeltemp.ident );
	modeltemp.version = LittleLong( modeltemp.version );
	modeltemp.numFrames = LittleLong( modeltemp.numFrames );
	modeltemp.numTags = LittleLong( modeltemp.numTags );
	modeltemp.numSurfaces = LittleLong( numRealSurfaces );
	modeltemp.ofsFrames = LittleLong( modeltemp.ofsFrames );
	modeltemp.ofsTags = LittleLong( modeltemp.ofsTags );
	modeltemp.ofsSurfaces = LittleLong( modeltemp.ofsSurfaces );
	modeltemp.ofsEnd = LittleLong( modeltemp.ofsEnd );

	SafeWrite( modelouthandle, &modeltemp, sizeof( modeltemp ) );

	//
	// write out the frames
	//
	for ( i = 0 ; i < numFrames ; i++ )
	{
		vec3_t tmpVec;
		float maxRadius = 0;

		//
		// compute localOrigin and radius
		//
		g_data.frames[i].localOrigin[0] =
			g_data.frames[i].localOrigin[1] =
				g_data.frames[i].localOrigin[2] = 0;

		for ( j = 0; j < 8; j++ )
		{
			tmpVec[0] = g_data.frames[i].bounds[( j & 1 ) != 0][0];
			tmpVec[1] = g_data.frames[i].bounds[( j & 2 ) != 0][1];
			tmpVec[2] = g_data.frames[i].bounds[( j & 4 ) != 0][2];

			if ( VectorLength( tmpVec ) > maxRadius ) {
				maxRadius = VectorLength( tmpVec );
			}
		}

		g_data.frames[i].radius = LittleFloat( maxRadius );

		// swap
		for ( j = 0 ; j < 3 ; j++ ) {
			g_data.frames[i].bounds[0][j] = LittleFloat( g_data.frames[i].bounds[0][j] );
			g_data.frames[i].bounds[1][j] = LittleFloat( g_data.frames[i].bounds[1][j] );
			g_data.frames[i].localOrigin[j] = LittleFloat( g_data.frames[i].localOrigin[j] );
		}
	}
	fseek( modelouthandle, g_data.model.ofsFrames, SEEK_SET );
	SafeWrite( modelouthandle, g_data.frames, numFrames * sizeof( g_data.frames[0] ) );

	//
	// write out the tags
	//
	fseek( modelouthandle, g_data.model.ofsTags, SEEK_SET );
	for ( f = 0 ; f < g_data.model.numFrames; f++ )
	{
		int t;

		for ( t = 0; t < g_data.model.numTags; t++ )
		{
			g_data.tags[f][t].origin[0] = LittleFloat( g_data.tags[f][t].origin[0] );
			g_data.tags[f][t].origin[1] = LittleFloat( g_data.tags[f][t].origin[1] );
			g_data.tags[f][t].origin[2] = LittleFloat( g_data.tags[f][t].origin[2] );

			for ( j = 0 ; j < 3 ; j++ )
			{
				g_data.tags[f][t].axis[0][j] = LittleFloat( g_data.tags[f][t].axis[0][j] );
				g_data.tags[f][t].axis[1][j] = LittleFloat( g_data.tags[f][t].axis[1][j] );
				g_data.tags[f][t].axis[2][j] = LittleFloat( g_data.tags[f][t].axis[2][j] );
			}
		}
		SafeWrite( modelouthandle, g_data.tags[f], g_data.model.numTags * sizeof( md3Tag_t ) );
	}

	//
	// write out the surfaces
	//
	fseek( modelouthandle, g_data.model.ofsSurfaces, SEEK_SET );
	for ( i = 0; i < g_data.model.numSurfaces; i++ )
	{
		WriteModelSurface( modelouthandle, &g_data.surfData[i] );
	}
}


/*
   ===============
   FinishModel
   ===============
 */
void FinishModel( int type ){
	FILE        *modelouthandle;
	FILE        *defaultSkinHandle;
	char name[1024];
	int i;

	if ( !g_data.model.numFrames ) {
		return;
	}

	//
	// build generalized triangle strips
	//
	OrderSurfaces();

	if ( type == TYPE_PLAYER ) {
		sprintf( name, "%s%s", writedir, g_modelname );
		*strrchr( name, '.' ) = 0;
		strcat( name, "_default.skin" );

		defaultSkinHandle = fopen( name, "wt" );
		for ( i = 0; i < g_data.model.numSurfaces; i++ )
		{
			fprintf( defaultSkinHandle, "%s,%s\n", g_data.surfData[i].header.name, g_data.surfData[i].shaders[0].name );
		}
		fclose( defaultSkinHandle );
	}

	sprintf( name, "%s%s", writedir, g_modelname );

	//
	// copy the model and its shaders to release directory tree
	// if doing a release build
	//
	if ( g_release ) {
		int i, j;
		md3SurfaceData_t *pSurf;

		ReleaseFile( g_modelname );

		for ( i = 0; i < g_data.model.numSurfaces; i++ ) {
			pSurf = &g_data.surfData[i];
			for ( j = 0; j < g_data.model.numSkins; j++ ) {
				ReleaseShader( pSurf->shaders[j].name );
			}
		}
		return;
	}

	//
	// write the model output file
	//
	printf( "saving to %s\n", name );
	CreatePath( name );
	modelouthandle = SafeOpenWrite( name );

	WriteModelFile( modelouthandle );

	printf( "%4d surfaces\n", g_data.model.numSurfaces );
	printf( "%4d frames\n", g_data.model.numFrames );
	printf( "%4d tags\n", g_data.model.numTags );
	printf( "file size: %d\n", (int)ftell( modelouthandle ) );
	printf( "---------------------\n" );

	fclose( modelouthandle );
}

/*
** OrderSurfaces
**
** Reorders triangles in all the surfaces.
*/
static void OrderSurfaces( void ){
	int s;
	extern qboolean g_stripify;

	// go through each surface and find best strip/fans possible
	for ( s = 0; s < g_data.model.numSurfaces; s++ )
	{
		int mesh[MD3_MAX_TRIANGLES][3];
		int i;

		printf( "stripifying surface %d/%d with %d tris\n", s, g_data.model.numSurfaces, g_data.surfData[s].header.numTriangles );

		for ( i = 0; i < g_data.surfData[s].header.numTriangles; i++ )
		{
			mesh[i][0] = g_data.surfData[s].lodTriangles[i][0];
			mesh[i][1] = g_data.surfData[s].lodTriangles[i][1];
			mesh[i][2] = g_data.surfData[s].lodTriangles[i][2];
		}

		if ( g_stripify ) {
			OrderMesh( mesh,                                    // input
					   g_data.surfData[s].orderedTriangles,     // output
					   g_data.surfData[s].header.numTriangles );
		}
		else
		{
			memcpy( g_data.surfData[s].orderedTriangles, mesh, sizeof( int ) * 3 * g_data.surfData[s].header.numTriangles );
		}
	}
}


/*
   ===============================================================

   BASE FRAME SETUP

   ===============================================================
 */
/*
   ============
   CopyTrianglesToBaseTriangles

   ============
 */
static void CopyTrianglesToBaseTriangles( triangle_t *ptri, int numtri, baseTriangle_t *bTri ){
	int i;
//	int			width, height, iwidth, iheight, swidth;
//	float		s_scale, t_scale;
//	float		scale;
//	vec3_t		mins, maxs;

/*
    //
    // find bounds of all the verts on the base frame
    //
    ClearBounds (mins, maxs);

    for (i=0 ; i<numtri ; i++)
        for (j=0 ; j<3 ; j++)
            AddPointToBounds (ptri[i].verts[j], mins, maxs);

    for (i=0 ; i<3 ; i++)
    {
        mins[i] = floor(mins[i]);
        maxs[i] = ceil(maxs[i]);
    }

    width = maxs[0] - mins[0];
    height = maxs[2] - mins[2];

    if (!g_data.fixedwidth)
    {	// old style
        scale = 8;
        if (width*scale >= 150)
            scale = 150.0 / width;
        if (height*scale >= 190)
            scale = 190.0 / height;

        s_scale = t_scale = scale;

        iwidth = ceil(width*s_scale);
        iheight = ceil(height*t_scale);

        iwidth += 4;
        iheight += 4;
    }
    else
    {	// new style
        iwidth = g_data.fixedwidth / 2;
        iheight = g_data.fixedheight;

        s_scale = (float)(iwidth-4) / width;
        t_scale = (float)(iheight-4) / height;
    }

    // make the width a multiple of 4; some hardware requires this, and it ensures
    // dword alignment for each scan
    swidth = iwidth*2;
    g_data.skinwidth = (swidth + 3) & ~3;
    g_data.skinheight = iheight;
 */

	for ( i = 0; i < numtri ; i++, ptri++, bTri++ )
	{
		int j;

		for ( j = 0 ; j < 3 ; j++ )
		{
			VectorCopy( ptri->verts[j], bTri->v[j].xyz );
			VectorCopy( ptri->normals[j], bTri->v[j].normal );

			bTri->v[j].st[0] = ptri->texcoords[j][0];
			bTri->v[j].st[1] = ptri->texcoords[j][1];
		}
	}
}

static void BuildBaseFrame( const char *filename, ObjectAnimationFrame_t *pOAF ){
	baseTriangle_t  *bTri;
	baseVertex_t    *bVert;
	int i, j;

	// calculate the base triangles
	for ( i = 0; i < g_data.model.numSurfaces; i++ )
	{
		CopyTrianglesToBaseTriangles( pOAF->surfaces[i]->triangles,
									  pOAF->surfaces[i]->numtriangles,
									  g_data.surfData[i].baseTriangles );

		strcpy( g_data.surfData[i].header.name, pOAF->surfaces[i]->name );

		g_data.surfData[i].header.numTriangles = pOAF->surfaces[i]->numtriangles;
		g_data.surfData[i].header.numVerts = 0;

/*
        if ( strstr( filename, gamedir + 1 ) )
        {
            strcpy( shaderName, strstr( filename, gamedir + 1 ) + strlen( gamedir ) - 1 );
        }
        else
        {
            strcpy( shaderName, filename );
        }

        if ( strrchr( shaderName, '/' ) )
   *( strrchr( shaderName, '/' ) + 1 ) = 0;


        strcpy( shaderName, pOAF->surfaces[i]->materialname );
 */
		strcpy( g_data.surfData[i].shaders[g_data.surfData[i].header.numShaders].name, pOAF->surfaces[i]->materialname );

		g_data.surfData[i].header.numShaders++;
	}

	//
	// compute unique vertices for each polyset
	//
	for ( i = 0; i < g_data.model.numSurfaces; i++ )
	{
		int t;

		for ( t = 0; t < pOAF->surfaces[i]->numtriangles; t++ )
		{
			bTri = &g_data.surfData[i].baseTriangles[t];

			for ( j = 0 ; j < 3 ; j++ )
			{
				int k;

				bVert = &bTri->v[j];

				// get the xyz index
				for ( k = 0; k < g_data.surfData[i].header.numVerts; k++ )
				{
					if ( ( g_data.surfData[i].baseVertexes[k].st[0] == bVert->st[0] ) &&
						 ( g_data.surfData[i].baseVertexes[k].st[1] == bVert->st[1] ) &&
						 ( VectorCompare( bVert->xyz, g_data.surfData[i].baseVertexes[k].xyz ) ) &&
						 ( VectorCompare( bVert->normal, g_data.surfData[i].baseVertexes[k].normal ) ) ) {
						break;  // this vertex is already in the base vertex list
					}
				}

				if ( k == g_data.surfData[i].header.numVerts ) {    // new index
					g_data.surfData[i].baseVertexes[g_data.surfData[i].header.numVerts] = *bVert;
					g_data.surfData[i].header.numVerts++;
				}

				bVert->index = k;

				g_data.surfData[i].lodTriangles[t][j] = k;
			}
		}
	}

	//
	// find tags
	//
	for ( i = 0; i < g_data.model.numSurfaces; i++ )
	{
		if ( strstr( pOAF->surfaces[i]->name, "tag_" ) == pOAF->surfaces[i]->name ) {
			if ( pOAF->surfaces[i]->numtriangles != 1 ) {
				Error( "tag polysets must consist of only one triangle" );
			}
			if ( strstr( filename, "_flash.md3" ) && !strcmp( pOAF->surfaces[i]->name, "tag_parent" ) ) {
				continue;
			}
			printf( "found tag '%s'\n", pOAF->surfaces[i]->name );
			g_data.model.numTags++;
		}
	}

}

static int LoadModelFile( const char *filename, polyset_t **psets, int *numpolysets ){
	int time1;
	char file1[1024];
	const char          *frameFile;

	printf( "---------------------\n" );
	if ( filename[1] != ':' ) {
		frameFile = filename;
		sprintf( file1, "%s/%s", g_cddir, frameFile );
	}
	else
	{
		strcpy( file1, filename );
	}

	time1 = FileTime( file1 );
	if ( time1 == -1 ) {
		Error( "%s doesn't exist", file1 );
	}

	//
	// load the base triangles
	//
	*psets = Polyset_LoadSets( file1, numpolysets, g_data.maxSurfaceTris );

	//
	// snap polysets
	//
	Polyset_SnapSets( *psets, *numpolysets );

	if ( strstr( file1, ".3ds" ) || strstr( file1, ".3DS" ) ) {
		return MD3_TYPE_BASE3DS;
	}

	Error( "Unknown model file type" );

	return MD3_TYPE_UNKNOWN;
}

/*
   =================
   Cmd_Base
   =================
 */
void Cmd_Base( void ){
	char filename[1024];

	GetToken( qfalse );
	sprintf( filename, "%s/%s", g_cddir, token );
	LoadBase( filename );
}

static void LoadBase( const char *filename ){
	int numpolysets;
	polyset_t *psets;
	int i;
	ObjectAnimationFrame_t oaf;

	// determine polyset splitting threshold
	if ( TokenAvailable() ) {
		GetToken( qfalse );
		g_data.maxSurfaceTris = atoi( token );
	}
	else
	{
		g_data.maxSurfaceTris = MAX_SURFACE_TRIS - 1;
	}

	g_data.type = LoadModelFile( filename, &psets, &numpolysets );

	Polyset_ComputeNormals( psets, numpolysets );

	g_data.model.numSurfaces = numpolysets;

	memset( &oaf, 0, sizeof( oaf ) );

	for ( i = 0; i < numpolysets; i++ )
	{
		oaf.surfaces[i] = &psets[i];
		oaf.numSurfaces = numpolysets;
	}

	BuildBaseFrame( filename, &oaf );

	free( psets[0].triangles );
	free( psets );
}

/*
   =================
   Cmd_SpriteBase

   $spritebase xorg yorg width height

   Generate a single square for the model
   =================
 */
void Cmd_SpriteBase( void ){
	float xl, yl, width, height;

	g_data.type = MD3_TYPE_SPRITE;

	GetToken( qfalse );
	xl = atof( token );
	GetToken( qfalse );
	yl = atof( token );
	GetToken( qfalse );
	width = atof( token );
	GetToken( qfalse );
	height = atof( token );

//	if (g_skipmodel || g_release || g_archive)
//		return;

	printf( "---------------------\n" );

	g_data.surfData[0].verts[0] = ( float * ) calloc( 1, sizeof( float ) * 6 * 4 );

	g_data.surfData[0].header.numVerts = 4;

	g_data.surfData[0].verts[0][0 + 0] = 0;
	g_data.surfData[0].verts[0][0 + 1] = -xl;
	g_data.surfData[0].verts[0][0 + 2] = yl + height;

	g_data.surfData[0].verts[0][0 + 3] = -1;
	g_data.surfData[0].verts[0][0 + 4] = 0;
	g_data.surfData[0].verts[0][0 + 5] = 0;
	g_data.surfData[0].baseVertexes[0].st[0] = 0;
	g_data.surfData[0].baseVertexes[0].st[1] = 0;


	g_data.surfData[0].verts[0][6 + 0] = 0;
	g_data.surfData[0].verts[0][6 + 1] = -xl - width;
	g_data.surfData[0].verts[0][6 + 2] = yl + height;

	g_data.surfData[0].verts[0][6 + 3] = -1;
	g_data.surfData[0].verts[0][6 + 4] = 0;
	g_data.surfData[0].verts[0][6 + 5] = 0;
	g_data.surfData[0].baseVertexes[1].st[0] = 1;
	g_data.surfData[0].baseVertexes[1].st[1] = 0;


	g_data.surfData[0].verts[0][12 + 0] = 0;
	g_data.surfData[0].verts[0][12 + 1] = -xl - width;
	g_data.surfData[0].verts[0][12 + 2] = yl;

	g_data.surfData[0].verts[0][12 + 3] = -1;
	g_data.surfData[0].verts[0][12 + 4] = 0;
	g_data.surfData[0].verts[0][12 + 5] = 0;
	g_data.surfData[0].baseVertexes[2].st[0] = 1;
	g_data.surfData[0].baseVertexes[2].st[1] = 1;


	g_data.surfData[0].verts[0][18 + 0] = 0;
	g_data.surfData[0].verts[0][18 + 1] = -xl;
	g_data.surfData[0].verts[0][18 + 2] = yl;

	g_data.surfData[0].verts[0][18 + 3] = -1;
	g_data.surfData[0].verts[0][18 + 4] = 0;
	g_data.surfData[0].verts[0][18 + 5] = 0;
	g_data.surfData[0].baseVertexes[3].st[0] = 0;
	g_data.surfData[0].baseVertexes[3].st[1] = 1;

	g_data.surfData[0].lodTriangles[0][0] = 0;
	g_data.surfData[0].lodTriangles[0][1] = 1;
	g_data.surfData[0].lodTriangles[0][2] = 2;

	g_data.surfData[0].lodTriangles[1][0] = 2;
	g_data.surfData[0].lodTriangles[1][1] = 3;
	g_data.surfData[0].lodTriangles[1][2] = 0;

	g_data.model.numSurfaces = 1;

	g_data.surfData[0].header.numTriangles = 2;
	g_data.surfData[0].header.numVerts = 4;

	g_data.model.numFrames = 1;
}

/*
   ===========================================================================

   FRAME GRABBING

   ===========================================================================
 */

/*
   ===============
   GrabFrame
   ===============
 */
void GrabFrame( const char *frame ){
	int i, j, k;
	char file1[1024];
	md3Frame_t      *fr;
	md3Tag_t tagParent;
	float       *frameXyz;
	float       *frameNormals;
	const char  *framefile;
	polyset_t       *psets;
	qboolean parentTagExists = qfalse;
	int numpolysets;
	int numtags = 0;
	int tagcount;

	// the frame 'run1' will be looked for as either
	// run.1 or run1.tri, so the new alias sequence save
	// feature an be used
	if ( frame[1] != ':' ) {
//		framefile = FindFrameFile (frame);
		framefile = frame;
		sprintf( file1, "%s/%s",g_cddir, framefile );
	}
	else
	{
		strcpy( file1, frame );
	}
	printf( "grabbing %s\n", file1 );

	if ( g_data.model.numFrames >= MD3_MAX_FRAMES ) {
		Error( "model.numFrames >= MD3_MAX_FRAMES" );
	}
	fr = &g_data.frames[g_data.model.numFrames];

	strcpy( fr->name, frame );

	psets = Polyset_LoadSets( file1, &numpolysets, g_data.maxSurfaceTris );

	//
	// snap polysets
	//
	Polyset_SnapSets( psets, numpolysets );

	//
	// compute vertex normals
	//
	Polyset_ComputeNormals( psets, numpolysets );

	//
	// flip everything to compensate for the alias coordinate system
	// and perform global scale and adjust
	//
	for ( i = 0; i < g_data.model.numSurfaces; i++ )
	{
		triangle_t *ptri = psets[i].triangles;
		int t;

		for ( t = 0; t < psets[i].numtriangles; t++ )
		{

			for ( j = 0; j < 3; j++ )
			{

				// scale and adjust
				for ( k = 0 ; k < 3 ; k++ ) {
					ptri[t].verts[j][k] = ptri[t].verts[j][k] * g_data.scale_up +
										  g_data.adjust[k];

					if ( ptri[t].verts[j][k] > 1023 ||
						 ptri[t].verts[j][k] < -1023 ) {
						Error( "Model extents too large" );
					}
				}
			}
		}
	}

	//
	// find and count tags, locate parent tag
	//
	for ( i = 0; i < numpolysets; i++ )
	{
		if ( strstr( psets[i].name, "tag_" ) == psets[i].name ) {
			if ( strstr( psets[i].name, "tag_parent" ) == psets[i].name ) {
				if ( strstr( psets[i].name, "tag_parent" ) ) {
					float tri[3][3];

					if ( parentTagExists ) {
						Error( "Multiple parent tags not allowed" );
					}

					memcpy( tri[0], psets[i].triangles[0].verts[0], sizeof( float ) * 3 );
					memcpy( tri[1], psets[i].triangles[0].verts[1], sizeof( float ) * 3 );
					memcpy( tri[2], psets[i].triangles[0].verts[2], sizeof( float ) * 3 );

					MD3_ComputeTagFromTri( &tagParent, tri );
					strcpy( tagParent.name, psets[i].name );
					g_data.tags[g_data.model.numFrames][numtags] = tagParent;
					parentTagExists = qtrue;

				}
			}
			numtags++;
		}

		if ( strcmp( psets[i].name, g_data.surfData[i].header.name ) ) {
			Error( "Mismatched surfaces from base('%s') to frame('%s') in model '%s'\n", g_data.surfData[i].header.name, psets[i].name, g_modelname );
		}
	}

	if ( numtags != g_data.model.numTags ) {
		Error( "mismatched number of tags in frame(%d) vs. base(%d)", numtags, g_data.model.numTags );
	}

	if ( numpolysets != g_data.model.numSurfaces ) {
		Error( "mismatched number of surfaces in frame(%d) vs. base(%d)", numpolysets - numtags, g_data.model.numSurfaces );
	}

	//
	// prepare to accumulate bounds and normals
	//
	ClearBounds( fr->bounds[0], fr->bounds[1] );

	//
	// store the frame's vertices in the same order as the base. This assumes the
	// triangles and vertices in this frame are in exactly the same order as in the
	// base
	//
	for ( i = 0, tagcount = 0; i < numpolysets; i++ )
	{
		int t;
		triangle_t *pTris = psets[i].triangles;

		strcpy( g_data.surfData[i].header.name, psets[i].name );

		//
		// parent tag adjust
		//
		if ( parentTagExists ) {
			for ( t = 0; t < psets[i].numtriangles; t++ )
			{
				for ( j = 0; j < 3 ; j++ )
				{
					vec3_t tmp;

					VectorSubtract( pTris[t].verts[j], tagParent.origin, tmp );

					pTris[t].verts[j][0] = DotProduct( tmp, tagParent.axis[0] );
					pTris[t].verts[j][1] = DotProduct( tmp, tagParent.axis[1] );
					pTris[t].verts[j][2] = DotProduct( tmp, tagParent.axis[2] );

					VectorCopy( pTris[t].normals[j], tmp );
					pTris[t].normals[j][0] = DotProduct( tmp, tagParent.axis[0] );
					pTris[t].normals[j][1] = DotProduct( tmp, tagParent.axis[1] );
					pTris[t].normals[j][2] = DotProduct( tmp, tagParent.axis[2] );
				}
			}
		}

		//
		// compute tag data
		//
		if ( strstr( psets[i].name, "tag_" ) == psets[i].name ) {
			md3Tag_t *pTag = &g_data.tags[g_data.model.numFrames][tagcount];
			float tri[3][3];

			strcpy( pTag->name, psets[i].name );

			memcpy( tri[0], pTris[0].verts[0], sizeof( float ) * 3 );
			memcpy( tri[1], pTris[0].verts[1], sizeof( float ) * 3 );
			memcpy( tri[2], pTris[0].verts[2], sizeof( float ) * 3 );

			MD3_ComputeTagFromTri( pTag, tri );
			tagcount++;
		}
		else
		{
			if ( g_data.surfData[i].verts[g_data.model.numFrames] ) {
				free( g_data.surfData[i].verts[g_data.model.numFrames] );
			}
			frameXyz = g_data.surfData[i].verts[g_data.model.numFrames] = calloc( 1, sizeof( float ) * 6 * g_data.surfData[i].header.numVerts );
			frameNormals = frameXyz + 3;

			for ( t = 0; t < psets[i].numtriangles; t++ )
			{
				for ( j = 0; j < 3 ; j++ )
				{
					int index;

					index = g_data.surfData[i].baseTriangles[t].v[j].index;
					frameXyz[index * 6 + 0] = pTris[t].verts[j][0];
					frameXyz[index * 6 + 1] = pTris[t].verts[j][1];
					frameXyz[index * 6 + 2] = pTris[t].verts[j][2];
					frameNormals[index * 6 + 0] =  pTris[t].normals[j][0];
					frameNormals[index * 6 + 1] =  pTris[t].normals[j][1];
					frameNormals[index * 6 + 2] =  pTris[t].normals[j][2];
					AddPointToBounds( &frameXyz[index * 6], fr->bounds[0], fr->bounds[1] );
				}
			}
		}
	}

	g_data.model.numFrames++;

	// only free the first triangle array, all of the psets in this array share the
	// same triangle pool!!!
//	free( psets[0].triangles );
//	free( psets );
}

//===========================================================================



/*
   ===============
   Cmd_Frame
   ===============
 */
void Cmd_Frame( void ){
	while ( TokenAvailable() )
	{
		GetToken( qfalse );
		if ( g_skipmodel ) {
			continue;
		}
		if ( g_release || g_archive ) {
			g_data.model.numFrames = 1; // don't skip the writeout
			continue;
		}

		GrabFrame( token );
	}
}


/*
   ===============
   Cmd_Skin

   ===============
 */
void SkinFrom3DS( const char *filename ){
	polyset_t *psets;
	char name[1024];
	int numPolysets;
	int i;

	_3DS_LoadPolysets( filename, &psets, &numPolysets, g_verbose );

	for ( i = 0; i < numPolysets; i++ )
	{
/*
        if ( strstr( filename, gamedir + 1 ) )
        {
            strcpy( name, strstr( filename, gamedir + 1 ) + strlen( gamedir ) - 1 );
        }
        else
        {
            strcpy( name, filename );
        }

        if ( strrchr( name, '/' ) )
   *( strrchr( name, '/' ) + 1 ) = 0;
 */
		strcpy( name, psets[i].materialname );
		strcpy( g_data.surfData[i].shaders[g_data.surfData[i].header.numShaders].name, name );

		g_data.surfData[i].header.numShaders++;
	}

	free( psets[0].triangles );
	free( psets );
}

void Cmd_Skin( void ){
	char skinfile[1024];

	if ( g_data.type == MD3_TYPE_BASE3DS ) {
		GetToken( qfalse );

		sprintf( skinfile, "%s/%s", g_cddir, token );

		if ( strstr( token, ".3ds" ) || strstr( token, ".3DS" ) ) {
			SkinFrom3DS( skinfile );
		}
		else
		{
			Error( "Unknown file format for $skin '%s'\n", skinfile );
		}
	}
	else
	{
		Error( "invalid model type while processing $skin" );
	}

	g_data.model.numSkins++;
}

/*
   =================
   Cmd_SpriteShader
   =================

   This routine is also called for $oldskin

 */
void Cmd_SpriteShader(){
	GetToken( qfalse );
	strcpy( g_data.surfData[0].shaders[g_data.surfData[0].header.numShaders].name, token );
	g_data.surfData[0].header.numShaders++;
	g_data.model.numSkins++;
}

/*
   =================
   Cmd_Origin
   =================
 */
void Cmd_Origin( void ){
	// rotate points into frame of reference so model points down the
	// positive x axis
	// FIXME: use alias native coordinate system
	GetToken( qfalse );
	g_data.adjust[1] = -atof( token );

	GetToken( qfalse );
	g_data.adjust[0] = atof( token );

	GetToken( qfalse );
	g_data.adjust[2] = -atof( token );
}


/*
   =================
   Cmd_ScaleUp
   =================
 */
void Cmd_ScaleUp( void ){
	GetToken( qfalse );
	g_data.scale_up = atof( token );
	if ( g_skipmodel || g_release || g_archive ) {
		return;
	}

	printf( "Scale up: %f\n", g_data.scale_up );
}


/*
   =================
   Cmd_Skinsize

   Set a skin size other than the default
   QUAKE3: not needed
   =================
 */
void Cmd_Skinsize( void ){
	GetToken( qfalse );
	g_data.fixedwidth = atoi( token );
	GetToken( qfalse );
	g_data.fixedheight = atoi( token );
}

/*
   =================
   Cmd_Modelname

   Begin creating a model of the given name
   =================
 */
void Cmd_Modelname( void ){
	FinishModel( TYPE_UNKNOWN );
	ClearModel();

	GetToken( qfalse );
	strcpy( g_modelname, token );
	StripExtension( g_modelname );
	strcat( g_modelname, ".md3" );
	strcpy( g_data.model.name, g_modelname );
}

/*
   ===============
   fCmd_Cd
   ===============
 */
void Cmd_Cd( void ){
	if ( g_cddir[0] ) {
		Error( "$cd command without a $modelname" );
	}

	GetToken( qfalse );

	sprintf( g_cddir, "%s%s", gamedir, token );

	// if -only was specified and this cd doesn't match,
	// skip the model (you only need to match leading chars,
	// so you could regrab all monsters with -only models/monsters)
	if ( !g_only[0] ) {
		return;
	}
	if ( strncmp( token, g_only, strlen( g_only ) ) ) {
		g_skipmodel = qtrue;
		printf( "skipping %s\n", token );
	}
}

void Convert3DStoMD3( const char *file ){
	LoadBase( file );
	GrabFrame( file );
	SkinFrom3DS( file );

	strcpy( g_data.model.name, g_modelname );

	FinishModel( TYPE_UNKNOWN );
	ClearModel();
}

/*
** Cmd_3DSConvert
*/
void Cmd_3DSConvert(){
	char file[1024];

	FinishModel( TYPE_UNKNOWN );
	ClearModel();

	GetToken( qfalse );

	sprintf( file, "%s%s", gamedir, token );
	strcpy( g_modelname, token );
	if ( strrchr( g_modelname, '.' ) ) {
		*strrchr( g_modelname, '.' ) = 0;
	}
	strcat( g_modelname, ".md3" );

	if ( FileTime( file ) == -1 ) {
		Error( "%s doesn't exist", file );
	}

	if ( TokenAvailable() ) {
		GetToken( qfalse );
		g_data.scale_up = atof( token );
	}

	Convert3DStoMD3( file );
}

static void ConvertASE( const char *filename, int type, qboolean grabAnims );

/*
** Cmd_ASEConvert
*/
void Cmd_ASEConvert( qboolean grabAnims ){
	char filename[1024];
	int type = TYPE_ITEM;

	FinishModel( TYPE_UNKNOWN );
	ClearModel();

	GetToken( qfalse );
	sprintf( filename, "%s%s", gamedir, token );

	strcpy( g_modelname, token );
	StripExtension( g_modelname );
	strcat( g_modelname, ".md3" );
	strcpy( g_data.model.name, g_modelname );

	if ( !strstr( filename, ".ase" ) && !strstr( filename, ".ASE" ) ) {
		strcat( filename, ".ASE" );
	}

	g_data.maxSurfaceTris = MAX_SURFACE_TRIS - 1;

	while ( TokenAvailable() )
	{
		GetToken( qfalse );
		if ( !strcmp( token, "-origin" ) ) {
			if ( !TokenAvailable() ) {
				Error( "missing parameter for -origin" );
			}
			GetToken( qfalse );
			g_data.aseAdjust[1] = -atof( token );

			if ( !TokenAvailable() ) {
				Error( "missing parameter for -origin" );
			}
			GetToken( qfalse );
			g_data.aseAdjust[0] = atof( token );

			if ( !TokenAvailable() ) {
				Error( "missing parameter for -origin" );
			}
			GetToken( qfalse );
			g_data.aseAdjust[2] = -atof( token );
		}
		else if ( !strcmp( token, "-lod" ) ) {
			if ( !TokenAvailable() ) {
				Error( "No parameter for -lod" );
			}
			GetToken( qfalse );
			g_data.currentLod = atoi( token );
			if ( g_data.currentLod > MD3_MAX_LODS - 1 ) {
				Error( "-lod parameter too large! (%d)\n", g_data.currentLod );
			}

			if ( !TokenAvailable() ) {
				Error( "No second parameter for -lod" );
			}
			GetToken( qfalse );
			g_data.lodBias = atof( token );
		}
		else if ( !strcmp( token, "-maxtris" ) ) {
			if ( !TokenAvailable() ) {
				Error( "No parameter for -maxtris" );
			}
			GetToken( qfalse );
			g_data.maxSurfaceTris = atoi( token );
		}
		else if ( !strcmp( token, "-playerparms" ) ) {
			if ( !TokenAvailable() ) {
				Error( "missing skip start parameter for -playerparms" );
			}
			GetToken( qfalse );
			g_data.lowerSkipFrameStart = atoi( token );

#if 0
			if ( !TokenAvailable() ) {
				Error( "missing skip end parameter for -playerparms" );
			}
			GetToken( qfalse );
			g_data.lowerSkipFrameEnd = atoi( token );
#endif

			if ( !TokenAvailable() ) {
				Error( "missing upper parameter for -playerparms" );
			}
			GetToken( qfalse );
			g_data.maxUpperFrames = atoi( token );

			g_data.lowerSkipFrameEnd = g_data.maxUpperFrames - 1;

#if 0
			if ( !TokenAvailable() ) {
				Error( "missing head parameter for -playerparms" );
			}
			GetToken( qfalse );
			g_data.maxHeadFrames = atoi( token );
#endif
			g_data.maxHeadFrames = 1;

			if ( type != TYPE_ITEM ) {
				Error( "invalid argument" );
			}

			type = TYPE_PLAYER;
		}
		else if ( !strcmp( token, "-weapon" ) ) {
			if ( type != TYPE_ITEM ) {
				Error( "invalid argument" );
			}

			type = TYPE_WEAPON;
		}
	}

	g_data.type = MD3_TYPE_ASE;

	if ( type == TYPE_WEAPON && grabAnims ) {
		Error( "can't grab anims with weapon models" );
	}
	if ( type == TYPE_PLAYER && !grabAnims ) {
		Error( "player models must be converted with $aseanimconvert" );
	}

	if ( type == TYPE_WEAPON ) {
		ConvertASE( filename, type, qfalse );
		ConvertASE( filename, TYPE_HAND, qtrue );
	}
	else
	{
		ConvertASE( filename, type, grabAnims );
	}
}

static int GetSurfaceAnimations( SurfaceAnimation_t sanims[MAX_ANIM_SURFACES],
								 const char *part,
								 int skipFrameStart,
								 int skipFrameEnd,
								 int maxFrames ){
	int numSurfaces;
	int numValidSurfaces;
	int i;
	int numFrames = -1;

	if ( ( numSurfaces = ASE_GetNumSurfaces() ) > MAX_ANIM_SURFACES ) {
		Error( "Too many surfaces in ASE" );
	}

	for ( numValidSurfaces = 0, i = 0; i < numSurfaces; i++ )
	{
		polyset_t *splitSets;
		int numNewFrames;
		const char *surfaceName = ASE_GetSurfaceName( i );

		if ( !surfaceName ) {
			continue;
//			Error( "Missing animation frames in model" );
		}

		if ( strstr( surfaceName, "tag_" ) ||
			 !strcmp( part, "any" ) ||
			 ( strstr( surfaceName, part ) == surfaceName ) ) {

			// skip this if it's an inappropriate tag
			if ( strcmp( part, "any" ) ) {
				// ignore non-"tag_head" tags if this is the head
				if ( !strcmp( part, "h_" ) && strstr( surfaceName, "tag_" ) && strcmp( surfaceName, "tag_head" ) ) {
					continue;
				}
				// ignore "tag_head" if this is the legs
				if ( !strcmp( part, "l_" ) && !strcmp( surfaceName, "tag_head" ) ) {
					continue;
				}
				// ignore "tag_weapon" if this is the legs
				if ( !strcmp( part, "l_" ) && !strcmp( surfaceName, "tag_weapon" ) ) {
					continue;
				}
			}

			if ( ( sanims[numValidSurfaces].frames = ASE_GetSurfaceAnimation( i, &sanims[numValidSurfaces].numFrames, skipFrameStart, skipFrameEnd, maxFrames ) ) != 0 ) {
				splitSets = Polyset_SplitSets( sanims[numValidSurfaces].frames, sanims[numValidSurfaces].numFrames, &numNewFrames, g_data.maxSurfaceTris );

				if ( numFrames == -1 ) {
					numFrames = sanims[numValidSurfaces].numFrames;
				}
				else if ( numFrames != sanims[numValidSurfaces].numFrames ) {
					Error( "Different number of animation frames on surfaces" );
				}

				if ( sanims[numValidSurfaces].frames != splitSets ) {
					int j;

					// free old data if we split the surfaces
					for ( j = 0; j < sanims[numValidSurfaces].numFrames; j++ )
					{
						free( sanims[numValidSurfaces].frames[j].triangles );
						free( sanims[numValidSurfaces].frames );
					}

					sanims[numValidSurfaces].frames = splitSets;
					sanims[numValidSurfaces].numFrames = numNewFrames;
				}
				Polyset_SnapSets( sanims[numValidSurfaces].frames, sanims[numValidSurfaces].numFrames );
				Polyset_ComputeNormals( sanims[numValidSurfaces].frames, sanims[numValidSurfaces].numFrames );

				numValidSurfaces++;
			}
		}
	}

	return numValidSurfaces;
}

static int SurfaceOrderToFrameOrder( SurfaceAnimation_t sanims[], ObjectAnimationFrame_t oanims[], int numSurfaces ){
	int i, s;
	int numFrames = -1;

	/*
	** we have the data here arranged in surface order, now we need to convert it to
	** frame order
	*/
	for ( i = 0, s = 0; i < numSurfaces; i++ )
	{
		int j;

		if ( sanims[i].frames ) {
			if ( numFrames == -1 ) {
				numFrames = sanims[i].numFrames;
			}
			else if ( numFrames != sanims[i].numFrames ) {
				Error( "numFrames != sanims[i].numFrames (%d != %d)\n", numFrames, sanims[i].numFrames );
			}

			for ( j = 0; j < sanims[i].numFrames; j++ )
			{
				oanims[j].surfaces[s] = &sanims[i].frames[j];
				oanims[j].numSurfaces = numSurfaces;
			}
			s++;
		}
	}

	return numFrames;
}

static void WriteMD3( const char *_filename, ObjectAnimationFrame_t oanims[], int numFrames ){
	char filename[1024];

	strcpy( filename, _filename );
	if ( strchr( filename, '.' ) ) {
		*strchr( filename, '.' ) = 0;
	}
	strcat( filename, ".md3" );
}

static void BuildAnimationFromOAFs( const char *filename, ObjectAnimationFrame_t oanims[], int numFrames, int type ){
	int f, i, j, tagcount;
	float *frameXyz;
	float *frameNormals;

	g_data.model.numSurfaces = oanims[0].numSurfaces;
	g_data.model.numFrames = numFrames;
	if ( g_data.model.numFrames < 0 ) {
		Error( "model.numFrames < 0" );
	}
	if ( g_data.model.numFrames >= MD3_MAX_FRAMES ) {
		Error( "model.numFrames >= MD3_MAX_FRAMES" );
	}

	// build base frame
	BuildBaseFrame( filename, &oanims[0] );

	// build animation frames
	for ( f = 0; f < numFrames; f++ )
	{
		ObjectAnimationFrame_t *pOAF = &oanims[f];
		qboolean parentTagExists = qfalse;
		md3Tag_t tagParent;
		int numtags = 0;
		md3Frame_t      *fr;

		fr = &g_data.frames[f];

		strcpy( fr->name, "(from ASE)" );

		// scale and adjust frame
		for ( i = 0; i < pOAF->numSurfaces; i++ )
		{
			triangle_t *pTris = pOAF->surfaces[i]->triangles;
			int t;

			for ( t = 0; t < pOAF->surfaces[i]->numtriangles; t++ )
			{
				for ( j = 0; j < 3; j++ )
				{
					int k;

					// scale and adjust
					for ( k = 0 ; k < 3 ; k++ ) {
						pTris[t].verts[j][k] = pTris[t].verts[j][k] * g_data.scale_up +
											   g_data.aseAdjust[k];

						if ( pTris[t].verts[j][k] > 1023 ||
							 pTris[t].verts[j][k] < -1023 ) {
							Error( "Model extents too large" );
						}
					}
				}
			}
		}

		//
		// find and count tags, locate parent tag
		//
		for ( i = 0; i < pOAF->numSurfaces; i++ )
		{
			if ( strstr( pOAF->surfaces[i]->name, "tag_" ) == pOAF->surfaces[i]->name ) {
				// ignore parent tags when grabbing a weapon model and this is the flash portion
				if ( !strcmp( pOAF->surfaces[i]->name, "tag_parent" ) && strstr( filename, "_flash.md3" ) ) {
					continue;
				}
				else if ( !strstr( filename, "_hand.md3" ) && (
							  ( !strcmp( pOAF->surfaces[i]->name, "tag_parent" ) && !strstr( filename, "_flash.md3" ) ) ||
							  ( !strcmp( pOAF->surfaces[i]->name, "tag_torso" ) && ( strstr( filename, "upper_" ) || strstr( filename, "upper.md3" ) ) ) ||
							  ( !strcmp( pOAF->surfaces[i]->name, "tag_head" ) && ( strstr( filename, "head.md3" ) || strstr( filename, "head_" ) ) ) ||
							  ( !strcmp( pOAF->surfaces[i]->name, "tag_flash" ) && strstr( filename, "_flash.md3" ) ) ||
							  ( !strcmp( pOAF->surfaces[i]->name, "tag_weapon" ) && type == TYPE_WEAPON ) ) ) {
					float tri[3][3];

					if ( parentTagExists ) {
						Error( "Multiple parent tags not allowed" );
					}

					memcpy( tri[0], pOAF->surfaces[i]->triangles[0].verts[0], sizeof( float ) * 3 );
					memcpy( tri[1], pOAF->surfaces[i]->triangles[0].verts[1], sizeof( float ) * 3 );
					memcpy( tri[2], pOAF->surfaces[i]->triangles[0].verts[2], sizeof( float ) * 3 );

					MD3_ComputeTagFromTri( &tagParent, tri );
					strcpy( tagParent.name, "tag_parent" );
					g_data.tags[f][numtags] = tagParent;
					parentTagExists = qtrue;
				}
				else
				{
					float tri[3][3];

					memcpy( tri[0], pOAF->surfaces[i]->triangles[0].verts[0], sizeof( float ) * 3 );
					memcpy( tri[1], pOAF->surfaces[i]->triangles[0].verts[1], sizeof( float ) * 3 );
					memcpy( tri[2], pOAF->surfaces[i]->triangles[0].verts[2], sizeof( float ) * 3 );

					MD3_ComputeTagFromTri( &g_data.tags[f][numtags], tri );
					strcpy( g_data.tags[f][numtags].name, pOAF->surfaces[i]->name );
					if ( strstr( g_data.tags[f][numtags].name, "tag_flash" ) ) {
						*( strstr( g_data.tags[f][numtags].name, "tag_flash" ) + strlen( "tag_flash" ) ) = 0;
					}
				}

				numtags++;
			}

			if ( strcmp( pOAF->surfaces[i]->name, g_data.surfData[i].header.name ) ) {
				Error( "Mismatched surfaces from base('%s') to frame('%s') in model '%s'\n", g_data.surfData[i].header.name, pOAF->surfaces[i]->name, filename );
			}
		}

		if ( numtags != g_data.model.numTags ) {
			Error( "mismatched number of tags in frame(%d) vs. base(%d)", numtags, g_data.model.numTags );
		}

		//
		// prepare to accumulate bounds and normals
		//
		ClearBounds( fr->bounds[0], fr->bounds[1] );

		//
		// store the frame's vertices in the same order as the base. This assumes the
		// triangles and vertices in this frame are in exactly the same order as in the
		// base
		//
		for ( i = 0, tagcount = 0; i < pOAF->numSurfaces; i++ )
		{
			int t;
			triangle_t *pTris = pOAF->surfaces[i]->triangles;

			//
			// parent tag adjust
			//
			if ( parentTagExists ) {
				for ( t = 0; t < pOAF->surfaces[i]->numtriangles; t++ )
				{
					for ( j = 0; j < 3 ; j++ )
					{
						vec3_t tmp;

						VectorSubtract( pTris[t].verts[j], tagParent.origin, tmp );

						pTris[t].verts[j][0] = DotProduct( tmp, tagParent.axis[0] );
						pTris[t].verts[j][1] = DotProduct( tmp, tagParent.axis[1] );
						pTris[t].verts[j][2] = DotProduct( tmp, tagParent.axis[2] );

						VectorCopy( pTris[t].normals[j], tmp );
						pTris[t].normals[j][0] = DotProduct( tmp, tagParent.axis[0] );
						pTris[t].normals[j][1] = DotProduct( tmp, tagParent.axis[1] );
						pTris[t].normals[j][2] = DotProduct( tmp, tagParent.axis[2] );
					}
				}
			}

			//
			// compute tag data
			//
			if ( strstr( pOAF->surfaces[i]->name, "tag_" ) == pOAF->surfaces[i]->name ) {
				md3Tag_t *pTag = &g_data.tags[f][tagcount];
				float tri[3][3];

				strcpy( pTag->name, pOAF->surfaces[i]->name );

				memcpy( tri[0], pTris[0].verts[0], sizeof( float ) * 3 );
				memcpy( tri[1], pTris[0].verts[1], sizeof( float ) * 3 );
				memcpy( tri[2], pTris[0].verts[2], sizeof( float ) * 3 );

				MD3_ComputeTagFromTri( pTag, tri );
				tagcount++;
			}
			else
			{
				if ( g_data.surfData[i].verts[f] ) {
					free( g_data.surfData[i].verts[f] );
				}
				frameXyz = g_data.surfData[i].verts[f] = calloc( 1, sizeof( float ) * 6 * g_data.surfData[i].header.numVerts );
				frameNormals = frameXyz + 3;

				for ( t = 0; t < pOAF->surfaces[i]->numtriangles; t++ )
				{
					for ( j = 0; j < 3 ; j++ )
					{
						int index;

						index = g_data.surfData[i].baseTriangles[t].v[j].index;
						frameXyz[index * 6 + 0] = pTris[t].verts[j][0];
						frameXyz[index * 6 + 1] = pTris[t].verts[j][1];
						frameXyz[index * 6 + 2] = pTris[t].verts[j][2];
						frameNormals[index * 6 + 0] =  pTris[t].normals[j][0];
						frameNormals[index * 6 + 1] =  pTris[t].normals[j][1];
						frameNormals[index * 6 + 2] =  pTris[t].normals[j][2];
						AddPointToBounds( &frameXyz[index * 6], fr->bounds[0], fr->bounds[1] );
					}
				}
			}
		}
	}

	if ( strstr( filename, gamedir + 1 ) ) {
		strcpy( g_modelname, strstr( filename, gamedir + 1 ) + strlen( gamedir ) - 1 );
	}
	else
	{
		strcpy( g_modelname, filename );
	}

	FinishModel( type );
	ClearModel();
}

static void ConvertASE( const char *filename, int type, qboolean grabAnims ){
	int i, j;
	int numSurfaces;
	int numFrames = -1;
	SurfaceAnimation_t surfaceAnimations[MAX_ANIM_SURFACES];
	ObjectAnimationFrame_t objectAnimationFrames[MAX_ANIM_FRAMES];
	char outfilename[1024];

	/*
	** load ASE into memory
	*/
	ASE_Load( filename, g_verbose, grabAnims );

	/*
	** process parts
	*/
	if ( type == TYPE_ITEM ) {
		numSurfaces = GetSurfaceAnimations( surfaceAnimations, "any", -1, -1, -1 );

		if ( numSurfaces <= 0 ) {
			Error( "numSurfaces <= 0" );
		}

		numFrames = SurfaceOrderToFrameOrder( surfaceAnimations, objectAnimationFrames, numSurfaces );

		if ( numFrames <= 0 ) {
			Error( "numFrames <= 0" );
		}

		strcpy( outfilename, filename );
		if ( strrchr( outfilename, '.' ) ) {
			*( strrchr( outfilename, '.' ) + 1 ) = 0;
		}
		strcat( outfilename, "md3" );
		BuildAnimationFromOAFs( outfilename, objectAnimationFrames, numFrames, type );

		// free memory
		for ( i = 0; i < numSurfaces; i++ )
		{
			if ( surfaceAnimations[i].frames ) {
				for ( j = 0; j < surfaceAnimations[i].numFrames; j++ )
				{
					free( surfaceAnimations[i].frames[j].triangles );
				}
				free( surfaceAnimations[i].frames );
				surfaceAnimations[i].frames = 0;
			}
		}
	}
	else if ( type == TYPE_PLAYER ) {
		qboolean tagTorso = qfalse;
		qboolean tagHead = qfalse;
		qboolean tagWeapon = qfalse;

		//
		// verify that all necessary tags exist
		//
		numSurfaces = ASE_GetNumSurfaces();
		for ( i = 0; i < numSurfaces; i++ )
		{
			if ( !strcmp( ASE_GetSurfaceName( i ), "tag_head" ) ) {
				tagHead = qtrue;
			}
			if ( !strcmp( ASE_GetSurfaceName( i ), "tag_torso" ) ) {
				tagTorso = qtrue;
			}
			if ( !strcmp( ASE_GetSurfaceName( i ), "tag_weapon" ) ) {
				tagWeapon = qtrue;
			}
		}

		if ( !tagHead ) {
			// todo: was never being checked; should we error now that it is?
			// Error( "Missing tag_head!" );
		}
		if ( !tagTorso ) {
			Error( "Missing tag_torso!" );
		}
		if ( !tagWeapon ) {
			Error( "Missing tag_weapon!" );
		}

		// get all upper body surfaces
		numSurfaces = GetSurfaceAnimations( surfaceAnimations, "u_", -1, -1, g_data.maxUpperFrames );
		numFrames = SurfaceOrderToFrameOrder( surfaceAnimations, objectAnimationFrames, numSurfaces );
		strcpy( outfilename, filename );
		if ( strrchr( outfilename, '/' ) ) {
			*( strrchr( outfilename, '/' ) + 1 ) = 0;
		}

		if ( g_data.currentLod == 0 ) {
			strcat( outfilename, "upper.md3" );
		}
		else
		{
			char temp[128];

			sprintf( temp, "upper_%d.md3", g_data.currentLod );
			strcat( outfilename, temp );
		}

		BuildAnimationFromOAFs( outfilename, objectAnimationFrames, numFrames, type );

		// free memory
		for ( i = 0; i < numSurfaces; i++ )
		{
			if ( surfaceAnimations[i].frames ) {
				for ( j = 0; j < surfaceAnimations[i].numFrames; j++ )
				{
					free( surfaceAnimations[i].frames[j].triangles );
				}
				free( surfaceAnimations[i].frames );
				surfaceAnimations[i].frames = 0;
			}
		}

		// get lower body surfaces
		numSurfaces = GetSurfaceAnimations( surfaceAnimations, "l_", g_data.lowerSkipFrameStart, g_data.lowerSkipFrameEnd, -1 );
		numFrames = SurfaceOrderToFrameOrder( surfaceAnimations, objectAnimationFrames, numSurfaces );
		strcpy( outfilename, filename );
		if ( strrchr( outfilename, '/' ) ) {
			*( strrchr( outfilename, '/' ) + 1 ) = 0;
		}

		if ( g_data.currentLod == 0 ) {
			strcat( outfilename, "lower.md3" );
		}
		else
		{
			char temp[128];

			sprintf( temp, "lower_%d.md3", g_data.currentLod );
			strcat( outfilename, temp );
		}
		BuildAnimationFromOAFs( outfilename, objectAnimationFrames, numFrames, type );

		// free memory
		for ( i = 0; i < numSurfaces; i++ )
		{
			if ( surfaceAnimations[i].frames ) {
				for ( j = 0; j < surfaceAnimations[i].numFrames; j++ )
				{
					free( surfaceAnimations[i].frames[j].triangles );
				}
				free( surfaceAnimations[i].frames );
				surfaceAnimations[i].frames = 0;
			}
		}

		// get head surfaces
		numSurfaces = GetSurfaceAnimations( surfaceAnimations, "h_", -1, -1, g_data.maxHeadFrames );
		numFrames = SurfaceOrderToFrameOrder( surfaceAnimations, objectAnimationFrames, numSurfaces );
		strcpy( outfilename, filename );
		if ( strrchr( outfilename, '/' ) ) {
			*( strrchr( outfilename, '/' ) + 1 ) = 0;
		}

		if ( g_data.currentLod == 0 ) {
			strcat( outfilename, "head.md3" );
		}
		else
		{
			char temp[128];

			sprintf( temp, "head_%d.md3", g_data.currentLod );
			strcat( outfilename, temp );
		}
		BuildAnimationFromOAFs( outfilename, objectAnimationFrames, numFrames, type );

		// free memory
		for ( i = 0; i < numSurfaces; i++ )
		{
			if ( surfaceAnimations[i].frames ) {
				for ( j = 0; j < surfaceAnimations[i].numFrames; j++ )
				{
					free( surfaceAnimations[i].frames[j].triangles );
				}
				free( surfaceAnimations[i].frames );
				surfaceAnimations[i].frames = 0;
			}
		}
	}
	else if ( type == TYPE_WEAPON ) {
		// get the weapon surfaces
		numSurfaces = GetSurfaceAnimations( surfaceAnimations, "w_", -1, -1, -1 );
		numFrames = SurfaceOrderToFrameOrder( surfaceAnimations, objectAnimationFrames, numSurfaces );

		strcpy( outfilename, filename );
		if ( strrchr( outfilename, '.' ) ) {
			*( strrchr( outfilename, '.' ) + 1 ) = 0;
		}
		strcat( outfilename, "md3" );
		BuildAnimationFromOAFs( outfilename, objectAnimationFrames, numFrames, type );

		// free memory
		for ( i = 0; i < numSurfaces; i++ )
		{
			if ( surfaceAnimations[i].frames ) {
				for ( j = 0; j < surfaceAnimations[i].numFrames; j++ )
				{
					free( surfaceAnimations[i].frames[j].triangles );
				}
				free( surfaceAnimations[i].frames );
				surfaceAnimations[i].frames = 0;
			}
		}

		// get the flash surfaces
		numSurfaces = GetSurfaceAnimations( surfaceAnimations, "f_", -1, -1, -1 );
		numFrames = SurfaceOrderToFrameOrder( surfaceAnimations, objectAnimationFrames, numSurfaces );

		strcpy( outfilename, filename );
		if ( strrchr( outfilename, '.' ) ) {
			*strrchr( outfilename, '.' ) = 0;
		}
		strcat( outfilename, "_flash.md3" );
		BuildAnimationFromOAFs( outfilename, objectAnimationFrames, numFrames, TYPE_ITEM );

		// free memory
		for ( i = 0; i < numSurfaces; i++ )
		{
			if ( surfaceAnimations[i].frames ) {
				for ( j = 0; j < surfaceAnimations[i].numFrames; j++ )
				{
					free( surfaceAnimations[i].frames[j].triangles );
				}
				free( surfaceAnimations[i].frames );
				surfaceAnimations[i].frames = 0;
			}
		}
	}
	else if ( type == TYPE_HAND ) {
		// get the hand tags
		numSurfaces = GetSurfaceAnimations( surfaceAnimations, "tag_", -1, -1, -1 );
		numFrames = SurfaceOrderToFrameOrder( surfaceAnimations, objectAnimationFrames, numSurfaces );

		strcpy( outfilename, filename );
		if ( strrchr( outfilename, '.' ) ) {
			*strrchr( outfilename, '.' ) = 0;
		}
		strcat( outfilename, "_hand.md3" );
		BuildAnimationFromOAFs( outfilename, objectAnimationFrames, numFrames, TYPE_HAND );

		// free memory
		for ( i = 0; i < numSurfaces; i++ )
		{
			if ( surfaceAnimations[i].frames ) {
				for ( j = 0; j < surfaceAnimations[i].numFrames; j++ )
				{
					free( surfaceAnimations[i].frames[j].triangles );
				}
				free( surfaceAnimations[i].frames );
				surfaceAnimations[i].frames = 0;
			}
		}
	}
	else
	{
		Error( "Unknown type passed to ConvertASE()" );
	}

	g_data.currentLod = 0;
	g_data.lodBias = 0;
	g_data.maxHeadFrames = 0;
	g_data.maxUpperFrames = 0;
	g_data.lowerSkipFrameStart = 0;
	g_data.lowerSkipFrameEnd = 0;
	VectorCopy( vec3_origin, g_data.aseAdjust );

	// unload ASE from memory
	ASE_Free();
}
