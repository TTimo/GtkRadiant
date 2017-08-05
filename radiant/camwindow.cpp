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
// Camera Window
//
// Leonardo Zide (leo@lokigames.com)
//

#include "stdafx.h"
#include <gtk/gtk.h>
#include <GL/gl.h>

extern void DrawPathLines();
extern void Select_ShiftTexture( int x, int y );
extern void Select_RotateTexture( int amt );

extern int g_nPatchClickedView;

brush_t* g_pSplitList = NULL;

// =============================================================================
// CamWnd class

CamWnd::CamWnd ()
	: GLWindow( TRUE ), m_XORRectangle( m_pWidget ){
	m_nNumTransBrushes = 0;
	memset( &m_Camera, 0, sizeof( camera_t ) );
	m_pSide_select = NULL;
	m_bClipMode = false;
	m_bFreeMove = false;
	Cam_Init();
}

CamWnd::~CamWnd (){
}

void CamWnd::OnCreate(){
	if ( !MakeCurrent() ) {
		Error( "camwindow: glMakeCurrent failed" );
	}

	// report OpenGL information
	Sys_Printf( "GL_VENDOR: %s\n", qglGetString( GL_VENDOR ) );
	Sys_Printf( "GL_RENDERER: %s\n", qglGetString( GL_RENDERER ) );
	Sys_Printf( "GL_VERSION: %s\n", qglGetString( GL_VERSION ) );
	Sys_Printf( "GL_EXTENSIONS: %s\n", qglGetString( GL_EXTENSIONS ) );

	// Set off texture compression supported
	g_qeglobals.bTextureCompressionSupported = 0;

	// finalize OpenGL init
	// NOTE
	// why is this here? well .. the Gtk objects get constructed when you enter gtk_main
	// and I wanted to have the extensions information in the editor startup console (avoid looking that up in the early console)
	// RIANT
	// I Split this up so as to add support for extension and user-friendly
	// compression format selection.
	// ADD new globals for your new format so as to minimise
	// calls to Sys_QGL_ExtensionSupported
	// NOTE TTimo: I don't really like this approach with globals. Frequent calls to Sys_QGL_ExtensionSupported don't sound like
	//   a problem to me. If there is some caching to be done, then I think it should be inside Sys_QGL_ExtensionSupported
	///////////////////////////////////////////
	// Check for default OpenGL
	if ( Sys_QGL_ExtensionSupported( "GL_ARB_texture_compression" ) ) {
		g_qeglobals.bTextureCompressionSupported = 1;
		g_qeglobals.m_bOpenGLCompressionSupported = 1;
	}

	// INSERT PROPRIETARY EXTENSIONS HERE
	// Check for S3 extensions
	// create a bool global for extension supported
	if ( Sys_QGL_ExtensionSupported( "GL_EXT_texture_compression_s3tc" ) ) {
		g_qeglobals.bTextureCompressionSupported = 1;
		g_qeglobals.m_bS3CompressionSupported = 1;
	}

	g_qeglobals.m_bOpenGLReady = true;

	g_PrefsDlg.UpdateTextureCompression();

#ifdef ATIHACK_812
	g_PrefsDlg.UpdateATIHack();
#endif

	g_qeglobals_gui.d_camera = m_pWidget;
}

void CamWnd::Cam_Init(){
	m_Camera.timing = false;
	m_Camera.origin[0] = 0.f;
	m_Camera.origin[1] = 20.f;
	m_Camera.origin[2] = 46.f;
	m_Camera.color[0] = 0.3f;
	m_Camera.color[1] = 0.3f;
	m_Camera.color[2] = 0.3f;
	m_nCambuttonstate = 0;
}

void CamWnd::OnSize( int cx, int cy ){
	m_Camera.width = cx;
	m_Camera.height = cy;
	gtk_widget_queue_draw( m_pWidget );
}

rectangle_t rectangle_from_area_cam(){
	const float left = MIN( g_qeglobals.d_vAreaTL[0], g_qeglobals.d_vAreaBR[0] );
	const float top = MAX( g_qeglobals.d_vAreaTL[1], g_qeglobals.d_vAreaBR[1] );
	const float right = MAX( g_qeglobals.d_vAreaTL[0], g_qeglobals.d_vAreaBR[0] );
	const float bottom = MIN( g_qeglobals.d_vAreaTL[1], g_qeglobals.d_vAreaBR[1] );
	return rectangle_t( left, bottom, right - left, top - bottom );
}

void update_xor_rectangle( XORRectangle& xor_rectangle ){
	rectangle_t rectangle;
	if ( g_qeglobals.d_select_mode == sel_area ) {
		rectangle = rectangle_from_area_cam();
	}
	xor_rectangle.set( rectangle );
}

void CamWnd::OnMouseMove( guint32 flags, int pointx, int pointy ){
	int height = m_pWidget->allocation.height;
	// NOTE RR2DO2 this hasn't got any use anymore really. It is an old qeradiant feature
	// that can be re-enabled by removing the checks for HasCapture and not shift/ctrl down
	// but the scaling/rotating (unless done with the steps set in the surface inspector
	// dialog) is way too sensitive to be of any use
	if ( HasCapture() && Sys_AltDown() &&
		 !( ( flags & MK_SHIFT ) || ( flags & MK_CONTROL ) ) ) {
		if ( flags & MK_CONTROL ) {
			Select_RotateTexture( pointy - m_ptLastCursorY );
		}
		else
		if ( flags & MK_SHIFT ) {
			Select_ScaleTexture( pointx - m_ptLastCursorX, m_ptLastCursorY - pointy );
		}
		else{
			Select_ShiftTexture( pointx - m_ptLastCursorX, m_ptLastCursorY - pointy );
		}
	}
	else
	{
		Cam_MouseMoved( pointx, height - 1 - pointy, flags );
	}
	m_ptLastCursorX = pointx;
	m_ptLastCursorY = pointy;

	update_xor_rectangle( m_XORRectangle );
}

void CamWnd::OnMouseWheel( bool bUp, int pointx, int pointy ){
	if ( bUp ) {
		VectorMA( m_Camera.origin, g_PrefsDlg.m_nMoveSpeed, m_Camera.forward, m_Camera.origin );
	}
	else{
		VectorMA( m_Camera.origin, -g_PrefsDlg.m_nMoveSpeed, m_Camera.forward, m_Camera.origin );
	}

	int nUpdate = ( g_PrefsDlg.m_bCamXYUpdate ) ? ( W_CAMERA | W_XY ) : ( W_CAMERA );
	Sys_UpdateWindows( nUpdate );
	g_pParentWnd->OnTimer();
}

void CamWnd::OnLButtonDown( guint32 nFlags, int pointx, int pointy ){
	m_ptLastCursorX = pointx;
	m_ptLastCursorY = pointy;
	OriginalMouseDown( nFlags, pointx, pointy );
}

void CamWnd::OnLButtonUp( guint32 nFlags, int pointx, int pointy ){
	OriginalMouseUp( nFlags, pointx, pointy );
}

void CamWnd::OnMButtonDown( guint32 nFlags, int pointx, int pointy ){
	OriginalMouseDown( nFlags, pointx, pointy );
}

void CamWnd::OnMButtonUp( guint32 nFlags, int pointx, int pointy ){
	OriginalMouseUp( nFlags, pointx, pointy );
}

void CamWnd::OnRButtonDown( guint32 nFlags, int pointx, int pointy ){
	OriginalMouseDown( nFlags, pointx, pointy );
}

void CamWnd::OnRButtonUp( guint32 nFlags, int pointx, int pointy ){
	OriginalMouseUp( nFlags, pointx, pointy );
}

