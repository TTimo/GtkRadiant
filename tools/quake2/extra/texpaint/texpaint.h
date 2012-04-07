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

#include "cmdlib.h"
#include "mathlib.h"
#include "lbmlib.h"
#include "trilib.h"
#include "l3dslib.h"

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "resource.h"
#include "afxres.h"

extern	HINSTANCE	main_instance;

extern	HGLRC	baseRC;

extern	HWND	mainwindow;
extern	HWND	camerawindow;
extern	HWND	palettewindow;
extern	HWND	skinwindow;

extern	int		screen_width, screen_height;

#define QE3_STYLE (WS_OVERLAPPED| WS_CAPTION | WS_THICKFRAME | \
		/* WS_MINIMIZEBOX | */ WS_MAXIMIZEBOX  | WS_CLIPSIBLINGS | \
		WS_CLIPCHILDREN | WS_CHILD)

extern	byte		pic[1024*512];
extern	unsigned	rgb[1024*512];
extern	unsigned	index_texture[1024*512];
extern	byte		palette[768];

extern	triangle_t	*faces;
extern	int		numfaces;
extern	float	tmcoords[10000][3][2];

extern	int		skinwidth, skinheight;
extern	int		picwidth, picheight;
extern	int		width, height;
extern	int		iwidth, iheight;
extern	int		width2, height2;		// padded to ^2

extern	char	tri_filename[1024];
extern	char	skin_filename[1024];

extern	int			selected_index;
extern	unsigned	selected_rgb;

extern	qboolean	model_lines;
extern	qboolean	skin_lines;

extern	qboolean	modified;
extern	qboolean	modified_past_autosave;

#define	TEXTURE_SKIN	1
#define	TEXTURE_INDEX	2

#define	MENU_VIEW		2

#define	MODEL_DISPLAYLIST	1

typedef void (APIENTRY *BINDTEXFUNCPTR)(GLenum, GLuint);

extern	BINDTEXFUNCPTR BindTextureEXT;

