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

QEGlobals_t  g_qeglobals;

void QE_CheckOpenGLForErrors(void)
{
    int	    i;

    while ( ( i = glGetError() ) != GL_NO_ERROR )
    {
		char buffer[100];

		sprintf( buffer, "OpenGL Error: %s", gluErrorString( i ) );

		MessageBox( g_qeglobals.d_hwndMain, buffer , "QuakeEd Error", MB_OK | MB_ICONEXCLAMATION );
		exit( 1 );
    }
}


char *ExpandReletivePath (char *p)
{
	static char	temp[1024];
	char	*base;

	if (!p || !p[0])
		return NULL;
	if (p[0] == '/' || p[0] == '\\')
		return p;

	base = ValueForKey(g_qeglobals.d_project_entity, "basepath");
	sprintf (temp, "%s/%s", base, p);
	return temp;
}



void *qmalloc (int size)
{
	void *b;
	b = malloc(size);
	memset (b, 0, size);
	return b;
}

char *copystring (char *s)
{
	char	*b;
	b = malloc(strlen(s)+1);
	strcpy (b,s);
	return b;
}

/*
===============
QE_CheckAutoSave

If five minutes have passed since making a change
and the map hasn't been saved, save it out.
===============
*/
void QE_CheckAutoSave( void )
{
	static clock_t s_start;
	clock_t        now;

	now = clock();

	if ( modified != 1 || !s_start)
	{
		s_start = now;
		return;
	}

	if ( now - s_start > ( CLOCKS_PER_SEC * 60 * QE_AUTOSAVE_INTERVAL ) )
	{
		Sys_Printf ("Autosaving...\n");
		Sys_Status ("Autosaving...", 0 );

		Map_SaveFile (ValueForKey(g_qeglobals.d_project_entity, "autosave"), false);

		Sys_Status ("Autosaving...Saved.", 0 );
		modified = 2;
		s_start = now;
	}
}



/*
===========
QE_LoadProject
===========
*/
qboolean QE_LoadProject (char *projectfile)
{
	char	*data;

	Sys_Printf ("QE_LoadProject (%s)\n", projectfile);

	if ( LoadFileNoCrash (projectfile, (void *)&data) == -1)
		return false;
	StartTokenParsing (data);
	g_qeglobals.d_project_entity = Entity_Parse (true);
	if (!g_qeglobals.d_project_entity)
		Error ("Couldn't parse %s", projectfile);
	free (data);

	Eclass_InitForSourceDirectory (ValueForKey (g_qeglobals.d_project_entity, "entitypath"));

	FillClassList ();		// list in entity window

	Map_New ();

	FillTextureMenu ();
	FillBSPMenu ();

	return true;
}

/*
===========
QE_KeyDown
===========
*/
#define	SPEED_MOVE	32
#define	SPEED_TURN	22.5

qboolean QE_KeyDown (int key)
{
	switch (key)
	{
	case 'K':
		PostMessage( g_qeglobals.d_hwndMain, WM_COMMAND, ID_MISC_SELECTENTITYCOLOR, 0 );
		break;

	case VK_UP:
		VectorMA (camera.origin, SPEED_MOVE, camera.forward, camera.origin);
		Sys_UpdateWindows (W_CAMERA|W_XY_OVERLAY);
		break;
	case VK_DOWN:
		VectorMA (camera.origin, -SPEED_MOVE, camera.forward, camera.origin);
		Sys_UpdateWindows (W_CAMERA|W_XY_OVERLAY);
		break;
	case VK_LEFT:
		camera.angles[1] += SPEED_TURN;
		Sys_UpdateWindows (W_CAMERA|W_XY_OVERLAY);
		break;
	case VK_RIGHT:
		camera.angles[1] -= SPEED_TURN;
		Sys_UpdateWindows (W_CAMERA|W_XY_OVERLAY);
		break;
	case 'D':
		camera.origin[2] += SPEED_MOVE;
		Sys_UpdateWindows (W_CAMERA|W_XY_OVERLAY|W_Z_OVERLAY);
		break;
	case 'C':
		camera.origin[2] -= SPEED_MOVE;
		Sys_UpdateWindows (W_CAMERA|W_XY_OVERLAY|W_Z_OVERLAY);
		break;
	case 'A':
		camera.angles[0] += SPEED_TURN;
		if (camera.angles[0] > 85)
			camera.angles[0] = 85;
		Sys_UpdateWindows (W_CAMERA|W_XY_OVERLAY);
		break;
	case 'Z':
		camera.angles[0] -= SPEED_TURN;
		if (camera.angles[0] < -85)
			camera.angles[0] = -85;
		Sys_UpdateWindows (W_CAMERA|W_XY_OVERLAY);
		break;
	case VK_COMMA:
		VectorMA (camera.origin, -SPEED_MOVE, camera.right, camera.origin);
		Sys_UpdateWindows (W_CAMERA|W_XY_OVERLAY);
		break;
	case VK_PERIOD:
		VectorMA (camera.origin, SPEED_MOVE, camera.right, camera.origin);
		Sys_UpdateWindows (W_CAMERA|W_XY_OVERLAY);
		break;

	case '0':
		g_qeglobals.d_showgrid = !g_qeglobals.d_showgrid;
		PostMessage( g_qeglobals.d_hwndXY, WM_PAINT, 0, 0 );
		break;
	case '1':
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_GRID_1, 0);
		break;
	case '2':
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_GRID_2, 0);
		break;
	case '3':
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_GRID_4, 0);
		break;
	case '4':
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_GRID_8, 0);
		break;
	case '5':
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_GRID_16, 0);
		break;
	case '6':
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_GRID_32, 0);
		break;
	case '7':
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_GRID_64, 0);
		break;

	case 'E':
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_SELECTION_DRAGEDGES, 0);
		break;
	case 'V':
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_SELECTION_DRAGVERTECIES, 0);
		break;

	case 'N':
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_VIEW_ENTITY, 0);
		break;
	case 'O':
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_VIEW_CONSOLE, 0);
		break;
	case 'T':
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_VIEW_TEXTURE, 0);
		break;
	case 'S':
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_TEXTURES_INSPECTOR, 0);
		break;

	case ' ':
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_SELECTION_CLONE, 0);
		break;

	case VK_BACK:
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_SELECTION_DELETE, 0);
		break;
	case VK_ESCAPE:
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_SELECTION_DESELECT, 0);
		break;
	case VK_END:
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_VIEW_CENTER, 0);
		break;

	case VK_DELETE:
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_VIEW_ZOOMIN, 0);
		break;
	case VK_INSERT:
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_VIEW_ZOOMOUT, 0);
		break;

	case VK_NEXT:
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_VIEW_DOWNFLOOR, 0);
		break;
	case VK_PRIOR:
		PostMessage (g_qeglobals.d_hwndMain, WM_COMMAND, ID_VIEW_UPFLOOR, 0);
		break;

	default:
		return false;

	}

	return true;
}

