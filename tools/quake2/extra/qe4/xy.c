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

#define	PAGEFLIPS	2

/*
============
XY_Init
============
*/
void XY_Init (void)
{
	g_qeglobals.d_xy.origin[0] = 0;
	g_qeglobals.d_xy.origin[1] = 20;
	g_qeglobals.d_xy.origin[2] = 46;

	g_qeglobals.d_xy.scale = 1;
}


/*
============================================================================

  MOUSE ACTIONS

============================================================================
*/

static	int	cursorx, cursory;
static	int	buttonstate;
static	int	pressx, pressy;
static	vec3_t	pressdelta;
static	qboolean	press_selection;

void XY_ToPoint (int x, int y, vec3_t point)
{
	point[0] = g_qeglobals.d_xy.origin[0] + (x - g_qeglobals.d_xy.width/2)/g_qeglobals.d_xy.scale;
	point[1] = g_qeglobals.d_xy.origin[1] + (y - g_qeglobals.d_xy.height/2)/g_qeglobals.d_xy.scale;
	point[2] = 0;
}

void XY_ToGridPoint (int x, int y, vec3_t point)
{
	point[0] = g_qeglobals.d_xy.origin[0] + (x - g_qeglobals.d_xy.width/2)/g_qeglobals.d_xy.scale;
	point[1] = g_qeglobals.d_xy.origin[1] + (y - g_qeglobals.d_xy.height/2)/g_qeglobals.d_xy.scale;
	point[2] = 0;
	point[0] = floor(point[0]/g_qeglobals.d_gridsize+0.5)*g_qeglobals.d_gridsize;
	point[1] = floor(point[1]/g_qeglobals.d_gridsize+0.5)*g_qeglobals.d_gridsize;
}

/*
==============
XY_MouseDown
==============
*/
void XY_MouseDown (int x, int y, int buttons)
{
	vec3_t	point;
	vec3_t	origin, dir, right, up;

	buttonstate = buttons;
	pressx = x;
	pressy = y;
	VectorCopy (vec3_origin, pressdelta);

	XY_ToPoint (x, y, point);

	VectorCopy (point, origin);
	origin[2] = 8192;

	dir[0] = 0; dir[1] = 0; dir[2] = -1;
	right[0] = 1/g_qeglobals.d_xy.scale; right[1] = 0; right[2] = 0;
	up[0] = 0; up[1] = 1/g_qeglobals.d_xy.scale; up[2] = 0;

	press_selection = (selected_brushes.next != &selected_brushes);

	Sys_GetCursorPos (&cursorx, &cursory);

	// lbutton = manipulate selection
	// shift-LBUTTON = select
	if ( (buttons == MK_LBUTTON)
		|| (buttons == (MK_LBUTTON | MK_SHIFT))
		|| (buttons == (MK_LBUTTON | MK_CONTROL))
		|| (buttons == (MK_LBUTTON | MK_CONTROL | MK_SHIFT)) )
	{
		Drag_Begin (x, y, buttons,
			right, up,
			origin, dir);
		return;
	}

	// control mbutton = move camera
	if (buttonstate == (MK_CONTROL|MK_MBUTTON) )
	{
		camera.origin[0] = point[0];
		camera.origin[1] = point[1];
		Sys_UpdateWindows (W_CAMERA|W_XY_OVERLAY);
	}

	// mbutton = angle camera
	if (buttonstate == MK_MBUTTON)
	{
		VectorSubtract (point, camera.origin, point);
		if (point[1] || point[0])
		{
			camera.angles[YAW] = 180/Q_PI*atan2 (point[1], point[0]);
			Sys_UpdateWindows (W_CAMERA|W_XY_OVERLAY);
		}
	}

	// shift mbutton = move z checker
	if (buttonstate == (MK_SHIFT|MK_MBUTTON) )
	{
		XY_ToPoint (x, y, point);
		z.origin[0] = point[0];
		z.origin[1] = point[1];
		Sys_UpdateWindows (W_XY_OVERLAY|W_Z);
		return;
	}

}

