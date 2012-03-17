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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int s_used[8192];        // same as MD3_MAX_TRIANGLES

/*
** FindNextTriangleInStrip
**
** Given a surface and triangle this tries to find the next triangle
** in the strip that would continue the strip.  The next triangle in
** the strip should have the same winding as this triangle.
*/
static int FindNextTriangleInStripOrFan( int mesh[][3], int tri, int orientation, int numTris, int odd ){
	int t;
	int sum = 0;
	int currentTri[3];
	int side;
	int a, b, c;
	int refa, refb;

	currentTri[0] = mesh[tri][( 0 + orientation ) % 3];
	currentTri[1] = mesh[tri][( 1 + orientation ) % 3];
	currentTri[2] = mesh[tri][( 2 + orientation ) % 3];

	if ( odd ) {
		refa = currentTri[1];
		refb = currentTri[2];
	}
	else
	{
		refa = currentTri[2];
		refb = currentTri[0];
	}

	// go through all triangles and look for sides that match
	// this triangle's
	for ( t = 0; t < numTris; t++ )
	{
		// don't check against self or against previously used triangles
		if ( t == tri ) {
			continue;
		}
		if ( s_used[t] ) {
			continue;
		}

		// check all three sides of the candidate triangle
		for ( side = 0; side < 3; side++ )
		{
			// check only the second (abutting) side
			if ( ( refa == mesh[t][( side + 1 ) % 3] ) &&
				 ( refb == mesh[t][side] ) ) {

				a = mesh[t][0];
				b = mesh[t][1];
				c = mesh[t][2];

				// rotate the candidate triangle to align it properly in the strip
				if ( side == 1 ) {
					mesh[t][0] = b;
					mesh[t][1] = c;
					mesh[t][2] = a;
				}
				else if ( side == 2 ) {
					mesh[t][0] = c;
					mesh[t][1] = a;
					mesh[t][2] = b;
				}

				return t;
			}
/*
            else
            {
                Error( "fans not implemented yet" );

                // check only the third (abutting) side
                if ( ( currentTri[2] == pSurf->baseTriangles[t].v[side].index ) &&
                    ( currentTri[0] == pSurf->baseTriangles[t].v[(side+1)%3].index ) )
                {
                    return t;
                }
            }
 */
		}
	}

	return -1;
}

/*
** StripLength
*/
static int StripLength( int mesh[][3], int strip[][3], int tri, int orientation, int numInputTris, int fillNo ){
	int stripIndex = 0;
	int next;

	int odd = 1;

	strip[stripIndex][0] = mesh[tri][( 0 + orientation ) % 3];
	strip[stripIndex][1] = mesh[tri][( 1 + orientation ) % 3];
	strip[stripIndex][2] = mesh[tri][( 2 + orientation ) % 3];
	s_used[tri] = fillNo;
	stripIndex++;

	next = tri;

	while ( ( next = FindNextTriangleInStripOrFan( mesh, next, orientation, numInputTris, odd ) ) != -1 )
	{
		s_used[next] = fillNo;
		odd = !odd;
		strip[stripIndex][0] = mesh[next][0];
		strip[stripIndex][1] = mesh[next][1];
		strip[stripIndex][2] = mesh[next][2];
		stripIndex++;

		// all iterations after first need to be with an unrotated reference triangle
		orientation = 0;
	}

	return stripIndex;
}

/*
** BuildOptimizedList
**
** Attempts to build the longest strip/fan possible.  Does not adhere
** to pure strip or fan, will intermix between the two so long as some
** type of connectivity can be maintained.
*/
#define MAX_ORIENTATIONS        3
#define MAX_MATCHED_SIDES       4
#define MAX_SEED_TRIANGLES      16

