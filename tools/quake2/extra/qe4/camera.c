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

void DrawPathLines (void);

camera_t	camera;

/*
============
Cam_Init
============
*/
void Cam_Init (void)
{
//	camera.draw_mode = cd_texture;
//	camera.draw_mode = cd_solid;
//	camera.draw_mode = cd_wire;

	camera.timing = false;

	camera.origin[0] = 0;
	camera.origin[1] = 20;
	camera.origin[2] = 46;

	camera.color[0] = 0.3;
	camera.color[1] = 0.3;
	camera.color[2] = 0.3;
}


//============================================================================

void Cam_BuildMatrix (void)
{
	float	xa, ya;
	float	matrix[4][4];
	int		i;

	xa = camera.angles[0]/180*Q_PI;
	ya = camera.angles[1]/180*Q_PI;

	// the movement matrix is kept 2d

    camera.forward[0] = cos(ya);
    camera.forward[1] = sin(ya);
    camera.right[0] = camera.forward[1];
    camera.right[1] = -camera.forward[0];

	glGetFloatv (GL_PROJECTION_MATRIX, &matrix[0][0]);

	for (i=0 ; i<3 ; i++)
	{
		camera.vright[i] = matrix[i][0];
		camera.vup[i] = matrix[i][1];
		camera.vpn[i] = matrix[i][2];
	}

	VectorNormalize (camera.vright);
	VectorNormalize (camera.vup);
	VectorNormalize (camera.vpn);
}

//===============================================

/*
===============
Cam_ChangeFloor
===============
*/
void Cam_ChangeFloor (qboolean up)
{
	brush_t	*b;
	float	d, bestd, current;
	vec3_t	start, dir;

	start[0] = camera.origin[0];
	start[1] = camera.origin[1];
	start[2] = 8192;
	dir[0] = dir[1] = 0;
	dir[2] = -1;

	current = 8192 - (camera.origin[2] - 48);
	if (up)
		bestd = 0;
	else
		bestd = 16384;

	for (b=active_brushes.next ; b != &active_brushes ; b=b->next)
	{
		if (!Brush_Ray (start, dir, b, &d))
			continue;
		if (up && d < current && d > bestd)
			bestd = d;
		if (!up && d > current && d < bestd)
			bestd = d;
	}

	if (bestd == 0 || bestd == 16384)
		return;

	camera.origin[2] += current - bestd;
	Sys_UpdateWindows (W_CAMERA|W_Z_OVERLAY);
}


//===============================================

int	cambuttonstate;
static	int	buttonx, buttony;
static	int	cursorx, cursory;

face_t	*side_select;

#define	ANGLE_SPEED	300
#define	MOVE_SPEED	400

/*
================
Cam_PositionDrag
================
*/
void Cam_PositionDrag (void)
{
	int		x, y;

	Sys_GetCursorPos (&x, &y);
	if (x != cursorx || y != cursory)
	{
		x -= cursorx;
		VectorMA (camera.origin, x, camera.vright, camera.origin);
		y -= cursory;
		camera.origin[2] -= y;

		Sys_SetCursorPos (cursorx, cursory);
		Sys_UpdateWindows (W_CAMERA | W_XY_OVERLAY);
	}
}

/*
===============
Cam_MouseControl
===============
*/
void Cam_MouseControl (float dtime)
{
	int		xl, xh;
	int		yl, yh;
	float	xf, yf;

	if (cambuttonstate != MK_RBUTTON)
		return;

	xf = (float)(buttonx - camera.width/2) / (camera.width/2);
	yf = (float)(buttony - camera.height/2) / (camera.height/2);

	xl = camera.width/3;
	xh = xl*2;
	yl = camera.height/3;
	yh = yl*2;

#if 0
	// strafe
	if (buttony < yl && (buttonx < xl || buttonx > xh))
		VectorMA (camera.origin, xf*dtime*MOVE_SPEED, camera.right, camera.origin);
	else
#endif
	{
		xf *= 1.0 - fabs(yf);
		if (xf < 0)
		{
			xf += 0.1;
			if (xf > 0)
				xf = 0;
		}
		else
		{
			xf -= 0.1;
			if (xf < 0)
				xf = 0;
		}

		VectorMA (camera.origin, yf*dtime*MOVE_SPEED, camera.forward, camera.origin);
		camera.angles[YAW] += xf*-dtime*ANGLE_SPEED;
	}
	Sys_UpdateWindows (W_CAMERA|W_XY_OVERLAY);
}




