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
// interface for all-purpose messaging and UI
// window class for MFC, Gtk or Q3 UI
// each version of Radiant implements the API, using the native code that it needs

#ifndef __IUI_H_
#define __IUI_H_

// this one can be hooked in the GL window procs for customizing GUI through plugins
// the class is implemented by the plugin module, and given to Radiant who calls into it
class IWindowListener
{
public:
virtual ~IWindowListener() { }
// Increment the number of references to this object
virtual void IncRef() = 0;
// Decrement the reference count
virtual void DecRef() = 0;
// since Radiant is MFC we don't use a WNDPROC, we wrap the MFC handlers
// the handler is called first, if returns false Radiant continues processing
//++timo maybe add more later ? OnKeyUp and OnKeyDown for instance
//++timo TODO: add handlers everywhere
// Gef: Changed 2nd & 3rd params to gdouble's for sub-integer grid sizes
virtual bool OnLButtonDown( guint32 nFlags, gdouble x, gdouble y ) = 0;
virtual bool OnMButtonDown( guint32 nFlags, gdouble x, gdouble y ) = 0;
virtual bool OnRButtonDown( guint32 nFlags, gdouble x, gdouble y ) = 0;
virtual bool OnLButtonUp( guint32 nFlags, gdouble x, gdouble y ) = 0;
virtual bool OnMButtonUp( guint32 nFlags, gdouble x, gdouble y ) = 0;
virtual bool OnRButtonUp( guint32 nFlags, gdouble x, gdouble y ) = 0;
virtual bool OnMouseMove( guint32 nFlags, gdouble x, gdouble y ) = 0;
virtual bool OnKeyPressed( char *s ) = 0;

// paint message, the caller makes the GL context current, calls Paint, then swaps GL buffers
// return value might be false if something failed and closure is requested .. then the buffer swap will be cancelled
virtual bool Paint() = 0;
// window is closing (nothing you can do, just telling)
virtual void Close() = 0;
};

// IWindowListener with additional properties
// NOTE: for now it is both a window and the GL widget
//   in the case of Gtk, there are two widgets, the window widget (a container) and the GL widget
class IWindow
{
public:
virtual ~IWindow() {}
// Increment the number of references to this object
virtual void IncRef() = 0;
// Decrement the reference count
virtual void DecRef() = 0;
// misc data ------------------------------------------------
// get pixel size
virtual int getHeight() = 0;
virtual int getWidth() = 0;
// initialisation stuff -------------------------------------
// set pixel size and other parameters before showing it
virtual void setSizeParm( int width, int height ) = 0;
// set the IWindowListener (implemented by the plugin using this window)
virtual void setListener( IWindowListener * ) = 0;
// set the window name
virtual void setName( char * ) = 0;
// will actually create the GL and the window based on the parameters
virtual bool Show() = 0;
// commands -------------------------------------------------
// call this to ask for a Redraw
virtual void Redraw() = 0;
};

// various Radiant messages --------
// this one holds the total number of supported messages (this is used to allocate structs)
#define RADIANT_MSGCOUNT 5
// they start with a 0, can be indexed in an array
// something was selected / deselected
#define RADIANT_SELECTION 0
// a brush face was selected / deselected
#define RADIANT_SFACE     1
// current texture / shader changed
#define RADIANT_TEXTURE   2
// Radiant is going to enter "sleep mode" (all GL contexts will be destroyed)
#define RADIANT_SLEEP     3
// Radiant has left "sleep mode" (GL contexts are recreated)
#define RADIANT_WAKEUP    4


// this one can be used to listen for Radiant-specific events, not related to a window
class IListener
{
public:
virtual ~IListener() {}
// Increment the number of references to this object
virtual void IncRef() = 0;
// Decrement the reference count
virtual void DecRef() = 0;
// message is one of the RADIANT_* consts
virtual void DispatchRadiantMsg( int Msg ) = 0;
};

// this one is provided by Radiant, it's a wrapper for some usefull functions
class IXYWndWrapper
{
public:
virtual ~IXYWndWrapper() {}
virtual void SnapToGrid( int x1, int y1, vec3_t pt ) = 0;
virtual VIEWTYPE GetViewType( void ) = 0;
};

#define UI_MAJOR "ui"

// create an IWindow with GL context
typedef IWindow* ( WINAPI * PFN_QERAPP_CREATEGLWINDOW )();

// will hook the given IWindowListener to the XY window and increment the ref count
//++timo TODO: add hooking in the CAM view and Z view
typedef void ( WINAPI * PFN_QERAPP_HOOKWINDOW )( IWindowListener * );
// will unhook the given IWindowListener
typedef void ( WINAPI * PFN_QERAPP_UNHOOKWINDOW )( IWindowListener * );
// to retrieve the IXYWndWrapper
typedef IXYWndWrapper* ( WINAPI * PFN_QERAPP_GETXYWNDWRAPPER )();

// will hook a given listener into Radiant listening for the given message and increment ref count
// call several times to listen for several messages
typedef void ( WINAPI * PFN_QERAPP_HOOKLISTENER )( IListener *, int Msg );
// will unhook the listener and return the number of messages the given listener was removed from
typedef int ( WINAPI * PFN_QERAPP_UNHOOKLISTENER )( IListener * );

// TODO: create GL widget, destroy it

struct _QERUITable
{
	int m_nSize;
	PFN_QERAPP_CREATEGLWINDOW m_pfnCreateGLWindow;
	PFN_QERAPP_HOOKWINDOW m_pfnHookWindow;
	PFN_QERAPP_UNHOOKWINDOW m_pfnUnHookWindow;
	PFN_QERAPP_GETXYWNDWRAPPER m_pfnGetXYWndWrapper;
	PFN_QERAPP_HOOKLISTENER m_pfnHookListener;
	PFN_QERAPP_UNHOOKLISTENER m_pfnUnHookListener;
};

#endif
