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

#include "qe3.h"
#include "io.h"

#define	TYP_MIPTEX	68
static unsigned	tex_palette[256];

static qtexture_t	*notexture;

static qboolean	nomips;

#define	FONT_HEIGHT	10

static HGLRC s_hglrcTexture;
static HDC	 s_hdcTexture;

//int		texture_mode = GL_NEAREST;
//int		texture_mode = GL_NEAREST_MIPMAP_NEAREST;
//int		texture_mode = GL_NEAREST_MIPMAP_LINEAR;
//int		texture_mode = GL_LINEAR;
//int		texture_mode = GL_LINEAR_MIPMAP_NEAREST;
int		texture_mode = GL_LINEAR_MIPMAP_LINEAR;

int		texture_extension_number = 1;

// current active texture directory.  if empty, show textures in use
char		texture_directory[32];	// use if texture_showinuse is false
qboolean	texture_showinuse;

// texture layout functions
qtexture_t	*current_texture;
int			current_x, current_y, current_row;

int			texture_nummenus;
#define		MAX_TEXTUREDIRS	100
char		texture_menunames[MAX_TEXTUREDIRS][64];

qboolean	g_dontuse;		// set to true to load the texture but not flag as used

void SelectTexture (int mx, int my);

void	Texture_MouseDown (int x, int y, int buttons);
void	Texture_MouseUp (int x, int y, int buttons);
void	Texture_MouseMoved (int x, int y, int buttons);

//=====================================================

void SortTextures(void)
{
	qtexture_t	*q, *qtemp, *qhead, *qcur, *qprev;

	// standard insertion sort
	// Take the first texture from the list and
	// add it to our new list
	if ( g_qeglobals.d_qtextures == NULL)
		return;

	qhead = g_qeglobals.d_qtextures;
	q = g_qeglobals.d_qtextures->next;
	qhead->next = NULL;

	// while there are still things on the old
	// list, keep adding them to the new list
	while (q)
	{
		qtemp = q;
		q = q->next;

		qprev = NULL;
		qcur = qhead;

		while (qcur)
		{
			// Insert it here?
			if (strcmp(qtemp->name, qcur->name) < 0)
			{
				qtemp->next = qcur;
				if (qprev)
					qprev->next = qtemp;
				else
					qhead = qtemp;
				break;
			}

			// Move on

			qprev = qcur;
			qcur = qcur->next;


			// is this one at the end?

			if (qcur == NULL)
			{
				qprev->next = qtemp;
				qtemp->next = NULL;
			}
		}


	}

	g_qeglobals.d_qtextures = qhead;
}

//=====================================================


/*
==============
Texture_InitPalette
==============
*/
void Texture_InitPalette (byte *pal)
{
    int		r,g,b,v;
    int		i;
	int		inf;
	byte	gammatable[256];
	float	gamma;

	gamma = g_qeglobals.d_savedinfo.fGamma;

	if (gamma == 1.0)
	{
		for (i=0 ; i<256 ; i++)
			gammatable[i] = i;
	}
	else
	{
		for (i=0 ; i<256 ; i++)
		{
			inf = 255 * pow ( (i+0.5)/255.5 , gamma ) + 0.5;
			if (inf < 0)
				inf = 0;
			if (inf > 255)
				inf = 255;
			gammatable[i] = inf;
		}
	}

    for (i=0 ; i<256 ; i++)
    {
		r = gammatable[pal[0]];
		g = gammatable[pal[1]];
		b = gammatable[pal[2]];
		pal += 3;

		v = (r<<24) + (g<<16) + (b<<8) + 255;
		v = BigLong (v);

		tex_palette[i] = v;
    }
}

void SetTexParameters (void)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_mode );

	switch ( texture_mode )
	{
	case GL_NEAREST:
	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_NEAREST_MIPMAP_LINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		break;
	case GL_LINEAR:
	case GL_LINEAR_MIPMAP_NEAREST:
	case GL_LINEAR_MIPMAP_LINEAR:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		break;
	}
}

