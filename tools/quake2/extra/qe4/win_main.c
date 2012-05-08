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
#include <process.h>
#include "mru.h"
#include "entityw.h"

static HWND      s_hwndToolbar;

BOOL SaveRegistryInfo(const char *pszName, void *pvBuf, long lSize);
BOOL LoadRegistryInfo(const char *pszName, void *pvBuf, long *plSize);

static HWND CreateMyStatusWindow(HINSTANCE hInst);
static HWND CreateToolBar(HINSTANCE hinst);

extern int WXY_Print( void );

/*
==============================================================================

  MENU

==============================================================================
*/

void OpenDialog (void);
void SaveAsDialog (void);
qboolean ConfirmModified (void);
void  Select_Ungroup (void);

void QE_ExpandBspString (char *bspaction, char *out, char *mapname)
{
	char	*in;
	char	src[1024];
	char	rsh[1024];
	char	base[256];

	ExtractFileName (mapname, base);
	sprintf (src, "%s/maps/%s", ValueForKey(g_qeglobals.d_project_entity, "remotebasepath"), base);
	strcpy (rsh, ValueForKey(g_qeglobals.d_project_entity, "rshcmd"));

	in = ValueForKey( g_qeglobals.d_project_entity, bspaction );
	while (*in)
	{
		if (in[0] == '!')
		{
			strcpy (out, rsh);
			out += strlen(rsh);
			in++;
			continue;
		}
		if (in[0] == '$')
		{
			strcpy (out, src);
			out += strlen(src);
			in++;
			continue;
		}
		if (in[0] == '@')
		{
			*out++ = '"';
			in++;
			continue;
		}
		*out++ = *in++;
	}
	*out = 0;
}



void RunBsp (char *command)
{
	char	sys[1024];
	char	batpath[1024];
	char	outputpath[1024];
	char	temppath[512];
	char	name[1024];
	FILE	*hFile;
	BOOL	ret;
	PROCESS_INFORMATION ProcessInformation;
	STARTUPINFO	startupinfo;

	SetInspectorMode (W_CONSOLE);

	if (bsp_process)
	{
		Sys_Printf ("BSP is still going...\n");
		return;
	}

	GetTempPath(512, temppath);
	sprintf (outputpath, "%sjunk.txt", temppath);

	strcpy (name, currentmap);
	if (region_active)
	{
		Map_SaveFile (name, false);
		StripExtension (name);
		strcat (name, ".reg");
	}

	Map_SaveFile (name, region_active);


	QE_ExpandBspString (command, sys, name);

	Sys_ClearPrintf ();
	Sys_Printf ("======================================\nRunning bsp command...\n");
	Sys_Printf ("\n%s\n", sys);

	//
	// write qe3bsp.bat
	//
	sprintf (batpath, "%sqe3bsp.bat", temppath);
	hFile = fopen(batpath, "w");
	if (!hFile)
		Error ("Can't write to %s", batpath);
	fprintf (hFile, sys);
	fclose (hFile);

	//
	// write qe3bsp2.bat
	//
	sprintf (batpath, "%sqe3bsp2.bat", temppath);
	hFile = fopen(batpath, "w");
	if (!hFile)
		Error ("Can't write to %s", batpath);
	fprintf (hFile, "%sqe3bsp.bat > %s", temppath, outputpath);
	fclose (hFile);

	Pointfile_Delete ();

	GetStartupInfo (&startupinfo);

	ret = CreateProcess(
    batpath,		// pointer to name of executable module
    NULL,			// pointer to command line string
    NULL,			// pointer to process security attributes
    NULL,			// pointer to thread security attributes
    FALSE,			// handle inheritance flag
    0 /*DETACHED_PROCESS*/,		// creation flags
    NULL,			// pointer to new environment block
    NULL,			// pointer to current directory name
    &startupinfo,	// pointer to STARTUPINFO
    &ProcessInformation 	// pointer to PROCESS_INFORMATION
   );

	if (!ret)
		Error ("CreateProcess failed");

	bsp_process = ProcessInformation.hProcess;

	Sleep (100);	// give the new process a chance to open it's window

	BringWindowToTop( g_qeglobals.d_hwndMain );	// pop us back on top
	SetFocus (g_qeglobals.d_hwndCamera);
}

