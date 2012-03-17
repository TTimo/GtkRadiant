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

#ifndef _WAL_H
#define _WAL_H

#include "imagewal.h"

#define PALETTE_SIZE    256
#define MIPLEVELS   4

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} rgb_t;


typedef struct miptex_s
{
	char name[32];
	unsigned width, height;
	unsigned offsets[MIPLEVELS];        // four mip maps stored
	char animname[32];                  // next frame in animation chain
	int flags;
	int contents;
	int value;
} miptex_t;

void LoadWAL( const char *name, unsigned char **pic, int *width, int *height );


#endif