static int BuildOptimizedList( int mesh[][3], int strip[][3], int numInputTris ){
	int t;
	int stripLen = 0;
	int startTri = -1;
	int bestTri = -1, bestLength = 0, bestOrientation = -1;
	int matchedSides = 0;
	int orientation = 0;
	int seedTriangles[MAX_MATCHED_SIDES][MAX_SEED_TRIANGLES];
	int seedLengths[MAX_ORIENTATIONS][MAX_MATCHED_SIDES][MAX_SEED_TRIANGLES];
	int numSeeds[MAX_MATCHED_SIDES] = { 0, 0, 0 };
	int i;

	// build a ranked list of candidate seed triangles based on
	// number of offshoot strips.  Precedence goes to orphans,
	// then corners, then edges, and interiors.
	memset( seedTriangles, 0xff, sizeof( seedTriangles ) );
	memset( seedLengths, 0xff, sizeof( seedLengths ) );

	for ( i = 0; i < MAX_MATCHED_SIDES; i++ )
	{
		// find the triangle with lowest number of child strips
		for ( t = 0; t < numInputTris; t++ )
		{
			int orientation;
			int n;

			if ( s_used[t] ) {
				continue;
			}

			// try the candidate triangle in three different orientations
			matchedSides = 0;
			for ( orientation = 0; orientation < 3; orientation++ )
			{
				if ( ( n = FindNextTriangleInStripOrFan( mesh, t, orientation, numInputTris, 1 ) ) != -1 ) {
					matchedSides++;
				}
			}

			if ( matchedSides == i ) {
				seedTriangles[i][numSeeds[i]] = t;
				numSeeds[i]++;
				if ( numSeeds[i] == MAX_SEED_TRIANGLES ) {
					break;
				}
			}
		}
	}

	// we have a list of potential seed triangles, so we now go through each
	// potential candidate and look to see which produces the longest strip
	// and select our startTri based on this
	for ( i = 0; i < MAX_MATCHED_SIDES; i++ )
	{
		int j;

		for ( j = 0; j < numSeeds[i]; j++ )
		{
			for ( orientation = 0; orientation < 3; orientation++ )
			{
				int k;

				seedLengths[orientation][i][j] = StripLength( mesh, strip, seedTriangles[i][j], orientation, numInputTris, 2 );

				if ( seedLengths[orientation][i][j] > bestLength ) {
					bestTri = seedTriangles[i][j];
					bestLength = seedLengths[orientation][i][j];
					bestOrientation = orientation;
				}

				for ( k = 0; k < numInputTris; k++ )
				{
					if ( s_used[k] == 2 ) {
						s_used[k] = 0;
					}
				}
			}
		}

		if ( bestTri != -1 ) {
			break;
		}
	}

	// build the strip for real
	if ( bestTri != -1 ) {
		stripLen = StripLength( mesh, strip, bestTri, bestOrientation, numInputTris, 1 );
	}

	return stripLen;
}

/*
** OrderMesh
**
** Given an input mesh and an output mesh, this routine will reorder
** the triangles within the mesh into strips/fans.
*/
void OrderMesh( int input[][3], int output[][3], int numTris ){
	int i;
	int sumStrippedTriangles = 0;
	int strippedTriangles;
	int totalStrips = 0;
	int strip[8192][3];                 // could dump directly into 'output', but
	                                    // this helps with debugging

	memset( s_used, 0, sizeof( s_used ) );

#if 0
	FILE *fp = fopen( "strip.txt", "wt" );

	for ( i = 0; i < numTris; i++ )
	{
		fprintf( fp, "%4d: %3d %3d %3d\n", i, input[i][0], input[i][1], input[i][2] );
	}
	fclose( fp );
#endif

	// while there are still triangles that are not part of a strip
	while ( sumStrippedTriangles < numTris )
	{
		// build a strip
		strippedTriangles = BuildOptimizedList( input, strip, numTris );

		for ( i = 0; i < strippedTriangles; i++ )
		{
			output[sumStrippedTriangles + i][0] = strip[i][0];
			output[sumStrippedTriangles + i][1] = strip[i][1];
			output[sumStrippedTriangles + i][2] = strip[i][2];
		}

		sumStrippedTriangles += strippedTriangles;
		totalStrips++;
	}

	printf( "Triangles on surface:		%d\n", sumStrippedTriangles );
	printf( "Total strips from surface: %d\n", totalStrips );
	printf( "Average strip length:      %f\n", ( float ) sumStrippedTriangles / totalStrips );
}
