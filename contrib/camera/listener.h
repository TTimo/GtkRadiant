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

class CListener : public IWindowListener
{
public:
bool OnMouseMove( guint32 nFlags, gdouble x, gdouble y );
bool OnLButtonDown( guint32 nFlags, gdouble x, gdouble y );
bool OnMButtonDown( guint32 nFlags, gdouble x, gdouble y );
bool OnRButtonDown( guint32 nFlags, gdouble x, gdouble y );
bool OnLButtonUp( guint32 nFlags, gdouble x, gdouble y );
bool OnMButtonUp( guint32 nFlags, gdouble x, gdouble y );
bool OnRButtonUp( guint32 nFlags, gdouble x, gdouble y );
bool OnKeyPressed( char *s ) { return false; }
bool Paint() { return true; }
void Close() { }

void UnRegister();
void Register();
CListener();
virtual ~CListener();

void IncRef() { refCount++; }
void DecRef() {
	refCount--; if ( refCount <= 0 ) {
		delete this;
	}
}

void SetViewType( VIEWTYPE vt ) {
	if ( m_vt != vt ) {
		oldValid = false;
	}
	m_vt = vt;
}

private:
IXYWndWrapper *g_pXYWndWrapper;

bool m_bHooked;
int refCount;
VIEWTYPE m_vt;

// mouse button status
bool m_bLeftMBPressed, m_bRightMBPressed, m_bMiddleMBPressed;

// old mouse coordinates
bool oldValid;
gdouble old_x, old_y;
};
