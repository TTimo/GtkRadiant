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
// trilib.h: header file for loading triangles from an Alias triangle file
//

#include "fmodel.h"

#define MAXTRIANGLES    MAX_FM_TRIANGLES

typedef struct
{
	vec3_t verts[3];
#if 1
	int indicies[3];
	float uv[3][2];
	qboolean HasUV;
#endif
} triangle_t;

#define NUM_CLUSTERS 8

typedef struct
{
	char name[64];
	byte tris[MAXTRIANGLES >> 3];
	byte verts[MAX_FM_VERTS >> 3];
	int start_glcmds, num_glcmds;

	int *clusters[NUM_CLUSTERS];
	struct IntListNode_s *vertLists[NUM_CLUSTERS];
	int num_verts[NUM_CLUSTERS + 1];
	int new_num_verts[NUM_CLUSTERS + 1];
	qboolean clustered;
} mesh_node_t;

void LoadTriangleList( char *filename, triangle_t **pptri, int *numtriangles, mesh_node_t **ppmnodes, int *num_mesh_nodes );