/*
=============
DoColor

=============
*/
qboolean DoColor(int iIndex)
{
	CHOOSECOLOR	cc;
	static COLORREF	custom[16];

	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = g_qeglobals.d_hwndMain;
	cc.hInstance = g_qeglobals.d_hInstance;
	cc.rgbResult =
		(int)(g_qeglobals.d_savedinfo.colors[iIndex][0]*255) +
		(((int)(g_qeglobals.d_savedinfo.colors[iIndex][1]*255))<<8) +
		(((int)(g_qeglobals.d_savedinfo.colors[iIndex][2]*255))<<16);
    cc.lpCustColors = custom;
    cc.Flags = CC_FULLOPEN|CC_RGBINIT;
    //cc.lCustData;
    //cc.lpfnHook;
    //cc.lpTemplateName

	if (!ChooseColor(&cc))
		return false;

	g_qeglobals.d_savedinfo.colors[iIndex][0] = (cc.rgbResult&255)/255.0;
	g_qeglobals.d_savedinfo.colors[iIndex][1] = ((cc.rgbResult>>8)&255)/255.0;
	g_qeglobals.d_savedinfo.colors[iIndex][2] = ((cc.rgbResult>>16)&255)/255.0;

	/*
	** scale colors so that at least one component is at 1.0F
	** if this is meant to select an entity color
	*/
	if ( iIndex == COLOR_ENTITY )
	{
		float largest = 0.0F;

		if ( g_qeglobals.d_savedinfo.colors[iIndex][0] > largest )
			largest = g_qeglobals.d_savedinfo.colors[iIndex][0];
		if ( g_qeglobals.d_savedinfo.colors[iIndex][1] > largest )
			largest = g_qeglobals.d_savedinfo.colors[iIndex][1];
		if ( g_qeglobals.d_savedinfo.colors[iIndex][2] > largest )
			largest = g_qeglobals.d_savedinfo.colors[iIndex][2];

		if ( largest == 0.0F )
		{
			g_qeglobals.d_savedinfo.colors[iIndex][0] = 1.0F;
			g_qeglobals.d_savedinfo.colors[iIndex][1] = 1.0F;
			g_qeglobals.d_savedinfo.colors[iIndex][2] = 1.0F;
		}
		else
		{
			float scaler = 1.0F / largest;

			g_qeglobals.d_savedinfo.colors[iIndex][0] *= scaler;
			g_qeglobals.d_savedinfo.colors[iIndex][1] *= scaler;
			g_qeglobals.d_savedinfo.colors[iIndex][2] *= scaler;
		}
	}

	Sys_UpdateWindows (W_ALL);

	return true;
}


/* Copied from MSDN */

BOOL DoMru(HWND hWnd,WORD wId)
{
	char szFileName[128];
	OFSTRUCT of;
	BOOL fExist;

	GetMenuItem(g_qeglobals.d_lpMruMenu, wId, TRUE, szFileName, sizeof(szFileName));

	// Test if the file exists.

	fExist = OpenFile(szFileName ,&of,OF_EXIST) != HFILE_ERROR;

	if (fExist) {

		// Place the file on the top of MRU.
		AddNewItem(g_qeglobals.d_lpMruMenu,(LPSTR)szFileName);

		// Now perform opening this file !!!
		Map_LoadFile (szFileName);
	}
	else
		// Remove the file on MRU.
		DelMenuItem(g_qeglobals.d_lpMruMenu,wId,TRUE);

	// Refresh the File menu.
	PlaceMenuMRUItem(g_qeglobals.d_lpMruMenu,GetSubMenu(GetMenu(hWnd),0),
			ID_FILE_EXIT);

	return fExist;
}


