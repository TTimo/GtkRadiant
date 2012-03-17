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
// Skeletons.c
//

#include "skeletons.h"

char *skeletonRootNames[] =
{
	"RAVEN_ROOT",
	"BOX_ROOT",
	"BEETLE_ROOT",
	"ELFLORD_ROOT",
	"PLAGUELF_ROOT",
	"ELF_BACKROOT",
};

int skeletonRNameOffsets[] =
{
	0,  // RAVEN
	1,  // BOX
	2,  // BEETLE
	3,  // ELFLORD
	4,  // PLAGUE ELF
	5,  // CORVUS
};

char *skeletonJointNames[] =
{
	"RAVEN_LOWERBACK",  // 0
	"RAVEN_UPPERBACK",
	"RAVEN_NECK",
	"BOX_CENTER",       // 3
	"BEETLE_NECK",      // 4
	"BEETLE_HEAD",
	"PLAGUELF_BACKB",   // 6
	"PLAGUELF_BACKC",
	"PLAGUELF_NECK",
	"ELF_BACKB",        // 9
	"ELF_BACKC",
	"ELF_NECKB",
};

int skeletonNameOffsets[] =
{
	0,  // RAVEN
	3,  // BOX
	4,  // BEETLE
	-1, // ELFLORD
	6,  // PLAGUE ELF
	9,  // CORVUS
};

char *skeletonEffectorNames[] =
{
	"BEETLE_EYES",      // 0
	"CORVUS_EYES",      // 1
};

int skeletonENameOffsets[] =
{
	-1, // RAVEN
	-1, // BOX
	0,  // BEETLE
	-1, // ELFLORD
	1,  // PLAGUE ELF
};

int numJointsInSkeleton[] =
{
	NUM_JOINTS_RAVEN,
	NUM_JOINTS_BOX,
	NUM_JOINTS_BEETLE,
	NUM_JOINTS_ELFLORD,
	NUM_JOINTS_PLAGUE_ELF,
	NUM_JOINTS_CORVUS,
};

int numNodesInSkeleton[] =
{
	2,  // RAVEN
	0,  // BOX
	1,  // BEETLE
	-1, // ELFLORD
	2,  // PLAGUE ELF
	2,  // CORVUS
};

void CreateRavenSkel( void *g_skeletalJoints, size_t jointSize, struct ArrayedListNode_s *g_jointNodes, int root );
void CreateBoxSkel( void *g_skeletalJoints, size_t jointSize, struct ArrayedListNode_s *g_jointNodes, int root );
void CreateBeetleSkel( void *g_skeletalJoints, size_t jointSize, ArrayedListNode_t *g_jointNodes, int rootIndex );
void CreateElfLordSkel( void *g_skeletalJoints, size_t jointSize, ArrayedListNode_t *g_jointNodes, int rootIndex );
void CreatePlagueElfSkel( void *g_skeletalJoints, size_t jointSize, ArrayedListNode_t *g_jointNodes, int rootIndex );

CreateSkeleton_t SkeletonCreators[NUM_SKELETONS] =
{
	CreateRavenSkel,
	CreateBoxSkel,
	CreateBeetleSkel,
	CreateElfLordSkel,
	CreatePlagueElfSkel,
	CreatePlagueElfSkel,    // Corvus has the same structure as the Plague Elf
};

void CreateRavenSkel( void *g_skeletalJoints, size_t jointSize, ArrayedListNode_t *g_jointNodes, int rootIndex ){
	char *root;
	int *children;
	int nodeIndex;

	root = (char *)g_skeletalJoints + rootIndex * jointSize;

	children = (int *)( root + RAVEN_HEAD * jointSize );
	*children = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode( g_jointNodes, MAX_ARRAYED_JOINT_NODES );

	children = (int *)( root + RAVEN_UPPERBACK * jointSize );
	*children = nodeIndex;

	g_jointNodes[nodeIndex].data = rootIndex + RAVEN_HEAD;
	g_jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode( g_jointNodes, MAX_ARRAYED_JOINT_NODES );

	children = (int *)( root + RAVEN_LOWERBACK * jointSize );
	*children = nodeIndex;

	g_jointNodes[nodeIndex].data = rootIndex + RAVEN_UPPERBACK;
	g_jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;
}

void CreateBoxSkel( void *g_skeletalJoints, size_t jointSize, ArrayedListNode_t *g_jointNodes, int rootIndex ){
	char *root;
	int *children;

	root = (char *)g_skeletalJoints + rootIndex * jointSize;

	children = (int *)( root + RAVEN_HEAD * jointSize );
	*children = ARRAYEDLISTNODE_NULL;
}

void CreateBeetleSkel( void *g_skeletalJoints, size_t jointSize, ArrayedListNode_t *g_jointNodes, int rootIndex ){
	char *root;
	int *children;
	int nodeIndex;

	root = (char *)g_skeletalJoints + rootIndex * jointSize;

	children = (int *)( root + BEETLE_HEAD * jointSize );
	*children = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode( g_jointNodes, MAX_ARRAYED_JOINT_NODES );

	children = (int *)( root + BEETLE_NECK * jointSize );
	*children = nodeIndex;

	g_jointNodes[nodeIndex].data = rootIndex + BEETLE_HEAD;
	g_jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;
}

void CreateElfLordSkel( void *g_skeletalJoints, size_t jointSize, ArrayedListNode_t *g_jointNodes, int rootIndex ){
	char *root;
	int *children;
	int nodeIndex;

	root = (char *)g_skeletalJoints + rootIndex * jointSize;

	children = (int *)( root + BEETLE_HEAD * jointSize );
	*children = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode( g_jointNodes, MAX_ARRAYED_JOINT_NODES );

	children = (int *)( root + BEETLE_NECK * jointSize );
	*children = nodeIndex;

	g_jointNodes[nodeIndex].data = rootIndex + BEETLE_HEAD;
	g_jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;
}

void CreatePlagueElfSkel( void *g_skeletalJoints, size_t jointSize, ArrayedListNode_t *g_jointNodes, int rootIndex ){
	char *root;
	int *children;
	int nodeIndex;

	root = (char *)g_skeletalJoints + rootIndex * jointSize;

	children = (int *)( root + PLAGUE_ELF_HEAD * jointSize );
	*children = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode( g_jointNodes, MAX_ARRAYED_JOINT_NODES );

	children = (int *)( root + PLAGUE_ELF_UPPERBACK * jointSize );
	*children = nodeIndex;

	g_jointNodes[nodeIndex].data = rootIndex + PLAGUE_ELF_HEAD;
	g_jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;

	nodeIndex = GetFreeNode( g_jointNodes, MAX_ARRAYED_JOINT_NODES );

	children = (int *)( root + PLAGUE_ELF_LOWERBACK * jointSize );
	*children = nodeIndex;

	g_jointNodes[nodeIndex].data = rootIndex + PLAGUE_ELF_UPPERBACK;
	g_jointNodes[nodeIndex].next = ARRAYEDLISTNODE_NULL;
}
