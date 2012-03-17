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

// DPlane.h: interface for the DPlane class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_DPLANE_H__FC37C021_F0A1_11D4_ACF7_004095A18133__INCLUDED_ )
#define AFX_DPLANE_H__FC37C021_F0A1_11D4_ACF7_004095A18133__INCLUDED_

#include "DPoint.h"

#define FACE_DETAIL 0x8000000

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DWinding;

class DPlane
{
public:
DPlane( vec3_t va, vec3_t vb, vec3_t vc, const char* textureName, bool bDetail );
void ScaleTexture();
DWinding* BaseWindingForPlane();

void Rebuild();

bool AddToBrush_t( brush_t *brush );
bool operator !=( DPlane& other );
bool operator ==( DPlane& other );

bool IsRedundant( list<DPoint*>& pointList );
bool PlaneIntersection( DPlane* pl1, DPlane* pl2, vec3_t out );;

vec_t DistanceToPoint( vec3_t pnt );

DPlane( vec3_t va, vec3_t vb, vec3_t vc, _QERFaceData* texData );
DPlane() { }
virtual ~DPlane();

bool m_bChkOk;
_QERFaceData texInfo;
vec3_t points[3];           // djbob:do we really need these any more?
vec3_t normal;
float _d;
};

//typedef CList<DPlane*, DPlane*> DPlaneList;
#endif // !defined(AFX_DPLANE_H__FC37C021_F0A1_11D4_ACF7_004095A18133__INCLUDED_)
