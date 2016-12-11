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

// select.c
#include "stdafx.h"
#include <assert.h>
#include "filters.h"

// externs
CPtrArray g_SelectedFaces;
CPtrArray g_SelectedFaceBrushes;
CPtrArray& g_ptrSelectedFaces = g_SelectedFaces;
CPtrArray& g_ptrSelectedFaceBrushes = g_SelectedFaceBrushes;

/*
   ===========
   Test_Ray
   ===========
 */
#define DIST_START  999999
trace_t Test_Ray( vec3_t origin, vec3_t dir, int flags ){
	brush_t *brush;
	face_t  *face;
	float dist;
	trace_t t;

	memset( &t, 0, sizeof( t ) );
	t.dist = DIST_START;

	if ( flags & SF_CYCLE ) {
		CPtrArray array;
		brush_t *pToSelect = ( selected_brushes.next != &selected_brushes ) ? selected_brushes.next : NULL;
		Select_Deselect();

		// go through active brushes and accumulate all "hit" brushes
		for ( brush = active_brushes.next ; brush != &active_brushes ; brush = brush->next )
		{
			//if ( (flags & SF_ENTITIES_FIRST) && brush->owner == world_entity)
			//  continue;

			if ( brush->bFiltered ) {
				continue;
			}

			if ( !g_PrefsDlg.m_bSelectCurves && brush->patchBrush ) {
				continue;
			}

			if ( !g_PrefsDlg.m_bSelectModels && ( brush->owner->eclass->nShowFlags & ECLASS_MISCMODEL ) ) {
				continue;
			}

			//if (!g_bShowPatchBounds && brush->patchBrush)
			//  continue;

			face = Brush_Ray( origin, dir, brush, &dist, flags );

			if ( face ) {
				array.Add( brush );
			}
		}

		int nSize = array.GetSize();
		if ( nSize > 0 ) {
			bool bFound = false;
			for ( int i = 0; i < nSize; i++ )
			{
				brush_t *b = reinterpret_cast<brush_t*>( array.GetAt( i ) );
				// did we hit the last one selected yet ?
				if ( b == pToSelect ) {
					// yes we want to select the next one in the list
					int n = ( i > 0 ) ? i - 1 : nSize - 1;
					pToSelect = reinterpret_cast<brush_t*>( array.GetAt( n ) );
					bFound = true;
					break;
				}
			}
			if ( !bFound ) {
				pToSelect = reinterpret_cast<brush_t*>( array.GetAt( 0 ) );
			}
		}
		if ( pToSelect ) {
			face = Brush_Ray( origin, dir, pToSelect, &dist, flags );
			t.dist = dist;
			t.brush = pToSelect;
			t.face = face;
			t.selected = false;
			return t;
		}
	}

	if ( !( flags & SF_SELECTED_ONLY ) ) {
		for ( brush = active_brushes.next ; brush != &active_brushes ; brush = brush->next )
		{
			if ( ( flags & SF_ENTITIES_FIRST ) && ( brush->owner == world_entity || !brush->owner->eclass->fixedsize ) ) {
				continue;
			}

			if ( brush->bFiltered ) {
				continue;
			}

			if ( !g_PrefsDlg.m_bSelectCurves && brush->patchBrush ) {
				continue;
			}

			if ( !g_PrefsDlg.m_bSelectModels && ( brush->owner->eclass->nShowFlags & ECLASS_MISCMODEL ) ) {
				continue;
			}

			//if (!g_bShowPatchBounds && brush->patchBrush)
			//  continue;

			face = Brush_Ray( origin, dir, brush, &dist, flags );
			if ( face && dist > 0 && dist < t.dist ) {
				t.dist = dist;
				t.brush = brush;
				t.face = face;
				t.selected = false;
			}
		}
	}


	for ( brush = selected_brushes.next ; brush != &selected_brushes ; brush = brush->next )
	{
		if ( ( flags & SF_ENTITIES_FIRST ) && ( brush->owner == world_entity || !brush->owner->eclass->fixedsize ) ) {
			continue;
		}

		if ( brush->bFiltered ) {
			continue;
		}

		if ( !g_PrefsDlg.m_bSelectCurves && brush->patchBrush ) {
			continue;
		}

		if ( !g_PrefsDlg.m_bSelectModels && ( brush->owner->eclass->nShowFlags & ECLASS_MISCMODEL ) ) {
			continue;
		}

		face = Brush_Ray( origin, dir, brush, &dist, flags );
		if ( dist > 0 && dist < t.dist ) {
			t.dist = dist;
			t.brush = brush;
			t.face = face;
			t.selected = true;
		}
	}

	// if entites first, but didn't find any, check regular

	if ( ( flags & SF_ENTITIES_FIRST ) && t.brush == NULL ) {
		return Test_Ray( origin, dir, flags & ~SF_ENTITIES_FIRST );
	}

	return t;

}


/*
   ============
   Select_Brush

   ============
 */
void Select_Brush( brush_t *brush, bool bComplete, bool bStatus ){
	brush_t *b;
	entity_t    *e;

	g_ptrSelectedFaces.RemoveAll();
	g_ptrSelectedFaceBrushes.RemoveAll();
	if ( g_qeglobals.d_select_count < 2 ) {
		g_qeglobals.d_select_order[g_qeglobals.d_select_count] = brush;
	}
	g_qeglobals.d_select_count++;

	e = brush->owner;
	if ( e ) {
		// select complete entity on first click
		if ( e != world_entity && bComplete == true ) {
			for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
				if ( b->owner == e ) {
					goto singleselect;
				}
			for ( b = e->brushes.onext ; b != &e->brushes ; b = b->onext )
			{
				if ( b == brush ) { // make sure we add the actual selected brush last, mainly for cycle select
					continue;
				}
				Brush_RemoveFromList( b );
				Brush_AddToList( b, &selected_brushes );
			}
			Brush_RemoveFromList( brush );
			Brush_AddToList( brush, &selected_brushes );
		}
		else
		{
singleselect:
			Brush_RemoveFromList( brush );
			Brush_AddToList( brush, &selected_brushes );
			UpdatePatchInspector();
		}

		if ( e->eclass ) {
			UpdateEntitySel( brush->owner->eclass );
		}

		UpdateSurfaceDialog();
	}

	if ( bStatus ) {
		vec3_t vMin, vMax, vSize;
		Select_GetBounds( vMin, vMax );
		VectorSubtract( vMax, vMin, vSize );
		CString strStatus;
		strStatus.Format( "Selection X:: %.1f  Y:: %.1f  Z:: %.1f", vSize[0], vSize[1], vSize[2] );
		g_pParentWnd->SetStatusText( 2, strStatus );
	}
}

/*
   =============
   Select_FaceInSelectedBrushes
   =============
 */
bool Select_FaceInSelectedBrushes( face_t *face ){
	brush_t *brush;
	face_t  *tface;

	for ( brush = selected_brushes.next; brush != &selected_brushes; brush = brush->next )
	{
		for ( tface = brush->brush_faces; tface; tface = tface->next )
		{
			if ( tface == face ) {
				return true;
			}
		}
	}

	return false;
}

/*
   ============
   Select_Ray

   If the origin is inside a brush, that brush will be ignored.
   ============
 */
