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

#ifndef _M8_H
#define _M8_H

#include "imagem8.h"

#define M8_VERSION      2
#define PALETTE_SIZE    256
#define MIP_LEVELS  16

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} rgb_t;

typedef struct
{
	int version;
	char name[32];
	unsigned width[MIP_LEVELS], height[MIP_LEVELS];
	unsigned offsets[MIP_LEVELS];
	char animname[32];
	rgb_t palette[PALETTE_SIZE];
	int flags;
	int contents;
	int value;
} m8_header_t;
/*
   typedef struct {
    int            bpp;
    int            width;
    int            height;
    unsigned char *data;
    rgb_t         *palette;
   } m8_t;
 */
void LoadM8( const char *name, unsigned char **pic, int *width, int *height );


#endif