/* handle all WM_COMMAND messages here */
LONG WINAPI CommandHandler (
    HWND    hWnd,
    WPARAM  wParam,
    LPARAM  lParam)
{
	HMENU hMenu;

    switch (LOWORD(wParam))
    {
//
// file menu
//
    	case ID_FILE_EXIT:
            /* exit application */
			if (!ConfirmModified())
				return TRUE;

            PostMessage (hWnd, WM_CLOSE, 0, 0L);
			break;

		case ID_FILE_OPEN:
			if (!ConfirmModified())
				return TRUE;
			OpenDialog ();
			break;

		case ID_FILE_NEW:
			if (!ConfirmModified())
				return TRUE;
			Map_New ();
			break;
		case ID_FILE_SAVE:
			if (!strcmp(currentmap, "unnamed.map"))
				SaveAsDialog ();
			else
				Map_SaveFile (currentmap, false);	// ignore region
			break;
		case ID_FILE_SAVEAS:
			SaveAsDialog ();
			break;

		case ID_FILE_LOADPROJECT:
			if (!ConfirmModified())
				return TRUE;
			ProjectDialog ();
			break;

		case ID_FILE_POINTFILE:
			if (g_qeglobals.d_pointfile_display_list)
				Pointfile_Clear ();
			else
				Pointfile_Check ();
			break;

//
// view menu
//
		case ID_VIEW_ENTITY:
			SetInspectorMode(W_ENTITY);
			break;
		case ID_VIEW_CONSOLE:
			SetInspectorMode(W_CONSOLE);
			break;
		case ID_VIEW_TEXTURE:
			SetInspectorMode(W_TEXTURE);
			break;

		case ID_VIEW_100:
			g_qeglobals.d_xy.scale = 1;
			Sys_UpdateWindows (W_XY|W_XY_OVERLAY);
			break;
		case ID_VIEW_ZOOMIN:
			g_qeglobals.d_xy.scale *= 5.0/4;
			if (g_qeglobals.d_xy.scale > 16)
				g_qeglobals.d_xy.scale = 16;
			Sys_UpdateWindows (W_XY|W_XY_OVERLAY);
			break;
		case ID_VIEW_ZOOMOUT:
			g_qeglobals.d_xy.scale *= 4.0/5;
			if (g_qeglobals.d_xy.scale < 0.1)
				g_qeglobals.d_xy.scale = 0.1;
			Sys_UpdateWindows (W_XY|W_XY_OVERLAY);
			break;

		case ID_VIEW_Z100:
			z.scale = 1;
			Sys_UpdateWindows (W_Z|W_Z_OVERLAY);
			break;
		case ID_VIEW_ZZOOMIN:
			z.scale *= 5.0/4;
			if (z.scale > 4)
				z.scale = 4;
			Sys_UpdateWindows (W_Z|W_Z_OVERLAY);
			break;
		case ID_VIEW_ZZOOMOUT:
			z.scale *= 4.0/5;
			if (z.scale < 0.125)
				z.scale = 0.125;
			Sys_UpdateWindows (W_Z|W_Z_OVERLAY);
			break;

		case ID_VIEW_CENTER:
			camera.angles[ROLL] = camera.angles[PITCH] = 0;
			camera.angles[YAW] = 22.5 *
				floor( (camera.angles[YAW]+11)/22.5 );
			Sys_UpdateWindows (W_CAMERA|W_XY_OVERLAY);
			break;

		case ID_VIEW_UPFLOOR:
			Cam_ChangeFloor (true);
			break;
		case ID_VIEW_DOWNFLOOR:
			Cam_ChangeFloor (false);
			break;

		case ID_VIEW_SHOWNAMES:
			g_qeglobals.d_savedinfo.show_names = !g_qeglobals.d_savedinfo.show_names;
			CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWNAMES, MF_BYCOMMAND | (g_qeglobals.d_savedinfo.show_names ? MF_CHECKED : MF_UNCHECKED)  );
			Map_BuildBrushData();
			Sys_UpdateWindows (W_XY);
			break;

		case ID_VIEW_SHOWCOORDINATES:
			g_qeglobals.d_savedinfo.show_coordinates ^= 1;
			CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWCOORDINATES, MF_BYCOMMAND | (g_qeglobals.d_savedinfo.show_coordinates ? MF_CHECKED : MF_UNCHECKED)  );
			Sys_UpdateWindows (W_XY);
			break;

		case ID_VIEW_SHOWBLOCKS:
			g_qeglobals.show_blocks ^= 1;
			CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWBLOCKS, MF_BYCOMMAND | (g_qeglobals.show_blocks ? MF_CHECKED : MF_UNCHECKED)  );
			Sys_UpdateWindows (W_XY);
			break;

		case ID_VIEW_SHOWLIGHTS:
			if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_LIGHTS ) & EXCLUDE_LIGHTS )
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWLIGHTS, MF_BYCOMMAND | MF_UNCHECKED );
			else
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWLIGHTS, MF_BYCOMMAND | MF_CHECKED );
			Sys_UpdateWindows (W_XY|W_CAMERA);
			break;

		case ID_VIEW_SHOWPATH:
			if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_PATHS ) & EXCLUDE_PATHS )
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWPATH, MF_BYCOMMAND | MF_UNCHECKED );
			else
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWPATH, MF_BYCOMMAND | MF_CHECKED );
			Sys_UpdateWindows (W_XY|W_CAMERA);
			break;

		case ID_VIEW_SHOWENT:
			if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_ENT ) & EXCLUDE_ENT )
				CheckMenuItem( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWENT, MF_BYCOMMAND | MF_UNCHECKED);
			else
				CheckMenuItem( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWENT, MF_BYCOMMAND | MF_CHECKED);
			Sys_UpdateWindows (W_XY|W_CAMERA);
			break;

		case ID_VIEW_SHOWWATER:
			if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_WATER ) & EXCLUDE_WATER )
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWWATER, MF_BYCOMMAND | MF_UNCHECKED );
			else
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWWATER, MF_BYCOMMAND | MF_CHECKED );
			Sys_UpdateWindows (W_XY|W_CAMERA);
			break;

		case ID_VIEW_SHOWCLIP:
			if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_CLIP ) & EXCLUDE_CLIP )
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWCLIP, MF_BYCOMMAND | MF_UNCHECKED );
			else
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWCLIP, MF_BYCOMMAND | MF_CHECKED );
			Sys_UpdateWindows (W_XY|W_CAMERA);
			break;

		case ID_VIEW_SHOWDETAIL:
			if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_DETAIL ) & EXCLUDE_DETAIL )
			{
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWDETAIL, MF_BYCOMMAND | MF_UNCHECKED );
				SetWindowText (g_qeglobals.d_hwndCamera, "Camera View (DETAIL EXCLUDED)");
			}
			else
			{
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWDETAIL, MF_BYCOMMAND | MF_CHECKED );
				SetWindowText (g_qeglobals.d_hwndCamera, "Camera View");
			}
			Sys_UpdateWindows (W_XY|W_CAMERA);
			break;

		case ID_VIEW_SHOWWORLD:
			if ( ( g_qeglobals.d_savedinfo.exclude ^= EXCLUDE_WORLD ) & EXCLUDE_WORLD )
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWWORLD, MF_BYCOMMAND | MF_UNCHECKED );
			else
				CheckMenuItem ( GetMenu(g_qeglobals.d_hwndMain), ID_VIEW_SHOWWORLD, MF_BYCOMMAND | MF_CHECKED );
			Sys_UpdateWindows (W_XY|W_CAMERA);
			break;


