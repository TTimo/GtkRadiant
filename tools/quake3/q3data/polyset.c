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

polyset_t *Polyset_SplitSets( polyset_t *psets, int numpolysets, int *pNumNewPolysets, int maxTris ){
	int p, np, op;
	int numNewPolysets = 0;
	int numSplitPolysets = 0;
	polyset_t *newpsets;
	int sumTriangles = 0;

	for ( p = 0; p < numpolysets; p++ )
	{
		numNewPolysets += psets[p].numtriangles / maxTris + 1;
	}

	if ( numNewPolysets == numpolysets ) {
		return psets;
	}

	printf( "Warning: creating %d polysets from input of %d polysets\n", numNewPolysets, numpolysets );

	newpsets = calloc( sizeof( polyset_t ) * numNewPolysets, 1 );

	for ( np = 0, op = 0; op < numpolysets; op++ )
	{
		numSplitPolysets = ( psets[op].numtriangles / ( maxTris + 1 ) ) + 1;
		if (  numSplitPolysets == 1 ) {
			memcpy( &newpsets[np], &psets[op], sizeof( polyset_t ) );
			np++;
		}
		else
		{
			sumTriangles = 0;

			// split this pset into multiple smaller psets
			for ( p = 0; p < numSplitPolysets; p++, np++ )
			{
				memcpy( &newpsets[np], &psets[op], sizeof( polyset_t ) );

				newpsets[np].triangles = psets[op].triangles + sumTriangles;

				if ( sumTriangles + maxTris > psets[op].numtriangles ) {
					newpsets[np].numtriangles = psets[op].numtriangles - sumTriangles;
				}
				else{
					newpsets[np].numtriangles = maxTris;
				}

				sumTriangles += newpsets[np].numtriangles;
			}
		}
	}

	*pNumNewPolysets = numNewPolysets;

	return newpsets;
}

polyset_t *Polyset_LoadSets( const char *file, int *numpolysets, int maxTrisPerSet ){
	polyset_t *psets;
	polyset_t *finalpsets;

	//
	// load the frame
	//
	if ( strstr( file, ".3DS" ) || strstr( file, ".3ds" ) ) {
		_3DS_LoadPolysets( file, &psets, numpolysets, g_verbose );
	}
	else{
		Error( "TRI files no longer supported" );
		return NULL;
	}
//		TRI_LoadPolysets( file, &psets, numpolysets );

/*
    //
    // scale polysets
    //
    for ( i = 0; i < psets; i++ )
    {
        int j;

        for ( j = 0; j < psets[i].numtriangles; j++ )
        {
        }
    }
 */

	//
	// split polysets if necessary
	//
	finalpsets = Polyset_SplitSets( psets, *numpolysets, numpolysets, maxTrisPerSet );

	return finalpsets;
}

polyset_t *Polyset_CollapseSets( polyset_t *psets, int numpolysets ){
	int p;
	int sumtriangles = 0;

	polyset_t *oldpsets = psets;

	//
	// no tag checking because this is an $oldbase and thus shouldn't have any
	// tags
	//
	for ( p = 0; p < numpolysets; p++ )
	{
		sumtriangles += oldpsets[p].numtriangles;
	}

	psets = calloc( 1, sizeof( polyset_t ) );
	psets[0].numtriangles = sumtriangles;
	psets[0].triangles = malloc( MD3_MAX_TRIANGLES * sizeof( triangle_t ) );

	// each call to "LoadPolysets" only allocates a single large chunk of
	// triangle memory that is utilized by all the polysets loaded by
	// that one call
	memcpy( psets[0].triangles, oldpsets[0].triangles, sizeof( triangle_t ) * sumtriangles );

	free( oldpsets[0].triangles );
	free( oldpsets );

	return psets;
}

static float SnapFloat( float x ){
	int ix;

	x *= 1.0f / MD3_XYZ_SCALE;
	ix = ( int ) x;
	x = ( float ) ix;
	x *= MD3_XYZ_SCALE;

	return x;
}

void Polyset_SnapSets( polyset_t *psets, int numpolysets ){
	int p;

	for ( p = 0; p < numpolysets; p++ )
	{
		int t;

		for ( t = 0; t < psets[p].numtriangles; t++ )
		{
			int v;

			for ( v = 0; v < 3; v++ )
			{
				psets[p].triangles[t].verts[v][0] = SnapFloat( psets[p].triangles[t].verts[v][0] );
				psets[p].triangles[t].verts[v][1] = SnapFloat( psets[p].triangles[t].verts[v][1] );
				psets[p].triangles[t].verts[v][2] = SnapFloat( psets[p].triangles[t].verts[v][2] );
			}
		}
	}
}

void Polyset_ComputeNormals( polyset_t *psets, int numpolysets ){
	int p;
	int i, t;
	int vertexIndex[MD3_MAX_TRIANGLES][3];
	vec3_t verts[MD3_MAX_VERTS];
	vec3_t normals[MD3_MAX_VERTS];
	vec3_t faceNormals[MD3_MAX_TRIANGLES];

	//
	// iterate through polysets
	//
	for ( p = 0; p < numpolysets; p++ )
	{
		int numUniqueVertices = 0;

		assert( psets[p].numtriangles < MD3_MAX_TRIANGLES );

		memset( vertexIndex, 0xff, sizeof( vertexIndex ) );
		memset( verts, 0, sizeof( verts ) );
		memset( normals, 0, sizeof( normals ) );

		//
		// unique vertices
		//
		for ( t = 0; t < psets[p].numtriangles; t++ )
		{
			int j;

			for ( j = 0; j < 3; j++ )
			{
				for ( i = 0; i < numUniqueVertices; i++ )
				{
					if ( VectorCompare( psets[p].triangles[t].verts[j], verts[i] ) ) {
						break;
					}
				}
				if ( i == numUniqueVertices ) {
					vertexIndex[t][j] = numUniqueVertices;
					VectorCopy( ( psets[p].triangles[t].verts[j] ), ( verts[numUniqueVertices] ) );
					numUniqueVertices++;
				}
				else
				{
					vertexIndex[t][j] = i;
				}
			}
		}

		//
		// compute face normals
		//
		for ( t = 0; t < psets[p].numtriangles; t++ )
		{
			vec3_t side0, side1, facenormal;

			VectorSubtract( psets[p].triangles[t].verts[0], psets[p].triangles[t].verts[1], side0 );
			VectorSubtract( psets[p].triangles[t].verts[2], psets[p].triangles[t].verts[1], side1 );

			CrossProduct( side0, side1, facenormal );
			VectorNormalize( facenormal, faceNormals[t] );
		}

		//
		// sum normals and copy them back
		//
		for ( i = 0; i < numUniqueVertices; i++ )
		{
			for ( t = 0; t < psets[p].numtriangles; t++ )
			{
				if ( vertexIndex[t][0] == i ||
					 vertexIndex[t][1] == i ||
					 vertexIndex[t][2] == i ) {
					normals[i][0] += faceNormals[t][0];
					normals[i][1] += faceNormals[t][1];
					normals[i][2] += faceNormals[t][2];
				}
			}
			VectorNormalize( normals[i], normals[i] );
		}


		for ( t = 0; t < psets[p].numtriangles; t++ )
		{
			VectorCopy( normals[vertexIndex[t][0]], psets[p].triangles[t].normals[0] );
			VectorCopy( normals[vertexIndex[t][1]], psets[p].triangles[t].normals[1] );
			VectorCopy( normals[vertexIndex[t][2]], psets[p].triangles[t].normals[2] );
		}
	}
}
