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

#ifndef Q_TYPEDEF_H
#define Q_TYPEDEF_H

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef double vec3d_t[3];
typedef vec_t vec5_t[5];

typedef float matrix3_t[3][3];
typedef float matrix3d_t[3][3];

typedef int fixed4_t;
typedef int fixed8_t;
typedef int fixed16_t;

typedef unsigned char byte;

#ifndef __cplusplus
typedef enum {false, true}  qboolean;
#else
typedef int qboolean;
#endif

typedef struct edict_s edict_t;

typedef struct paletteRGBA_s
{
	union
	{
		struct
		{
			byte r,g,b,a;
		};
		unsigned c;
		byte c_array[4];
	};
} paletteRGBA_t;

#endif