/*
==============
XY_MouseUp
==============
*/
void XY_MouseUp (int x, int y, int buttons)
{
	Drag_MouseUp ();

	if (!press_selection)
		Sys_UpdateWindows (W_ALL);

	buttonstate = 0;
}

qboolean DragDelta (int x, int y, vec3_t move)
{
	vec3_t	xvec, yvec, delta;
	int		i;

	xvec[0] = 1/g_qeglobals.d_xy.scale;
	xvec[1] = xvec[2] = 0;
	yvec[1] = 1/g_qeglobals.d_xy.scale;
	yvec[0] = yvec[2] = 0;

	for (i=0 ; i<3 ; i++)
	{
		delta[i] = xvec[i]*(x - pressx) + yvec[i]*(y - pressy);
		delta[i] = floor(delta[i]/g_qeglobals.d_gridsize+0.5)*g_qeglobals.d_gridsize;
	}
	VectorSubtract (delta, pressdelta, move);
	VectorCopy (delta, pressdelta);

	if (move[0] || move[1] || move[2])
		return true;
	return false;
}

/*
==============
NewBrushDrag
==============
*/
void NewBrushDrag (int x, int y)
{
	vec3_t	mins, maxs, junk;
	int		i;
	float	temp;
	brush_t	*n;

	if (!DragDelta (x,y, junk))
		return;
	// delete the current selection
	if (selected_brushes.next != &selected_brushes)
		Brush_Free (selected_brushes.next);
	XY_ToGridPoint (pressx, pressy, mins);
	mins[2] = g_qeglobals.d_gridsize * ((int)(g_qeglobals.d_new_brush_bottom_z/g_qeglobals.d_gridsize));
	XY_ToGridPoint (x, y, maxs);
	maxs[2] = g_qeglobals.d_gridsize * ((int)(g_qeglobals.d_new_brush_top_z/g_qeglobals.d_gridsize));
	if (maxs[2] <= mins[2])
		maxs[2] = mins[2] + g_qeglobals.d_gridsize;

	for (i=0 ; i<3 ; i++)
	{
		if (mins[i] == maxs[i])
			return;	// don't create a degenerate brush
		if (mins[i] > maxs[i])
		{
			temp = mins[i];
			mins[i] = maxs[i];
			maxs[i] = temp;
		}
	}

	n = Brush_Create (mins, maxs, &g_qeglobals.d_texturewin.texdef);
	if (!n)
		return;

	Brush_AddToList (n, &selected_brushes);

	Entity_LinkBrush (world_entity, n);

	Brush_Build( n );

//	Sys_UpdateWindows (W_ALL);
	Sys_UpdateWindows (W_XY| W_CAMERA);
}

/*
==============
XY_MouseMoved
==============
*/
void XY_MouseMoved (int x, int y, int buttons)
{
	vec3_t	point;

	if (!buttonstate)
		return;

	// lbutton without selection = drag new brush
	if (buttonstate == MK_LBUTTON && !press_selection)
	{
		NewBrushDrag (x, y);
		return;
	}

	// lbutton (possibly with control and or shift)
	// with selection = drag selection
	if (buttonstate & MK_LBUTTON)
	{
		Drag_MouseMoved (x, y, buttons);
		Sys_UpdateWindows (W_XY_OVERLAY | W_CAMERA);
		return;
	}

	// control mbutton = move camera
	if (buttonstate == (MK_CONTROL|MK_MBUTTON) )
	{
		XY_ToPoint (x, y, point);
		camera.origin[0] = point[0];
		camera.origin[1] = point[1];
		Sys_UpdateWindows (W_XY_OVERLAY | W_CAMERA);
		return;
	}

	// shift mbutton = move z checker
	if (buttonstate == (MK_SHIFT|MK_MBUTTON) )
	{
		XY_ToPoint (x, y, point);
		z.origin[0] = point[0];
		z.origin[1] = point[1];
		Sys_UpdateWindows (W_XY_OVERLAY|W_Z);
		return;
	}

	// mbutton = angle camera
	if (buttonstate == MK_MBUTTON )
	{
		XY_ToPoint (x, y, point);
		VectorSubtract (point, camera.origin, point);
		if (point[1] || point[0])
		{
			camera.angles[YAW] = 180/Q_PI*atan2 (point[1], point[0]);
			Sys_UpdateWindows (W_XY_OVERLAY | W_CAMERA);
		}
		return;
	}

	// rbutton = drag xy origin
	if (buttonstate == MK_RBUTTON)
	{
		Sys_GetCursorPos (&x, &y);
		if (x != cursorx || y != cursory)
		{
			g_qeglobals.d_xy.origin[0] -= (x-cursorx)/g_qeglobals.d_xy.scale;
			g_qeglobals.d_xy.origin[1] += (y-cursory)/g_qeglobals.d_xy.scale;
			Sys_SetCursorPos (cursorx, cursory);
			Sys_UpdateWindows (W_XY | W_XY_OVERLAY);
		}
		return;
	}
}


