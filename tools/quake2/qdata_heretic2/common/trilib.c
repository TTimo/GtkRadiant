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
// trilib.c: library for loading triangles from an Alias triangle file
//

#include <stdio.h>
#include "cmdlib.h"
#include "inout.h"
#include "mathlib.h"
#include "trilib.h"
#include "token.h"
#include "l3dslib.h"
#include "fmodel.h"
#if 1
#include "qd_skeletons.h"
#endif

// on disk representation of a face
#define FLOAT_START 99999.0
#define FLOAT_END   -FLOAT_START
#define MAGIC       123322
#ifndef M_PI
  #define M_PI      3.14159265
#endif

float FixHTRRotateX = 0.0;
float FixHTRRotateY = 0.0;
float FixHTRRotateZ = 0.0;
float FixHTRTranslateX = 0.0;
float FixHTRTranslateY = 0.0;
float FixHTRTranslateZ = 0.0;

//#define NOISY 1

typedef struct {
	float v[3];
} vector;

typedef struct
{
	vector n;    /* normal */
	vector p;    /* point */
	vector c;    /* color */
	float u;     /* u */
	float v;     /* v */
} aliaspoint_t;

typedef struct {
	aliaspoint_t pt[3];
} tf_triangle;


void ByteSwapTri( tf_triangle *tri ){
	int i;

	for ( i = 0 ; i < sizeof( tf_triangle ) / 4 ; i++ )
	{
		( (int *)tri )[i] = BigLong( ( (int *)tri )[i] );
	}
}

void LoadTRI( char *filename, triangle_t **pptri, int *numtriangles, mesh_node_t **nodesList, int *num_mesh_nodes ){
	FILE        *input;
	float start;
	char name[256], tex[256];
	int i, count, magic;
	tf_triangle tri;
	triangle_t  *ptri;
	int iLevel;
	int exitpattern;
	float t;

	if ( nodesList ) {
		*num_mesh_nodes = 0;
		*nodesList = (mesh_node_t *) SafeMalloc( MAX_FM_MESH_NODES * sizeof( mesh_node_t ), "Mesh Node List" );
	}

	t = -FLOAT_START;
	*( (unsigned char *)&exitpattern + 0 ) = *( (unsigned char *)&t + 3 );
	*( (unsigned char *)&exitpattern + 1 ) = *( (unsigned char *)&t + 2 );
	*( (unsigned char *)&exitpattern + 2 ) = *( (unsigned char *)&t + 1 );
	*( (unsigned char *)&exitpattern + 3 ) = *( (unsigned char *)&t + 0 );

	if ( ( input = fopen( filename, "rb" ) ) == 0 ) {
		Error( "reader: could not open file '%s'", filename );
	}

	iLevel = 0;

	fread( &magic, sizeof( int ), 1, input );
	if ( BigLong( magic ) != MAGIC ) {
		Error( "%s is not a Alias object separated triangle file, magic number is wrong.", filename );
	}

	ptri = malloc( MAXTRIANGLES * sizeof( triangle_t ) );

	*pptri = ptri;

	while ( feof( input ) == 0 ) {
		if ( fread( &start,  sizeof( float ), 1, input ) < 1 ) {
			break;
		}
		*(int *)&start = BigLong( *(int *)&start );
		if ( *(int *)&start != exitpattern ) {
			if ( start == FLOAT_START ) {
				/* Start of an object or group of objects. */
				i = -1;
				do {
					/* There are probably better ways to read a string from */
					/* a file, but this does allow you to do error checking */
					/* (which I'm not doing) on a per character basis.      */
					++i;
					fread( &( name[i] ), sizeof( char ), 1, input );
				} while ( name[i] != '\0' );

//				indent();
//				fprintf(stdout,"OBJECT START: %s\n",name);
				fread( &count, sizeof( int ), 1, input );
				count = BigLong( count );
				++iLevel;
				if ( count != 0 ) {
//					indent();
//					fprintf(stdout,"NUMBER OF TRIANGLES: %d\n",count);

					i = -1;
					do {
						++i;
						fread( &( tex[i] ), sizeof( char ), 1, input );
					} while ( tex[i] != '\0' );

//					indent();
//					fprintf(stdout,"  Object texture name: '%s'\n",tex);
				}

				/* Else (count == 0) this is the start of a group, and */
				/* no texture name is present. */
			}
			else if ( start == FLOAT_END ) {
				/* End of an object or group. Yes, the name should be */
				/* obvious from context, but it is in here just to be */
				/* safe and to provide a little extra information for */
				/* those who do not wish to write a recursive reader. */
				/* Mia culpa. */
				--iLevel;
				i = -1;
				do {
					++i;
					fread( &( name[i] ), sizeof( char ), 1, input );
				} while ( name[i] != '\0' );

//				indent();
//				fprintf(stdout,"OBJECT END: %s\n",name);
				continue;
			}
		}

//
// read the triangles
//
		for ( i = 0; i < count; ++i ) {
			int j;

			fread( &tri, sizeof( tf_triangle ), 1, input );
			ByteSwapTri( &tri );
			for ( j = 0 ; j < 3 ; j++ )
			{
				int k;

				for ( k = 0 ; k < 3 ; k++ )
				{
					ptri->verts[j][k] = tri.pt[j].p.v[k];
				}
			}

			ptri++;

			if ( ( ptri - *pptri ) >= MAXTRIANGLES ) {
				Error( "Error: too many triangles; increase MAXTRIANGLES\n" );
			}
		}
	}

	*numtriangles = ptri - *pptri;

	fclose( input );

	DefaultNodesList( nodesList,num_mesh_nodes,numtriangles );
}


