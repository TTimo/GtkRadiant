/*
   Copyright (c) 2001, Loki software, inc.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of Loki software nor the names of its contributors may be used
   to endorse or promote products derived from this software without specific prior
   written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT,INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// GLWindow - Base class for the small views used by Radiant
//
// Leonardo Zide (leo@lokigames.com
//

#include "stdafx.h"
#include "glwidget.h"
#include "glwindow.h"

// =============================================================================
// static functions

static void realize( GtkWidget *widget, gpointer data ){
	GLWindow *wnd = (GLWindow*)data;

	wnd->OnCreate();
}

static gint expose( GtkWidget *widget, GdkEventExpose *event, gpointer data ){
	GLWindow *wnd = (GLWindow*)data;

#ifndef _WIN32
	if ( event->count > 0 ) {
		return TRUE;
	}
#endif

	if ( !g_pParentWnd->IsSleeping() ) {
		wnd->OnExpose();
	}

	return TRUE;
}

static void button_press( GtkWidget *widget, GdkEventButton *event, gpointer data ){
	GLWindow *wnd = (GLWindow*)data;
	guint32 flags = 0;

	gdk_pointer_grab( gtk_widget_get_window( widget ), FALSE,
					  (GdkEventMask)( GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK ),
					  NULL, NULL, GDK_CURRENT_TIME );

	gtk_window_set_focus( GTK_WINDOW( g_pParentWnd->m_pWidget ), widget );

	switch ( event->button )
	{
	case 1: flags |= MK_LBUTTON; break;
	case 2: flags |= MK_MBUTTON; break;
	case 3: flags |= MK_RBUTTON; break;
#if !GTK_CHECK_VERSION( 1,3,0 )
	case 4: wnd->OnMouseWheel( true, (int)event->x, (int)event->y ); break;
	case 5: wnd->OnMouseWheel( false, (int)event->x, (int)event->y ); break;
#endif
	}

	if ( ( event->state & GDK_CONTROL_MASK ) != 0 ) {
		flags |= MK_CONTROL;
	}

	if ( ( event->state & GDK_SHIFT_MASK ) != 0 ) {
		flags |= MK_SHIFT;
	}

	if ( event->type == GDK_BUTTON_PRESS ) {
		switch ( event->button )
		{
		case 1:
			wnd->OnLButtonDown( flags, (int)event->x, (int)event->y ); break;
		case 2:
			wnd->OnMButtonDown( flags, (int)event->x, (int)event->y ); break;
		case 3:
			wnd->OnRButtonDown( flags, (int)event->x, (int)event->y ); break;
		}
	}
	else if ( event->type == GDK_2BUTTON_PRESS ) {
		// do nothing
	}
}

static void button_release( GtkWidget *widget, GdkEventButton *event, gpointer data ){
	GLWindow *wnd = (GLWindow*)data;
	guint32 flags = 0;

	gdk_pointer_ungrab( GDK_CURRENT_TIME );

	if ( ( event->state & GDK_CONTROL_MASK ) != 0 ) {
		flags |= MK_CONTROL;
	}

	if ( ( event->state & GDK_SHIFT_MASK ) != 0 ) {
		flags |= MK_SHIFT;
	}

	switch ( event->button )
	{
	case 1:
		wnd->OnLButtonUp( flags, (int)event->x, (int)event->y ); break;
	case 2:
		wnd->OnMButtonUp( flags, (int)event->x, (int)event->y ); break;
	case 3:
		wnd->OnRButtonUp( flags, (int)event->x, (int)event->y ); break;
	}
}

static void motion( GtkWidget *widget, GdkEventMotion *event, gpointer data ){
	GLWindow *wnd = (GLWindow*)data;
	guint32 flags = 0;

	if ( ( event->state & GDK_BUTTON1_MASK ) != 0 ) {
		flags |= MK_LBUTTON;
	}

	if ( ( event->state & GDK_BUTTON2_MASK ) != 0 ) {
		flags |= MK_MBUTTON;
	}

	if ( ( event->state & GDK_BUTTON3_MASK ) != 0 ) {
		flags |= MK_RBUTTON;
	}

	if ( ( event->state & GDK_CONTROL_MASK ) != 0 ) {
		flags |= MK_CONTROL;
	}

	if ( ( event->state & GDK_SHIFT_MASK ) != 0 ) {
		flags |= MK_SHIFT;
	}

	wnd->OnMouseMove( flags, (int)event->x, (int)event->y );
}

static void resize( GtkWidget *widget, GtkAllocation *allocation, gpointer data ){
	GLWindow *wnd = (GLWindow*)data;
	wnd->OnSize( allocation->width, allocation->height );
}

static gint timer( gpointer data ){
	GLWindow *wnd = (GLWindow*)data;
	wnd->OnTimer();

	return TRUE;
}

static gint scroll_event( GtkWidget *widget,
						  GdkEventScroll *event,
						  gpointer data ){
	GLWindow *wnd = (GLWindow*)data;
	wnd->OnMouseWheel( ( event->direction == GDK_SCROLL_UP ) ? true : false, (int)event->x, (int)event->y );
	return TRUE;
}

// =============================================================================
// GLWindow class

#ifdef _DEBUG
//#define DBG_GLWINDOW
#endif

GLWindow::GLWindow( bool zbuffer ) {
	m_nTimer = 0;
	m_bMouseCapture = FALSE;
	m_pParent = NULL;

	m_pWidget = gtk_glwidget_new( zbuffer, g_qeglobals_gui.d_glBase );
	gtk_widget_set_can_focus( m_pWidget, TRUE );

#ifdef DBG_GLWINDOW
	Sys_Printf( "GLWindow::GLWindow m_pWidget = %p\n", m_pWidget );
#endif

	if ( g_qeglobals_gui.d_glBase == NULL ) {
		g_qeglobals_gui.d_glBase = m_pWidget;
	}

	gtk_widget_set_events( m_pWidget, GDK_DESTROY | GDK_EXPOSURE_MASK |
						   GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK );

	// Connect signal handlers
	g_signal_connect( G_OBJECT( m_pWidget ), "realize", G_CALLBACK( realize ), this );
	g_signal_connect( G_OBJECT( m_pWidget ), "expose-event", G_CALLBACK( expose ), this );
	g_signal_connect( G_OBJECT( m_pWidget ), "motion-notify-event", G_CALLBACK( motion ), this );
	g_signal_connect( G_OBJECT( m_pWidget ), "button-press-event", G_CALLBACK( button_press ), this );
	g_signal_connect( G_OBJECT( m_pWidget ), "button-release-event",G_CALLBACK( button_release ), this );
	g_signal_connect( G_OBJECT( m_pWidget ), "size-allocate", G_CALLBACK( resize ), this );
	g_signal_connect( G_OBJECT( m_pWidget ), "scroll-event", G_CALLBACK( scroll_event ), this );
}

GLWindow::~GLWindow (){
#ifdef DBG_GLWINDOW
	Sys_Printf( "GLWindow::~GLWindow m_pWidget = %p\n", m_pWidget );
#endif

	if ( m_pWidget && GTK_IS_WIDGET( m_pWidget ) ) {
		gtk_widget_destroy( m_pWidget );
	}
}

void GLWindow::DestroyContext(){
	gtk_glwidget_destroy_context( m_pWidget );
}

void GLWindow::CreateContext(){
	gtk_glwidget_create_context( m_pWidget );
}

void GLWindow::SetTimer( guint millisec ){
	m_nTimer = g_timeout_add( millisec, timer, this );
}

void GLWindow::KillTimer(){
	g_source_remove( m_nTimer );
	m_nTimer = 0;
}

bool GLWindow::MakeCurrent(){
	return gtk_glwidget_make_current( m_pWidget );
}

void GLWindow::SwapBuffers(){
	gtk_glwidget_swap_buffers( m_pWidget );
}
