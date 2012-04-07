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

#define	SKIN_WINDOW_CLASS	"TPSkin"

HDC		skindc;
int		skinw_width, skinw_height;	// size of the window

float	skin_x = 128, skin_y = 128, skin_z = 100;

qboolean	skin_lines = false;

char	tri_filename[1024];
char	skin_filename[1024];
int		skin_width, skin_height;	// size of the .lbm image

unsigned	index_texture[1024*512];


void UpdateTexture (int offset)
{
	int		x, y;

	y = offset / width2;
	x = offset % width2;

	BindTextureEXT (GL_TEXTURE_2D, TEXTURE_SKIN);
//	glTexImage2D (GL_TEXTURE_2D, 0, 3, width2, height2, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgb);

	glTexSubImage2D (GL_TEXTURE_2D, 0, x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgb+offset);
}

/*
===================================================================

  TEXEL MODIFICATION

===================================================================
*/

#define	MAX_MODIFY	8192

typedef struct
{
	int		offset;
	int		oldvalue;
} modify_t;

int			modify_index;
int			undo_index;
modify_t	modify[MAX_MODIFY];

void SetSkinModified (void)
{
	char	text[1024];

	if (modified && modified_past_autosave)
		return;

	modified = true;
	modified_past_autosave = true;

	sprintf (text, "%s *", skin_filename);
	SetWindowText (skinwindow, text);
}

void SetSkin (int index, int pixel)
{
	modify_t	*m;

	if (!modified)
		SetSkinModified ();

	// save undo info
	m = &modify[undo_index];
	m->offset = index;
	m->oldvalue = pic[index];

	modify_index = (++undo_index)&(MAX_MODIFY-1);

	// modify it
	rgb[index] = selected_rgb;
	pic[index] = selected_index;
	UpdateTexture (index);
	InvalidateRect (skinwindow, NULL, false);
	InvalidateRect (camerawindow, NULL, false);
}

void Undo (void)
{
	modify_t	*m;
	int			temp;

	if (!undo_index)
		return;

	if (!--undo_index)
	{	// back to unmodified state
		modified = false;
		SetWindowText (skinwindow, skin_filename);
	}
	m = &modify[undo_index];

	// modify it
	temp = pic[m->offset];
	pic[m->offset] = m->oldvalue;
	rgb[m->offset] = palette[m->oldvalue*3] +
		(palette[m->oldvalue*3+1]<<8) + (palette[m->oldvalue*3+2]<<16);
	m->oldvalue = temp;
	UpdateTexture (m->offset);
	InvalidateRect (skinwindow, NULL, false);
	InvalidateRect (camerawindow, NULL, false);

}

void Redo (void)
{
	modify_t	*m;
	int			temp;

	if (undo_index == modify_index)
		return;

	m = &modify[undo_index];

	// modify it
	temp = pic[m->offset];
	pic[m->offset] = m->oldvalue;
	rgb[m->offset] = palette[m->oldvalue*3] +
		(palette[m->oldvalue*3+1]<<8) + (palette[m->oldvalue*3+2]<<16);
	m->oldvalue = temp;
	UpdateTexture (m->offset);
	InvalidateRect (skinwindow, NULL, false);
	InvalidateRect (camerawindow, NULL, false);

	if (!undo_index++)
	{	// modified again
		char	text[1024];

		modified = true;
		sprintf (text, "%s *", skin_filename);
		SetWindowText (skinwindow, text);
	}
}

//===================================================================

/*
=============
Skin_SaveFile

Load a skin texture and the base.tri from the same directory
=============
*/
void Skin_SaveFile (char *name)
{
	byte	*data;
	int		i, j;
	char	backup[1024];

	// back up the current file if it exists
	sprintf (backup, "%s.bak", name);
	remove (backup);
	rename (name, backup);

	modified = false;
	modified_past_autosave = false;
	modify_index = undo_index = 0;
	SetWindowText (skinwindow, skin_filename);

	data = malloc(skin_width*skin_height);
	for (i=0 ; i<skin_height ; i++)
		memcpy (data + i*skin_width, pic + i*width2, skin_width);
	Save256Image (name, data, palette, skin_width, skin_height);

	free(data);
}