/*
============
Texture_SetMode
============
*/
void Texture_SetMode(int iMenu)
{
	int	i, iMode;
	HMENU hMenu;
	qboolean texturing = true;

	hMenu = GetMenu(g_qeglobals.d_hwndMain);

	switch(iMenu) {
	case ID_VIEW_NEAREST:
		iMode = GL_NEAREST;
		break;
	case ID_VIEW_NEARESTMIPMAP:
		iMode = GL_NEAREST_MIPMAP_NEAREST;
		break;
	case ID_VIEW_LINEAR:
		iMode = GL_NEAREST_MIPMAP_LINEAR;
		break;
	case ID_VIEW_BILINEAR:
		iMode = GL_LINEAR;
		break;
	case ID_VIEW_BILINEARMIPMAP:
		iMode = GL_LINEAR_MIPMAP_NEAREST;
		break;
	case ID_VIEW_TRILINEAR:
		iMode = GL_LINEAR_MIPMAP_LINEAR;
		break;

	case ID_TEXTURES_WIREFRAME:
		iMode = 0;
		texturing = false;
		break;

	case ID_TEXTURES_FLATSHADE:
		iMode = 0;
		texturing = false;
		break;

	}

	CheckMenuItem(hMenu, ID_VIEW_NEAREST, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_NEARESTMIPMAP, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_LINEAR, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_BILINEARMIPMAP, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_BILINEAR, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_TRILINEAR, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_TEXTURES_WIREFRAME, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_TEXTURES_FLATSHADE, MF_BYCOMMAND | MF_UNCHECKED);

	CheckMenuItem(hMenu, iMenu, MF_BYCOMMAND | MF_CHECKED);

	g_qeglobals.d_savedinfo.iTexMenu = iMenu;
	texture_mode = iMode;
	if ( texturing )
		SetTexParameters ();

	if ( !texturing && iMenu == ID_TEXTURES_WIREFRAME)
	{
		camera.draw_mode = cd_wire;
		Map_BuildBrushData();
		Sys_UpdateWindows (W_ALL);
		return;

	} else if ( !texturing && iMenu == ID_TEXTURES_FLATSHADE) {

		camera.draw_mode = cd_solid;
		Map_BuildBrushData();
		Sys_UpdateWindows (W_ALL);
		return;
	}

	for (i=1 ; i<texture_extension_number ; i++)
	{
		glBindTexture( GL_TEXTURE_2D, i );
		SetTexParameters ();
	}

	// select the default texture
	glBindTexture( GL_TEXTURE_2D, 0 );

	glFinish();

	if (camera.draw_mode != cd_texture)
	{
		camera.draw_mode = cd_texture;
		Map_BuildBrushData();
	}

	Sys_UpdateWindows (W_ALL);
}


/*
=================
Texture_LoadTexture
=================
*/
qtexture_t *Texture_LoadTexture (miptex_t *qtex)
{
    byte		*source;
    unsigned	*dest;
    int			width, height, i, count;
	int			total[3];
    qtexture_t	*q;

    q = qmalloc(sizeof(*q));
    width = LittleLong(qtex->width);
    height = LittleLong(qtex->height);

    q->width = width;
    q->height = height;

	q->flags = qtex->flags;
	q->value = qtex->value;
	q->contents = qtex->contents;

	dest = qmalloc (width*height*4);

    count = width*height;
    source = (byte *)qtex + LittleLong(qtex->offsets[0]);

	// The dib is upside down so we want to copy it into
	// the buffer bottom up.

	total[0] = total[1] = total[2] = 0;
    for (i=0 ; i<count ; i++)
	{
		dest[i] = tex_palette[source[i]];

		total[0] += ((byte *)(dest+i))[0];
		total[1] += ((byte *)(dest+i))[1];
		total[2] += ((byte *)(dest+i))[2];
	}

	q->color[0] = (float)total[0]/(count*255);
	q->color[1] = (float)total[1]/(count*255);
	q->color[2] = (float)total[2]/(count*255);

    q->texture_number = texture_extension_number++;

	glBindTexture( GL_TEXTURE_2D, q->texture_number );
	SetTexParameters ();

	if (nomips)
		glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dest);
	else
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height,GL_RGBA, GL_UNSIGNED_BYTE, dest);

	free (dest);

	glBindTexture( GL_TEXTURE_2D, 0 );

    return q;
}

