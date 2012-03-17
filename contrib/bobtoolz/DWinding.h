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

// DWinding.h: interface for the DWinding class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_DWINDING_H__35B2C524_F0A7_11D4_ACF7_004095A18133__INCLUDED_ )
#define AFX_DWINDING_H__35B2C524_F0A7_11D4_ACF7_004095A18133__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DPlane;

class DWinding
{
public:
DWinding();
virtual ~DWinding();

void AllocWinding( int points );

bool ChopWinding( DPlane* chopPlane );
bool ChopWindingInPlace( DPlane* chopPlane, vec_t ON_EPSILON );
void ClipWindingEpsilon( DPlane* chopPlane, vec_t epsilon, DWinding** front, DWinding** back );

void CheckWinding();
void WindingCentre( vec3_t centre );
void WindingBounds( vec3_t mins, vec3_t maxs );
void RemoveColinearPoints();

DWinding* ReverseWinding();
DWinding* CopyWinding();
DPlane* WindingPlane();

int WindingOnPlaneSide( vec3_t normal, vec_t dist );

vec_t WindingArea();

//	members
int numpoints;
vec3_t* p;
vec3_t clr;
};

#define MAX_POINTS_ON_WINDING   64

#define ON_EPSILON  0.01

#endif // !defined(AFX_DWINDING_H__35B2C524_F0A7_11D4_ACF7_004095A18133__INCLUDED_)