void CamWnd::OriginalMouseUp( guint32 nFlags, int pointx, int pointy ){
	int height = m_pWidget->allocation.height;

	if ( g_qeglobals.d_select_mode == sel_facets_on || g_qeglobals.d_select_mode == sel_facets_off ) {
		g_qeglobals.d_select_mode = sel_brush;
	}

	Cam_MouseUp( pointx, height - 1 - pointy, nFlags );
	ReleaseCapture();

	update_xor_rectangle( m_XORRectangle );
}

void CamWnd::OriginalMouseDown( guint32 nFlags, int pointx, int pointy ){
	int height = m_pWidget->allocation.height;

	SetFocus();
	SetCapture();
	Cam_MouseDown( pointx, height - 1 - pointy, nFlags );

	update_xor_rectangle( m_XORRectangle );
}

void CamWnd::Cam_BuildMatrix(){
	float ya;
	float matrix[4][4];
	int i;

	if ( !m_bFreeMove ) {
		ya = m_Camera.angles[1] / 180 * Q_PI;

		// the movement matrix is kept 2d
		m_Camera.forward[0] = cos( ya );
		m_Camera.forward[1] = sin( ya );
		m_Camera.forward[2] = 0;
		m_Camera.right[0] = m_Camera.forward[1];
		m_Camera.right[1] = -m_Camera.forward[0];
	}
	else
	{
		AngleVectors( m_Camera.angles, m_Camera.forward, m_Camera.right, NULL );
		m_Camera.forward[2] = -m_Camera.forward[2];
	}

	memcpy( matrix, m_Camera.projection, sizeof( m4x4_t ) );
	m4x4_multiply_by_m4x4( &matrix[0][0], &m_Camera.modelview[0][0] );

	//qglGetFloatv (GL_PROJECTION_MATRIX, &matrix[0][0]);

	for ( i = 0 ; i < 3 ; i++ )
	{
		m_Camera.vright[i] = matrix[i][0];
		m_Camera.vup[i] = matrix[i][1];
		m_Camera.vpn[i] = matrix[i][2];
	}

	VectorNormalize( m_Camera.vright, m_Camera.vright );
	VectorNormalize( m_Camera.vup, m_Camera.vup );
	VectorNormalize( m_Camera.vpn, m_Camera.vpn );
}

void CamWnd::Cam_ChangeFloor( qboolean up ){
	brush_t   *b;
	float d, bestd, current;
	vec3_t start, dir;

	start[0] = m_Camera.origin[0];
	start[1] = m_Camera.origin[1];
	start[2] = g_MaxWorldCoord;
	dir[0] = dir[1] = 0;
	dir[2] = -1;

	current = g_MaxWorldCoord - ( m_Camera.origin[2] - 48 );
	if ( up ) {
		bestd = 0;
	}
	else{
		bestd = 2 * g_MaxWorldCoord;
	}

	for ( b = active_brushes.next ; b != &active_brushes ; b = b->next )
	{
		if ( !Brush_Ray( start, dir, b, &d ) ) {
			continue;
		}
		if ( up && d < current && d > bestd ) {
			bestd = d;
		}
		if ( !up && d > current && d < bestd ) {
			bestd = d;
		}
	}

	if ( bestd == 0 || bestd == 2 * g_MaxWorldCoord ) {
		return;
	}

	m_Camera.origin[2] += current - bestd;
	Sys_UpdateWindows( W_CAMERA | W_Z_OVERLAY );
}

void CamWnd::Cam_PositionDrag(){
	int x, y;

	Sys_GetCursorPos( &x, &y );
	if ( x != m_ptCursorX || y != m_ptCursorY ) {
		x -= m_ptCursorX;
		VectorMA( m_Camera.origin, x, m_Camera.vright, m_Camera.origin );
		y -= m_ptCursorY;
		m_Camera.origin[2] -= y;
		Sys_SetCursorPos( m_ptCursorX, m_ptCursorY );
		Sys_UpdateWindows( W_CAMERA | W_XY_OVERLAY );
	}
}

void CamWnd::Cam_MouseControl( float dtime ){
	Cam_KeyControl( dtime );

	if ( g_PrefsDlg.m_bCamFreeLook ) {
		int dx, dy;
		gint x, y;

		if ( !m_bFreeMove || m_nCambuttonstate == MK_CONTROL ) {
			return;
		}

		// Update angles
		Sys_GetCursorPos( &m_ptCursorX, &m_ptCursorY );

		dx = m_ptLastCamCursorX - m_ptCursorX;
		dy = m_ptLastCamCursorY - m_ptCursorY;

		gdk_window_get_origin( gtk_widget_get_window( m_pWidget ), &x, &y );

		m_ptLastCamCursorX = x + ( m_Camera.width / 2 );
		m_ptLastCamCursorY = y + ( m_Camera.height / 2 );

		Sys_SetCursorPos( m_ptLastCamCursorX, m_ptLastCamCursorY );

		// Don't use pitch
		if ( !g_PrefsDlg.m_bCamFreeLookStrafe ) {
			if ( g_PrefsDlg.m_bCamInverseMouse ) {
				m_Camera.angles[PITCH] -= dy * dtime * g_PrefsDlg.m_nAngleSpeed;
			}
			else{
				m_Camera.angles[PITCH] += dy * dtime * g_PrefsDlg.m_nAngleSpeed;
			}
		}
		else {
			VectorMA( m_Camera.origin, dy * (float) ( g_PrefsDlg.m_nMoveSpeed / 6.0f ), m_Camera.forward, m_Camera.origin );
		}

		m_Camera.angles[YAW] += dx * dtime * g_PrefsDlg.m_nAngleSpeed;

		if ( m_Camera.angles[PITCH] > 90 ) {
			m_Camera.angles[PITCH] = 90;
		}
		else if ( m_Camera.angles[PITCH] < -90 ) {
			m_Camera.angles[PITCH] = -90;
		}

		if ( m_Camera.angles[YAW] >= 360 ) {
			m_Camera.angles[YAW] = 0;
		}
		else if ( m_Camera.angles[YAW] <= -360 ) {
			m_Camera.angles[YAW] = 0;
		}

		if ( dx || dy || m_Camera.movementflags ) {
			int nUpdate = ( g_PrefsDlg.m_bCamXYUpdate ) ? ( W_CAMERA | W_XY ) : ( W_CAMERA );
			Sys_UpdateWindows( nUpdate );
			g_pParentWnd->OnTimer();
		}
	}
	else
	{
		int xl, xh;
		int yl, yh;
		float xf, yf;

		if ( g_PrefsDlg.m_nMouseButtons == 2 ) {
			if ( m_nCambuttonstate != ( MK_RBUTTON | MK_SHIFT ) ) {
				return;
			}
		}
		else
		{
			if ( m_nCambuttonstate != MK_RBUTTON ) {
				return;
			}
		}

		xf = (float)( m_ptButtonX - m_Camera.width / 2 ) / ( m_Camera.width / 2 );
		yf = (float)( m_ptButtonY - m_Camera.height / 2 ) / ( m_Camera.height / 2 );

		xl = m_Camera.width / 3;
		xh = xl * 2;
		yl = m_Camera.height / 3;
		yh = yl * 2;

		xf *= 1.0 - fabs( yf );
		if ( xf < 0 ) {
			xf += 0.1f;
			if ( xf > 0 ) {
				xf = 0;
			}
		}
		else
		{
			xf -= 0.1f;
			if ( xf < 0 ) {
				xf = 0;
			}
		}

		VectorMA( m_Camera.origin, yf * dtime * g_PrefsDlg.m_nMoveSpeed, m_Camera.forward, m_Camera.origin );
		m_Camera.angles[YAW] += xf * -dtime * g_PrefsDlg.m_nAngleSpeed;

		int nUpdate = ( g_PrefsDlg.m_bCamXYUpdate ) ? ( W_CAMERA | W_XY ) : ( W_CAMERA );
		Sys_UpdateWindows( nUpdate );
		g_pParentWnd->OnTimer();
	}
}

