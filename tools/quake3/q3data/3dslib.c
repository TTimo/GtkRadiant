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

static void Load3DS( const char *filename, _3DS_t *p3DS, qboolean verbose );

static qboolean s_verbose;

#define MAX_MATERIALS 100
#define MAX_NAMED_OBJECTS 100
#define MAX_MESH_MATERIAL_GROUPS 100
#define MAX_TRI_OBJECTS 512

static char s_buffer[1000000];

static int ReadString( FILE *fp, char *buffer ){
	int i = 0;
	int bytesRead = 0;

	do
	{
		fread( &buffer[i], 1, sizeof( char ), fp );
		bytesRead++;
	} while ( buffer[i++] != 0 );
	buffer[i] = 0;

	return bytesRead;
}

static int ReadChunkAndLength( FILE *fp, unsigned short *chunk, long *len ){
	if ( fread( chunk, sizeof( short ), 1, fp ) != 1 ) {
		return 0;
	}
	if ( fread( len, sizeof( long ), 1, fp ) != 1 ) {
		Error( "Unexpected EOF found" );
	}
	return 1;
}

static void LoadMapName( FILE *fp, char *buffer, int thisChunkLen ){
	unsigned short chunkID;
	long chunkLen;
	long bytesRead = 0;

	while ( ReadChunkAndLength( fp, &chunkID, &chunkLen ) )
	{
		switch ( chunkID )
		{
		case _3DS_CHUNK_MAT_MAPNAME:
			fread( buffer, chunkLen - 6, 1, fp );
			break;
		default:
			fread( s_buffer, chunkLen - 6, 1, fp );
			break;
		}
		bytesRead += chunkLen;
		if ( bytesRead >= thisChunkLen ) {
			return;
		}
	}
}

static void LoadMaterialList( FILE *fp, long thisChunkLen, _3DSMaterial_t *pMat ){
	long chunkLen;
	unsigned short chunkID;
	long bytesRead = 0;
	_3DSMaterial_t mat;
	char curdir[1024];
	char buffer[2048];

	memset( &mat, 0, sizeof( mat ) );

	if ( s_verbose ) {
		printf( "    >>> MATERIAL LIST\n" );
	}

	while ( ReadChunkAndLength( fp, &chunkID, &chunkLen ) )
	{
		switch ( chunkID )
		{
		case _3DS_CHUNK_MAT_NAME:
			fread( mat.name, chunkLen - 6, 1, fp );
			if ( s_verbose ) {
				printf( "        found mat name '%s'\n", mat.name );
			}
			break;
		case _3DS_CHUNK_TEXMAP:
			LoadMapName( fp, mat.texture, chunkLen - 6 );
			if ( s_verbose ) {
				printf( "        found texture '%s'\n", mat.texture );
			}
			break;
		case _3DS_CHUNK_SPECMAP:
			LoadMapName( fp, mat.specular, chunkLen - 6 );
			if ( s_verbose ) {
				printf( "        found specular map '%s'\n", mat.specular );
			}
			break;
		case _3DS_CHUNK_OPACMAP:
			LoadMapName( fp, mat.opacity, chunkLen - 6 );
			if ( s_verbose ) {
				printf( "        found opacity map '%s'\n", mat.opacity );
			}
			break;
		case _3DS_CHUNK_REFLMAP:
			LoadMapName( fp, mat.reflection, chunkLen - 6 );
			if ( s_verbose ) {
				printf( "        found reflection map '%s'\n", mat.reflection );
			}
			break;
		case _3DS_CHUNK_BUMPMAP:
			LoadMapName( fp, mat.bump, chunkLen - 6 );
			if ( s_verbose ) {
				printf( "        found bump map '%s'\n", mat.bump );
			}
			break;
		default:
			fread( s_buffer, chunkLen - 6, 1, fp );
			break;
		}

		bytesRead += chunkLen;

		if ( bytesRead >= thisChunkLen ) {
			break;
		}
	}

	Q_getwd( curdir );

	if ( mat.texture[0] ) {
		sprintf( buffer, "%s%s", curdir, mat.texture );
		if ( strstr( buffer, gamedir + 1 ) ) {
			strcpy( mat.texture, strstr( buffer, gamedir + 1 ) + strlen( gamedir ) - 1 );
		}
		else{
			strcpy( mat.texture, buffer );
		}
	}

	if ( mat.specular[0] ) {
		sprintf( buffer, "%s%s", curdir, mat.specular );
		if ( strstr( buffer, gamedir + 1 ) ) {
			strcpy( mat.specular, strstr( buffer, gamedir + 1 ) + strlen( gamedir ) - 1 );
		}
		else{
			strcpy( mat.specular, buffer );
		}
	}

	if ( mat.bump[0] ) {
		sprintf( buffer, "%s%s", curdir, mat.bump );
		if ( strstr( buffer, gamedir + 1 ) ) {
			strcpy( mat.bump, strstr( buffer, gamedir + 1 ) + strlen( gamedir ) - 1 );
		}
		else{
			strcpy( mat.bump, buffer );
		}
	}

	if ( mat.reflection[0] ) {
		sprintf( buffer, "%s%s", curdir, mat.reflection );
		if ( strstr( buffer, gamedir + 1 ) ) {
			strcpy( mat.reflection, strstr( buffer, gamedir + 1 ) + strlen( gamedir ) - 1 );
		}
		else{
			strcpy( mat.reflection, buffer );
		}
	}

	if ( mat.opacity[0] ) {
		sprintf( buffer, "%s%s", curdir, mat.opacity );
		if ( strstr( buffer, gamedir + 1 ) ) {
			strcpy( mat.opacity, strstr( buffer, gamedir + 1 ) + strlen( gamedir ) - 1 );
		}
		else{
			strcpy( mat.opacity, buffer );
		}
	}

	*pMat = mat;
}

