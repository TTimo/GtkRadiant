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

// DWinding.cpp: implementation of the DWinding class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DWinding.h"
#include "DPlane.h"
#include "misc.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DWinding::DWinding(){
	numpoints = 0;
	p = NULL;
}

DWinding::~DWinding(){
	if ( p ) {
		delete[] p;
	}
}

//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

#define BOGUS_RANGE 4096

void DWinding::AllocWinding( int points ){
	numpoints = points;
	if ( p ) {
		delete[] p;
	}
	p = new vec3_t[points];
}

vec_t DWinding::WindingArea(){
	vec3_t d1, d2, cross;
	vec_t total;

	total = 0;
	for ( int i = 2; i < numpoints ; i++ )
	{
		VectorSubtract( p[i - 1], p[0], d1 );
		VectorSubtract( p[i], p[0], d2 );

		CrossProduct( d1, d2, cross );

		total += 0.5f * VectorLength( cross );
	}

	return total;
}

void DWinding::RemoveColinearPoints(){
	vec3_t p2[MAX_POINTS_ON_WINDING];

	int nump = 0;
	for ( int i = 0; i < numpoints; i++ )
	{
		int j = ( i + 1 ) % numpoints;
		int k = ( i + numpoints - 1 ) % numpoints;

		vec3_t v1, v2;
		VectorSubtract( p[j], p[i], v1 );
		VectorSubtract( p[i], p[k], v2 );
		VectorNormalize( v1, v1 );
		VectorNormalize( v2, v2 );

		if ( DotProduct( v1, v2 ) < 0.999 ) {
			VectorCopy( p[i], p2[nump] );
			nump++;
		}
	}

	if ( nump == numpoints ) {
		return;
	}

	AllocWinding( nump );
	memcpy( p, p2, nump * sizeof( vec3_t ) );
}

DPlane* DWinding::WindingPlane(){
	DPlane* newPlane = new DPlane( p[0], p[1], p[2], NULL );
	return newPlane;
}

void DWinding::WindingBounds( vec3_t mins, vec3_t maxs ){
	if ( numpoints == 0 ) {
		return;
	}

	VectorCopy( mins, p[0] );
	VectorCopy( maxs, p[0] );

	for ( int i = 1; i < numpoints ; i++ )
	{
		for ( int j = 0; j < 3; j++ )
		{
			vec_t v = p[i][j];
			if ( v < mins[j] ) {
				mins[j] = v;
			}
			if ( v > maxs[j] ) {
				maxs[j] = v;
			}
		}
	}
}

void DWinding::WindingCentre( vec3_t centre ){
	VectorCopy( vec3_origin, centre );
	for ( int i = 0; i < numpoints; i++ )
		VectorAdd( p[i], centre, centre );

	float scale = 1.0f / numpoints;
	VectorScale( centre, scale, centre );
}


DWinding* DWinding::CopyWinding(){
	DWinding* c = new DWinding;
	c->AllocWinding( numpoints );
	memcpy( c->p, p, numpoints * sizeof( vec3_t ) );
	return c;
}


int DWinding::WindingOnPlaneSide( vec3_t normal, vec_t dist ){
	bool front = FALSE;
	bool back = FALSE;

	for ( int i = 0; i < numpoints; i++ )
	{
		vec_t d = DotProduct( p[i], normal ) - dist;
		if ( d < -ON_EPSILON ) {
			if ( front ) {
				return SIDE_CROSS;
			}
			back = TRUE;
			continue;
		}
		if ( d > ON_EPSILON ) {
			if ( back ) {
				return SIDE_CROSS;
			}
			front = TRUE;
			continue;
		}
	}

	if ( back ) {
		return SIDE_BACK;
	}
	if ( front ) {
		return SIDE_FRONT;
	}
	return SIDE_ON;
}

