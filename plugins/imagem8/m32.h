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

#ifndef _M32_H
#define _M32_H

#include "imagem8.h"

#define M32_VERSION     4
#define M32_MIP_LEVELS  16

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} rgb_t;

typedef struct
{
	int version;
	char name[128];
	char altname[128];                          // texture substitution
	char animname[128];                         // next frame in animation chain
	char damagename[128];                       // image that should be shown when damaged
	unsigned width[M32_MIP_LEVELS], height[M32_MIP_LEVELS];
	unsigned offsets[M32_MIP_LEVELS];
	int flags;
	int contents;
	int value;
	float scale_x, scale_y;
	int mip_scale;

	// detail texturing info
	char dt_name[128];                  // detailed texture name
	float dt_scale_x, dt_scale_y;
	float dt_u, dt_v;
	float dt_alpha;
	int dt_src_blend_mode, dt_dst_blend_mode;

	int unused[20];                                 // future expansion to maintain compatibility with h2
} m32_header_t;

void LoadM32( const char *name, unsigned char **pic, int *width, int *height );


#endif