/*
============================================================================

DRAWING

============================================================================
*/


/*
==============
XY_DrawGrid
==============
*/
void XY_DrawGrid (void)
{
	float	x, y, xb, xe, yb, ye;
	int		w, h;
	char	text[32];

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	w = g_qeglobals.d_xy.width/2 / g_qeglobals.d_xy.scale;
	h = g_qeglobals.d_xy.height/2 / g_qeglobals.d_xy.scale;

	xb = g_qeglobals.d_xy.origin[0] - w;
	if (xb < region_mins[0])
		xb = region_mins[0];
	xb = 64 * floor (xb/64);

	xe = g_qeglobals.d_xy.origin[0] + w;
	if (xe > region_maxs[0])
		xe = region_maxs[0];
	xe = 64 * ceil (xe/64);

	yb = g_qeglobals.d_xy.origin[1] - h;
	if (yb < region_mins[1])
		yb = region_mins[1];
	yb = 64 * floor (yb/64);

	ye = g_qeglobals.d_xy.origin[1] + h;
	if (ye > region_maxs[1])
		ye = region_maxs[1];
	ye = 64 * ceil (ye/64);

	// draw major blocks

	glColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_GRIDMAJOR]);

	if ( g_qeglobals.d_showgrid )
	{

		glBegin (GL_LINES);

		for (x=xb ; x<=xe ; x+=64)
		{
			glVertex2f (x, yb);
			glVertex2f (x, ye);
		}
		for (y=yb ; y<=ye ; y+=64)
		{
			glVertex2f (xb, y);
			glVertex2f (xe, y);
		}

		glEnd ();

	}

	// draw minor blocks
	if ( g_qeglobals.d_showgrid && g_qeglobals.d_gridsize*g_qeglobals.d_xy.scale >= 4)
	{
		glColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR]);

		glBegin (GL_LINES);
		for (x=xb ; x<xe ; x += g_qeglobals.d_gridsize)
		{
			if ( ! ((int)x & 63) )
				continue;
			glVertex2f (x, yb);
			glVertex2f (x, ye);
		}
		for (y=yb ; y<ye ; y+=g_qeglobals.d_gridsize)
		{
			if ( ! ((int)y & 63) )
				continue;
			glVertex2f (xb, y);
			glVertex2f (xe, y);
		}
		glEnd ();
	}

	// draw coordinate text if needed

	if ( g_qeglobals.d_savedinfo.show_coordinates)
	{
		glColor4f(0, 0, 0, 0);

		for (x=xb ; x<xe ; x+=64)
		{
			glRasterPos2f (x, g_qeglobals.d_xy.origin[1] + h - 6/g_qeglobals.d_xy.scale);
			sprintf (text, "%i",(int)x);
			glCallLists (strlen(text), GL_UNSIGNED_BYTE, text);
		}
		for (y=yb ; y<ye ; y+=64)
		{
			glRasterPos2f (g_qeglobals.d_xy.origin[0] - w + 1, y);
			sprintf (text, "%i",(int)y);
			glCallLists (strlen(text), GL_UNSIGNED_BYTE, text);
		}
	}
}

