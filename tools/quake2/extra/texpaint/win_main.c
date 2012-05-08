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

HINSTANCE	main_instance;

int		screen_width, screen_height;

HWND	mainwindow;
HWND	camerawindow;
HWND	palettewindow;
HWND	skinwindow;

/*
=================
Sys_Error

For abnormal program terminations
=================
*/
void Sys_Error (char *error, ...)
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
    MessageBox(mainwindow, text2, "Error", 0 /* MB_OK */ );

	exit (1);
}


/*
======================================================================

FILE DIALOGS

======================================================================
*/

qboolean	modified;
qboolean	modified_past_autosave;

qboolean ConfirmModified (void)
{
	if (!modified)
		return true;

	if (MessageBox (mainwindow, "This will lose changes to the skin"
		, "warning", MB_OKCANCEL) == IDCANCEL)
		return false;
	return true;
}

OPENFILENAME ofn;       /* common dialog box structure   */
char szDirName[MAX_PATH];    /* directory string              */
char szFile[260];       /* filename string               */
char szFileTitle[260];  /* file title string             */
char szSkinFilter[260] =     /* filter string                 */
	"Skin texture (*.lbm *.pcx)\0*.lbm;*.pcx\0\0";
char szFrameFilter[260] =     /* filter string                 */
	"Model frame (*.tri)\0*.tri\0\0";
char chReplace;         /* string separator for szFilter */
int i, cbString;        /* integer count variables       */
HANDLE hf;              /* file handle                   */

void OpenSkinDialog (void)
{
//	strcpy (szDirName, ValueForKey (project_entity, "basepath") );
//	strcat (szDirName, "\\maps");

	/* Place the terminating null character in the szFile. */

	szFile[0] = '\0';

	/* Set the members of the OPENFILENAME structure. */

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = mainwindow;
	ofn.lpstrFilter = szSkinFilter;
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

	Skin_LoadFile (ofn.lpstrFile);
}

void OpenFrameDialog (void)
{
//	strcpy (szDirName, ValueForKey (project_entity, "basepath") );
//	strcat (szDirName, "\\maps");

	/* Place the terminating null character in the szFile. */

	szFile[0] = '\0';

	/* Set the members of the OPENFILENAME structure. */

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = mainwindow;
	ofn.lpstrFilter = szFrameFilter;
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

	LoadTriFile (ofn.lpstrFile);
}

void SaveSkinDialog (void)
{
//	strcpy (szDirName, ValueForKey (project_entity, "basepath") );
//	strcat (szDirName, "\\maps");

	/* Place the terminating null character in the szFile. */

	szFile[0] = '\0';

	/* Set the members of the OPENFILENAME structure. */

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = mainwindow;
	ofn.lpstrFilter = szSkinFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = szDirName;
	ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST |
		OFN_FILEMUSTEXIST;

	/* Display the Open dialog box. */

	if (!GetSaveFileName(&ofn))
		return;	// canceled

	DefaultExtension (ofn.lpstrFile, ".lbm");
	Skin_SaveFile (ofn.lpstrFile);
	strcpy (skin_filename, ofn.lpstrFile);
}

//==========================================================================

BOOL bSetupPixelFormat(HDC hDC)
{
    static PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
	1,				// version number
	PFD_DRAW_TO_WINDOW |		// support window
	  PFD_SUPPORT_OPENGL |		// support OpenGL
	  PFD_DOUBLEBUFFER,		// double buffered
	PFD_TYPE_RGBA,			// RGBA type
	24,				// 24-bit color depth
	0, 0, 0, 0, 0, 0,		// color bits ignored
	0,				// no alpha buffer
	0,				// shift bit ignored
	0,				// no accumulation buffer
	0, 0, 0, 0, 			// accum bits ignored
	32,				// 32-bit z-buffer
	0,				// no stencil buffer
	0,				// no auxiliary buffer
	PFD_MAIN_PLANE,			// main layer
	0,				// reserved
	0, 0, 0				// layer masks ignored
    };
    int pixelformat = 0;
	PIXELFORMATDESCRIPTOR newp;

    if ( (pixelformat = ChoosePixelFormat(hDC, &pfd)) == 0 )
	{
		printf("%d",GetLastError());
        Error ("ChoosePixelFormat failed");
	}
    if (!SetPixelFormat(hDC, pixelformat, &pfd))
        Error ("SetPixelFormat failed");

    return TRUE;
}


/*
==============================================================================

  MENU

==============================================================================
*/