//
// grid menu
//
		case ID_GRID_1:
		case ID_GRID_2:
		case ID_GRID_4:
		case ID_GRID_8:
		case ID_GRID_16:
		case ID_GRID_32:
		case ID_GRID_64:
		{
			hMenu = GetMenu(hWnd);

			CheckMenuItem(hMenu, ID_GRID_1, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_GRID_2, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_GRID_4, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_GRID_8, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_GRID_16, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_GRID_32, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(hMenu, ID_GRID_64, MF_BYCOMMAND | MF_UNCHECKED);

			switch (LOWORD(wParam))
			{
				case ID_GRID_1: g_qeglobals.d_gridsize = 0; break;
				case ID_GRID_2: g_qeglobals.d_gridsize = 1; break;
				case ID_GRID_4: g_qeglobals.d_gridsize = 2; break;
				case ID_GRID_8: g_qeglobals.d_gridsize = 3; break;
				case ID_GRID_16: g_qeglobals.d_gridsize = 4; break;
				case ID_GRID_32: g_qeglobals.d_gridsize = 5; break;
				case ID_GRID_64: g_qeglobals.d_gridsize = 6; break;
			}
			g_qeglobals.d_gridsize = 1 << g_qeglobals.d_gridsize;

			CheckMenuItem(hMenu, LOWORD(wParam), MF_BYCOMMAND | MF_CHECKED);
			Sys_UpdateWindows (W_XY|W_Z);
			break;
		}

//
// texture menu
//
		case ID_VIEW_NEAREST:
		case ID_VIEW_NEARESTMIPMAP:
		case ID_VIEW_LINEAR:
		case ID_VIEW_BILINEAR:
		case ID_VIEW_BILINEARMIPMAP:
		case ID_VIEW_TRILINEAR:
		case ID_TEXTURES_WIREFRAME:
		case ID_TEXTURES_FLATSHADE:
			Texture_SetMode (LOWORD(wParam));
			break;

		case ID_TEXTURES_SHOWINUSE:
			Sys_BeginWait ();
			Texture_ShowInuse ();
			SetInspectorMode(W_TEXTURE);
			break;

		case ID_TEXTURES_INSPECTOR:
			DoSurface ();
			break;

		case CMD_TEXTUREWAD:
		case CMD_TEXTUREWAD+1:
		case CMD_TEXTUREWAD+2:
		case CMD_TEXTUREWAD+3:
		case CMD_TEXTUREWAD+4:
		case CMD_TEXTUREWAD+5:
		case CMD_TEXTUREWAD+6:
		case CMD_TEXTUREWAD+7:
		case CMD_TEXTUREWAD+8:
		case CMD_TEXTUREWAD+9:
		case CMD_TEXTUREWAD+10:
		case CMD_TEXTUREWAD+11:
		case CMD_TEXTUREWAD+12:
		case CMD_TEXTUREWAD+13:
		case CMD_TEXTUREWAD+14:
		case CMD_TEXTUREWAD+15:
		case CMD_TEXTUREWAD+16:
		case CMD_TEXTUREWAD+17:
		case CMD_TEXTUREWAD+18:
		case CMD_TEXTUREWAD+19:
		case CMD_TEXTUREWAD+20:
		case CMD_TEXTUREWAD+21:
		case CMD_TEXTUREWAD+22:
		case CMD_TEXTUREWAD+23:
		case CMD_TEXTUREWAD+24:
		case CMD_TEXTUREWAD+25:
		case CMD_TEXTUREWAD+26:
		case CMD_TEXTUREWAD+27:
		case CMD_TEXTUREWAD+28:
		case CMD_TEXTUREWAD+29:
		case CMD_TEXTUREWAD+30:
		case CMD_TEXTUREWAD+31:
			Sys_BeginWait ();
			Texture_ShowDirectory (LOWORD(wParam));
			SetInspectorMode(W_TEXTURE);
			break;

//
// bsp menu
//
		case CMD_BSPCOMMAND:
		case CMD_BSPCOMMAND+1:
		case CMD_BSPCOMMAND+2:
		case CMD_BSPCOMMAND+3:
		case CMD_BSPCOMMAND+4:
		case CMD_BSPCOMMAND+5:
		case CMD_BSPCOMMAND+6:
		case CMD_BSPCOMMAND+7:
		case CMD_BSPCOMMAND+8:
		case CMD_BSPCOMMAND+9:
		case CMD_BSPCOMMAND+10:
		case CMD_BSPCOMMAND+11:
		case CMD_BSPCOMMAND+12:
		case CMD_BSPCOMMAND+13:
		case CMD_BSPCOMMAND+14:
		case CMD_BSPCOMMAND+15:
		case CMD_BSPCOMMAND+16:
		case CMD_BSPCOMMAND+17:
		case CMD_BSPCOMMAND+18:
		case CMD_BSPCOMMAND+19:
		case CMD_BSPCOMMAND+20:
		case CMD_BSPCOMMAND+21:
		case CMD_BSPCOMMAND+22:
		case CMD_BSPCOMMAND+23:
		case CMD_BSPCOMMAND+24:
		case CMD_BSPCOMMAND+25:
		case CMD_BSPCOMMAND+26:
		case CMD_BSPCOMMAND+27:
		case CMD_BSPCOMMAND+28:
		case CMD_BSPCOMMAND+29:
		case CMD_BSPCOMMAND+30:
		case CMD_BSPCOMMAND+31:
			{
				extern	char	*bsp_commands[256];

				RunBsp (bsp_commands[LOWORD(wParam-CMD_BSPCOMMAND)]);
			}
			break;

//
// misc menu
//
		case ID_MISC_BENCHMARK:
			SendMessage ( g_qeglobals.d_hwndCamera,
			WM_USER+267,	0, 0);
			break;

		case ID_TEXTUREBK:
			DoColor(COLOR_TEXTUREBACK);
			Sys_UpdateWindows (W_ALL);
			break;

		case ID_MISC_SELECTENTITYCOLOR:
			{
				extern int inspector_mode;

				if ( ( inspector_mode == W_ENTITY ) && DoColor(COLOR_ENTITY) == true )
				{
					extern void AddProp( void );

					char buffer[100];

					sprintf( buffer, "%f %f %f", g_qeglobals.d_savedinfo.colors[COLOR_ENTITY][0],
						g_qeglobals.d_savedinfo.colors[COLOR_ENTITY][1],
						g_qeglobals.d_savedinfo.colors[COLOR_ENTITY][2] );

					SetWindowText( hwndEnt[EntValueField], buffer );
					SetWindowText( hwndEnt[EntKeyField], "_color" );
					AddProp();
				}
				Sys_UpdateWindows( W_ALL );
			}
			break;

		case ID_MISC_PRINTXY:
			WXY_Print();
			break;

		case ID_COLORS_XYBK:
			DoColor(COLOR_GRIDBACK);
			Sys_UpdateWindows (W_ALL);
			break;

		case ID_COLORS_MAJOR:
			DoColor(COLOR_GRIDMAJOR);
			Sys_UpdateWindows (W_ALL);
			break;

		case ID_COLORS_MINOR:
			DoColor(COLOR_GRIDMINOR);
			Sys_UpdateWindows (W_ALL);
			break;

		case ID_MISC_GAMMA:
			DoGamma();
			break;

		case ID_MISC_FINDBRUSH:
			DoFind();
			break;

		case ID_MISC_NEXTLEAKSPOT:
			Pointfile_Next();
			break;
		case ID_MISC_PREVIOUSLEAKSPOT:
			Pointfile_Prev();
			break;

//
// brush menu
//
		case ID_BRUSH_3SIDED:
			Brush_MakeSided (3);
			break;
		case ID_BRUSH_4SIDED:
			Brush_MakeSided (4);
			break;
		case ID_BRUSH_5SIDED:
			Brush_MakeSided (5);
			break;
		case ID_BRUSH_6SIDED:
			Brush_MakeSided (6);
			break;
		case ID_BRUSH_7SIDED:
			Brush_MakeSided (7);
			break;
		case ID_BRUSH_8SIDED:
			Brush_MakeSided (8);
			break;
		case ID_BRUSH_9SIDED:
			Brush_MakeSided (9);
			break;
		case ID_BRUSH_ARBITRARYSIDED:
			DoSides ();
			break;

//
// select menu
//
		case ID_BRUSH_FLIPX:
			Select_FlipAxis (0);
			break;
		case ID_BRUSH_FLIPY:
			Select_FlipAxis (1);
			break;
		case ID_BRUSH_FLIPZ:
			Select_FlipAxis (2);
			break;
		case ID_BRUSH_ROTATEX:
			Select_RotateAxis (0, 90);
			break;
		case ID_BRUSH_ROTATEY:
			Select_RotateAxis (1, 90);
			break;
		case ID_BRUSH_ROTATEZ:
			Select_RotateAxis (2, 90);
			break;

		case ID_SELECTION_ARBITRARYROTATION:
			DoRotate ();
			break;

		case ID_SELECTION_UNGROUPENTITY:
			Select_Ungroup ();
			break;

		case ID_SELECTION_CONNECT:
			ConnectEntities ();
			break;

		case ID_SELECTION_DRAGVERTECIES:
			if (g_qeglobals.d_select_mode == sel_vertex)
			{
				g_qeglobals.d_select_mode = sel_brush;
				Sys_UpdateWindows (W_ALL);
			}
			else
			{
				SetupVertexSelection ();
				if (g_qeglobals.d_numpoints)
					g_qeglobals.d_select_mode = sel_vertex;
			}
			break;
		case ID_SELECTION_DRAGEDGES:
			if (g_qeglobals.d_select_mode == sel_edge)
			{
				g_qeglobals.d_select_mode = sel_brush;
				Sys_UpdateWindows (W_ALL);
			}
			else
			{
				SetupVertexSelection ();
				if (g_qeglobals.d_numpoints)
					g_qeglobals.d_select_mode = sel_edge;
			}
			break;

		case ID_SELECTION_SELECTPARTIALTALL:
			Select_PartialTall ();
			break;
		case ID_SELECTION_SELECTCOMPLETETALL:
			Select_CompleteTall ();
			break;
		case ID_SELECTION_SELECTTOUCHING:
			Select_Touching ();
			break;
		case ID_SELECTION_SELECTINSIDE:
			Select_Inside ();
			break;
		case ID_SELECTION_CSGSUBTRACT:
			CSG_Subtract ();
			break;
		case ID_SELECTION_MAKEHOLLOW:
			CSG_MakeHollow ();
			break;

		case ID_SELECTION_CLONE:
			Select_Clone ();
			break;
		case ID_SELECTION_DELETE:
			Select_Delete ();
			break;
		case ID_SELECTION_DESELECT:
			Select_Deselect ();
			break;

		case ID_SELECTION_MAKE_DETAIL:
			Select_MakeDetail ();
			break;
		case ID_SELECTION_MAKE_STRUCTURAL:
			Select_MakeStructural ();
			break;


//
// region menu
//
		case ID_REGION_OFF:
			Map_RegionOff ();
			break;
		case ID_REGION_SETXY:
			Map_RegionXY ();
			break;
		case ID_REGION_SETTALLBRUSH:
			Map_RegionTallBrush ();
			break;
		case ID_REGION_SETBRUSH:
			Map_RegionBrush ();
			break;
		case ID_REGION_SETSELECTION:
			Map_RegionSelectedBrushes ();
			break;

		case IDMRU+1:
		case IDMRU+2:
		case IDMRU+3:
		case IDMRU+4:
		case IDMRU+5:
		case IDMRU+6:
		case IDMRU+7:
		case IDMRU+8:
		case IDMRU+9:
			DoMru(hWnd,LOWORD(wParam));
			break;

//
// help menu
//

		case ID_HELP_ABOUT:
			DoAbout();
			break;

		default:
            return FALSE;
    }

    return TRUE;
}

