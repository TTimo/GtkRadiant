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

CRenderer::CRenderer() {

	refCount = 1;

	m_bHooked = FALSE;

	Register();
	Initialize();
}

CRenderer::~CRenderer() {
	if ( m_bHooked ) {
		UnRegister();
	}
}

void CRenderer::Register() {
	g_QglTable.m_pfnHookGL2DWindow( this );
	g_QglTable.m_pfnHookGL3DWindow( this );
	m_bHooked = TRUE;
}

void CRenderer::UnRegister() {
	if ( g_QglTable.m_nSize ) {
		g_QglTable.m_pfnUnHookGL2DWindow( this );
		g_QglTable.m_pfnUnHookGL3DWindow( this );
	}
	m_bHooked = FALSE;
}

void CRenderer::Initialize() {

}

void CRenderer::Draw2D( VIEWTYPE vt ) {

	g_QglTable.m_pfn_qglPushAttrib( GL_ALL_ATTRIB_BITS );
	g_QglTable.m_pfn_qglPushMatrix();

	switch ( vt )
	{
	case XY:
		break;
	case XZ:
		g_QglTable.m_pfn_qglRotatef( 270.0f, 1.0f, 0.0f, 0.0f );
		break;
	case YZ:
		g_QglTable.m_pfn_qglRotatef( 270.0f, 1.0f, 0.0f, 0.0f );
		g_QglTable.m_pfn_qglRotatef( 270.0f, 0.0f, 0.0f, 1.0f );
		break;
	}

	CCamera *cam = firstCam;
	while ( cam ) {
		cam->GetCam()->draw( ( ( Listener && cam == g_pCurrentEditCam ) ? true : false ) );
		cam = cam->GetNext();
	}

	g_QglTable.m_pfn_qglPopMatrix();
	g_QglTable.m_pfn_qglPopAttrib();
}

void CRenderer::Draw3D() {
	// FIXME: really need a mainloop callback from the editor core
	static long start;
	static float cycle;
	static long msecs;
	static long current;

	if ( g_iPreviewRunning ) {
		if ( g_iPreviewRunning == 1 ) {
			start = g_FuncTable.m_pfnQGetTickCount();
			GetCurrentCam()->GetCam()->startCamera( start );
			cycle = GetCurrentCam()->GetCam()->getTotalTime();
			msecs = (long)( cycle * 1000 );
			current = start;
			g_iPreviewRunning = 2;
		}

		if ( current < start + msecs ) {
			float fov;
			vec3_t origin = {0.0f, 0.0f, 0.0f}, dir = {0.0f, 0.0f, 0.0f}, angles;

			GetCurrentCam()->GetCam()->getCameraInfo( current, &origin[0], &dir[0], &fov );
			VectorSet( angles, asin( dir[2] ) * 180 / 3.14159, atan2( dir[1], dir[0] ) * 180 / 3.14159, 0 );
			g_CameraTable.m_pfnSetCamera( origin, angles );
			current = g_FuncTable.m_pfnQGetTickCount();
		}
		else {
			g_iPreviewRunning = 0;
			GetCurrentCam()->GetCam()->setRunning( false );
			g_FuncTable.m_pfnSysUpdateWindows( W_XY_OVERLAY | W_CAMERA );
		}
	}

	g_QglTable.m_pfn_qglPushAttrib( GL_ALL_ATTRIB_BITS );

	CCamera *cam = firstCam;
	while ( cam ) {
		cam->GetCam()->draw( ( ( Listener && cam == g_pCurrentEditCam ) ? true : false ) );
		cam = cam->GetNext();
	}

	if ( g_iPreviewRunning ) {
		int x, y, width, height, i;
		float degInRad;

		g_CameraTable.m_pfnGetCamWindowExtents( &x, &y, &width, &height );

		// setup orthographic projection mode
		g_QglTable.m_pfn_qglMatrixMode( GL_PROJECTION );
		g_QglTable.m_pfn_qglLoadIdentity();
		g_QglTable.m_pfn_qglDisable( GL_DEPTH_TEST );
		g_QglTable.m_pfn_qglOrtho( 0, (float)width, 0, (float)height, -100, 100 );
		g_QglTable.m_pfn_qglMatrixMode( GL_MODELVIEW );

		g_QglTable.m_pfn_qglLoadIdentity();
		g_QglTable.m_pfn_qglColor3f( 1.f, 1.f, 1.f );
		g_QglTable.m_pfn_qglBegin( GL_LINE_LOOP );
		g_QglTable.m_pfn_qglVertex2f( 10, 10 );
		g_QglTable.m_pfn_qglVertex2f( 40, 10 );
		g_QglTable.m_pfn_qglVertex2f( 40, 25 );
		g_QglTable.m_pfn_qglVertex2f( 10, 25 );
		g_QglTable.m_pfn_qglEnd();

		g_QglTable.m_pfn_qglBegin( GL_LINE_LOOP );
		for ( i = 0; i < 360; i += 60 ) {
			degInRad = i * ( 3.14159265358979323846 / 180.f );
			g_QglTable.m_pfn_qglVertex2f( 18 + cos( degInRad ) * 5, 18 + sin( degInRad ) * 5 );
		}
		g_QglTable.m_pfn_qglEnd();

		degInRad = ( 360 - ( ( current - start ) % 360 ) ) * ( 3.14159265358979323846 / 180.f );
		g_QglTable.m_pfn_qglBegin( GL_LINES );
		g_QglTable.m_pfn_qglVertex2f( 18, 18 );
		g_QglTable.m_pfn_qglVertex2f( 18 + cos( degInRad ) * 5, 18 + sin( degInRad ) * 5 );
		g_QglTable.m_pfn_qglVertex2f( 32, 18 );
		g_QglTable.m_pfn_qglVertex2f( 32 + cos( degInRad ) * 5, 18 + sin( degInRad ) * 5 );
		g_QglTable.m_pfn_qglEnd();

		g_QglTable.m_pfn_qglBegin( GL_LINE_LOOP );
		for ( i = 0; i < 360; i += 60 ) {
			degInRad = i * ( 3.14159265358979323846 / 180.f );
			g_QglTable.m_pfn_qglVertex2f( 32 + cos( degInRad ) * 5, 18 + sin( degInRad ) * 5 );
		}
		g_QglTable.m_pfn_qglEnd();

		g_QglTable.m_pfn_qglBegin( GL_LINES );
		g_QglTable.m_pfn_qglVertex2f( 40, 22 );
		g_QglTable.m_pfn_qglVertex2f( 52, 31 );
		g_QglTable.m_pfn_qglVertex2f( 40, 13 );
		g_QglTable.m_pfn_qglVertex2f( 52, 4 );
		g_QglTable.m_pfn_qglEnd();
	}

	g_QglTable.m_pfn_qglPopAttrib();
}
