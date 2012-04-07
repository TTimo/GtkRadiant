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

#define	PALETTE_WINDOW_CLASS	"TPPalette"

HDC		paldc;
int		pal_width, pal_height;
int		blocks_x, blocks_y;

int			selected_index;
unsigned	selected_rgb;

byte	palette[768];

float SnapAspect (float aspect)
{
	if (aspect > 128)
		return 256;
	if (aspect > 32)
		return 128;
	if (aspect > 8)
		return 64;
	if (aspect > 2)
		return 32;
	return 16;
}

void Pal_SetIndex (int index)
{
	selected_index = index;
	selected_rgb = palette[index*3] + (palette[index*3+1]<<8) + (palette[index*3+2]<<16);
	InvalidateRect (palettewindow, NULL, false);
}

void Pal_Draw (void)
{
	int		x, y;
	float	aspect;
	float	xs, ys;
	int		c;

	if (pal_width < 1 || pal_height < 1)
		return;

	//
	// determine the block arrangement
	//
	if (pal_width > pal_height)
	{
		aspect = SnapAspect (pal_width / pal_height);
		blocks_x = aspect;
		blocks_y = 256/blocks_x;
	}
	else
	{
		aspect = SnapAspect (pal_height / pal_width);
		blocks_y = aspect;
		blocks_x = 256/blocks_y;
	}

	//
	// draw it
	//
	glViewport (0,0,pal_width, pal_height);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0,1,0,1,-100,100);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glClear (GL_COLOR_BUFFER_BIT);
	glDisable (GL_DEPTH_TEST);
	glDisable (GL_CULL_FACE);
	glDisable (GL_TEXTURE_2D);

	xs = 1.0/blocks_x;
	ys = 1.0/blocks_y;

	for (x=0 ; x<blocks_x ; x++)
	{
		for (y=0 ; y<blocks_y ; y++)
		{
			c = x*blocks_y+(blocks_y-1-y);
			glColor3ubv (palette+c*3);
			glRectf (x*xs, y*ys, (x+1)*xs, (y+1)*ys);
		}
	}

	// highlight the selected texture
	y = selected_index % blocks_y;
	x = selected_index / blocks_y;
	y = blocks_y-1-y;

	glColor3f (0,0,0);
	glRectf ( (x+0.4)*xs, (y+0.4)*ys, (x+0.6)*xs, (y+0.6)*ys);
}

void Pal_Click (int x, int y)
{
	int		index;

	x = x*blocks_x/pal_width;
	y = y*blocks_y/pal_height;
	y = blocks_y-1-y;

	index = x*blocks_y + y;
	Pal_SetIndex (index);
}

/*
============
Palette_WndProc
============
*/
LONG WINAPI Palette_WndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    LONG    lRet = 1;
	int		fwKeys, xPos, yPos;
    RECT	rect;

    GetClientRect(hWnd, &rect);
	pal_width = rect.right-rect.left;
	pal_height = rect.bottom-rect.top;

    switch (uMsg)
    {
	case WM_CREATE:
        paldc = GetDC(hWnd);
	    bSetupPixelFormat(paldc);
		break;
	case WM_PAINT:
        {
		    PAINTSTRUCT	ps;

		    BeginPaint(hWnd, &ps);
            if (!wglMakeCurrent( paldc, baseRC ))
				Error ("wglMakeCurrent failed");
			Pal_Draw ();
		    EndPaint(hWnd, &ps);
			SwapBuffers(paldc);
        }
		break;

		case WM_MOUSEMOVE:
			if (wParam != MK_LBUTTON)
				break;
		case WM_LBUTTONDOWN:
			if (GetTopWindow(mainwindow) != hWnd)
				BringWindowToTop(hWnd);

			xPos = (short)LOWORD(lParam);  // horizontal position of cursor
			yPos = (short)HIWORD(lParam);  // vertical position of cursor
			yPos = (int)rect.bottom - 1 - yPos;

			Pal_Click (xPos, yPos);
			break;

		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_LBUTTONUP:
			fwKeys = wParam;        // key flags
			xPos = (short)LOWORD(lParam);  // horizontal position of cursor
			yPos = (short)HIWORD(lParam);  // vertical position of cursor
			yPos = (int)rect.bottom - 1 - yPos;
			ReleaseCapture ();
			break;

    	case WM_SIZE:
			InvalidateRect(skinwindow, NULL, false);
            break;
		case WM_NCCALCSIZE:// don't let windows copy pixels
			lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
			return WVR_REDRAW;
   	    case WM_CLOSE:
            /* call destroy window to cleanup and go away */
            DestroyWindow (hWnd);
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
WPal_Create
==============
*/
void WPal_Create (HINSTANCE hInstance)
{
    WNDCLASS   wc;

    /* Register the skin class */
	memset (&wc, 0, sizeof(wc));

    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)Palette_WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = PALETTE_WINDOW_CLASS;

    if (!RegisterClass (&wc) )
        Error ("RegisterClass failed");

	palettewindow = CreateWindow (PALETTE_WINDOW_CLASS ,
		"Palette View",
		QE3_STYLE,
		(int)(screen_width*0.5),
		0,
		(int)(screen_width*0.5),
		(int)(screen_height*.2),	// size
		mainwindow,	// parent window
		0,		// no menu
		hInstance,
		0);
	if (!palettewindow)
		Error ("Couldn't create palettewindow");

//	RestoreWindowState(palettewindow, "palettewindow");
    ShowWindow (palettewindow, SW_SHOWDEFAULT);
}
