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

BOOL CALLBACK GammaDlgProc (
    HWND hwndDlg,	// handle to dialog box
    UINT uMsg,	// message
    WPARAM wParam,	// first message parameter
    LPARAM lParam 	// second message parameter
   )
{
	char sz[256];

	switch (uMsg)
    {
	case WM_INITDIALOG:
		sprintf(sz, "%1.1f", g_qeglobals.d_savedinfo.fGamma);
		SetWindowText(GetDlgItem(hwndDlg, IDC_G_EDIT), sz);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case IDOK:
			GetWindowText(GetDlgItem(hwndDlg, IDC_G_EDIT), sz, 255);
			g_qeglobals.d_savedinfo.fGamma = atof(sz);
			EndDialog(hwndDlg, 1);
			return TRUE;

		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		}
	}
	return FALSE;
}



void DoGamma(void)
{
	char *psz, sz[256];
	if ( DialogBox(g_qeglobals.d_hInstance, (char *)IDD_GAMMA, g_qeglobals.d_hwndMain, GammaDlgProc))
	{
		psz = ValueForKey(world_entity, "_wad");
		if (psz)
		{
			strcpy(sz, psz);
			Texture_Flush();
			Texture_ShowInuse();
		}
	}
}

//================================================


void SelectBrush (int entitynum, int brushnum)
{
	entity_t	*e;
	brush_t		*b;
	int			i;

	if (entitynum == 0)
		e = world_entity;
	else
	{
		e = entities.next;
		while (--entitynum)
		{
			e=e->next;
			if (e == &entities)
			{
				Sys_Status ("No such entity.", 0);
				return;
			}
		}
	}

	b = e->brushes.onext;
	if (b == &e->brushes)
	{
		Sys_Status ("No such brush.", 0);
		return;
	}
	while (brushnum--)
	{
		b=b->onext;
		if (b == &e->brushes)
		{
			Sys_Status ("No such brush.", 0);
			return;
		}
	}

	Brush_RemoveFromList (b);
	Brush_AddToList (b, &selected_brushes);


	Sys_UpdateWindows (W_ALL);
	for (i=0 ; i<3 ; i++)
		g_qeglobals.d_xy.origin[i] = (b->mins[i] + b->maxs[i])/2;

	Sys_Status ("Selected.", 0);
}

/*
=================
GetSelectionIndex
=================
*/
void GetSelectionIndex (int *ent, int *brush)
{
	brush_t		*b, *b2;
	entity_t	*entity;

	*ent = *brush = 0;

	b = selected_brushes.next;
	if (b == &selected_brushes)
		return;

	// find entity
	if (b->owner != world_entity)
	{
		(*ent)++;
		for (entity = entities.next ; entity != &entities
			; entity=entity->next, (*ent)++)
		;
	}

	// find brush
	for (b2=b->owner->brushes.onext
		; b2 != b && b2 != &b->owner->brushes
		; b2=b2->onext, (*brush)++)
	;
}

BOOL CALLBACK FindBrushDlgProc (
    HWND hwndDlg,	// handle to dialog box
    UINT uMsg,	// message
    WPARAM wParam,	// first message parameter
    LPARAM lParam 	// second message parameter
   )
{
	char entstr[256];
	char brushstr[256];
	HWND	h;
	int		ent, brush;

	switch (uMsg)
    {
	case WM_INITDIALOG:
		// set entity and brush number
		GetSelectionIndex (&ent, &brush);
		sprintf (entstr, "%i", ent);
		sprintf (brushstr, "%i", brush);
		SetWindowText(GetDlgItem(hwndDlg, IDC_FIND_ENTITY), entstr);
		SetWindowText(GetDlgItem(hwndDlg, IDC_FIND_BRUSH), brushstr);

		h = GetDlgItem(hwndDlg, IDC_FIND_ENTITY);
		SetFocus (h);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDOK:
				GetWindowText(GetDlgItem(hwndDlg, IDC_FIND_ENTITY), entstr, 255);
				GetWindowText(GetDlgItem(hwndDlg, IDC_FIND_BRUSH), brushstr, 255);
				SelectBrush (atoi(entstr), atoi(brushstr));
				EndDialog(hwndDlg, 1);
				return TRUE;

			case IDCANCEL:
				EndDialog(hwndDlg, 0);
				return TRUE;
		}
	}
	return FALSE;
}



void DoFind(void)
{
	DialogBox(g_qeglobals.d_hInstance, (char *)IDD_FINDBRUSH, g_qeglobals.d_hwndMain, FindBrushDlgProc);
}

/*
===================================================

  ARBITRARY ROTATE

===================================================
*/