void Select_Ray( vec3_t origin, vec3_t dir, int flags ){
	trace_t t;
	face_t  *tface;
	bool bOk;
	static trace_t lastTrace = {
		NULL, //	brush
		NULL, //	face
		0,  //	dist
		false //	selected
	};

	t = Test_Ray( origin, dir, flags );
	if ( !t.brush ) {
		return;
	}

	if ( flags & SF_SINGLEFACE ) {
		if ( flags & SF_DRAG ) {
			if ( t.brush == lastTrace.brush && t.face == lastTrace.face ) {
				return;
			}
		}
		lastTrace = t;

		if ( Select_FaceInSelectedBrushes( t.face ) ) {
			// Deselect the brush
			Brush_RemoveFromList( t.brush );
			Brush_AddToList( t.brush, &active_brushes );
			UpdatePatchInspector();

			// Select all of the brush's faces except the one we are pointing at
			for ( tface = t.brush->brush_faces; tface; tface = tface->next )
			{
				if ( tface == t.face ) {
					continue;
				}

				bOk = true;
				// NOTE: keep the size check in the loop, we remove stuff inside
				for ( int i = 0; i < g_SelectedFaces.GetSize(); i++ )
				{
					if ( tface == reinterpret_cast<face_t*>( g_SelectedFaces.GetAt( i ) ) ) {
						bOk = false;
					}
				}

				if ( bOk ) {
					g_SelectedFaces.Add( tface );
					g_SelectedFaceBrushes.Add( t.brush );
				}
			}
			g_qeglobals.d_select_mode = sel_facets_off;
		}
		else
		{
			bOk = true;
			// NOTE: keep the size check in the loop, we remove stuff inside
			for ( int i = 0; i < g_SelectedFaces.GetSize(); i++ )
			{
				if ( t.face == reinterpret_cast<face_t*>( g_SelectedFaces.GetAt( i ) ) ) {
					bOk = false;
					if ( flags & SF_DRAG_ON ) {
						continue;
					}

					g_qeglobals.d_select_mode = sel_facets_off;
					// need to remove i'th entry
					g_SelectedFaces.RemoveAt( i, 1 );
					g_SelectedFaceBrushes.RemoveAt( i, 1 );
				}
			}

			if ( bOk && !( flags & SF_DRAG_OFF ) ) {
				g_SelectedFaces.Add( t.face );
				g_SelectedFaceBrushes.Add( t.brush );
				g_qeglobals.d_select_mode = sel_facets_on;
			}
		}
		UpdateSurfaceDialog();
		Sys_UpdateWindows( W_ALL );
		//g_qeglobals.d_select_mode = sel_brush;
		// Texture_SetTexture requires a brushprimit_texdef fitted to the default width=2 height=2 texture
		brushprimit_texdef_t brushprimit_texdef;
		ConvertTexMatWithQTexture( &t.face->brushprimit_texdef, t.face->d_texture, &brushprimit_texdef, NULL );
		Texture_SetTexture( &t.face->texdef, &brushprimit_texdef, false, NULL, false );
		return;
	}

	// move the brush to the other list
	if ( t.selected ) {
		if ( flags & SF_DRAG_ON ) {
			return;
		}

		g_qeglobals.d_select_mode = sel_brush_off;
		Brush_RemoveFromList( t.brush );
		Brush_AddToList( t.brush, &active_brushes );

		UpdatePatchInspector();
	}
	else
	{
		if ( flags & SF_DRAG_OFF ) {
			return;
		}

		g_qeglobals.d_select_mode = sel_brush_on;
		Select_Brush( t.brush, g_PrefsDlg.m_nCamDragMultiSelect == 1 ? Sys_AltDown() : !Sys_AltDown() );
	}
	UpdateSurfaceDialog();
	Sys_UpdateWindows( W_ALL );
}


void Select_Delete( void ){
	brush_t *brush;
	entity_t *e;

	g_ptrSelectedFaces.RemoveAll();
	g_ptrSelectedFaceBrushes.RemoveAll();

	g_qeglobals.d_select_mode = sel_brush;

	g_qeglobals.d_select_count = 0;
	g_qeglobals.d_num_move_points = 0;
	while ( selected_brushes.next != &selected_brushes )
	{
		brush = selected_brushes.next;
		if ( brush->patchBrush ) {
			Patch_Delete( brush->pPatch );
		}
		e = brush->owner;
		Brush_Free( brush );
		// remove if no brushes
		if ( e != world_entity && e->brushes.onext == &e->brushes ) {
			Entity_Free( e );
		}
	}

	Sys_MarkMapModified();
	UpdateSurfaceDialog();
	Sys_UpdateWindows( W_ALL );
}

// update the workzone to a given brush
void UpdateWorkzone_ForBrush( brush_t* b ){
	VectorCopy( b->mins, g_qeglobals.d_work_min );
	VectorCopy( b->maxs, g_qeglobals.d_work_max );
	//++timo clean
#if 0
	// will update the workzone to the given brush
	// g_pParentWnd->ActiveXY()->GetViewType()
	// cf VIEWTYPE defintion: enum VIEWTYPE {YZ, XZ, XY};
	// we fit our work zone to the last brush on the list (b)
	int nViewType = g_pParentWnd->ActiveXY()->GetViewType();
	int nDim1 = ( nViewType == YZ ) ? 1 : 0;
	int nDim2 = ( nViewType == XY ) ? 1 : 2;
	g_qeglobals.d_work_min[nDim1] = b->mins[nDim1];
	g_qeglobals.d_work_max[nDim1] = b->maxs[nDim1];
	g_qeglobals.d_work_min[nDim2] = b->mins[nDim2];
	g_qeglobals.d_work_max[nDim2] = b->maxs[nDim2];
#endif
}

// here to filter new brushes once unselected
extern void PerformFiltering();

void Select_Deselect( bool bDeselectFaces ){
	brush_t *b;

	Patch_Deselect();

	g_pParentWnd->ActiveXY()->UndoClear();

	g_qeglobals.d_workcount++;
	g_qeglobals.d_select_count = 0;
	g_qeglobals.d_num_move_points = 0;
	b = selected_brushes.next;

	if ( b == &selected_brushes ) {
		if ( bDeselectFaces ) {
			g_ptrSelectedFaces.RemoveAll();
			g_ptrSelectedFaceBrushes.RemoveAll();
		}
		PerformFiltering();
		UpdateSurfaceDialog();
		Sys_UpdateWindows( W_ALL );
		return;
	}

	if ( bDeselectFaces ) {
		g_ptrSelectedFaces.RemoveAll();
		g_ptrSelectedFaceBrushes.RemoveAll();
	}

	g_qeglobals.d_select_mode = sel_brush;

	UpdateWorkzone_ForBrush( b );

	selected_brushes.next->prev = &active_brushes;
	selected_brushes.prev->next = active_brushes.next;
	active_brushes.next->prev = selected_brushes.prev;
	active_brushes.next = selected_brushes.next;
	selected_brushes.prev = selected_brushes.next = &selected_brushes;

	// filter newly created stuff once it's unselected
	PerformFiltering();
	UpdateSurfaceDialog();
	Sys_UpdateWindows( W_ALL );
}

/*
   ============
   Select_Move
   ============
 */
/*! Moves the currently selected brush/patch
    \param delta How far to move the selection (x,y,z)
    \param bSnap If the move should snap to grid points
 */
void Select_Move( vec3_t delta, bool bSnap ){
	brush_t *b;

	// actually move the selected brushes
	for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
		Brush_Move( b, delta, bSnap );

	vec3_t vMin, vMax;
	Select_GetBounds( vMin, vMax );
	CString strStatus;
	strStatus.Format( "Origin X:: %.1f  Y:: %.1f  Z:: %.1f", vMin[0], vMax[1], vMax[2] );
	g_pParentWnd->SetStatusText( 2, strStatus );

	//Sys_UpdateWindows (W_ALL);
}

/*
   =================
   Select_NudgeVerts
   =================
 */
/*! Moves the currently selected brush/patch vertices
    \param delta How far to move the vertices (x,y,z)
    \param bSnap If the move should snap to grid points
 */
void Select_NudgePoint( vec3_t delta, qboolean bSnap ){
	if ( g_qeglobals.d_select_mode == sel_vertex ) {
		// move selected verts
		brush_t *b;
		vec3_t end;
		qboolean success = true;
		for ( b = selected_brushes.next; b != &selected_brushes; b = b->next )
		{
			success &= (qboolean)Brush_MoveVertex( b, g_qeglobals.d_move_points[0], delta, end, bSnap );
		}
		if ( success ) {
			VectorCopy( end, g_qeglobals.d_move_points[0] );
		}
	}
	else if ( g_qeglobals.d_select_mode == sel_curvepoint ) {
		// move selected patch control points
		Patch_UpdateSelected( delta );
	}
}

/*
   ============
   Select_Clone

   Creates an exact duplicate of the selection in place, then moves
   the selected brushes off of their old positions
   ============
 */
void Select_Clone( void ){
	g_bScreenUpdates = false;
	g_pParentWnd->Copy();
	Select_Deselect();
	g_pParentWnd->Paste();
	g_pParentWnd->NudgeSelection( 2, g_qeglobals.d_gridsize );
	g_pParentWnd->NudgeSelection( 3, g_qeglobals.d_gridsize );
	Undo_Start( "clone" );
	Undo_EndBrushList( &selected_brushes );
	Undo_End();
	g_bScreenUpdates = true;
	Sys_UpdateWindows( W_ALL );
}

