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
#include <math.h>
#include "token.h"
#include "joints.h"
#include "angles.h"
#include "inout.h"

char *SKEL_ROOT_NAMES[] =
{
	"RAVEN_SPINE"
};

char *SKEL_NAMES[] =
{
	"RAVEN_WAIST1",
	"RAVEN_WAIST2",
	"RAVEN_NECK"
};

int NAME_OFFSETS[] =
{
	0
};

int numJointsForSkeleton[] =
{
	NUM_JOINTS_RAVEN,
	NUM_JOINTS_BOX
};

float g_scaling[3];
float g_rotation[3];
float g_translation[3];

//==========================================================================
//
// LoadHRCClustered
//
//==========================================================================

static void LoadHRCClustered( char *fileName, int **clusterList, int *num_verts, int skelType ){
	int i, j;

	TK_OpenSource( fileName );
	TK_FetchRequire( TK_HRCH );
	TK_FetchRequire( TK_COLON );
	TK_FetchRequire( TK_SOFTIMAGE );

	TK_Beyond( TK_CLUSTERS );

	while ( TK_Search( TK_CLUSTER_NAME ) != TK_EOF )
	{
		TK_Require( TK_STRING );

		for ( i = 0; i < numJointsForSkeleton[skelType]; ++i )
		{
			if ( stricmp( tk_String, SKEL_NAMES[NAME_OFFSETS[skelType] + i] ) == 0 ) {
				i = -i + numJointsForSkeleton[skelType] - 1;

				TK_BeyondRequire( TK_NUM_CLUSTER_VERTICES, TK_INTNUMBER );

				num_verts[i + 1] = tk_IntNumber;

				clusterList[i] = (int *) SafeMalloc( num_verts[i + 1] * sizeof( int ), "LoadHRCClustered" );
				assert( clusterList[i] );
				// currently this function is only called by LoadTriangleListClustered, which in turn is only
				// called by Cmd_Base in qdata.  This is where the only call to free for this memory is being made.

				TK_Beyond( TK_LBRACE );

				for ( j = 0; j < num_verts[i + 1]; ++j )
				{
					TK_Require( TK_INTNUMBER );
					clusterList[i][j] = tk_IntNumber;
					TK_Fetch();
				}

				break;
			}
		}
	}

	num_verts[0] = numJointsForSkeleton[skelType];
}

static void LoadHRCGlobals( char *fileName ){
	int i;

	TK_OpenSource( fileName );
	TK_FetchRequire( TK_HRCH );
	TK_FetchRequire( TK_COLON );
	TK_FetchRequire( TK_SOFTIMAGE );
	TK_Beyond( TK_MODEL );

	TK_Beyond( TK_SCALING );
	for ( i = 0; i < 3; i++ )
	{
		TK_Require( TK_FLOATNUMBER );
		g_scaling[i] = tk_FloatNumber;
		TK_Fetch();
	}

	TK_Beyond( TK_ROTATION );
	for ( i = 0; i < 3; i++ )
	{
		TK_Require( TK_FLOATNUMBER );
		g_rotation[i] = tk_FloatNumber;
		TK_Fetch();
	}

	TK_Beyond( TK_TRANSLATION );
	for ( i = 0; i < 3; i++ )
	{
		TK_Require( TK_FLOATNUMBER );
		g_translation[i] = tk_FloatNumber;
		TK_Fetch();
	}
}

static void ParseVec3( vec3_t in ){
	TK_Require( TK_FLOATNUMBER );
	in[1] = tk_FloatNumber;
	TK_FetchRequire( TK_FLOATNUMBER );
	in[2] = tk_FloatNumber;
	TK_FetchRequire( TK_FLOATNUMBER );
	in[0] = tk_FloatNumber;
}

static void ParseRotation3( vec3_t in ){
	TK_Require( TK_FLOATNUMBER );
	in[1] = -tk_FloatNumber;
	TK_FetchRequire( TK_FLOATNUMBER );
	in[2] = tk_FloatNumber;
	TK_FetchRequire( TK_FLOATNUMBER );
	in[0] = tk_FloatNumber;
}

static void ParseTranslation3( vec3_t in ){
	TK_Require( TK_FLOATNUMBER );
	in[1] = tk_FloatNumber;
	TK_FetchRequire( TK_FLOATNUMBER );
	in[2] = tk_FloatNumber;
	TK_FetchRequire( TK_FLOATNUMBER );
	in[0] = tk_FloatNumber;
}