/*
==============
XY_DrawBlockGrid
==============
*/
void XY_DrawBlockGrid (void)
{
	float	x, y, xb, xe, yb, ye;
	int		w, h;
	char	text[32];

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	w = g_qeglobals.d_xy.width/2 / g_qeglobals.d_xy.scale;
	h = g_qeglobals.d_xy.height/2 / g_qeglobals.d_xy.scale;

	xb = g_qeglobals.d_xy.origin[0] - w;
	if (xb < region_mins[0])
		xb = region_mins[0];
	xb = 1024 * floor (xb/1024);

	xe = g_qeglobals.d_xy.origin[0] + w;
	if (xe > region_maxs[0])
		xe = region_maxs[0];
	xe = 1024 * ceil (xe/1024);

	yb = g_qeglobals.d_xy.origin[1] - h;
	if (yb < region_mins[1])
		yb = region_mins[1];
	yb = 1024 * floor (yb/1024);

	ye = g_qeglobals.d_xy.origin[1] + h;
	if (ye > region_maxs[1])
		ye = region_maxs[1];
	ye = 1024 * ceil (ye/1024);

	// draw major blocks

	glColor3f(0,0,1);
	glLineWidth (2);

	glBegin (GL_LINES);

	for (x=xb ; x<=xe ; x+=1024)
	{
		glVertex2f (x, yb);
		glVertex2f (x, ye);
	}
	for (y=yb ; y<=ye ; y+=1024)
	{
		glVertex2f (xb, y);
		glVertex2f (xe, y);
	}

	glEnd ();
	glLineWidth (1);

	// draw coordinate text if needed

	for (x=xb ; x<xe ; x+=1024)
		for (y=yb ; y<ye ; y+=1024)
		{
			glRasterPos2f (x+512, y+512);
			sprintf (text, "%i,%i",(int)floor(x/1024), (int)floor(y/1024) );
			glCallLists (strlen(text), GL_UNSIGNED_BYTE, text);
		}

	glColor4f(0, 0, 0, 0);
}


void DrawCameraIcon (void)
{
	float	x, y, a;

	x = camera.origin[0];
	y = camera.origin[1];
	a = camera.angles[YAW]/180*Q_PI;

	glColor3f (0.0, 0.0, 1.0);
	glBegin(GL_LINE_STRIP);
	glVertex3f (x-16,y,0);
	glVertex3f (x,y+8,0);
	glVertex3f (x+16,y,0);
	glVertex3f (x,y-8,0);
	glVertex3f (x-16,y,0);
	glVertex3f (x+16,y,0);
	glEnd ();

	glBegin(GL_LINE_STRIP);
	glVertex3f (x+48*cos(a+Q_PI/4), y+48*sin(a+Q_PI/4), 0);
	glVertex3f (x, y, 0);
	glVertex3f (x+48*cos(a-Q_PI/4), y+48*sin(a-Q_PI/4), 0);
	glEnd ();

}

void DrawZIcon (void)
{
	float	x, y;

	x = z.origin[0];
	y = z.origin[1];

	glEnable (GL_BLEND);
	glDisable (GL_TEXTURE_2D);
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glDisable (GL_CULL_FACE);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f (0.0, 0.0, 1.0, 0.25);
	glBegin(GL_QUADS);
	glVertex3f (x-8,y-8,0);
	glVertex3f (x+8,y-8,0);
	glVertex3f (x+8,y+8,0);
	glVertex3f (x-8,y+8,0);
	glEnd ();
	glDisable (GL_BLEND);

	glColor4f (0.0, 0.0, 1.0, 1);

	glBegin(GL_LINE_LOOP);
	glVertex3f (x-8,y-8,0);
	glVertex3f (x+8,y-8,0);
	glVertex3f (x+8,y+8,0);
	glVertex3f (x-8,y+8,0);
	glEnd ();

	glBegin(GL_LINE_STRIP);
	glVertex3f (x-4,y+4,0);
	glVertex3f (x+4,y+4,0);
	glVertex3f (x-4,y-4,0);
	glVertex3f (x+4,y-4,0);
	glEnd ();
}