void CamWnd::Cam_KeyControl( float dtime ) {

	// Update angles
	if ( m_Camera.movementflags & MOVE_ROTLEFT ) {
		m_Camera.angles[YAW] += 15 * dtime * g_PrefsDlg.m_nAngleSpeed;
	}
	if ( m_Camera.movementflags & MOVE_ROTRIGHT ) {
		m_Camera.angles[YAW] -= 15 * dtime * g_PrefsDlg.m_nAngleSpeed;
	}

	// Update position
	if ( m_Camera.movementflags & MOVE_FORWARD ) {
		VectorMA( m_Camera.origin, dtime * g_PrefsDlg.m_nMoveSpeed, m_Camera.forward, m_Camera.origin );
	}
	if ( m_Camera.movementflags & MOVE_BACK ) {
		VectorMA( m_Camera.origin, -dtime * g_PrefsDlg.m_nMoveSpeed, m_Camera.forward, m_Camera.origin );
	}
	if ( m_Camera.movementflags & MOVE_STRAFELEFT ) {
		VectorMA( m_Camera.origin, -dtime * g_PrefsDlg.m_nMoveSpeed, m_Camera.right, m_Camera.origin );
	}
	if ( m_Camera.movementflags & MOVE_STRAFERIGHT ) {
		VectorMA( m_Camera.origin, dtime * g_PrefsDlg.m_nMoveSpeed, m_Camera.right, m_Camera.origin );
	}

	// Save a screen update (when m_bFreeMove is enabled, mousecontrol does the update)
	if ( !m_bFreeMove && m_Camera.movementflags ) {
		int nUpdate = ( g_PrefsDlg.m_bCamXYUpdate ) ? ( W_CAMERA | W_XY ) : ( W_CAMERA );
		Sys_UpdateWindows( nUpdate );
		g_pParentWnd->OnTimer();
	}
}

// NOTE TTimo if there's an OS-level focus out of the application
//   then we can release the camera cursor grab
static gint camwindow_focusout( GtkWidget* widget, GdkEventKey* event, gpointer data ){
	g_pParentWnd->GetCamWnd()->ToggleFreeMove();
	return FALSE;
}

void CamWnd::ToggleFreeMove(){
	GdkWindow *window;
	GtkWidget *widget;

	m_bFreeMove = !m_bFreeMove;
	Camera()->movementflags = 0;
	m_ptLastCamCursorX = m_ptCursorX;
	m_ptLastCamCursorY = m_ptCursorY;

	if ( g_pParentWnd->CurrentStyle() == MainFrame::eFloating ) {
		widget = g_pParentWnd->GetCamWnd()->m_pParent;
		window = gtk_widget_get_window( widget );
	}
	else
	{
		widget = g_pParentWnd->m_pWidget;
		window = gtk_widget_get_window( widget );
	}

	if ( m_bFreeMove ) {
		GdkDisplay *display;
		GdkCursor *cursor;

		SetFocus();
		SetCapture();

		display = gdk_window_get_display( window );
		cursor = gdk_cursor_new_for_display( display, GDK_BLANK_CURSOR );
		gdk_window_set_cursor( window, cursor );

		// RR2DO2: FIXME why does this only work the 2nd and
		// further times the event is called? (floating windows
		// mode seems to work fine though...)
		m_FocusOutHandler_id = g_signal_connect( G_OBJECT( widget ), "focus-out-event",
												   G_CALLBACK( camwindow_focusout ), g_pParentWnd );

		{
			GdkEventMask mask = (GdkEventMask)( GDK_POINTER_MOTION_MASK
												| GDK_POINTER_MOTION_HINT_MASK
												| GDK_BUTTON_MOTION_MASK
												| GDK_BUTTON1_MOTION_MASK
												| GDK_BUTTON2_MOTION_MASK
												| GDK_BUTTON3_MOTION_MASK
												| GDK_BUTTON_PRESS_MASK
												| GDK_BUTTON_RELEASE_MASK );

			gdk_pointer_grab( gtk_widget_get_window( widget ), TRUE, mask, gtk_widget_get_window( widget ), NULL, GDK_CURRENT_TIME );
		}
#if GTK_CHECK_VERSION( 3, 0, 0 )
		g_object_unref( cursor );
#else
		gdk_cursor_unref( cursor );
#endif
	}
	else
	{
		GdkDisplay *display;
		GdkCursor *cursor;

		gdk_pointer_ungrab( GDK_CURRENT_TIME );

		g_signal_handler_disconnect( G_OBJECT( widget ), m_FocusOutHandler_id );

		display = gdk_window_get_display( window );
		cursor = gdk_cursor_new_for_display( display, GDK_LEFT_PTR );

		gdk_window_set_cursor( window, cursor );
#if GTK_CHECK_VERSION( 3, 0, 0 )
		g_object_unref( cursor );
#else
		gdk_cursor_unref( cursor );
#endif

		ReleaseCapture();
	}

	int nUpdate = ( g_PrefsDlg.m_bCamXYUpdate ) ? ( W_CAMERA | W_XY ) : ( W_CAMERA );
	Sys_UpdateWindows( nUpdate );
	g_pParentWnd->OnTimer();
}

void CamWnd::Cam_MouseDown( int x, int y, int buttons ){
	vec3_t dir;
	float f, r, u;
	int i;


	//
	// calc ray direction
	//
	u = (float)( y - ( m_Camera.height * .5f ) ) / ( m_Camera.width * .5f );
	r = (float)( x - ( m_Camera.width * .5f ) ) / ( m_Camera.width * .5f );
	f = 1;

	for ( i = 0 ; i < 3 ; i++ )
		dir[i] = m_Camera.vpn[i] * f + m_Camera.vright[i] * r + m_Camera.vup[i] * u;
	VectorNormalize( dir, dir );

	Sys_GetCursorPos( &m_ptCursorX, &m_ptCursorY );

	m_nCambuttonstate = buttons;
	m_ptButtonX = x;
	m_ptButtonY = y;

	// LBUTTON = manipulate selection
	// shift-LBUTTON = select
	// middle button = grab texture
	// ctrl-middle button = set entire brush to texture
	// ctrl-shift-middle button = set single face to texture
	int nMouseButton = g_PrefsDlg.m_nMouseButtons == 2 ? MK_RBUTTON : MK_MBUTTON;
	if ( ( buttons == MK_LBUTTON )
		 || ( buttons == ( MK_LBUTTON | MK_SHIFT ) )
		 || ( buttons == ( MK_LBUTTON | MK_CONTROL ) )
		 || ( buttons == ( MK_LBUTTON | MK_CONTROL | MK_SHIFT ) )
		 || ( buttons == nMouseButton )
		 || ( buttons == ( nMouseButton | MK_SHIFT ) )
		 || ( buttons == ( nMouseButton | MK_CONTROL ) )
		 || ( buttons == ( nMouseButton | MK_SHIFT | MK_CONTROL ) ) ) {
		if ( g_PrefsDlg.m_nMouseButtons == 2 && ( buttons == ( MK_RBUTTON | MK_SHIFT ) ) ) {
			if ( g_PrefsDlg.m_bCamFreeLook ) {
				ToggleFreeMove();
			}
			else{
				Cam_MouseControl( 0.1f );
			}
		}
		else
		{
			// something global needs to track which window is responsible for stuff
			Patch_SetView( W_CAMERA );
			Drag_Begin( x, y, buttons, m_Camera.vright, m_Camera.vup, m_Camera.origin, dir, true );
		}
		return;
	}

	if ( buttons == MK_RBUTTON ) {
		if ( g_PrefsDlg.m_bCamFreeLook ) {
			ToggleFreeMove();
		}
		else{
			Cam_MouseControl( 0.1f );
		}
		return;
	}
}