/*
==============
Cam_MouseDown
==============
*/
void Cam_MouseDown (int x, int y, int buttons)
{
	vec3_t		dir;
	float		f, r, u;
	int			i;

	//
	// calc ray direction
	//
	u = (float)(y - camera.height/2) / (camera.width/2);
	r = (float)(x - camera.width/2) / (camera.width/2);
	f = 1;

	for (i=0 ; i<3 ; i++)
		dir[i] = camera.vpn[i] * f + camera.vright[i] * r + camera.vup[i] * u;
	VectorNormalize (dir);

	Sys_GetCursorPos (&cursorx, &cursory);

	cambuttonstate = buttons;
	buttonx = x;
	buttony = y;

	// LBUTTON = manipulate selection
	// shift-LBUTTON = select
	// middle button = grab texture
	// ctrl-middle button = set entire brush to texture
	// ctrl-shift-middle button = set single face to texture
	if ( (buttons == MK_LBUTTON)
		|| (buttons == (MK_LBUTTON | MK_SHIFT))
		|| (buttons == (MK_LBUTTON | MK_CONTROL))
		|| (buttons == (MK_LBUTTON | MK_CONTROL | MK_SHIFT))
		|| (buttons == MK_MBUTTON)
		|| (buttons == (MK_MBUTTON|MK_CONTROL))
		|| (buttons == (MK_MBUTTON|MK_SHIFT|MK_CONTROL)) )
	{
		Drag_Begin (x, y, buttons,
			camera.vright, camera.vup,
			camera.origin, dir);
		return;
	}

	if (buttons == MK_RBUTTON)
	{
		Cam_MouseControl (0.1);
		return;
	}
}

/*
==============
Cam_MouseUp
==============
*/
void Cam_MouseUp (int x, int y, int buttons)
{
	cambuttonstate = 0;
	Drag_MouseUp ();
}


/*
==============
Cam_MouseMoved
==============
*/
void Cam_MouseMoved (int x, int y, int buttons)
{
	cambuttonstate = buttons;
	if (!buttons)
		return;
	buttonx = x;
	buttony = y;

	if (buttons == (MK_RBUTTON|MK_CONTROL) )
	{
		Cam_PositionDrag ();
		Sys_UpdateWindows (W_XY|W_CAMERA|W_Z);
		return;
	}

	Sys_GetCursorPos (&cursorx, &cursory);

	if (buttons & (MK_LBUTTON | MK_MBUTTON) )
	{
		Drag_MouseMoved (x, y, buttons);
		Sys_UpdateWindows (W_XY|W_CAMERA|W_Z);
	}
}


vec3_t	cull1, cull2;
int		cullv1[3], cullv2[3];

void InitCull (void)
{
	int		i;

	VectorSubtract (camera.vpn, camera.vright, cull1);
	VectorAdd (camera.vpn, camera.vright, cull2);

	for (i=0 ; i<3 ; i++)
	{
		if (cull1[i] > 0)
			cullv1[i] = 3+i;
		else
			cullv1[i] = i;
		if (cull2[i] > 0)
			cullv2[i] = 3+i;
		else
			cullv2[i] = i;
	}
}

qboolean CullBrush (brush_t *b)
{
	int		i;
	vec3_t	point;
	float	d;

	for (i=0 ; i<3 ; i++)
		point[i] = b->mins[cullv1[i]] - camera.origin[i];

	d = DotProduct (point, cull1);
	if (d < -1)
		return true;

	for (i=0 ; i<3 ; i++)
		point[i] = b->mins[cullv2[i]] - camera.origin[i];

	d = DotProduct (point, cull2);
	if (d < -1)
		return true;

	return false;
}


