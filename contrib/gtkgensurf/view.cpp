/*
   GenSurf plugin for GtkRadiant
   Copyright (C) 2001 David Hyde, Loki software and qeradiant.com

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "gensurf.h"

#undef ISOMETRIC

extern double backface;
extern double dh, dv;
extern double xmin,xmax,ymin,ymax,zmin,zmax;

double SF, SFG;            // Graphics scale factors
double XLo, XHi, YLo, YHi, ZLo, ZHi;
double yaw,roll;
double elevation,azimuth;
int cxChar = 10, cyChar = 16;
int X0, Y0;
int X0G, Y0G;

static RECT rcCoord;   // where X= Y= is drawn
static RECT rcGrid;    // rectangle within rcLower that forms the border of the grid, plus
                       //   a 3 pixel slop.
static RECT rcLower;   // lower half of window, where plan view is drawn
static RECT rcUpper;   // upper half or entire window, where isometric projection is drawn

void vertex_selected();
void texfont_init();
void texfont_write( const char *text, float l, float t );

#define PEN_GRID { \
		g_GLTable.m_pfn_qglLineWidth( 1 ); \
		g_GLTable.m_pfn_qglColor3f( 0, 1, 0 ); \
		g_GLTable.m_pfn_qglDisable( GL_LINE_STIPPLE ); }

#define PEN_RED { \
		g_GLTable.m_pfn_qglLineWidth( 2 ); \
		g_GLTable.m_pfn_qglColor3f( 1, 0, 0 ); \
		g_GLTable.m_pfn_qglDisable( GL_LINE_STIPPLE ); }

#define PEN_DASH { \
		g_GLTable.m_pfn_qglLineWidth( 1 ); \
		g_GLTable.m_pfn_qglColor3f( 0, 1, 0 ); \
		g_GLTable.m_pfn_qglLineStipple( 1, 0xF0F0 ); \
		g_GLTable.m_pfn_qglEnable( GL_LINE_STIPPLE ); }

#define DRAW_QUAD( rc,r,g,b ) {	\
		g_GLTable.m_pfn_qglBegin( GL_QUADS ); \
		g_GLTable.m_pfn_qglColor3f( 0,1,0 ); \
		g_GLTable.m_pfn_qglVertex2f( rc.left - 1, rc.bottom ); \
		g_GLTable.m_pfn_qglVertex2f( rc.right, rc.bottom );	\
		g_GLTable.m_pfn_qglVertex2f( rc.right, rc.top + 1 ); \
		g_GLTable.m_pfn_qglVertex2f( rc.left - 1, rc.top + 1 );	\
		g_GLTable.m_pfn_qglColor3f( r,g,b ); \
		g_GLTable.m_pfn_qglVertex2f( rc.left, rc.bottom + 1 ); \
		g_GLTable.m_pfn_qglVertex2f( rc.right - 1, rc.bottom + 1 );	\
		g_GLTable.m_pfn_qglVertex2f( rc.right - 1, rc.top ); \
		g_GLTable.m_pfn_qglVertex2f( rc.left, rc.top );	\
		g_GLTable.m_pfn_qglEnd(); }


#ifndef ISOMETRIC
double D = 65536.;
double ct[3],st[3];
double Hhi, Hlo, Vhi, Vlo;
#endif

#define SUBDIVS 6


void ShowPreview(){
	if ( Preview ) {
		if ( g_pWndPreview == NULL ) {
			CreateViewWindow();
		}
		gtk_widget_show( g_pWndPreview );

		UpdatePreview( true );
	}
	else{
		gtk_widget_hide( g_pWndPreview );
	}
}

static void draw_preview(){
	int width = g_pPreviewWidget->allocation.width, height = g_pPreviewWidget->allocation.height;

	g_GLTable.m_pfn_qglClearColor( 0, 0, 0, 1 );
	g_GLTable.m_pfn_qglViewport( 0, 0, width, height );
	g_GLTable.m_pfn_qglMatrixMode( GL_PROJECTION );
	g_GLTable.m_pfn_qglLoadIdentity();
	g_GLTable.m_pfn_qglOrtho( 0, width, 0, height, -1, 1 );
	g_GLTable.m_pfn_qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// ^Fishman - Antializing for the preview window.
	if ( Antialiasing ) {
		g_GLTable.m_pfn_qglEnable( GL_BLEND );
		g_GLTable.m_pfn_qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		g_GLTable.m_pfn_qglEnable( GL_LINE_SMOOTH );
	}
	else
	{
		g_GLTable.m_pfn_qglDisable( GL_BLEND );
		g_GLTable.m_pfn_qglDisable( GL_LINE_SMOOTH );
	}

	texfont_init();

	if ( !ValidSurface() ) {
		return;
	}

	rcUpper.left = 0;
	rcUpper.right = width;
	rcUpper.bottom = 0;
	rcUpper.top = height;
	rcLower.left = 0;
	rcLower.right = width;
	rcLower.bottom = 0;
	rcLower.top = height;

	if ( VertexMode ) {
		rcUpper.bottom = rcUpper.top / 2;
		DrawPreview( rcUpper );
		g_GLTable.m_pfn_qglBegin( GL_LINES );
		g_GLTable.m_pfn_qglVertex2f( rcUpper.left, rcUpper.bottom );
		g_GLTable.m_pfn_qglVertex2f( rcUpper.right, rcUpper.bottom );
		g_GLTable.m_pfn_qglEnd();
		rcLower.top = rcUpper.bottom - 1;
		DrawGrid( rcLower );
		rcCoord.left = rcLower.left;
		rcCoord.right = rcLower.right;
		rcCoord.bottom = rcLower.bottom;
		rcCoord.top = rcLower.top;
		rcCoord.top = rcCoord.bottom + cyChar;
		rcCoord.right = rcCoord.left + 15 * cxChar;
		rcGrid.left   = X0G - 3;
		rcGrid.bottom = Y0G - 3;
		rcGrid.right  = X0G + (int)( SFG * ( Hur - Hll ) ) + 3;
		rcGrid.top    = Y0G + (int)( SFG * ( Vur - Vll ) ) + 3;
	}
	else{
		DrawPreview( rcUpper );
	}
}

static gint expose( GtkWidget *widget, GdkEventExpose *event, gpointer data ){
	if ( event->count > 0 ) {
		return TRUE;
	}

	if ( !g_UIGtkTable.m_pfn_glwidget_make_current( g_pPreviewWidget ) ) {
		g_FuncTable.m_pfnSysPrintf( "GtkGenSurf: glMakeCurrent failed\n" );
		return TRUE;
	}

	draw_preview();

	g_UIGtkTable.m_pfn_glwidget_swap_buffers( g_pPreviewWidget );
	g_GLTable.m_pfn_QE_CheckOpenGLForErrors();

	return TRUE;
}

static void button_press( GtkWidget *widget, GdkEventButton *event, gpointer data ){
	POINT pt = { (long)event->x, widget->allocation.height - (long)event->y };
	bool Selected;
	double x,y;
	int i, j, k, ks;
	int i0, i1, j0, j1;

	if ( ( !VertexMode ) || ( event->button != 1 ) ) {
		return;
	}

	if ( !PtInRect( &rcGrid,pt ) ) {
		gdk_beep();
		return;
	}

	x = Hll + ( pt.x - X0G ) / SFG;
	y = Vur - ( pt.y - Y0G ) / SFG;
	i = (int)( floor( ( x - Hll ) / dh - 0.5 ) + 1 );
	j = (int)( floor( ( y - Vll ) / dv - 0.5 ) + 1 );
	if ( i < 0 || i > NH || j < 0 || j > NV ) {
		gdk_beep();
		return;
	}

	if ( !CanEdit( i,j ) ) {
		gdk_beep();
		return;
	}

	// Control key pressed - add this point, or remove it if already selected
	if ( ( event->state & GDK_CONTROL_MASK ) != 0 ) {
		Selected = FALSE;
		if ( NumVerticesSelected ) {
			for ( k = 0; k < NumVerticesSelected && !Selected; k++ )
			{
				if ( Vertex[k].i == i && Vertex[k].j == j ) {
					Selected = TRUE;
					ks = k;
				}
			}
		}

		// Already selected - unselect it.
		if ( Selected ) {
			if ( ks < NumVerticesSelected ) {
				for ( k = ks; k < NumVerticesSelected - 1; k++ )
				{
					Vertex[k].i = Vertex[k + 1].i;
					Vertex[k].j = Vertex[k + 1].j;
				}
				NumVerticesSelected--;
			}
		}
		else
		{
			Vertex[NumVerticesSelected].i = i;
			Vertex[NumVerticesSelected].j = j;
			NumVerticesSelected++;
		}
	}
	else if ( ( event->state & GDK_SHIFT_MASK ) != 0 ) {
		if ( NumVerticesSelected ) {
			NumVerticesSelected = 1;
			i0 = min( Vertex[0].i, i );
			i1 = max( Vertex[0].i, i );
			j0 = min( Vertex[0].j, j );
			j1 = max( Vertex[0].j, j );
			for ( i = i0; i <= i1; i++ )
			{
				for ( j = j0; j <= j1; j++ )
				{
					if ( i == 0  && j == 0 ) {
						continue;
					}
					if ( i == NH && j == 0 ) {
						continue;
					}
					if ( i == 0  && j == NV ) {
						continue;
					}
					if ( i == NH && j == NV ) {
						continue;
					}
					if ( i != Vertex[0].i || j != Vertex[0].j ) {
						Vertex[NumVerticesSelected].i = i;
						Vertex[NumVerticesSelected].j = j;
						NumVerticesSelected++;
					}
				}
			}
		}
		else
		{
			Vertex[0].i = i;
			Vertex[0].j = j;
			NumVerticesSelected = 1;
		}
	}
	else
	{
		Vertex[0].i = i;
		Vertex[0].j = j;
		NumVerticesSelected = 1;
	}

	vertex_selected();
}

static void motion( GtkWidget *widget, GdkEventMotion *event, gpointer data ){
	POINT pt = { (long)event->x, widget->allocation.height - (long)event->y };

	if ( !VertexMode ) {
		return;
	}

	if ( !g_UIGtkTable.m_pfn_glwidget_make_current( g_pPreviewWidget ) ) {
		g_FuncTable.m_pfnSysPrintf( "GtkGenSurf: glMakeCurrent failed\n" );
		return;
	}

	g_GLTable.m_pfn_qglEnable( GL_SCISSOR_TEST );
	g_GLTable.m_pfn_qglScissor( rcCoord.left, rcCoord.bottom, rcCoord.right - rcCoord.left,
								rcCoord.top - rcCoord.bottom );
	g_GLTable.m_pfn_qglClear( GL_COLOR_BUFFER_BIT );

	if ( PtInRect( &rcGrid,pt ) ) {
		GdkWindow *window;
		GdkDisplay *display;
		GdkCursor *cursor;

		window = gtk_widget_get_window( g_pWndPreview );
		display = gdk_window_get_display( window );
		cursor = gdk_cursor_new_for_display( display, GDK_CROSS );

		gdk_window_set_cursor( window, cursor );
#if GTK_CHECK_VERSION( 3, 0, 0 )
		g_object_unref( cursor );
#else
		gdk_cursor_unref( cursor );
#endif

		char Text[32];
		int x, y;

		x = (int)( Hll + ( pt.x - X0G ) / SFG );
		y = (int)( Vur - ( pt.y - Y0G ) / SFG );
		switch ( Plane )
		{
		case PLANE_XZ0:
		case PLANE_XZ1:
			sprintf( Text," x=%d, z=%d   ",(int)( floor( x - 0.5 ) + 1. ),(int)( floor( y - 0.5 ) + 1. ) );
			break;
		case PLANE_YZ0:
		case PLANE_YZ1:
			sprintf( Text," y=%d, z=%d   ",(int)( floor( x - 0.5 ) + 1. ),(int)( floor( y - 0.5 ) + 1. ) );
			break;
		default:
			sprintf( Text," x=%d, y=%d   ",(int)( floor( x - 0.5 ) + 1. ),(int)( floor( y - 0.5 ) + 1. ) );
		}

		texfont_write( Text, rcCoord.left, rcCoord.top );
	}
	else
	{
		gdk_window_set_cursor( gtk_widget_get_window( g_pWndPreview ), NULL );
	}

	g_UIGtkTable.m_pfn_glwidget_swap_buffers( g_pPreviewWidget );
	g_GLTable.m_pfn_QE_CheckOpenGLForErrors();
	g_GLTable.m_pfn_qglDisable( GL_SCISSOR_TEST );
}

static gint preview_close( GtkWidget *widget, gpointer data ){
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( g_object_get_data( G_OBJECT( g_pWnd ), "main_preview" ) ), FALSE );
	return TRUE;
}

static void preview_focusout( GtkSpinButton *spin, GdkEventFocus *event, double *data ){
	*data = DegreesToRadians( (double)( gtk_spin_button_get_value_as_int( spin ) % 360 ) );
	UpdatePreview( false );
}

static gint doublevariable_spinfocusout( GtkWidget* widget, GdkEventFocus* event, gpointer data ){
	preview_focusout( GTK_SPIN_BUTTON( widget ), event, reinterpret_cast<double*>( data ) );
	return FALSE;
}

static void preview_spin( GtkAdjustment *adj, double *data ){
	*data = DegreesToRadians( gtk_adjustment_get_value( adj ) );
	UpdatePreview( false );
}

void CreateViewWindow(){
	GtkWidget *dlg, *vbox, *hbox, *label, *spin, *frame;
	GtkAdjustment *adj;

#ifndef ISOMETRIC
	elevation = PI / 6.;
	azimuth   = PI / 6.;
#endif

	g_pWndPreview = dlg = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_title( GTK_WINDOW( dlg ), _( "GtkGenSurf Preview" ) );
	g_signal_connect( G_OBJECT( dlg ), "delete-event", G_CALLBACK( preview_close ), NULL );
	g_signal_connect( G_OBJECT( dlg ), "destroy", G_CALLBACK( gtk_widget_destroy ), NULL );
	gtk_window_set_transient_for( GTK_WINDOW( dlg ), GTK_WINDOW( g_pWnd ) );
	gtk_window_set_default_size( GTK_WINDOW( dlg ), 300, 400 );

	vbox = gtk_vbox_new( FALSE, 5 );
	gtk_container_add( GTK_CONTAINER( dlg ), vbox );
	gtk_widget_show( vbox );

#ifndef ISOMETRIC
	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_set_homogeneous( GTK_BOX( hbox ), TRUE );
	gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, TRUE, 0 );
	gtk_container_set_border_width( GTK_CONTAINER( hbox ), 3 );
	gtk_widget_show( hbox );

	label = gtk_label_new( _( "Elevation" ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_box_pack_start( GTK_BOX( hbox ), label, FALSE, TRUE, 0 );
	gtk_widget_show( label );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 30, -90, 90, 1, 10, 0 ) );
	g_signal_connect( adj, "value-changed", G_CALLBACK( preview_spin ), &elevation );
	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_box_pack_start( GTK_BOX( hbox ), spin, FALSE, TRUE, 0 );
	gtk_widget_show( spin );
	g_signal_connect( G_OBJECT( spin ), "focus-out-event", G_CALLBACK( doublevariable_spinfocusout ), &elevation );

	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 30, 0, 359, 1, 10, 0 ) );
	g_signal_connect( adj, "value-changed", G_CALLBACK( preview_spin ), &azimuth );
	spin = gtk_spin_button_new( GTK_ADJUSTMENT( adj ), 1, 0 );
	gtk_entry_set_alignment( GTK_ENTRY( spin ), 1.0 ); //right
	gtk_spin_button_set_wrap( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( spin ), TRUE );
	gtk_box_pack_end( GTK_BOX( hbox ), spin, FALSE, TRUE, 0 );
	gtk_widget_show( spin );

	label = gtk_label_new( _( "Azimuth" ) );
	gtk_misc_set_alignment( GTK_MISC( label ), 0.0, 0.5 );
	gtk_box_pack_end( GTK_BOX( hbox ), label, FALSE, TRUE, 0 );
	gtk_widget_show( label );
	g_signal_connect( G_OBJECT( spin ), "focus-out-event", G_CALLBACK( doublevariable_spinfocusout ), &azimuth );

#endif

	frame = gtk_frame_new( NULL );
	gtk_frame_set_shadow_type( GTK_FRAME( frame ), GTK_SHADOW_IN );
	gtk_box_pack_start( GTK_BOX( vbox ), frame, TRUE, TRUE, 0 );
	gtk_widget_show( frame );

	g_pPreviewWidget = g_UIGtkTable.m_pfn_glwidget_new( FALSE, NULL );

	gtk_widget_set_events( g_pPreviewWidget, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK );
	g_signal_connect( G_OBJECT( g_pPreviewWidget ), "expose-event", G_CALLBACK( expose ), NULL );
	g_signal_connect( G_OBJECT( g_pPreviewWidget ), "motion-notify-event", G_CALLBACK( motion ), NULL );
	g_signal_connect( G_OBJECT( g_pPreviewWidget ), "button-press-event",
						G_CALLBACK( button_press ), NULL );

	gtk_container_add( GTK_CONTAINER( frame ), g_pPreviewWidget );
	gtk_widget_show( g_pPreviewWidget );

	if ( Preview ) {
		gtk_widget_show( g_pWndPreview );
	}

	UpdatePreview( true );
}

//=============================================================
/* DrawPreview */
void DrawPreview( RECT rc ){
#define COSXA 0.8660254037844
#define SINXA 0.5
#define COSYA 0.8660254037844
#define SINYA 0.5

	double L;
	double x,y;
	int i, j;
	POINT pt[8];
	XYZ v[8];
	char axis[3][2] = {"X","Y","Z"};

#ifndef ISOMETRIC
	evaluate();
#endif

	XLo = xmin;
	XHi = xmax;
	YLo = ymin;
	YHi = ymax;
	ZLo = zmin;
	ZHi = zmax;
	switch ( Plane )
	{
	case PLANE_XY1:
		ZHi = backface;
		break;
	case PLANE_XZ0:
		YLo = backface;
		break;
	case PLANE_XZ1:
		YHi = backface;
		break;
	case PLANE_YZ0:
		XLo = backface;
		break;
	case PLANE_YZ1:
		XHi = backface;
		break;
	default:
		ZLo = backface;
	}



	GetScaleFactor( rc );
	//PEN_GRID
	g_GLTable.m_pfn_qglLineWidth( 1 );
	g_GLTable.m_pfn_qglColor3f( 0, 1, 0 );
	g_GLTable.m_pfn_qglDisable( GL_LINE_STIPPLE );

	if ( Decimate > 0 && ( Game != QUAKE3 || UsePatches == 0 ) ) {
		XYZ  *vv;

		vv   = (XYZ *) malloc( gNumNodes * sizeof( XYZ ) );
		for ( i = 0; i < gNumNodes; i++ )
		{
			for ( j = 0; j < 3; j++ )
				vv[i].p[j] = (double)( gNode[i].p[j] );
			project( &vv[i] );
		}

		for ( i = 0; i < gNumTris; i++ )
		{
			for ( j = 0; j < 3; j++ )
				Scale( rc,vv[gTri[i].v[j]],&pt[j] );

			g_GLTable.m_pfn_qglBegin( GL_LINE_STRIP );
			g_GLTable.m_pfn_qglVertex2f( pt[0].x, pt[0].y );
			g_GLTable.m_pfn_qglVertex2f( pt[1].x, pt[1].y );
			g_GLTable.m_pfn_qglVertex2f( pt[2].x, pt[2].y );
			g_GLTable.m_pfn_qglVertex2f( pt[0].x, pt[0].y );
			g_GLTable.m_pfn_qglEnd();
		}
		free( vv );
	}
	else if ( Game == QUAKE3 && UsePatches != 0 ) {
		int axis, ii, jj, k;
		float u, v;
		XYZ uv[3][3];
		XYZ Ctrl[3],out;

		switch ( Plane )
		{
		case PLANE_XY0:
		case PLANE_XY1:
			k = 2;
			break;
		case PLANE_XZ0:
		case PLANE_XZ1:
			k = 1;
			break;
		default:
			k = 0;
		}
		for ( i = 0; i < NH; i += 2 )
		{
			for ( j = 0; j < NV; j += 2 )
			{
				VectorCopy( xyz[i  ][j  ].p,uv[0][0].p );
				VectorCopy( xyz[i + 1][j  ].p,uv[1][0].p );
				VectorCopy( xyz[i + 2][j  ].p,uv[2][0].p );
				VectorCopy( xyz[i  ][j + 1].p,uv[0][1].p );
				VectorCopy( xyz[i + 1][j + 1].p,uv[1][1].p );
				VectorCopy( xyz[i + 2][j + 1].p,uv[2][1].p );
				VectorCopy( xyz[i  ][j + 2].p,uv[0][2].p );
				VectorCopy( xyz[i + 1][j + 2].p,uv[1][2].p );
				VectorCopy( xyz[i + 2][j + 2].p,uv[2][2].p );
				uv[1][0].p[k] = ( 4 * xyz[i + 1][j  ].p[k] - xyz[i  ][j  ].p[k] - xyz[i + 2][j  ].p[k] ) / 2;
				uv[0][1].p[k] = ( 4 * xyz[i  ][j + 1].p[k] - xyz[i  ][j  ].p[k] - xyz[i  ][j + 2].p[k] ) / 2;
				uv[2][1].p[k] = ( 4 * xyz[i + 2][j + 1].p[k] - xyz[i + 2][j  ].p[k] - xyz[i + 2][j + 2].p[k] ) / 2;
				uv[1][2].p[k] = ( 4 * xyz[i + 1][j + 2].p[k] - xyz[i  ][j + 2].p[k] - xyz[i + 2][j + 2].p[k] ) / 2;
				uv[1][1].p[k] = ( 16 * xyz[i + 1][j + 1].p[k] -
								  xyz[i  ][j  ].p[k] - 2 * xyz[i + 1][j  ].p[k] -  xyz[i + 2][j  ].p[k] -
								  2 * xyz[i  ][j + 1].p[k]                        - 2 * xyz[i + 2][j + 1].p[k] -
								  xyz[i  ][j + 2].p[k] - 2 * xyz[i + 1][j + 2].p[k] -  xyz[i + 2][j + 2].p[k]   ) / 4;

				for ( ii = 0; ii <= SUBDIVS; ii++ )
				{
					if ( ii == 0 || ii == SUBDIVS / 2 || ii == SUBDIVS ) {
						g_GLTable.m_pfn_qglLineWidth( 1 );
						g_GLTable.m_pfn_qglColor3f( 0, 1, 0 );
						g_GLTable.m_pfn_qglDisable( GL_LINE_STIPPLE );
						// PEN_GRID
					}
					else
					{
						g_GLTable.m_pfn_qglLineWidth( 1 );
						g_GLTable.m_pfn_qglColor3f( 0, 1, 0 );
						g_GLTable.m_pfn_qglLineStipple( 1, 0xF0F0 );
						g_GLTable.m_pfn_qglEnable( GL_LINE_STIPPLE );
						// PEN_DASH
					}

					u = (float)( ii ) / (float)( SUBDIVS );
					for ( jj = 0; jj < 3; jj++ )
					{
						for ( axis = 0; axis < 3; axis++ )
						{
							float a, b, c;
							float qA, qB, qC;
							a = (float)uv[0][jj].p[axis];
							b = (float)uv[1][jj].p[axis];
							c = (float)uv[2][jj].p[axis];
							qA = a - 2 * b + c;
							qB = 2 * b - 2 * a;
							qC = a;
							Ctrl[jj].p[axis] = qA * u * u + qB * u + qC;
						}
					}
					VectorCopy( Ctrl[0].p,out.p );
					project( &out );
					Scale( rc,out,&pt[0] );
					g_GLTable.m_pfn_qglBegin( GL_LINE_STRIP );
					g_GLTable.m_pfn_qglVertex2f( pt[0].x, pt[0].y );
					for ( jj = 1; jj <= SUBDIVS; jj++ )
					{
						v = (float)( jj ) / (float)( SUBDIVS );
						for ( axis = 0 ; axis < 3 ; axis++ )
						{
							float a, b, c;
							float qA, qB, qC;
							a = (float)Ctrl[0].p[axis];
							b = (float)Ctrl[1].p[axis];
							c = (float)Ctrl[2].p[axis];
							qA = a - 2 * b + c;
							qB = 2 * b - 2 * a;
							qC = a;
							out.p[axis] = qA * v * v + qB * v + qC;
						}
						project( &out );
						Scale( rc,out,&pt[0] );
						g_GLTable.m_pfn_qglVertex2f( pt[0].x, pt[0].y );
					}
					g_GLTable.m_pfn_qglEnd();
				}
				for ( jj = 0; jj <= SUBDIVS; jj++ )
				{
					if ( jj == 0 || jj == SUBDIVS / 2 || jj == SUBDIVS ) {
						g_GLTable.m_pfn_qglLineWidth( 1 );
						g_GLTable.m_pfn_qglColor3f( 0, 1, 0 );
						g_GLTable.m_pfn_qglDisable( GL_LINE_STIPPLE );
						// PEN_GRID
					}
					else
					{
						g_GLTable.m_pfn_qglLineWidth( 1 );
						g_GLTable.m_pfn_qglColor3f( 0, 1, 0 );
						g_GLTable.m_pfn_qglLineStipple( 1, 0xF0F0 );
						g_GLTable.m_pfn_qglEnable( GL_LINE_STIPPLE );
						// PEN_DASH
					}

					v = (float)( jj ) / (float)( SUBDIVS );
					for ( ii = 0; ii < 3; ii++ )
					{
						for ( axis = 0; axis < 3; axis++ )
						{
							float a, b, c;
							float qA, qB, qC;
							a = (float)uv[ii][0].p[axis];
							b = (float)uv[ii][1].p[axis];
							c = (float)uv[ii][2].p[axis];
							qA = a - 2 * b + c;
							qB = 2 * b - 2 * a;
							qC = a;
							Ctrl[ii].p[axis] = qA * v * v + qB * v + qC;
						}
					}
					VectorCopy( Ctrl[0].p,out.p );
					project( &out );
					Scale( rc,out,&pt[0] );
					g_GLTable.m_pfn_qglBegin( GL_LINE_STRIP );
					g_GLTable.m_pfn_qglVertex2f( pt[0].x, pt[0].y );
					for ( ii = 1; ii <= SUBDIVS; ii++ )
					{
						u = (float)( ii ) / (float)( SUBDIVS );
						for ( axis = 0 ; axis < 3 ; axis++ )
						{
							float a, b, c;
							float qA, qB, qC;
							a = (float)Ctrl[0].p[axis];
							b = (float)Ctrl[1].p[axis];
							c = (float)Ctrl[2].p[axis];
							qA = a - 2 * b + c;
							qB = 2 * b - 2 * a;
							qC = a;
							out.p[axis] = qA * u * u + qB * u + qC;
						}
						project( &out );
						Scale( rc,out,&pt[0] );
						g_GLTable.m_pfn_qglVertex2f( pt[0].x, pt[0].y );
					}
					g_GLTable.m_pfn_qglEnd();
				}
			}
		}
	}
	else
	{
		for ( i = 0; i <= NH; i++ )
		{
			Scale( rc,xyz[i][0],&pt[0] );
			g_GLTable.m_pfn_qglBegin( GL_LINE_STRIP );
			g_GLTable.m_pfn_qglVertex2f( pt[0].x, pt[0].y );
			for ( j = 1; j <= NV; j++ )
			{
				Scale( rc,xyz[i][j],&pt[0] );
				g_GLTable.m_pfn_qglVertex2f( pt[0].x, pt[0].y );
			}
			g_GLTable.m_pfn_qglEnd();
		}
		for ( j = 0; j <= NV; j++ )
		{
			Scale( rc,xyz[0][j],&pt[0] );
			g_GLTable.m_pfn_qglBegin( GL_LINE_STRIP );
			g_GLTable.m_pfn_qglVertex2f( pt[0].x, pt[0].y );
			for ( i = 1; i <= NH; i++ )
			{
				Scale( rc,xyz[i][j],&pt[0] );
				g_GLTable.m_pfn_qglVertex2f( pt[0].x, pt[0].y );
			}
			g_GLTable.m_pfn_qglEnd();
		}
	}

	if ( Game != QUAKE3 || UsePatches == 0 ) {
		// Draw lines from corners to base, and lines around base
		for ( i = 0; i <= NH; i += NH )
		{
			for ( j = 0; j <= NV; j += NV )
			{
				VectorCopy( xyz[i][j].p, v[0].p );
				switch ( Plane )
				{
				case PLANE_XZ0:
				case PLANE_XZ1:
					v[0].p[1] = backface;
					break;
				case PLANE_YZ0:
				case PLANE_YZ1:
					v[0].p[0] = backface;
					break;
				default:
					v[0].p[2] = backface;
				}
				Scale( rc,xyz[i][j],&pt[0] );
#ifndef ISOMETRIC
				project( &v[0] );
#endif
				Scale( rc,v[0],&pt[1] );
				g_GLTable.m_pfn_qglBegin( GL_LINE_STRIP );
				g_GLTable.m_pfn_qglVertex2f( pt[0].x, pt[0].y );
				g_GLTable.m_pfn_qglVertex2f( pt[1].x, pt[1].y );
				g_GLTable.m_pfn_qglEnd();
			}
		}
		VectorCopy( xyz[ 0][ 0].p, v[0].p );
		VectorCopy( xyz[NH][ 0].p, v[1].p );
		VectorCopy( xyz[NH][NV].p, v[2].p );
		VectorCopy( xyz[ 0][NV].p, v[3].p );
		switch ( Plane )
		{
		case PLANE_XZ0:
		case PLANE_XZ1:
			v[0].p[1] = backface;;
			v[1].p[1] = v[0].p[1];
			v[2].p[1] = v[0].p[1];
			v[3].p[1] = v[0].p[1];
			break;
		case PLANE_YZ0:
		case PLANE_YZ1:
			v[0].p[0] = backface;
			v[1].p[0] = v[0].p[0];
			v[2].p[0] = v[0].p[0];
			v[3].p[0] = v[0].p[0];
			break;
		default:
			v[0].p[2] = backface;
			v[1].p[2] = v[0].p[2];
			v[2].p[2] = v[0].p[2];
			v[3].p[2] = v[0].p[2];
		}
#ifndef ISOMETRIC
		project( &v[3] );
#endif
		Scale( rc,v[3],&pt[0] );
		g_GLTable.m_pfn_qglBegin( GL_LINE_STRIP );
		g_GLTable.m_pfn_qglVertex2f( pt[0].x, pt[0].y );
		for ( i = 0; i < 3; i++ )
		{
#ifndef ISOMETRIC
			project( &v[i] );
#endif
			Scale( rc,v[i],&pt[1] );
			g_GLTable.m_pfn_qglVertex2f( pt[1].x, pt[1].y );
		}
		g_GLTable.m_pfn_qglVertex2f( pt[0].x, pt[0].y );
		g_GLTable.m_pfn_qglEnd();
	}

	g_GLTable.m_pfn_qglLineWidth( 1 );
	g_GLTable.m_pfn_qglColor3f( 0, 1, 0 );
	g_GLTable.m_pfn_qglDisable( GL_LINE_STIPPLE );

#ifdef ISOMETRIC
	// Draw small depiction of coordinate axes
	pt[0].x = rc.right  - cxChar   - cxChar / 2 -  cyChar;
	pt[0].y = rc.bottom - cyChar / 2 - cxChar / 2;
	pt[1].x = pt[0].x + (int)( cyChar * COSXA );
	pt[1].y = pt[0].y - (int)( cyChar * SINXA );
	MoveToEx( hdc,pt[0].x,pt[0].y,NULL );
	LineTo( hdc,pt[1].x,pt[1].y );
	SetTextAlign( hdc,TA_LEFT | TA_TOP );
	TextOut( hdc,pt[1].x,pt[1].y - cyChar / 2,"X",1 );
	pt[1].x = pt[0].x - (int)( cyChar * COSYA );
	pt[1].y = pt[0].y - (int)( cyChar * SINYA );
	MoveToEx( hdc,pt[0].x,pt[0].y,NULL );
	LineTo( hdc,pt[1].x,pt[1].y );
	SetTextAlign( hdc,TA_RIGHT | TA_TOP );
	TextOut( hdc,pt[1].x,pt[1].y - cyChar / 2,"Y",1 );
	pt[1].x = pt[0].x;
	pt[1].y = pt[0].y - cyChar;
	MoveToEx( hdc,pt[0].x,pt[0].y,NULL );
	LineTo( hdc,pt[1].x,pt[1].y );
	SetTextAlign( hdc,TA_CENTER | TA_BOTTOM );
	TextOut( hdc,pt[1].x,pt[1].y,"Z",1 );
#else
	L = 2 * (double)cyChar / SF;
	v[0].p[0] = 0.;
	v[0].p[1] = 0.;
	v[0].p[2] = 0.;
	v[1].p[0] = L;
	v[1].p[1] = 0.;
	v[1].p[2] = 0.;
	v[2].p[0] = 0.;
	v[2].p[1] = L;
	v[2].p[2] = 0.;
	v[3].p[0] = 0.;
	v[3].p[1] = 0.;
	v[3].p[2] = L;
	for ( i = 0; i <= 3; i++ )
	{
		project( &v[i] );
		Scale( rc,v[i],&pt[i] );
	}
	for ( i = 1; i <= 3; i++ )
	{
		pt[i].x += -pt[0].x + rc.right  - 2 * cyChar;
		pt[i].y += -pt[0].y + rc.bottom + 2 * cyChar;
	}
	pt[0].x = rc.right  - 2 * cyChar;
	pt[0].y = rc.bottom + 2 * cyChar;

	for ( i = 1; i <= 3; i++ )
	{
		g_GLTable.m_pfn_qglBegin( GL_LINES );
		g_GLTable.m_pfn_qglVertex2f( pt[0].x, pt[0].y );
		g_GLTable.m_pfn_qglVertex2f( pt[i].x, pt[i].y );
		g_GLTable.m_pfn_qglEnd();
		texfont_write( axis[i - 1], pt[i].x - cxChar / 2,pt[i].y + cyChar / 2 );
	}
#endif

	// Draw player model's bounding box in red to give a sense of scale
	// PEN_RED
	g_GLTable.m_pfn_qglLineWidth( 2 );
	g_GLTable.m_pfn_qglColor3f( 1, 0, 0 );
	g_GLTable.m_pfn_qglDisable( GL_LINE_STIPPLE );

	switch ( Plane )
	{
	case PLANE_XY1:
		v[0].p[0] = xyz[NH / 2][NV / 2].p[0] + PlayerBox[Game].x[0];
		v[0].p[1] = xyz[NH / 2][NV / 2].p[1] + PlayerBox[Game].y[0];
		v[0].p[2] = zmin - PlayerBox[Game].z[0] - 32;
		break;
	case PLANE_XZ0:
		v[0].p[0] = ( xmax + xmin ) / 2 + PlayerBox[Game].x[0];
		v[0].p[1] = ymax + 64;
		v[0].p[2] = zmin;
		break;
	case PLANE_XZ1:
		v[0].p[0] = ( xmax + xmin ) / 2 + PlayerBox[Game].x[0];
		v[0].p[1] = ymin - 64;
		v[0].p[2] = zmin;
		break;
	case PLANE_YZ0:
		v[0].p[0] = xmax + 64;
		v[0].p[1] = ( ymax + ymin ) / 2 + PlayerBox[Game].y[0];
		v[0].p[2] = zmin;
		break;
	case PLANE_YZ1:
		v[0].p[0] = xmin - 64;
		v[0].p[1] = ( ymax + ymin ) / 2 + PlayerBox[Game].y[0];
		v[0].p[2] = zmin;
		break;
	default:
		// Put player on a node. For patches, put on an even numbered node.
		if ( Game == QUAKE3 && UsePatches != 0 ) {
			if ( NH > 2 ) {
				x = Hll + dh * (int)( NH / 2 + 1 );
			}
			else{
				x = Hll + dh * (int)( NH / 2 );
			}
			if ( NV > 2 ) {
				y = Vll + dv * (int)( NV / 2 + 1 );
			}
			else{
				y = Vll + dv * (int)( NV / 2 );
			}
		}
		else
		{
			if ( NH > 1 ) {
				x = Hll + dh * (int)( NH / 2 );
			}
			else{
				x = Hll + dh / 2;
			}
			if ( NV > 1 ) {
				y = Vll + dv * (int)( NV / 2 );
			}
			else{
				y = Vll + dv / 2;
			}
		}
//		x = (Hll+Hur)/2.;
//		y = (Vll+Vur)/2.;
		v[0].p[0] = x + PlayerBox[Game].x[0];
		v[0].p[1] = y + PlayerBox[Game].y[0];
		v[0].p[2] = PlayerStartZ( x,y ) + PlayerBox[Game].z[0] + 8; // add 8 cuz I'm a pessimist
	}
	v[1].p[0] = v[0].p[0] + PlayerBox[Game].x[1] - PlayerBox[Game].x[0];
	v[1].p[1] = v[0].p[1];
	v[1].p[2] = v[0].p[2];
	v[2].p[0] = v[1].p[0];
	v[2].p[1] = v[1].p[1] + PlayerBox[Game].y[1] - PlayerBox[Game].y[0];
	v[2].p[2] = v[0].p[2];
	v[3].p[0] = v[0].p[0];
	v[3].p[1] = v[2].p[1];
	v[3].p[2] = v[0].p[2];
	VectorCopy( v[0].p,v[4].p );
	VectorCopy( v[1].p,v[5].p );
	VectorCopy( v[2].p,v[6].p );
	VectorCopy( v[3].p,v[7].p );
	v[4].p[2] += PlayerBox[Game].z[1] - PlayerBox[Game].z[0];
	v[5].p[2] += PlayerBox[Game].z[1] - PlayerBox[Game].z[0];
	v[6].p[2] += PlayerBox[Game].z[1] - PlayerBox[Game].z[0];
	v[7].p[2] += PlayerBox[Game].z[1] - PlayerBox[Game].z[0];
	for ( i = 0; i <= 7; i++ )
	{
#ifndef ISOMETRIC
		project( &v[i] );
#endif
		Scale( rc,v[i],&pt[i] );
	}
	g_GLTable.m_pfn_qglBegin( GL_LINE_STRIP );
	g_GLTable.m_pfn_qglVertex2f( pt[3].x, pt[3].y );
	for ( i = 0; i <= 3; i++ )
		g_GLTable.m_pfn_qglVertex2f( pt[i].x, pt[i].y );
	g_GLTable.m_pfn_qglEnd();
	g_GLTable.m_pfn_qglBegin( GL_LINE_STRIP );
	g_GLTable.m_pfn_qglVertex2f( pt[7].x, pt[7].y );
	for ( i = 4; i <= 7; i++ )
		g_GLTable.m_pfn_qglVertex2f( pt[i].x, pt[i].y );
	g_GLTable.m_pfn_qglEnd();
	g_GLTable.m_pfn_qglBegin( GL_LINES );
	for ( i = 0; i <= 3; i++ )
	{
		g_GLTable.m_pfn_qglVertex2f( pt[i].x,pt[i].y );
		g_GLTable.m_pfn_qglVertex2f( pt[i + 4].x,pt[i + 4].y );
	}
	g_GLTable.m_pfn_qglEnd();

	g_GLTable.m_pfn_qglLineWidth( 1 );
	g_GLTable.m_pfn_qglColor3f( 0, 1, 0 );
	g_GLTable.m_pfn_qglDisable( GL_LINE_STIPPLE );
}
//=============================================================
void DrawGrid( RECT rc ){
	int i, j, k;
	double h,w,x,y;
	POINT pt[2];
	RECT rcBox;

	w = (double)( rc.right - rc.left + 1 ) - cxChar;
	h = (double)( rc.top - rc.bottom + 1 ) - cxChar - cyChar;

	SFG = w / ( Hur - Hll );
	SFG = min( SFG, h / ( Vur - Vll ) );

	// Center drawing
	X0G = (int)( rc.left + rc.right - (int)( SFG * ( Hur - Hll ) ) ) / 2;
	Y0G = (int)( rc.top + rc.bottom + cyChar - (int)( SFG * ( Vur - Vll ) ) ) / 2;

	g_GLTable.m_pfn_qglLineWidth( 2 );
	g_GLTable.m_pfn_qglColor3f( 0, 1, 0 );
	g_GLTable.m_pfn_qglDisable( GL_LINE_STIPPLE );

	pt[0].y = Y0G;
	pt[1].y = Y0G + (int)( SFG * ( Vur - Vll ) );
	g_GLTable.m_pfn_qglBegin( GL_LINES );
	for ( i = 0; i <= NH; i++ )
	{
		x = Hll + i * dh;
		pt[0].x = X0G + (int)( SFG * ( x - Hll ) );
		g_GLTable.m_pfn_qglVertex2f( pt[0].x, pt[0].y );
		g_GLTable.m_pfn_qglVertex2f( pt[0].x, pt[1].y );
	}
	g_GLTable.m_pfn_qglEnd();
	pt[0].x = X0G;
	pt[1].x = X0G + (int)( SFG * ( Hur - Hll ) );
	g_GLTable.m_pfn_qglBegin( GL_LINES );
	for ( i = 0; i <= NV; i++ )
	{
		y = Vll + i * dv;
		pt[0].y = Y0G + (int)( SFG * ( Vur - y ) );
		g_GLTable.m_pfn_qglVertex2f( pt[0].x,pt[0].y );
		g_GLTable.m_pfn_qglVertex2f( pt[1].x,pt[0].y );
	}
	g_GLTable.m_pfn_qglEnd();

	g_GLTable.m_pfn_qglLineWidth( 1 );

	// Draw axes
	pt[0].x = rc.right  - cyChar - cxChar - cyChar / 2;
	pt[0].y = rc.bottom + cyChar / 2;
	pt[1].x = pt[0].x + cyChar;
	pt[1].y = pt[0].y;
	g_GLTable.m_pfn_qglBegin( GL_LINES );
	g_GLTable.m_pfn_qglVertex2f( pt[0].x,pt[0].y );
	g_GLTable.m_pfn_qglVertex2f( pt[1].x,pt[1].y );
	g_GLTable.m_pfn_qglEnd();
	switch ( Plane )
	{
	case PLANE_YZ0:
	case PLANE_YZ1:
		texfont_write( "Y", pt[1].x, pt[1].y + cyChar / 2 );
		break;
	default:
		texfont_write( "X", pt[1].x, pt[1].y + cyChar / 2 );
	}
	pt[1].x = pt[0].x;
	pt[1].y = pt[0].y + cyChar;
	g_GLTable.m_pfn_qglBegin( GL_LINES );
	g_GLTable.m_pfn_qglVertex2f( pt[0].x,pt[0].y );
	g_GLTable.m_pfn_qglVertex2f( pt[1].x,pt[1].y );
	g_GLTable.m_pfn_qglEnd();
	switch ( Plane )
	{
	case PLANE_XY0:
	case PLANE_XY1:
		texfont_write( "Y", pt[1].x - cyChar / 2, pt[1].y + cyChar );
		break;
	default:
		texfont_write( "Z", pt[1].x - cyChar / 2, pt[1].y + cyChar );
	}

	// Denote fixed points with a 5x5 red rectangle
	for ( i = 0; i <= NH; i++ )
	{
		for ( j = 0; j <= NV; j++ )
		{
			if ( xyz[i][j].fixed ) {
				x = Hll + i * dh;
				y = Vll + j * dv;
				rcBox.left   = X0G + (int)( SFG * ( x - Hll ) ) - 2;
				rcBox.top    = Y0G + (int)( SFG * ( Vur - y ) ) + 2;
				rcBox.right  = rcBox.left + 5;
				rcBox.bottom = rcBox.top  - 5;

				DRAW_QUAD( rcBox, 1,0,0 );
			}
		}
	}

	// Denote currently selected point with a 5x5 green rectangle
	if ( NumVerticesSelected ) {
		for ( k = 0; k < NumVerticesSelected; k++ )
		{
			x = Hll + Vertex[k].i * dh;
			y = Vll + Vertex[k].j * dv;
			rcBox.left   = X0G + (int)( SFG * ( x - Hll ) ) - 2;
			rcBox.top    = Y0G + (int)( SFG * ( Vur - y ) ) + 2;
			rcBox.right  = rcBox.left + 5;
			rcBox.bottom = rcBox.top  - 5;

			DRAW_QUAD( rcBox, 0,1,0 );
		}
	}

	// Unmovable vertices
	for ( i = 0; i <= NH; i++ )
	{
		for ( j = 0; j <= NV; j++ )
		{
			if ( !CanEdit( i,j ) ) {
				x = Hll + i * dh;
				y = Vll + j * dv;
				rcBox.left   = X0G + (int)( SFG * ( x - Hll ) ) - 2;
				rcBox.top    = Y0G + (int)( SFG * ( Vur - y ) ) + 2;
				rcBox.right  = rcBox.left + 5;
				rcBox.bottom = rcBox.top  - 5;

				DRAW_QUAD( rcBox, 1,1,0 );
			}
		}
	}

	// Legend
	rcBox.left   = rc.left + cxChar / 2 - 2;
	rcBox.top    = rc.top  - cyChar / 2 - 2;
	rcBox.right  = rcBox.left + 5;
	rcBox.bottom = rcBox.top  - 5;
	DRAW_QUAD( rcBox, 1,0,0 );
	texfont_write( "Fixed points", rcBox.right + cxChar,rcBox.top - 4 + cyChar / 2 );

	rcBox.top    -= cyChar;
	rcBox.bottom -= cyChar;
	DRAW_QUAD( rcBox, 1,1,0 );
	texfont_write( "Not movable", rcBox.right + cxChar, rcBox.top - 4 + cyChar / 2 );

	rcBox.top    -= cyChar;
	rcBox.bottom -= cyChar;
	DRAW_QUAD( rcBox, 0,1,0 );
	texfont_write( "Selected", rcBox.right + cxChar, rcBox.top - 4 + cyChar / 2 );
}

