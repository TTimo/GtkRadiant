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

#ifndef QD_SKELETONS_H
#define QD_SKELETONS_H

#include "placement.h"


typedef struct Placement_d_s
{
	vec3d_t origin;
	vec3d_t direction;
	vec3d_t up;
} Placement_d_t;

typedef struct QD_SkeletalJoint_s
{
	Placement_d_t placement;
	vec3d_t rotation;
} QD_SkeletalJoint_t;

#define NUM_CLUSTERS 8

typedef struct IntListNode_s
{
	int data;
	struct IntListNode_s *next;
} IntListNode_t;  // gaak

typedef struct Skeletalfmheader_s
{
	int type;
	int clustered;
	int references;

	int *clusters[NUM_CLUSTERS];
	IntListNode_t *vertLists[NUM_CLUSTERS];
	int num_verts[NUM_CLUSTERS + 1];
	int new_num_verts[NUM_CLUSTERS + 1];

	float scaling[3];
	float rotation[3];
	float translation[3];
} Skeletalfmheader_t;

#define SKELETAL_NAME_MAX 32

extern Skeletalfmheader_t g_skelModel;

void ClearSkeletalModel();
void GrabModelTransform( char *frame );
void GrabSkeletalFrame( char *frame );
void GrabReferencedFrame( char *frame );

// Reference Stuff
#define NUM_REFERENCES 8

#define REF_MAX_POINTS  16
#define REF_MAX_STRLEN  32

// We're assuming no more than 16 reference points, with no more than 32 characters in the name
extern char RefPointNameList[REF_MAX_POINTS][REF_MAX_STRLEN];
extern int RefPointNum;

#endif