static void LoadMeshMaterialGroup( FILE *fp, long thisChunkLen, _3DSMeshMaterialGroup_t *pMMG ){
	_3DSMeshMaterialGroup_t mmg;

	memset( &mmg, 0, sizeof( mmg ) );

	ReadString( fp, mmg.name );

	fread( &mmg.numFaces, sizeof( mmg.numFaces ), 1, fp );
	mmg.pFaces = malloc( sizeof( mmg.pFaces[0] ) * mmg.numFaces );
	fread( mmg.pFaces, sizeof( mmg.pFaces[0] ), mmg.numFaces, fp );

	if ( s_verbose ) {
		printf( "    >>> MESH MATERIAL GROUP '%s' (%d faces)\n", mmg.name, mmg.numFaces );

		{
			int i;

			for ( i = 0; i < mmg.numFaces; i++ )
			{
				printf( "        %d\n", mmg.pFaces[i] );
			}
		}
	}

	*pMMG = mmg;
}

static void LoadNamedTriObject( FILE *fp, long thisChunkLen, _3DSTriObject_t *pTO ){
	long chunkLen;
	unsigned short chunkID;
	int i = 0;
	long bytesRead = 0;
	_3DSTriObject_t triObj;
	_3DSMeshMaterialGroup_t meshMaterialGroups[MAX_MESH_MATERIAL_GROUPS];
	int numMeshMaterialGroups = 0;

	memset( &triObj, 0, sizeof( triObj ) );

	if ( s_verbose ) {
		printf( "        >>> NAMED TRI OBJECT\n" );
	}

	while ( ReadChunkAndLength( fp, &chunkID, &chunkLen ) )
	{
		switch ( chunkID )
		{
		case _3DS_CHUNK_MSH_MAT_GROUP:
			LoadMeshMaterialGroup( fp, chunkLen - 6, &meshMaterialGroups[numMeshMaterialGroups] );
			bytesRead += chunkLen;
			numMeshMaterialGroups++;
			break;
		case _3DS_CHUNK_FACE_ARRAY:
			fread( &triObj.numFaces, sizeof( triObj.numFaces ), 1, fp );
			assert( triObj.pFaces == 0 );

			triObj.pFaces = malloc( sizeof( triObj.pFaces[0] ) * triObj.numFaces );
			fread( triObj.pFaces, sizeof( triObj.pFaces[0] ), triObj.numFaces, fp );
			bytesRead += sizeof( triObj.numFaces ) + triObj.numFaces * sizeof( triObj.pFaces[0] ) + 6;

			if ( s_verbose ) {
				printf( "            found face array with %d faces\n", triObj.numFaces );
				for ( i = 0; i < triObj.numFaces; i++ )
				{
					printf( "                %d: %d,%d,%d\n", i, triObj.pFaces[i].a, triObj.pFaces[i].b, triObj.pFaces[i].c );
				}
			}

			break;
		case _3DS_CHUNK_POINT_ARRAY:
			fread( &triObj.numPoints, sizeof( triObj.numPoints ), 1, fp );
			triObj.pPoints = malloc( sizeof( triObj.pPoints[0] ) * triObj.numPoints );
			fread( triObj.pPoints, sizeof( triObj.pPoints[0] ), triObj.numPoints, fp );
			bytesRead += sizeof( triObj.numPoints ) + triObj.numPoints * sizeof( triObj.pPoints[0] ) + 6;

			// flip points around into our coordinate system
			for ( i = 0; i < triObj.numPoints; i++ )
			{
				float x, y, z;

				x = triObj.pPoints[i].x;
				y = triObj.pPoints[i].y;
				z = triObj.pPoints[i].z;

				triObj.pPoints[i].x = -y;
				triObj.pPoints[i].y = x;
				triObj.pPoints[i].z = z;
			}

			if ( s_verbose ) {
				printf( "            found point array with %d points\n", triObj.numPoints );
				for ( i = 0; i < triObj.numPoints; i++ )
				{
					printf( "                %d: %f,%f,%f\n", i, triObj.pPoints[i].x, triObj.pPoints[i].y, triObj.pPoints[i].z );
				}
			}
			break;
		case _3DS_CHUNK_TEX_VERTS:
			fread( &triObj.numTexVerts, sizeof( triObj.numTexVerts ), 1, fp );
			triObj.pTexVerts = malloc( sizeof( triObj.pTexVerts[0] ) * triObj.numTexVerts );
			fread( triObj.pTexVerts, sizeof( triObj.pTexVerts[0] ), triObj.numTexVerts, fp );
			bytesRead += sizeof( triObj.numTexVerts ) + sizeof( triObj.pTexVerts[0] ) * triObj.numTexVerts + 6;

			if ( s_verbose ) {
				printf( "            found tex vert array with %d tex verts\n", triObj.numTexVerts );
				for ( i = 0; i < triObj.numTexVerts; i++ )
				{
					printf( "                %d: %f,%f\n", i, triObj.pTexVerts[i].s, triObj.pTexVerts[i].t );
				}
			}
			break;
		default:
			fread( s_buffer, chunkLen - 6, 1, fp );
			bytesRead += chunkLen;
			break;
		}

		if ( bytesRead >= thisChunkLen ) {
			break;
		}
	}
	*pTO = triObj;

	if ( numMeshMaterialGroups == 0 ) {
		numMeshMaterialGroups = 1;
		strcpy( meshMaterialGroups[0].name, "(null)" );
		if ( pTO->numTexVerts ) {
			printf( "Warning: assigning (null) skin to tri object\n" );
		}
	}
	else
	{
		assert( pTO->numFaces == meshMaterialGroups[0].numFaces );
	}

	pTO->pMeshMaterialGroups = malloc( sizeof( _3DSMeshMaterialGroup_t ) * numMeshMaterialGroups );
	memcpy( pTO->pMeshMaterialGroups, meshMaterialGroups, numMeshMaterialGroups * sizeof( meshMaterialGroups[0] ) );
	pTO->numMeshMaterialGroups = numMeshMaterialGroups;

	//
	// sanity checks
	//
	assert( numMeshMaterialGroups <= 1 );
}