void CamWnd::Cam_MouseUp( int x, int y, int buttons ){
	m_nCambuttonstate = 0;
	Drag_MouseUp( buttons );
}

void CamWnd::Cam_MouseMoved( int x, int y, int buttons ){
	m_nCambuttonstate = buttons;
	if ( !buttons ) {
		return;
	}

	if ( g_PrefsDlg.m_nCamDragMultiSelect ) {
		if ( g_qeglobals.d_select_mode == sel_brush_on  || g_qeglobals.d_select_mode == sel_brush_off ) {
			bool bDoDragMultiSelect = FALSE;

			if ( g_PrefsDlg.m_nCamDragMultiSelect == 1 && buttons == ( MK_LBUTTON | MK_SHIFT ) ) {
				bDoDragMultiSelect = TRUE;
			}
			else if ( g_PrefsDlg.m_nCamDragMultiSelect == 2 && buttons == ( MK_LBUTTON | MK_CONTROL ) && Sys_AltDown() ) {
				bDoDragMultiSelect = TRUE;
			}

			if ( bDoDragMultiSelect ) {
				vec3_t dir;
				float f, r, u;
				int i;

				//
				// calc ray direction
				//
				u = (float)( y - ( m_Camera.height * .5f ) ) / ( m_Camera.width * .5f );
				r = (float)( x - ( m_Camera.width * .5f ) ) / ( m_Camera.width * .5f );
				f = 1;

				for ( i = 0 ; i < 3 ; i++ )
					dir[i] = m_Camera.vpn[i] * f + m_Camera.vright[i] * r + m_Camera.vup[i] * u;
				VectorNormalize( dir,dir );

				switch ( g_qeglobals.d_select_mode )
				{
				case sel_brush_on:
					Select_Ray( m_Camera.origin, dir, ( SF_DRAG_ON | SF_CAMERA ) );
					break;

				case sel_brush_off:
					Select_Ray( m_Camera.origin, dir, ( SF_DRAG_OFF | SF_CAMERA ) );
					break;

				default:
					break;
				}
				return;
			}
		}
		else if ( g_qeglobals.d_select_mode == sel_facets_on || g_qeglobals.d_select_mode == sel_facets_off ) {
			if ( buttons == ( MK_LBUTTON | MK_CONTROL | MK_SHIFT ) ) {
				vec3_t dir;
				float f, r, u;
				int i;

				//
				// calc ray direction
				//
				u = (float)( y - ( m_Camera.height * .5f ) ) / ( m_Camera.width * .5f );
				r = (float)( x - ( m_Camera.width * .5f ) ) / ( m_Camera.width * .5f );
				f = 1;

				for ( i = 0 ; i < 3 ; i++ )
					dir[i] = m_Camera.vpn[i] * f + m_Camera.vright[i] * r + m_Camera.vup[i] * u;
				VectorNormalize( dir,dir );

				switch ( g_qeglobals.d_select_mode )
				{
				case sel_facets_on:
					Select_Ray( m_Camera.origin, dir, ( SF_SINGLEFACE | SF_DRAG_ON | SF_CAMERA ) );
					break;

				case sel_facets_off:
					Select_Ray( m_Camera.origin, dir, ( SF_SINGLEFACE | SF_DRAG_OFF | SF_CAMERA ) );
					break;

				default:
					break;
				}
				return;
			}
		}
	}

	m_ptButtonX = x;
	m_ptButtonY = y;

	if ( ( m_bFreeMove && ( buttons & MK_CONTROL ) && !( buttons & MK_SHIFT ) ) || ( !m_bFreeMove && ( buttons == ( MK_RBUTTON | MK_CONTROL ) ) ) ) {
		Cam_PositionDrag();
		Sys_UpdateWindows( W_XY | W_CAMERA | W_Z );
		return;
	}

	Sys_GetCursorPos( &m_ptCursorX, &m_ptCursorY );

	if ( buttons & ( MK_LBUTTON | MK_MBUTTON ) ) {
		Drag_MouseMoved( x, y, buttons );
		if ( g_qeglobals.d_select_mode != sel_area ) {
			Sys_UpdateWindows( W_XY | W_CAMERA | W_Z );
		}
	}
}

void CamWnd::InitCull(){
	int i;

	VectorSubtract( m_Camera.vpn, m_Camera.vright, m_vCull1 );
	VectorAdd( m_Camera.vpn, m_Camera.vright, m_vCull2 );

	for ( i = 0 ; i < 3 ; i++ )
	{
		if ( m_vCull1[i] > 0 ) {
			m_nCullv1[i] = 3 + i;
		}
		else{
			m_nCullv1[i] = i;
		}
		if ( m_vCull2[i] > 0 ) {
			m_nCullv2[i] = 3 + i;
		}
		else{
			m_nCullv2[i] = i;
		}
	}
}

qboolean CamWnd::CullBrush( brush_t *b ){
	int i;
	vec3_t point;
	float d;

	if ( g_PrefsDlg.m_bCubicClipping ) {
		float fLevel = g_PrefsDlg.m_nCubicScale * 64;

		point[0] = m_Camera.origin[0] - fLevel;
		point[1] = m_Camera.origin[1] - fLevel;
		point[2] = m_Camera.origin[2] - fLevel;

		for ( i = 0; i < 3; i++ )
			if ( b->mins[i] < point[i] && b->maxs[i] < point[i] ) {
				return true;
			}

		point[0] = m_Camera.origin[0] + fLevel;
		point[1] = m_Camera.origin[1] + fLevel;
		point[2] = m_Camera.origin[2] + fLevel;

		for ( i = 0; i < 3; i++ )
			if ( b->mins[i] > point[i] && b->maxs[i] > point[i] ) {
				return true;
			}
	}

	for ( i = 0 ; i < 3 ; i++ )
		point[i] = b->mins[m_nCullv1[i]] - m_Camera.origin[i];

	d = DotProduct( point, m_vCull1 );
	if ( d < -1 ) {
		return true;
	}

	for ( i = 0 ; i < 3 ; i++ )
		point[i] = b->mins[m_nCullv2[i]] - m_Camera.origin[i];

	d = DotProduct( point, m_vCull2 );
	if ( d < -1 ) {
		return true;
	}

	return false;
}

// project a 3D point onto the camera space
// we use the GL viewing matrixes
// this is the implementation of a glu function (I realized that afterwards): gluProject
void CamWnd::ProjectCamera( const vec3_t A, vec_t B[2] ){

	vec_t P1[4],P2[4],P3[4];
	VectorCopy( A,P1 ); P1[3] = 1;

	GLMatMul( m_Camera.modelview, P1, P2 );
	GLMatMul( m_Camera.projection, P2, P3 );

	// we ASSUME that the view port is 0 0 m_Camera.width m_Camera.height (you can check in Cam_Draw)
	B[0] = (float)m_Camera.width  * ( P3[0] + 1.0 ) / 2.0;
	B[1] = (float)m_Camera.height * ( P3[1] + 1.0 ) / 2.0;

}

