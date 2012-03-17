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

#include "qd_skeletons.h"
#include "skeletons.h"
#include "qd_fmodel.h"
#include "angles.h"
#include "token.h"
#include "qdata.h"
#include "reference.h"

#include <assert.h>
#include <math.h>
#include <memory.h>


// We're assuming no more than 16 reference points, with no more than 32 characters in the name
char RefPointNameList[REF_MAX_POINTS][REF_MAX_STRLEN];
int RefPointNum = 0;

Skeletalfmheader_t g_skelModel;

void ClearSkeletalModel(){
	g_skelModel.type = SKEL_NULL;
	g_skelModel.clustered = false;
	g_skelModel.references = REF_NULL;
}

//==========================================================================
//
// LoadHRCClustered
//
//==========================================================================

// Places the null terminated src string into the dest string less any trailing digits or underscores
void StripTrailingDigits( char *src, char *dest ){
#ifndef NDEBUG
	int max = SKELETAL_NAME_MAX; // should be sufficient for inteded use on names from hrc files
#endif
	int i = 0;

	while ( src[i] != '\0' )
	{
		++i;
#ifndef NDEBUG
		assert( i < max );
#endif
	}

	while ( ( src[--i] >= '0' && src[i] <= '9' ) || src[i] == '_' )
	{

	}

	memcpy( dest, src, ++i );

	dest[i] = '\0';
}

static void LoadHRCClustered( char *fileName, int **clusterList, int *num_verts, int skelType ){
	extern void HandleHRCModel( triangle_t **triList, int *triangleCount,
								mesh_node_t **nodesList, int *num_mesh_nodes, int ActiveNode, int Depth );

	extern mesh_node_t  *pmnodes;

	triangle_t *triList;
//      mesh_node_t *nodesList;
	int num_mesh_nodes = 0, triangleCount = 0;

#if 0
	int i;
	int j, numVerts;
	char stripped[SKELETAL_NAME_MAX];

	for ( i = 1; i < numJointsInSkeleton[skelType] + 1; ++i )
	{
		num_verts[i] = 0;
	}

	TK_OpenSource( fileName );
	TK_FetchRequire( TK_HRCH );
	TK_FetchRequire( TK_COLON );
	TK_FetchRequire( TK_SOFTIMAGE );

	TK_Beyond( TK_CLUSTERS );

	while ( TK_Search( TK_CLUSTER_NAME ) != TK_EOF )
	{
		TK_Require( TK_STRING );

		StripTrailingDigits( tk_String, stripped );

		for ( i = 0; i < numJointsInSkeleton[skelType]; ++i )
		{
			if ( stricmp( stripped, skeletonJointNames[skeletonNameOffsets[skelType] + i] ) == 0 ) {
				i = -i + numJointsInSkeleton[skelType] - 1;

				TK_BeyondRequire( TK_NUM_CLUSTER_VERTICES, TK_INTNUMBER );

				numVerts = tk_IntNumber;

				if ( !num_verts[i + 1] ) { // first set of verts for cluster
					clusterList[i] = SafeMalloc( numVerts * sizeof( int ), "LoadHRCClustered" );
					assert( clusterList[i] );
				}
				else                // any later sets of verts need to copy current
				{
					int *temp;

					temp = SafeMalloc( ( num_verts[i + 1] + numVerts ) * sizeof( int ), "LoadHRCClustered" );
					assert( temp );

					memcpy( temp + numVerts, clusterList[i], num_verts[i + 1] * sizeof( int ) );

					free( clusterList[i] );

					clusterList[i] = temp;
				}

				// currently this function is only called by LoadModelClusters.
				// Apparently the matching free has disappeared,
				// should probably be free at the end of FMCmd_Base

				TK_Beyond( TK_LBRACE );

				for ( j = 0; j < numVerts; ++j )
				{
					TK_Require( TK_INTNUMBER );
					clusterList[i][j] = tk_IntNumber;
					TK_Fetch();
				}

				num_verts[i + 1] += numVerts;

				break;
			}
		}
	}

	num_verts[0] = numJointsInSkeleton[skelType];
#endif

#if 1   // get the index number localized to the root
//	for( i = 1; i < numJointsInSkeleton[skelType] + 1; ++i)
//	{
//		g_skelModel.num_verts[i] = 0;
//	}

	TK_OpenSource( fileName );
	TK_FetchRequire( TK_HRCH );
	TK_FetchRequire( TK_COLON );
	TK_FetchRequire( TK_SOFTIMAGE );

	// prime it
	TK_Beyond( TK_MODEL );

	triList = (triangle_t *) SafeMalloc( MAXTRIANGLES * sizeof( triangle_t ), "Triangle list" );
	memset( triList,0,MAXTRIANGLES * sizeof( triangle_t ) );
//	nodesList = SafeMalloc(MAX_FM_MESH_NODES * sizeof(mesh_node_t), "Mesh Node List");
	pmnodes = (mesh_node_t *) SafeMalloc( MAX_FM_MESH_NODES * sizeof( mesh_node_t ), "Mesh Node List" );

	memset( pmnodes, 0, MAX_FM_MESH_NODES * sizeof( mesh_node_t ) );

	// this should eventually use a stripped down version of this
	HandleHRCModel( &triList, &triangleCount, &pmnodes, &num_mesh_nodes, 0, 0 );

//	free(nodesList);
	free( triList );

	num_verts[0] = numJointsInSkeleton[skelType];
#endif
}