static void LoadNamedObject( FILE *fp, long thisChunkLen, _3DSNamedObject_t *pNO ){
	long chunkLen;
	unsigned short chunkID;
	int i = 0;
	long bytesRead = 0;
	char name[100];
	_3DSTriObject_t triObj[MAX_TRI_OBJECTS];
	int numTriObjects = 0;

	memset( triObj, 0, sizeof( triObj ) );

	bytesRead += ReadString( fp, name );

	if ( s_verbose ) {
		printf( "    >>> NAMED OBJECT '%s'\n", name );
	}

	while ( ReadChunkAndLength( fp, &chunkID, &chunkLen ) )
	{
		switch ( chunkID )
		{
		case _3DS_CHUNK_NAMED_TRI_OBJECT:
			LoadNamedTriObject( fp, chunkLen - 6, &triObj[numTriObjects] );
			numTriObjects++;
			break;
		default:
			fread( s_buffer, chunkLen - 6, 1, fp );
			break;
		}

		bytesRead += chunkLen;

		if ( bytesRead >= thisChunkLen ) {
			break;
		}
	}

	strcpy( pNO->name, name );
	pNO->pTriObjects = malloc( sizeof( _3DSTriObject_t ) * numTriObjects );
	memcpy( pNO->pTriObjects, triObj, sizeof( triObj[0] ) * numTriObjects );
	pNO->numTriObjects = numTriObjects;

	assert( numTriObjects <= 1 );
}