// vec defines a direction in geometric space and P an origin point
// the user is interacting from the camera view
// (for example with texture adjustment shortcuts)
// and intuitively if he hits left / right / up / down
//   what happens in geometric space should match the left/right/up/down move in camera space
// axis = 0: vec is along left/right
// axis = 1: vec is along up/down
// sgn = +1: same directions
// sgn = -1: opposite directions
// Implementation:
//   typical use case is giving a face center and a normalized vector
//   1) compute start and endpoint, project them in camera view, get the direction
//     depending on the situation, we might bump into precision issues with that
//   2) possible to compute the projected direction independently?
//     this solution would be better but right now I don't see how to do it..
void CamWnd::MatchViewAxes( const vec3_t P, const vec3_t vec, int &axis, float &sgn ){

	vec_t A[2],B[2],V[2];
	ProjectCamera( P,A );
	vec3_t Q;
	VectorAdd( P,vec,Q );
	ProjectCamera( Q,B );
	// V is the vector projected in camera space
	V[0] = B[0] - A[0];
	V[1] = B[1] - A[1];
	if ( fabs( V[0] ) > fabs( V[1] ) ) {
		// best match is against right
		axis = 0;
		if ( V[0] > 0 ) {
			sgn = +1;
		}
		else{
			sgn = -1;
		}
	}
	else
	{
		// best match is against up
		axis = 1;
		if ( V[1] > 0 ) {
			sgn = +1;
		}
		else{
			sgn = -1;
		}
	}
}

#if 0
void CamWnd::DrawLightRadius( brush_t* pBrush ){
	// if lighting
	int nRadius = Brush_LightRadius( pBrush );
	if ( nRadius > 0 ) {
		Brush_SetLightColor( pBrush );
		qglEnable( GL_BLEND );
		qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		qglDisable( GL_TEXTURE_2D );

		qglEnable( GL_TEXTURE_2D );
		qglDisable( GL_BLEND );
		qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	}
}
#endif

extern void DrawPatchMesh( patchMesh_t *pm );
extern void DrawPatchControls( patchMesh_t *pm );
extern void Brush_DrawFacingAngle( brush_t *b, entity_t *e );
extern void Brush_DrawModel( brush_t *b, bool bTextured = false );
extern void DrawModelOrigin( brush_t *b );
extern void DrawModelBBox( brush_t *b );

void CamWnd::Cam_DrawBrush( brush_t *b, int mode ){
	int nGLState = m_Camera.draw_glstate;
	int nModelMode = g_PrefsDlg.m_nEntityShowState;

	GLfloat material[4], identity[4];
	VectorSet( identity, 0.8f, 0.8f, 0.8f );
	IShader *pShader;

	// lights
	if ( b->owner->eclass->fixedsize && b->owner->eclass->nShowFlags & ECLASS_LIGHT && g_PrefsDlg.m_bNewLightDraw ) {
		switch ( mode )
		{
		case DRAW_SOLID:
			VectorCopy( b->owner->color, material );
			VectorScale( material, 0.8f, material );
			material[3] = 1.0f;

			qglColor4fv( material );

			if ( g_PrefsDlg.m_bNewLightDraw ) {
				DrawLight( b->owner, nGLState, ( IsBrushSelected( b ) ) ? g_PrefsDlg.m_nLightRadiuses : 0, 0 );
			}

			break;
		}
	}

	// models
	else if ( b->owner->eclass->fixedsize && b->owner->model.pRender
			  && !( !IsBrushSelected( b ) && ( nModelMode & ENTITY_SELECTED_ONLY ) ) ) {
		switch ( mode )
		{
		case DRAW_TEXTURED:
			if ( !( nModelMode & ENTITY_WIREFRAME ) && nModelMode != ENTITY_BOX ) {
				VectorCopy( b->owner->eclass->color, material );
				material[3] = identity[3] = 1.0f;

				qglEnable( GL_CULL_FACE );

				if ( !( nGLState & DRAW_GL_TEXTURE_2D ) ) {
					qglColor4fv( material );
				}
				else { 
					qglColor4fv( identity );
				}

				if ( nGLState & DRAW_GL_LIGHTING ) {
					qglShadeModel( GL_SMOOTH );
				}

				// Check model validity
				// If the model is NULL or invalid, draw a box instead
				bool isModelValid = b->owner->model.pRender->IsModelNotNull();
				if ( isModelValid ) {
					b->owner->model.pRender->Draw( nGLState, DRAW_RF_CAM );
				}
				else {
					qglColor4fv( material );
					aabb_draw( b->owner->model.pRender->GetAABB(), DRAW_GL_WIRE );
				}
			}
			break;
		case DRAW_WIRE:
			VectorCopy( b->owner->eclass->color, material );
			material[3] = 1.0f;
			qglColor4fv( material );

			// model view mode "wireframe" or "selected wire"
			if ( nModelMode & ENTITY_WIREFRAME ) {
				b->owner->model.pRender->Draw( nGLState, DRAW_RF_CAM );
			}

			// model view mode "skinned and boxed"
			if ( !( b->owner->eclass->nShowFlags & ECLASS_MISCMODEL ) ) {
				qglColor4fv( material );
				aabb_draw( b->owner->model.pRender->GetAABB(), DRAW_GL_WIRE );
			}
			else if ( nModelMode & ENTITY_BOXED ) {
				aabb_draw( b->owner->model.pRender->GetAABB(), DRAW_GL_WIRE );
			}
/*
      if(!(nModelMode & ENTITY_BOXED) && b->owner->eclass->nShowFlags & ECLASS_MISCMODEL)
              DrawModelOrigin(b);
 */
		}
	}

	// patches
	else if ( b->patchBrush ) {
		bool bTrans = ( b->pPatch->pShader->getTrans() < 1.0f );
		switch ( mode )
		{
		case DRAW_TEXTURED:
			if ( !g_bPatchWireFrame && ( ( nGLState & DRAW_GL_BLEND && bTrans ) || ( !( nGLState & DRAW_GL_BLEND ) && !bTrans ) ) ) {
				qglDisable( GL_CULL_FACE );

				pShader = b->pPatch->pShader;
				VectorCopy( pShader->getTexture()->color, material );
				material[3] = identity[3] = pShader->getTrans();

				if ( nGLState & DRAW_GL_TEXTURE_2D ) {
					qglColor4fv( identity );
					qglBindTexture( GL_TEXTURE_2D, pShader->getTexture()->texture_number );
				}
				else{
					qglColor4fv( material );
				}
				if ( nGLState & DRAW_GL_LIGHTING ) {
					qglShadeModel( GL_SMOOTH );
				}

				DrawPatchMesh( b->pPatch );
			}
			break;
		case DRAW_WIRE:
			if ( g_bPatchWireFrame ) {
				VectorCopy( b->pPatch->pShader->getTexture()->color, material );
				material[3] = 1.0;
				qglColor4fv( material );
				DrawPatchMesh( b->pPatch );
			}
			if ( b->pPatch->bSelected && ( g_qeglobals.d_select_mode == sel_curvepoint
										   || g_qeglobals.d_select_mode == sel_area
										   || g_bPatchBendMode ) ) {
				DrawPatchControls( b->pPatch );
			}
		}
	}

	// brushes
	else if ( b->owner->eclass->fixedsize ) {
		switch ( mode )
		{
		case DRAW_SOLID:
			VectorCopy( b->owner->eclass->color, material );
			VectorScale( material, 0.8f, material );
			material[3] = 1.0f;
			qglColor4fv( material );

			qglEnable( GL_CULL_FACE );
			qglShadeModel( GL_FLAT );
			Brush_Draw( b );
			break;
		case DRAW_WIRE:
			if ( ( g_qeglobals.d_savedinfo.include & INCLUDE_ANGLES )
				 && ( b->owner->eclass->nShowFlags & ECLASS_ANGLE ) ) {
				Brush_DrawFacingAngle( b, b->owner );
			}
		}
	}

	// brushes
	else
	{
		switch ( mode )
		{
		case DRAW_TEXTURED:
			qglEnable( GL_CULL_FACE );
			qglShadeModel( GL_FLAT );
			Brush_Draw( b );
		}
	}
}