void ReadHRCClusterList( mesh_node_t *meshNode, int baseIndex ){
	int i, j, numVerts;
	tokenType_t nextToken;
	char stripped[SKELETAL_NAME_MAX];

	meshNode->clustered = true;

	nextToken = TK_Get( TK_CLUSTER_NAME );

	while ( nextToken == TK_CLUSTER_NAME )
	{
		TK_FetchRequire( TK_STRING );

		StripTrailingDigits( tk_String, stripped );

		for ( i = 0; i < numJointsInSkeleton[g_skelModel.type]; ++i )
		{
			if ( stricmp( stripped, skeletonJointNames[skeletonNameOffsets[g_skelModel.type] + i] ) == 0 ) {
				i = -i + numJointsInSkeleton[g_skelModel.type] - 1;

				TK_BeyondRequire( TK_NUM_CLUSTER_VERTICES, TK_INTNUMBER );

				numVerts = tk_IntNumber;

				if ( !baseIndex ) {
					meshNode->clusters[i] = (int *) SafeMalloc( numVerts * sizeof( int ), "ReadHRCClusterList" );
					assert( meshNode->clusters[i] );
				}
				else
				{
					int *temp;

					temp = meshNode->clusters[i];
					meshNode->clusters[i] = (int *) SafeMalloc( ( meshNode->num_verts[i + 1] + numVerts ) * sizeof( int ), "ReadHRCClusterList" );
					assert( meshNode->clusters[i] );

					memcpy( meshNode->clusters[i], temp, meshNode->num_verts[i + 1] * sizeof( int ) );
					free( temp );
				}

				// currently this function is only called by LoadModelClusters.
				// Apparently the matching free has disappeared,
				// should probably be free at the end of FMCmd_Base

				TK_Beyond( TK_LBRACE );

				for ( j = 0; j < numVerts; ++j )
				{
					TK_Require( TK_INTNUMBER );
					meshNode->clusters[i][baseIndex + j] = tk_IntNumber + baseIndex;
					TK_Fetch();
				}

				if ( baseIndex ) {
					meshNode->num_verts[i + 1] += numVerts;
				}
				else
				{
					meshNode->num_verts[i + 1] = numVerts;
				}

				break;
			}
		}

		TK_BeyondRequire( TK_CLUSTER_STATE, TK_INTNUMBER );
		nextToken = TK_Fetch();
	}
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
		g_skelModel.scaling[i] = tk_FloatNumber;
		TK_Fetch();
	}

	TK_Beyond( TK_ROTATION );
	for ( i = 0; i < 3; i++ )
	{
		TK_Require( TK_FLOATNUMBER );
		g_skelModel.rotation[i] = tk_FloatNumber;
		TK_Fetch();
	}

	TK_Beyond( TK_TRANSLATION );
	for ( i = 0; i < 3; i++ )
	{
		TK_Require( TK_FLOATNUMBER );
		g_skelModel.translation[i] = tk_FloatNumber;
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

static void ParseVec3d( vec3d_t in ){
	TK_Require( TK_FLOATNUMBER );
	in[1] = tk_FloatNumber;
	TK_FetchRequire( TK_FLOATNUMBER );
	in[2] = tk_FloatNumber;
	TK_FetchRequire( TK_FLOATNUMBER );
	in[0] = tk_FloatNumber;
}

static void ParseRotation3( vec3_t in ){
	TK_Require( TK_FLOATNUMBER );
	in[1] = tk_FloatNumber;
	TK_FetchRequire( TK_FLOATNUMBER );
	in[2] = tk_FloatNumber;
	TK_FetchRequire( TK_FLOATNUMBER );
	in[0] = tk_FloatNumber;
}

static void ParseRotation3d( vec3d_t in ){
	TK_Require( TK_FLOATNUMBER );
	in[1] = tk_FloatNumber;
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

static void ParseTranslation3d( vec3d_t in ){
	TK_Require( TK_FLOATNUMBER );
	in[1] = tk_FloatNumber;
	TK_FetchRequire( TK_FLOATNUMBER );
	in[2] = tk_FloatNumber;
	TK_FetchRequire( TK_FLOATNUMBER );
	in[0] = tk_FloatNumber;
}

static void LoadHRCJointList( char *fileName, QD_SkeletalJoint_t *jointList, int skelType ){
#define MAX_STACK 64
	int i, j;
	vec3d_t curTranslation[MAX_STACK], curRotation[MAX_STACK], curScale[MAX_STACK];
	int curCorrespondingJoint[MAX_STACK];
	int currentStack = 0, stackSize;
	double cx, sx, cy, sy, cz, sz;
	double rx, ry, rz;
	double x2, y2, z2;
	char stripped[SKELETAL_NAME_MAX];
	Placement_d_t *placement;

	TK_OpenSource( fileName );
	TK_FetchRequire( TK_HRCH );
	TK_FetchRequire( TK_COLON );
	TK_FetchRequire( TK_SOFTIMAGE );

	TK_Beyond( TK_MODEL );

	while ( TK_Search( TK_NAME ) != TK_EOF )
	{
		TK_Require( TK_STRING );

		StripTrailingDigits( tk_String, stripped );

		if ( stricmp( stripped, skeletonRootNames[skeletonRNameOffsets[skelType]] ) == 0 ) {
			break;
		}
	}

	if ( tk_Token == TK_EOF ) {
		Error( "Bone Chain Root: %s not found\n", skeletonRootNames[skeletonRNameOffsets[skelType]] );
		return;
	}

	TK_Beyond( TK_SCALING );

	ParseVec3d( curScale[currentStack] );

	TK_Beyond( TK_ROTATION );

	ParseRotation3d( curRotation[currentStack] );

	TK_Beyond( TK_TRANSLATION );

	ParseVec3d( curTranslation[currentStack] );

	// account for global model translation
	curTranslation[currentStack][1] += g_skelModel.translation[0];
	curTranslation[currentStack][2] += g_skelModel.translation[1];
	curTranslation[currentStack][0] += g_skelModel.translation[2];

	curCorrespondingJoint[currentStack] = -1;

	++currentStack;

	for ( i = 0; i < numJointsInSkeleton[skelType]; ++i )
	{
		while ( 1 )
		{
			TK_Beyond( TK_MODEL );

			TK_BeyondRequire( TK_NAME, TK_STRING );

			StripTrailingDigits( tk_String, stripped );

			if ( stricmp( stripped, skeletonJointNames[skeletonNameOffsets[skelType] + i] ) == 0 ) {
				break;
			}

			TK_Beyond( TK_SCALING );

			ParseVec3d( curScale[currentStack] );

			TK_Beyond( TK_ROTATION );

			ParseRotation3d( curRotation[currentStack] );

			TK_Beyond( TK_TRANSLATION );

			ParseVec3d( curTranslation[currentStack] );

			curCorrespondingJoint[currentStack] = -1;

			++currentStack;
		}

		TK_Beyond( TK_SCALING );

		ParseVec3d( curScale[currentStack] );

		TK_Beyond( TK_ROTATION );

		ParseRotation3d( curRotation[currentStack] );

		jointList[i].rotation[1] = curRotation[currentStack][1];
		jointList[i].rotation[2] = curRotation[currentStack][2];
		jointList[i].rotation[0] = curRotation[currentStack][0];

		TK_Beyond( TK_TRANSLATION );

		ParseVec3d( curTranslation[currentStack] );

//		jointList[i].placement.origin[1] = curTranslation[currentStack][1];
//		jointList[i].placement.origin[2] = curTranslation[currentStack][2];
//		jointList[i].placement.origin[0] = curTranslation[currentStack][0];

		jointList[i].placement.origin[1] = 0.0;
		jointList[i].placement.origin[2] = 0.0;
		jointList[i].placement.origin[0] = 0.0;

		jointList[i].placement.direction[1] = 20.0;
		jointList[i].placement.direction[2] = 0.0;
		jointList[i].placement.direction[0] = 0.0;

		jointList[i].placement.up[1] = 0.0;
		jointList[i].placement.up[2] = 20.0;
		jointList[i].placement.up[0] = 0.0;

		curCorrespondingJoint[currentStack] = i;

		++currentStack;
	}

	stackSize = currentStack;

#if 0
	// rotate the direction and up vectors to correspond to the rotation
	for ( i = 0; i < numJointsInSkeleton[skelType]; ++i )
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

		// translate direction to a point in the model
		jointList[i].placement.direction[0] += jointList[i].placement.origin[0];
		jointList[i].placement.direction[1] += jointList[i].placement.origin[1];
		jointList[i].placement.direction[2] += jointList[i].placement.origin[2];

		// translate up to a point in the model
		jointList[i].placement.up[0] += jointList[i].placement.origin[0];
		jointList[i].placement.up[1] += jointList[i].placement.origin[1];
		jointList[i].placement.up[2] += jointList[i].placement.origin[2];
	}
#endif

	for ( i = stackSize - 1; i >= 0; --i )
	{
		rx = curRotation[i][0] * ANGLE_TO_RAD;
		ry = curRotation[i][1] * ANGLE_TO_RAD;
		rz = curRotation[i][2] * ANGLE_TO_RAD;

		cx = cos( rx );
		sx = sin( rx );

		cy = cos( ry );
		sy = sin( ry );

		cz = cos( rz );
		sz = sin( rz );

#if 1
		for ( j = i; j < stackSize; ++j )
		{
			if ( curCorrespondingJoint[j] != -1 ) {
				placement = &jointList[curCorrespondingJoint[j]].placement;

				// y-axis rotation for origin
				x2 = placement->origin[0] * cy + placement->origin[2] * sy;
				z2 = -placement->origin[0] * sy + placement->origin[2] * cy;
				placement->origin[0] = x2;
				placement->origin[2] = z2;

				// y-axis rotation for direction
				x2 = placement->direction[0] * cy + placement->direction[2] * sy;
				z2 = -placement->direction[0] * sy + placement->direction[2] * cy;
				placement->direction[0] = x2;
				placement->direction[2] = z2;

				// y-axis rotation for up
				x2 = placement->up[0] * cy + placement->up[2] * sy;
				z2 = -placement->up[0] * sy + placement->up[2] * cy;
				placement->up[0] = x2;
				placement->up[2] = z2;

				// z-axis rotation for origin
				x2 = placement->origin[0] * cz - placement->origin[1] * sz;
				y2 = placement->origin[0] * sz + placement->origin[1] * cz;
				placement->origin[0] = x2;
				placement->origin[1] = y2;

				// z-axis rotation for direction
				x2 = placement->direction[0] * cz - placement->direction[1] * sz;
				y2 = placement->direction[0] * sz + placement->direction[1] * cz;
				placement->direction[0] = x2;
				placement->direction[1] = y2;

				// z-axis rotation for up
				x2 = placement->up[0] * cz - placement->up[1] * sz;
				y2 = placement->up[0] * sz + placement->up[1] * cz;
				placement->up[0] = x2;
				placement->up[1] = y2;

				// x-axis rotation for origin
				y2 = placement->origin[1] * cx - placement->origin[2] * sx;
				z2 = placement->origin[1] * sx + placement->origin[2] * cx;
				placement->origin[1] = y2;
				placement->origin[2] = z2;

				// x-axis rotation for direction vector
				y2 = placement->direction[1] * cx - placement->direction[2] * sx;
				z2 = placement->direction[1] * sx + placement->direction[2] * cx;
				placement->direction[1] = y2;
				placement->direction[2] = z2;

				// x-axis rotation for up vector
				y2 = placement->up[1] * cx - placement->up[2] * sx;
				z2 = placement->up[1] * sx + placement->up[2] * cx;
				placement->up[1] = y2;
				placement->up[2] = z2;

				// translate origin
				placement->origin[0] += curTranslation[i][0];
				placement->origin[1] += curTranslation[i][1];
				placement->origin[2] += curTranslation[i][2];

				// translate back to local coord
				placement->direction[0] += curTranslation[i][0];
				placement->direction[1] += curTranslation[i][1];
				placement->direction[2] += curTranslation[i][2];

				// translate back to local coord
				placement->up[0] += curTranslation[i][0];
				placement->up[1] += curTranslation[i][1];
				placement->up[2] += curTranslation[i][2];
			}
		}
#else
		// This screwed up and needs to be sorted out!!!
		// The stack info needs to be written too instead of the jointList for j > numJoints for Skeleton
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
#endif
	}
}

void LoadModelTransform( char *fileName ){
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
//			printf("\n");
			fclose( file1 );
			if ( strcmp( dotstart,".hrc" ) == 0 || strcmp( dotstart,".HRC" ) == 0 ) {
				LoadHRCGlobals( fileName );
				return;
			}
		}

		Error( "Could not open file '%s':\n",fileName );
	}
}