/*
==================
FilterBrush
==================
*/
BOOL FilterBrush(brush_t *pb)
{
	if (!pb->owner)
		return FALSE;		// during construction

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_CLIP)
	{
		if (!strncmp(pb->brush_faces->texdef.name, "clip", 4))
			return TRUE;
	}

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_WATER)
	{
		if (pb->brush_faces->texdef.name[0] == '*')
			return TRUE;
	}

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_DETAIL)
	{
		if (pb->brush_faces->texdef.contents & CONTENTS_DETAIL)
			return TRUE;
	}

	if (pb->owner == world_entity)
	{
		if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_WORLD)
			return TRUE;
		return FALSE;
	}
	else if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_ENT)
		return TRUE;

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_LIGHTS)
	{
		if (!strncmp(pb->owner->eclass->name, "light", 5))
			return TRUE;
	}

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_PATHS)
	{
		if (!strncmp(pb->owner->eclass->name, "path", 4))
			return TRUE;
	}

	return FALSE;
}

/*
=============================================================

  PATH LINES

=============================================================
*/

/*
==================
DrawPathLines

Draws connections between entities.
Needs to consider all entities, not just ones on screen,
because the lines can be visible when neither end is.
Called for both camera view and xy view.
==================
*/
void DrawPathLines (void)
{
	int		i, j, k;
	vec3_t	mid, mid1;
	entity_t *se, *te;
	brush_t	*sb, *tb;
	char	*psz;
	vec3_t	dir, s1, s2;
	vec_t	len, f;
	int		arrows;
	int			num_entities;
	char		*ent_target[MAX_MAP_ENTITIES];
	entity_t	*ent_entity[MAX_MAP_ENTITIES];


	num_entities = 0;
	for (te = entities.next ; te != &entities && num_entities != MAX_MAP_ENTITIES ; te = te->next)
	{
		ent_target[num_entities] = ValueForKey (te, "target");
		if (ent_target[num_entities][0])
		{
			ent_entity[num_entities] = te;
			num_entities++;
		}
	}

	for (se = entities.next ; se != &entities ; se = se->next)
	{
		psz = ValueForKey(se, "targetname");

		if (psz == NULL || psz[0] == '\0')
			continue;

		sb = se->brushes.onext;
		if (sb == &se->brushes)
			continue;

		for (k=0 ; k<num_entities ; k++)
		{
			if (strcmp (ent_target[k], psz))
				continue;

			te = ent_entity[k];
			tb = te->brushes.onext;
			if (tb == &te->brushes)
				continue;

			for (i=0 ; i<3 ; i++)
				mid[i] = (sb->mins[i] + sb->maxs[i])*0.5;

			for (i=0 ; i<3 ; i++)
				mid1[i] = (tb->mins[i] + tb->maxs[i])*0.5;

			VectorSubtract (mid1, mid, dir);
			len = VectorNormalize (dir);
			s1[0] = -dir[1]*8 + dir[0]*8;
			s2[0] = dir[1]*8 + dir[0]*8;
			s1[1] = dir[0]*8 + dir[1]*8;
			s2[1] = -dir[0]*8 + dir[1]*8;

			glColor3f (se->eclass->color[0], se->eclass->color[1], se->eclass->color[2]);

			glBegin(GL_LINES);
			glVertex3fv(mid);
			glVertex3fv(mid1);

			arrows = (int)(len / 256) + 1;

			for (i=0 ; i<arrows ; i++)
			{
				f = len * (i + 0.5) / arrows;

				for (j=0 ; j<3 ; j++)
					mid1[j] = mid[j] + f*dir[j];
				glVertex3fv (mid1);
				glVertex3f (mid1[0] + s1[0], mid1[1] + s1[1], mid1[2]);
				glVertex3fv (mid1);
				glVertex3f (mid1[0] + s2[0], mid1[1] + s2[1], mid1[2]);
			}

			glEnd();
		}
	}

	return;
}