//=============================================================
void GetScaleFactor( RECT rc ){
#ifdef ISOMETRIC
	double h, w;

	w = (double)( rc.right - rc.left + 1 ) - cxChar;
	h = (double)( rc.top - rc.bottom + 1 ) - cxChar;

	SF = w / ( ( XHi - XLo ) * COSXA + ( YHi - YLo ) * COSYA );
	SF = min( SF, h / ( ( XHi - XLo ) * SINXA + ( YHi - YLo ) * SINYA + ZHi - ZLo ) );
	// Center drawing
	X0 = (int)( rc.left + rc.right - (int)( SF * ( ( XHi - XLo ) * COSXA + ( YHi - YLo ) * COSYA ) ) ) / 2;
	Y0 = (int)( rc.top + rc.bottom - (int)( SF * ( ( XHi - XLo ) * SINXA + ( YHi - YLo ) * SINYA + ZHi - ZLo ) ) ) / 2;

#else
	double h, w;

	w = (double)( rc.right - rc.left + 1 ) - cxChar;
	h = (double)( rc.top - rc.bottom + 1 ) - cxChar;

	SF = w / ( Hhi - Hlo );
	SF = min( SF, h / ( Vhi - Vlo ) );
	X0 = (int)( rc.left + rc.right - (int)( SF * ( Hhi - Hlo ) ) ) / 2;
	Y0 = (int)( rc.top + rc.bottom + (int)( SF * ( Vhi - Vlo ) ) ) / 2;
#endif
}

