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
// l3dslib.c: library for loading triangles from an Alias triangle file
//

#include <stdio.h>
#include "cmdlib.h"
#include "inout.h"
#include "mathlib.h"
#include "trilib.h"
#include "l3dslib.h"
#include "token.h"
#include "fmodel.h"
#include "bspfile.h"

#define MAIN3DS       0x4D4D
#define EDIT3DS       0x3D3D  // this is the start of the editor config
#define EDIT_OBJECT   0x4000
#define OBJ_TRIMESH   0x4100
#define TRI_VERTEXL   0x4110
#define TRI_FACEL1    0x4120

#define MAXVERTS    2000

typedef struct {
	int v[4];
} tri;

float fverts[MAXVERTS][3];
tri tris[MAXTRIANGLES];

int bytesread, level, numtris, totaltris;
int vertsfound, trisfound;

triangle_t  *ptri;



void DefaultNodesList( mesh_node_t **nodesList, int *num_mesh_nodes, int *numtriangles ){
	int pos, bit, i;

	if ( nodesList ) {
		*num_mesh_nodes = 1;
		memset( &( *nodesList )[0], 0, sizeof( mesh_node_t ) );
		strcpy( ( *nodesList )[0].name, "default" );

		// set all of the tris to be used for the top node
		for ( i = 0; i < ( *numtriangles ); i++ )
		{
			pos = ( i ) >> 3;
			bit = 1 << ( ( i ) & 7 );

			( *nodesList )[0].tris[pos] |= bit;
		}
	}
}


// Alias stores triangles as 3 explicit vertices in .tri files, so even though we
// start out with a vertex pool and vertex indices for triangles, we have to convert
// to raw, explicit triangles
void StoreAliasTriangles( void ){
	int i, j, k;

	if ( ( totaltris + numtris ) > MAXTRIANGLES ) {
		Error( "Error: Too many triangles" );
	}

	for ( i = 0; i < numtris ; i++ )
	{
		for ( j = 0 ; j < 3 ; j++ )
		{
			for ( k = 0 ; k < 3 ; k++ )
			{
				ptri[i + totaltris].verts[j][k] = fverts[tris[i].v[j]][k];
			}
		}
	}

	totaltris += numtris;
	numtris = 0;
	vertsfound = 0;
	trisfound = 0;
}


int ParseVertexL( FILE *input ){
	int i, j, startbytesread, numverts;
	unsigned short tshort;

	if ( vertsfound ) {
		Error( "Error: Multiple vertex chunks" );
	}

	vertsfound = 1;
	startbytesread = bytesread;

	if ( feof( input ) ) {
		Error( "Error: unexpected end of file" );
	}

	fread( &tshort, sizeof( tshort ), 1, input );
	bytesread += sizeof( tshort );
	numverts = (int)tshort;

	if ( numverts > MAXVERTS ) {
		Error( "Error: Too many vertices" );
	}

	for ( i = 0 ; i < numverts ; i++ )
	{
		for ( j = 0 ; j < 3 ; j++ )
		{
			if ( feof( input ) ) {
				Error( "Error: unexpected end of file" );
			}

			fread( &fverts[i][j], sizeof( float ), 1, input );
			bytesread += sizeof( float );
		}
	}

	if ( vertsfound && trisfound ) {
		StoreAliasTriangles();
	}

	return bytesread - startbytesread;
}


int ParseFaceL1( FILE *input ){

	int i, j, startbytesread;
	unsigned short tshort;

	if ( trisfound ) {
		Error( "Error: Multiple face chunks" );
	}

	trisfound = 1;
	startbytesread = bytesread;

	if ( feof( input ) ) {
		Error( "Error: unexpected end of file" );
	}

	fread( &tshort, sizeof( tshort ), 1, input );
	bytesread += sizeof( tshort );
	numtris = (int)tshort;

	if ( numtris > MAXTRIANGLES ) {
		Error( "Error: Too many triangles" );
	}

	for ( i = 0 ; i < numtris ; i++ )
	{
		for ( j = 0 ; j < 4 ; j++ )
		{
			if ( feof( input ) ) {
				Error( "Error: unexpected end of file" );
			}

			fread( &tshort, sizeof( tshort ), 1, input );
			bytesread += sizeof( tshort );
			tris[i].v[j] = (int)tshort;
		}
	}

	if ( vertsfound && trisfound ) {
		StoreAliasTriangles();
	}

	return bytesread - startbytesread;
}


