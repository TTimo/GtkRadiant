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
#include "mru.h"

int	screen_width;
int	screen_height;
qboolean	have_quit;

int	update_bits;

HANDLE	bsp_process;

//===========================================

void Sys_SetTitle (char *text)
{
	SetWindowText (g_qeglobals.d_hwndMain, text);
}

HCURSOR	waitcursor;

void Sys_BeginWait (void)
{
	waitcursor = SetCursor (LoadCursor (NULL, IDC_WAIT));
}

void Sys_EndWait (void)
{
	if (waitcursor)
	{
		SetCursor (waitcursor);
		waitcursor = NULL;
	}
}


void Sys_GetCursorPos (int *x, int *y)
{
	POINT lpPoint;

	GetCursorPos (&lpPoint);
	*x = lpPoint.x;
	*y = lpPoint.y;
}

void Sys_SetCursorPos (int x, int y)
{
	SetCursorPos (x, y);
}

void Sys_UpdateWindows (int bits)
{
//	Sys_Printf("updating 0x%X\n", bits);
	update_bits |= bits;
//update_bits = -1;
}


void Sys_Beep (void)
{
	MessageBeep (MB_ICONASTERISK);
}

char	*TranslateString (char *buf)
{
	static	char	buf2[32768];
	int		i, l;
	char	*out;

	l = strlen(buf);
	out = buf2;
	for (i=0 ; i<l ; i++)
	{
		if (buf[i] == '\n')
		{
			*out++ = '\r';
			*out++ = '\n';
		}
		else
			*out++ = buf[i];
	}
	*out++ = 0;

	return buf2;
}

void Sys_ClearPrintf (void)
{
	char	text[4];

	text[0] = 0;

	SendMessage (g_qeglobals.d_hwndEdit,
		WM_SETTEXT,
		0,
		(LPARAM)text);
}

void Sys_Printf (char *text, ...)
{
	va_list argptr;
	char	buf[32768];
	char	*out;

	va_start (argptr,text);
	vsprintf (buf, text,argptr);
	va_end (argptr);

	out = TranslateString (buf);

#ifdef LATER
	Sys_Status(out);
#else
	SendMessage (g_qeglobals.d_hwndEdit,
		EM_REPLACESEL,
		0,
		(LPARAM)out);
#endif

}

double Sys_DoubleTime (void)
{
	return clock()/ 1000.0;
}

void PrintPixels (HDC hDC)
{
	int		i;
	PIXELFORMATDESCRIPTOR p[64];

	printf ("### flags color layer\n");
	for (i=1 ; i<64 ; i++)
	{
		if (!DescribePixelFormat ( hDC, i, sizeof(p[0]), &p[i]))
			break;
		printf ("%3i %5i %5i %5i\n", i,
			p[i].dwFlags,
			p[i].cColorBits,
			p[i].bReserved);
	}
	printf ("%i modes\n", i-1);
}



//==========================================================================

void QEW_StopGL( HWND hWnd, HGLRC hGLRC, HDC hDC )
{
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( hGLRC );
	ReleaseDC( hWnd, hDC );
}

int QEW_SetupPixelFormat(HDC hDC, qboolean zbuffer )
{
    static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
		1,								// version number
		PFD_DRAW_TO_WINDOW |			// support window
		PFD_SUPPORT_OPENGL |			// support OpenGL
		PFD_DOUBLEBUFFER,				// double buffered
		PFD_TYPE_RGBA,					// RGBA type
		24,								// 24-bit color depth
		0, 0, 0, 0, 0, 0,				// color bits ignored
		0,								// no alpha buffer
		0,								// shift bit ignored
		0,								// no accumulation buffer
		0, 0, 0, 0,						// accum bits ignored
		32,							    // depth bits
		0,								// no stencil buffer
		0,								// no auxiliary buffer
		PFD_MAIN_PLANE,					// main layer
		0,								// reserved
		0, 0, 0							// layer masks ignored
    };
    int pixelformat = 0;

	zbuffer = true;
	if ( !zbuffer )
		pfd.cDepthBits = 0;

    if ( (pixelformat = ChoosePixelFormat(hDC, &pfd)) == 0 )
	{
		printf("%d",GetLastError());
        Error ("ChoosePixelFormat failed");
	}

    if (!SetPixelFormat(hDC, pixelformat, &pfd))
        Error ("SetPixelFormat failed");

	return pixelformat;
}

/*
=================
Error

For abnormal program terminations
=================
*/
void Error (char *error, ...)
{
	va_list argptr;
	char	text[1024];
	char	text2[1024];
	int		err;

	err = GetLastError ();

	va_start (argptr,error);
	vsprintf (text, error,argptr);
	va_end (argptr);

	sprintf (text2, "%s\nGetLastError() = %i", text, err);
    MessageBox(g_qeglobals.d_hwndMain, text2, "Error", 0 /* MB_OK */ );

	exit (1);
}