//++timo clean
#if 0
/*
   ============
   Select_SetTexture
   Timo : bFitScale to compute scale on the plane and counteract plane / axial plane snapping
   Timo :	brush primitive texturing
        the brushprimit_texdef given must be understood as a qtexture_t width=2 height=2 ( HiRes )
   Timo :  texture plugin, added an IPluginTexdef* parameter
        must be casted to an IPluginTexdef!
        if not NULL, get ->Copy() of it into each face or brush ( and remember to hook )
        if NULL, means we have no information, ask for a default
   TTimo - shader code cleanup
   added IShader* parameter
   ============
 */
void WINAPI Select_SetTexture2( IShader* pShader, texdef_t *texdef, brushprimit_texdef_t *brushprimit_texdef, bool bFitScale, void* pPlugTexdef ){
	brush_t *b;
	int nCount = g_ptrSelectedFaces.GetSize();
	if ( nCount > 0 ) {
		Undo_Start( "set face textures" );
		ASSERT( g_ptrSelectedFaces.GetSize() == g_ptrSelectedFaceBrushes.GetSize() );
		for ( int i = 0; i < nCount; i++ )
		{
			face_t *selFace = reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( i ) );
			brush_t *selBrush = reinterpret_cast<brush_t*>( g_ptrSelectedFaceBrushes.GetAt( i ) );
			Undo_AddBrush( selBrush );
			//++timo TODO: propagate the IShader* ..
			SetFaceTexdef( selFace, texdef, brushprimit_texdef, bFitScale, static_cast<IPluginTexdef *>( pPlugTexdef ) );
			Brush_Build( selBrush, bFitScale );
			Undo_EndBrush( selBrush );
		}
		Undo_End();
	}
	else if ( selected_brushes.next != &selected_brushes ) {
		Undo_Start( "set brush textures" );
		for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
			if ( !b->owner->eclass->fixedsize ) {
				Undo_AddBrush( b );
				Brush_SetTexture2( b, pShader, texdef, brushprimit_texdef, bFitScale, static_cast<IPluginTexdef *>( pPlugTexdef ) );
				Undo_EndBrush( b );
			}
		Undo_End();
	}
	Sys_UpdateWindows( W_ALL );
}
#endif

/*
   ============
   Select_SetTexture
   Timo : bFitScale to compute scale on the plane and counteract plane / axial plane snapping
   Timo :	brush primitive texturing
        the brushprimit_texdef given must be understood as a qtexture_t width=2 height=2 ( HiRes )
   Timo :  texture plugin, added an IPluginTexdef* parameter
        must be casted to an IPluginTexdef!
        if not NULL, get ->Copy() of it into each face or brush ( and remember to hook )
        if NULL, means we have no information, ask for a default
   ============
 */
void WINAPI Select_SetTexture( texdef_t *texdef, brushprimit_texdef_t *brushprimit_texdef, bool bFitScale, void* pPlugTexdef ){
	/*
	   #ifdef _DEBUG
	   static int count = 0;
	   #endif
	 */
	brush_t *b;
	/*
	   #ifdef _DEBUG
	   count++;
	   Sys_Printf("count: %d\n", count);
	   if(count==4)
	   Sys_Printf("break!\n");
	   #endif
	 */
	int nCount = g_ptrSelectedFaces.GetSize();
	if ( nCount > 0 ) {
		Undo_Start( "set face textures" );
		assert( g_ptrSelectedFaces.GetSize() == g_ptrSelectedFaceBrushes.GetSize() );
		for ( int i = 0; i < nCount; i++ )
		{
			face_t *selFace = reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( i ) );
			brush_t *selBrush = reinterpret_cast<brush_t*>( g_ptrSelectedFaceBrushes.GetAt( i ) );
			Undo_AddBrush( selBrush );
			SetFaceTexdef( selFace, texdef, brushprimit_texdef, bFitScale, static_cast<IPluginTexdef *>( pPlugTexdef ) );
			Brush_Build( selBrush, bFitScale );
			Undo_EndBrush( selBrush );
		}
		Undo_End();
	}
	else if ( selected_brushes.next != &selected_brushes ) {
		Undo_Start( "set brush textures" );
		for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
			if ( !b->owner->eclass->fixedsize ) {
				Undo_AddBrush( b );
				Brush_SetTexture( b, texdef, brushprimit_texdef, bFitScale, static_cast<IPluginTexdef *>( pPlugTexdef ) );
				Undo_EndBrush( b );
			}
		Undo_End();
	}
	//++timo FIXME: not necessary in every cases, write a message defering / move one level up
	Sys_UpdateWindows( W_ALL );
}


/*
   ================================================================

   TRANSFORMATIONS

   ================================================================
 */

void Select_GetBounds( vec3_t mins, vec3_t maxs ){
	brush_t *b;
	int i;

	for ( i = 0 ; i < 3 ; i++ )
	{
		mins[i] = 99999;
		maxs[i] = -99999;
	}

	for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
	{
		if ( b->owner->eclass->fixedsize ) {
			for ( i = 0 ; i < 3 ; i++ )
			{
				if ( b->owner->origin[i] < mins[i] ) {
					mins[i] = b->owner->origin[i];
				}
				if ( b->owner->origin[i] > maxs[i] ) {
					maxs[i] = b->owner->origin[i];
				}
			}
		}
		else
		{
			for ( i = 0 ; i < 3 ; i++ )
			{
				if ( b->mins[i] < mins[i] ) {
					mins[i] = b->mins[i];
				}
				if ( b->maxs[i] > maxs[i] ) {
					maxs[i] = b->maxs[i];
				}
			}
		}
	}
}

void Select_GetTrueMid( vec3_t mid ){
	vec3_t mins, maxs;
	Select_GetBounds( mins, maxs );

	for ( int i = 0 ; i < 3 ; i++ )
		mid[i] = ( mins[i] + ( ( maxs[i] - mins[i] ) / 2 ) );
}

void Select_GetMid( vec3_t mid ){
	vec3_t mins, maxs;
	int i;

	if ( !g_PrefsDlg.m_bSnap ) {
		Select_GetTrueMid( mid );
		return;
	}

	Select_GetBounds( mins, maxs );

	for ( i = 0 ; i < 3 ; i++ )
		mid[i] = g_qeglobals.d_gridsize * floor( ( ( mins[i] + maxs[i] ) * 0.5 ) / g_qeglobals.d_gridsize );
}

vec3_t select_origin;
vec3_t select_matrix[3];
qboolean select_fliporder;

// FIXME: bApplyBPrimit is supposed to be temporary
// TODO: manage Brush_Build calls, too many of them with the texture processing
// FIXME: the undo doesn't seem to work correctly on texturing and flip/rotate operations?? this is not supposed to be related to the texture locking code, so what is happening?
// FIXME: ApplyMatrix works on flipping operation, b0rks on Rotations (so does the "regular" rotation code??)
// FIXME: what is getting called in free rotation mode? that used to work right?
void Select_ApplyMatrix( bool bSnap, bool bRotation, int nAxis, float fDeg ){ //, qboolean bApplyBPrimit)
	brush_t *b;
	face_t  *f;
	int i, j;
	vec3_t temp, tmporigin;

	for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
	{
		if ( b->owner->eclass->fixedsize ) {
			VectorCopy( b->owner->origin, tmporigin );
			// transform the origin point
			VectorSubtract( b->owner->origin, select_origin, temp );
			for ( j = 0 ; j < 3 ; j++ )
				b->owner->origin[j] = DotProduct( temp, select_matrix[j] ) + select_origin[j];

			// update the origin key
			char text[64];
			sprintf( text, "%i %i %i",
					 (int)b->owner->origin[0], (int)b->owner->origin[1], (int)b->owner->origin[2] );
			SetKeyValue( b->owner, "origin", text );

			/*\todo remove brush-based bounding box for fixedsize entities */
			VectorSubtract( b->owner->origin, tmporigin, temp );
			for ( f = b->brush_faces ; f ; f = f->next )
			{
				// move fixedsize bbox to new origin
				for ( i = 0 ; i < 3 ; i++ )
					VectorAdd( f->planepts[i], temp, f->planepts[i] );
			}
			Brush_Build( b, bSnap,true,false,false ); // don't filter

		}
		else if ( b->patchBrush ) {
			if ( !bRotation && !( ( g_qeglobals.d_select_mode == sel_curvepoint && g_qeglobals.d_num_move_points != 0 ) || g_bPatchBendMode ) ) {
				// invert patch if this is a mirroring operation, unless points are selected or bendmode is active
				patchInvert( b->pPatch );
			}
			// NOTE: does not clamp points to integers
			Patch_ApplyMatrix( b->pPatch, select_origin, select_matrix, false );
		}
		else
		{
			for ( f = b->brush_faces ; f ; f = f->next )
			{
				// FIXME: only in BP mode!
				// if we are using Brush Primitives texturing, we need to compute the texture matrix after the geometric transformation
				// (with the default texturing you don't need to compute anything for flipping and mirroring operations)
				//      if (bApplyBPrimit) {
				//        ApplyMatrix_BrushPrimit (f, select_matrix, select_origin, select_fliporder);
				//      }
				for ( i = 0 ; i < 3 ; i++ )
				{
					VectorSubtract( f->planepts[i], select_origin, temp );
					for ( j = 0 ; j < 3 ; j++ )
						f->planepts[i][j] = DotProduct( temp, select_matrix[j] ) + select_origin[j];
				}
				if ( select_fliporder ) {
					VectorCopy( f->planepts[0], temp );
					VectorCopy( f->planepts[2], f->planepts[0] );
					VectorCopy( temp, f->planepts[2] );
				}
			}
			Brush_Build( b, bSnap,true,false,false ); // don't filter
		}
	}
}