/*
===============
Texture_CreateSolid

Create a single pixel texture of the apropriate color
===============
*/
qtexture_t *Texture_CreateSolid (char *name)
{
	byte	data[4];
	qtexture_t	*q;

    q = qmalloc(sizeof(*q));

	sscanf (name, "(%f %f %f)", &q->color[0], &q->color[1], &q->color[2]);

	data[0] = q->color[0]*255;
	data[1] = q->color[1]*255;
	data[2] = q->color[2]*255;
	data[3] = 255;

	q->width = q->height = 1;
    q->texture_number = texture_extension_number++;
	glBindTexture( GL_TEXTURE_2D, q->texture_number );
	SetTexParameters ();

	if (nomips)
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	else
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 1, 1,GL_RGBA, GL_UNSIGNED_BYTE, data);

	glBindTexture( GL_TEXTURE_2D, 0 );

	return q;
}


/*
=================
Texture_MakeNotexture
=================
*/
void Texture_MakeNotexture (void)
{
    qtexture_t	*q;
    byte		data[4][4];

	notexture = q = qmalloc(sizeof(*q));
	strcpy (q->name, "notexture");
    q->width = q->height = 64;

	memset (data, 0, sizeof(data));
	data[0][2] = data[3][2] = 255;

	q->color[0] = 0;
	q->color[1] = 0;
	q->color[2] = 0.5;

    q->texture_number = texture_extension_number++;
	glBindTexture( GL_TEXTURE_2D, q->texture_number );
	SetTexParameters ();

	if (nomips)
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	else
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 2, 2,GL_RGBA, GL_UNSIGNED_BYTE, data);

	glBindTexture( GL_TEXTURE_2D, 0 );
}



/*
===============
Texture_ForName
===============
*/
qtexture_t *Texture_ForName (char *name)
{
    byte    *lump;
	qtexture_t	*q;
	char	filename[1024];

//return notexture;
	for (q=g_qeglobals.d_qtextures ; q ; q=q->next)
    {
		if (!strcmp(name,  q->name))
		{
			if (!g_dontuse)
				q->inuse = true;
		    return q;
		}
    }

	if (name[0] == '(')
	{
		q = Texture_CreateSolid (name);
		strncpy (q->name, name, sizeof(q->name)-1);
	}
	else
	{
		// load the file
		sprintf (filename, "%s/%s.wal",
			ValueForKey (g_qeglobals.d_project_entity, "texturepath"),
			name);
		Sys_Printf ("Loading %s\n", name);
		if (LoadFile (filename, &lump) == -1)
		{
			Sys_Printf ("     load failed!\n");
			return notexture;
		}
		q = Texture_LoadTexture ((miptex_t *)lump);
		free (lump);
		strncpy (q->name, name, sizeof(q->name)-1);
		StripExtension (q->name);
	}

	if (!g_dontuse)
		q->inuse = true;
	q->next = g_qeglobals.d_qtextures;
	g_qeglobals.d_qtextures = q;

    return q;
}

/*
==================
FillTextureMenu

==================
*/
void FillTextureMenu (void)
{
	HMENU	hmenu;
	int		i;
	struct _finddata_t fileinfo;
	int		handle;
	char	dirstring[1024];
	char	*path;

	hmenu = GetSubMenu (GetMenu(g_qeglobals.d_hwndMain), MENU_TEXTURE);

	// delete everything
	for (i=0 ; i<texture_nummenus ; i++)
		DeleteMenu (hmenu, CMD_TEXTUREWAD+i, MF_BYCOMMAND);

	// add everything
	path = ValueForKey (g_qeglobals.d_project_entity, "texturepath");

	sprintf (dirstring, "%s/*.*", path);

	handle = _findfirst (dirstring, &fileinfo);
	if (handle == -1)
		return;

	do
	{
		if (!(fileinfo.attrib & _A_SUBDIR))
			continue;
		if (fileinfo.name[0] == '.')
			continue;
		// add this directory to the menu
		AppendMenu (hmenu, MF_ENABLED|MF_STRING,
			CMD_TEXTUREWAD+texture_nummenus, (LPCTSTR)fileinfo.name);
		strcpy (texture_menunames[texture_nummenus], fileinfo.name);
		strcat (texture_menunames[texture_nummenus], "/");
		if (++texture_nummenus == MAX_TEXTUREDIRS)
			break;
	} while (_findnext( handle, &fileinfo ) != -1);

	_findclose (handle);
}


