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

#ifndef FMODEL_H
#define FMODEL_H
#include "fmodel.h"
#endif
#include "qd_skeletons.h"

typedef struct
{
	int numnormals;
	vec3_t normalsum;
} fmvertexnormals_t;

typedef struct
{
	vec3_t v;
	int lightnormalindex;
	fmvertexnormals_t vnorm;
} fmtrivert_t;

#define FRAME_NAME_LEN ( 16 )

typedef struct
{
	vec3_t mins, maxs;
	char name[FRAME_NAME_LEN];
	fmtrivert_t v[MAX_FM_VERTS];
	struct QD_SkeletalJoint_s joints[NUM_CLUSTERS];
	struct QD_SkeletalJoint_s references[NUM_REFERENCES];
} fmframe_t;

extern fmframe_t g_frames[MAX_FM_FRAMES];

extern fmheader_t fmheader;
extern char cdarchive[1024];                        // set by $fmcd
extern char cdpartial[1024];                        // set by $fmcd
extern char cddir[1024];                            // set by $fmcd

void GrabFrame( char *frame );
void H_printf( char *fmt, ... );
char *FindFrameFile( char *frame );
