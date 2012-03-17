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

// window system independent camera view code

typedef enum
{
	cd_wire,
	cd_solid,
	cd_texture,
	cd_light,
//  cd_blend
} camera_draw_mode;

#define DRAW_GL_FILL          0x0001
#define DRAW_GL_LIGHTING      0x0010
#define DRAW_GL_TEXTURE_2D    0x0100
#define DRAW_GL_BLEND         0x1000

#define DRAW_GL_WIRE                    0x0000
#define DRAW_GL_FLAT                    0x0001
#define DRAW_GL_SOLID                   0x0011
#define DRAW_GL_TEXTURED            0x0111

#define DRAW_WIRE                           0
#define DRAW_SOLID                      1
#define DRAW_TEXTURED                   2

// TTimo: camera code is a huge mess
//   someone courageous should clean it up
//   this will probably happen when we have new rendering code

#define MOVE_FORWARD      0x001
#define MOVE_BACK         0x002
#define MOVE_ROTRIGHT     0x004
#define MOVE_ROTLEFT      0x008
#define MOVE_STRAFERIGHT  0x010
#define MOVE_STRAFELEFT   0x020

typedef struct
{
	int width, height;

	qboolean timing;

	vec3_t origin;
	// TTimo
	// indexes: PITCH = 0 YAW = 1 ROLL = 3
	// AFAIK in Radiant we always have ROLL=0
	vec3_t angles;

	camera_draw_mode draw_mode;
	int draw_glstate;

	vec3_t color;           // background

	vec3_t forward, right; // move matrix (TTimo: used to have up but it was not updated)
	vec3_t vup, vpn, vright;    // view matrix (taken from the GL_PROJECTION matrix)

	float projection[4][4];
	float modelview[4][4];

	unsigned int movementflags; // movement flags

} camera_t;