/*
==================
Texture_ClearInuse

A new map is being loaded, so clear inuse markers
==================
*/
void Texture_ClearInuse (void)
{
	qtexture_t	*q;

	for (q=g_qeglobals.d_qtextures ; q ; q=q->next)
    {
		q->inuse = false;
	}
}



/*
==============
Texture_ShowDirectory
==============
*/
void	Texture_ShowDirectory (int menunum)
{
	struct _finddata_t fileinfo;
	int		handle;
	char	name[1024];
	char	dirstring[1024];

	texture_showinuse = false;
	strcpy (texture_directory, texture_menunames[menunum-CMD_TEXTUREWAD]);

	g_qeglobals.d_texturewin.originy = 0;
	Sys_Status("loading all textures\n", 0);

	// load all .wal files
	sprintf (dirstring, "%s/textures/%s*.wal",
		ValueForKey (g_qeglobals.d_project_entity, "basepath"),
		texture_menunames[menunum-CMD_TEXTUREWAD]);

	Sys_Printf ("Scanning %s\n", dirstring);

	handle = _findfirst (dirstring, &fileinfo);
	if (handle == -1)
		return;

	g_dontuse = true;
	do
	{
		sprintf (name, "%s%s", texture_directory, fileinfo.name);
		StripExtension (name);
		Texture_ForName (name);
	} while (_findnext( handle, &fileinfo ) != -1);
	g_dontuse = false;

	_findclose (handle);

	SortTextures();
	SetInspectorMode(W_TEXTURE);
	Sys_UpdateWindows(W_TEXTURE);

	sprintf (name, "Textures: %s", texture_directory);
	SetWindowText(g_qeglobals.d_hwndEntity, name);

	// select the first texture in the list
	if (!g_qeglobals.d_texturewin.texdef.name[0])
		SelectTexture (16, g_qeglobals.d_texturewin.height -16);
}

/*
==============
Texture_ShowInuse
==============
*/
void	Texture_ShowInuse (void)
{
	char	name[1024];
	face_t	*f;
	brush_t	*b;

	texture_showinuse = true;

	g_qeglobals.d_texturewin.originy = 0;
	Sys_Status("Selecting active textures\n", 0);
	Texture_ClearInuse ();

	for (b=active_brushes.next ; b != NULL && b != &active_brushes ; b=b->next)
		for (f=b->brush_faces ; f ; f=f->next)
			Texture_ForName (f->texdef.name);

	for (b=selected_brushes.next ; b != NULL && b != &selected_brushes ; b=b->next)
		for (f=b->brush_faces ; f ; f=f->next)
			Texture_ForName (f->texdef.name);

	SortTextures();
	SetInspectorMode(W_TEXTURE);
	Sys_UpdateWindows (W_TEXTURE);

	sprintf (name, "Textures: in use");
	SetWindowText(g_qeglobals.d_hwndEntity, name);

	// select the first texture in the list
	if (!g_qeglobals.d_texturewin.texdef.name[0])
		SelectTexture (16, g_qeglobals.d_texturewin.height -16);
}

/*
============================================================================

TEXTURE LAYOUT

============================================================================
*/

void Texture_StartPos (void)
{
	current_texture = g_qeglobals.d_qtextures;
	current_x = 8;
	current_y = -8;
	current_row = 0;
}

