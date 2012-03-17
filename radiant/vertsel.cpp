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

#include "stdafx.h"
//#include "qe3.h"
#include "winding.h"

int FindPoint( vec3_t point ){
	int i, j;

	for ( i = 0 ; i < g_qeglobals.d_numpoints ; i++ )
	{
		for ( j = 0 ; j < 3 ; j++ )
			if ( fabs( point[j] - g_qeglobals.d_points[i][j] ) > 0.1 ) {
				break;
			}
		if ( j == 3 ) {
			return i;
		}
	}

	VectorCopy( point, g_qeglobals.d_points[g_qeglobals.d_numpoints] );
	//qeglobals.d_points[g_qeglobals.d_numpoints] = point;
	if ( g_qeglobals.d_numpoints < MAX_POINTS - 1 ) {
		g_qeglobals.d_numpoints++;
	}

	return g_qeglobals.d_numpoints - 1;
}

//#define DBG_WNDG
int FindEdge( int p1, int p2, face_t *f ){
	int i;

	for ( i = 0 ; i < g_qeglobals.d_numedges ; i++ )
		if ( g_qeglobals.d_edges[i].p1 == p2 && g_qeglobals.d_edges[i].p2 == p1 ) {
			g_qeglobals.d_edges[i].f2 = f;
#ifdef DBG_WNDG
			Sys_Printf( "g_qeglobals.d_edges[%d].f2 = %p\n", i, f );
#endif
			return i;
		}

	g_qeglobals.d_edges[g_qeglobals.d_numedges].p1 = p1;
	g_qeglobals.d_edges[g_qeglobals.d_numedges].p2 = p2;
	g_qeglobals.d_edges[g_qeglobals.d_numedges].f1 = f;
#ifdef DBG_WNDG
	Sys_Printf( "g_qeglobals.d_edges[%d].f1 = %p\n", g_qeglobals.d_numedges, f );
#endif

	if ( g_qeglobals.d_numedges < MAX_EDGES - 1 ) {
		g_qeglobals.d_numedges++;
	}

	return g_qeglobals.d_numedges - 1;
}

void MakeFace( brush_t* b, face_t *f ){
	winding_t   *w;
	int i;
	int pnum[128];

	w = Brush_MakeFaceWinding( b, f );
	if ( !w ) {
		return;
	}
	for ( i = 0 ; i < w->numpoints ; i++ )
		pnum[i] = FindPoint( w->points[i] );
	for ( i = 0 ; i < w->numpoints ; i++ )
		FindEdge( pnum[i], pnum[( i + 1 ) % w->numpoints], f );

	free( w );
}

void SetupVertexSelection( void ){
	face_t  *f;
	brush_t *b;

	g_qeglobals.d_numpoints = 0;
	g_qeglobals.d_numedges = 0;

	for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
	{
		if ( b->patchBrush || b->owner->eclass->fixedsize ) {
			continue; // don't make edge and vertex handles for patchbrushes
		}
		for ( f = b->brush_faces ; f ; f = f->next )
			MakeFace( b,f );
	}
}

