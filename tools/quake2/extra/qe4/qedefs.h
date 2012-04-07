/*
===========================================================================
Copyright (C) 1997-2006 Id Software, Inc.

This file is part of Quake 2 Tools source code.

Quake 2 Tools source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake 2 Tools source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake 2 Tools source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#ifndef __QEDEFS_H__
#define __QEDEFS_H__

#define QE_VERSION  0x0401

#define QE3_STYLE (WS_OVERLAPPED| WS_CAPTION | WS_THICKFRAME | \
		/* WS_MINIMIZEBOX | */ WS_MAXIMIZEBOX  | WS_CLIPSIBLINGS | \
		WS_CLIPCHILDREN | WS_CHILD)

#define QE_AUTOSAVE_INTERVAL  5       // number of minutes between autosaves

#define	CAMERA_WINDOW_CLASS	"QCamera"
#define	XY_WINDOW_CLASS	    "QXY"
#define	Z_WINDOW_CLASS   	"QZ"
#define	ENT_WINDOW_CLASS	"QENT"

#define	ZWIN_WIDTH	40
#define CWIN_SIZE	(0.4)

#define	MAX_EDGES	256
#define	MAX_POINTS	512

#define	CMD_TEXTUREWAD	60000
#define	CMD_BSPCOMMAND	61000

#define	PITCH	0
#define	YAW		1
#define	ROLL	2

#define QE_TIMER0   1

#define	PLANE_X		0
#define	PLANE_Y		1
#define	PLANE_Z		2
#define	PLANE_ANYX	3
#define	PLANE_ANYY	4
#define	PLANE_ANYZ	5

#define	ON_EPSILON	0.01

#define	KEY_FORWARD		1
#define	KEY_BACK		2
#define	KEY_TURNLEFT	4
#define	KEY_TURNRIGHT	8
#define	KEY_LEFT		16
#define	KEY_RIGHT		32
#define	KEY_LOOKUP		64
#define	KEY_LOOKDOWN	128
#define	KEY_UP			256
#define	KEY_DOWN		512

// xy.c
#define EXCLUDE_LIGHTS	1
#define EXCLUDE_ENT		2
#define EXCLUDE_PATHS	4
#define EXCLUDE_WATER	8
#define EXCLUDE_WORLD	16
#define EXCLUDE_CLIP	32
#define	EXCLUDE_DETAIL	64


//
// menu indexes for modifying menus
//
#define	MENU_VIEW		2
#define	MENU_BSP		4
#define	MENU_TEXTURE	6


// odd things not in windows header...
#define	VK_COMMA		188
#define	VK_PERIOD		190

/*
** window bits
*/
#define	W_CAMERA		0x0001
#define	W_XY			0x0002
#define	W_XY_OVERLAY	0x0004
#define	W_Z				0x0008
#define	W_TEXTURE		0x0010
#define	W_Z_OVERLAY		0x0020
#define W_CONSOLE		0x0040
#define W_ENTITY		0x0080
#define	W_ALL			0xFFFFFFFF

#define	COLOR_TEXTUREBACK	0
#define	COLOR_GRIDBACK		1
#define	COLOR_GRIDMINOR		2
#define	COLOR_GRIDMAJOR		3
#define	COLOR_CAMERABACK	4
#define COLOR_ENTITY        5
#define COLOR_LAST          6

#endif