void LoadModelClusters( char *fileName, int **clusterList, int *num_verts, int skelType ){
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
//			printf("\n");
			fclose( file1 );
			if ( strcmp( dotstart,".hrc" ) == 0 || strcmp( dotstart,".HRC" ) == 0 ) {
				LoadHRCClustered( fileName, clusterList, num_verts, skelType );
				return;
			}
		}

		Error( "Could not open file '%s':\n",fileName );
	}
}

void LoadSkeleton( char *fileName, QD_SkeletalJoint_t *jointList, int skelType ){
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
//			printf("\n");
			fclose( file1 );
			if ( strcmp( dotstart,".hrc" ) == 0 || strcmp( dotstart,".HRC" ) == 0 ) {
				LoadHRCJointList( fileName, jointList, skelType );

				return;
			}
		}

		Error( "Could not open file '%s':\n",fileName );
	}
}

/*
   ===============
   GrabSkeletalFrame
   ===============
 */
void GrabSkeletalFrame( char *frame ){
	char file1[1024];
	char    *framefile;
	fmframe_t   *fr;

	framefile = FindFrameFile( frame );

	sprintf( file1, "%s/%s", cdarchive, framefile );
	ExpandPathAndArchive( file1 );

	sprintf( file1, "%s/%s",cddir, framefile );

	printf( "Grabbing Skeletal Frame %s\n", file1 );

	fr = &g_frames[fmheader.num_frames - 1]; // last frame read in

	LoadSkeleton( file1, fr->joints, g_skelModel.type );
}

