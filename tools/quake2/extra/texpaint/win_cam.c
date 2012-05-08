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

#define	CAMERA_WINDOW_CLASS	"TPCamera"

HDC		camdc;
HGLRC	baseRC;

float	pitch, yaw, roll;
qboolean	model_lines = false;

float	cam_x, cam_y=-64, cam_z=32;

int		cam_width, cam_height;

BINDTEXFUNCPTR BindTextureEXT;

void InitIndexTexture (void)
{
	int		i;

	BindTextureEXT (GL_TEXTURE_2D, TEXTURE_INDEX);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	for (i=0 ; i<sizeof(index_texture)/4 ; i++)
		index_texture[i] = i+1;

	glTexImage2D (GL_TEXTURE_2D, 0, 3, width2, height2, 0, GL_RGBA, GL_UNSIGNED_BYTE, index_texture);

	BindTextureEXT (GL_TEXTURE_2D, TEXTURE_SKIN);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void CreateDisplaylist (void)
{
}

void DrawModel (void)
{
	int		i, j;

	glColor4f (1,1,1,1);

	glBegin (GL_TRIANGLES);
	for (i=0 ; i<numfaces ; i++)
	{
		for (j=0 ; j<3 ; j++)
		{
			glTexCoord2f (tmcoords[i][j][0], tmcoords[i][j][1]);
			glVertex3fv (faces[i].verts[j]);
		}
	}
	glEnd ();
}

/*
=============
Cam_Click
=============
*/
int		cam_last_index;
void Cam_Click (int x, int y, qboolean shift)
{
	int		index;
	index = 0;
	glReadBuffer (GL_BACK);
	glReadPixels (x, y, 1,1, GL_RGB, GL_UNSIGNED_BYTE, &index);

	index--;
	if (index == -1)
		return;
	if (index >= width2*height2)
		return;

	if (index == cam_last_index)
		return;		// in same pixel
	cam_last_index = index;
	if (shift)
	{
		Pal_SetIndex (pic[index]);
		return;
	}

	SetSkin (index, selected_rgb);
	UpdateWindow (camerawindow);
}


void Cam_DrawSetup (void)
{
	glViewport (0,0,cam_width, cam_height);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (90,  (float)cam_width/cam_height,  2,  1024);
	gluLookAt (cam_x, cam_y, cam_z,   cam_x, cam_y+1, cam_z,  0, 0, 1);

	glRotated (-roll*0.3, 0, 1, 0);
	glRotated (-pitch*0.3, 1, 0, 0);
	glRotated (yaw*0.3, 0, 0, 1);

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable (GL_DEPTH_TEST);
	glEnable (GL_CULL_FACE);
	glEnable (GL_TEXTURE_2D);
	glCullFace (GL_FRONT);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void Cam_Draw (void)
{
	if (!cam_width || !cam_height)
		return;

	glClearColor (0.3,0.3,0.3,1);
	Cam_DrawSetup ();

	BindTextureEXT (GL_TEXTURE_2D, TEXTURE_SKIN);

	DrawModel ();

	if (model_lines)
	{
		glDisable (GL_TEXTURE_2D);
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		glDepthFunc (GL_LEQUAL);
		glDepthRange (0, 0.999);	// nudge depth to avoid dropouts
		DrawModel ();
		glDepthRange (0, 1);

		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glEnable (GL_TEXTURE_2D);
	}

	SwapBuffers(camdc);

	// now fill the back buffer with the index texture
	glClearColor (0,0,0,0);
	glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	BindTextureEXT (GL_TEXTURE_2D, TEXTURE_INDEX);
	DrawModel ();

	BindTextureEXT (GL_TEXTURE_2D, TEXTURE_SKIN);
}



/*
============
CameraWndProc
============
*/
LONG WINAPI WCam_WndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    LONG    lRet = 1;
	int		fwKeys, xPos, yPos;
    RECT	rect;
	static int	oldx, oldy;
	POINT	pt;

    GetClientRect(hWnd, &rect);
	cam_width = rect.right-rect.left;
	cam_height = rect.bottom-rect.top;

    switch (uMsg)
    {
	case WM_CREATE:
        camdc = GetDC(hWnd);
	    bSetupPixelFormat(camdc);

        baseRC = wglCreateContext( camdc );
		if (!baseRC)
			Sys_Error ("wglCreateContext failed");
        if (!wglMakeCurrent( camdc, baseRC ))
			Sys_Error ("wglMakeCurrent failed");
		BindTextureEXT = (void *)wglGetProcAddress((LPCSTR) "glBindTextureEXT");
		if (!BindTextureEXT)
			Sys_Error ("GetProcAddress for BindTextureEXT failed");

		break;
	case WM_PAINT:
        {
		    PAINTSTRUCT	ps;

		    BeginPaint(hWnd, &ps);
            if (!wglMakeCurrent( camdc, baseRC ))
				Sys_Error ("wglMakeCurrent failed");
			Cam_Draw ();
		    EndPaint(hWnd, &ps);
        }
		break;

		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			if (GetTopWindow(mainwindow) != hWnd)
				BringWindowToTop(hWnd);

			SetFocus (camerawindow);
			SetCapture (camerawindow);
			GetCursorPos (&pt);
			xPos = pt.x;
			yPos = pt.y;
			oldx = xPos;
			oldy = yPos;
			break;

		case WM_LBUTTONDOWN:
			cam_last_index = -1;
draw:
			if (GetTopWindow(mainwindow) != hWnd)
				BringWindowToTop(hWnd);

			SetFocus (camerawindow);
			SetCapture (camerawindow);
			fwKeys = wParam;        // key flags
			xPos = (short)LOWORD(lParam);  // horizontal position of cursor
			yPos = (short)HIWORD(lParam);  // vertical position of cursor
			yPos = (int)rect.bottom - 1 - yPos;
            if (!wglMakeCurrent( camdc, baseRC ))
				Sys_Error ("wglMakeCurrent failed");

			Cam_Click (xPos, yPos, !!(wParam&(MK_SHIFT|MK_CONTROL)) );

//			Cam_MouseDown (xPos, yPos, fwKeys);
			break;

		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_LBUTTONUP:
			if (! (wParam & (MK_LBUTTON|MK_RBUTTON|MK_MBUTTON)))
				ReleaseCapture ();
			break;

		case WM_MOUSEMOVE:
			{
				int		dx, dy;

				if (wParam & MK_LBUTTON)
					goto draw;

				GetCursorPos (&pt);
				xPos = pt.x;
				yPos = pt.y;
				if (!(wParam & (MK_RBUTTON|MK_MBUTTON)))
				{
					oldx = xPos;
					oldy = yPos;
					break;
				}
				dx = xPos-oldx;
				dy = oldy-yPos;
				if (!dx && !dy)
					break;
				SetCursorPos (oldx, oldy);

				if (wParam == (MK_RBUTTON|MK_CONTROL) )
				{
					if (abs(dx) > abs(dy))
						cam_y -= 0.1*dx;
					else
						cam_y -= 0.1*dy;
					InvalidateRect (camerawindow, NULL, false);
				}
				if (wParam == MK_RBUTTON)
				{
					cam_x -= 0.1*dx;
					cam_z -= 0.1*dy;
					InvalidateRect (camerawindow, NULL, false);
				}
				if (wParam == (MK_MBUTTON|MK_CONTROL) )
				{
					if (abs(dx) > abs(dy))
						roll -= dx;
					else
						roll -= dy;
					InvalidateRect (camerawindow, NULL, false);
				}
				if (wParam == MK_MBUTTON)
				{
					yaw += dx;
					pitch += dy;
					InvalidateRect (camerawindow, NULL, false);
				}
			}
			break;



    	case WM_SIZE:
//			camera.width = rect.right;
//			camera.height = rect.bottom;
			InvalidateRect(camerawindow, NULL, false);
            break;
		case WM_NCCALCSIZE:// don't let windows copy pixels
			lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
			return WVR_REDRAW;
   	    case WM_CLOSE:
            /* call destroy window to cleanup and go away */
            DestroyWindow (hWnd);
        break;

   	    case WM_DESTROY:
        {
    	    HGLRC hRC;
    	    HDC	  hDC;

                /* release and free the device context and rendering context */
    	    hRC = wglGetCurrentContext();
    	    hDC = wglGetCurrentDC();

    	    wglMakeCurrent(NULL, NULL);

    	    if (hRC)
    	    	wglDeleteContext(hRC);
    	    if (hDC)
    	        ReleaseDC(hWnd, hDC);
        }
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
WCam_Register
==============
*/
void WCam_Register (HINSTANCE hInstance)
{
    WNDCLASS   wc;

    /* Register the camera class */
	memset (&wc, 0, sizeof(wc));

    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)WCam_WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = CAMERA_WINDOW_CLASS;

    if (!RegisterClass (&wc) )
        Sys_Error ("WCam_Register: failed");
}


void WCam_Create (HINSTANCE hInstance)
{
	WCam_Register (hInstance);

	camerawindow = CreateWindow (CAMERA_WINDOW_CLASS ,
		"Camera View",
		QE3_STYLE,
		0,
		0,
		(int)(screen_width*0.5),
		(int)(screen_height-20),	// size

		mainwindow,	// parent window
		0,		// no menu
		hInstance,
		0);
	if (!camerawindow)
		Sys_Error ("Couldn't create camerawindow");

	RestoreWindowState(camerawindow, "camerawindow");
    ShowWindow (camerawindow, SW_SHOWDEFAULT);
}
