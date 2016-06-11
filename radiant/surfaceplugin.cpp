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

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
// implementation of isurfaceplugin-interface specifics

#include "stdafx.h"

void QERApp_GetTwoSelectedPatch( patchMesh_t **p1, patchMesh_t **p2 ){
	*p1 = NULL; *p2 = NULL;
	for ( brush_t *pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next )
	{
		if ( pb->patchBrush ) {
			if ( !( *p1 ) ) {
				*p1 = pb->pPatch;
			}
			else if ( !( *p2 ) ) {
				*p2 = pb->pPatch;
				return;
			}
		}
	}
#ifdef _DEBUG
	Sys_FPrintf( SYS_WRN, "WARNING: QERApp_GetTwoSelectedPatch failed (did not find two patches)\n" );
#endif
	return;
}

// Nurail: The following functions are used by the Surface Inspector module

// Queries the number of faces from selected brushes
int SI_GetSelectedFaceCountfromBrushes( void ){
	face_t    *f;
	brush_t   *b;
	int num_of_faces = 0;

	if ( selected_brushes.next == &selected_brushes ) {
		return( 0 );
	}

	for ( b = selected_brushes.next; b != &selected_brushes; b = b->next )
		if ( !( b->patchBrush ) ) {
			for ( f = b->brush_faces; f ; f = f->next, num_of_faces++ ) ;
		}

	return num_of_faces;
}

void SI_GetSelFacesTexdef( texdef_to_face_t *allocd_block_texdef ){
	int i;
	face_t    *f;
	brush_t   *b;
	texdef_to_face_t *position, *prev_pos;
	brushprimit_texdef_t bp;

	if ( selected_brushes.next != &selected_brushes ) {
		prev_pos = position = allocd_block_texdef;
		for ( b = selected_brushes.next; b != &selected_brushes; b = b->next )
		{
			if ( !( b->patchBrush ) ) {
				for ( f = b->brush_faces; f ; f = f->next )
				{
					position->face = f;
					position->brush = b;
					position->texdef = f->texdef;
					if ( g_qeglobals.m_bBrushPrimitMode ) {
						ConvertTexMatWithQTexture( &f->brushprimit_texdef, QERApp_Shader_ForName( f->texdef.GetName() )->getTexture(), &bp, NULL );
						TexMatToFakeTexCoords( bp.coords, position->texdef.shift, &position->texdef.rotate, position->texdef.scale );
						position->orig_bp_texdef = bp;
					}
					position->orig_texdef = position->texdef;
					prev_pos->next = position;
					prev_pos = position;
					position++;
				}
				prev_pos->next = NULL;
			}
		}
	}
	else if ( g_ptrSelectedFaces.GetSize() != 0 ) {
		f = (face_t *) g_ptrSelectedFaces.GetAt( 0 );
		b = (brush_t *) g_ptrSelectedFaceBrushes.GetAt( 0 );
		position = (texdef_to_face_t*) allocd_block_texdef;
		position->face = f;
		position->brush = b;
		position->texdef = f->texdef;
		if ( g_qeglobals.m_bBrushPrimitMode ) {
			ConvertTexMatWithQTexture( &f->brushprimit_texdef, QERApp_Shader_ForName( f->texdef.GetName() )->getTexture(), &bp, NULL );
			TexMatToFakeTexCoords( bp.coords, position->texdef.shift, &position->texdef.rotate, position->texdef.scale );
			position->orig_bp_texdef = bp;
		}
		position->orig_texdef = position->texdef;
		prev_pos = position;
		for ( i = 1; i < g_ptrSelectedFaces.GetSize(); i++ )
		{
			f = (face_t *) g_ptrSelectedFaces.GetAt( i );
			b = (brush_t *) g_ptrSelectedFaceBrushes.GetAt( i );
			position = allocd_block_texdef + i;
			position->face = f;
			position->brush = b;
			position->texdef = f->texdef;
			if ( g_qeglobals.m_bBrushPrimitMode ) {
				ConvertTexMatWithQTexture( &f->brushprimit_texdef, QERApp_Shader_ForName( f->texdef.GetName() )->getTexture(), &bp, NULL );
				TexMatToFakeTexCoords( bp.coords, position->texdef.shift, &position->texdef.rotate, position->texdef.scale );
				position->orig_bp_texdef = bp;
			}
			position->orig_texdef = position->texdef;
			prev_pos->next = position;
			prev_pos = position;
		}
		position->next = NULL;
	}

}

/*
   SetFaceTexdef_Q2

   This doesn't mess with CONTENTS_DETAIL needed for Quake2 content flag

 */