static void LoadEditChunk( FILE *fp, long thisChunkLen, _3DSEditChunk_t *pEC ){
	unsigned short chunkID;
	long chunkLen;
	long bytesRead = 0;
	_3DSEditChunk_t editChunk;

	_3DSMaterial_t mat[MAX_MATERIALS];
	_3DSNamedObject_t namedObjects[MAX_NAMED_OBJECTS];

	int numMaterials = 0, numNamedObjects = 0;

	memset( &editChunk, 0, sizeof( editChunk ) );

	if ( s_verbose ) {
		printf( ">>> EDIT CHUNK\n" );
	}

	while ( ReadChunkAndLength( fp, &chunkID, &chunkLen ) )
	{
		switch ( chunkID )
		{
		case _3DS_CHUNK_MAT_LIST:
			LoadMaterialList( fp, chunkLen - 6, &mat[numMaterials] );
			numMaterials++;
			break;
		case _3DS_CHUNK_NAMED_OBJECT:
			LoadNamedObject( fp, chunkLen - 6, &namedObjects[numNamedObjects] );
			if ( namedObjects[numNamedObjects].numTriObjects != 0 ) {
				++numNamedObjects;
			}
			break;
		case _3DS_CHUNK_MESH_VERSION:
		default:
			fread( s_buffer, chunkLen - 6, 1, fp );
			break;
		}

		bytesRead += chunkLen;

		if ( bytesRead >= thisChunkLen ) {
			break;
		}
	}

	if ( numMaterials == 0 ) {
		numMaterials = 1;
		strcpy( mat[0].name, "(null)" );
		printf( "Warning: no material definitions found\n" );
	}

	pEC->numNamedObjects = numNamedObjects;

	pEC->pMaterials = malloc( sizeof( _3DSMaterial_t ) * numMaterials );
	pEC->pNamedObjects = malloc( sizeof( _3DSNamedObject_t ) * numNamedObjects );

	memcpy( pEC->pMaterials, mat, numMaterials * sizeof( mat[0] ) );
	memcpy( pEC->pNamedObjects, namedObjects, numNamedObjects * sizeof( namedObjects[0] ) );
}