/* handle all WM_COMMAND messages here */
LONG WINAPI CommandHandler (
    HWND    hWnd,
    WPARAM  wParam,
    LPARAM  lParam)
{
	unsigned short	cmd;

	cmd = LOWORD(wParam);

    switch (cmd)
    {
		//
		// file menu
		//
	case ID_FILE_RESAMPLESKIN:
		ResampleSkin ();
		break;

	case ID_FILE_NEWSKIN:
		NewSkin ();
		break;

	case ID_FILE_OPENFRAME:
		OpenFrameDialog ();
		break;

	case ID_FILE_OPENSKIN:
		if (!ConfirmModified())
			break;
		OpenSkinDialog ();
		break;

	case ID_FILE_RELOADSKIN:
		if (!ConfirmModified())
			break;
		Skin_LoadFile (skin_filename);
		break;

	case ID_FILE_SAVESKIN:
		Skin_SaveFile (skin_filename);
		break;

	case ID_FILE_SAVESKINAS:
		SaveSkinDialog ();
		break;
	case ID_FILE_EXIT:
		if (!ConfirmModified())
			break;
        PostQuitMessage (0);
		break;

		//
		// edit menu
		//
	case ID_EDIT_UNDO:
		Undo();
		break;
	case ID_EDIT_REDO:
		Redo();
		break;

		//
		// view menu
		//
	case ID_VIEW_MODELLINES:
		model_lines ^= 1;
		CheckMenuItem ( GetSubMenu (GetMenu(mainwindow), MENU_VIEW)
			, ID_VIEW_MODELLINES
			, MF_BYCOMMAND | (model_lines ? MF_CHECKED : MF_UNCHECKED)  );
		InvalidateRect (camerawindow, NULL, false);
		break;
	case ID_VIEW_TEXTURELINES:
		skin_lines ^= 1;
		CheckMenuItem ( GetSubMenu (GetMenu(mainwindow), MENU_VIEW)
			, ID_VIEW_TEXTURELINES
			, MF_BYCOMMAND | (skin_lines ? MF_CHECKED : MF_UNCHECKED)  );
		InvalidateRect (skinwindow, NULL, false);
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
	case WM_CREATE:
        maindc = GetDC(hWnd);
	    bSetupPixelFormat(maindc);
		break;
    case WM_COMMAND:
		lRet = CommandHandler (hWnd, wParam, lParam);
        break;

	case WM_CLOSE:
		if (!ConfirmModified())
			break;
        PostQuitMessage (0);
		break;
	default:
        /* pass all unhandled messages to DefWindowProc */
        lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
		break;
    }

    /* return 1 if handled message, 0 if not */
    return lRet;
}




/*
==============
Main_Create
==============
*/
void Main_Create (HINSTANCE hInstance)
{
    WNDCLASS   wc;

    /* Register the class */
	memset (&wc, 0, sizeof(wc));

    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)WMAIN_WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU2);
    wc.lpszClassName = "TEXPAINT_MAIN";

    if (!RegisterClass (&wc) )
        Error ("WCam_Register: failed");


	mainwindow = CreateWindow ("TEXPAINT_MAIN" ,
		"Texpaint",
		WS_OVERLAPPEDWINDOW |
		WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN,
		0,0,screen_width,screen_height,	// size
		0,
		NULL,		// no menu
		hInstance,
		NULL);
	if (!mainwindow)
		Error ("Couldn't create main window");

//	GetWindowInfo("mainwindow", &SavedInfo, NULL);
	ShowWindow (mainwindow, SW_SHOWDEFAULT);
}




BOOL SaveWindowInfo(const char *pszName, void *pvBuf, long lSize)
{
	LONG lres;
	DWORD dwDisp;
	HKEY  hKeyId;

	lres = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\id\\Texpaint", 0, NULL,
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyId, &dwDisp);

	if (lres != ERROR_SUCCESS)
		return FALSE;

	lres = RegSetValueEx(hKeyId, pszName, 0, REG_BINARY, pvBuf, lSize);

	RegCloseKey(hKeyId);

	if (lres != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}


BOOL GetWindowInfo(const char *pszName, void *pvBuf, long *plSize)
{
	HKEY  hKey;
	long lres, lType, lSize;

	if (plSize == NULL)
		plSize = &lSize;

	lres = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\id\\Texpaint", 0, KEY_READ, &hKey);

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
	MapWindowPoints(NULL, mainwindow, (POINT *)&rc, 2);
	return SaveWindowInfo(pszName, &rc, sizeof(rc));
}


BOOL RestoreWindowState(HWND hWnd, const char *pszName)
{
	RECT rc;
	LONG lSize = sizeof(rc);

	if (GetWindowInfo(pszName, &rc, &lSize))
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