int ParseChunk( FILE *input ){
#define BLOCK_SIZE  4096
	char temp[BLOCK_SIZE];
	unsigned short type;
	int i, length, w, t, retval;

	level++;
	retval = 0;

// chunk type
	if ( feof( input ) ) {
		Error( "Error: unexpected end of file" );
	}

	fread( &type, sizeof( type ), 1, input );
	bytesread += sizeof( type );

// chunk length
	if ( feof( input ) ) {
		Error( "Error: unexpected end of file" );
	}

	fread( &length, sizeof( length ), 1, input );
	bytesread += sizeof( length );
	w = length - 6;

// process chunk if we care about it, otherwise skip it
	switch ( type )
	{
	case TRI_VERTEXL:
		w -= ParseVertexL( input );
		goto ParseSubchunk;

	case TRI_FACEL1:
		w -= ParseFaceL1( input );
		goto ParseSubchunk;

	case EDIT_OBJECT:
		// read the name
		i = 0;

		do
		{
			if ( feof( input ) ) {
				Error( "Error: unexpected end of file" );
			}

			fread( &temp[i], 1, 1, input );
			i++;
			w--;
			bytesread++;
		} while ( temp[i - 1] );

	case MAIN3DS:
	case OBJ_TRIMESH:
	case EDIT3DS:
		// parse through subchunks
ParseSubchunk:
		while ( w > 0 )
		{
			w -= ParseChunk( input );
		}

		retval = length;
		goto Done;

	default:
		// skip other chunks
		while ( w > 0 )
		{
			t = w;

			if ( t > BLOCK_SIZE ) {
				t = BLOCK_SIZE;
			}

			if ( feof( input ) ) {
				Error( "Error: unexpected end of file" );
			}

			fread( &temp, t, 1, input );
			bytesread += t;

			w -= t;
		}

		retval = length;
		goto Done;
	}

Done:
	level--;
	return retval;
}


void Load3DSTriangleList( char *filename, triangle_t **pptri, int *numtriangles, mesh_node_t **nodesList, int *num_mesh_nodes ){
	FILE        *input;
	short int tshort;

	if ( nodesList ) {
		*num_mesh_nodes = 0;
		*nodesList = (mesh_node_t *) SafeMalloc( MAX_FM_MESH_NODES * sizeof( mesh_node_t ), "Mesh Node List" );
	}

	bytesread = 0;
	level = 0;
	numtris = 0;
	totaltris = 0;
	vertsfound = 0;
	trisfound = 0;

	if ( ( input = fopen( filename, "rb" ) ) == 0 ) {
		fprintf( stderr,"reader: could not open file '%s'\n", filename );
		exit( 0 );
	}

	fread( &tshort, sizeof( tshort ), 1, input );

// should only be MAIN3DS, but some files seem to start with EDIT3DS, with
// no MAIN3DS
	if ( ( tshort != MAIN3DS ) && ( tshort != EDIT3DS ) ) {
		fprintf( stderr,"File is not a 3DS file.\n" );
		exit( 0 );
	}

// back to top of file so we can parse the first chunk descriptor
	fseek( input, 0, SEEK_SET );

	ptri = malloc( MAXTRIANGLES * sizeof( triangle_t ) );

	*pptri = ptri;

// parse through looking for the relevant chunk tree (MAIN3DS | EDIT3DS | EDIT_OBJECT |
// OBJ_TRIMESH | {TRI_VERTEXL, TRI_FACEL1}) and skipping other chunks
	ParseChunk( input );

	if ( vertsfound || trisfound ) {
		Error( "Incomplete triangle set" );
	}

	*numtriangles = totaltris;

	fclose( input );

	DefaultNodesList( nodesList,num_mesh_nodes,numtriangles );
}

//==========================================================================
//
// LoadASC
//
//==========================================================================