/*
=============
Expand256Texture

=============
*/
void Expand256Texture (void)
{
	int		i, j;
	int		p;

	memset (rgb, 0, sizeof(rgb));
	for (i=0 ; i<skin_height ; i++)
	{
		for (j=0 ; j<skin_width ; j++)
		{
			p = pic[i*width2+j];
			rgb[i*width2+j] = (palette[p*3+0]<<0) + (palette[p*3+1]<<8) + (palette[p*3+2]<<16);
		}
	}

	BindTextureEXT (GL_TEXTURE_2D, TEXTURE_SKIN);
	glTexImage2D (GL_TEXTURE_2D, 0, 3, width2, height2, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgb);
}

void SetSizes (int width, int height)
{
	int		i;

	if (width < 32)
		width = 16;
	if (height < 32)
		height = 16;

	skin_width = width;
	skin_height = height;

	if (skin_width > 1024 || skin_height > 512)
		Sys_Error ("Skin file is too large");

	width2 = 1;
	height2 = 1;
	for (i=0 ; i<12 ; i++)
	{
		if (width2 < skin_width)
			width2<<=1;
		if (height2 < skin_height)
			height2<<=1;
	}

	// compatability shit for auto sizing of old skins
	if (skin_width != 320 || skin_height != 200)
	{
		skinwidth = skin_width;
		skinheight = skin_height;
	}
	else
	{
		skinwidth = 0;
		skinheight = 0;
	}
}

/*
=============
Skin_LoadFile

Load a skin texture and the base.tri from the same directory
=============
*/
void Skin_LoadFile (char *name)
{
	int		i, j, p;
	byte	*lbmpic;
	byte	*lbmpal;
	char	trifile[1024];
	int		width, height;

	modified = false;
	modified_past_autosave = false;
	modify_index = undo_index = 0;
	strcpy (skin_filename, name);
	SetWindowText (skinwindow, skin_filename);

	//
	// read the texture
	//
	Load256Image (skin_filename, &lbmpic, &lbmpal, &width, &height);
	memcpy (palette, lbmpal, sizeof(palette));
	free (lbmpal);

	SetSizes (width, height);

	memset (pic, 0, sizeof(pic));
	for (i=0 ; i<skin_height ; i++)
	{
		for (j=0 ; j<skin_width ; j++)
		{
			p = lbmpic[i*skin_width + j];
			pic[i*width2+j] = p;
		}
	}
	free (lbmpic);

	Expand256Texture ();

	InitIndexTexture ();

	Pal_SetIndex (selected_index);

	//
	// read the polfile and
	// generate the texture coordinates
	//
	strcpy (trifile, skin_filename);
	StripExtension (trifile);
	strcat (trifile, ".tri");
	if (FileExists (trifile))
	{
		LoadTriFile (trifile);
		CalcTmCoords ();
	}
	else
	{
		ExtractFilePath (name, trifile);
		strcat (trifile, "base.tri");
		if (FileExists (trifile))
		{
			LoadTriFile (trifile);
			CalcTmCoords ();
		}
	}

	InvalidateRect (palettewindow, NULL, false);
	InvalidateRect (skinwindow, NULL, false);
	InvalidateRect (camerawindow, NULL, false);

}


/*
=============
Skin_Click
=============
*/
int		skin_last_index;
void Skin_Click (int x, int y, qboolean shift)
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

	if (index == skin_last_index)
		return;		// in same pixel
	skin_last_index = index;

	if (shift)
	{
		Pal_SetIndex (pic[index]);
		return;
	}

	SetSkin (index, selected_index);
	UpdateWindow (skinwindow);
}


void DrawModelST (void)
{
	int		i, j;

	glColor4f (1,1,1,1);

	glBegin (GL_TRIANGLES);
	for (i=0 ; i<numfaces ; i++)
	{
		for (j=0 ; j<3 ; j++)
		{
			glVertex2f (tmcoords[i][j][0]*width2, (1-tmcoords[i][j][1])*height2);
		}
	}
	glEnd ();
}

void DrawSkin (void)
{
	glBegin (GL_POLYGON);
	glTexCoord2f (0,1);
	glVertex2f (0,0);

	glTexCoord2f (0,0);
	glVertex2f (0,height2);

	glTexCoord2f (1,0);
	glVertex2f (width2,height2);

	glTexCoord2f (1,1);
	glVertex2f (width2,0);
	glEnd ();

}