void SelectFaceEdge( brush_t* b, face_t *f, int p1, int p2 ){
	winding_t   *w;
	int i, j, k;
	int pnum[128];

#ifdef DBG_WNDG
	if ( f == NULL ) {
		Sys_Printf( "SelectFaceEdge %p %p\n", b, f );
	}
#endif

	w = Winding_Clone( f->face_winding ); //Brush_MakeFaceWinding (b, f);
	if ( !w ) {
		return;
	}
	for ( i = 0 ; i < w->numpoints ; i++ )
		pnum[i] = FindPoint( w->points[i] );

	for ( i = 0 ; i < w->numpoints ; i++ )
		if ( pnum[i] == p1 && pnum[( i + 1 ) % w->numpoints] == p2 ) {
			VectorCopy( g_qeglobals.d_points[pnum[i]], f->planepts[0] );
			VectorCopy( g_qeglobals.d_points[pnum[( i + 1 ) % w->numpoints]], f->planepts[1] );
			VectorCopy( g_qeglobals.d_points[pnum[( i + 2 ) % w->numpoints]], f->planepts[2] );
			for ( j = 0 ; j < 3 ; j++ )
			{
				for ( k = 0 ; k < 3 ; k++ )
				{
					f->planepts[j][k] = floor( f->planepts[j][k] / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;
				}
			}

			AddPlanept( f->planepts[0] );
			AddPlanept( f->planepts[1] );
			break;
		}

	if ( i == w->numpoints ) {
		Sys_Printf( "SelectFaceEdge: failed\n" );
	}
	Winding_Free( w );
}


void SelectVertex( int p1 ){
	brush_t     *b;
	winding_t   *w;
	int i;
	face_t      *f;

	for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
	{
		for ( f = b->brush_faces ; f ; f = f->next )
		{
			w =  Brush_MakeFaceWinding( b, f );
			if ( !w ) {
				continue;
			}
			for ( i = 0 ; i < w->numpoints ; i++ )
			{
				if ( FindPoint( w->points[i] ) == p1 ) {
					VectorCopy( w->points[( i + w->numpoints - 1 ) % w->numpoints], f->planepts[0] );
					VectorCopy( w->points[i], f->planepts[1] );
					VectorCopy( w->points[( i + 1 ) % w->numpoints], f->planepts[2] );
					// NOTE: used to be a planepts clamping to grid here

					AddPlanept( f->planepts[1] );

					break;
				}
			}
			free( w );
		}
	}
}

#define SELECT_EPSILON 8

void SelectVertexByRay( vec3_t org, vec3_t dir ){
	int i, besti;
	float d, bestd = VEC_MAX;
	vec_t epsilon, divergence;
	ray_t ray;
	ray_construct_for_vec3( &ray, org, dir );

	// find the point closest to the ray
	besti = -1;
	if ( ( fabs( org[0] ) == g_MaxWorldCoord || fabs( org[1] ) == g_MaxWorldCoord || fabs( org[2] ) == g_MaxWorldCoord )
		 && ( fabs( dir[0] ) == 1.0f || fabs( dir[1] ) == 1.0f || fabs( dir[2] ) == 1.0f ) ) { // very unlikely unless 2d view
		divergence = 0;
		epsilon = SELECT_EPSILON /  g_pParentWnd->GetXYWnd()->Scale(); // compensate for zoom level
	}
	else
	{
		divergence = SELECT_EPSILON / ( g_pParentWnd->GetCamWnd()->Camera()->width * 0.5 ); // radius / focal length
		epsilon = 0;
	}

	for ( i = 0 ; i < g_qeglobals.d_numpoints ; i++ )
	{
		d = ray_intersect_point( &ray, g_qeglobals.d_points[i], epsilon, divergence );

		if ( d < bestd ) {
			bestd = d;
			besti = i;
		}
	}

	if ( besti == -1 ) {
		Sys_Printf( "Click didn't hit a vertex\n" );
		return;
	}
	Sys_Printf( "hit vertex\n" );
	g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = g_qeglobals.d_points[besti];
	if ( !g_PrefsDlg.m_bVertexSplit ) {
		SelectVertex( besti );
	}
}

// TTimo: NOTE: we should not have to put extern funcs like that
//   those should be defined in qe3.h
extern void AddPatchMovePoint( vec3_t v, bool bMulti, bool bFull );
extern int PointInMoveList( float *pf );
void SelectCurvePointByRay( vec3_t org, vec3_t dir, int buttons ){
	int i, j;
	float d, bestd = VEC_MAX;
	vec3_t  *pPointBest;
	vec_t epsilon, divergence;
	ray_t ray;
	ray_construct_for_vec3( &ray, org, dir );

	// find the point closest to the ray
	pPointBest = NULL;
	if ( ( fabs( org[0] ) == g_MaxWorldCoord || fabs( org[1] ) == g_MaxWorldCoord || fabs( org[2] ) == g_MaxWorldCoord )
		 && ( fabs( dir[0] ) == 1.0f || fabs( dir[1] ) == 1.0f || fabs( dir[2] ) == 1.0f ) ) { // very unlikely unless 2d view
		divergence = 0;
		epsilon = SELECT_EPSILON /  g_pParentWnd->GetXYWnd()->Scale(); // compensate for zoom level
	}
	else
	{
		divergence = SELECT_EPSILON / ( g_pParentWnd->GetCamWnd()->Camera()->width * 0.5 ); // radius / focal length
		epsilon = 0;
	}


	g_qeglobals.d_numpoints = 0;

	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			patchMesh_t* p = pb->pPatch;

			for ( i = 0 ; i < p->width ; i++ )
			{
				for ( j = 0 ; j < p->height ; j++ )
				{
					d = ray_intersect_point( &ray, p->ctrl[i][j].xyz, epsilon, divergence );

					if ( d >= bestd ) {
						continue;
					}

					bestd = d;

					if ( PointInMoveList( *pPointBest ) != -1 && PointInMoveList( p->ctrl[i][j].xyz ) == -1 ) {
						continue; // choose selected points with preference over unselected

					}
					pPointBest = &p->ctrl[i][j].xyz;

				}
			}
		}
	}

	if ( pPointBest == NULL ) {
		if ( g_pParentWnd->ActiveXY()->AreaSelectOK() ) {
			g_qeglobals.d_select_mode = sel_area;
			VectorCopy( org, g_qeglobals.d_vAreaTL );
			VectorCopy( org, g_qeglobals.d_vAreaBR );
		}
		return;
	}
	else{
		AddPatchMovePoint( pPointBest[0], buttons & MK_CONTROL, buttons & MK_SHIFT );
	}
}

