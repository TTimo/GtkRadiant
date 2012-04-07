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

#include "texpaint.h"

triangle_t	*faces;
int		numfaces;

int		skinwidth, skinheight;
int		picwidth, picheight;
int		width, height;
int		iwidth, iheight;
int		width2, height2;		// padded to ^2

float	tmcoords[10000][3][2];

byte		pic[1024*512];
unsigned	rgb[1024*512];

float	scale;
float	s_scale, t_scale;

char	filename[1024];
char	picfilename[1024];


/*
================
BoundFaces
================
*/
vec3_t	mins, maxs;

void BoundFaces (void)
{
	int		i,j,k;
	triangle_t	*pol;
	float	v;

	for (i=0 ; i<3 ; i++)
	{
		mins[i] = 9999;
		maxs[i] = -9999;
	}

	for (i=0 ; i<numfaces ; i++)
	{
		pol = &faces[i];
		for (j=0 ; j<3 ; j++)
			for (k=0 ; k<3 ; k++)
			{
				v = pol->verts[j][k];
				if (v<mins[k])
					mins[k] = v;
				if (v>maxs[k])
					maxs[k] = v;
			}
	}

	for (i=0 ; i<3 ; i++)
	{
		mins[i] = floor(mins[i]);
		maxs[i] = ceil(maxs[i]);
	}

	width = maxs[0] - mins[0];
	height = maxs[2] - mins[2];

	printf ("width: %i  height: %i\n",width, height);

	if (!skinwidth)
	{	// old way
		scale = 8;
		if (width*scale >= 150)
			scale = 150.0 / width;
		if (height*scale >= 190)
			scale = 190.0 / height;
		s_scale = t_scale = scale;
		iwidth = ceil(width*scale) + 4;
		iheight = ceil(height*scale) + 4;
	}
	else
	{	// new way
		s_scale = (skinwidth/2-4)/(float)width;
		t_scale = (skinheight-4)/(float)height;
		iwidth = skinwidth/2;
		iheight = skinheight;
	}

	printf ("scale: %f\n",scale);
	printf ("iwidth: %i  iheight: %i\n",iwidth, iheight);
}



/*
============
AddFace
============
*/
void AddFace (int facenum, triangle_t *f)
{
	vec3_t		v1, v2, normal;
	int		basex, basey;
	int			i, j;
	int		coords[3][2];

//
// determine which side to map the teture to
//
	VectorSubtract (f->verts[0], f->verts[1], v1);
	VectorSubtract (f->verts[2], f->verts[1], v2);
	CrossProduct (v1, v2, normal);

	if (normal[1] > 0)
		basex = iwidth + 2;
	else
		basex = 2;
	basey = 2;

	for (i=0 ; i<3 ; i++)
	{
		coords[i][0] = Q_rint((f->verts[i][0] - mins[0])*s_scale + basex);
		coords[i][1] = Q_rint( (maxs[2] - f->verts[i][2])*t_scale + basey);
tmcoords[facenum][i][0] = coords[i][0]/(float)width2;
tmcoords[facenum][i][1] = coords[i][1]/(float)height2;
	}

}


void CalcTmCoords (void)
{
	int		j;

	BoundFaces ();

	for (j=0 ; j<numfaces ; j++)
		AddFace (j, &faces[j]);

	printf ("numfaces: %i\n",numfaces);
}

//===============================================================================



#define	MAX_NUM_ARGVS	32
int		argc;
char	*argv[MAX_NUM_ARGVS];

/*
============
ParseCommandLine
============
*/
void ParseCommandLine (char *lpCmdLine)
{
	argc = 1;
	argv[0] = "programname";

	while (*lpCmdLine && (argc < MAX_NUM_ARGVS))
	{
		while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
			lpCmdLine++;

		if (*lpCmdLine)
		{
			argv[argc] = lpCmdLine;
			argc++;

			while (*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine <= 126)))
				lpCmdLine++;

			if (*lpCmdLine)
			{
				*lpCmdLine = 0;
				lpCmdLine++;
			}

		}
	}
}

/*
=================
LoadTriFile
=================
*/
void LoadTriFile (char *name)
{
	strcpy (tri_filename, name);
	SetWindowText (camerawindow, tri_filename);

	LoadTriangleList (tri_filename, &faces, &numfaces);
	InvalidateRect (camerawindow, NULL, false);
}

/*
==================
TimerProc

==================
*/
int CALLBACK TimerProc(
    HWND hwnd,	// handle of window for timer messages
    UINT uMsg,	// WM_TIMER message
    UINT idEvent,	// timer identifier
    DWORD dwTime 	// current system time
   )
{
	static int	counter;
	char		name[1024];

	if (!skin_filename[0])
		return 0;

	if (!modified_past_autosave)
	{
		counter = 0;
		return 0;
	}

	counter++;

	if (counter < 3*5)
		return 0;		// save every five minutes

	strcpy (name, skin_filename);
	StripExtension (name);
	strcat (name, "_autosave.lbm");
	Skin_SaveFile (name);

	modified_past_autosave = false;
	counter = 0;

	return 0;
}

/*
==================
WinMain

==================
*/
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance
					,LPSTR lpCmdLine, int nCmdShow)
{
    MSG			msg;
	HACCEL		accelerators;

	main_instance = hInstance;

	ParseCommandLine (lpCmdLine);

	screen_width = GetSystemMetrics (SM_CXFULLSCREEN);
	screen_height = GetSystemMetrics (SM_CYFULLSCREEN);

	// hack for broken NT 4.0 dual screen
	if (screen_width > 2*screen_height)
		screen_width /= 2;

	accelerators = LoadAccelerators (hInstance
		, MAKEINTRESOURCE(IDR_ACCELERATOR1));
	if (!accelerators)
		Sys_Error ("LoadAccelerators failed");

	Main_Create (hInstance);
	WCam_Create (hInstance);
	WPal_Create (hInstance);
	WSkin_Create (hInstance);

	if (argc == 2)
		Skin_LoadFile (argv[1]);

	SetTimer ( mainwindow, 1, 1000*20, TimerProc );

	while (1)
	{
		if (!GetMessage (&msg, mainwindow, 0, 0))
			break;
		if (!TranslateAccelerator(mainwindow, accelerators, &msg) )
		{
      		TranslateMessage (&msg);
      		DispatchMessage (&msg);
		}
	}

    /* return success of application */
    return TRUE;
}