void LoadASC( char *fileName, triangle_t **triList, int *triangleCount, mesh_node_t **nodesList, int *num_mesh_nodes ){
	int i, j;
	int vertexCount;
	struct
	{
		float v[3];
	}           *vList;
	int triCount;
	triangle_t  *tList;
	float x, y, z;
//	float		x2, y2, z2;
//	float		rx, ry, rz;
	qboolean goodObject;

	if ( nodesList ) {
		*num_mesh_nodes = 0;
		*nodesList = (mesh_node_t *) SafeMalloc( MAX_FM_MESH_NODES * sizeof( mesh_node_t ), "Mesh Node List" );
	}

	TK_OpenSource( fileName );

	goodObject = false;
	while ( goodObject == false )
	{
		TK_Beyond( TK_C_NAMED );
		TK_Beyond( TK_OBJECT );
		TK_Beyond( TK_C_TRI );
		TK_Beyond( TK_MESH );
		TK_BeyondRequire( TK_C_VERTICES, TK_COLON );
		TK_FetchRequire( TK_INTNUMBER );
		vertexCount = tk_IntNumber;
		if ( vertexCount > 0 ) {
			goodObject = true;
		}
	}
	TK_BeyondRequire( TK_C_FACES, TK_COLON );
	TK_FetchRequire( TK_INTNUMBER );
	triCount = tk_IntNumber;
	if ( triCount >= MAXTRIANGLES ) {
		Error( "Too many triangles in file %s\n", fileName );
	}
	*triangleCount = triCount;
	tList = (triangle_t *) SafeMalloc( MAXTRIANGLES * sizeof( triangle_t ), "Triangle list" );
	*triList = tList;

	memset( *triList,0,MAXTRIANGLES * sizeof( triangle_t ) );
	TK_BeyondRequire( TK_C_VERTEX, TK_LIST );

/*	rx = ((rotation[0]+90.0)/360.0)*2.0*M_PI;
    //rx = (rotation[0]/360.0)*2.0*M_PI;
    ry = (rotation[1]/360.0)*2.0*M_PI;
    rz = (rotation[2]/360.0)*2.0*M_PI;
 */
	vList = (void *) SafeMalloc( vertexCount * sizeof vList[0], "Vertex list" );
	for ( i = 0; i < vertexCount; i++ )
	{
		TK_BeyondRequire( TK_C_VERTEX, TK_INTNUMBER );
		if ( tk_IntNumber != i ) {
			Error( "File '%s', line %d:\nVertex index mismatch.\n",
				   tk_SourceName, tk_Line );
		}
		TK_FetchRequireFetch( TK_COLON );

		TK_BeyondRequire( TK_COLON, TK_FLOATNUMBER );
		x = tk_FloatNumber;
		TK_BeyondRequire( TK_COLON, TK_FLOATNUMBER );
		y = tk_FloatNumber;
		TK_BeyondRequire( TK_COLON, TK_FLOATNUMBER );
		z = tk_FloatNumber;

/*		x2 = x*cos(rz)+y*sin(rz);
        y2 = -x*sin(rz)+y*cos(rz);
        x = x2;
        y = y2;
        y2 = y*cos(rx)+z*sin(rx);
        z2 = -y*sin(rx)+z*cos(rx);
        y = y2;
        z = z2;
        x2 = x*cos(ry)-z*sin(ry);
        z2 = x*sin(ry)+z*cos(ry);
        x = x2;
        z = z2;
 */
		vList[i].v[0] = x;
		vList[i].v[1] = y;
		vList[i].v[2] = z;
	}
	TK_BeyondRequire( TK_C_FACE, TK_LIST );
	for ( i = 0; i < triCount; i++ )
	{
		TK_BeyondRequire( TK_C_FACE, TK_INTNUMBER );
		if ( tk_IntNumber != i ) {
			Error( "File '%s', line %d:\nTriangle index mismatch.\n",
				   tk_SourceName, tk_Line );
		}
		for ( j = 0; j < 3; j++ )
		{
			TK_BeyondRequire( TK_IDENTIFIER, TK_COLON );
			TK_FetchRequire( TK_INTNUMBER );
			if ( tk_IntNumber >= vertexCount ) {
				Error( "File '%s', line %d:\nVertex number"
					   " > vertexCount: %d\n", tk_SourceName, tk_Line,
					   tk_IntNumber );
			}
			tList[i].verts[2 - j][0] = vList[tk_IntNumber].v[0];
			tList[i].verts[2 - j][1] = vList[tk_IntNumber].v[1];
			tList[i].verts[2 - j][2] = vList[tk_IntNumber].v[2];
#ifdef _QDATA
			tList[i].indicies[2 - j] = tk_IntNumber;
#endif
		}

/*		printf("Face %i:\n  v0: %f, %f, %f\n  v1: %f, %f, %f\n"
            "  v2: %f, %f, %f\n", i,
            tList[i].verts[0][0],
            tList[i].verts[0][1],
            tList[i].verts[0][2],
            tList[i].verts[1][0],
            tList[i].verts[1][1],
            tList[i].verts[1][2],
            tList[i].verts[2][0],
            tList[i].verts[2][1],
            tList[i].verts[2][2]);
 */
	}

	DefaultNodesList( nodesList,num_mesh_nodes,triangleCount );
}