void CamWnd::Cam_DrawBrushes( int mode ){
	brush_t *b;
	brush_t *pList = ( g_bClipMode && g_pSplitList ) ? g_pSplitList : &selected_brushes;

	for ( b = active_brushes.next; b != &active_brushes; b = b->next )
		if ( !b->bFiltered && !b->bCamCulled ) {
			Cam_DrawBrush( b, mode );
		}
	for ( b = pList->next; b != pList; b = b->next )
		if ( !b->bFiltered && !b->bCamCulled ) {
			Cam_DrawBrush( b, mode );
		}
}

void CamWnd::Cam_DrawStuff(){
	GLfloat identity[4];
	VectorSet( identity, 0.8f, 0.8f, 0.8f );
	brush_t *b;

	for ( b = active_brushes.next; b != &active_brushes; b = b->next )
		b->bCamCulled = CullBrush( b );

	for ( b = selected_brushes.next; b != &selected_brushes; b = b->next )
		b->bCamCulled = CullBrush( b );

	switch ( m_Camera.draw_mode )
	{
	case cd_wire:
		qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		qglDisable( GL_TEXTURE_2D );
		qglDisable( GL_TEXTURE_1D );
		qglDisable( GL_BLEND );
		qglEnable( GL_DEPTH_TEST );
		qglEnableClientState( GL_VERTEX_ARRAY );
		qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
		qglShadeModel( GL_FLAT );
		if ( g_PrefsDlg.m_bGLLighting ) {
			qglDisable( GL_LIGHTING );
			qglDisable( GL_COLOR_MATERIAL );
			qglDisableClientState( GL_NORMAL_ARRAY );
		}
		m_Camera.draw_glstate = DRAW_GL_WIRE;
		break;

	case cd_solid:
		qglCullFace( GL_FRONT );
		qglEnable( GL_CULL_FACE );
		qglShadeModel( GL_FLAT );
		qglPolygonMode( GL_FRONT, GL_LINE );
		qglPolygonMode( GL_BACK, GL_FILL );
		qglDisable( GL_TEXTURE_2D );
		qglDisable( GL_BLEND );
		qglEnable( GL_DEPTH_TEST );
		qglEnableClientState( GL_VERTEX_ARRAY );
		qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
		qglPolygonOffset( -1.0, 2 );
		if ( g_PrefsDlg.m_bGLLighting ) {
			qglEnable( GL_LIGHTING );
			qglEnable( GL_COLOR_MATERIAL );
//    qglEnable(GL_RESCALE_NORMAL);
			qglEnableClientState( GL_NORMAL_ARRAY );
		}
		m_Camera.draw_glstate = DRAW_GL_SOLID;
		break;

	case cd_texture:
		qglCullFace( GL_FRONT );
		qglEnable( GL_CULL_FACE );
		qglShadeModel( GL_FLAT );
		qglPolygonMode( GL_FRONT, GL_LINE );
		qglPolygonMode( GL_BACK, GL_FILL );
		qglEnable( GL_TEXTURE_2D );
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		qglDisable( GL_BLEND );
		qglEnable( GL_DEPTH_TEST );
		qglEnableClientState( GL_VERTEX_ARRAY );
		qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
		if ( g_PrefsDlg.m_bGLLighting ) {
			qglEnable( GL_LIGHTING );
			qglDisable( GL_COLOR_MATERIAL );
			qglMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, identity );
			qglEnableClientState( GL_NORMAL_ARRAY );
//    qglEnable(GL_RESCALE_NORMAL);
		}
		qglPolygonOffset( -1.0, 2 );
		m_Camera.draw_glstate = DRAW_GL_TEXTURED;
		break;

	default: Sys_Printf( "CamWnd::Cam_DrawStuff:invalid render mode\n" );
	}

	Cam_DrawBrushes( DRAW_TEXTURED );

	// setup for solid stuff
	switch ( m_Camera.draw_mode )
	{
	case cd_texture:
		qglDisable( GL_TEXTURE_2D );
		m_Camera.draw_glstate &= ~DRAW_GL_TEXTURE_2D;
		if ( g_PrefsDlg.m_bGLLighting ) {
			qglEnable( GL_COLOR_MATERIAL );
		}
		qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
		break;
	case cd_solid:
		break;
	case cd_wire:
		break;
	default: Sys_Printf( "CamWnd::Cam_DrawStuff:invalid render mode\n" );
	}

	qglEnable( GL_CULL_FACE );
	qglShadeModel( GL_FLAT );
	Cam_DrawBrushes( DRAW_SOLID );

	// setup for wireframe stuff
	switch ( m_Camera.draw_mode )
	{
	case cd_texture:
		if ( g_PrefsDlg.m_bGLLighting ) {
			qglDisable( GL_LIGHTING );
			qglDisable( GL_COLOR_MATERIAL );
			qglDisableClientState( GL_NORMAL_ARRAY );
//      qglDisable(GL_RESCALE_NORMAL);
		}
		qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		break;
	case cd_solid:
		if ( g_PrefsDlg.m_bGLLighting ) {
			qglDisable( GL_LIGHTING );
			qglDisable( GL_COLOR_MATERIAL );
			qglDisableClientState( GL_NORMAL_ARRAY );
//      qglDisable(GL_RESCALE_NORMAL);
		}
		qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		break;
	case cd_wire:
		break;
	default: Sys_Printf( "CamWnd::Cam_DrawStuff:invalid render mode\n" );
	}

	qglDisable( GL_CULL_FACE );
	Cam_DrawBrushes( DRAW_WIRE );

	// setup for transparent texture stuff
	switch ( m_Camera.draw_mode )
	{
	case cd_texture:
		qglPolygonMode( GL_FRONT, GL_LINE );
		qglPolygonMode( GL_BACK, GL_FILL );
		if ( g_PrefsDlg.m_bGLLighting ) {
			qglEnable( GL_COLOR_MATERIAL );
			qglEnableClientState( GL_NORMAL_ARRAY );
			qglMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, identity );
		}
		qglEnable( GL_TEXTURE_2D );
		qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
		m_Camera.draw_glstate = DRAW_GL_TEXTURED;
		break;
	case cd_solid:
		qglPolygonMode( GL_FRONT, GL_LINE );
		qglPolygonMode( GL_BACK, GL_FILL );
		if ( g_PrefsDlg.m_bGLLighting ) {
			qglEnable( GL_LIGHTING );
			qglEnable( GL_COLOR_MATERIAL );
			qglEnableClientState( GL_NORMAL_ARRAY );
//      qglEnable(GL_RESCALE_NORMAL);
		}
		m_Camera.draw_glstate = DRAW_GL_SOLID;
		break;
	case cd_wire:
		m_Camera.draw_glstate = DRAW_GL_WIRE;
		break;
	default: Sys_Printf( "CamWnd::Cam_DrawStuff:invalid render mode\n" );
	}


	qglEnable( GL_BLEND );
	m_Camera.draw_glstate |= DRAW_GL_BLEND;
	qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	// FIXME: some .TGA are buggy, have a completely empty alpha channel
	// if such brushes are rendered in this loop they would be totally transparent with GL_MODULATE
	// so I decided using GL_DECAL instead
	// if an empty-alpha-channel or nearly-empty texture is used. It will be blank-transparent.
	// this could get better if you can get qglTexEnviv (GL_TEXTURE_ENV, to work .. patches are welcome
	// Arnout: empty alpha channels are now always filled with data. Don't set this anymore (would cause problems with qer_alphafunc too)
//  qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	Cam_DrawBrushes( DRAW_TEXTURED );