//=============================================================


/*
==============
XY_Draw
==============
*/
void XY_Draw (void)
{
    brush_t	*brush;
	float	w, h;
	entity_t	*e;
	double	start, end;
	vec3_t	mins, maxs;
	int		drawn, culled;
	int		i;

	if (!active_brushes.next)
		return;	// not valid yet

	if (g_qeglobals.d_xy.timing)
		start = Sys_DoubleTime ();

	//
	// clear
	//
	g_qeglobals.d_xy.d_dirty = false;

	glViewport(0, 0, g_qeglobals.d_xy.width, g_qeglobals.d_xy.height);
	glClearColor (
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][0],
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][1],
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][2],
		0);

    glClear(GL_COLOR_BUFFER_BIT);

	//
	// set up viewpoint
	//
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity ();

	w = g_qeglobals.d_xy.width/2 / g_qeglobals.d_xy.scale;
	h = g_qeglobals.d_xy.height/2 / g_qeglobals.d_xy.scale;
	mins[0] = g_qeglobals.d_xy.origin[0] - w;
	maxs[0] = g_qeglobals.d_xy.origin[0] + w;
	mins[1] = g_qeglobals.d_xy.origin[1] - h;
	maxs[1] = g_qeglobals.d_xy.origin[1] + h;

	glOrtho (mins[0], maxs[0], mins[1], maxs[1], -8000, 8000);

	//
	// now draw the grid
	//
	XY_DrawGrid ();

	//
	// draw stuff
	//
    glShadeModel (GL_FLAT);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glColor3f(0, 0, 0);
//		glEnable (GL_LINE_SMOOTH);

	drawn = culled = 0;

	e = NULL;
	for (brush = active_brushes.next ; brush != &active_brushes ; brush=brush->next)
	{
		if (brush->mins[0] > maxs[0]
			|| brush->mins[1] > maxs[1]
			|| brush->maxs[0] < mins[0]
			|| brush->maxs[1] < mins[1]	)
		{
			culled++;
			continue;		// off screen
		}

		if (FilterBrush (brush))
			continue;
		drawn++;
		if (brush->owner != e)
		{
			e = brush->owner;
			glColor3fv(e->eclass->color);
		}
		Brush_DrawXY( brush );
	}

	DrawPathLines ();

	//
	// draw pointfile
	//
	if ( g_qeglobals.d_pointfile_display_list)
		glCallList (g_qeglobals.d_pointfile_display_list);

	//
	// draw block grid
	//
	if ( g_qeglobals.show_blocks)
		XY_DrawBlockGrid ();

	//
	// now draw selected brushes
	//
	glTranslatef( g_qeglobals.d_select_translate[0], g_qeglobals.d_select_translate[1], g_qeglobals.d_select_translate[2]);

	glColor3f(1.0, 0.0, 0.0);
	glEnable (GL_LINE_STIPPLE);
	glLineStipple (3, 0xaaaa);
	glLineWidth (2);

	for (brush = selected_brushes.next ; brush != &selected_brushes ; brush=brush->next)
	{
		drawn++;
		Brush_DrawXY( brush );
	}

	glDisable (GL_LINE_STIPPLE);
	glLineWidth (1);

	// edge / vertex flags

	if (g_qeglobals.d_select_mode == sel_vertex)
	{
		glPointSize (4);
		glColor3f (0,1,0);
		glBegin (GL_POINTS);
		for (i=0 ; i<g_qeglobals.d_numpoints ; i++)
			glVertex3fv (g_qeglobals.d_points[i]);
		glEnd ();
		glPointSize (1);
	}
	else if (g_qeglobals.d_select_mode == sel_edge)
	{
		float	*v1, *v2;

		glPointSize (4);
		glColor3f (0,0,1);
		glBegin (GL_POINTS);
		for (i=0 ; i<g_qeglobals.d_numedges ; i++)
		{
			v1 = g_qeglobals.d_points[g_qeglobals.d_edges[i].p1];
			v2 = g_qeglobals.d_points[g_qeglobals.d_edges[i].p2];
			glVertex3f ( (v1[0]+v2[0])*0.5,(v1[1]+v2[1])*0.5,(v1[2]+v2[2])*0.5);
		}
		glEnd ();
		glPointSize (1);
	}
	glTranslatef (-g_qeglobals.d_select_translate[0], -g_qeglobals.d_select_translate[1], -g_qeglobals.d_select_translate[2]);

	//
	// now draw camera point
	//
	DrawCameraIcon ();
	DrawZIcon ();

    glFinish();
	QE_CheckOpenGLForErrors();

	if (g_qeglobals.d_xy.timing)
	{
		end = Sys_DoubleTime ();
		Sys_Printf ("xy: %i ms\n", (int)(1000*(end-start)));
	}
}