void ProjectOnPlane( vec3_t& normal,float dist,vec3_t& ez, vec3_t& p ){
	if ( fabs( ez[0] ) == 1 ) {
		p[0] = ( dist - normal[1] * p[1] - normal[2] * p[2] ) / normal[0];
	}
	else if ( fabs( ez[1] ) == 1 ) {
		p[1] = ( dist - normal[0] * p[0] - normal[2] * p[2] ) / normal[1];
	}
	else{
		p[2] = ( dist - normal[0] * p[0] - normal[1] * p[1] ) / normal[2];
	}
}

void Back( vec3_t& dir, vec3_t& p ){
	if ( fabs( dir[0] ) == 1 ) {
		p[0] = 0;
	}
	else if ( fabs( dir[1] ) == 1 ) {
		p[1] = 0;
	}
	else{p[2] = 0; }
}



// using scale[0] and scale[1]
void ComputeScale( vec3_t& rex, vec3_t& rey, vec3_t& p, face_t* f ){
	float px = DotProduct( rex, p );
	float py = DotProduct( rey, p );
	px *= f->texdef.scale[0];
	py *= f->texdef.scale[1];
	vec3_t aux;
	VectorCopy( rex, aux );
	VectorScale( aux, px, aux );
	VectorCopy( aux, p );
	VectorCopy( rey, aux );
	VectorScale( aux, py, aux );
	VectorAdd( p, aux, p );
}

void ComputeAbsolute( face_t* f, vec3_t& p1, vec3_t& p2, vec3_t& p3 ){
	vec3_t ex,ey,ez;            // local axis base

#ifdef _DEBUG
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		Sys_FPrintf( SYS_WRN, "Warning : illegal call of ComputeAbsolute in brush primitive mode\n" );
	}
#endif

	// compute first local axis base
	TextureAxisFromPlane( &f->plane, ex, ey );
	CrossProduct( ex, ey, ez );

	vec3_t aux;
	VectorCopy( ex, aux );
	VectorScale( aux, -f->texdef.shift[0], aux );
	VectorCopy( aux, p1 );
	VectorCopy( ey, aux );
	VectorScale( aux, -f->texdef.shift[1], aux );
	VectorAdd( p1, aux, p1 );
	VectorCopy( p1, p2 );
	VectorAdd( p2, ex, p2 );
	VectorCopy( p1, p3 );
	VectorAdd( p3, ey, p3 );
	VectorCopy( ez, aux );
	VectorScale( aux, -f->texdef.rotate, aux );
	VectorRotate( p1, aux, p1 );
	VectorRotate( p2, aux, p2 );
	VectorRotate( p3, aux, p3 );
	// computing rotated local axis base
	vec3_t rex,rey;
	VectorCopy( ex, rex );
	VectorRotate( rex, aux, rex );
	VectorCopy( ey, rey );
	VectorRotate( rey, aux, rey );

	ComputeScale( rex,rey,p1,f );
	ComputeScale( rex,rey,p2,f );
	ComputeScale( rex,rey,p3,f );

	// project on normal plane
	// along ez
	// assumes plane normal is normalized
	ProjectOnPlane( f->plane.normal,f->plane.dist,ez,p1 );
	ProjectOnPlane( f->plane.normal,f->plane.dist,ez,p2 );
	ProjectOnPlane( f->plane.normal,f->plane.dist,ez,p3 );
};


void AbsoluteToLocal( plane_t normal2, face_t* f, vec3_t& p1, vec3_t& p2, vec3_t& p3 ){
	vec3_t ex,ey,ez;

#ifdef _DEBUG
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		Sys_FPrintf( SYS_WRN, "Warning : illegal call of AbsoluteToLocal in brush primitive mode\n" );
	}
#endif

	// computing new local axis base
	TextureAxisFromPlane( &normal2, ex, ey );
	CrossProduct( ex, ey, ez );

	// projecting back on (ex,ey)
	Back( ez,p1 );
	Back( ez,p2 );
	Back( ez,p3 );

	vec3_t aux;
	// rotation
	VectorCopy( p2, aux );
	VectorSubtract( aux, p1,aux );

	float x = DotProduct( aux,ex );
	float y = DotProduct( aux,ey );
	f->texdef.rotate = 180 * atan2( y,x ) / Q_PI;

	vec3_t rex,rey;
	// computing rotated local axis base
	VectorCopy( ez, aux );
	VectorScale( aux, f->texdef.rotate, aux );
	VectorCopy( ex, rex );
	VectorRotate( rex, aux, rex );
	VectorCopy( ey, rey );
	VectorRotate( rey, aux, rey );

	// scale
	VectorCopy( p2, aux );
	VectorSubtract( aux, p1, aux );
	f->texdef.scale[0] = DotProduct( aux, rex );
	VectorCopy( p3, aux );
	VectorSubtract( aux, p1, aux );
	f->texdef.scale[1] = DotProduct( aux, rey );

	// shift
	// only using p1
	x = DotProduct( rex,p1 );
	y = DotProduct( rey,p1 );
	x /= f->texdef.scale[0];
	y /= f->texdef.scale[1];

	VectorCopy( rex, p1 );
	VectorScale( p1, x, p1 );
	VectorCopy( rey, aux );
	VectorScale( aux, y, aux );
	VectorAdd( p1, aux, p1 );
	VectorCopy( ez, aux );
	VectorScale( aux, -f->texdef.rotate, aux );
	VectorRotate( p1, aux, p1 );
	f->texdef.shift[0] = -DotProduct( p1, ex );
	f->texdef.shift[1] = -DotProduct( p1, ey );

	// stored rot is good considering local axis base
	// change it if necessary
	f->texdef.rotate = -f->texdef.rotate;

	Clamp( f->texdef.shift[0], f->d_texture->width );
	Clamp( f->texdef.shift[1], f->d_texture->height );
	Clamp( f->texdef.rotate, 360 );

}

void RotateFaceTexture( face_t* f, int nAxis, float fDeg ){
	vec3_t p1,p2,p3, rota;
	p1[0] = p1[1] = p1[2] = 0;
	VectorCopy( p1, p2 );
	VectorCopy( p1, p3 );
	VectorCopy( p1, rota );
	ComputeAbsolute( f, p1, p2, p3 );

	rota[nAxis] = fDeg;
	VectorRotateOrigin( p1, rota, select_origin, p1 );
	VectorRotateOrigin( p2, rota, select_origin, p2 );
	VectorRotateOrigin( p3, rota, select_origin, p3 );

	plane_t normal2;
	vec3_t vNormal;
	vNormal[0] = f->plane.normal[0];
	vNormal[1] = f->plane.normal[1];
	vNormal[2] = f->plane.normal[2];
	VectorRotate( vNormal, rota, vNormal );
	normal2.normal[0] = vNormal[0];
	normal2.normal[1] = vNormal[1];
	normal2.normal[2] = vNormal[2];
	AbsoluteToLocal( normal2, f, p1, p2,p3 );

}

