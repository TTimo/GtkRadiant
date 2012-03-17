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

/*
   Camera plugin for GtkRadiant
   Copyright (C) 2002 Splash Damage Ltd.
 */

#include "camera.h"

CListener::CListener(){
	refCount = 1;

	m_bHooked = FALSE;

	m_bLeftMBPressed = m_bRightMBPressed = m_bMiddleMBPressed = false;

	oldValid = false;

	Register();
}

CListener::~CListener(){
	UnRegister();
}

void CListener::Register(){
	g_UITable.m_pfnHookWindow( this );
	g_pXYWndWrapper = g_UITable.m_pfnGetXYWndWrapper();
	m_bHooked = TRUE;
}

void CListener::UnRegister(){
	if ( m_bHooked ) {
		g_UITable.m_pfnUnHookWindow( this );
		g_pXYWndWrapper = NULL;
		m_bHooked = FALSE;
	}
}

bool CListener::OnMouseMove( guint32 nFlags, gdouble x, gdouble y ){
	SetViewType( g_pXYWndWrapper->GetViewType() );

	if ( m_bLeftMBPressed && oldValid && g_iEditMode == 0 ) {
		vec3_t click, delta;

		g_pXYWndWrapper->SnapToGrid( (int)x, (int)y, click );

		switch ( m_vt ) {
		case XY:
			VectorSet( delta, click[0] - old_x, click[1] - old_y, 0 );
			old_x = click[0]; old_y = click[1];
			break;
		case XZ:
			VectorSet( delta, click[0] - old_x, 0, click[2] - old_y );
			old_x = click[0]; old_y = click[2];
			break;
		case YZ:
			VectorSet( delta, 0, click[1] - old_x, click[2] - old_y );
			old_x = click[1]; old_y = click[2];
			break;
		}

		if ( g_iActiveTarget < 0 ) {
			GetCurrentCam()->GetCam()->getPositionObj()->updateSelection( delta[0], delta[1], delta[2] );
		}
		else{
			GetCurrentCam()->GetCam()->getActiveTarget( g_iActiveTarget )->updateSelection( delta[0], delta[1], delta[2] );
		}

		GetCurrentCam()->HasBeenModified();

		g_FuncTable.m_pfnSysUpdateWindows( W_XY_OVERLAY | W_CAMERA );

		return true;
	}

	return false;
}

bool CListener::OnLButtonDown( guint32 nFlags, gdouble x, gdouble y ){
	SetViewType( g_pXYWndWrapper->GetViewType() );

	m_bLeftMBPressed = true;
	oldValid = true;

	vec3_t org, delta;

	g_pXYWndWrapper->SnapToGrid( (int)x, (int)y, org );

	switch ( m_vt ) {
	case XY:
		old_x = org[0]; old_y = org[1]; org[2] = 64 * 1024;
		VectorSet( delta, 0, 0, -1 );
		break;
	case XZ:
		old_x = org[0]; old_y = org[2]; org[1] = 64 * 1024;
		VectorSet( delta, 0, -1, 0 );
		break;
	case YZ:
		old_x = org[1]; old_y = org[2]; org[0] = 64 * 1024;
		VectorSet( delta, -1, 0, 0 );
		break;
	}

	if ( g_iEditMode == 0 ) {
		if ( g_iActiveTarget < 0 ) {
			GetCurrentCam()->GetCam()->getPositionObj()->selectPointByRay( org[0], org[1], org[2], delta[0], delta[1], delta[2], true );
		}
		else{
			GetCurrentCam()->GetCam()->getActiveTarget( g_iActiveTarget )->selectPointByRay( org[0], org[1], org[2], delta[0], delta[1], delta[2], true );
		}
	}
	else if ( g_iEditMode == 1 ) {
		idVec3 *lastcoord;
		idCameraPosition *camera;

		if ( g_iActiveTarget < 0 ) {
			camera = GetCurrentCam()->GetCam()->getPositionObj();
		}
		else {
			camera = GetCurrentCam()->GetCam()->getActiveTarget( g_iActiveTarget );
		}

		if ( camera->numPoints() ) {
			lastcoord = camera->getPoint( camera->numPoints() - 1 );
			switch ( m_vt ) {
			case XY:
				camera->addPoint( org[0], org[1], lastcoord->z );
				break;
			case XZ:
				camera->addPoint( org[0], lastcoord->y, org[2] );
				break;
			case YZ:
				camera->addPoint( lastcoord->x, org[1], org[2] );
				break;
			}
		}
		else {
			switch ( m_vt ) {
			case XY:
				camera->addPoint( org[0], org[1], 0 );
				break;
			case XZ:
				camera->addPoint( org[0], 0, org[2] );
				break;
			case YZ:
				camera->addPoint( 0, org[1], org[2] );
				break;
			}
		}

		GetCurrentCam()->HasBeenModified();
	}

	g_FuncTable.m_pfnSysUpdateWindows( W_XY_OVERLAY | W_CAMERA );

	return true;

	//return false;
}

bool CListener::OnLButtonUp( guint32 nFlags, gdouble x, gdouble y ){
	SetViewType( g_pXYWndWrapper->GetViewType() );

	m_bLeftMBPressed = false;
	oldValid = false;

	if ( g_iEditMode == 0 ) {
		if ( g_iActiveTarget < 0 ) {
			GetCurrentCam()->GetCam()->getPositionObj()->deselectAll();
		}
		else{
			GetCurrentCam()->GetCam()->getActiveTarget( g_iActiveTarget )->deselectAll();
		}

		g_FuncTable.m_pfnSysUpdateWindows( W_XY_OVERLAY | W_CAMERA );
	}

	return false;
}

bool CListener::OnRButtonDown( guint32 nFlags, gdouble x, gdouble y ){
	SetViewType( g_pXYWndWrapper->GetViewType() );

	m_bRightMBPressed = true;

	return false;
}

bool CListener::OnRButtonUp( guint32 nFlags, gdouble x, gdouble y ){
	SetViewType( g_pXYWndWrapper->GetViewType() );

	m_bRightMBPressed = false;

	return false;
}

bool CListener::OnMButtonDown( guint32 nFlags, gdouble x, gdouble y ){
	SetViewType( g_pXYWndWrapper->GetViewType() );

	m_bMiddleMBPressed = true;

	return false;
}

bool CListener::OnMButtonUp( guint32 nFlags, gdouble x, gdouble y ){
	SetViewType( g_pXYWndWrapper->GetViewType() );

	m_bMiddleMBPressed = false;

	return false;
}