void DWinding::CheckWinding(){
	vec_t   *p1, *p2;
	vec_t edgedist;
	vec3_t dir, edgenormal;

	if ( numpoints < 3 ) {
		Sys_Printf( "CheckWinding: %i points", numpoints );
	}

	vec_t area = WindingArea();
	if ( area < 1 ) {
		Sys_Printf( "CheckWinding: %f area", area );
	}

	DPlane* wPlane = WindingPlane();
	int i;
	for ( i = 0; i < numpoints; i++ )
	{
		p1 = p[i];

		int j;
		for ( j = 0; j < 3; j++ )
			if ( p1[j] > BOGUS_RANGE || p1[j] < -BOGUS_RANGE ) {
				Sys_Printf( "CheckFace: BUGUS_RANGE: %f", p1[j] );
			}

		j = i + 1 == numpoints ? 0 : i + 1;

		// check the point is on the face plane
		vec_t d = DotProduct( p1, wPlane->normal ) - wPlane->_d;
		if ( d < -ON_EPSILON || d > ON_EPSILON ) {
			Sys_Printf( "CheckWinding: point off plane" );
		}

		// check the edge isnt degenerate
		p2 = p[j];
		VectorSubtract( p2, p1, dir );

		if ( VectorLength( dir ) < ON_EPSILON ) {
			Sys_Printf( "CheckWinding: degenerate edge" );
		}

		CrossProduct( wPlane->normal, dir, edgenormal );
		VectorNormalize( edgenormal, edgenormal );
		edgedist = DotProduct( p1, edgenormal );

		// all other points must be on front side
		for ( j = 0 ; j < numpoints ; j++ )
		{
			if ( j == i ) {
				continue;
			}

			d = DotProduct( p[j], edgenormal );
			if ( d > ( edgedist + ON_EPSILON ) ) {
				Sys_Printf( "CheckWinding: non-convex" );
			}
		}
	}

	delete wPlane;
}

DWinding* DWinding::ReverseWinding(){
	DWinding* c = new DWinding;
	c->AllocWinding( numpoints );

	for ( int i = 0; i < numpoints ; i++ )
		VectorCopy( p[numpoints - 1 - i], c->p[i] );

	return c;
}

bool DWinding::ChopWindingInPlace( DPlane* chopPlane, vec_t epsilon ){
	vec_t dists[MAX_POINTS_ON_WINDING + 4];
	int sides[MAX_POINTS_ON_WINDING + 4];
	int counts[3];
	vec_t   *p1, *p2;
	vec3_t mid;

	counts[0] = counts[1] = counts[2] = 0;

// determine sides for each point
	int i;
	for ( i = 0; i < numpoints; i++ )
	{
		vec_t dot = DotProduct( p[i], chopPlane->normal );
		dot -= chopPlane->_d;
		dists[i] = dot;

		if ( dot > epsilon ) {
			sides[i] = SIDE_FRONT;
		}
		else if ( dot < -epsilon ) {
			sides[i] = SIDE_BACK;
		}
		else{
			sides[i] = SIDE_ON;
		}

		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	if ( !counts[0] ) {
		delete this;
		return FALSE;
	}

	if ( !counts[1] ) {
		return TRUE;
	}

	int maxpts = numpoints + 4;   // cant use counts[0]+2 because
	                              // of fp grouping errors

	DWinding* f = new DWinding;
	f->AllocWinding( maxpts );
	f->numpoints = 0;

	for ( i = 0; i < numpoints; i++ )
	{
		p1 = p[i];

		if ( sides[i] == SIDE_ON ) {
			VectorCopy( p1, f->p[f->numpoints] );
			f->numpoints++;
			continue;
		}

		if ( sides[i] == SIDE_FRONT ) {
			VectorCopy( p1, f->p[f->numpoints] );
			f->numpoints++;
		}

		if ( sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i] ) {
			continue;
		}

		// generate a split point
		p2 = p[( i + 1 ) % numpoints];

		vec_t dot = dists[i] / ( dists[i] - dists[i + 1] );
		for ( int j = 0; j < 3; j++ )
		{
			if ( chopPlane->normal[j] == 1 ) {
				mid[j] = chopPlane->_d;
			}
			else if ( chopPlane->normal[j] == -1 ) {
				mid[j] = -chopPlane->_d;
			}
			else{
				mid[j] = p1[j] + dot * ( p2[j] - p1[j] );
			}
		}

		VectorCopy( mid, f->p[f->numpoints] );
		f->numpoints++;
	}

	if ( f->numpoints > maxpts ) {
		Sys_Printf( "ClipWinding: points exceeded estimate" );
	}
	if ( f->numpoints > MAX_POINTS_ON_WINDING ) {
		Sys_Printf( "ClipWinding: MAX_POINTS_ON_WINDING" );
	}

	delete[] p;
	p = f->p;
	f->p = NULL;
	delete f;
	return TRUE;
}