void RotateTextures( int nAxis, float fDeg, vec3_t vOrigin ){
	for ( brush_t* b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
	{
		for ( face_t* f = b->brush_faces ; f ; f = f->next )
		{
			if ( g_qeglobals.m_bBrushPrimitMode ) {
				RotateFaceTexture_BrushPrimit( f, nAxis, fDeg, vOrigin );
			}
			else{
				RotateFaceTexture( f, nAxis, fDeg );
			}
		}
		Brush_Build( b, false,true,false,false ); // don't filter
	}
}

void Select_ApplyMatrix_BrushPrimit(){
  #ifdef _DEBUG
	if ( !g_qeglobals.m_bBrushPrimitMode ) {
		Sys_FPrintf( SYS_ERR,"ERROR: Select_ApplyMatrix_BrushPrimit called in non-BP mode\n" );
	}
  #endif
	for ( brush_t* b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
	{
		for ( face_t* f = b->brush_faces ; f ; f = f->next )
		{
			ApplyMatrix_BrushPrimit( f, select_matrix, select_origin );
		}
	}
}

void Select_FlipAxis( int axis ){
	int i;

	Select_GetMid( select_origin );
	for ( i = 0 ; i < 3 ; i++ )
	{
		VectorCopy( vec3_origin, select_matrix[i] );
		select_matrix[i][i] = 1;
	}
	select_matrix[axis][axis] = -1;
	select_fliporder = true;

	// texture locking
	if ( g_PrefsDlg.m_bRotateLock ) {
		// axis flipping inverts space orientation, we have to use a general texture locking algorithm instead of the RotateFaceTexture
		if ( g_qeglobals.m_bBrushPrimitMode ) {
			Select_ApplyMatrix_BrushPrimit();
		}
		else
		{
			// there's never been flip locking for non BP mode, this would be tricky to write and there's not much interest for it with the coming of BP format
			// what could be done is converting regular to BP, locking, then back to regular :)
			Sys_FPrintf( SYS_WRN, "WARNING: regular texturing doesn't have texture lock on flipping operations\n" );
		}
	}
	// geometric transformation
	Select_ApplyMatrix( true, false, 0, 0 );
	Sys_UpdateWindows( W_ALL );
}


void Select_Scale( float x, float y, float z ){
	Select_GetMid( select_origin );
	for ( brush_t* b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
	{
		// ignore fixedsize entities
		if ( b->owner->eclass->fixedsize ) {
			continue;
		}
		for ( face_t* f = b->brush_faces ; f ; f = f->next )
		{
			for ( int i = 0 ; i < 3 ; i++ )
			{
				f->planepts[i][0] -= select_origin[0];
				f->planepts[i][1] -= select_origin[1];
				f->planepts[i][2] -= select_origin[2];
				f->planepts[i][0] *= x;
				f->planepts[i][1] *= y;
				f->planepts[i][2] *= z;

				f->planepts[i][0] += select_origin[0];
				f->planepts[i][1] += select_origin[1];
				f->planepts[i][2] += select_origin[2];
			}
		}
		Brush_Build( b, false,true,false,false ); // don't filter
		if ( b->patchBrush ) {
			vec3_t v;
			v[0] = x;
			v[1] = y;
			v[2] = z;
			Patch_Scale( b->pPatch, select_origin, v );
		}
	}
}

void Select_RotateAxis( int axis, float deg, bool bPaint, bool bMouse ){
	int i;
	vec_t c, s;

	if ( deg == 0 ) {
		return;
	}

	if ( bMouse ) {
		VectorCopy( g_pParentWnd->ActiveXY()->RotateOrigin(), select_origin );
	}
	else
	{
		Select_GetMid( select_origin );
	}

	/*
	   if(axis == 2)
	   {
	   vec3_t rotation;
	   VectorSet(rotation, 0, 0, 360 - deg);
	   for(brush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next)
	    if(b->owner->model.pEdit)
	      b->owner->model.pEdit->Rotate(select_origin, rotation);
	   }
	 */

	select_fliporder = false;

	// the "90" degrees algorithm is mostly used on axis rotate as a speedup and possibly avoiding rounding errors as much as possible
	// previous implementation was doing an indirect-oriented rotation over the plane whereas the general algo below was doing a direct-oriented rotation
	// this was confusing the texture locking algorithms, fixed it to be direct-oriented (side consequence is that the axis rotate toolbar button rotates the other way now)
	// NOTE: previous algo was using vec3_origin in the matrix computation..
	//   I don't see what an origin does in linear transformations (3x3 matrixes always relate to a (0,0,0) origin)
	//   in Radiant it's initialized as (0,0,0) and never set to another value
	//   so I got rid of it when it's not used for initialisation tasks (and even if it's not (0,0,0) it should not matter
	if ( deg == 90 ) {
		c = 0;
		s = 1;
	}
	else
	{
		c = cos( deg * Q_PI / 180.0 );
		s = sin( deg * Q_PI / 180.0 );
	}

	for ( i = 0 ; i < 3 ; i++ )
	{
		VectorCopy( vec3_origin, select_matrix[i] );
		select_matrix[i][i] = 1;
	}

	switch ( axis )
	{
	case 0:
		select_matrix[1][1] = c;
		select_matrix[1][2] = s;
		select_matrix[2][1] = -s;
		select_matrix[2][2] = c;
		break;
	case 1:
		select_matrix[0][0] = c;
		select_matrix[0][2] = s;
		select_matrix[2][0] = -s;
		select_matrix[2][2] = c;
		break;
	case 2:
		select_matrix[0][0] = c;
		select_matrix[0][1] = s;
		select_matrix[1][0] = -s;
		select_matrix[1][1] = c;
		break;
	}


	// texture locking
	if ( g_PrefsDlg.m_bRotateLock ) {
		// Terrible hack, reversing input rotation angle to correct
		// texture rotation direction for X and Z axes.
		// RotateTextures needs to be changed to fix this properly?
		if ( axis == 1 ) {
			RotateTextures( axis, deg, select_origin );
		}
		else{
			RotateTextures( axis, deg * -1, select_origin );
		}
	}
	// geometric transformation
	Select_ApplyMatrix( !bMouse, true, axis, deg ); //, false);

	if ( bPaint ) {
		Sys_UpdateWindows( W_ALL );
	}
}

/*
   ================================================================

   GROUP SELECTIONS

   ================================================================
 */

void Select_RealCompleteTall( vec3_t mins, vec3_t maxs ){
	brush_t *b, *next;

	int nDim1 = ( g_pParentWnd->ActiveXY()->GetViewType() == YZ ) ? 1 : 0;
	int nDim2 = ( g_pParentWnd->ActiveXY()->GetViewType() == XY ) ? 1 : 2;

	g_qeglobals.d_select_mode = sel_brush;

	for ( b = active_brushes.next ; b != &active_brushes ; b = next )
	{
		next = b->next;

		if ( b->bFiltered ) {
			continue;
		}

		if ( ( b->maxs[nDim1] > maxs[nDim1] || b->mins[nDim1] < mins[nDim1] )
			 || ( b->maxs[nDim2] > maxs[nDim2] || b->mins[nDim2] < mins[nDim2] ) ) {
			continue;
		}

		Brush_RemoveFromList( b );
		Brush_AddToList( b, &selected_brushes );
	}
}

void Select_CompleteTall( void ){
	vec3_t mins, maxs;

	if ( !QE_SingleBrush() ) {
		return;
	}

	Undo_Start( "select complete tall" );
	Undo_AddBrushList( &selected_brushes );
	Undo_End();

	VectorCopy( selected_brushes.next->mins, mins );
	VectorCopy( selected_brushes.next->maxs, maxs );
	Select_Delete();

	Select_RealCompleteTall( mins, maxs );
	Sys_UpdateWindows( W_ALL );
}

void Select_PartialTall( void ){
	brush_t *b, *next;
	vec3_t mins, maxs;

	if ( !QE_SingleBrush() ) {
		return;
	}

	Undo_Start( "select complete tall" );
	Undo_AddBrushList( &selected_brushes );
	Undo_End();

	g_qeglobals.d_select_mode = sel_brush;

	VectorCopy( selected_brushes.next->mins, mins );
	VectorCopy( selected_brushes.next->maxs, maxs );
	Select_Delete();

	int nDim1 = ( g_pParentWnd->ActiveXY()->GetViewType() == YZ ) ? 1 : 0;
	int nDim2 = ( g_pParentWnd->ActiveXY()->GetViewType() == XY ) ? 1 : 2;

	for ( b = active_brushes.next ; b != &active_brushes ; b = next )
	{
		next = b->next;

		if ( b->bFiltered ) {
			continue;
		}

		if ( ( b->mins[nDim1] > maxs[nDim1] || b->maxs[nDim1] < mins[nDim1] )
			 || ( b->mins[nDim2] > maxs[nDim2] || b->maxs[nDim2] < mins[nDim2] ) ) {
			continue;
		}


		Brush_RemoveFromList( b );
		Brush_AddToList( b, &selected_brushes );
	}

	Sys_UpdateWindows( W_ALL );
}

void Select_Touching( void ){
	brush_t *b, *next;
	int i;
	vec3_t mins, maxs;

	if ( !QE_SingleBrush() ) {
		return;
	}

	g_qeglobals.d_select_mode = sel_brush;

	VectorCopy( selected_brushes.next->mins, mins );
	VectorCopy( selected_brushes.next->maxs, maxs );

	for ( b = active_brushes.next ; b != &active_brushes ; b = next )
	{
		next = b->next;

		if ( b->bFiltered ) {
			continue;
		}

		for ( i = 0 ; i < 3 ; i++ )
			if ( b->mins[i] > maxs[i] + 1 || b->maxs[i] < mins[i] - 1 ) {
				break;
			}

		if ( i == 3 ) {
			Brush_RemoveFromList( b );
			Brush_AddToList( b, &selected_brushes );
		}
	}

	Sys_UpdateWindows( W_ALL );
}

void Select_Inside( void ){
	brush_t *b, *next;
	int i;
	vec3_t mins, maxs;

	if ( !QE_SingleBrush() ) {
		return;
	}

	Undo_Start( "select inside" );
	Undo_AddBrushList( &selected_brushes );
	Undo_End();

	g_qeglobals.d_select_mode = sel_brush;

	VectorCopy( selected_brushes.next->mins, mins );
	VectorCopy( selected_brushes.next->maxs, maxs );
	Select_Delete();

	for ( b = active_brushes.next ; b != &active_brushes ; b = next )
	{
		next = b->next;

		if ( b->bFiltered ) {
			continue;
		}

		for ( i = 0 ; i < 3 ; i++ )
			if ( b->maxs[i] > maxs[i] || b->mins[i] < mins[i] ) {
				break;
			}
		if ( i == 3 ) {
			Brush_RemoveFromList( b );
			Brush_AddToList( b, &selected_brushes );
		}
	}

	Sys_UpdateWindows( W_ALL );
}

void Select_SelectGroup( entity_t* group ){
	brush_t*  b;
	//brush_t*  next;

	Undo_Start( "select func group" );
	Undo_AddBrushList( &selected_brushes );
	Undo_End();

	Select_Deselect();

	b = &group->brushes;

	do
	{
		b = b->onext;
		Brush_RemoveFromList( b );
		Brush_AddToList( b, &selected_brushes );
	} while ( b->onext != &group->brushes );

	Sys_UpdateWindows( W_ALL );
}


void Select_Ungroup( void ){
	int numselectedgroups;
	entity_t    *e;
	brush_t     *b,* sb;

	numselectedgroups = 0;
	for ( sb = selected_brushes.next; sb != &selected_brushes; sb = sb->next )
	{
		e = sb->owner;

		if ( e == world_entity || e->eclass->fixedsize ) {
			continue;
		}

		for ( b = e->brushes.onext; b != &e->brushes; b = e->brushes.onext )
		{
			Entity_UnlinkBrush( b );
			Entity_LinkBrush( world_entity, b );
		}
		Entity_Free( e );
		numselectedgroups++;
	}

	if ( numselectedgroups <= 0 ) {
		Sys_Printf( "No grouped entities selected.\n" );
		return;
	}
	Sys_Printf( "Ungrouped %d entit%s.\n", numselectedgroups, ( numselectedgroups == 1 ) ? "y" : "ies" );
	Sys_UpdateWindows( W_ALL );
}

/*!
   group selected brushes into specified entity
   if an entity is empty afterwards, destroy it
 */
void Select_GroupEntity( entity_t* group ){
	entity_t* e;
	brush_t *b;

	if ( group->eclass->fixedsize ) {
		Sys_FPrintf( SYS_ERR, "Select_GroupEntity: can't group anything to a fixedsize entity\n" );
		return;
	}

	for ( b = selected_brushes.next; b != &selected_brushes; b = b->next )
	{
		if ( b->owner->eclass->fixedsize ) {
			continue;
		}
		e = b->owner;
		Entity_UnlinkBrush( b );
		Entity_LinkBrush( group, b );
		if ( e != world_entity && e->brushes.onext == &e->brushes ) {
			Undo_AddEntity( e );
			Entity_Free( e );
		}
	}
}

/*!
   merge all selected entities together into the first one selected
   NOTE: makes use of order of selected_brushes list
   can be used to move world brushes in an entity, or to merge several ents together
   NOTE: didn't devise a strategy on the epairs, we merge into the first entity and use those
 */
void Select_MergeEntity(){
	entity_t* e = NULL;
	brush_t* b;
	for ( b = selected_brushes.next; b != &selected_brushes; b = b->next )
	{
		if ( !b->owner->eclass->fixedsize ) {
			e = b->owner;
			break;
		}
	}

	if ( e != NULL ) {
		Select_GroupEntity( e );

		int count = 0;
		for ( b = e->brushes.onext; b != &e->brushes; b = b->onext )
		{
			//Brush_RemoveFromList (b);
			//Brush_AddToList(b, &active_brushes);
			count++;
		}
		Sys_Printf( "Merged %d brushes into %s entity\n", count, ValueForKey( e, "classname" ) );
	}
}

/*
   ====================
   Select_Seperate
   ====================
 */
void Select_Seperate( void ) {
	Select_GroupEntity( world_entity );
}

/*
   ====================
   Select_MakeStructural
   ====================
 */
void Select_MakeStructural( void ){
	brush_t *b;
	face_t  *f;

	for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
	{
		for ( f = b->brush_faces ; f ; f = f->next )
			f->texdef.contents &= ~CONTENTS_DETAIL;
		b->bFiltered = FilterBrush( b );
	}
	Select_Deselect();
	Sys_UpdateWindows( W_ALL );
}

void Select_MakeDetail( void ){
	brush_t *b;
	face_t  *f;

	for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
	{
		for ( f = b->brush_faces ; f ; f = f->next )
			f->texdef.contents |= CONTENTS_DETAIL;
		b->bFiltered = FilterBrush( b );
	}
	Select_Deselect();
	Sys_UpdateWindows( W_ALL );
}

// brush primitive texture adjustments, use the camera view to map adjustments
// ShiftTextureRelative_BrushPrimit ( s , t ) will shift relative to the texture
void ShiftTextureRelative_Camera( face_t *f, int x, int y ){
	vec3_t vecS, vecT;
	vec_t XY[2]; // the values we are going to send for translation
	vec_t sgn[2]; // +1 or -1
	int axis[2];
	CamWnd* pCam;

	// get the two relative texture axes for the current texturing
	BrushPrimit_GetRelativeAxes( f, vecS, vecT );

	// center point of the face, project it on the camera space
	vec3_t C;
	VectorClear( C );
	int i;
	for ( i = 0; i < f->face_winding->numpoints; i++ )
	{
		VectorAdd( C,f->face_winding->points[i],C );
	}
	VectorScale( C,1.0 / f->face_winding->numpoints,C );

	pCam = g_pParentWnd->GetCamWnd();
	pCam->MatchViewAxes( C, vecS, axis[0], sgn[0] );
	pCam->MatchViewAxes( C, vecT, axis[1], sgn[1] );

	// this happens when the two directions can't be mapped on two different directions on the screen
	// then the move will occur against a single axis
	// (i.e. the user is not positioned well enough to send understandable shift commands)
	// NOTE: in most cases this warning is not very relevant because the user would use one of the two axes
	// for which the solution is easy (the other one being unknown)
	// so this warning could be removed
	if ( axis[0] == axis[1] ) {
		Sys_FPrintf( SYS_WRN, "Warning: degenerate in ShiftTextureRelative_Camera\n" );
	}

	// compute the X Y geometric increments
	// those geometric increments will be applied along the texture axes (the ones we computed above)
	XY[0] = 0;
	XY[1] = 0;
	if ( x != 0 ) {
		// moving right/left
		XY[axis[0]] += sgn[0] * x;
	}
	if ( y != 0 ) {
		XY[axis[1]] += sgn[1] * y;
	}
	// we worked out a move along vecS vecT, and we now it's geometric amplitude
	// apply it
	ShiftTextureRelative_BrushPrimit( f, XY[0], XY[1] );
}

void Select_ShiftTexture( int x, int y ){
	brush_t     *b;
	face_t      *f;

	int nFaceCount = g_ptrSelectedFaces.GetSize();

	if ( selected_brushes.next == &selected_brushes && nFaceCount == 0 ) {
		return;
	}

	for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
	{
		for ( f = b->brush_faces ; f ; f = f->next )
		{
			if ( g_qeglobals.m_bBrushPrimitMode ) {
				ShiftTextureRelative_Camera( f, x, y );
			}
			else
			{
				f->texdef.shift[0] += x;
				f->texdef.shift[1] += y;
			}
		}
		Brush_Build( b,true,true,false,false ); // don't filter
		if ( b->patchBrush ) {
			Patch_ShiftTexture( b->pPatch, x, y );
		}
	}

	if ( nFaceCount > 0 ) {
		for ( int i = 0; i < nFaceCount; i++ )
		{
			face_t *selFace = reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( i ) );
			brush_t *selBrush = reinterpret_cast<brush_t*>( g_ptrSelectedFaceBrushes.GetAt( i ) );
			if ( g_qeglobals.m_bBrushPrimitMode ) {
				ShiftTextureRelative_Camera( selFace, x, y );
			}
			else
			{
				selFace->texdef.shift[0] += x;
				selFace->texdef.shift[1] += y;
			}
			Brush_Build( selBrush,true,true,false,false ); // don't filter
		}
	}

	Sys_UpdateWindows( W_CAMERA );
}

//  setting float as input
void Select_ScaleTexture( float x, float y ){
	brush_t     *b;
	face_t      *f;

	int nFaceCount = g_ptrSelectedFaces.GetSize();

	if ( selected_brushes.next == &selected_brushes && nFaceCount == 0 ) {
		return;
	}

	for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
	{
		for ( f = b->brush_faces ; f ; f = f->next )
		{
			if ( g_qeglobals.m_bBrushPrimitMode ) {
				// apply same scale as the spinner button of the surface inspector
				float shift[2];
				float rotate;
				float scale[2];
				brushprimit_texdef_t bp;
				// compute normalized texture matrix
				ConvertTexMatWithQTexture( &f->brushprimit_texdef, f->d_texture, &bp, NULL );
				// compute fake shift scale rot
				TexMatToFakeTexCoords( bp.coords, shift, &rotate, scale );
				// update
				scale[0] += static_cast<float>( x ) * 0.1;
				scale[1] += static_cast<float>( y ) * 0.1;
				// compute new normalized texture matrix
				FakeTexCoordsToTexMat( shift, rotate, scale, bp.coords );
				// apply to face texture matrix
				ConvertTexMatWithQTexture( &bp, NULL, &f->brushprimit_texdef, f->d_texture );
			}
			else
			{
				f->texdef.scale[0] += x;
				f->texdef.scale[1] += y;
			}
		}
		Brush_Build( b,true,true,false,false ); // don't filter
		if ( b->patchBrush ) {
			Patch_ScaleTexture( b->pPatch, x, y );
		}
	}

	if ( nFaceCount > 0 ) {
		for ( int i = 0; i < nFaceCount; i++ )
		{
			face_t *selFace = reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( i ) );
			brush_t *selBrush = reinterpret_cast<brush_t*>( g_ptrSelectedFaceBrushes.GetAt( i ) );
			if ( g_qeglobals.m_bBrushPrimitMode ) {
				float shift[2];
				float rotate;
				float scale[2];
				brushprimit_texdef_t bp;
				ConvertTexMatWithQTexture( &selFace->brushprimit_texdef, selFace->d_texture, &bp, NULL );
				TexMatToFakeTexCoords( bp.coords, shift, &rotate, scale );
				scale[0] += static_cast<float>( x ) * 0.1;
				scale[1] += static_cast<float>( y ) * 0.1;
				FakeTexCoordsToTexMat( shift, rotate, scale, bp.coords );
				ConvertTexMatWithQTexture( &bp, NULL, &selFace->brushprimit_texdef, selFace->d_texture );
			}
			else
			{
				selFace->texdef.scale[0] += x;
				selFace->texdef.scale[1] += y;
			}
			Brush_Build( selBrush,true,true,false,false ); // don't filter
		}
	}

	Sys_UpdateWindows( W_CAMERA );
}

void Select_RotateTexture( int amt ){
	brush_t     *b;
	face_t      *f;

	int nFaceCount = g_ptrSelectedFaces.GetSize();

	if ( selected_brushes.next == &selected_brushes && nFaceCount == 0 ) {
		return;
	}

	for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
	{
		for ( f = b->brush_faces ; f ; f = f->next )
		{
			if ( g_qeglobals.m_bBrushPrimitMode ) {
				// apply same scale as the spinner button of the surface inspector
				float shift[2];
				float rotate;
				float scale[2];
				brushprimit_texdef_t bp;
				// compute normalized texture matrix
				ConvertTexMatWithQTexture( &f->brushprimit_texdef, f->d_texture, &bp, NULL );
				// compute fake shift scale rot
				TexMatToFakeTexCoords( bp.coords, shift, &rotate, scale );
				// update
				rotate += amt;
				// compute new normalized texture matrix
				FakeTexCoordsToTexMat( shift, rotate, scale, bp.coords );
				// apply to face texture matrix
				ConvertTexMatWithQTexture( &bp, NULL, &f->brushprimit_texdef, f->d_texture );
			}
			else
			{
				f->texdef.rotate += amt;
				f->texdef.rotate = static_cast<int>( f->texdef.rotate ) % 360;
			}
		}
		Brush_Build( b,true,true,false,false ); // don't filter
		if ( b->patchBrush ) {
			//Patch_RotateTexture(b->nPatchID, amt);
			Patch_RotateTexture( b->pPatch, amt );
		}
	}

	if ( nFaceCount > 0 ) {
		for ( int i = 0; i < nFaceCount; i++ )
		{
			face_t *selFace = reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( i ) );
			brush_t *selBrush = reinterpret_cast<brush_t*>( g_ptrSelectedFaceBrushes.GetAt( i ) );
			if ( g_qeglobals.m_bBrushPrimitMode ) {
				float shift[2];
				float rotate;
				float scale[2];
				brushprimit_texdef_t bp;
				ConvertTexMatWithQTexture( &selFace->brushprimit_texdef, selFace->d_texture, &bp, NULL );
				TexMatToFakeTexCoords( bp.coords, shift, &rotate, scale );
				rotate += amt;
				FakeTexCoordsToTexMat( shift, rotate, scale, bp.coords );
				ConvertTexMatWithQTexture( &bp, NULL, &selFace->brushprimit_texdef, selFace->d_texture );
			}
			else
			{
				selFace->texdef.rotate += amt;
				selFace->texdef.rotate = static_cast<int>( selFace->texdef.rotate ) % 360;
			}
			Brush_Build( selBrush,true,true,false,false ); // don't filter
		}
	}

	Sys_UpdateWindows( W_CAMERA );
}