/*
============
WMAIN_WndProc
============
*/
LONG WINAPI WMAIN_WndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    LONG    lRet = 1;
    RECT	rect;
	HDC		maindc;

    GetClientRect(hWnd, &rect);

    switch (uMsg)
    {
	case WM_TIMER:
		QE_CountBrushesAndUpdateStatusBar();
		QE_CheckAutoSave();
		return 0;

	case WM_DESTROY:
		SaveMruInReg(g_qeglobals.d_lpMruMenu,"Software\\id\\QuakeEd4\\MRU");
		DeleteMruMenu(g_qeglobals.d_lpMruMenu);
		PostQuitMessage(0);
		KillTimer( hWnd, QE_TIMER0 );
		return 0;

	case WM_CREATE:
        maindc = GetDC(hWnd);
//	    QEW_SetupPixelFormat(maindc, false);
		g_qeglobals.d_lpMruMenu = CreateMruMenuDefault();
		LoadMruInReg(g_qeglobals.d_lpMruMenu,"Software\\id\\QuakeEd4\\MRU");

		// Refresh the File menu.
		PlaceMenuMRUItem(g_qeglobals.d_lpMruMenu,GetSubMenu(GetMenu(hWnd),0),
				ID_FILE_EXIT);

		return 0;

	case WM_SIZE:
		// resize the status window
		MoveWindow( g_qeglobals.d_hwndStatus, -100, 100, 10, 10, true);
		return 0;

	case WM_KEYDOWN:
		return QE_KeyDown (wParam);

   	case WM_CLOSE:
        /* call destroy window to cleanup and go away */
		SaveWindowState(g_qeglobals.d_hwndXY, "xywindow");
		SaveWindowState(g_qeglobals.d_hwndCamera, "camerawindow");
		SaveWindowState(g_qeglobals.d_hwndZ, "zwindow");
		SaveWindowState(g_qeglobals.d_hwndEntity, "EntityWindow");
		SaveWindowState(g_qeglobals.d_hwndMain, "mainwindow");

		// FIXME: is this right?
		SaveRegistryInfo("SavedInfo", &g_qeglobals.d_savedinfo, sizeof(g_qeglobals.d_savedinfo));
        DestroyWindow (hWnd);
		return 0;

    case WM_COMMAND:
		return CommandHandler (hWnd, wParam, lParam);
		return 0;
    }

    return DefWindowProc (hWnd, uMsg, wParam, lParam);
}




