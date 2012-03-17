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

// DPlane.cpp: implementation of the DPlane class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DPlane.h"
#include "DWinding.h"
#include "misc.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DPlane::DPlane( vec3_t va, vec3_t vb, vec3_t vc, _QERFaceData* texData ){
	MakeNormal( va, vb, vc, normal );
	if ( VectorNormalize( normal, normal ) == 0 ) {  // normalizes and returns length
		Sys_ERROR( "DPlane::DPlane: Bad Normal.\n" );
	}

	_d = ( normal[0] * va[0] ) + ( normal[1] * va[1] ) + ( normal[2] * va[2] );

	VectorCopy( va, points[0] );
	VectorCopy( vb, points[1] );
	VectorCopy( vc, points[2] );

	m_bChkOk = TRUE;

	if ( texData ) {
		memcpy( &texInfo, texData, sizeof( _QERFaceData ) );
	}
	else{
		FillDefaultTexture( &texInfo, points[0], points[1], points[2], "textures/common/caulk" );
	}
}

DPlane::~DPlane(){

}

//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

vec_t DPlane::DistanceToPoint( vec3_t pnt ){
	vec3_t tmp;
	VectorSubtract( pnt, points[0], tmp );
	return DotProduct( tmp, normal );
}

bool DPlane::PlaneIntersection( DPlane *pl1, DPlane *pl2, vec3_t out ){
	float a1, a2, a3;
	float b1, b2, b3;
	float c1, c2, c3;

	a1 = normal[0];         a2 = normal[1];         a3 = normal[2];
	b1 = pl1->normal[0];    b2 = pl1->normal[1];    b3 = pl1->normal[2];
	c1 = pl2->normal[0];    c2 = pl2->normal[1];    c3 = pl2->normal[2];

	float d = Determinant3x3( a1, a2, a3, b1, b2, b3, c1, c2, c3 );

	if ( d == 0 ) {
		return FALSE;
	}

	float v1 = _d;
	float v2 = pl1->_d;
	float v3 = pl2->_d;

	float d1 = Determinant3x3( v1, a2, a3, v2, b2, b3, v3, c2, c3 );
	float d2 = Determinant3x3( a1, v1, a3, b1, v2, b3, c1, v3, c3 );
	float d3 = Determinant3x3( a1, a2, v1, b1, b2, v2, c1, c2, v3 );

	out[0] = d1 / d;
	out[1] = d2 / d;
	out[2] = d3 / d;

	return TRUE;
}

bool DPlane::IsRedundant( list<DPoint*>& pointList ){
	int cnt = 0;

	//list<DPoint *>::const_iterator point=pointList.begin();
	for ( list<DPoint *>::const_iterator point = pointList.begin(); point != pointList.end(); point++ )
	{
		if ( fabs( DistanceToPoint( ( *point )->_pnt ) ) < MAX_ROUND_ERROR ) {
			cnt++;
		}

		if ( cnt == 3 ) {
			return FALSE;
		}
	}
	return TRUE;
}

bool DPlane::operator ==( DPlane& other ){
	vec3_t chk;
	VectorSubtract( other.normal, normal, chk );
	if ( fabs( VectorLength( chk ) ) > MAX_ROUND_ERROR ) {
		return FALSE;
	}

	if ( fabs( other._d - _d ) > MAX_ROUND_ERROR ) {
		return FALSE;
	}

	return TRUE;
}

bool DPlane::operator !=( DPlane& other ){
	vec3_t chk;
	VectorAdd( other.normal, normal, chk );
	if ( fabs( VectorLength( chk ) ) > MAX_ROUND_ERROR ) {
		return FALSE;
	}

	return TRUE;
}

DWinding* DPlane::BaseWindingForPlane(){
	int i, x;
	vec_t max, v;
	vec3_t org, vright, vup;

// find the major axis

	max = -131072;
	x = -1;
	for ( i = 0 ; i < 3; i++ )
	{
		v = (float)fabs( normal[i] );
		if ( v > max ) {
			x = i;
			max = v;
		}
	}
	if ( x == -1 ) {
		Sys_Printf( "BaseWindingForPlane: no axis found" );
	}

	VectorCopy( vec3_origin, vup );
	switch ( x )
	{
	case 0:
	case 1:
		vup[2] = 1;
		break;
	case 2:
		vup[0] = 1;
		break;
	}

	v = DotProduct( vup, normal );
	VectorMA( vup, -v, normal, vup );
	VectorNormalize( vup, vup );

	VectorScale( normal, _d, org );

	CrossProduct( vup, normal, vright );

	VectorScale( vup, 131072, vup );
	VectorScale( vright, 131072, vright );

// project a really big	axis aligned box onto the plane
	DWinding* w = new DWinding;
	w->AllocWinding( 4 );

	VectorSubtract( org, vright, w->p[0] );
	VectorAdd( w->p[0], vup, w->p[0] );

	VectorAdd( org, vright, w->p[1] );
	VectorAdd( w->p[1], vup, w->p[1] );

	VectorAdd( org, vright, w->p[2] );
	VectorSubtract( w->p[2], vup, w->p[2] );

	VectorSubtract( org, vright, w->p[3] );
	VectorSubtract( w->p[3], vup, w->p[3] );

	return w;
}

void DPlane::Rebuild(){
	vec3_t v1, v2;
	VectorSubtract( points[0], points[1], v1 );
	VectorSubtract( points[2], points[1], v2 );
	CrossProduct( v1, v2, normal );

	if ( VectorNormalize( normal, normal ) == 0 ) {  // normalizes and returns length
		Sys_ERROR( "DPlane::Rebuild: Bad Normal.\n" );
	}

	_d = ( normal[0] * points[0][0] ) + ( normal[1] * points[0][1] ) + ( normal[2] * points[0][2] );

	VectorCopy( points[0], texInfo.m_v1 );
	VectorCopy( points[1], texInfo.m_v2 );
	VectorCopy( points[2], texInfo.m_v3 );
}

bool DPlane::AddToBrush_t( brush_t *brush ){
	if ( m_bChkOk || !strcmp( texInfo.m_TextureName, "textures/common/caulk" ) ) {
		g_FuncTable.m_pfnAddFaceData( brush, &texInfo );
		return FALSE;
	}

	strcpy( texInfo.m_TextureName, "textures/common/caulk" );
	g_FuncTable.m_pfnAddFaceData( brush, &texInfo );
	return TRUE;
}

void DPlane::ScaleTexture()
{ }

DPlane::DPlane( vec3_t va, vec3_t vb, vec3_t vc, const char* textureName, bool bDetail ){
	vec3_t v1, v2;
	VectorSubtract( va, vb, v1 );
	VectorSubtract( vc, vb, v2 );
	CrossProduct( v1, v2, normal );

	if ( VectorNormalize( normal, normal ) == 0 ) {  // normalizes and returns length
		Sys_ERROR( "DPlane::DPlane: Bad Normal.\n" );
	}

	_d = ( normal[0] * va[0] ) + ( normal[1] * va[1] ) + ( normal[2] * va[2] );

	VectorCopy( va, points[0] );
	VectorCopy( vb, points[1] );
	VectorCopy( vc, points[2] );

	m_bChkOk = TRUE;

	FillDefaultTexture( &texInfo, points[0], points[1], points[2], textureName );
	if ( bDetail ) {
		texInfo.m_nContents |= FACE_DETAIL;
	}
}