void SetFaceTexdef_Q2( face_t *f, texdef_t *texdef, bool bFitScale ){

	if ( strcmp( f->texdef.GetName(), texdef->GetName() ) != 0 ) { // set shader here instead of Brush_Build
		Face_SetShader( f, texdef->GetName() );
	}

	if ( bFitScale ) {
		f->texdef = *texdef;
		// fit the scaling of the texture on the actual plane
		vec3_t p1,p2,p3;     // absolute coordinates
		// compute absolute coordinates
		ComputeAbsolute( f,p1,p2,p3 );
		// compute the scale
		vec3_t vx,vy;
		VectorSubtract( p2,p1,vx );
		VectorNormalize( vx, vx );
		VectorSubtract( p3,p1,vy );
		VectorNormalize( vy, vy );
		// assign scale
		VectorScale( vx,texdef->scale[0],vx );
		VectorScale( vy,texdef->scale[1],vy );
		VectorAdd( p1,vx,p2 );
		VectorAdd( p1,vy,p3 );
		// compute back shift scale rot
		AbsoluteToLocal( f->plane,f,p1,p2,p3 );
	}
	else
	{
		f->texdef = *texdef;
	}
}



void SI_SetTexdef_FaceList( texdef_to_face_t* texdef_face_list, bool b_SetUndoPoint, bool bFit_to_Scale ){
	texdef_to_face_t* texdef_to_face;

	if ( !texdef_face_list ) {
		return;
	}

	if ( b_SetUndoPoint ) {
		if ( g_ptrSelectedFaces.GetSize() > 1 ) {
			Sys_FPrintf( SYS_WRN, "WARNING: Undo NOT supported for multiple face selections\n" );
		}
		else if ( ( selected_brushes.next != &selected_brushes ) || ( g_ptrSelectedFaces.GetSize() == 1 ) ) {
			// Give something to undo to
			for ( texdef_to_face = texdef_face_list; texdef_to_face; texdef_to_face = texdef_to_face->next ) {
			  SetFaceTexdef( texdef_to_face->face, &texdef_to_face->orig_texdef, &texdef_to_face->orig_bp_texdef, bFit_to_Scale );
                        }

			Undo_Start( "set facelist texdefs" );

			if ( selected_brushes.next != &selected_brushes ) {
				Undo_AddBrushList( &selected_brushes );
			}
			else{
				Undo_AddBrush( texdef_face_list->brush );
			}

                }
	}

	for ( texdef_to_face = texdef_face_list; texdef_to_face; texdef_to_face = texdef_to_face->next )
	{
		{
			brushprimit_texdef_t brushprimit_texdef;
			FakeTexCoordsToTexMat( texdef_to_face->texdef.shift, texdef_to_face->texdef.rotate, texdef_to_face->texdef.scale, brushprimit_texdef.coords );
			SetFaceTexdef( texdef_to_face->face, &texdef_to_face->texdef, &brushprimit_texdef, bFit_to_Scale );
		}
		Brush_Build( texdef_to_face->brush );
		if ( bFit_to_Scale ) {
			texdef_to_face->texdef = texdef_to_face->face->texdef;
		}
	}

	if ( b_SetUndoPoint ) {
		if ( ( selected_brushes.next != &selected_brushes ) || ( g_ptrSelectedFaces.GetSize() == 1 ) ) {
			if ( selected_brushes.next != &selected_brushes ) {
				Undo_EndBrushList( &selected_brushes );
			}
			else{
				Undo_EndBrush( texdef_face_list->brush );
			}

			Undo_End();
			// Over-write the orig_texdef list, cementing the change.
			for ( texdef_to_face = texdef_face_list; texdef_to_face; texdef_to_face = texdef_to_face->next )
			{
				texdef_to_face->orig_texdef = texdef_to_face->texdef;
				texdef_to_face->orig_bp_texdef = texdef_to_face->face->brushprimit_texdef;
			}
		}
	}

	Sys_UpdateWindows( W_ALL );
}

void SI_FaceList_FitTexture( texdef_to_face_t* si_texdef_face_list, int nHeight, int nWidth ){
	texdef_to_face_t* temp_texdef_face_list;
	brushprimit_texdef_t bp;

	if ( !si_texdef_face_list ) {
		return;
	}

	for ( temp_texdef_face_list = si_texdef_face_list; temp_texdef_face_list; temp_texdef_face_list = temp_texdef_face_list->next )
	{
		Face_FitTexture( temp_texdef_face_list->face, nHeight, nWidth );
		Brush_Build( temp_texdef_face_list->brush,true,true,false,false );
		// Write changes to our working Texdef list

		if ( g_qeglobals.m_bBrushPrimitMode ) {
			ConvertTexMatWithQTexture( &temp_texdef_face_list->face->brushprimit_texdef, QERApp_Shader_ForName( temp_texdef_face_list->face->texdef.GetName() )->getTexture(), &bp, NULL );
			TexMatToFakeTexCoords( bp.coords, temp_texdef_face_list->face->texdef.shift, &temp_texdef_face_list->face->texdef.rotate, temp_texdef_face_list->face->texdef.scale );
		}
		temp_texdef_face_list->texdef = temp_texdef_face_list->face->texdef;
	}

	Sys_UpdateWindows( W_CAMERA );

}

GtkWindow* SI_GetMainWindow( void ){
	return GTK_WINDOW( g_qeglobals_gui.d_main_window );
}

void SI_SetWinPos_from_Prefs( GtkWidget *win ){
	load_window_pos( win, g_PrefsDlg.mWindowInfo.posSurfaceWnd );
}