BOOL CALLBACK RotateDlgProc (
    HWND hwndDlg,	// handle to dialog box
    UINT uMsg,	// message
    WPARAM wParam,	// first message parameter
    LPARAM lParam 	// second message parameter
   )
{
	char	str[256];
	HWND	h;
	float	v;

	switch (uMsg)
    {
	case WM_INITDIALOG:
		h = GetDlgItem(hwndDlg, IDC_FIND_ENTITY);
		SetFocus (h);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case IDOK:
			GetWindowText(GetDlgItem(hwndDlg, IDC_ROTX), str, 255);
			v = atof(str);
			if (v)
				Select_RotateAxis (0, v);

			GetWindowText(GetDlgItem(hwndDlg, IDC_ROTY), str, 255);
			v = atof(str);
			if (v)
				Select_RotateAxis (1, v);

			GetWindowText(GetDlgItem(hwndDlg, IDC_ROTZ), str, 255);
			v = atof(str);
			if (v)
				Select_RotateAxis (2, v);

			EndDialog(hwndDlg, 1);
			return TRUE;

		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		}
	}

	return FALSE;
}



void DoRotate(void)
{
	DialogBox(g_qeglobals.d_hInstance, (char *)IDD_ROTATE, g_qeglobals.d_hwndMain, RotateDlgProc);
}

/*
===================================================

  ARBITRARY SIDES

===================================================
*/


BOOL CALLBACK SidesDlgProc (
    HWND hwndDlg,	// handle to dialog box
    UINT uMsg,	// message
    WPARAM wParam,	// first message parameter
    LPARAM lParam 	// second message parameter
   )
{
	char str[256];
	HWND	h;

	switch (uMsg)
    {
	case WM_INITDIALOG:
		h = GetDlgItem(hwndDlg, IDC_FIND_ENTITY);
		SetFocus (h);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case IDOK:
			GetWindowText(GetDlgItem(hwndDlg, IDC_SIDES), str, 255);
			Brush_MakeSided (atoi(str));

			EndDialog(hwndDlg, 1);
		break;

		case IDCANCEL:
			EndDialog(hwndDlg, 0);
		break;
	}
	default:
		return FALSE;
	}
}



void DoSides(void)
{
	DialogBox(g_qeglobals.d_hInstance, (char *)IDD_SIDES, g_qeglobals.d_hwndMain, SidesDlgProc);
}

//======================================================================

/*
===================
DoAbout
===================
*/
BOOL CALLBACK AboutDlgProc( HWND hwndDlg,
						    UINT uMsg,
						    WPARAM wParam,
						    LPARAM lParam )
{
	switch (uMsg)
    {
	case WM_INITDIALOG:
		{
			char renderer[1024];
			char version[1024];
			char vendor[1024];
			char extensions[4096];

			sprintf( renderer, "Renderer:\t%s", glGetString( GL_RENDERER ) );
			sprintf( version, "Version:\t\t%s", glGetString( GL_VERSION ) );
			sprintf( vendor, "Vendor:\t\t%s", glGetString( GL_VENDOR ) );
			sprintf( extensions, "\n%s", glGetString( GL_EXTENSIONS ) );

			SetWindowText( GetDlgItem( hwndDlg, IDC_ABOUT_GLRENDERER ),   renderer );
			SetWindowText( GetDlgItem( hwndDlg, IDC_ABOUT_GLVERSION ),    version );
			SetWindowText( GetDlgItem( hwndDlg, IDC_ABOUT_GLVENDOR ),     vendor );
			SetWindowText( GetDlgItem( hwndDlg, IDC_ABOUT_GLEXTENSIONS ), extensions );
		}
		return TRUE;

	case WM_CLOSE:
		EndDialog( hwndDlg, 1 );
		return TRUE;

	case WM_COMMAND:
		if ( LOWORD( wParam ) == IDOK )
			EndDialog(hwndDlg, 1);
		return TRUE;
	}
	return FALSE;
}

void DoAbout(void)
{
	DialogBox( g_qeglobals.d_hInstance, ( char * ) IDD_ABOUT, g_qeglobals.d_hwndMain, AboutDlgProc );
}


/*
===================================================

  SURFACE INSPECTOR

===================================================
*/

texdef_t	g_old_texdef;
HWND		g_surfwin;
qboolean	g_changed_surface;

