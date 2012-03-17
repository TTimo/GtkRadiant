/*
   GenSurf plugin for GtkRadiant
   Copyright (C) 2001 David Hyde, Loki software and qeradiant.com

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

#include <stdlib.h>
#include <math.h>
#include "gensurf.h"

#define MAX_FACES 128    // Maximum number of faces on a brush
#define MAX_POINTS_ON_WINDING   64
#define SIDE_FRONT      0
#define SIDE_ON         2
#define SIDE_BACK       1
#define SIDE_CROSS      -2

vec3 gensurf_vec3_origin;

void PlaneFromPoints( float *p0, float *p1, float *p2, PLANE *plane ){
	vec3 t1, t2;
	vec length;

	VectorSubtract( p0, p1, t1 );
	VectorSubtract( p2, p1, t2 );
	plane->normal[0] = t1[1] * t2[2] - t1[2] * t2[1];
	plane->normal[1] = t1[2] * t2[0] - t1[0] * t2[2];
	plane->normal[2] = t1[0] * t2[1] - t1[1] * t2[0];

	length = (vec)( sqrt( plane->normal[0] * plane->normal[0] +
						  plane->normal[1] * plane->normal[1] +
						  plane->normal[2] * plane->normal[2]  ) );
	if ( length == 0 ) {
		VectorClear( plane->normal );
	}
	else
	{
		plane->normal[0] /= length;
		plane->normal[1] /= length;
		plane->normal[2] /= length;
	}
	plane->dist = DotProduct( p0, plane->normal );
}

void VectorMA( vec3 va, vec scale, vec3 vb, vec3 vc ){
	vc[0] = va[0] + scale * vb[0];
	vc[1] = va[1] + scale * vb[1];
	vc[2] = va[2] + scale * vb[2];
}

void CrossProduct( vec3 v1, vec3 v2, vec3 cross ){
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

/*
   =============
   AllocWinding
   =============
 */
MY_WINDING  *AllocWinding( int points ){
	MY_WINDING  *w;
	int s;

	s = sizeof( vec ) * 3 * points + sizeof( int );
	w = (MY_WINDING*)malloc( s );
	memset( w, 0, s );
	return w;
}

vec VectorNormalize( vec3 in, vec3 out ){
	vec length, ilength;

	length = (vec)( sqrt( in[0] * in[0] + in[1] * in[1] + in[2] * in[2] ) );
	if ( length == 0 ) {
		VectorClear( out );
		return 0;
	}

	ilength = (vec)1.0 / length;
	out[0] = in[0] * ilength;
	out[1] = in[1] * ilength;
	out[2] = in[2] * ilength;

	return length;
}

/*
   =================
   BaseWindingForPlane
   =================
 */
MY_WINDING *BaseWindingForPlane( vec3 normal, vec dist ){
	int i, x;
	vec max, v;
	vec3 org, vright, vup;
	MY_WINDING *w;

// find the major axis

	max = -BOGUS_RANGE;
	x = -1;
	for ( i = 0 ; i < 3; i++ )
	{
		v = (vec)( fabs( normal[i] ) );
		if ( v > max ) {
			x = i;
			max = v;
		}
	}
	if ( x == -1 ) {
		x = 2;
	}

	VectorCopy( gensurf_vec3_origin,vup );
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

	VectorScale( normal, dist, org );

	CrossProduct( vup, normal, vright );

	VectorScale( vup, 65536, vup );
	VectorScale( vright, 65536, vright );

// project a really big	axis aligned box onto the plane
	w = AllocWinding( 4 );

	VectorSubtract( org, vright, w->p[0] );
	VectorAdd( w->p[0], vup, w->p[0] );

	VectorAdd( org, vright, w->p[1] );
	VectorAdd( w->p[1], vup, w->p[1] );

	VectorAdd( org, vright, w->p[2] );
	VectorSubtract( w->p[2], vup, w->p[2] );

	VectorSubtract( org, vright, w->p[3] );
	VectorSubtract( w->p[3], vup, w->p[3] );

	w->numpoints = 4;

	return w;
}

void FreeWinding( MY_WINDING *w ){
	if ( *(unsigned *)w == 0xdeaddead ) {
//		Error ("FreeWinding: freed a freed winding");
		return;
	}
	*(unsigned *)w = 0xdeaddead;

	free( w );
}

/*
   =============
   ChopWindingInPlace
   =============
 */