void Skin_Draw (void)
{
	int		x, y;
	float	aspect;
	float	xs, ys;
	int		c;

	//
	// draw it
	//
	if (skin_z < 20)
		skin_z = 20;

	glViewport (0,0,skinw_width, skinw_height);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (90,  (float)skinw_width/skinw_height,  2,  16384);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	gluLookAt (skin_x, skin_y, skin_z,   skin_x, skin_y, skin_z-1,  0, 1, 0);

	glClearColor (0.3,0.3,0.3,1);
	glClear (GL_COLOR_BUFFER_BIT);
	glDisable (GL_DEPTH_TEST);
	glDisable (GL_CULL_FACE);
	glEnable (GL_TEXTURE_2D);

	glColor4f (1,1,1,1);

	DrawSkin ();

	if (skin_lines)
	{
		glDisable (GL_TEXTURE_2D);
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

		DrawModelST ();

		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glEnable (GL_TEXTURE_2D);
	}

	SwapBuffers(skindc);


	// now fill the back buffer with the index texture
	glClearColor (0,0,0,0);
	glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	BindTextureEXT (GL_TEXTURE_2D, TEXTURE_INDEX);
	DrawSkin ();

	BindTextureEXT (GL_TEXTURE_2D, TEXTURE_SKIN);
}

/*
============
Skin_WndProc
============
*/
LONG WINAPI Skin_WndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    LONG    lRet = 1;
	int		fwKeys, xPos, yPos;
    RECT	rect;

    GetClientRect(hWnd, &rect);
	skinw_width = rect.right-rect.left;
	skinw_height = rect.bottom-rect.top;

    switch (uMsg)
    {
	case WM_CREATE:
        skindc = GetDC(hWnd);
	    bSetupPixelFormat(skindc);
		break;
	case WM_PAINT:
        {
		    PAINTSTRUCT	ps;

		    BeginPaint(hWnd, &ps);
            if (!wglMakeCurrent( skindc, baseRC ))
				Sys_Error ("wglMakeCurrent failed");
			Skin_Draw ();
		    EndPaint(hWnd, &ps);
        }
		break;

	case WM_LBUTTONDOWN:
		skin_last_index = -1;
draw:
		if (GetTopWindow(mainwindow) != hWnd)
			BringWindowToTop(hWnd);

		SetFocus (skinwindow);
		SetCapture (skinwindow);
		fwKeys = wParam;        // key flags
		xPos = (short)LOWORD(lParam);  // horizontal position of cursor
		yPos = (short)HIWORD(lParam);  // vertical position of cursor
		yPos = (int)rect.bottom - 1 - yPos;
        if (!wglMakeCurrent( skindc, baseRC ))
			Sys_Error ("wglMakeCurrent failed");
		Skin_Click (xPos, yPos, !!(wParam&(MK_SHIFT|MK_CONTROL)) );
		break;

	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	case WM_LBUTTONUP:
		fwKeys = wParam;        // key flags
		if (! (fwKeys & (MK_LBUTTON|MK_RBUTTON|MK_MBUTTON)))
			ReleaseCapture ();
		break;

	case WM_MOUSEMOVE:
		{
			static int	oldx, oldy;
			int		dx, dy;
			POINT	pt;

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
					skin_z += 0.25*dx;
				else
					skin_z += 0.25*dy;
				InvalidateRect (skinwindow, NULL, false);
			}
			if (wParam == MK_RBUTTON)
			{
				skin_x -= 0.25*dx;
				skin_y -= 0.25*dy;
				InvalidateRect (skinwindow, NULL, false);
			}
		}
		break;

    case WM_SIZE:
		InvalidateRect(camerawindow, NULL, false);
        break;
	case WM_NCCALCSIZE:// don't let windows copy pixels
		lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
		return WVR_REDRAW;
   	case WM_CLOSE:
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
WSkin_Create
==============
*/
void WSkin_Create (HINSTANCE hInstance)
{
    WNDCLASS   wc;

    /* Register the camera class */
	memset (&wc, 0, sizeof(wc));

    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)Skin_WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = SKIN_WINDOW_CLASS;

    if (!RegisterClass (&wc) )
        Sys_Error ("RegisterClass failed");

	skinwindow = CreateWindow (SKIN_WINDOW_CLASS ,
		"Skin View",
		QE3_STYLE,
		(int)(screen_width*0.5),
		(int)(screen_height*0.2),
		(int)(screen_width*0.5),
		(int)(screen_height*0.8),	// size
		mainwindow,	// parent window
		0,		// no menu
		hInstance,
		0);
	if (!skinwindow)
		Error ("Couldn't create skinwindow");

