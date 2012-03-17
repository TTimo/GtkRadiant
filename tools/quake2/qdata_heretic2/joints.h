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

#ifndef JOINTS_H
#define JOINTS_H

#ifdef _HERETIC2_
#include "angles.h"
#endif

//typedef float vec3_t[3];
//typedef unsigned char byte;

#ifndef _WIN32
#define stricmp strcasecmp
#define strcmpi strcasecmp
#endif

typedef struct Placement_s
{
	vec3_t origin;
	vec3_t direction;
	vec3_t up;
} Placement_t;

#if 1
typedef struct QDataJoint_s
{
	Placement_t placement;
	vec3_t rotation;
} QDataJoint_t;
#endif

typedef struct ArrayedListNode_s
{
	int data;
	int next;
	int inUse;
} ArrayedListNode_t;

#define ARRAYEDLISTNODE_NULL -1

typedef struct JointAngles_s
{
	float angles[3];
	int children;
	int created;
} JointAngles_t;

typedef struct JointAngles2_s
{
	float angles[3];
	int children;
	int changed[3];
	int inUse;
} JointAngles2_t;

#define MAX_MODELJOINTS         256
#define MAX_MODELJOINTNODES     255

extern JointAngles_t jointAngles[MAX_MODELJOINTS];
extern JointAngles2_t jointAngles2[MAX_MODELJOINTS];

extern ArrayedListNode_t jointAngleNodes[MAX_MODELJOINTNODES];

// Skeletal structures enums
enum {
	SKEL_RAVEN = 0,
	SKEL_BOX,
	NUM_SKELETONS
};

// Raven Skeletal structures enums
enum {
	RAVEN_WAIST1 = 0,
	RAVEN_WAIST2 = 1,
	RAVEN_HEAD = 2,
	NUM_JOINTS_RAVEN
};

// Box Skeletal structures enums
enum {
	BOX_CENTER = 0,
	NUM_JOINTS_BOX
};

extern int numJointsForSkeleton[];
extern char *RAVEN_SKEL_NAMES[];

#define J_NEW_SKELETON      0x00001000
#define J_YAW_CHANGED       0x00002000
#define J_PITCH_CHANGED     0x00004000
#define J_ROLL_CHANGED      0x00008000
#define MAX_JOINTS          0x00000fff
/*
   inline int GetFreeNode(ArrayedListNode_t *nodeArray, int max)
   {	// yeah, I know this is a sucky, inefficient way to do this, but I didn't feel like taking the time to write a real resource manager in C
    int i;

    for(i = 0; i < max; ++i)
    {
        if(!nodeArray[i].inUse)
        {
            nodeArray[i].inUse = 1;
            return i;
        }
    }

    assert(0);
    return -1;
   }

   inline void FreeNode(ArrayedListNode_t *nodeArray, int index)
   {
    nodeArray[index].inUse = 0;
   }
 */
int CreateSkeleton( int structure );
void CreateSkeletonAtIndex( int structure, int index );
void FreeSkeleton( int structure, int index );
void SetJointAngle( int jointIndex, int angleIndex, float angle );
float ModifyJointAngle( int jointIndex, int angleIndex, float deltaAngle );
int ZeroJointAngle( int jointIndex, int angleIndex, float angVel );
int ApplyAngVelToJoint( int jointIndex, int angleIndex, float angVel, float destAng );

#endif