/*
==============
Cam_Draw
==============
*/
void Cam_Draw (void)
{
    brush_t	*brush;
	face_t	*face;
	float	screenaspect;
	float	yfov;
	double	start, end;
	int		i;

	if (!active_brushes.next)
		return;	// not valid yet

	if (camera.timing)
		start = Sys_DoubleTime ();

	//
	// clear
	//
	QE_CheckOpenGLForErrors();

	glViewport(0, 0, camera.width, camera.height);
	glScissor(0, 0, camera.width, camera.height);
	glClearColor (
		g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][0],
		g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][1],
		g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][2],
		0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//
	// set up viewpoint
	//
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity ();

    screenaspect = (float)camera.width/camera.height;
	yfov = 2*atan((float)camera.height/camera.width)*180/Q_PI;
    gluPerspective (yfov,  screenaspect,  2,  8192);

    glRotatef (-90,  1, 0, 0);	    // put Z going up
    glRotatef (90,  0, 0, 1);	    // put Z going up
    glRotatef (camera.angles[0],  0, 1, 0);
    glRotatef (-camera.angles[1],  0, 0, 1);
    glTranslatef (-camera.origin[0],  -camera.origin[1],  -camera.origin[2]);

	Cam_BuildMatrix ();

	InitCull ();

	//
	// draw stuff
	//

	switch (camera.draw_mode)
	{
	case cd_wire:
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	    glDisable(GL_TEXTURE_2D);
	    glDisable(GL_TEXTURE_1D);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
	    glColor3f(1.0, 1.0, 1.0);
//		glEnable (GL_LINE_SMOOTH);
		break;

	case cd_solid:
		glCullFace(GL_FRONT);
		glEnable(GL_CULL_FACE);
		glShadeModel (GL_FLAT);

		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_TEXTURE_2D);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc (GL_LEQUAL);
		break;

	case cd_texture:
		glCullFace(GL_FRONT);
		glEnable(GL_CULL_FACE);

		glShadeModel (GL_FLAT);

		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_TEXTURE_2D);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc (GL_LEQUAL);

#if 0

		{
	   GLfloat fogColor[4] = {0.0, 1.0, 0.0, 0.25};

		glFogi (GL_FOG_MODE, GL_LINEAR);
		glHint (GL_FOG_HINT, GL_NICEST);  /*  per pixel   */
		glFogf (GL_FOG_START, -8192);
		glFogf (GL_FOG_END, 65536);
	   glFogfv (GL_FOG_COLOR, fogColor);

		}

#endif
		break;

	case cd_blend:
		glCullFace(GL_FRONT);
		glEnable(GL_CULL_FACE);

		glShadeModel (GL_FLAT);

		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_TEXTURE_2D);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glDisable(GL_DEPTH_TEST);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	}

	glMatrixMode(GL_TEXTURE);
	for (brush = active_brushes.next ; brush != &active_brushes ; brush=brush->next)
	{
		if (CullBrush (brush))
			continue;
		if (FilterBrush (brush))
			continue;

		Brush_Draw( brush );
	}
	glMatrixMode(GL_PROJECTION);

	//
	// now draw selected brushes
	//

	glTranslatef (g_qeglobals.d_select_translate[0], g_qeglobals.d_select_translate[1], g_qeglobals.d_select_translate[2]);
	glMatrixMode(GL_TEXTURE);

	// draw normally
	for (brush = selected_brushes.next ; brush != &selected_brushes ; brush=brush->next)
	{
		Brush_Draw( brush );
	}

	// blend on top
	glMatrixMode(GL_PROJECTION);

	glColor4f(1.0, 0.0, 0.0, 0.3);
	glEnable (GL_BLEND);
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable (GL_TEXTURE_2D);
	for (brush = selected_brushes.next ; brush != &selected_brushes ; brush=brush->next)
		for (face=brush->brush_faces ; face ; face=face->next)
			Face_Draw( face );
	if (selected_face)
		Face_Draw(selected_face);

	// non-zbuffered outline

	glDisable (GL_BLEND);
	glDisable (GL_DEPTH_TEST);
	glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	glColor3f (1, 1, 1);
	for (brush = selected_brushes.next ; brush != &selected_brushes ; brush=brush->next)
		for (face=brush->brush_faces ; face ; face=face->next)
			Face_Draw( face );

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

	//
	// draw pointfile
	//
	glEnable(GL_DEPTH_TEST);

	DrawPathLines ();

	if (g_qeglobals.d_pointfile_display_list)
	{
		Pointfile_Draw();
//		glCallList (g_qeglobals.d_pointfile_display_list);
	}

	// bind back to the default texture so that we don't have problems
	// elsewhere using/modifying texture maps between contexts
	glBindTexture( GL_TEXTURE_2D, 0 );

    glFinish();
	QE_CheckOpenGLForErrors();
//	Sys_EndWait();
	if (camera.timing)
	{
		end = Sys_DoubleTime ();
		Sys_Printf ("Camera: %i ms\n", (int)(1000*(end-start)));
	}
}