/*
==============
XY_Overlay
==============
*/
void XY_Overlay (void)
{
	int	w, h;
	int	r[4];
	static	vec3_t	lastz;
	static	vec3_t	lastcamera;


	glViewport(0, 0, g_qeglobals.d_xy.width, g_qeglobals.d_xy.height);

	//
	// set up viewpoint
	//
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity ();

	w = g_qeglobals.d_xy.width/2 / g_qeglobals.d_xy.scale;
	h = g_qeglobals.d_xy.height/2 / g_qeglobals.d_xy.scale;
	glOrtho (g_qeglobals.d_xy.origin[0] - w, g_qeglobals.d_xy.origin[0] + w
		, g_qeglobals.d_xy.origin[1] - h, g_qeglobals.d_xy.origin[1] + h, -8000, 8000);
	//
	// erase the old camera and z checker positions
	// if the entire xy hasn't been redrawn
	//
	if (g_qeglobals.d_xy.d_dirty)
	{
		glReadBuffer (GL_BACK);
		glDrawBuffer (GL_FRONT);

		glRasterPos2f (lastz[0]-9, lastz[1]-9);
		glGetIntegerv (GL_CURRENT_RASTER_POSITION,r);
		glCopyPixels(r[0], r[1], 18,18, GL_COLOR);

		glRasterPos2f (lastcamera[0]-50, lastcamera[1]-50);
		glGetIntegerv (GL_CURRENT_RASTER_POSITION,r);
		glCopyPixels(r[0], r[1], 100,100, GL_COLOR);
	}
	g_qeglobals.d_xy.d_dirty = true;

	//
	// save off underneath where we are about to draw
	//
	VectorCopy (z.origin, lastz);
	VectorCopy (camera.origin, lastcamera);

	glReadBuffer (GL_FRONT);
	glDrawBuffer (GL_BACK);

	glRasterPos2f (lastz[0]-9, lastz[1]-9);
	glGetIntegerv (GL_CURRENT_RASTER_POSITION,r);
	glCopyPixels(r[0], r[1], 18,18, GL_COLOR);

	glRasterPos2f (lastcamera[0]-50, lastcamera[1]-50);
	glGetIntegerv (GL_CURRENT_RASTER_POSITION,r);
	glCopyPixels(r[0], r[1], 100,100, GL_COLOR);

	//
	// draw the new icons
	//
	glDrawBuffer (GL_FRONT);

    glShadeModel (GL_FLAT);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glColor3f(0, 0, 0);

	DrawCameraIcon ();
	DrawZIcon ();

	glDrawBuffer (GL_BACK);
    glFinish();
}