void ChopWindingInPlace( MY_WINDING **inout, vec3 normal, vec dist, vec epsilon ){
	MY_WINDING *in;
	vec dists[MAX_POINTS_ON_WINDING + 4];
	int sides[MAX_POINTS_ON_WINDING + 4];
	int counts[3];
	static vec dot;     // VC 4.2 optimizer bug if not static
	int i, j;
	vec        *p1, *p2;
	vec3 mid;
	MY_WINDING *f;
	int maxpts;

	in = *inout;
	counts[0] = counts[1] = counts[2] = 0;

// determine sides for each point
	for ( i = 0 ; i < in->numpoints ; i++ )
	{
		dot = DotProduct( in->p[i], normal );
		dot -= dist;
		dists[i] = dot;
		if ( dot > epsilon ) {
			sides[i] = SIDE_FRONT;
		}
		else if ( dot < -epsilon ) {
			sides[i] = SIDE_BACK;
		}
		else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	if ( !counts[0] ) {
		FreeWinding( in );
		*inout = NULL;
		return;
	}
	if ( !counts[1] ) {
		return;     // inout stays the same

	}
	maxpts = in->numpoints + 4;   // cant use counts[0]+2 because
	                              // of fp grouping errors

	f = AllocWinding( maxpts );

	for ( i = 0 ; i < in->numpoints ; i++ )
	{
		p1 = in->p[i];

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
		p2 = in->p[( i + 1 ) % in->numpoints];

		dot = dists[i] / ( dists[i] - dists[i + 1] );
		for ( j = 0 ; j < 3 ; j++ )
		{   // avoid round off error when possible
			if ( normal[j] == 1 ) {
				mid[j] = dist;
			}
			else if ( normal[j] == -1 ) {
				mid[j] = -dist;
			}
			else{
				mid[j] = p1[j] + dot * ( p2[j] - p1[j] );
			}
		}

		VectorCopy( mid, f->p[f->numpoints] );
		f->numpoints++;
	}

//	if (f->numpoints > maxpts)
//		Error ("ClipWinding: points exceeded estimate");
//	if (f->numpoints > MAX_POINTS_ON_WINDING)
//		Error ("ClipWinding: MAX_POINTS_ON_WINDING");

	FreeWinding( in );
	*inout = f;
}

void UseFaceBounds(){
	LPVOID vp;
	float Dot, BestDot;
	float planepts[3][3];
	int BestFace;
	int i, j;
	int NumFaces;
	vec3 SurfNormal;
	vec3 vmin,vmax;
	_QERFaceData *QERFaceData;
	PLANE plane[MAX_FACES * 2];
	PLANE pface;
	MY_WINDING   *w;

	switch ( Plane )
	{
	case PLANE_XY1:
		SurfNormal[0] = 0.0;
		SurfNormal[1] = 0.0;
		SurfNormal[2] = -1.0;
		break;
	case PLANE_XZ0:
		SurfNormal[0] = 0.0;
		SurfNormal[1] = 1.0;
		SurfNormal[2] = 0.0;
		break;
	case PLANE_XZ1:
		SurfNormal[0] = 0.0;
		SurfNormal[1] = -1.0;
		SurfNormal[2] = 0.0;
		break;
	case PLANE_YZ0:
		SurfNormal[0] = 1.0;
		SurfNormal[1] = 0.0;
		SurfNormal[2] = 0.0;
		break;
	case PLANE_YZ1:
		SurfNormal[0] = -1.0;
		SurfNormal[1] = 0.0;
		SurfNormal[2] = 0.0;
		break;
	default:
		SurfNormal[0] = 0.0;
		SurfNormal[1] = 0.0;
		SurfNormal[2] = 1.0;
	}

	i  = g_FuncTable.m_pfnAllocateSelectedBrushHandles();
	vp = g_FuncTable.m_pfnGetSelectedBrushHandle( 0 );
	NumFaces = g_FuncTable.m_pfnGetFaceCount( vp );

	BestFace = -1;
	BestDot  = 0.0;

	for ( i = 0; i < NumFaces; i++ )
	{
		QERFaceData = g_FuncTable.m_pfnGetFaceData( vp,i );
		planepts[0][0] = QERFaceData->m_v1[0];
		planepts[0][1] = QERFaceData->m_v1[1];
		planepts[0][2] = QERFaceData->m_v1[2];
		planepts[1][0] = QERFaceData->m_v2[0];
		planepts[1][1] = QERFaceData->m_v2[1];
		planepts[1][2] = QERFaceData->m_v2[2];
		planepts[2][0] = QERFaceData->m_v3[0];
		planepts[2][1] = QERFaceData->m_v3[1];
		planepts[2][2] = QERFaceData->m_v3[2];

		PlaneFromPoints( planepts[0], planepts[1], planepts[2], &plane[2 * i] );
		VectorSubtract( gensurf_vec3_origin, plane[2 * i].normal, plane[2 * i + 1].normal );
		plane[2 * i + 1].dist = -plane[2 * i].dist;

		Dot = DotProduct( plane[2 * i].normal,SurfNormal );
		if ( Dot > BestDot ) {
			BestDot  = Dot;
			BestFace = i;
			if ( strlen( QERFaceData->m_TextureName ) ) {
				strcpy( Texture[Game][0],QERFaceData->m_TextureName );
			}
		}
	}
	for ( i = 0; i < NumFaces; i++ )
	{
		if ( i == BestFace ) {
			continue;
		}
		QERFaceData = g_FuncTable.m_pfnGetFaceData( vp,i );
		if ( strlen( QERFaceData->m_TextureName ) ) {
			if ( strcmp( Texture[Game][0],QERFaceData->m_TextureName ) ) {
				strcpy( Texture[Game][1],QERFaceData->m_TextureName );
			}
		}
	}


	g_FuncTable.m_pfnReleaseSelectedBrushHandles();

	w = BaseWindingForPlane( plane[BestFace * 2].normal, plane[BestFace * 2].dist );

	for ( i = 0 ; i < NumFaces && w; i++ )
	{
		if ( BestFace == i ) {
			continue;
		}
		ChopWindingInPlace( &w, plane[i * 2 + 1].normal, plane[i * 2 + 1].dist, 0 );
	}
	if ( !w ) {
		return;
	}

	// Get bounding box for this face
	vmin[0] = vmax[0] = w->p[0][0];
	vmin[1] = vmax[1] = w->p[0][1];
	vmin[2] = vmax[2] = w->p[0][2];
	for ( j = 1; j < w->numpoints; j++ )
	{
		vmin[0] = min( vmin[0],w->p[j][0] );
		vmin[1] = min( vmin[1],w->p[j][1] );
		vmin[2] = min( vmin[2],w->p[j][2] );
		vmax[0] = max( vmax[0],w->p[j][0] );
		vmax[1] = max( vmax[1],w->p[j][1] );
		vmax[2] = max( vmax[2],w->p[j][2] );
	}

	FreeWinding( w );

	VectorCopy( plane[BestFace * 2].normal,pface.normal );
	pface.dist = plane[BestFace * 2].dist;
	switch ( Plane )
	{
	case PLANE_XZ0:
	case PLANE_XZ1:
		if ( pface.normal[1] == 0. ) {
			return;
		}
		Hll = vmin[0];
		Hur = vmax[0];
		Vll = vmin[2];
		Vur = vmax[2];
		Z00 = ( pface.dist - pface.normal[0] * Hll - pface.normal[2] * Vll ) / pface.normal[1];
		Z01 = ( pface.dist - pface.normal[0] * Hll - pface.normal[2] * Vur ) / pface.normal[1];
		Z10 = ( pface.dist - pface.normal[0] * Hur - pface.normal[2] * Vll ) / pface.normal[1];
		Z11 = ( pface.dist - pface.normal[0] * Hur - pface.normal[2] * Vur ) / pface.normal[1];
		break;
	case PLANE_YZ0:
	case PLANE_YZ1:
		if ( pface.normal[0] == 0. ) {
			return;
		}
		Hll = vmin[1];
		Hur = vmax[1];
		Vll = vmin[2];
		Vur = vmax[2];
		Z00 = ( pface.dist - pface.normal[1] * Hll - pface.normal[2] * Vll ) / pface.normal[0];
		Z01 = ( pface.dist - pface.normal[1] * Hll - pface.normal[2] * Vur ) / pface.normal[0];
		Z10 = ( pface.dist - pface.normal[1] * Hur - pface.normal[2] * Vll ) / pface.normal[0];
		Z11 = ( pface.dist - pface.normal[1] * Hur - pface.normal[2] * Vur ) / pface.normal[0];
		break;
	default:
		if ( pface.normal[2] == 0. ) {
			return;
		}
		Hll = vmin[0];
		Hur = vmax[0];
		Vll = vmin[1];
		Vur = vmax[1];
		Z00 = ( pface.dist - pface.normal[0] * Hll - pface.normal[1] * Vll ) / pface.normal[2];
		Z01 = ( pface.dist - pface.normal[0] * Hll - pface.normal[1] * Vur ) / pface.normal[2];
		Z10 = ( pface.dist - pface.normal[0] * Hur - pface.normal[1] * Vll ) / pface.normal[2];
		Z11 = ( pface.dist - pface.normal[0] * Hur - pface.normal[1] * Vur ) / pface.normal[2];
	}
}