qtexture_t *Texture_NextPos (int *x, int *y)
{
	qtexture_t	*q;

	while (1)
	{
		q = current_texture;
		if (!q)
			return q;
		current_texture = current_texture->next;
		if (q->name[0] == '(')	// fake color texture
			continue;
		if (q->inuse)
			break;			// allways show in use
		if (!texture_showinuse && strncmp (q->name, texture_directory, strlen(texture_directory)))
			continue;
		break;
	}

	if (current_x + q->width > g_qeglobals.d_texturewin.width-8 && current_row)
	{	// go to the next row unless the texture is the first on the row
		current_x = 8;
		current_y -= current_row + FONT_HEIGHT + 4;
		current_row = 0;
	}

	*x = current_x;
	*y = current_y;

	// Is our texture larger than the row? If so, grow the
	// row height to match it

    if (current_row < q->height)
		current_row = q->height;

	// never go less than 64, or the names get all crunched up
	current_x += q->width < 64 ? 64 : q->width;
	current_x += 8;

	return q;
}

/*
============================================================================

  MOUSE ACTIONS

============================================================================
*/

static	int	textures_cursorx, textures_cursory;


/*
============
Texture_SetTexture

============
*/
void Texture_SetTexture (texdef_t *texdef)
{
	qtexture_t	*q;
	int			x,y;
	char		sz[256];

	if (texdef->name[0] == '(')
	{
		Sys_Status("Can't select an entity texture\n", 0);
		return;
	}
	g_qeglobals.d_texturewin.texdef = *texdef;

	Sys_UpdateWindows (W_TEXTURE);
	sprintf(sz, "Selected texture: %s\n", texdef->name);
	Sys_Status(sz, 0);
	Select_SetTexture(texdef);

// scroll origin so the texture is completely on screen
	Texture_StartPos ();
	while (1)
	{
		q = Texture_NextPos (&x, &y);
		if (!q)
			break;
		if (!strcmpi(texdef->name, q->name))
		{
			if (y > g_qeglobals.d_texturewin.originy)
			{
				g_qeglobals.d_texturewin.originy = y;
				Sys_UpdateWindows (W_TEXTURE);
				return;
			}

			if (y-q->height-2*FONT_HEIGHT < g_qeglobals.d_texturewin.originy-g_qeglobals.d_texturewin.height)
			{
				g_qeglobals.d_texturewin.originy = y-q->height-2*FONT_HEIGHT+g_qeglobals.d_texturewin.height;
				Sys_UpdateWindows (W_TEXTURE);
				return;
			}

			return;
		}
	}
}


/*
==============
SelectTexture

  By mouse click
==============
*/
void SelectTexture (int mx, int my)
{
	int		x, y;
	qtexture_t	*q;
	texdef_t	tex;

	my += g_qeglobals.d_texturewin.originy-g_qeglobals.d_texturewin.height;

	Texture_StartPos ();
	while (1)
	{
		q = Texture_NextPos (&x, &y);
		if (!q)
			break;
		if (mx > x && mx - x < q->width
			&& my < y && y - my < q->height + FONT_HEIGHT)
		{
			memset (&tex, 0, sizeof(tex));
			tex.scale[0] = 1;
			tex.scale[1] = 1;
			tex.flags = q->flags;
			tex.value = q->value;
			tex.contents = q->contents;
			strcpy (tex.name, q->name);
			Texture_SetTexture (&tex);
			return;
		}
	}

	Sys_Status("Did not select a texture\n", 0);
}

/*
==============
Texture_MouseDown
==============
*/
void Texture_MouseDown (int x, int y, int buttons)
{
	Sys_GetCursorPos (&textures_cursorx, &textures_cursory);

	// lbutton = select texture
	if (buttons == MK_LBUTTON )
	{
		SelectTexture (x, g_qeglobals.d_texturewin.height - 1 - y);
		return;
	}

}

/*
==============
Texture_MouseUp
==============
*/
void Texture_MouseUp (int x, int y, int buttons)
{
}

/*
==============
Texture_MouseMoved
==============
*/
void Texture_MouseMoved (int x, int y, int buttons)
{
	int scale = 1;

	if ( buttons & MK_SHIFT )
		scale = 4;

	// rbutton = drag texture origin
	if (buttons & MK_RBUTTON)
	{
		Sys_GetCursorPos (&x, &y);
		if ( y != textures_cursory)
		{
			g_qeglobals.d_texturewin.originy += ( y-textures_cursory) * scale;
			if (g_qeglobals.d_texturewin.originy > 0)
				g_qeglobals.d_texturewin.originy = 0;
			Sys_SetCursorPos (textures_cursorx, textures_cursory);
			Sys_UpdateWindows (W_TEXTURE);
		}
		return;
	}
}