//	RestoreWindowState(palettewindow, "palettewindow");
    ShowWindow (skinwindow, SW_SHOWDEFAULT);
}


/*
===================================================================

  SKIN RESAMPLING

===================================================================
*/

HWND	resamplewindow;
HDC		resampledc;

#define	RESAMPLE_WINDOW_CLASS	"TPResample"

/*
============
Resample_WndProc
============
*/
LONG WINAPI Resample_WndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch (uMsg)
    {
	case WM_CREATE:
		resampledc = GetDC(hWnd);
	    bSetupPixelFormat(resampledc);
		break;
	}

	return	DefWindowProc (hWnd, uMsg, wParam, lParam);
}

/*
==============
ResampleWindow
==============
*/
void ResampleWindow (HINSTANCE hInstance)
{
    WNDCLASS   wc;
	static qboolean registered;

	if (!registered)
	{
		registered = true;
		/* Register the camera class */
		memset (&wc, 0, sizeof(wc));

		wc.style         = 0;
		wc.lpfnWndProc   = (WNDPROC)Resample_WndProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = 0;
		wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = 0;
		wc.lpszClassName = RESAMPLE_WINDOW_CLASS;

		if (!RegisterClass (&wc) )
			Sys_Error ("RegisterClass failed");
	}

	resamplewindow = CreateWindow (RESAMPLE_WINDOW_CLASS ,
		"ResampleWindow",
		WS_OVERLAPPED,
		0, 0, width2+32, height2+32,	// size
		NULL,	// parent window
		0,		// no menu
		hInstance,
		0);
	if (!resamplewindow)
		Error ("Couldn't create skinwindow");

    ShowWindow (resamplewindow, SW_SHOWDEFAULT);
}


void OutlineTexture (byte *pic)
{
	int		i, j;
	int		x, y;
	int		empty;
	byte	oldpic[1024*512];

	memcpy (oldpic, pic, width2*height2);

	empty = oldpic[0];

	for (i=0 ; i<height2 ; i++)
	{
		for (j=0 ; j<width2 ; j++)
		{
			if (oldpic[i*width2+j] != empty)
				continue;
			for (x=-1 ; x<=1 ; x++)
			{
				for (y=-1 ; y<=1 ; y++)
				{
					if (i+y < 0 || i+y >= height2)
						continue;
					if (j+x < 0 || j+x >= width2)
						continue;
					if (oldpic[(i+y)*width2 + j+x] != empty)
					{
						pic[i*width2+j] = oldpic[(i+y)*width2 + j+x];
						goto done;
					}
				}
			}
done: ;
		}
	}
}

void ResampleSkin (void)
{
	int		i, j;
	static	float	oldtmcoords[10000][3][2];
	static	int		newindex[1024*512];
	static	byte	oldpic[1024*512];

	// open a window of the texture size
	ResampleWindow (main_instance);

	// get new S/T from current frame
	memcpy (oldtmcoords, tmcoords, numfaces*3*2*4);
	CalcTmCoords ();

	// draw all the triangles with the index texture
    if (!wglMakeCurrent( resampledc, baseRC ))
		Sys_Error ("wglMakeCurrent failed");

	glViewport (0,0,width2, height2);
	glClearColor (0,0,0,0);
	glClear (GL_COLOR_BUFFER_BIT);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, width2, 0, height2, -100, 100);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glColor4f (1,1,1,1);
	glDisable (GL_DEPTH_TEST);
	glDisable (GL_CULL_FACE);
	BindTextureEXT (GL_TEXTURE_2D, TEXTURE_INDEX);
#if 0
	glDisable(GL_TEXTURE_2D);
	glBegin (GL_LINE_LOOP);
	glVertex3f (1,1,10);
	glVertex3f (skin_width-1,0,10);
	glVertex3f (skin_width-1,skin_height-1,10);
	glVertex3f (1,skin_height-1,10);
	glEnd ();
	glEnable(GL_TEXTURE_2D);