/*
   ===============
   GrabModelTransform
   ===============
 */
void GrabModelTransform( char *frame ){
	char file1[1024];
	char    *framefile;
	fmframe_t   *fr;

	framefile = FindFrameFile( frame );

	sprintf( file1, "%s/%s", cdarchive, framefile );
	ExpandPathAndArchive( file1 );

	sprintf( file1, "%s/%s",cddir, framefile );

//	printf ("grabbing %s\n", file1);

	fr = &g_frames[fmheader.num_frames - 1]; // last frame read in

	LoadModelTransform( file1 );
}

void Cmd_FMCluster(){
	char file1[1024];

	GetScriptToken( false );

	printf( "---------------------\n" );
	sprintf( file1, "%s/%s", cdpartial, token );
	printf( "%s\n", file1 );

	ExpandPathAndArchive( file1 );

	sprintf( file1, "%s/%s", cddir, token );

	g_skelModel.clustered = -1;

	LoadModelClusters( file1, (int **)&g_skelModel.clusters, (int *)&g_skelModel.num_verts, g_skelModel.type );

	g_skelModel.new_num_verts[0] = g_skelModel.num_verts[0];

	g_skelModel.clustered = true;
}

void Cmd_FMSkeleton(){
	GetScriptToken( false );
	g_skelModel.type = atoi( token );
}