/*
============================================================================

DRAWING

============================================================================
*/

int imax(int iFloor, int i) { if (i>iFloor) return iFloor; return i; }
HFONT ghFont = NULL;

/*
============
Texture_Draw2
============
*/
void Texture_Draw2 (int width, int height)
{
	qtexture_t	*q;
	int			x, y;
	char		*name;

	glClearColor (
		g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][0],
		g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][1],
		g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][2],
		0);
	glViewport (0,0,width,height);
	glClear (GL_COLOR_BUFFER_BIT);
	glDisable (GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, width, g_qeglobals.d_texturewin.originy-height, g_qeglobals.d_texturewin.originy, -100, 100);
	glEnable (GL_TEXTURE_2D);

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	g_qeglobals.d_texturewin.width = width;
	g_qeglobals.d_texturewin.height = height;
	Texture_StartPos ();

	while (1)
	{
		q = Texture_NextPos (&x, &y);
		if (!q)
			break;

		// Is this texture visible?
		if ( (y-q->height-FONT_HEIGHT < g_qeglobals.d_texturewin.originy)
			&& (y > g_qeglobals.d_texturewin.originy - height) )
		{

			// if in use, draw a background
			if (q->inuse && !texture_showinuse)
			{
				glLineWidth (1);
				glColor3f (0.5,1,0.5);
				glDisable (GL_TEXTURE_2D);

				glBegin (GL_LINE_LOOP);
				glVertex2f (x-1,y+1-FONT_HEIGHT);
				glVertex2f (x-1,y-q->height-1-FONT_HEIGHT);
				glVertex2f (x+1+q->width,y-q->height-1-FONT_HEIGHT);
				glVertex2f (x+1+q->width,y+1-FONT_HEIGHT);
				glEnd ();

				glEnable (GL_TEXTURE_2D);
			}

			// Draw the texture
			glColor3f (1,1,1);
			glBindTexture( GL_TEXTURE_2D, q->texture_number );
			glBegin (GL_QUADS);
			glTexCoord2f (0,0);
			glVertex2f (x,y-FONT_HEIGHT);
			glTexCoord2f (1,0);
			glVertex2f (x+q->width,y-FONT_HEIGHT);
			glTexCoord2f (1,1);
			glVertex2f (x+q->width,y-FONT_HEIGHT-q->height);
			glTexCoord2f (0,1);
			glVertex2f (x,y-FONT_HEIGHT-q->height);
			glEnd ();

			// draw the selection border
			if (!strcmpi(g_qeglobals.d_texturewin.texdef.name, q->name))
			{
				glLineWidth (3);
				glColor3f (1,0,0);
				glDisable (GL_TEXTURE_2D);

				glBegin (GL_LINE_LOOP);
				glVertex2f (x-4,y-FONT_HEIGHT+4);
				glVertex2f (x-4,y-FONT_HEIGHT-q->height-4);
				glVertex2f (x+4+q->width,y-FONT_HEIGHT-q->height-4);
				glVertex2f (x+4+q->width,y-FONT_HEIGHT+4);
				glEnd ();

				glEnable (GL_TEXTURE_2D);
				glLineWidth (1);
			}

			// draw the texture name
			glColor3f (0,0,0);
			glRasterPos2f (x, y-FONT_HEIGHT+2);

			// don't draw the directory name
			for (name = q->name ; *name && *name != '/' && *name != '\\' ; name++)
				;
			if (!*name)
				name = q->name;
			else
				name++;
			glCallLists (strlen(name), GL_UNSIGNED_BYTE, name);
		}
	}

	// reset the current texture
	glBindTexture( GL_TEXTURE_2D, 0 );
	glFinish();
}