//=============================================================
void Scale( RECT rc,XYZ xyz,POINT *pt ){

#ifdef ISOMETRIC

	pt[0].x = X0 + (int)( SF * ( ( xyz.p[0] - XLo ) * COSXA +
								 ( YHi - xyz.p[1] ) * COSYA   ) );
	pt[0].y = Y0 + (int)( SF * (  ZHi - xyz.p[2] +
								  ( YHi - xyz.p[1] ) * SINYA +
								  ( XHi - xyz.p[0] ) * SINXA   ) );
#else
	pt[0].x = X0 + (int)( SF * ( xyz.pp[0] - Hlo ) );
	pt[0].y = Y0 - (int)( SF * ( Vhi - xyz.pp[1] ) );
#endif

}

#ifndef ISOMETRIC
/* ======================================================================= */
void project( XYZ *v ){
	// project a 3D point (x,y,z) onto view plane
	double x, y, z, xa, ya, za;

	x = v->p[0];
	y = v->p[1];
	z = v->p[2];

	// yaw
	xa = ct[0] * x - st[0] * z;
	za = st[0] * x + ct[0] * z;

	// roll
	x  = ct[1] * xa + st[1] * y;
	ya = ct[1] * y  - st[1] * xa;

	// azimuth
	z  = ct[2] * za - st[2] * ya;
	y  = ct[2] * ya + st[2] * za;

	// horizontal and vertical projections:
//	v->pp[0] = D*x/z;
//	v->pp[1] = D*y/z;
	v->pp[0] = -y;
	v->pp[1] =  x;
	v->pp[2] =  z;

	// NOTE: if perspective transformation is desired,
	// set "persp" to the range from the surface,
	// then:
	// v->projected_h = -v->projected_h * persp/(v->projected_z-persp);
	// v->projected_v = -v->projected_v * persp/(v->projected_z-persp);
}
/*=======================================================================*/
void evaluate(){
	int i, j;
	XYZ v[4];

	if ( elevation > PI ) {
		elevation -= 2. * PI;
	}
	roll = elevation * sin( azimuth );
	yaw  = 1.5 * PI + elevation*cos( azimuth );

	//	Find angles from midpoint to viewpoint:
	st[0] = sin( yaw );
	st[1] = sin( roll );
	st[2] = sin( azimuth );
	ct[0] = cos( yaw );
	ct[1] = cos( roll );
	ct[2] = cos( azimuth );

	for ( i = 0; i <= NH; i++ )
	{
		for ( j = 0; j <= NV; j++ )
		{
			project( &xyz[i][j] );
		}
	}

	Hhi = xyz[0][0].pp[0];
	Hlo = Hhi;
	Vhi = xyz[0][0].pp[1];
	Vlo = Vhi;
	for ( i = 0; i <= NH; i++ )
	{
		for ( j = 0; j <= NV; j++ )
		{
			Hlo = min( Hlo,xyz[i][j].pp[0] );
			Hhi = max( Hhi,xyz[i][j].pp[0] );
			Vlo = min( Vlo,xyz[i][j].pp[1] );
			Vhi = max( Vhi,xyz[i][j].pp[1] );
		}
	}

	// Include backface in min-max
	VectorCopy( xyz[ 0][ 0].p,v[0].p );
	VectorCopy( xyz[NH][ 0].p,v[1].p );
	VectorCopy( xyz[NH][NV].p,v[2].p );
	VectorCopy( xyz[ 0][NV].p,v[3].p );
	switch ( Plane )
	{
	case PLANE_XZ0:
	case PLANE_XZ1:
		v[0].p[1] = backface;
		v[1].p[1] = v[0].p[1];
		v[2].p[1] = v[0].p[1];
		v[3].p[1] = v[0].p[1];
		break;
	case PLANE_YZ0:
	case PLANE_YZ1:
		v[0].p[0] = backface;
		v[1].p[0] = v[0].p[0];
		v[2].p[0] = v[0].p[0];
		v[3].p[0] = v[0].p[0];
		break;
	default:
		v[0].p[2] = backface;
		v[1].p[2] = v[0].p[2];
		v[2].p[2] = v[0].p[2];
		v[3].p[2] = v[0].p[2];
	}
	for ( i = 0; i <= 3; i++ )
	{
		project( &v[i] );
		Hlo = min( Hlo,v[i].pp[0] );
		Hhi = max( Hhi,v[i].pp[0] );
		Vlo = min( Vlo,v[i].pp[1] );
		Vhi = max( Vhi,v[i].pp[1] );
	}

}
#endif