//==========================================================================
//
// LoadHRC
//
//==========================================================================

float scaling[3];
float rotation[3];
float translation[3];
static char     *hrc_name;

struct
{
	float v[3];
} vList[8192];

void HandleHRCModel( triangle_t **triList, int *triangleCount, mesh_node_t **nodesList, int *num_mesh_nodes,
					 int ActiveNode, int Depth, int numVerts ){
	void ReadHRCClusterList( mesh_node_t *meshNode, int baseIndex );

	int i, j;
	int vertexCount;
	int triCount;
	triangle_t  *tList;
	mesh_node_t *meshNode;
	float x, y, z;
	float x2, y2, z2;
	float rx, ry, rz;
	tokenType_t nextToken;
	float orig_scaling[3];
	float orig_rotation[3];
	float orig_translation[3];
	int start_tri;
	int pos,bit;
	int vertIndexBase;

	// Update Node Info
	if ( nodesList ) {
		TK_BeyondRequire( TK_NAME, TK_STRING );

		if ( Depth == 0 || tk_String[0] == '_' ) { // Root
			ActiveNode = *num_mesh_nodes;
			( *num_mesh_nodes )++;
			if ( ( *num_mesh_nodes ) > MAX_FM_MESH_NODES ) {
				Error( "Too many mesh nodes in file %s\n", hrc_name );
			}
			meshNode = &( *nodesList )[ActiveNode];

//			memset(meshNode, 0, sizeof(mesh_node_t));
			strcpy( meshNode->name, tk_String );

			memset( meshNode->tris, 0, sizeof( meshNode->tris ) );
			memset( meshNode->verts, 0, sizeof( meshNode->verts ) );

			meshNode->start_glcmds = 0;
			meshNode->num_glcmds = 0;
			vertIndexBase = 0;
		}
		else
		{   // Childs under the children
			meshNode = &( *nodesList )[ActiveNode];
			vertIndexBase = numVerts;
		}
	}
	else
	{
		meshNode = NULL;
	}


	// Get the scaling, rotation, and translation values
	TK_Beyond( TK_SCALING );
	for ( i = 0; i < 3; i++ )
	{
		orig_scaling[i] = scaling[i];

		TK_Require( TK_FLOATNUMBER );
		scaling[i] *= tk_FloatNumber;

		TK_Fetch();
	}
	TK_Beyond( TK_ROTATION );
	for ( i = 0; i < 3; i++ )
	{
		orig_rotation[i] = rotation[i];

		TK_Require( TK_FLOATNUMBER );
		rotation[i] = tk_FloatNumber;

		TK_Fetch();
	}
	TK_Beyond( TK_TRANSLATION );
	for ( i = 0; i < 3; i++ )
	{
		orig_translation[i] = translation[i];

		TK_Require( TK_FLOATNUMBER );
		translation[i] += tk_FloatNumber;

		TK_Fetch();
	}

	rx = ( ( rotation[0] - 90.0 ) / 360.0 ) * 2.0 * M_PI;
	ry = ( rotation[2] / 360.0 ) * 2.0 * M_PI;
	rz = ( rotation[1] / 360.0 ) * 2.0 * M_PI;

	// rjr - might not work if there an item doesn't have a mesh
	nextToken = tk_Token;
	if ( nextToken == TK_ACTOR_DATA ) {
		while ( nextToken != TK_MODEL && nextToken != TK_RBRACE )
		{
			nextToken = TK_Fetch();
		}
	}

	while ( nextToken == TK_SPLINE )
	{   // spline node has two right braces
		nextToken = TK_Beyond( TK_RBRACE );
		nextToken = TK_Beyond( TK_RBRACE );
	}

	while ( nextToken == TK_MATERIAL )
	{
		nextToken = TK_Beyond( TK_RBRACE );
	}

	while ( nextToken == TK_MODEL )
	{
		HandleHRCModel( triList,triangleCount,nodesList,num_mesh_nodes,ActiveNode, Depth + 1, 0 );

		nextToken = TK_Fetch();
	}

	if ( nextToken == TK_MESH ) {
		// Get all the tri and vertex info
		TK_BeyondRequire( TK_VERTICES, TK_INTNUMBER );
		vertexCount = tk_IntNumber;
		for ( i = 0; i < vertexCount; i++ )
		{
			TK_BeyondRequire( TK_LBRACKET, TK_INTNUMBER );
			if ( tk_IntNumber != i ) {
				Error( "File '%s', line %d:\nVertex index mismatch.\n",
					   tk_SourceName, tk_Line );
			}
			TK_Beyond( TK_POSITION );
			// Apply the scaling, rotation, and translation in the order
			// specified in the HRC file.  This could be wrong.
			TK_Require( TK_FLOATNUMBER );
			x = tk_FloatNumber * scaling[0];
			TK_FetchRequire( TK_FLOATNUMBER );
			y = tk_FloatNumber * scaling[1];
			TK_FetchRequire( TK_FLOATNUMBER );
			z = tk_FloatNumber * scaling[2];

			y2 = y * cos( rx ) + z*sin( rx );
			z2 = -y*sin( rx ) + z*cos( rx );
			y = y2;
			z = z2;

			x2 = x * cos( ry ) - z*sin( ry );
			z2 = x * sin( ry ) + z*cos( ry );
			x = x2;
			z = z2;

			x2 = x * cos( rz ) + y*sin( rz );
			y2 = -x*sin( rz ) + y*cos( rz );
			x = x2;
			y = y2;

			vList[i].v[0] = x + translation[0];
			vList[i].v[1] = y - translation[2];
			vList[i].v[2] = z + translation[1];
		}
		TK_BeyondRequire( TK_POLYGONS, TK_INTNUMBER );
		triCount = tk_IntNumber;
		if ( triCount >= MAXTRIANGLES ) {
			Error( "Too many triangles in file %s\n", hrc_name );
		}

		start_tri = *triangleCount;
		*triangleCount += triCount;

		tList = *triList;

		for ( i = 0; i < triCount; i++ )
		{
			if ( meshNode ) { // Update the node
				pos = ( i + start_tri ) >> 3;
				bit = 1 << ( ( i + start_tri ) & 7 );
				meshNode->tris[pos] |= bit;
			}

			TK_BeyondRequire( TK_LBRACKET, TK_INTNUMBER );
			if ( tk_IntNumber != i ) {
				Error( "File '%s', line %d:\nTriangle index mismatch.\n",
					   tk_SourceName, tk_Line );
			}
			TK_BeyondRequire( TK_NODES, TK_INTNUMBER );
			if ( tk_IntNumber != 3 ) {
				Error( "File '%s', line %d:\nBad polygon vertex count: %d.",
					   tk_SourceName, tk_Line, tk_IntNumber );
			}
			tList[i + start_tri].HasUV = true;
			for ( j = 0; j < 3; j++ )
			{
				TK_BeyondRequire( TK_LBRACKET, TK_INTNUMBER );
				if ( tk_IntNumber != j ) {
					Error( "File '%s', line %d:\nTriangle vertex index"
						   " mismatch.  %d should be %d\n", tk_SourceName, tk_Line,
						   tk_IntNumber, j );
				}
				TK_BeyondRequire( TK_VERTEX, TK_INTNUMBER );

				tList[i + start_tri].verts[2 - j][0] = vList[tk_IntNumber].v[0];
				tList[i + start_tri].verts[2 - j][1] = vList[tk_IntNumber].v[1];
				tList[i + start_tri].verts[2 - j][2] = vList[tk_IntNumber].v[2];
#if 1
				tList[i + start_tri].indicies[2 - j] = tk_IntNumber + vertIndexBase;
#endif
				TK_BeyondRequire( TK_UVTEXTURE, TK_FLOATNUMBER );
				tList[i + start_tri].uv[2 - j][0] = tk_FloatNumber;
				TK_Fetch();
				TK_Require( TK_FLOATNUMBER );
				tList[i + start_tri].uv[2 - j][1] = tk_FloatNumber;
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

		TK_Beyond( TK_RBRACE );
		TK_Beyond( TK_RBRACE );

		if ( tk_Token == TK_EDGES ) {
			//	TK_Beyond(TK_EDGES);
			TK_Beyond( TK_RBRACE );
		}

		scaling[0] = scaling[1] = scaling[2] = 1.0;
		//	rotation[0] = rotation[1] = rotation[2] = 0.0;
		//	translation[0] = translation[1] = translation[2] = 0.0;

		// See if there are any other models belonging to this node

#if 1
		TK_Fetch();

		nextToken = tk_Token;
		if ( nextToken == TK_CLUSTERS ) {
			if ( g_skelModel.clustered == -1 ) {
				ReadHRCClusterList( meshNode, vertIndexBase );
			}
			else
			{
				nextToken = TK_Get( TK_CLUSTER_NAME );

				while ( nextToken == TK_CLUSTER_NAME )
				{
					TK_BeyondRequire( TK_CLUSTER_STATE, TK_INTNUMBER );
					nextToken = TK_Fetch();
				}
			}

			// one right brace follow the list of clusters
			nextToken = TK_Beyond( TK_RBRACE );
		}
		else
		{
			if ( g_skelModel.clustered == -1 && !vertIndexBase ) {
				meshNode->clustered = false;
			}
		}
#endif

		nextToken = tk_Token;
		if ( nextToken == TK_SPLINE ) {
			while ( nextToken == TK_SPLINE )
			{   // spline node has two right braces
				nextToken = TK_Beyond( TK_RBRACE );
				nextToken = TK_Beyond( TK_RBRACE );
			}

			nextToken = TK_Beyond( TK_RBRACE );
		}

		while ( nextToken == TK_MATERIAL )
		{
			nextToken = TK_Beyond( TK_RBRACE );
		}

		while ( nextToken == TK_MODEL )
		{
			HandleHRCModel( triList,triangleCount,nodesList, num_mesh_nodes, ActiveNode, Depth + 1, vertexCount + vertIndexBase );

			nextToken = TK_Fetch();
		}
	}

	for ( i = 0; i < 3; i++ )
	{
		scaling[i] = orig_scaling[i];
		rotation[i] = orig_rotation[i];
		translation[i] = orig_translation[i];
	}
}

static void LoadHRC( char *fileName, triangle_t **triList, int *triangleCount, mesh_node_t **nodesList, int *num_mesh_nodes ){
	if ( nodesList ) {
		*num_mesh_nodes = 0;

		if ( !*nodesList ) {
			*nodesList = (mesh_node_t *) SafeMalloc( MAX_FM_MESH_NODES * sizeof( mesh_node_t ), "Mesh Node List" );
		}
	}

	hrc_name = fileName;

	scaling[0] = scaling[1] = scaling[2] = 1.0;
	rotation[0] = rotation[1] = rotation[2] = 0.0;
	translation[0] = translation[1] = translation[2] = 0.0;

	*triangleCount = 0;
	*triList = (triangle_t *) SafeMalloc( MAXTRIANGLES * sizeof( triangle_t ), "Triangle list" );
	memset( *triList,0,MAXTRIANGLES * sizeof( triangle_t ) );

	TK_OpenSource( fileName );
	TK_FetchRequire( TK_HRCH );
	TK_FetchRequire( TK_COLON );
	TK_FetchRequire( TK_SOFTIMAGE );

	// prime it
	TK_Beyond( TK_MODEL );

	HandleHRCModel( triList, triangleCount, nodesList, num_mesh_nodes, 0, 0, 0 );
	TK_CloseSource();
}

//==========================================================================
//
// LoadHTR
//
//==========================================================================
/*
   static int Version2;

   void HandleHTRModel(triangle_t **triList, int *triangleCount, mesh_node_t **nodesList, int *num_mesh_nodes,
                           int ActiveNode, int Depth, int numVerts)
   {
    int			i, j;
    int			vertexCount;
    int			vertexNum;
    int			triCount;
    float		origin[3];
    triangle_t	*tList;
    float		x, y, z;
    float		x2, y2, z2;
    float		rx, ry, rz;
    mesh_node_t *meshNode;
    int			pos,bit;
    int			vertIndexBase;
    int			start_tri;

    if (nodesList)
    {
        TK_BeyondRequire(TK_NAME, TK_STRING);

        if (Depth == 0 || tk_String[0] == '_')
        {	// Root
            ActiveNode = *num_mesh_nodes;
            (*num_mesh_nodes)++;
            if ((*num_mesh_nodes) > MAX_FM_MESH_NODES)
            {
                Error("Too many mesh nodes in file %s\n", hrc_name);
            }
            meshNode = &(*nodesList)[ActiveNode];

   //			memset(meshNode, 0, sizeof(mesh_node_t));
            strcpy(meshNode->name, tk_String);

            memset(meshNode->tris, 0, sizeof(meshNode->tris));
            memset(meshNode->verts, 0, sizeof(meshNode->verts));

            meshNode->start_glcmds = 0;
            meshNode->num_glcmds = 0;
            vertIndexBase = 0;
        }
        else
        {	// Childs under the children
            meshNode = &(*nodesList)[ActiveNode];
            vertIndexBase = numVerts;
        }
    }
    else
    {
        meshNode = NULL;
    }

    // Get vertex count
    TK_BeyondRequire(TK_VERTICES, TK_INTNUMBER);
    vertexCount = tk_IntNumber;

    // Get triangle count
    TK_BeyondRequire(TK_FACES, TK_INTNUMBER);
    triCount = tk_IntNumber;
    if(triCount >= MAXTRIANGLES)
    {
        Error("Too many triangles in file %s\n", hrc_name);
    }

    // Get origin
    TK_Beyond(TK_ORIGIN);
    TK_Require(TK_FLOATNUMBER);
    origin[0] = tk_FloatNumber;
    TK_FetchRequire(TK_FLOATNUMBER);
    origin[1] = tk_FloatNumber;
    TK_FetchRequire(TK_FLOATNUMBER);
    origin[2] = tk_FloatNumber;

    //rx = 90.0/360.0*2.0*M_PI;
    rx = FixHTRRotateX/360.0*2.0*M_PI;
    ry = FixHTRRotateY/360.0*2.0*M_PI;
    rz = FixHTRRotateZ/360.0*2.0*M_PI;

    // Get vertex list
    for(i = 0; i < vertexCount; i++)
    {
        TK_FetchRequire(TK_VERTEX);
        TK_FetchRequire(TK_FLOATNUMBER);
        x = tk_FloatNumber-origin[0];
        TK_FetchRequire(TK_FLOATNUMBER);
        y = tk_FloatNumber-origin[1];
        TK_FetchRequire(TK_FLOATNUMBER);
        z = tk_FloatNumber-origin[2];

        x += FixHTRTranslateX;
        y += FixHTRTranslateY;
        z += FixHTRTranslateZ;

        y2 = y*cos(rx)-z*sin(rx);
        z2 = y*sin(rx)+z*cos(rx);
        y = y2;
        z = z2;
        x2 = x*cos(ry)+z*sin(ry);
        z2 = -x*sin(ry)+z*cos(ry);
        x = x2;
        z = z2;
        x2 = x*cos(rz)-y*sin(rz);
        y2 = x*sin(rz)+y*cos(rz);
        x = x2;
        y = y2;

        vList[i].v[0] = x;
        vList[i].v[1] = y;
        vList[i].v[2] = z;
    }

    start_tri = *triangleCount;
   *triangleCount += triCount;

    tList = *triList;

    // Get face list
    for(i = 0; i < triCount; i++)
    {
        if (meshNode)
        {	// Update the node
            pos = (i + start_tri) >> 3;
            bit = 1 << ((i + start_tri) & 7 );
            meshNode->tris[pos] |= bit;
        }

        TK_FetchRequire(TK_FACE);
        TK_FetchRequire(TK_LPAREN);
        for(j = 0; j < 3; j++)
        {
            TK_FetchRequire(TK_INTNUMBER);
            vertexNum = tk_IntNumber-1;
            if(vertexNum >= vertexCount)
            {
                Error("File '%s', line %d:\nVertex number"
                    " >= vertexCount: %d\n", tk_SourceName, tk_Line,
                    tk_IntNumber);
            }
            tList[i+start_tri].verts[2-j][0] = vList[vertexNum].v[0];
            tList[i+start_tri].verts[2-j][1] = vList[vertexNum].v[1];
            tList[i+start_tri].verts[2-j][2] = vList[vertexNum].v[2];
        }
        TK_FetchRequire(TK_RPAREN);
   #ifdef _QDATA
        if (Version2)
        {
            TK_FetchRequire(TK_FLOATNUMBER);
            tList[i+start_tri].uv[0][0]=tk_FloatNumber;
            TK_FetchRequire(TK_FLOATNUMBER);
            tList[i+start_tri].uv[0][1]=tk_FloatNumber;
            TK_FetchRequire(TK_FLOATNUMBER);
            tList[i+start_tri].uv[1][0]=tk_FloatNumber;
            TK_FetchRequire(TK_FLOATNUMBER);
            tList[i+start_tri].uv[1][1]=tk_FloatNumber;
            TK_FetchRequire(TK_FLOATNUMBER);
            tList[i+start_tri].uv[2][0]=tk_FloatNumber;
            TK_FetchRequire(TK_FLOATNUMBER);
            tList[i+start_tri].uv[2][1]=tk_FloatNumber;
            tList[i+start_tri].HasUV=1;
        }
        else
            tList[i+start_tri].HasUV=0;
   #endif
   //		printf("Face %i:\n  v0: %f, %f, %f\n  v1: %f, %f, %f\n"
   //			"  v2: %f, %f, %f\n", i,
   //			tList[i].verts[0][0],
   //			tList[i].verts[0][1],
   //			tList[i].verts[0][2],
   //			tList[i].verts[1][0],
   //			tList[i].verts[1][1],
   //			tList[i].verts[1][2],
   //			tList[i].verts[2][0],
   //			tList[i].verts[2][1],
   //			tList[i].verts[2][2]);

    }

    TK_Fetch();

    if (tk_Token == TK_VERTICES)
    {
        HandleHTRModel(triList,triangleCount,nodesList, num_mesh_nodes, ActiveNode, Depth+1, vertexCount+vertIndexBase);
    }
   }

   static void LoadHTR(char *fileName, triangle_t **triList, int *triangleCount, mesh_node_t **nodesList, int *num_mesh_nodes)
   {
    if (nodesList)
    {
   *num_mesh_nodes = 0;

        if(!*nodesList)
        {
   *nodesList = SafeMalloc(MAX_FM_MESH_NODES * sizeof(mesh_node_t), "Mesh Node List");
        }
    }

    hrc_name = fileName;

    scaling[0] = scaling[1] = scaling[2] = 1.0;
    rotation[0] = rotation[1] = rotation[2] = 0.0;
    translation[0] = translation[1] = translation[2] = 0.0;

   *triangleCount = 0;
   *triList = SafeMalloc(MAXTRIANGLES*sizeof(triangle_t), "Triangle list");
    memset(*triList,0,MAXTRIANGLES*sizeof(triangle_t));

    TK_OpenSource(fileName);

    TK_Beyond(TK_C_HEXEN);
    TK_Beyond(TK_C_TRIANGLES);
    TK_BeyondRequire(TK_C_VERSION, TK_INTNUMBER);
    if(tk_IntNumber != 1&&tk_IntNumber != 2)
    {
        Error("Unsupported version (%d) in file %s\n", tk_IntNumber,
            fileName);
    }
    Version2=(tk_IntNumber==2);


    HandleHTRModel(triList, triangleCount, nodesList, num_mesh_nodes, 0, 0, 0);
   }

 */

static void LoadHTR( char *fileName, triangle_t **triList, int *triangleCount, mesh_node_t **nodesList, int *num_mesh_nodes ){
	int Version2 = 0;
	int i, j;
	int vertexCount;
	int vertexNum;
	struct
	{
		float v[3];
	}           *vList;
	int triCount;
	float origin[3];
	triangle_t  *tList;
	float x, y, z;
	float x2, y2, z2;
	float rx, ry, rz;

	if ( nodesList ) {
		*num_mesh_nodes = 0;
		*nodesList = (mesh_node_t *) SafeMalloc( MAX_FM_MESH_NODES * sizeof( mesh_node_t ), "Mesh Node List" );
	}

	TK_OpenSource( fileName );

	TK_Beyond( TK_C_HEXEN );
	TK_Beyond( TK_C_TRIANGLES );
	TK_BeyondRequire( TK_C_VERSION, TK_INTNUMBER );
	if ( tk_IntNumber != 1 && tk_IntNumber != 2 ) {
		Error( "Unsupported version (%d) in file %s\n", tk_IntNumber,
			   fileName );
	}
	Version2 = ( tk_IntNumber == 2 );


	// Get vertex count
	TK_BeyondRequire( TK_VERTICES, TK_INTNUMBER );
	vertexCount = tk_IntNumber;
	vList = (void *) SafeMalloc( vertexCount * sizeof vList[0], "Vertex list" );

	// Get triangle count
	TK_BeyondRequire( TK_FACES, TK_INTNUMBER );
	triCount = tk_IntNumber;
	if ( triCount >= MAXTRIANGLES ) {
		Error( "Too many triangles in file %s\n", fileName );
	}
	*triangleCount = triCount;
	tList = (triangle_t *) SafeMalloc( MAXTRIANGLES * sizeof( triangle_t ), "Triangle list" );
	*triList = tList;
	memset( *triList,0,MAXTRIANGLES * sizeof( triangle_t ) );

	// Get origin
	TK_Beyond( TK_ORIGIN );
	TK_Require( TK_FLOATNUMBER );
	origin[0] = tk_FloatNumber;
	TK_FetchRequire( TK_FLOATNUMBER );
	origin[1] = tk_FloatNumber;
	TK_FetchRequire( TK_FLOATNUMBER );
	origin[2] = tk_FloatNumber;

	//rx = 90.0/360.0*2.0*M_PI;
	rx = FixHTRRotateX / 360.0 * 2.0 * M_PI;
	ry = FixHTRRotateY / 360.0 * 2.0 * M_PI;
	rz = FixHTRRotateZ / 360.0 * 2.0 * M_PI;

	// Get vertex list
	for ( i = 0; i < vertexCount; i++ )
	{
		TK_FetchRequire( TK_VERTEX );
		TK_FetchRequire( TK_FLOATNUMBER );
		x = tk_FloatNumber - origin[0];
		TK_FetchRequire( TK_FLOATNUMBER );
		y = tk_FloatNumber - origin[1];
		TK_FetchRequire( TK_FLOATNUMBER );
		z = tk_FloatNumber - origin[2];

		x += FixHTRTranslateX;
		y += FixHTRTranslateY;
		z += FixHTRTranslateZ;

		y2 = y * cos( rx ) - z*sin( rx );
		z2 = y * sin( rx ) + z*cos( rx );
		y = y2;
		z = z2;
		x2 = x * cos( ry ) + z*sin( ry );
		z2 = -x*sin( ry ) + z*cos( ry );
		x = x2;
		z = z2;
		x2 = x * cos( rz ) - y*sin( rz );
		y2 = x * sin( rz ) + y*cos( rz );
		x = x2;
		y = y2;

		vList[i].v[0] = x;
		vList[i].v[1] = y;
		vList[i].v[2] = z;
	}

	// Get face list
	for ( i = 0; i < triCount; i++ )
	{
		TK_FetchRequire( TK_FACE );
		TK_FetchRequire( TK_LPAREN );
		for ( j = 0; j < 3; j++ )
		{
			TK_FetchRequire( TK_INTNUMBER );
			vertexNum = tk_IntNumber - 1;
			if ( vertexNum >= vertexCount ) {
				Error( "File '%s', line %d:\nVertex number"
					   " >= vertexCount: %d\n", tk_SourceName, tk_Line,
					   tk_IntNumber );
			}
			tList[i].verts[2 - j][0] = vList[vertexNum].v[0];
			tList[i].verts[2 - j][1] = vList[vertexNum].v[1];
			tList[i].verts[2 - j][2] = vList[vertexNum].v[2];
		}
		TK_FetchRequire( TK_RPAREN );
#if 1
		if ( Version2 ) {
			TK_FetchRequire( TK_FLOATNUMBER );
			tList[i].uv[2][0] = fmod( 1000 + tk_FloatNumber,1 );
			TK_FetchRequire( TK_FLOATNUMBER );
			tList[i].uv[2][1] = fmod( 1000 + tk_FloatNumber,1 );
			TK_FetchRequire( TK_FLOATNUMBER );
			tList[i].uv[1][0] = fmod( 1000 + tk_FloatNumber,1 );
			TK_FetchRequire( TK_FLOATNUMBER );
			tList[i].uv[1][1] = fmod( 1000 + tk_FloatNumber,1 );
			TK_FetchRequire( TK_FLOATNUMBER );
			tList[i].uv[0][0] = fmod( 1000 + tk_FloatNumber,1 );
			TK_FetchRequire( TK_FLOATNUMBER );
			tList[i].uv[0][1] = fmod( 1000 + tk_FloatNumber,1 );
			tList[i].HasUV = 1;
		}
		else{
			tList[i].HasUV = 0;
		}
#endif
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

	free( vList );
	TK_CloseSource();
	DefaultNodesList( nodesList,num_mesh_nodes,triangleCount );
}

//==========================================================================
//
// LoadTriangleList
//
//==========================================================================

void LoadTriangleList( char *fileName, triangle_t **triList, int *triangleCount, mesh_node_t **ppmnodes, int *num_mesh_nodes ){
	FILE    *file1;
	int dot = '.';
	char    *dotstart;
	char InputFileName[256];

	dotstart = strrchr( fileName,dot ); // Does it already have an extension on the file name?

	if ( !dotstart ) {
		strcpy( InputFileName, fileName );
		strcat( InputFileName, ".hrc" );
		if ( ( file1 = fopen( InputFileName, "rb" ) ) != NULL ) {
			fclose( file1 );
			LoadHRC( InputFileName, triList, triangleCount, ppmnodes, num_mesh_nodes );
			printf( " - assuming .HRC\n" );
			return;
		}

		strcpy( InputFileName, fileName );
		strcat( InputFileName, ".asc" );
		if ( ( file1 = fopen( InputFileName, "rb" ) ) != NULL ) {
			fclose( file1 );
			LoadASC( InputFileName, triList, triangleCount, ppmnodes, num_mesh_nodes );
			printf( " - assuming .ASC\n" );
			return;
		}

		strcpy( InputFileName, fileName );
		strcat( InputFileName, ".tri" );
		if ( ( file1 = fopen( InputFileName, "rb" ) ) != NULL ) {
			fclose( file1 );
			LoadTRI( InputFileName, triList, triangleCount, ppmnodes, num_mesh_nodes );
			printf( " - assuming .TRI\n" );
			return;
		}

		strcpy( InputFileName, fileName );
		strcat( InputFileName, ".3ds" );
		if ( ( file1 = fopen( InputFileName, "rb" ) ) != NULL ) {
			fclose( file1 );
			Load3DSTriangleList( InputFileName, triList, triangleCount, ppmnodes, num_mesh_nodes );
			printf( " - assuming .3DS\n" );
			return;
		}

		strcpy( InputFileName, fileName );
		strcat( InputFileName, ".htr" );
		if ( ( file1 = fopen( InputFileName, "rb" ) ) != NULL ) {
			fclose( file1 );
			LoadHTR( InputFileName, triList, triangleCount, ppmnodes, num_mesh_nodes );
			printf( " - assuming .HTR\n" );
			return;
		}
		Error( "\n Could not open file '%s':\n"
			   "No HRC, ASC, 3DS, HTR, or TRI match.\n", fileName );
	}
	else
	{
		if ( ( file1 = fopen( fileName, "rb" ) ) != NULL ) {
			printf( "\n" );
			fclose( file1 );
			if ( strcmp( dotstart,".hrc" ) == 0 || strcmp( dotstart,".HRC" ) == 0 ) {
				LoadHRC( fileName, triList, triangleCount, ppmnodes, num_mesh_nodes );
			}
			else if ( strcmp( dotstart,".asc" ) == 0 || strcmp( dotstart,".ASC" ) == 0 ) {
				LoadASC( fileName, triList, triangleCount, ppmnodes, num_mesh_nodes );
			}
			else if ( strcmp( dotstart,".tri" ) == 0 || strcmp( dotstart,".TRI" ) == 0 ) {
				LoadTRI( fileName, triList, triangleCount, ppmnodes, num_mesh_nodes );
			}
			else if ( strcmp( dotstart,".3ds" ) == 0 || strcmp( dotstart,".3DS" ) == 0 ) {
				Load3DSTriangleList( fileName, triList, triangleCount, ppmnodes, num_mesh_nodes );
			}
			else if ( strcmp( dotstart,".htr" ) == 0 || strcmp( dotstart,".HTR" ) == 0 ) {
				LoadHTR( fileName, triList, triangleCount, ppmnodes, num_mesh_nodes );
			}
			else
			{
				Error( "Could not open file '%s':\n",fileName );
				return;
			}
		}
		else    //failed to load file
		{
			Error( "Could not open file '%s':\n",fileName );
		}

	}
}