void DWinding::ClipWindingEpsilon( DPlane* chopPlane, vec_t epsilon, DWinding **front, DWinding **back ){
	vec_t dists[MAX_POINTS_ON_WINDING + 4];
	int sides[MAX_POINTS_ON_WINDING + 4];
	int counts[3];
	vec_t   *p1, *p2;
	vec3_t mid;

	counts[0] = counts[1] = counts[2] = 0;

// determine sides for each point
	int i;
	for ( i = 0; i < numpoints; i++ )
	{
		vec_t dot = -chopPlane->DistanceToPoint( p[i] );
		dists[i] = dot;

		if ( dot > epsilon ) {
			sides[i] = SIDE_FRONT;
		}
		else if ( dot < -epsilon ) {
			sides[i] = SIDE_BACK;
		}
		else{
			sides[i] = SIDE_ON;
		}

		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	*front = *back = NULL;

	if ( !counts[0] ) {
		*back = CopyWinding();
		return;
	}
	if ( !counts[1] ) {
		*front = CopyWinding();
		return;
	}

	int maxpts = numpoints + 4;   // cant use counts[0]+2 because
	                              // of fp grouping errors

	DWinding* f = new DWinding;
	DWinding* b = new DWinding;

	f->AllocWinding( maxpts );
	f->numpoints = 0;

	b->AllocWinding( maxpts );
	b->numpoints = 0;

	*front = f;
	*back = b;

	for ( i = 0; i < numpoints ; i++ )
	{
		p1 = p[i];

		if ( sides[i] == SIDE_ON ) {
			VectorCopy( p1, f->p[f->numpoints] );
			f->numpoints++;
			VectorCopy( p1, b->p[b->numpoints] );
			b->numpoints++;
			continue;
		}

		if ( sides[i] == SIDE_FRONT ) {
			VectorCopy( p1, f->p[f->numpoints] );
			f->numpoints++;
		}
		if ( sides[i] == SIDE_BACK ) {
			VectorCopy( p1, b->p[b->numpoints] );
			b->numpoints++;
		}

		if ( sides[i + 1] == SIDE_ON || sides[i + 1] == sides[i] ) {
			continue;
		}

		// generate a split point
		p2 = p[( i + 1 ) % numpoints];

		vec_t dot = dists[i] / ( dists[i] - dists[i + 1] );
		for ( int j = 0; j < 3; j++ )
		{
			if ( chopPlane->normal[j] == 1 ) {
				mid[j] = chopPlane->_d;
			}
			else if ( chopPlane->normal[j] == -1 ) {
				mid[j] = -chopPlane->_d;
			}
			else{
				mid[j] = p1[j] + dot * ( p2[j] - p1[j] );
			}
		}

		VectorCopy( mid, f->p[f->numpoints] );
		f->numpoints++;
		VectorCopy( mid, b->p[b->numpoints] );
		b->numpoints++;
	}

	if ( f->numpoints > maxpts || b->numpoints > maxpts ) {
		Sys_Printf( "ClipWinding: points exceeded estimate" );
	}
	if ( f->numpoints > MAX_POINTS_ON_WINDING || b->numpoints > MAX_POINTS_ON_WINDING ) {
		Sys_Printf( "ClipWinding: MAX_POINTS_ON_WINDING" );
	}
}

bool DWinding::ChopWinding( DPlane* chopPlane ){
	DWinding *f, *b;

	ClipWindingEpsilon( chopPlane, (float)ON_EPSILON, &f, &b );

	if ( b ) {
		delete ( b );
	}


	if ( !f ) {
		delete this;
		return FALSE;
	}

	delete[] p;
	p = f->p;
	f->p = NULL;
	numpoints = f->numpoints;
	delete f;

	return TRUE;
}