//  qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	qglDisable( GL_BLEND );

	// setup for wireframe stuff
	switch ( m_Camera.draw_mode )
	{
	case cd_texture:
		if ( g_PrefsDlg.m_bGLLighting ) {
			qglDisable( GL_COLOR_MATERIAL );
			qglDisable( GL_LIGHTING );
//      qglDisable(GL_RESCALE_NORMAL);
		}
		break;
	case cd_solid:
		if ( g_PrefsDlg.m_bGLLighting ) {
			qglDisable( GL_COLOR_MATERIAL );
			qglDisable( GL_LIGHTING );
//      qglDisable(GL_RESCALE_NORMAL);
		}
		break;
	case cd_wire:
		break;
	default: Sys_Printf( "CamWnd::Cam_DrawStuff:invalid render mode\n" );
	}

}

/*
   ==============
   Cam_Draw
   ==============
 */

void QueueClear();
void QueueDraw();

void CamWnd::Cam_Draw(){
	brush_t   *brush;
	face_t    *face;
	float screenaspect;
	float yfov;
	double start = 0.0, end;
	int i;

	if ( !active_brushes.next ) {
		return; // not valid yet

	}
	if ( m_Camera.timing ) {
		start = Sys_DoubleTime();
	}

	//
	// clear
	//
	QE_CheckOpenGLForErrors();

	qglViewport( 0, 0, m_Camera.width, m_Camera.height );
	qglClearColor( g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][0],
				   g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][1],
				   g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][2], 0 );
	qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	//
	// set up viewpoint
	//


	qglMatrixMode( GL_PROJECTION );
	qglLoadIdentity();

	screenaspect = (float)m_Camera.width / m_Camera.height;
	yfov = 2 * atan( (float)m_Camera.height / m_Camera.width ) * 180 / Q_PI;
	qgluPerspective( yfov,  screenaspect,  8,  32768 );

	// we're too lazy to calc projection matrix ourselves!!!
	qglGetFloatv( GL_PROJECTION_MATRIX, &m_Camera.projection[0][0] );

	vec3_t vec;

	m4x4_identity( &m_Camera.modelview[0][0] );
	VectorSet( vec, -90, 0, 0 );
	m4x4_rotate_by_vec3( &m_Camera.modelview[0][0], vec, eXYZ );
	VectorSet( vec, 0, 0, 90 );
	m4x4_rotate_by_vec3( &m_Camera.modelview[0][0], vec, eXYZ );
	VectorSet( vec, 0, m_Camera.angles[0], 0 );
	m4x4_rotate_by_vec3( &m_Camera.modelview[0][0], vec, eXYZ );
	VectorSet( vec, 0, 0, -m_Camera.angles[1] );
	m4x4_rotate_by_vec3( &m_Camera.modelview[0][0], vec, eXYZ );
	VectorSet( vec, -m_Camera.origin[0],  -m_Camera.origin[1],  -m_Camera.origin[2] );
	m4x4_translate_by_vec3( &m_Camera.modelview[0][0], vec );

	Cam_BuildMatrix();

	qglMatrixMode( GL_MODELVIEW );
	qglLoadIdentity();

	qglMultMatrixf( &m_Camera.modelview[0][0] );

	// grab the GL_PROJECTION and GL_MODELVIEW matrixes
	//   used in GetRelativeAxes
	//qglGetFloatv (GL_PROJECTION_MATRIX, &m_Camera.projection[0][0]);
	//qglGetFloatv (GL_MODELVIEW_MATRIX, &m_Camera.modelview[0][0]);

#if 0
	// TTimo: this is not used, just for verification (0, 0, m_Camera.width, m_Camera.height)
	GLint viewprt[4];
	qglGetIntegerv( GL_VIEWPORT, viewprt );