// optimization bug:
// had to use the #define DBG_WNDG to identify
// the first loop that checks the best edge is broken in release-optimized build
// unrolled the mid[] loop and forced floating consistency on seems to fix
#ifdef _WIN32
#pragma optimize( "p", on )
#endif
void SelectEdgeByRay( vec3_t org, vec3_t dir ){
	int i, besti;
	float d, bestd = VEC_MAX;
	vec3_t mid;
	pedge_t *e;
	vec_t epsilon, divergence;
	ray_t ray;
	ray_construct_for_vec3( &ray, org, dir );

	// find the edge closest to the ray
	besti = -1;
	if ( ( fabs( org[0] ) == g_MaxWorldCoord || fabs( org[1] ) == g_MaxWorldCoord || fabs( org[2] ) == g_MaxWorldCoord )
		 && ( fabs( dir[0] ) == 1.0f || fabs( dir[1] ) == 1.0f || fabs( dir[2] ) == 1.0f ) ) { // very unlikely unless 2d view
		divergence = 0;
		epsilon = SELECT_EPSILON /  g_pParentWnd->GetXYWnd()->Scale(); // compensate for zoom level
	}
	else
	{
		divergence = SELECT_EPSILON / ( g_pParentWnd->GetCamWnd()->Camera()->width * 0.5 ); // radius / focal length
		epsilon = 0;
	}

	for ( i = 0 ; i < g_qeglobals.d_numedges ; i++ )
	{
		mid[0] = 0.5f * ( g_qeglobals.d_points[g_qeglobals.d_edges[i].p1][0] + g_qeglobals.d_points[g_qeglobals.d_edges[i].p2][0] );
		mid[1] = 0.5f * ( g_qeglobals.d_points[g_qeglobals.d_edges[i].p1][1] + g_qeglobals.d_points[g_qeglobals.d_edges[i].p2][1] );
		mid[2] = 0.5f * ( g_qeglobals.d_points[g_qeglobals.d_edges[i].p1][2] + g_qeglobals.d_points[g_qeglobals.d_edges[i].p2][2] );

		d = ray_intersect_point( &ray, mid, epsilon, divergence );

#ifdef DBG_WNDG
		Sys_Printf( "d: %f\n", d );
#endif
		if ( d < bestd ) {
#ifdef DBG_WNDG
			Sys_Printf( "bestd = d\n" );
#endif
			bestd = d;
			besti = i;
		}
	}

	if ( besti == -1 ) {
		Sys_Printf( "Click didn't hit an edge\n" );
		return;
	}
	Sys_Printf( "Hit edge\n" );

	// make the two faces that border the edge use the two edge points
	// as primary drag points
	g_qeglobals.d_num_move_points = 0;
	e = &g_qeglobals.d_edges[besti];
#ifdef DBG_WNDG
	Sys_Printf( "besti: %d\n", besti );
	if ( e->f1 == NULL ) {
		Sys_Printf( "e->f1 == NULL e->f2 %p\n", e->f2 );
	}
	if ( e->f2 == NULL ) {
		Sys_Printf( "e->f1 %p e->f2 == NULL\n",e->f1 );
	}
#endif
	for ( brush_t* b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
	{
		SelectFaceEdge( b, e->f1, e->p1, e->p2 );
		SelectFaceEdge( b, e->f2, e->p2, e->p1 );
	}
}