/*
==============
Main_Create
==============
*/
void Main_Create (HINSTANCE hInstance)
{
    WNDCLASS   wc;
	int		   i;
	HMENU      hMenu;

    /* Register the camera class */
	memset (&wc, 0, sizeof(wc));

    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)WMAIN_WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
    wc.lpszClassName = "QUAKE_MAIN";

    if (!RegisterClass (&wc) )
        Error ("WCam_Register: failed");


	g_qeglobals.d_hwndMain = CreateWindow ("QUAKE_MAIN" ,
		"QuakeEd 3",
		WS_OVERLAPPEDWINDOW |
		WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN,
		0,0,screen_width,screen_height+GetSystemMetrics(SM_CYSIZE),	// size
		0,
		0,		// no menu
		hInstance,
		NULL);
	if (!g_qeglobals.d_hwndMain)
		Error ("Couldn't create main window");

	/* create a timer so that we can count brushes */
	SetTimer( g_qeglobals.d_hwndMain,
			  QE_TIMER0,
			  1000,
		      NULL );

	LoadWindowState(g_qeglobals.d_hwndMain, "mainwindow");

	s_hwndToolbar = CreateToolBar(hInstance);

	g_qeglobals.d_hwndStatus = CreateMyStatusWindow(hInstance);

	//
	// load misc info from registry
	//
	i = sizeof(g_qeglobals.d_savedinfo);
	LoadRegistryInfo("SavedInfo", &g_qeglobals.d_savedinfo, &i);

	if (g_qeglobals.d_savedinfo.iSize != sizeof(g_qeglobals.d_savedinfo))
	{
		// fill in new defaults

		g_qeglobals.d_savedinfo.iSize = sizeof(g_qeglobals.d_savedinfo);
		g_qeglobals.d_savedinfo.fGamma = 1.0;
		g_qeglobals.d_savedinfo.iTexMenu = ID_VIEW_NEAREST;

		g_qeglobals.d_savedinfo.exclude = 0;
		g_qeglobals.d_savedinfo.show_coordinates = true;
		g_qeglobals.d_savedinfo.show_names       = true;

		for (i=0 ; i<3 ; i++)
		{
			g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][i] = 0.25;
			g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][i] = 1.0;
			g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR][i] = 0.75;
			g_qeglobals.d_savedinfo.colors[COLOR_GRIDMAJOR][i] = 0.5;
			g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][i] = 0.25;
		}
	}

	if ( ( hMenu = GetMenu( g_qeglobals.d_hwndMain ) ) != 0 )
	{
		/*
		** by default all of these are checked because that's how they're defined in the menu editor
		*/
		if ( !g_qeglobals.d_savedinfo.show_names )
			CheckMenuItem( hMenu, ID_VIEW_SHOWNAMES, MF_BYCOMMAND | MF_UNCHECKED );
		if ( !g_qeglobals.d_savedinfo.show_coordinates )
			CheckMenuItem( hMenu, ID_VIEW_SHOWCOORDINATES, MF_BYCOMMAND | MF_UNCHECKED );

		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_LIGHTS )
			CheckMenuItem( hMenu, ID_VIEW_SHOWLIGHTS, MF_BYCOMMAND | MF_UNCHECKED );
		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_ENT )
			CheckMenuItem( hMenu, ID_VIEW_ENTITY, MF_BYCOMMAND | MF_UNCHECKED );
		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_PATHS )
			CheckMenuItem( hMenu, ID_VIEW_SHOWPATH, MF_BYCOMMAND | MF_UNCHECKED );
		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_WATER )
			CheckMenuItem( hMenu, ID_VIEW_SHOWWATER, MF_BYCOMMAND | MF_UNCHECKED );
		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_WORLD )
			CheckMenuItem( hMenu, ID_VIEW_SHOWWORLD, MF_BYCOMMAND | MF_UNCHECKED );
		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_CLIP )
			CheckMenuItem( hMenu, ID_VIEW_SHOWCLIP, MF_BYCOMMAND | MF_UNCHECKED );
	}

	ShowWindow (g_qeglobals.d_hwndMain, SW_SHOWDEFAULT);
}