// TTimo modified to handle shader architecture:
// expects shader names at input, comparison relies on shader names .. texture names no longer relevant
void FindReplaceTextures( const char* pFind, const char* pReplace, bool bSelected, bool bForce, bool bSelectMatchingFaces ){
	if ( strchr( pFind, ' ' ) || strchr( pReplace, ' ' ) ) {
		Sys_FPrintf( SYS_WRN, "FindReplaceTextures: '%s' or '%s' have spaces, aborted\n", pFind, pReplace );
		return;
	}

	brush_t* pList = ( bSelected ) ? &selected_brushes : &active_brushes;
	if ( !bSelected ) {
		Select_Deselect();
	}

	//++timo BP mode: replacing a texture in BP mode is not that easy, you need to recompute the texture matrix
	// if the size of the replacing texture differs, otherwise you get wrong scaling
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		Sys_Printf( "TODO: finalize find/replace code for brush primitives" );
	}

	CPtrArray mFaces;
	for ( brush_t* pBrush = pList->next ; pBrush != pList; pBrush = pBrush->next )
	{
		if ( !bSelectMatchingFaces && pBrush->patchBrush ) {
			Patch_FindReplaceTexture( pBrush, pFind, pReplace, bForce );
		}

		bool found = false; //spog
		for ( face_t* pFace = pBrush->brush_faces; pFace; pFace = pFace->next )
		{
			if ( bForce || strcmpi( pFace->pShader->getName(), pFind ) == 0 ) {
				if ( !bSelectMatchingFaces ) {
					pFace->pShader->DecRef();
					pFace->pShader = QERApp_Shader_ForName( pReplace );
					pFace->pShader->IncRef();
					pFace->d_texture = pFace->pShader->getTexture();
					pFace->texdef.SetName( pReplace );
					found = true;
				}
				else if ( bSelectMatchingFaces ) {
					mFaces.Add( pFace );
				}
			}
		}

		if ( found ) { // spog - speed increase, only build brushes that changed
			Brush_Build( pBrush );
		}

	}

	if ( bSelectMatchingFaces ) {
		if ( bSelected ) {
			Select_Deselect();
		}

		int nSize = mFaces.GetSize();
		for ( int i = 0; i < nSize; i++ ) {
			g_SelectedFaces.Add( reinterpret_cast<face_t *>( mFaces.GetAt( i ) ) );
		}
	}

	Sys_UpdateWindows( W_CAMERA );
}