/*
======================================================================

FILE DIALOGS

======================================================================
*/

qboolean ConfirmModified (void)
{
	if (!modified)
		return true;

	if (MessageBox (g_qeglobals.d_hwndMain, "This will lose changes to the map"
		, "warning", MB_OKCANCEL) == IDCANCEL)
		return false;
	return true;
}

static OPENFILENAME ofn;       /* common dialog box structure   */
static char szDirName[MAX_PATH];    /* directory string              */
static char szFile[260];       /* filename string               */
static char szFileTitle[260];  /* file title string             */
static char szFilter[260] =     /* filter string                 */
	"QuakeEd file (*.map)\0*.map\0\0";
static char szProjectFilter[260] =     /* filter string                 */
	"QuakeEd project (*.qe4)\0*.qe4\0\0";
static char chReplace;         /* string separator for szFilter */
static int i, cbString;        /* integer count variables       */
static HANDLE hf;              /* file handle                   */

void OpenDialog (void)
{
	/*
	 * Obtain the system directory name and
	 * store it in szDirName.
	 */

	strcpy (szDirName, ValueForKey (g_qeglobals.d_project_entity, "basepath") );
	strcat (szDirName, "\\maps");

	/* Place the terminating null character in the szFile. */

	szFile[0] = '\0';

	/* Set the members of the OPENFILENAME structure. */

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_qeglobals.d_hwndCamera;
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = szDirName;
	ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST |
		OFN_FILEMUSTEXIST;

	/* Display the Open dialog box. */

	if (!GetOpenFileName(&ofn))
		return;	// canceled

	// Add the file in MRU.
	AddNewItem( g_qeglobals.d_lpMruMenu, ofn.lpstrFile);

	// Refresh the File menu.
	PlaceMenuMRUItem(g_qeglobals.d_lpMruMenu,GetSubMenu(GetMenu(g_qeglobals.d_hwndMain),0),
			ID_FILE_EXIT);

	/* Open the file. */

	Map_LoadFile (ofn.lpstrFile);
}

void ProjectDialog (void)
{
	/*
	 * Obtain the system directory name and
	 * store it in szDirName.
	 */

	strcpy (szDirName, ValueForKey(g_qeglobals.d_project_entity, "basepath") );
	strcat (szDirName, "\\scripts");

	/* Place the terminating null character in the szFile. */

	szFile[0] = '\0';

	/* Set the members of the OPENFILENAME structure. */

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_qeglobals.d_hwndCamera;
	ofn.lpstrFilter = szProjectFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = szDirName;
	ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST |
		OFN_FILEMUSTEXIST;

	/* Display the Open dialog box. */

	if (!GetOpenFileName(&ofn))
		return;	// canceled

	// Refresh the File menu.
	PlaceMenuMRUItem(g_qeglobals.d_lpMruMenu,GetSubMenu(GetMenu(g_qeglobals.d_hwndMain),0),
			ID_FILE_EXIT);

	/* Open the file. */
	if (!QE_LoadProject(ofn.lpstrFile))
		Error ("Couldn't load project file");
}


void SaveAsDialog (void)
{
	strcpy (szDirName, ValueForKey (g_qeglobals.d_project_entity, "basepath") );
	strcat (szDirName, "\\maps");

	/* Place the terminating null character in the szFile. */

	szFile[0] = '\0';

	/* Set the members of the OPENFILENAME structure. */

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_qeglobals.d_hwndCamera;
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = szDirName;
	ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST |
		OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

	/* Display the Open dialog box. */

	if (!GetSaveFileName(&ofn))
		return;	// canceled

	DefaultExtension (ofn.lpstrFile, ".map");
	strcpy (currentmap, ofn.lpstrFile);

	// Add the file in MRU.
	AddNewItem(g_qeglobals.d_lpMruMenu, ofn.lpstrFile);

	// Refresh the File menu.
	PlaceMenuMRUItem(g_qeglobals.d_lpMruMenu,GetSubMenu(GetMenu(g_qeglobals.d_hwndMain),0),
			ID_FILE_EXIT);

	Map_SaveFile (ofn.lpstrFile, false);	// ignore region
}

/*
=======================================================

Menu modifications

=======================================================
*/

/*
==================
FillBSPMenu

==================
*/
char	*bsp_commands[256];