static void Load3DS( const char *filename, _3DS_t *p3DS, qboolean verbose ){
	FILE *fp;
	unsigned short chunkID;
	long chunkLen;
	_3DSEditChunk_t editChunk;

	s_verbose = verbose;

	if ( ( fp = fopen( filename, "rb" ) ) == 0 ) {
		Error( "Unable to open '%s'", filename );
	}

	// read magic number
	if ( ( fread( &chunkID, sizeof( short ), 1, fp ) != 1 ) ||
		 ( LittleShort( chunkID ) != _3DS_CHUNK_MAGIC ) ) {
		Error( "Missing or incorrect magic number in '%s'", filename );
	}
	if ( fread( &chunkLen, sizeof( chunkLen ), 1, fp ) != 1 ) {
		Error( "Unexpected EOF encountered in '%s'", filename );
	}
	// version number
	if ( !ReadChunkAndLength( fp, &chunkID, &chunkLen ) ) {
		Error( "Missing version number in '%s'", filename );
	}
	if ( fread( s_buffer, chunkLen - 6, 1, fp ) != 1 ) {
		Error( "Unexpected EOF encountered in '%s'", filename );
	}

	while ( ReadChunkAndLength( fp, &chunkID, &chunkLen ) )
	{
		switch ( chunkID )
		{
		case _3DS_CHUNK_EDIT:
			LoadEditChunk( fp, chunkLen - 6, &editChunk );
			break;
		case _3DS_CHUNK_KEYFRAME_DATA:
			fread( s_buffer, chunkLen - 6, 1, fp );
			break;
		default:
			fread( s_buffer, chunkLen - 6, 1, fp );
			break;
		}
	}

	fclose( fp );

	p3DS->editChunk = editChunk;
}

static void ComputeNormals( _3DSTriObject_t *pTO, triangle_t *pTris ){
	vec3_t faceNormals[POLYSET_MAXTRIANGLES];
	vec3_t vertexNormals[POLYSET_MAXTRIANGLES * 3];
	vec3_t side0, side1, facenormal;
	int f, v;

	memset( faceNormals, 0, sizeof( faceNormals ) );
	memset( vertexNormals, 0, sizeof( vertexNormals ) );

	//
	// compute face normals
	//
	for ( f = 0; f < pTO->numFaces; f++ )
	{
		VectorSubtract( pTris[f].verts[0], pTris[f].verts[1], side0 );
		VectorSubtract( pTris[f].verts[2], pTris[f].verts[1], side1 );

		CrossProduct( side0, side1, facenormal );
		VectorNormalize( facenormal, faceNormals[f] );
	}

	//
	// sum vertex normals
	//
	for ( v = 0; v < pTO->numPoints; v++ )
	{
		for ( f = 0; f < pTO->numFaces; f++ )
		{
			if ( ( pTO->pFaces[f].a == v ) ||
				 ( pTO->pFaces[f].b == v ) ||
				 ( pTO->pFaces[f].c == v ) ) {
				vertexNormals[v][0] += faceNormals[f][0];
				vertexNormals[v][1] += faceNormals[f][1];
				vertexNormals[v][2] += faceNormals[f][2];
			}
		}

		VectorNormalize( vertexNormals[v], vertexNormals[v] );
	}

	//
	// copy vertex normals into triangles
	//
	for ( f = 0; f < pTO->numFaces; f++ )
	{
		int i0 = pTO->pFaces[f].c;
		int i1 = pTO->pFaces[f].b;
		int i2 = pTO->pFaces[f].a;

		VectorCopy( vertexNormals[i0], pTris[f].normals[0] );
		VectorCopy( vertexNormals[i1], pTris[f].normals[1] );
		VectorCopy( vertexNormals[i2], pTris[f].normals[2] );
	}
}

