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

// DBobView.h: interface for the DBobView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_BOBVIEW_H__6E36062A_EF0B_11D4_ACF7_004095A18133__INCLUDED_ )
#define AFX_BOBVIEW_H__6E36062A_EF0B_11D4_ACF7_004095A18133__INCLUDED_

class DListener;

#define BOUNDS_ALL  0
#define BOUNDS_APEX 1

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DBobView :
	public IGL2DWindow,
	public IGL3DWindow
{
public:
DBobView();
virtual ~DBobView();

protected:
vec3_t* path;
int refCount;
public:
bool m_bShowExtra;
int boundingShow;
DListener* eyes;
float fVarGravity;

bool UpdatePath();
char entTarget[256];
char entTrigger[256];
void Begin( const char*, const char*, float, int, float, bool, bool );
bool CalculateTrajectory( vec3_t, vec3_t, float, int, float );

void SetPath( vec3_t* pPath );
void UnRegister();
void Register();
void Draw3D();
void Draw2D( VIEWTYPE vt );
void IncRef() { refCount++; }
void DecRef() {
	refCount--; if ( refCount <= 0 ) {
		delete this;
	}
}

float fMultiplier;
bool m_bHooked;
int nPathCount;
};

#endif // !defined(AFX_BOBVIEW_H__6E36062A_EF0B_11D4_ACF7_004095A18133__INCLUDED_)