void Select_AllOfType(){
	brush_t *b, *next;
	entity_t    *e;
	// if no brush selected, we will select based on texture
	//   the first selected face's texture if any, or the current texture
	// if a brush is selected, we will select entities (first non-worldspawn owner in selected brushes)
	if ( selected_brushes.next == &selected_brushes ) {

		CString strName;
		if ( g_ptrSelectedFaces.GetSize() == 0 ) {
			strName = g_qeglobals.d_texturewin.texdef.GetName();
		}
		else
		{
			face_t *selFace = reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( 0 ) );
			strName = selFace->texdef.GetName();
		}

		Sys_Printf( "Selecting all brushes with the texture %s\n", strName.GetBuffer() );

		Select_Deselect();
		for ( b = active_brushes.next ; b != &active_brushes ; b = next )
		{
			next = b->next;

			if ( b->bFiltered ) {
				continue;
			}

			if ( b->patchBrush ) {
				if ( strcmpi( strName, b->pPatch->pShader->getName() ) == 0 ) {
					Brush_RemoveFromList( b );
					Brush_AddToList( b, &selected_brushes );
				}
			}
			else
			{
				for ( face_t* pFace = b->brush_faces; pFace; pFace = pFace->next )
				{
					if ( strcmpi( strName, pFace->texdef.GetName() ) == 0 ) {
						Brush_RemoveFromList( b );
						Brush_AddToList( b, &selected_brushes );
					}
				}
			}
		}
		Sys_UpdateWindows( W_ALL );
		return;
	}


	b = selected_brushes.next;
	e = b->owner;

	if ( e != NULL ) {
		if ( e != world_entity ) {
			CString strName = e->eclass->name;
			CString strKey, strVal;
			bool bCriteria = GetSelectAllCriteria( strKey, strVal );
			Sys_Printf( "Selecting all %s entities\n", strName.GetBuffer() );
			Select_Deselect();

			for ( b = active_brushes.next ; b != &active_brushes ; b = next )
			{
				next = b->next;

				if ( b->bFiltered ) {
					continue;
				}

				e = b->owner;
				if ( e != NULL ) {
					if ( strcmpi( e->eclass->name, strName ) == 0 ) {
						bool doIt = true;
						if ( bCriteria ) {
							CString str = ValueForKey( e, strKey );
							if ( str.CompareNoCase( strVal ) != 0 ) {
								doIt = false;
							}
						}
						if ( doIt ) {
							Brush_RemoveFromList( b );
							Brush_AddToList( b, &selected_brushes );
						}
					}
				}
			}
		}
	}
	Sys_UpdateWindows( W_ALL );

}

