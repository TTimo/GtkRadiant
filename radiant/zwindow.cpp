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
	pointy = m_pWidget->allocation.height - 1 - pointy;
	float fz = z.origin[2] + ( pointy - ( z.height / 2 ) ) / z.scale;
	fz = floor( fz / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;
	CString strStatus;
	strStatus.Format( "Z:: %.1f", fz );
	g_pParentWnd->SetStatusText( 1, strStatus );
	Z_MouseMoved( pointx, pointy, nFlags );

	// very handy for understanding
	//float mouse_pos_in_3d = z.origin[2] + ( pointy - ( z.height / 2 ) ) / z.scale;
	//Sys_Printf("Z_MouseMoved x=%d y=%d mouse_pos_in_3d=%f (z.origin[2]=%f + ( y=%d - ( z.height=%d / 2 ) / z.scale=%f)\n", pointx, pointy, mouse_pos_in_3d, z.origin[2], pointy, z.height, z.scale);
}

void ZWnd::OnMouseWheel(bool bUp, int pointx, int pointy) {

	float old_z = z.origin[2] + ( pointy - ( z.height / 2 ) ) / z.scale;

	if (bUp)
		z.scale *= 2;
	else
		z.scale /= 2;

	float new_z = z.origin[2] + ( pointy - ( z.height / 2 ) ) / z.scale;

	float delta = new_z - old_z;
	//Sys_Printf("Delta: %f\n", delta);
	// Zoom into the mouse position
	z.origin[2] += delta;


	//Sys_Printf("ZWnd::OnMouseWheel> bUp=%d pointx=%d pointy=%d z.scale=%f\n", bUp, pointx, pointy, z.scale);
	Sys_UpdateWindows(W_Z);
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