#endif

	if ( g_PrefsDlg.m_bGLLighting ) {
		GLfloat inverse_cam_dir[4], ambient[4], diffuse[4]; //, material[4];

		ambient[0] = ambient[1] = ambient[2] = 0.6f;
		ambient[3] = 1.0f;
		diffuse[0] = diffuse[1] = diffuse[2] = 0.4f;
		diffuse[3] = 1.0f;
		//material[0] = material[1] = material[2] = 0.8f;
		//material[3] = 1.0f;

		vec3_t vCam, vRotate;
		VectorSet( vCam, -1, 0, 0 ); //default cam pos
		VectorSet( vRotate, 0, -m_Camera.angles[0], 0 );
		VectorRotate( vCam, vRotate, vCam );
		VectorSet( vRotate, 0, 0, m_Camera.angles[1] );
		VectorRotate( vCam, vRotate, vCam );

		inverse_cam_dir[0] = vCam[0];
		inverse_cam_dir[1] = vCam[1];
		inverse_cam_dir[2] = vCam[2];
		inverse_cam_dir[3] = 0;

		qglColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );

		qglLightfv( GL_LIGHT0, GL_POSITION, inverse_cam_dir );

		qglLightfv( GL_LIGHT0, GL_AMBIENT, ambient );
		qglLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );

		qglEnable( GL_LIGHT0 );
	}

	InitCull();

	//
	// draw stuff
	//

	Cam_DrawStuff();

	qglEnableClientState( GL_VERTEX_ARRAY );
	qglDisableClientState( GL_NORMAL_ARRAY );
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
	qglDisable( GL_TEXTURE_2D );
	qglDisable( GL_LIGHTING );
	qglDisable( GL_COLOR_MATERIAL );

	qglEnable( GL_CULL_FACE );

	brush_t* pList = ( g_bClipMode && g_pSplitList ) ? g_pSplitList : &selected_brushes;

	if ( g_qeglobals.d_savedinfo.iSelectedOutlinesStyle & OUTLINE_BSEL ) {
		qglColor4f( g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES3D][0], g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES3D][1], g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES3D][2], 0.3f );
		qglEnable( GL_BLEND );
		qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		qglDepthFunc( GL_LEQUAL );
		for ( brush = pList->next ; brush != pList ; brush = brush->next )
		{
			if ( brush->bCamCulled ) { // draw selected faces of filtered brushes to remind that there is a selection
				continue;
			}

			if ( brush->patchBrush && ( g_qeglobals.d_select_mode == sel_curvepoint || g_qeglobals.d_select_mode == sel_area ) ) {
				continue;
			}

			if ( !g_PrefsDlg.m_bPatchBBoxSelect && brush->patchBrush ) {
				DrawPatchMesh( brush->pPatch );
			}
			else if ( brush->owner->model.pRender && g_PrefsDlg.m_nEntityShowState != ENTITY_BOX ) {
				brush->owner->model.pRender->Draw( DRAW_GL_FLAT, ( DRAW_RF_SEL_OUTLINE | DRAW_RF_CAM ) );
			}
			else
			{
				for ( face = brush->brush_faces ; face ; face = face->next )
					Brush_FaceDraw( face, DRAW_GL_FLAT );
			}
		}


		int nCount = g_ptrSelectedFaces.GetSize();
		if ( nCount > 0 ) {
			for ( int i = 0; i < nCount; i++ )
			{
				face_t *selFace = reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( i ) );
				Brush_FaceDraw( selFace, DRAW_GL_FLAT );
			}
		}

		qglDisableClientState( GL_NORMAL_ARRAY );
		qglDepthFunc( GL_LESS );
	}

	if ( g_qeglobals.d_savedinfo.iSelectedOutlinesStyle & OUTLINE_ZBUF ) {
		// non-zbuffered outline
		qglDisable( GL_BLEND );
		qglDisable( GL_DEPTH_TEST );
		qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		qglColor3f( 1, 1, 1 );
		for ( brush = pList->next ; brush != pList ; brush = brush->next )
		{
			if ( ( brush->patchBrush && ( g_qeglobals.d_select_mode == sel_curvepoint || g_qeglobals.d_select_mode == sel_area ) ) ) {
				continue;
			}

			if ( !g_PrefsDlg.m_bPatchBBoxSelect && brush->patchBrush ) {
				DrawPatchMesh( brush->pPatch );
			}
			else if ( brush->owner->model.pRender && g_PrefsDlg.m_nEntityShowState != ENTITY_BOX ) {
				brush->owner->model.pRender->Draw( DRAW_GL_WIRE, ( DRAW_RF_SEL_FILL | DRAW_RF_CAM ) );

				// Hydra : always draw bbox outline!
				aabb_draw( brush->owner->model.pRender->GetAABB(), DRAW_GL_WIRE );
			}
			else
			{
				for ( face = brush->brush_faces ; face ; face = face->next )
					Brush_FaceDraw( face, DRAW_GL_WIRE );
			}
		}
	}

	// edge / vertex flags
	if ( g_qeglobals.d_select_mode == sel_vertex ) {
			// brush verts
			qglPointSize( 4 );
			qglColor3f( 0,1,0 );
			qglBegin( GL_POINTS );
			for ( i = 0 ; i < g_qeglobals.d_numpoints ; i++ )
				qglVertex3fv( g_qeglobals.d_points[i] );
			qglEnd();

			if ( g_qeglobals.d_num_move_points ) {
				// selected brush verts
				qglPointSize( 5 );
				qglColor3f( 0,0,1 );
				qglBegin( GL_POINTS );
				for ( i = 0; i < g_qeglobals.d_num_move_points; i++ )
					qglVertex3fv( g_qeglobals.d_move_points[i] );
				qglEnd();
			}

			qglPointSize( 1 );
		}
	else if ( g_qeglobals.d_select_mode == sel_edge ) {
		float   *v1, *v2;
		qglPointSize( 4 );
		qglColor3f( 0,0,1 );
		qglBegin( GL_POINTS );
		for ( i = 0 ; i < g_qeglobals.d_numedges ; i++ )
		{
			v1 = g_qeglobals.d_points[g_qeglobals.d_edges[i].p1];
			v2 = g_qeglobals.d_points[g_qeglobals.d_edges[i].p2];
			qglVertex3f( ( v1[0] + v2[0] ) * 0.5,( v1[1] + v2[1] ) * 0.5,( v1[2] + v2[2] ) * 0.5 );
		}
		qglEnd();
		qglPointSize( 1 );
	}

	//
	// draw pointfile
	//
	qglEnable( GL_DEPTH_TEST );
	DrawPathLines();

	if ( g_qeglobals.d_pointfile_display_list ) {
		Pointfile_Draw();
	}

	// call the drawing routine of plugin entities
	//++timo FIXME: we might need to hook in other places as well for transparency etc.
	//++timo FIXME: also needs a way to get some parameters about the view
	//++timo FIXME: maybe provide some culling API on Radiant side?
	Draw3DPluginEntities();

	// draw the crosshair
	if ( m_bFreeMove ) {
		// setup orthographic projection mode
		qglMatrixMode( GL_PROJECTION );
		//qglPushMatrix();
		qglLoadIdentity();
		qglDisable( GL_DEPTH_TEST );
		qglOrtho( 0, (float)m_Camera.width, 0, (float)m_Camera.height, -100, 100 );
		qglScalef( 1, -1, 1 );
		qglTranslatef( 0, -(float)m_Camera.height, 0 );
		qglMatrixMode( GL_MODELVIEW );

		// draw crosshair
		//qglPushMatrix();
		qglLoadIdentity();
		qglColor3f( 1.f, 1.f, 1.f );
		qglBegin( GL_LINES );
		qglVertex2f( (float)m_Camera.width / 2.f, (float)m_Camera.height / 2.f + 6 );
		qglVertex2f( (float)m_Camera.width / 2.f, (float)m_Camera.height / 2.f + 2 );
		qglVertex2f( (float)m_Camera.width / 2.f, (float)m_Camera.height / 2.f - 6 );
		qglVertex2f( (float)m_Camera.width / 2.f, (float)m_Camera.height / 2.f - 2 );
		qglVertex2f( (float)m_Camera.width / 2.f + 6, (float)m_Camera.height / 2.f );
		qglVertex2f( (float)m_Camera.width / 2.f + 2, (float)m_Camera.height / 2.f );
		qglVertex2f( (float)m_Camera.width / 2.f - 6, (float)m_Camera.height / 2.f );
		qglVertex2f( (float)m_Camera.width / 2.f - 2, (float)m_Camera.height / 2.f );
		qglEnd();
		//qglPopMatrix();

		// reset perspective projection
		//qglMatrixMode(GL_PROJECTION);
		//qglPopMatrix();
		//qglMatrixMode(GL_MODELVIEW);
	}

#if 0
	if ( ( g_qeglobals.d_select_mode == sel_area ) && ( g_nPatchClickedView == W_CAMERA ) ) {
		// setup orthographic projection mode
		qglMatrixMode( GL_PROJECTION );
		//qglPushMatrix();
		qglLoadIdentity();
		qglDisable( GL_DEPTH_TEST );
		qglOrtho( 0, (float)m_Camera.width, 0, (float)m_Camera.height, -100, 100 );
		//qglScalef(1, -1, 1);
		//qglTranslatef(0, -(float)m_Camera.height, 0);
		qglMatrixMode( GL_MODELVIEW );

		// area selection hack
		qglLoadIdentity();
		qglDisable( GL_CULL_FACE );
		qglEnable( GL_BLEND );
		qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		qglColor4f( 0.0, 0.0, 1.0, 0.25 );
		qglRectf( g_qeglobals.d_vAreaTL[0], g_qeglobals.d_vAreaTL[1], g_qeglobals.d_vAreaBR[0], g_qeglobals.d_vAreaBR[1] );
		qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		qglDisable( GL_BLEND );
		qglEnable( GL_CULL_FACE );
	}
#endif

	// bind back to the default texture so that we don't have problems
	// elsewhere using/modifying texture maps between contexts
	qglBindTexture( GL_TEXTURE_2D, 0 );

	qglFinish();
	QE_CheckOpenGLForErrors();
	//	Sys_EndWait();
	if ( m_Camera.timing ) {
		end = Sys_DoubleTime();
		Sys_Printf( "Camera: %i ms\n", (int)( 1000 * ( end - start ) ) );
	}

	for ( brush = active_brushes.next ; brush != &active_brushes ; brush = brush->next )
		brush->bCamCulled = false;

	for ( brush = pList->next ; brush != pList ; brush = brush->next )
		brush->bCamCulled = false;
}

void CamWnd::OnExpose(){
	if ( !MakeCurrent() ) {
		Sys_FPrintf( SYS_ERR, "ERROR: glXMakeCurrent failed..\n " );
		Sys_Printf( "Please restart Radiant if the camera view is not working\n" );
	}
	else
	{
		QE_CheckOpenGLForErrors();
		g_pSplitList = NULL;
		if ( g_bClipMode ) {
			if ( g_Clip1.Set() && g_Clip2.Set() ) {
				g_pSplitList = ( g_bSwitch ) ?
							   &g_brBackSplits : &g_brFrontSplits;
			}
		}

		Patch_LODMatchAll(); // spog

		Cam_Draw();
		QE_CheckOpenGLForErrors();

		m_XORRectangle.set( rectangle_t() );
		SwapBuffers();
	}
}

void CamWnd::BenchMark(){
	if ( !MakeCurrent() ) {
		Error( "glXMakeCurrent failed in Benchmark" );
	}

	qglDrawBuffer( GL_FRONT );
	double dStart = Sys_DoubleTime();
	for ( int i = 0 ; i < 100 ; i++ )
	{
		m_Camera.angles[YAW] = i * 4;
		Cam_Draw();
	}
	SwapBuffers();
	qglDrawBuffer( GL_BACK );
	double dEnd = Sys_DoubleTime();
	Sys_Printf( "%5.2f seconds\n", dEnd - dStart );
}
