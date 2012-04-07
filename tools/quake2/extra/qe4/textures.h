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

typedef struct
{
	char	name[32];
	float	shift[2];
	float	rotate;
	float	scale[2];
	int		contents;
	int		flags;
	int		value;
} texdef_t;


typedef struct
{
	int			width, height;
	int			originy;
	texdef_t	texdef;
} texturewin_t;

typedef struct qtexture_s
{
	struct	qtexture_s *next;
	char	name[64];		// includes partial directory and extension
    int		width,  height;
	int		contents;
	int		flags;
	int		value;
	int		texture_number;	// gl bind number
	vec3_t	color;			// for flat shade mode
	qboolean	inuse;		// true = is present on the level
} qtexture_t;


// a texturename of the form (0 0 0) will
// create a solid color texture

void	Texture_Init (void);
void	Texture_Flush (void);
void	Texture_ClearInuse (void);
void	Texture_ShowInuse (void);
void	Texture_ShowDirectory (int menunum);

qtexture_t *Texture_ForName (char *name);

void	Texture_Init (void);
void	Texture_SetTexture (texdef_t *texdef);

void	Texture_SetMode(int iMenu);	// GL_TEXTURE_NEAREST, etc..