void FillBSPMenu (void)
{
	HMENU	hmenu;
	epair_t	*ep;
	int		i;
	static int count;

	hmenu = GetSubMenu (GetMenu(g_qeglobals.d_hwndMain), MENU_BSP);

	for (i=0 ; i<count ; i++)
		DeleteMenu (hmenu, CMD_BSPCOMMAND+i, MF_BYCOMMAND);
	count = 0;

	i = 0;
	for (ep = g_qeglobals.d_project_entity->epairs ; ep ; ep=ep->next)
	{
		if (ep->key[0] == 'b' && ep->key[1] == 's' && ep->key[2] == 'p')
		{
			bsp_commands[i] = ep->key;
			AppendMenu (hmenu, MF_ENABLED|MF_STRING,
			CMD_BSPCOMMAND+i, (LPCTSTR)ep->key);
			i++;
		}
	}
	count = i;
}

//==============================================

/*
===============
CheckBspProcess

See if the BSP is done yet
===============
*/
void CheckBspProcess (void)
{
	char	outputpath[1024];
	char	temppath[512];
	DWORD	exitcode;
	char	*out;
	BOOL	ret;

	if (!bsp_process)
		return;

	ret = GetExitCodeProcess (bsp_process, &exitcode);
	if (!ret)
		Error ("GetExitCodeProcess failed");
	if (exitcode == STILL_ACTIVE)
		return;

	bsp_process = 0;

	GetTempPath(512, temppath);
	sprintf (outputpath, "%sjunk.txt", temppath);

	LoadFile (outputpath, (void *)&out);
	Sys_Printf ("%s", out);
	Sys_Printf ("\ncompleted.\n");
	free (out);
	Sys_Beep ();

	Pointfile_Check ();
}

extern int	cambuttonstate;

/*
==================
WinMain

==================
*/
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance
					,LPSTR lpCmdLine, int nCmdShow)
{
    MSG        msg;
	double		time, oldtime, delta;
	HACCEL		accelerators;

	g_qeglobals.d_hInstance = hInstance;

	InitCommonControls ();

	screen_width = GetSystemMetrics (SM_CXFULLSCREEN);
	screen_height = GetSystemMetrics (SM_CYFULLSCREEN);

	// hack for broken NT 4.0 dual screen
	if (screen_width > 2*screen_height)
		screen_width /= 2;

	accelerators = LoadAccelerators (hInstance
		, MAKEINTRESOURCE(IDR_ACCELERATOR1));
	if (!accelerators)
		Error ("LoadAccelerators failed");

	Main_Create (hInstance);

	WCam_Create (hInstance);
	WXY_Create (hInstance);
	WZ_Create (hInstance);
	CreateEntityWindow(hInstance);

	// the project file can be specified on the command line,
	// or implicitly found in the scripts directory
	if (lpCmdLine && strlen(lpCmdLine))
	{
		ParseCommandLine (lpCmdLine);
		if (!QE_LoadProject(argv[1]))
			Error ("Couldn't load %s project file", argv[1]);
	}
	else if (!QE_LoadProject("scripts/quake.qe4"))
		Error ("Couldn't load scripts/quake.qe4 project file");

	QE_Init ();

	Sys_Printf ("Entering message loop\n");

	oldtime = Sys_DoubleTime ();

	while (!have_quit)
	{
		Sys_EndWait ();		// remove wait cursor if active

		while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (!TranslateAccelerator(g_qeglobals.d_hwndMain, accelerators, &msg) )
			{
      			TranslateMessage (&msg);
      			DispatchMessage (&msg);
			}
			if (msg.message == WM_QUIT)
				have_quit = true;
		}


		CheckBspProcess ();

		time = Sys_DoubleTime ();
		delta = time - oldtime;
		oldtime = time;
		if (delta > 0.2)
			delta = 0.2;

		// run time dependant behavior
		Cam_MouseControl (delta);

		// update any windows now
		if (update_bits & W_CAMERA)
		{
			InvalidateRect(g_qeglobals.d_hwndCamera, NULL, false);
			UpdateWindow (g_qeglobals.d_hwndCamera);
		}
		if (update_bits & (W_Z | W_Z_OVERLAY) )
		{
			InvalidateRect(g_qeglobals.d_hwndZ, NULL, false);
			UpdateWindow (g_qeglobals.d_hwndZ);
		}

		if ( update_bits & W_TEXTURE )
		{
			InvalidateRect(g_qeglobals.d_hwndTexture, NULL, false);
			UpdateWindow (g_qeglobals.d_hwndEntity);
		}

		if (update_bits & (W_XY | W_XY_OVERLAY))
		{
			InvalidateRect(g_qeglobals.d_hwndXY, NULL, false);
			UpdateWindow (g_qeglobals.d_hwndXY);
		}

		update_bits = 0;

		if (!cambuttonstate && !have_quit)
		{	// if not driving in the camera view, block
			WaitMessage ();
		}

	}

    /* return success of application */
    return TRUE;

}

