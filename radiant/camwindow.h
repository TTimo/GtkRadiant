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

#ifndef _CAMWINDOW_H_
#define _CAMWINDOW_H_

class XYWnd;

#include "glwindow.h"

class rectangle_t
{
public:
rectangle_t()
	: x( 0 ), y( 0 ), w( 0 ), h( 0 )
{}
rectangle_t( float _x, float _y, float _w, float _h )
	: x( _x ), y( _y ), w( _w ), h( _h )
{}
float x;
float y;
float w;
float h;
};

class XORRectangle
{
public:
XORRectangle( GtkWidget* widget )
	: m_widget( widget ), m_gc( NULL )
{}
~XORRectangle(){
	if ( initialised() ) {
		gdk_gc_unref( m_gc );
	}
}
void set( rectangle_t rectangle ){
	lazy_init();
	draw();
	m_rectangle = rectangle;
	draw();
}
private:
bool initialised() const {
	return m_gc != NULL;
}
void lazy_init(){
	if ( !initialised() ) {
		m_gc = gdk_gc_new( m_widget->window );

		GdkColor color = { 0, 0xffff, 0xffff, 0xffff, };
		GdkColormap* colormap = gdk_window_get_colormap( m_widget->window );
		gdk_colormap_alloc_color( colormap, &color, FALSE, TRUE );
		gdk_gc_copy( m_gc, m_widget->style->white_gc );
		gdk_gc_set_foreground( m_gc, &color );
		gdk_gc_set_background( m_gc, &color );

		gdk_gc_set_function( m_gc, GDK_XOR );
	}
}
void draw() const {
	const int x = (int)m_rectangle.x;
	const int y = (int)m_rectangle.y;
	const int w = (int)m_rectangle.w;
	const int h = (int)m_rectangle.h;
	gdk_draw_rectangle( m_widget->window, m_gc, TRUE, x, -( h ) - ( y - m_widget->allocation.height ), w, h );
}

rectangle_t m_rectangle;

GtkWidget* m_widget;
GdkGC* m_gc;
};

class CamWnd : public GLWindow
{
public:
void MatchViewAxes( const vec3_t P, const vec3_t vec, int &axis, float &sgn );
void ReInitGL();
void BenchMark();
CamWnd();
virtual ~CamWnd();
camera_t *Camera(){return &m_Camera; };
void Cam_MouseControl( float dtime );
void Cam_ChangeFloor( qboolean up );
void ToggleFreeMove();
bool m_bFreeMove;

protected:
void Cam_Init();
void Cam_BuildMatrix();
void Cam_PositionDrag();
void Cam_KeyControl( float dtime );
void Cam_MouseDown( int x, int y, int buttons );
void Cam_MouseUp( int x, int y, int buttons );
void Cam_MouseMoved( int x, int y, int buttons );
void InitCull();
qboolean CullBrush( brush_t *b );
void Cam_Draw();
void Cam_DrawStuff();
void Cam_DrawBrushes( int mode );
void Cam_DrawBrush( brush_t *b, int mode );

brush_t* m_TransBrushes[MAX_MAP_BRUSHES];
int m_nNumTransBrushes;
camera_t m_Camera;
int m_nCambuttonstate;
int m_ptButtonX;
int m_ptCursorX;
int m_ptLastCursorX;
int m_ptLastCamCursorX;
int m_ptButtonY;
int m_ptCursorY;
int m_ptLastCursorY;
int m_ptLastCamCursorY;
face_t* m_pSide_select;
vec3_t m_vCull1;
vec3_t m_vCull2;
int m_nCullv1[3];
int m_nCullv2[3];
bool m_bClipMode;
guint m_FocusOutHandler_id;

void OnCreate();
void OnExpose();
void OnLButtonDown( guint32 flags, int x, int y );
void OnRButtonDown( guint32 flags, int x, int y );
void OnMButtonDown( guint32 flags, int x, int y );
void OnLButtonUp( guint32 flags, int pointx, int pointy );
void OnRButtonUp( guint32 flags, int pointx, int pointy );
void OnMButtonUp( guint32 flags, int pointx, int pointy );
void OnMouseMove( guint32 flags, int pointx, int pointy );
void OnMouseWheel( bool bUp, int pointx, int pointy );
void OnSize( int cx, int cy );

protected:
void OriginalMouseDown( guint32 nFlags, int pointX, int pointY );
void OriginalMouseUp( guint32 nFlags, int pointX, int pointY );

private:
XORRectangle m_XORRectangle;

// project a point in geometric space into camera space
void ProjectCamera( const vec3_t A, vec_t B[2] );
};


#endif // _CAMWINDOW_H_