void Cmd_FMSkeletalFrame(){
	while ( ScriptTokenAvailable() )
	{
		GetScriptToken( false );
		if ( g_skipmodel ) {
			GetScriptToken( false );
			continue;
		}
		if ( g_release || g_archive ) {
			fmheader.num_frames = 1;    // don't skip the writeout
			GetScriptToken( false );
			continue;
		}

		H_printf( "#define FRAME_%-16s\t%i\n", token, fmheader.num_frames );

		GrabModelTransform( token );
		GrabFrame( token );
		GrabSkeletalFrame( token );

		// need to add the up and dir points to the frame bounds here
		// using AddPointToBounds (ptrivert[index_xyz].v, fr->mins, fr->maxs);
		// then remove fudge in determining scale on frame write out
	}
}

static void LoadHRCReferences( char *fileName, fmframe_t *fr ){
#define MAX_STACK 64
	int i, j, k;
	vec3d_t curTranslation[MAX_STACK], curRotation[MAX_STACK];
	int curCorrespondingJoint[MAX_STACK];
	int currentStack, stackSize;
	double cx, sx, cy, sy, cz, sz;
	double rx, ry, rz;
	double x2, y2, z2;
	char stripped[SKELETAL_NAME_MAX];
	Placement_d_t *placement;
	int refnum;

	TK_OpenSource( fileName );
	TK_FetchRequire( TK_HRCH );
	TK_FetchRequire( TK_COLON );
	TK_FetchRequire( TK_SOFTIMAGE );

	if ( RefPointNum <= 0 ) { // There were no labels indicated in the QDT, so use the hard-coded stuff.
		refnum = numReferences[g_skelModel.references];
	}
	else
	{
		refnum = RefPointNum;
	}

	for ( k = 0; k < refnum; ++k )
	{
		currentStack = 0;

		// Load the root to get translation and initial rotation
//		TK_Beyond(TK_MODEL);

		while ( TK_Search( TK_NAME ) != TK_EOF )
		{
			TK_Require( TK_STRING );

			StripTrailingDigits( tk_String, stripped );

			if ( RefPointNum == 0 ) { // Hard coded refpoint labels
				if ( stricmp( stripped,
							  referenceRootNames[referenceRootNameOffsets[g_skelModel.references] + k] ) == 0 ) {
					break;
				}
			}
			else
			{   // labels indicated by the QDT
				if ( stricmp( stripped, RefPointNameList[k] ) == 0 ) {
					break;
				}
			}
		}

		if ( tk_Token == TK_EOF ) {
			if ( RefPointNum == 0 ) { // Hard coded refpoint labels
				Error( "Bone Chain Root: %s not found\n", referenceRootNames[referenceRootNameOffsets[g_skelModel.references]] );
			}
			else
			{   // labels indicated by the QDT
				Error( "Bone Chain Root: %s not found\n", RefPointNameList[k] );
			}
			return;
		}

//		TK_Beyond(TK_SCALING);

//		ParseVec3d(curScale[currentStack]);

		TK_Beyond( TK_ROTATION );

		ParseRotation3d( curRotation[currentStack] );

		TK_Beyond( TK_TRANSLATION );

		ParseVec3d( curTranslation[currentStack] );

		// account for global model translation
		curTranslation[currentStack][1] += g_skelModel.translation[0];
		curTranslation[currentStack][2] += g_skelModel.translation[1];
		curTranslation[currentStack][0] += g_skelModel.translation[2];

		curCorrespondingJoint[currentStack] = -1;

//		rjr - this one not needed, as there is also a stack increment 20 lines below???
//		++currentStack;

		// Load the joint to get orientation
		TK_Beyond( TK_MODEL );

//		TK_Beyond(TK_SCALING);

//		ParseVec3d(curScale[currentStack]);

		TK_Beyond( TK_ROTATION );

		ParseRotation3d( curRotation[currentStack] );

//		TK_Beyond(TK_TRANSLATION);

//		ParseVec3d(curTranslation[currentStack]);

		fr->references[k].placement.origin[1] = 0.0;
		fr->references[k].placement.origin[2] = 0.0;
		fr->references[k].placement.origin[0] = 0.0;

		fr->references[k].placement.direction[1] = 20.0;
		fr->references[k].placement.direction[2] = 0.0;
		fr->references[k].placement.direction[0] = 0.0;

		fr->references[k].placement.up[1] = 0.0;
		fr->references[k].placement.up[2] = 20.0;
		fr->references[k].placement.up[0] = 0.0;

		curCorrespondingJoint[currentStack] = k;

		++currentStack;

		stackSize = currentStack;

		for ( i = stackSize - 1; i >= 0; --i )
		{
			rx = curRotation[i][0] * ANGLE_TO_RAD;
			ry = curRotation[i][1] * ANGLE_TO_RAD;
			rz = curRotation[i][2] * ANGLE_TO_RAD;

			cx = cos( rx );
			sx = sin( rx );

			cy = cos( ry );
			sy = sin( ry );

			cz = cos( rz );
			sz = sin( rz );

			for ( j = i; j < stackSize; ++j )
			{
				if ( curCorrespondingJoint[j] != -1 ) {
					placement = &fr->references[curCorrespondingJoint[j]].placement;

					// y-axis rotation for origin
					x2 = placement->origin[0] * cy + placement->origin[2] * sy;
					z2 = -placement->origin[0] * sy + placement->origin[2] * cy;
					placement->origin[0] = x2;
					placement->origin[2] = z2;

					// y-axis rotation for direction
					x2 = placement->direction[0] * cy + placement->direction[2] * sy;
					z2 = -placement->direction[0] * sy + placement->direction[2] * cy;
					placement->direction[0] = x2;
					placement->direction[2] = z2;

					// y-axis rotation for up
					x2 = placement->up[0] * cy + placement->up[2] * sy;
					z2 = -placement->up[0] * sy + placement->up[2] * cy;
					placement->up[0] = x2;
					placement->up[2] = z2;

					// z-axis rotation for origin
					x2 = placement->origin[0] * cz - placement->origin[1] * sz;
					y2 = placement->origin[0] * sz + placement->origin[1] * cz;
					placement->origin[0] = x2;
					placement->origin[1] = y2;

					// z-axis rotation for direction
					x2 = placement->direction[0] * cz - placement->direction[1] * sz;
					y2 = placement->direction[0] * sz + placement->direction[1] * cz;
					placement->direction[0] = x2;
					placement->direction[1] = y2;

					// z-axis rotation for up
					x2 = placement->up[0] * cz - placement->up[1] * sz;
					y2 = placement->up[0] * sz + placement->up[1] * cz;
					placement->up[0] = x2;
					placement->up[1] = y2;

					// x-axis rotation for origin
					y2 = placement->origin[1] * cx - placement->origin[2] * sx;
					z2 = placement->origin[1] * sx + placement->origin[2] * cx;
					placement->origin[1] = y2;
					placement->origin[2] = z2;

					// x-axis rotation for direction vector
					y2 = placement->direction[1] * cx - placement->direction[2] * sx;
					z2 = placement->direction[1] * sx + placement->direction[2] * cx;
					placement->direction[1] = y2;
					placement->direction[2] = z2;

					// x-axis rotation for up vector
					y2 = placement->up[1] * cx - placement->up[2] * sx;
					z2 = placement->up[1] * sx + placement->up[2] * cx;
					placement->up[1] = y2;
					placement->up[2] = z2;

					// translate origin
					placement->origin[0] += curTranslation[i][0];
					placement->origin[1] += curTranslation[i][1];
					placement->origin[2] += curTranslation[i][2];

					// translate back to local coord
					placement->direction[0] += curTranslation[i][0];
					placement->direction[1] += curTranslation[i][1];
					placement->direction[2] += curTranslation[i][2];

					// translate back to local coord
					placement->up[0] += curTranslation[i][0];
					placement->up[1] += curTranslation[i][1];
					placement->up[2] += curTranslation[i][2];

				}
			}
		}
		printf( "%f, %f, %f\n", placement->origin[0], placement->origin[1], placement->origin[2] );
	}
	printf( "\n" );
}