int	g_checkboxes[64] = {
	IDC_CHECK1, IDC_CHECK2, IDC_CHECK3, IDC_CHECK4,
	IDC_CHECK5, IDC_CHECK6, IDC_CHECK7, IDC_CHECK8,
	IDC_CHECK9, IDC_CHECK10, IDC_CHECK11, IDC_CHECK12,
	IDC_CHECK13, IDC_CHECK14, IDC_CHECK15, IDC_CHECK16,
	IDC_CHECK17, IDC_CHECK18, IDC_CHECK19, IDC_CHECK20,
	IDC_CHECK21, IDC_CHECK22, IDC_CHECK23, IDC_CHECK24,
	IDC_CHECK25, IDC_CHECK26, IDC_CHECK27, IDC_CHECK28,
	IDC_CHECK29, IDC_CHECK30, IDC_CHECK31, IDC_CHECK32,

	IDC_CHECK33, IDC_CHECK34, IDC_CHECK35, IDC_CHECK36,
	IDC_CHECK37, IDC_CHECK38, IDC_CHECK39, IDC_CHECK40,
	IDC_CHECK41, IDC_CHECK42, IDC_CHECK43, IDC_CHECK44,
	IDC_CHECK45, IDC_CHECK46, IDC_CHECK47, IDC_CHECK48,
	IDC_CHECK49, IDC_CHECK50, IDC_CHECK51, IDC_CHECK52,
	IDC_CHECK53, IDC_CHECK54, IDC_CHECK55, IDC_CHECK56,
	IDC_CHECK57, IDC_CHECK58, IDC_CHECK59, IDC_CHECK60,
	IDC_CHECK61, IDC_CHECK62, IDC_CHECK63, IDC_CHECK64
 };

/*
==============
SetTexMods

Set the fields to the current texdef
===============
*/
void SetTexMods(void)
{
	char	sz[128];
	texdef_t *pt;
	int		i;

	pt = &g_qeglobals.d_texturewin.texdef;

	SendMessage (g_surfwin, WM_SETREDRAW, 0, 0);

	SetWindowText(GetDlgItem(g_surfwin, IDC_TEXTURE), pt->name);

	sprintf(sz, "%d", (int)pt->shift[0]);
	SetWindowText(GetDlgItem(g_surfwin, IDC_HSHIFT), sz);

	sprintf(sz, "%d", (int)pt->shift[1]);
	SetWindowText(GetDlgItem(g_surfwin, IDC_VSHIFT), sz);

	sprintf(sz, "%4.2f", pt->scale[0]);
	SetWindowText(GetDlgItem(g_surfwin, IDC_HSCALE), sz);

	sprintf(sz, "%4.2f", pt->scale[1]);
	SetWindowText(GetDlgItem(g_surfwin, IDC_VSCALE), sz);

	sprintf(sz, "%d", (int)pt->rotate);
	SetWindowText(GetDlgItem(g_surfwin, IDC_ROTATE), sz);

	sprintf(sz, "%d", (int)pt->value);
	SetWindowText(GetDlgItem(g_surfwin, IDC_VALUE), sz);

	for (i=0 ; i<32 ; i++)
		SendMessage(GetDlgItem(g_surfwin, g_checkboxes[i]), BM_SETCHECK, !!(pt->flags&(1<<i)), 0 );
	for (i=0 ; i<32 ; i++)
		SendMessage(GetDlgItem(g_surfwin, g_checkboxes[32+i]), BM_SETCHECK, !!(pt->contents&(1<<i)), 0 );

	SendMessage (g_surfwin, WM_SETREDRAW, 1, 0);
	InvalidateRect (g_surfwin, NULL, true);
}


/*
==============
GetTexMods

Reads the fields to get the current texdef
===============
*/
void GetTexMods(void)
{
	char	sz[128];
	texdef_t *pt;
	int		b;
	int		i;

	pt = &g_qeglobals.d_texturewin.texdef;

	GetWindowText (GetDlgItem(g_surfwin, IDC_TEXTURE), sz, 127);
	strncpy (pt->name, sz, sizeof(pt->name)-1);
	if (pt->name[0] <= ' ')
	{
		strcpy (pt->name, "none");
		SetWindowText(GetDlgItem(g_surfwin, IDC_TEXTURE), pt->name);
	}

	GetWindowText (GetDlgItem(g_surfwin, IDC_HSHIFT), sz, 127);
	pt->shift[0] = atof(sz);

	GetWindowText (GetDlgItem(g_surfwin, IDC_VSHIFT), sz, 127);
	pt->shift[1] = atof(sz);

	GetWindowText(GetDlgItem(g_surfwin, IDC_HSCALE), sz, 127);
	pt->scale[0] = atof(sz);

	GetWindowText(GetDlgItem(g_surfwin, IDC_VSCALE), sz, 127);
	pt->scale[1] = atof(sz);

	GetWindowText(GetDlgItem(g_surfwin, IDC_ROTATE), sz, 127);
	pt->rotate = atof(sz);

	GetWindowText(GetDlgItem(g_surfwin, IDC_VALUE), sz, 127);
	pt->value = atof(sz);

	pt->flags = 0;
	for (i=0 ; i<32 ; i++)
	{
		b = SendMessage(GetDlgItem(g_surfwin, g_checkboxes[i]), BM_GETCHECK, 0, 0);
		if (b != 1 && b != 0)
			continue;
		pt->flags |= b<<i;
	}

	pt->contents = 0;
	for (i=0 ; i<32 ; i++)
	{
		b = SendMessage(GetDlgItem(g_surfwin, g_checkboxes[32+i]), BM_GETCHECK, 0, 0);
		if (b != 1 && b != 0)
			continue;
		pt->contents |= b<<i;
	}

	g_changed_surface = true;
	Select_SetTexture(pt);
}

