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

#ifndef _JOINTED_H
#define _JOINTED_H

#include "joints.h"

void LoadGlobals( char *fileName );
void LoadClusters( char *fileName, int **clusterList, int *num_verts, int skel_type );
void LoadJointList( char *fileName, struct QDataJoint_s *jointList, int num_verts );

#define NUM_CLUSTERS 8

#define NOT_JOINTED -1

#endif //_JOINTED_H