void Cmd_FMReferenced(){
	int i;

	GetScriptToken( false );
	g_skelModel.references = atoi( token );

	// Guess what?  Now, we now want a list of strings to look for here instead of a hard-coded list
	for ( i = 0; i < REF_MAX_POINTS; i++ )
	{
		if ( ScriptTokenAvailable() ) { // There is yet another reference point waiting.
			GetScriptToken( false );
			strcpy( RefPointNameList[i], token );
		}
		else
		{
			break;
		}
	}

	RefPointNum = i;

	if ( RefPointNum > 0 ) {
		printf( "Searching for %d different reference points.\n", RefPointNum );
	}
	else
	{
		printf( "Using built-in reference points.\n" );
	}

}

void LoadReferences( char *fileName, fmframe_t *fr ){
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

			LoadHRCReferences( InputFileName, fr );

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
				LoadHRCReferences( fileName, fr );

				return;
			}
		}

		Error( "Could not open file '%s':\n",fileName );
	}
}

void GrabReferencedFrame( char *frame ){
	char file1[1024];
	char    *framefile;
	fmframe_t   *fr;

	framefile = FindFrameFile( frame );

	sprintf( file1, "%s/%s", cdarchive, framefile );
	ExpandPathAndArchive( file1 );

	sprintf( file1, "%s/%s",cddir, framefile );

	printf( "Grabbing Referenced %s\n", file1 );

	fr = &g_frames[fmheader.num_frames - 1]; // last frame read in

	LoadReferences( file1, fr );
}