/*
** void _3DS_LoadPolysets
*/
void _3DS_LoadPolysets( const char *filename, polyset_t **ppPSET, int *numpsets, qboolean verbose ){
	_3DS_t _3ds;
	int numPolysets;
	polyset_t *pPSET;
	triangle_t *ptri, *triangles;
	int i;

	// load the 3DS
	memset( &_3ds, 0, sizeof( _3ds ) );
	Load3DS( filename, &_3ds, verbose );

	// compute information
	numPolysets = _3ds.editChunk.numNamedObjects;

	// allocate memory
	pPSET = calloc( 1, numPolysets * sizeof( polyset_t ) );
	triangles = ptri = calloc( 1, POLYSET_MAXTRIANGLES * sizeof( triangle_t ) );

	// copy the data over
	for ( i = 0; i < numPolysets; i++ )
	{
		char matnamebuf[1024];
		int j;
		triangle_t *tri;
		_3DSTriObject_t *pTO = &_3ds.editChunk.pNamedObjects[i].pTriObjects[0];

		pPSET[i].triangles = ptri;
		pPSET[i].numtriangles = pTO->numFaces;
		strcpy( pPSET[i].name, _3ds.editChunk.pNamedObjects[i].name );

		strcpy( matnamebuf, filename );
		if ( strrchr( matnamebuf, '/' ) ) {
			*( strrchr( matnamebuf, '/' ) + 1 ) = 0;
		}
		strcat( matnamebuf, pTO->pMeshMaterialGroups[0].name );

		if ( strstr( matnamebuf, gamedir ) ) {
			strcpy( pPSET[i].materialname, strstr( matnamebuf, gamedir ) + strlen( gamedir ) );
		}
		else{
			strcpy( pPSET[i].materialname, pTO->pMeshMaterialGroups[0].name );
		}

		assert( pPSET[i].numtriangles < POLYSET_MAXTRIANGLES );

		for ( tri = ptri, j = 0; j < pPSET[i].numtriangles; j++ )
		{
			int i0 = pTO->pFaces[j].c;
			int i1 = pTO->pFaces[j].b;
			int i2 = pTO->pFaces[j].a;

			tri->verts[0][0] = pTO->pPoints[i0].x;
			tri->verts[0][1] = pTO->pPoints[i0].y;
			tri->verts[0][2] = pTO->pPoints[i0].z;

			tri->verts[1][0] = pTO->pPoints[i1].x;
			tri->verts[1][1] = pTO->pPoints[i1].y;
			tri->verts[1][2] = pTO->pPoints[i1].z;

			tri->verts[2][0] = pTO->pPoints[i2].x;
			tri->verts[2][1] = pTO->pPoints[i2].y;
			tri->verts[2][2] = pTO->pPoints[i2].z;
/*
            for ( k = 0; k < 3; k++ )
            {
                tri->colors[0][k] = 1;
                tri->colors[1][k] = 1;
                tri->colors[2][k] = 1;
            }
 */

			if ( pTO->pTexVerts ) {
				tri->texcoords[0][0] = pTO->pTexVerts[i0].s;
				tri->texcoords[0][1] = 1.0f - pTO->pTexVerts[i0].t;
				tri->texcoords[1][0] = pTO->pTexVerts[i1].s;
				tri->texcoords[1][1] = 1.0f - pTO->pTexVerts[i1].t;
				tri->texcoords[2][0] = pTO->pTexVerts[i2].s;
				tri->texcoords[2][1] = 1.0f - pTO->pTexVerts[i2].t;
			}

			tri++;
		}

		ptri += pPSET[i].numtriangles;
		assert( ptri - triangles < POLYSET_MAXTRIANGLES );
	}

	// compute normal data
#if 0
	for ( i = 0; i < numPolysets; i++ )
	{
		// unique vertices based solely on vertex position
		ComputeNormals( &_3ds.editChunk.pNamedObjects[i].pTriObjects[0],
						pPSET[i].triangles );
	}
#endif

	free( _3ds.editChunk.pMaterials );
	free( _3ds.editChunk.pNamedObjects );

	*ppPSET = pPSET;
	*numpsets = numPolysets;
}