#endif
	glBegin (GL_TRIANGLES);
	for (i=0 ; i<numfaces ; i++)
	{
		for (j=0 ; j<3 ; j++)
		{
			glTexCoord2f (oldtmcoords[i][j][0], oldtmcoords[i][j][1]);
			glVertex3f (tmcoords[i][j][0]*width2, tmcoords[i][j][1]*height2, 10);
		}
	}
	glEnd ();
	SwapBuffers (resampledc);

	// build the new color texture
	memcpy (oldpic, pic, width2*height2);
	glReadBuffer (GL_FRONT);
	glReadPixels (0,0,width2,height2,GL_RGBA,GL_UNSIGNED_BYTE, &newindex);
	for (i=0 ; i<height2 ; i++)
		for (j=0 ; j<width2 ; j++)
			pic[i*width2+j] = oldpic[newindex[i*width2+j]&0xffffff];

	// outline it
	OutlineTexture (pic);
	Expand256Texture ();

	InvalidateRect (skinwindow, NULL, false);
	InvalidateRect (camerawindow, NULL, false);

	// change name
	strcpy (skin_filename, tri_filename);
	StripExtension (skin_filename);
	strcat (skin_filename, ".lbm");

	SetSkinModified ();

	wglMakeCurrent (NULL, NULL);
	DestroyWindow (resamplewindow);
}

/*
===================================================================

  NEW SKIN

===================================================================
*/

BOOL CALLBACK NewSkinDlgProc (
    HWND hwndDlg,	// handle to dialog box
    UINT uMsg,	// message
    WPARAM wParam,	// first message parameter
    LPARAM lParam 	// second message parameter
   )
{
	char sz[256];
	int		width, height;

	switch (uMsg)
    {
	case WM_INITDIALOG:
		SetWindowText(GetDlgItem(hwndDlg, IDC_WIDTH), "320");
		SetWindowText(GetDlgItem(hwndDlg, IDC_HEIGHT), "200");
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case IDOK:
			GetWindowText(GetDlgItem(hwndDlg, IDC_WIDTH), sz, 255);
			width = atoi(sz);
			GetWindowText(GetDlgItem(hwndDlg, IDC_HEIGHT), sz, 255);
			height = atoi(sz);
			SetSizes (width, height);
			EndDialog(hwndDlg, 1);
			return TRUE;

		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		}
	}
	return FALSE;
}


void NewSkin (void)
{
	int		i, j;
	byte	*buf;

	if (!DialogBox(main_instance, (char *)IDD_NEWSKIN, mainwindow, NewSkinDlgProc))
		return;

	// open a window of the texture size
	ResampleWindow (main_instance);

	// get new S/T from current frame
	CalcTmCoords ();

	// draw all the triangles
    if (!wglMakeCurrent( resampledc, baseRC ))
		Sys_Error ("wglMakeCurrent failed");

	glViewport (0,0,width2, height2);
	glClearColor (0,0,0,0);
	glClear (GL_COLOR_BUFFER_BIT);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, width2, 0, height2, -100, 100);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glColor4f (1,1,1,1);
	glDisable (GL_DEPTH_TEST);
	glDisable (GL_CULL_FACE);
	glDisable (GL_TEXTURE_2D);

	for (i=0 ; i<numfaces ; i++)
	{
		glColor3f ((i&255)/255.0, (i&255)/255.0, (i&255)/255.0);
		glBegin (GL_TRIANGLES);
		for (j=0 ; j<3 ; j++)
			glVertex3f (tmcoords[i][j][0]*width2, tmcoords[i][j][1]*height2, 10);
		glEnd ();
	}

	SwapBuffers (resampledc);

	// build the new color texture
	glReadBuffer (GL_FRONT);
	buf = malloc(width2*height2*4);
	glReadPixels (0,0,width2,height2,GL_RGBA,GL_UNSIGNED_BYTE, buf);
	for (i=0 ; i<width2*height2 ; i++)
		pic[i] = buf[i*4];
	free (buf);

	// outline it
	OutlineTexture (pic);
	Expand256Texture ();
	InitIndexTexture ();

	InvalidateRect (skinwindow, NULL, false);
	InvalidateRect (camerawindow, NULL, false);

	// change name
	strcpy (skin_filename, tri_filename);
	StripExtension (skin_filename);
	strcat (skin_filename, ".lbm");

	SetSkinModified ();

	wglMakeCurrent (NULL, NULL);
	DestroyWindow (resamplewindow);
}