/*
============
WTexWndProc
============
*/
LONG WINAPI WTex_WndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
	int		xPos, yPos;
    RECT	rect;

    GetClientRect(hWnd, &rect);

    switch (uMsg)
    {
	case WM_CREATE:
        s_hdcTexture = GetDC(hWnd);
		QEW_SetupPixelFormat(s_hdcTexture, false);

		if ( ( s_hglrcTexture = wglCreateContext( s_hdcTexture ) ) == 0 )
			Error( "wglCreateContext in WTex_WndProc failed" );

        if (!wglMakeCurrent( s_hdcTexture, s_hglrcTexture ))
			Error ("wglMakeCurrent in WTex_WndProc failed");

		if (!wglShareLists( g_qeglobals.d_hglrcBase, s_hglrcTexture ) )
			Error( "wglShareLists in WTex_WndProc failed" );

		return 0;

	case WM_DESTROY:
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( s_hglrcTexture );
		ReleaseDC( hWnd, s_hdcTexture );
		return 0;

	case WM_PAINT:
        {
		    PAINTSTRUCT	ps;

		    BeginPaint(hWnd, &ps);

            if ( !wglMakeCurrent( s_hdcTexture, s_hglrcTexture ) )
				Error ("wglMakeCurrent failed");
			Texture_Draw2 (rect.right-rect.left, rect.bottom-rect.top);
			SwapBuffers(s_hdcTexture);

		    EndPaint(hWnd, &ps);
        }
		return 0;

	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
		SetCapture( g_qeglobals.d_hwndTexture );
		xPos = (short)LOWORD(lParam);  // horizontal position of cursor
		yPos = (short)HIWORD(lParam);  // vertical position of cursor

		Texture_MouseDown (xPos, yPos, wParam);
		return 0;

	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	case WM_LBUTTONUP:
		xPos = (short)LOWORD(lParam);  // horizontal position of cursor
		yPos = (short)HIWORD(lParam);  // vertical position of cursor

		Texture_MouseUp (xPos, yPos, wParam);
		if (! (wParam & (MK_LBUTTON|MK_RBUTTON|MK_MBUTTON)))
			ReleaseCapture ();
		return 0;

	case WM_MOUSEMOVE:
		xPos = (short)LOWORD(lParam);  // horizontal position of cursor
		yPos = (short)HIWORD(lParam);  // vertical position of cursor

		Texture_MouseMoved (xPos, yPos, wParam);
		return 0;
    }

    return DefWindowProc (hWnd, uMsg, wParam, lParam);
}



/*
==================
CreateTextureWindow

We need to create a seperate window for the textures
in the inspector window, because we can't share
gl and gdi drawing in a single window
==================
*/
#define	TEXTURE_WINDOW_CLASS	"QTEX"
HWND CreateTextureWindow (void)
{
    WNDCLASS   wc;
	HWND		hwnd;

    /* Register the camera class */
	memset (&wc, 0, sizeof(wc));

    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)WTex_WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_qeglobals.d_hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = TEXTURE_WINDOW_CLASS;

    if (!RegisterClass (&wc) )
        Error ("WCam_Register: failed");

	hwnd = CreateWindow (TEXTURE_WINDOW_CLASS ,
		"Texture View",
		WS_BORDER|WS_CHILD|WS_VISIBLE,
		20,
		20,
		64,
		64,	// size

		g_qeglobals.d_hwndEntity,	// parent window
		0,		// no menu
		g_qeglobals.d_hInstance,
		0);
	if (!hwnd)
		Error ("Couldn't create texturewindow");

	return hwnd;
}

/*
==================
Texture_Flush
==================
*/
void Texture_Flush (void)
{
}


/*
==================
Texture_Init
==================
*/
void Texture_Init (void)
{
	char	name[1024];
	byte	*pal;

	// load the palette
	sprintf (name, "%s/pics/colormap.pcx",
		ValueForKey (g_qeglobals.d_project_entity, "basepath"));
	Load256Image (name, NULL, &pal, NULL, NULL);
	if (!pal)
		Error ("Couldn't load %s", name);
	Texture_InitPalette (pal);
	free (pal);

	// create the fallback texture
	Texture_MakeNotexture ();

	g_qeglobals.d_qtextures = NULL;
}