/*
=================
UpdateSpinners
=================
*/
void UpdateSpinners(unsigned uMsg, WPARAM wParam, LPARAM lParam)
{
	int nScrollCode;
	HWND hwnd;
	texdef_t *pt;

	pt = &g_qeglobals.d_texturewin.texdef;

	nScrollCode = (int) LOWORD(wParam);  // scroll bar value
	hwnd = (HWND) lParam;       // handle of scroll bar

	if ((nScrollCode != SB_LINEUP) && (nScrollCode != SB_LINEDOWN))
		return;

	if (hwnd == GetDlgItem(g_surfwin, IDC_ROTATEA))
	{
		if (nScrollCode == SB_LINEUP)
			pt->rotate += 45;
		else
			pt->rotate -= 45;

		if (pt->rotate < 0)
			pt->rotate += 360;

		if (pt->rotate >= 360)
			pt->rotate -= 360;
	}

	else if (hwnd == GetDlgItem(g_surfwin, IDC_HSCALEA))
	{
		if (nScrollCode == SB_LINEDOWN)
			pt->scale[0] -= 0.1;
		else
			pt->scale[0] += 0.1;
	}

	else if (hwnd == GetDlgItem(g_surfwin, IDC_VSCALEA))
	{
		if (nScrollCode == SB_LINEUP)
			pt->scale[1] += 0.1;
		else
			pt->scale[1] -= 0.1;
	}

	else if (hwnd == GetDlgItem(g_surfwin, IDC_HSHIFTA))
	{
		if (nScrollCode == SB_LINEDOWN)
			pt->shift[0] -= 8;
		else
			pt->shift[0] += 8;
	}

	else if (hwnd == GetDlgItem(g_surfwin, IDC_VSHIFTA))
	{
		if (nScrollCode == SB_LINEUP)
			pt->shift[1] += 8;
		else
			pt->shift[1] -= 8;
	}

	SetTexMods();
	g_changed_surface = true;
	Select_SetTexture(pt);
}



BOOL CALLBACK SurfaceDlgProc (
    HWND hwndDlg,	// handle to dialog box
    UINT uMsg,	// message
    WPARAM wParam,	// first message parameter
    LPARAM lParam 	// second message parameter
   )
{
	switch (uMsg)
    {
	case WM_INITDIALOG:
		g_surfwin = hwndDlg;
		SetTexMods ();
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case IDOK:
			GetTexMods ();
			EndDialog(hwndDlg, 1);
		break;

		case IDAPPLY:
			GetTexMods ();
			InvalidateRect(g_qeglobals.d_hwndCamera, NULL, false);
			UpdateWindow (g_qeglobals.d_hwndCamera);
		break;

		case IDCANCEL:
			g_qeglobals.d_texturewin.texdef = g_old_texdef;
			if (g_changed_surface)
				Select_SetTexture(&g_qeglobals.d_texturewin.texdef);
			EndDialog(hwndDlg, 0);
		break;
		}
		break;

	case WM_HSCROLL:
	case WM_VSCROLL:
		UpdateSpinners(uMsg, wParam, lParam);
		InvalidateRect(g_qeglobals.d_hwndCamera, NULL, false);
		UpdateWindow (g_qeglobals.d_hwndCamera);
		return 0;

	default:
		return FALSE;
	}
}



void DoSurface (void)
{
	// save current state for cancel
	g_old_texdef = g_qeglobals.d_texturewin.texdef;
	g_changed_surface = false;

	DialogBox(g_qeglobals.d_hInstance, (char *)IDD_SURFACE, g_qeglobals.d_hwndMain, SurfaceDlgProc);
}