/*
=============================================================

REGISTRY INFO

=============================================================
*/

BOOL SaveRegistryInfo(const char *pszName, void *pvBuf, long lSize)
{
	LONG lres;
	DWORD dwDisp;
	HKEY  hKeyId;

	lres = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\id\\QuakeEd4", 0, NULL,
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyId, &dwDisp);

	if (lres != ERROR_SUCCESS)
		return FALSE;

	lres = RegSetValueEx(hKeyId, pszName, 0, REG_BINARY, pvBuf, lSize);

	RegCloseKey(hKeyId);

	if (lres != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

BOOL LoadRegistryInfo(const char *pszName, void *pvBuf, long *plSize)
{
	HKEY  hKey;
	long lres, lType, lSize;

	if (plSize == NULL)
		plSize = &lSize;

	lres = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\id\\QuakeEd4", 0, KEY_READ, &hKey);

	if (lres != ERROR_SUCCESS)
		return FALSE;

	lres = RegQueryValueEx(hKey, pszName, NULL, &lType, pvBuf, plSize);

	RegCloseKey(hKey);

	if (lres != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

BOOL SaveWindowState(HWND hWnd, const char *pszName)
{
	RECT rc;

	GetWindowRect(hWnd, &rc);
	if (hWnd != g_qeglobals.d_hwndMain)
		MapWindowPoints(NULL, g_qeglobals.d_hwndMain, (POINT *)&rc, 2);
	return SaveRegistryInfo(pszName, &rc, sizeof(rc));
}


BOOL LoadWindowState(HWND hWnd, const char *pszName)
{
	RECT rc;
	LONG lSize = sizeof(rc);

	if (LoadRegistryInfo(pszName, &rc, &lSize))
	{
		if (rc.left < 0)
			rc.left = 0;
		if (rc.top < 0)
			rc.top = 0;
		if (rc.right < rc.left + 16)
			rc.right = rc.left + 16;
		if (rc.bottom < rc.top + 16)
			rc.bottom = rc.top + 16;

		MoveWindow(hWnd, rc.left, rc.top, rc.right - rc.left,
				rc.bottom - rc.top, FALSE);
		return TRUE;
	}

	return FALSE;
}

/*
===============================================================

  STATUS WINDOW

===============================================================
*/

void Sys_UpdateStatusBar( void )
{
	extern int   g_numbrushes, g_numentities;

	char numbrushbuffer[100]="";

	sprintf( numbrushbuffer, "Brushes: %d Entities: %d", g_numbrushes, g_numentities );

	Sys_Status( numbrushbuffer, 2 );
}

void Sys_Status(const char *psz, int part )
{
	SendMessage(g_qeglobals.d_hwndStatus, SB_SETTEXT, part, (LPARAM)psz);
}

static HWND CreateMyStatusWindow(HINSTANCE hInst)
{
	HWND hWnd;
	int partsize[3] = { 300, 1100, -1 };

	hWnd = CreateWindowEx( WS_EX_TOPMOST, // no extended styles
            STATUSCLASSNAME,                 // status bar
            "",                              // no text
            WS_CHILD | WS_BORDER | WS_VISIBLE,  // styles
            -100, -100, 10, 10,              // x, y, cx, cy
            g_qeglobals.d_hwndMain,          // parent window
            (HMENU)100,                      // window ID
            hInst,                           // instance
            NULL);							 // window data

	SendMessage( hWnd, SB_SETPARTS, 3, ( long ) partsize );

	return hWnd;
}

//==============================================================

#define NUMBUTTONS 15
HWND CreateToolBar(HINSTANCE hinst)
{
    HWND hwndTB;
    TBADDBITMAP tbab;
    TBBUTTON tbb[NUMBUTTONS];

     // Ensure that the common control DLL is loaded.

    InitCommonControls();

    // Create a toolbar that the user can customize and that has a
    // tooltip associated with it.

    hwndTB = CreateWindowEx(0, TOOLBARCLASSNAME, (LPSTR) NULL,
        WS_CHILD | TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE | WS_BORDER,
        0, 0, 0, 0, g_qeglobals.d_hwndMain, (HMENU) IDR_TOOLBAR1, hinst, NULL);

    // Send the TB_BUTTONSTRUCTSIZE message, which is required for
    // backward compatibility.

    SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);

    // Add the bitmap containing button images to the toolbar.

    tbab.hInst = hinst;
    tbab.nID   = IDR_TOOLBAR1;
    SendMessage(hwndTB, TB_ADDBITMAP, (WPARAM)NUMBUTTONS, (WPARAM) &tbab);

    // Fill the TBBUTTON array with button information, and add the
    // buttons to the toolbar.

    tbb[0].iBitmap = 0;
    tbb[0].idCommand = ID_BRUSH_FLIPX;
    tbb[0].fsState = TBSTATE_ENABLED;
    tbb[0].fsStyle = TBSTYLE_BUTTON;
    tbb[0].dwData = 0;
    tbb[0].iString = 0;

    tbb[1].iBitmap = 2;
    tbb[1].idCommand = ID_BRUSH_FLIPY;
    tbb[1].fsState = TBSTATE_ENABLED;
    tbb[1].fsStyle = TBSTYLE_BUTTON;
    tbb[1].dwData = 0;
    tbb[1].iString = 0;

    tbb[2].iBitmap = 4;
    tbb[2].idCommand = ID_BRUSH_FLIPZ;
    tbb[2].fsState = TBSTATE_ENABLED;
    tbb[2].fsStyle = TBSTYLE_BUTTON;
    tbb[2].dwData = 0;
    tbb[2].iString = 0;

    tbb[3].iBitmap = 1;
    tbb[3].idCommand = ID_BRUSH_ROTATEX;
    tbb[3].fsState = TBSTATE_ENABLED;
    tbb[3].fsStyle = TBSTYLE_BUTTON;
    tbb[3].dwData = 0;
    tbb[3].iString = 0;

    tbb[4].iBitmap = 3;
    tbb[4].idCommand = ID_BRUSH_ROTATEY;
    tbb[4].fsState = TBSTATE_ENABLED;
    tbb[4].fsStyle = TBSTYLE_BUTTON;
    tbb[4].dwData = 0;
    tbb[4].iString = 0;

    tbb[5].iBitmap = 5;
    tbb[5].idCommand = ID_BRUSH_ROTATEZ;
    tbb[5].fsState = TBSTATE_ENABLED;
    tbb[5].fsStyle = TBSTYLE_BUTTON;
    tbb[5].dwData = 0;
    tbb[5].iString = 0;

    tbb[6].iBitmap = 6;
    tbb[6].idCommand = ID_SELECTION_SELECTCOMPLETETALL;
    tbb[6].fsState = TBSTATE_ENABLED;
    tbb[6].fsStyle = TBSTYLE_BUTTON;
    tbb[6].dwData = 0;
    tbb[6].iString = 0;

    tbb[7].iBitmap = 7;
    tbb[7].idCommand = ID_SELECTION_SELECTTOUCHING;
    tbb[7].fsState = TBSTATE_ENABLED;
    tbb[7].fsStyle = TBSTYLE_BUTTON;
    tbb[7].dwData = 0;
    tbb[7].iString = 0;

    tbb[8].iBitmap = 8;
    tbb[8].idCommand = ID_SELECTION_SELECTPARTIALTALL;
    tbb[8].fsState = TBSTATE_ENABLED;
    tbb[8].fsStyle = TBSTYLE_BUTTON;
    tbb[8].dwData = 0;
    tbb[8].iString = 0;


    tbb[9].iBitmap = 9;
    tbb[9].idCommand = ID_SELECTION_SELECTINSIDE;
    tbb[9].fsState = TBSTATE_ENABLED;
    tbb[9].fsStyle = TBSTYLE_BUTTON;
    tbb[9].dwData = 0;
    tbb[9].iString = 0;

    tbb[10].iBitmap = 10;
    tbb[10].idCommand = ID_SELECTION_CSGSUBTRACT;
    tbb[10].fsState = TBSTATE_ENABLED;
    tbb[10].fsStyle = TBSTYLE_BUTTON;
    tbb[10].dwData = 0;
    tbb[10].iString = 0;


    tbb[11].iBitmap = 11;
    tbb[11].idCommand = ID_SELECTION_MAKEHOLLOW;
    tbb[11].fsState = TBSTATE_ENABLED;
    tbb[11].fsStyle = TBSTYLE_BUTTON;
    tbb[11].dwData = 0;
    tbb[11].iString = 0;

    tbb[12].iBitmap = 12;
    tbb[12].idCommand = ID_TEXTURES_WIREFRAME;
    tbb[12].fsState = TBSTATE_ENABLED;
    tbb[12].fsStyle = TBSTYLE_BUTTON;
    tbb[12].dwData = 0;
    tbb[12].iString = 0;

    tbb[13].iBitmap = 13;
    tbb[13].idCommand = ID_TEXTURES_FLATSHADE;
    tbb[13].fsState = TBSTATE_ENABLED;
    tbb[13].fsStyle = TBSTYLE_BUTTON;
    tbb[13].dwData = 0;
    tbb[13].iString = 0;

    tbb[14].iBitmap = 14;
    tbb[14].idCommand = ID_VIEW_TRILINEAR;
    tbb[14].fsState = TBSTATE_ENABLED;
    tbb[14].fsStyle = TBSTYLE_BUTTON;
    tbb[14].dwData = 0;
    tbb[14].iString = 0;

    SendMessage(hwndTB, TB_ADDBUTTONS, (WPARAM)NUMBUTTONS,
        (LPARAM) (LPTBBUTTON) &tbb);

    ShowWindow(hwndTB, SW_SHOW);

    return hwndTB;
}