static void LoadHRCJointList( char *fileName, QDataJoint_t *jointList, int skelType ){
#define MAX_STACK 64
	int i, j;
	vec3_t curTranslation[MAX_STACK], curRotation[MAX_STACK], curScale[MAX_STACK];
	int curCorrespondingJoint[MAX_STACK];
	int currentStack = 0, stackSize;
	int baseJoint;
	float cx, sx, cy, sy, cz, sz;
	float rx, ry, rz;
	float x2, y2, z2;


	TK_OpenSource( fileName );
	TK_FetchRequire( TK_HRCH );
	TK_FetchRequire( TK_COLON );
	TK_FetchRequire( TK_SOFTIMAGE );

	TK_Beyond( TK_MODEL );
	TK_Beyond( TK_MODEL );

/*	while(1)
    {
        TK_Beyond(TK_MODEL);
        TK_BeyondRequire(TK_NAME, TK_STRING);

        if(_stricmp(tk_String, SKEL_ROOT_NAMES[skelType]) == 0)
            break;
    }*/

	TK_Beyond( TK_SCALING );

	ParseVec3( curScale[currentStack] );

	TK_Beyond( TK_ROTATION );

	ParseRotation3( curRotation[currentStack] );

	TK_Beyond( TK_TRANSLATION );

	ParseVec3( curTranslation[currentStack] );

	// account for global model translation
	curTranslation[currentStack][1] += g_translation[0];
	curTranslation[currentStack][2] += g_translation[1];
	curTranslation[currentStack][0] += g_translation[2];

	++currentStack;

	for ( i = 0; i < NUM_JOINTS_RAVEN; ++i )
	{
		while ( 1 )
		{
			TK_Beyond( TK_MODEL );

//			TK_BeyondRequire(TK_NAME, TK_STRING);

//			if(_stricmp(tk_String, SKEL_NAMES[NAME_OFFSETS[skelType]+i]) == 0)
			break;

			TK_Beyond( TK_SCALING );

			ParseVec3( curScale[currentStack] );

			TK_Beyond( TK_ROTATION );

			ParseRotation3( curRotation[currentStack] );

			TK_Beyond( TK_TRANSLATION );

			ParseVec3( curTranslation[currentStack] );

			curCorrespondingJoint[currentStack] = -1;

			++currentStack;
		}

		TK_Beyond( TK_SCALING );

		ParseVec3( curScale[currentStack] );

		TK_Beyond( TK_ROTATION );

		ParseRotation3( curRotation[currentStack] );

		jointList[i].rotation[1] = curRotation[currentStack][1];
		jointList[i].rotation[2] = curRotation[currentStack][2];
		jointList[i].rotation[0] = curRotation[currentStack][0];

		TK_Beyond( TK_TRANSLATION );

		ParseVec3( curTranslation[currentStack] );

		jointList[i].placement.origin[1] = curTranslation[currentStack][1];
		jointList[i].placement.origin[2] = curTranslation[currentStack][2];
		jointList[i].placement.origin[0] = curTranslation[currentStack][0];

		jointList[i].placement.direction[1] = 7.5;
		jointList[i].placement.direction[2] = 0.0;
		jointList[i].placement.direction[0] = 0.0;

		jointList[i].placement.up[1] = 0.0;
		jointList[i].placement.up[2] = 7.5;
		jointList[i].placement.up[0] = 0.0;

		curCorrespondingJoint[currentStack] = i;

		++currentStack;
	}

	stackSize = currentStack;

	for ( i = 0; i < NUM_JOINTS_RAVEN; ++i )
	{
		rx = jointList[i].rotation[0] * ANGLE_TO_RAD;
		ry = jointList[i].rotation[1] * ANGLE_TO_RAD;
		rz = jointList[i].rotation[2] * ANGLE_TO_RAD;

		cx = cos( rx );
		sx = sin( rx );

		cy = cos( ry );
		sy = sin( ry );

		cz = cos( rz );
		sz = sin( rz );

		// y-axis rotation for direction
		x2 = jointList[i].placement.direction[0] * cy + jointList[i].placement.direction[2] * sy;
		z2 = -jointList[i].placement.direction[0] * sy + jointList[i].placement.direction[2] * cy;
		jointList[i].placement.direction[0] = x2;
		jointList[i].placement.direction[2] = z2;

		// y-axis rotation for up
		x2 = jointList[i].placement.up[0] * cy + jointList[i].placement.up[2] * sy;
		z2 = -jointList[i].placement.up[0] * sy + jointList[i].placement.up[2] * cy;
		jointList[i].placement.up[0] = x2;
		jointList[i].placement.up[2] = z2;

		// z-axis rotation for direction
		x2 = jointList[i].placement.direction[0] * cz - jointList[i].placement.direction[1] * sz;
		y2 = jointList[i].placement.direction[0] * sz + jointList[i].placement.direction[1] * cz;
		jointList[i].placement.direction[0] = x2;
		jointList[i].placement.direction[1] = y2;

		// z-axis rotation for up
		x2 = jointList[i].placement.up[0] * cz - jointList[i].placement.up[1] * sz;
		y2 = jointList[i].placement.up[0] * sz + jointList[i].placement.up[1] * cz;
		jointList[i].placement.up[0] = x2;
		jointList[i].placement.up[1] = y2;

		// x-axis rotation for direction vector
		y2 = jointList[i].placement.direction[1] * cx - jointList[i].placement.direction[2] * sx;
		z2 = jointList[i].placement.direction[1] * sx + jointList[i].placement.direction[2] * cx;
		jointList[i].placement.direction[1] = y2;
		jointList[i].placement.direction[2] = z2;

		// x-axis rotation for up vector
		y2 = jointList[i].placement.up[1] * cx - jointList[i].placement.up[2] * sx;
		z2 = jointList[i].placement.up[1] * sx + jointList[i].placement.up[2] * cx;
		jointList[i].placement.up[1] = y2;
		jointList[i].placement.up[2] = z2;

		// translate to position in model
		jointList[i].placement.direction[0] += jointList[i].placement.origin[0];
		jointList[i].placement.direction[1] += jointList[i].placement.origin[1];
		jointList[i].placement.direction[2] += jointList[i].placement.origin[2];

		// translate to position in model
		jointList[i].placement.up[0] += jointList[i].placement.origin[0];
		jointList[i].placement.up[1] += jointList[i].placement.origin[1];
		jointList[i].placement.up[2] += jointList[i].placement.origin[2];
	}

	baseJoint = NUM_JOINTS_RAVEN;

	for ( i = stackSize /*NUM_JOINTS_RAVEN*/ - 1; i > 0; --i )
	{

		rx = curRotation[i - 1][0] * ANGLE_TO_RAD;
		ry = curRotation[i - 1][1] * ANGLE_TO_RAD;
		rz = curRotation[i - 1][2] * ANGLE_TO_RAD;

		cx = cos( rx );
		sx = sin( rx );

		cy = cos( ry );
		sy = sin( ry );

		cz = cos( rz );
		sz = sin( rz );

		for ( j = i - 1; j < stackSize - 1; ++j )
		{
			// y-axis rotation for origin
			x2 = jointList[j].placement.origin[0] * cy + jointList[j].placement.origin[2] * sy;
			z2 = -jointList[j].placement.origin[0] * sy + jointList[j].placement.origin[2] * cy;
			jointList[j].placement.origin[0] = x2;
			jointList[j].placement.origin[2] = z2;

			// y-axis rotation for direction
			x2 = jointList[j].placement.direction[0] * cy + jointList[j].placement.direction[2] * sy;
			z2 = -jointList[j].placement.direction[0] * sy + jointList[j].placement.direction[2] * cy;
			jointList[j].placement.direction[0] = x2;
			jointList[j].placement.direction[2] = z2;

			// y-axis rotation for up
			x2 = jointList[j].placement.up[0] * cy + jointList[j].placement.up[2] * sy;
			z2 = -jointList[j].placement.up[0] * sy + jointList[j].placement.up[2] * cy;
			jointList[j].placement.up[0] = x2;
			jointList[j].placement.up[2] = z2;

			// z-axis rotation for origin
			x2 = jointList[j].placement.origin[0] * cz - jointList[j].placement.origin[1] * sz;
			y2 = jointList[j].placement.origin[0] * sz + jointList[j].placement.origin[1] * cz;
			jointList[j].placement.origin[0] = x2;
			jointList[j].placement.origin[1] = y2;

			// z-axis rotation for direction
			x2 = jointList[j].placement.direction[0] * cz - jointList[j].placement.direction[1] * sz;
			y2 = jointList[j].placement.direction[0] * sz + jointList[j].placement.direction[1] * cz;
			jointList[j].placement.direction[0] = x2;
			jointList[j].placement.direction[1] = y2;

			// z-axis rotation for up
			x2 = jointList[j].placement.up[0] * cz - jointList[j].placement.up[1] * sz;
			y2 = jointList[j].placement.up[0] * sz + jointList[j].placement.up[1] * cz;
			jointList[j].placement.up[0] = x2;
			jointList[j].placement.up[1] = y2;

			// x-axis rotation for origin
			y2 = jointList[j].placement.origin[1] * cx - jointList[j].placement.origin[2] * sx;
			z2 = jointList[j].placement.origin[1] * sx + jointList[j].placement.origin[2] * cx;
			jointList[j].placement.origin[1] = y2;
			jointList[j].placement.origin[2] = z2;

			// x-axis rotation for direction vector
			y2 = jointList[j].placement.direction[1] * cx - jointList[j].placement.direction[2] * sx;
			z2 = jointList[j].placement.direction[1] * sx + jointList[j].placement.direction[2] * cx;
			jointList[j].placement.direction[1] = y2;
			jointList[j].placement.direction[2] = z2;

			// x-axis rotation for up vector
			y2 = jointList[j].placement.up[1] * cx - jointList[j].placement.up[2] * sx;
			z2 = jointList[j].placement.up[1] * sx + jointList[j].placement.up[2] * cx;
			jointList[j].placement.up[1] = y2;
			jointList[j].placement.up[2] = z2;

			if ( curCorrespondingJoint[j + 1] != -1 ) {
				// translate origin
				jointList[j].placement.origin[0] += curTranslation[i - 1][0];
				jointList[j].placement.origin[1] += curTranslation[i - 1][1];
				jointList[j].placement.origin[2] += curTranslation[i - 1][2];

				// translate back to local coord
				jointList[j].placement.direction[0] += curTranslation[i - 1][0];
				jointList[j].placement.direction[1] += curTranslation[i - 1][1];
				jointList[j].placement.direction[2] += curTranslation[i - 1][2];

				// translate back to local coord
				jointList[j].placement.up[0] += curTranslation[i - 1][0];
				jointList[j].placement.up[1] += curTranslation[i - 1][1];
				jointList[j].placement.up[2] += curTranslation[i - 1][2];
			}
		}
	}
}

