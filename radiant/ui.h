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

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
// headers for internal classes used in Messaging.cpp
//

#ifndef __MESSAGING_H_
#define __MESSAGING_H_

class CXYWndWrapper : public IXYWndWrapper
{
public:
virtual ~CXYWndWrapper() {}
void SnapToGrid( int x1, int y1, vec3_t pt );
VIEWTYPE GetViewType( void );
};

// implementation of the IWindow API
class CGtkWindow : public IWindow
{
int refCount;
GtkWidget *m_pWnd;
GtkWidget *m_pGLWidget;
int m_nWidthParam,m_nHeightParam;
IWindowListener *m_pListen;
Str m_Name;
public:
CGtkWindow() { refCount = 0; m_pWnd = NULL; m_pGLWidget = NULL; m_nWidthParam = 0; m_nHeightParam = 0; m_pListen = 0; m_Name = "CGtkWindow"; }
virtual ~CGtkWindow(){
	if ( m_pListen ) {
		m_pListen->DecRef(); m_pListen = NULL;
	}
	if ( m_pWnd ) {
		gtk_widget_destroy( m_pWnd ); m_pWnd = NULL;
	}
}
// refcounting ----------------------------------------
// Increment the number of references to this object
void IncRef() { refCount++; }
// Decrement the reference count
void DecRef(){
	if ( --refCount <= 0 ) {
		delete this;
	}
}
// IWindow --------------------------------------------
// get pixel size
int getHeight() { return m_pWnd->allocation.height; }
int getWidth() { return m_pWnd->allocation.width; }
// set pixel size and other parameters before showing it
void setSizeParm( int width, int height ) { m_nWidthParam = width; m_nHeightParam = height; }
// set the IWindowListener (implemented by the plugin using this window)
void setListener( IWindowListener * pListen ) { m_pListen = pListen; m_pListen->IncRef(); }
// set the name (optional)
void setName( char *name ) { m_Name = name; }
// will actually create the GL and the window based on the parameters
bool Show();
// CGtkWindow -----------------------------------------
// called upon a closure of the widget
void Close();
// called to manage GL context and buffer swapping before display
void DoExpose();
// commands -------------------------------------------
// call this to ask for a Redraw
void Redraw();
};

#endif
