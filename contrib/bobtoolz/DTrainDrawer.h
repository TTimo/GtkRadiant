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

// DTrainDrawer.h: interface for the DTrainDrawer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_TRAINDRAWER_H__6E36062A_EF0B_11D4_ACF7_004095A18133__INCLUDED_ )
#define AFX_TRAINDRAWER_H__6E36062A_EF0B_11D4_ACF7_004095A18133__INCLUDED_

#include "DEntity.h"

#if _MSC_VER > 1000

#pragma once
#endif // _MSC_VER > 1000

typedef struct {
	char strName[64];

	vec3_t vOrigin;
} controlPoint_t;

typedef struct {
	controlPoint_t point;

	char strControl[64];
	char strTarget[64];

	list<controlPoint_t> m_pointList;
	list<DPoint> m_vertexList;

	controlPoint_t* pTarget;
} splinePoint_t;

class DTrainDrawer :
	public IGL2DWindow,
	public IGL3DWindow
{
private:
list<splinePoint_t*> m_splineList;
list<controlPoint_t*> m_pointList;
int refCount;

bool m_bHooked;
bool m_bDisplay;
public:
void UnRegister();
void Register();

DTrainDrawer();
virtual ~DTrainDrawer( void );

void Draw3D();
void Draw2D( VIEWTYPE vt );
void IncRef() { refCount++; }
void DecRef() {
	refCount--; if ( refCount <= 0 ) {
		delete this;
	}
}
void ClearSplines();
void ClearPoints();
void BuildPaths();
void AddControlPoint( const char* name, vec_t* origin );
splinePoint_t* AddSplinePoint( const char* name, const char* target, vec_t* origin );
controlPoint_t* FindControlPoint( const char* name );
};

#endif // !defined(AFX_TRAINDRAWER_H__6E36062A_EF0B_11D4_ACF7_004095A18133__INCLUDED_)