void LoadGlobals( char *fileName ){
	FILE *file1;
	int dot = '.';
	char *dotstart;
	char InputFileName[256];

	dotstart = strrchr( fileName,dot ); // Does it already have an extension on the file name?

	if ( !dotstart ) {
		strcpy( InputFileName, fileName );
		strcat( InputFileName, ".hrc" );
		if ( ( file1 = fopen( InputFileName, "rb" ) ) != NULL ) {
			fclose( file1 );

			LoadHRCGlobals( InputFileName );

			printf( " - assuming .HRC\n" );
			return;
		}

		Error( "\n Could not open file '%s':\n"
			   "No HRC match.\n", fileName );
	}
	else
	{
		if ( ( file1 = fopen( fileName, "rb" ) ) != NULL ) {
			printf( "\n" );
			fclose( file1 );
			if ( strcmp( dotstart,".hrc" ) == 0 || strcmp( dotstart,".HRC" ) == 0 ) {
				LoadHRCGlobals( fileName );
				return;
			}
		}

		Error( "Could not open file '%s':\n",fileName );
	}
}

void LoadClusters( char *fileName, int **clusterList, int *num_verts, int skelType ){
	FILE *file1;
	int dot = '.';
	char *dotstart;
	char InputFileName[256];

	dotstart = strrchr( fileName,dot ); // Does it already have an extension on the file name?

	if ( !dotstart ) {
		strcpy( InputFileName, fileName );
		strcat( InputFileName, ".hrc" );
		if ( ( file1 = fopen( InputFileName, "rb" ) ) != NULL ) {
			fclose( file1 );

			LoadHRCClustered( InputFileName, clusterList, num_verts, skelType );

			printf( " - assuming .HRC\n" );
			return;
		}

		Error( "\n Could not open file '%s':\n"
			   "No HRC match.\n", fileName );
	}
	else
	{
		if ( ( file1 = fopen( fileName, "rb" ) ) != NULL ) {
			printf( "\n" );
			fclose( file1 );
			if ( strcmp( dotstart,".hrc" ) == 0 || strcmp( dotstart,".HRC" ) == 0 ) {
				LoadHRCClustered( fileName, clusterList, num_verts, skelType );
				return;
			}
		}

		Error( "Could not open file '%s':\n",fileName );
	}
}

void LoadJointList( char *fileName, QDataJoint_t *jointList, int skelType ){
	FILE *file1;
	int dot = '.';
	char *dotstart;
	char InputFileName[256];

	dotstart = strrchr( fileName,dot ); // Does it already have an extension on the file name?

	if ( !dotstart ) {
		strcpy( InputFileName, fileName );
		strcat( InputFileName, ".hrc" );
		if ( ( file1 = fopen( InputFileName, "rb" ) ) != NULL ) {
			fclose( file1 );

			LoadHRCJointList( InputFileName, jointList, skelType );

			printf( " - assuming .HRC\n" );
			return;
		}

		Error( "\n Could not open file '%s':\n"
			   "No HRC.\n", fileName );
	}
	else
	{
		if ( ( file1 = fopen( fileName, "rb" ) ) != NULL ) {
			printf( "\n" );
			fclose( file1 );
			if ( strcmp( dotstart,".hrc" ) == 0 || strcmp( dotstart,".HRC" ) == 0 ) {
				LoadHRCJointList( fileName, jointList, skelType );

				return;
			}
		}

		Error( "Could not open file '%s':\n",fileName );
	}
}
