/*
   Copyright (C) 1999-2007 id Software, Inc. and contributors.
   For a list of contributors, see the accompanying CONTRIBUTORS file.

   This file is part of GtkRadiant.

   GtkRadiant is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   GtkRadiant is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GtkRadiant; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

//
// Z Window
//
// Leonardo Zide (leo@lokigames.com)
//

#include "stdafx.h"
#include "zwindow.h"

// =============================================================================
// ZWnd class

ZWnd::ZWnd ()
	: GLWindow( FALSE ){
}

ZWnd::~ZWnd(){
}

void ZWnd::OnCreate(){
	g_qeglobals_gui.d_z = m_pWidget;

	if ( !MakeCurrent() ) {
		Error( "wglMakeCurrent in CZWnd::OnCreate failed" );
	}
}

void ZWnd::OnLButtonDown( guint32 nFlags, int pointx, int pointy ){
	SetFocus();
	SetCapture();
	Z_MouseDown( pointx, m_pWidget->allocation.height - 1 - pointy, nFlags );
}

void ZWnd::OnMButtonDown( guint32 nFlags, int pointx, int pointy ){
	SetFocus();
	SetCapture();
	Z_MouseDown( pointx, m_pWidget->allocation.height - 1 - pointy, nFlags );
}

void ZWnd::OnRButtonDown( guint32 nFlags, int pointx, int pointy ){
	SetFocus();
	SetCapture();
	Z_MouseDown( pointx, m_pWidget->allocation.height - 1 - pointy, nFlags );
}

void ZWnd::OnLButtonUp( guint32 nFlags, int pointx, int pointy ){
	Z_MouseUp( pointx, m_pWidget->allocation.height - 1 - pointy, nFlags );
	ReleaseCapture();
}

void ZWnd::OnMButtonUp( guint32 nFlags, int pointx, int pointy ){
	Z_MouseUp( pointx, m_pWidget->allocation.height - 1 - pointy, nFlags );
	ReleaseCapture();
}

void ZWnd::OnRButtonUp( guint32 nFlags, int pointx, int pointy ){
	Z_MouseUp( pointx, m_pWidget->allocation.height - 1 - pointy, nFlags );
	ReleaseCapture();
}

void ZWnd::OnMouseMove( guint32 nFlags, int pointx, int pointy ){
	float fz = z.origin[2] + ( ( m_pWidget->allocation.height - 1 - pointy ) - ( z.height / 2 ) ) / z.scale;
	fz = floor( fz / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;
	CString strStatus;
	strStatus.Format( "Z:: %.1f", fz );
	g_pParentWnd->SetStatusText( 1, strStatus );
	Z_MouseMoved( pointx, m_pWidget->allocation.height - 1 - pointy, nFlags );
}

void ZWnd::OnExpose(){
	if ( !MakeCurrent() ) {
		Sys_FPrintf( SYS_ERR, "ERROR: wglMakeCurrent failed..\n " );
		Sys_Printf( "Please restart Radiant if the Z view is not working\n" );
	}
	else
	{
		QE_CheckOpenGLForErrors();
		Z_Draw();
		QE_CheckOpenGLForErrors();
		SwapBuffers();
	}
}

void ZWnd::OnSize( int cx, int cy ){
	z.width = cx;
	z.height = cy;
	if ( z.width < 10 ) {
		z.width = 10;
	}
	if ( z.height < 10 ) {
		z.height = 10;
	}
	RedrawWindow();
}
