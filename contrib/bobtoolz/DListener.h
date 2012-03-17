/*
   BobToolz plugin for GtkRadiant
   Copyright (C) 2001 Gordon Biggans

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

// DListener.h: interface for the DListener class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_DLISTENER_H__53EBE342_F0B2_11D4_ACF7_004095A18133__INCLUDED_ )
#define AFX_DLISTENER_H__53EBE342_F0B2_11D4_ACF7_004095A18133__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DBobView.h"

class DListener : public IWindowListener
{
public:
DBobView* parent;

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
DListener();
virtual ~DListener();

void IncRef() { refCount++; }
void DecRef() {
	refCount--; if ( refCount <= 0 ) {
		delete this;
	}
}

private:
bool m_bHooked;
int refCount;
};

#endif // !defined(AFX_DLISTENER_H__53EBE342_F0B2_11D4_ACF7_004095A18133__INCLUDED_)