void Select_Reselect(){
	Select_Brush( selected_brushes.next );
	Sys_UpdateWindows( W_ALL );
}


void Select_FitTexture( int nHeight, int nWidth ){
	brush_t     *b;

	int nFaceCount = g_ptrSelectedFaces.GetSize();

	if ( selected_brushes.next == &selected_brushes && nFaceCount == 0 ) {
		return;
	}

	for ( b = selected_brushes.next ; b != &selected_brushes ; b = b->next )
	{
		Brush_FitTexture( b, nHeight, nWidth );
		Brush_Build( b,true,true,false,false ); // don't filter
	}

	if ( nFaceCount > 0 ) {
		for ( int i = 0; i < nFaceCount; i++ )
		{
			face_t *selFace = reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( i ) );
			brush_t *selBrush = reinterpret_cast<brush_t*>( g_ptrSelectedFaceBrushes.GetAt( i ) );
			Face_FitTexture( selFace, nHeight, nWidth );
			Brush_Build( selBrush,true,true,false,false ); // don't filter
		}
	}

	Sys_UpdateWindows( W_CAMERA );
}

void Select_Hide(){
	for ( brush_t* b = selected_brushes.next ; b && b != &selected_brushes ; b = b->next )
	{
		b->hiddenBrush = true;
		b->bFiltered = true;
	}
	Sys_UpdateWindows( W_ALL );
}

void Select_ShowAllHidden(){
	brush_t* b;
	for ( b = selected_brushes.next ; b && b != &selected_brushes ; b = b->next )
	{
		if ( b->hiddenBrush ) {
			b->hiddenBrush = false;
			b->bFiltered = FilterBrush( b );
		}
	}
	for ( b = active_brushes.next ; b && b != &active_brushes ; b = b->next )
	{
		if ( b->hiddenBrush ) {
			b->hiddenBrush = false;
			b->bFiltered = FilterBrush( b );
		}
	}
	Sys_UpdateWindows( W_ALL );
}


/*
   ============
   Select_Invert
   ============
 */
void Select_Invert( void ){
	brush_t *next, *prev, *b;

	Sys_Printf( "inverting selection...\n" );

	next = active_brushes.next;
	prev = active_brushes.prev;
	if ( selected_brushes.next != &selected_brushes ) {
		active_brushes.next = selected_brushes.next;
		active_brushes.prev = selected_brushes.prev;
		active_brushes.next->prev = &active_brushes;
		active_brushes.prev->next = &active_brushes;
	}
	else
	{
		active_brushes.next = &active_brushes;
		active_brushes.prev = &active_brushes;
	}
	if ( next != &active_brushes ) {
		selected_brushes.next = next;
		selected_brushes.prev = prev;
		selected_brushes.next->prev = &selected_brushes;
		selected_brushes.prev->next = &selected_brushes;
	}
	else
	{
		selected_brushes.next = &selected_brushes;
		selected_brushes.prev = &selected_brushes;
	}

	// now check if any hidden brush is selected
	for ( b = selected_brushes.next; b != &selected_brushes; )
	{
		if ( b->patchBrush ) {
			b->pPatch->bSelected = true;
		}

		if ( b->bFiltered ) {
			brush_t *pb = b;
			b = b->next;
			Brush_RemoveFromList( pb );
			Brush_AddToList( pb, &active_brushes );
		}
		else{ b = b->next; }

	}

	for ( b = active_brushes.next; b != &active_brushes; b = b->next )
	{
		if ( b->patchBrush ) {
			b->pPatch->bSelected = false;
		}
	}

	// since invert selection only works at the brush level,
	// set g_qeglobals.d_select_mode accordingly
	g_qeglobals.d_select_mode = sel_brush;

	// since invert selection only works at the brush level,
	// set g_qeglobals.d_select_mode accordingly
	g_qeglobals.d_select_mode = sel_brush;

	Sys_UpdateWindows( W_ALL );

	Sys_Printf( "done.\n" );
}

#ifdef ENABLE_GROUPS
/*
   ===========
   Select_Name
   ===========
 */
void Select_Name( const char *pName ){
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		for ( brush_t* b = selected_brushes.next ; b && b != &selected_brushes ; b = b->next )
		{
			Brush_SetEpair( b, "Name", pName );
		}
	}
}

/*
   =================
   Select_AddToGroup
   add selected brushes to a group, update the tree
   =================
 */
void Select_AddToGroup( const char *pName ){
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		for ( brush_t* b = selected_brushes.next ; b && b != &selected_brushes ; b = b->next )
		{
			Brush_SetEpair( b, "group", pName );
			Group_AddToProperGroup( b );
		}
	}
}
#endif