/*
===============
ConnectEntities

Sets target / targetname on the two entities selected
from the first selected to the secon
===============
*/
void ConnectEntities (void)
{
	entity_t	*e1, *e2, *e;
	char		*target, *tn;
	int			maxtarg, targetnum;
	char		newtarg[32];

	if (g_qeglobals.d_select_count != 2)
	{
		Sys_Status ("Must have two brushes selected.", 0);
		Sys_Beep ();
		return;
	}

	e1 = g_qeglobals.d_select_order[0]->owner;
	e2 = g_qeglobals.d_select_order[1]->owner;

	if (e1 == world_entity || e2 == world_entity)
	{
		Sys_Status ("Can't connect to the world.", 0);
		Sys_Beep ();
		return;
	}

	if (e1 == e2)
	{
		Sys_Status ("Brushes are from same entity.", 0);
		Sys_Beep ();
		return;
	}

	target = ValueForKey (e1, "target");
	if (target && target[0])
		strcpy (newtarg, target);
	else
	{
		target = ValueForKey (e2, "targetname");
		if (target && target[0])
			strcpy (newtarg, target);
		else
		{
			// make a unique target value
			maxtarg = 0;
			for (e=entities.next ; e != &entities ; e=e->next)
			{
				tn = ValueForKey (e, "targetname");
				if (tn && tn[0])
				{
					targetnum = atoi(tn+1);
					if (targetnum > maxtarg)
						maxtarg = targetnum;
				}
			}
			sprintf (newtarg, "t%i", maxtarg+1);
		}
	}

	SetKeyValue (e1, "target", newtarg);
	SetKeyValue (e2, "targetname", newtarg);
	Sys_UpdateWindows (W_XY | W_CAMERA);

	Select_Deselect();
	Select_Brush (g_qeglobals.d_select_order[1]);
}

qboolean QE_SingleBrush (void)
{
	if ( (selected_brushes.next == &selected_brushes)
		|| (selected_brushes.next->next != &selected_brushes) )
	{
		Sys_Printf ("Error: you must have a single brush selected\n");
		return false;
	}
	if (selected_brushes.next->owner->eclass->fixedsize)
	{
		Sys_Printf ("Error: you cannot manipulate fixed size entities\n");
		return false;
	}

	return true;
}

void QE_Init (void)
{
	/*
	** initialize variables
	*/
	g_qeglobals.d_gridsize = 8;
	g_qeglobals.d_showgrid = true;

	/*
	** other stuff
	*/
	Texture_Init ();
	Cam_Init ();
	XY_Init ();
	Z_Init ();
}

void QE_ConvertDOSToUnixName( char *dst, const char *src )
{
	while ( *src )
	{
		if ( *src == '\\' )
			*dst = '/';
		else
			*dst = *src;
		dst++; src++;
	}
	*dst = 0;
}

int g_numbrushes, g_numentities;

void QE_CountBrushesAndUpdateStatusBar( void )
{
	static int      s_lastbrushcount, s_lastentitycount;
	static qboolean s_didonce;

	entity_t   *e;
	brush_t	   *b, *next;

	g_numbrushes = 0;
	g_numentities = 0;

	if ( active_brushes.next != NULL )
	{
		for ( b = active_brushes.next ; b != NULL && b != &active_brushes ; b=next)
		{
			next = b->next;
			if (b->brush_faces )
			{
				if ( !b->owner->eclass->fixedsize)
					g_numbrushes++;
				else
					g_numentities++;
			}
		}
	}

	if ( entities.next != NULL )
	{
		for ( e = entities.next ; e != &entities && g_numentities != MAX_MAP_ENTITIES ; e = e->next)
		{
			g_numentities++;
		}
	}

	if ( ( ( g_numbrushes != s_lastbrushcount ) || ( g_numentities != s_lastentitycount ) ) || ( !s_didonce ) )
	{
		Sys_UpdateStatusBar();

		s_lastbrushcount = g_numbrushes;
		s_lastentitycount = g_numentities;
		s_didonce = true;
	}
}

