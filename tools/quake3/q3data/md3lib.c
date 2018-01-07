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
#ifdef WIN32
#include <io.h>
#endif
#include "md3lib.h"

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
#define filelength Q_filelength
#endif

/*
** MD3_ComputeTagFromTri
*/
void MD3_ComputeTagFromTri( md3Tag_t *pTag, const float pTri[3][3] ){
	float len[3];
	vec3_t axes[3], sides[3];
	int longestSide, shortestSide, hypotSide;
	int origin;
	int j;
	float d;

	memset( axes, 0, sizeof( axes ) );
	memset( sides, 0, sizeof( sides ) );

	//
	// compute sides
	//
	for ( j = 0; j < 3; j++ )
	{
		sides[j][0] = pTri[( j + 1 ) % 3][0] - pTri[j][0];
		sides[j][1] = pTri[( j + 1 ) % 3][1] - pTri[j][1];
		sides[j][2] = pTri[( j + 1 ) % 3][2] - pTri[j][2];

		len[j] = ( float ) sqrt( DotProduct( sides[j], sides[j] ) );
	}

#if 0
	if ( len[0] > len[1] && len[0] > len[2] ) {
		longestSide = 0; shortestSide = 1; origin = 2;
	}
	else if ( len[1] > len[0] && len[1] > len[2] ) {
		longestSide = 1; shortestSide = 2; origin = 0;
	}
	else if ( len[2] > len[0] && len[2] > len[1] ) {
		longestSide = 2; shortestSide = 0; origin = 1;
	}
	else
	{
		Error( "invalid tag triangle, must be a right triangle with unequal length sides" );
	}
#endif
	if ( len[0] > len[1] && len[0] > len[2] ) {
		hypotSide = 0;
		origin = 2;
	}
	else if ( len[1] > len[0] && len[1] > len[2] ) {
		hypotSide = 1;
		origin = 0;
	}
	else if ( len[2] > len[0] && len[2] > len[1] ) {
		hypotSide = 2;
		origin = 1;
	}
	else
	{
		Error( "invalid tag triangle, must be a right triangle with unequal length sides" );
		return;
	}
	len[hypotSide] = -1;

	if ( len[0] > len[1] && len[0] > len[2] ) {
		longestSide = 0;
	}
	else if ( len[1] > len[0] && len[1] > len[2] ) {
		longestSide = 1;
	}
	else if ( len[2] > len[0] && len[2] > len[1] ) {
		longestSide = 2;
	}
	len[longestSide] = -1;

	if ( len[0] > len[1] && len[0] > len[2] ) {
		shortestSide = 0;
	}
	else if ( len[1] > len[0] && len[1] > len[2] ) {
		shortestSide = 1;
	}
	else if ( len[2] > len[0] && len[2] > len[1] ) {
		shortestSide = 2;
	}
	len[shortestSide] = -1;



//	VectorNormalize( sides[shortestSide], axes[0] );
//	VectorNormalize( sides[longestSide], axes[1] );
	VectorNormalize( sides[longestSide], axes[0] );
	VectorNormalize( sides[shortestSide], axes[1] );

	// project shortest side so that it is exactly 90 degrees to the longer side
	d = DotProduct( axes[0], axes[1] );
	VectorMA( axes[0], -d, axes[1], axes[0] );
	VectorNormalize( axes[0], axes[0] );

	CrossProduct( sides[longestSide], sides[shortestSide], axes[2] );
	VectorNormalize( axes[2], axes[2] );

	pTag->origin[0] = pTri[origin][0];
	pTag->origin[1] = pTri[origin][1];
	pTag->origin[2] = pTri[origin][2];

	VectorCopy( axes[0], pTag->axis[0] );
	VectorCopy( axes[1], pTag->axis[1] );
	VectorCopy( axes[2], pTag->axis[2] );
}

/*
   ==============
   MD3_Dump
   ==============
 */
void MD3_Dump( const char *filename ){
	md3Header_t header;
	md3Tag_t *pTag;
	md3Surface_t *pSurface;
	FILE *fp;
	void *_buffer;
	void *buffer;
	long fileSize;
	int i;

	if ( ( fp = fopen( filename, "rb" ) ) == 0 ) {
		Error( "Unable to open '%s'\n", filename );
	}

	fileSize = Q_filelength( fp );
	_buffer = malloc( fileSize );
	fread( _buffer, fileSize, 1, fp );
	fclose( fp );

	buffer = ( char * ) _buffer;
	header = *( md3Header_t * ) _buffer;

	if ( header.ident != MD3_IDENT ) {
		Error( "Incorrect ident for '%s'\n", filename );
	}

	printf( "Contents of '%s'\n", filename );
	printf( "  version:        %d\n", header.version );
	printf( "  name:           %s\n", header.name );
	printf( "  num frames:     %d\n", header.numFrames );
	printf( "  num tags:       %d\n", header.numTags );
	printf( "  num surfaces:   %d\n", header.numSurfaces );
	printf( "  num skins:      %d\n", header.numSkins );
	printf( "  file size:      %ld\n", fileSize );

	printf( "--- TAGS ---\n" );
	pTag = ( md3Tag_t * ) ( ( ( char * ) buffer ) + header.ofsTags );
	for ( i = 0; i < header.numTags; i++, pTag++ )
	{
		printf( "  tag %d ('%s')\n", i, pTag->name );
		printf( "    origin: %f,%f,%f\n", pTag->origin[0], pTag->origin[1], pTag->origin[2] );
		printf( "        vf: %f,%f,%f\n", pTag->axis[0][0], pTag->axis[0][1], pTag->axis[0][2] );
		printf( "        vr: %f,%f,%f\n", pTag->axis[1][0], pTag->axis[1][1], pTag->axis[1][2] );
		printf( "        vu: %f,%f,%f\n", pTag->axis[2][0], pTag->axis[2][1], pTag->axis[2][2] );
	}

	printf( "--- SURFACES ---\n" );
	pSurface = ( md3Surface_t * ) ( ( ( char * ) buffer ) + header.ofsSurfaces );

	for ( i = 0; i < header.numSurfaces; i++ )
	{
		int j;

		md3Shader_t *pShader = ( md3Shader_t * ) ( ( ( char * ) pSurface ) + pSurface->ofsShaders );

		printf( "\n  surface %d ('%s')\n", i, pSurface->name );
		printf( "    num frames: %d\n", pSurface->numFrames );
		printf( "    num shaders: %d\n", pSurface->numShaders );
		printf( "    num tris: %d\n", pSurface->numTriangles );
		printf( "    num verts: %d\n", pSurface->numVerts );

		if ( pSurface->numShaders > 0 ) {
			printf( "    --- SHADERS ---\n" );

			for ( j = 0; j < pSurface->numShaders; j++, pShader++ )
			{
				printf( "    shader %d ('%s')\n", j, pShader->name );
			}
		}
		pSurface = ( md3Surface_t * ) ( ( ( char * ) pSurface ) + pSurface->ofsEnd );
	}

	free( _buffer );
}
