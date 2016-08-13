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
// Quick interface hack for selected face interface
// this one really needs more work, but I'm in a hurry with TexTool

#include "stdafx.h"

int WINAPI QERApp_GetSelectedFaceCount(){
	return g_ptrSelectedFaces.GetSize();
}

face_t* WINAPI QERApp_GetSelectedFace( int iface ){
	if ( iface >= g_ptrSelectedFaces.GetSize() ) {
		Sys_FPrintf( SYS_ERR, "QERApp_GetFace: selected faces count exceeded\n" );
		return NULL;
	}
	return reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( iface ) );
}

brush_t* WINAPI QERApp_GetSelectedFaceBrush( int iface ){
	if ( iface >= g_ptrSelectedFaceBrushes.GetSize() ) {
		Sys_FPrintf( SYS_ERR, "QERApp_GetFace: selected faces count exceeded\n" );
		return NULL;
	}
	return reinterpret_cast<brush_t*>( g_ptrSelectedFaceBrushes.GetAt( iface ) );
}

// NOTE: we expect pWinding to have MAX_POINTS_ON_WINDING points ready for writing
int WINAPI QERApp_GetFaceInfo( int iface, _QERFaceData *pFaceData, winding_t *pWinding ){
	size_t size;

	if ( iface >= g_ptrSelectedFaces.GetSize() ) {
		Sys_FPrintf( SYS_ERR, "QERApp_GetFaceInfo: selected faces count exceeded\n" );
		return 0;
	}
	if ( !g_qeglobals.m_bBrushPrimitMode ) {
		Sys_FPrintf( SYS_WRN, "Warning: unexpected QERApp_GetFaceInfo out of brush primitive mode\n" );
		return 0;
	}
	face_t *selFace = reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( iface ) );
	strcpy( pFaceData->m_TextureName, selFace->texdef.GetName() );
	VectorCopy( selFace->planepts[0], pFaceData->m_v1 );
	VectorCopy( selFace->planepts[1], pFaceData->m_v2 );
	VectorCopy( selFace->planepts[2], pFaceData->m_v3 );
	pFaceData->m_bBPrimit = true;
	memcpy( &pFaceData->brushprimit_texdef, &selFace->brushprimit_texdef, sizeof( brushprimit_texdef_t ) );
	size = (size_t)( (winding_t *)0 )->points[selFace->face_winding->numpoints];
	memcpy( pWinding, selFace->face_winding, size );
	return 1;
}

int WINAPI QERApp_SetFaceInfo( int iface, _QERFaceData *pFaceData ){
	if ( iface >= g_ptrSelectedFaces.GetSize() ) {
		Sys_FPrintf( SYS_ERR, "QERApp_SetFaceInfo: selected faces count exceeded\n" );
		return 0;
	}
	if ( !g_qeglobals.m_bBrushPrimitMode ) {
		Sys_FPrintf( SYS_WRN, "Warning: unexpected QERApp_SetFaceInfo out of brush primitive mode\n" );
		return 0;
	}
	face_t *selFace = reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( iface ) );
	brush_t *selBrush = reinterpret_cast<brush_t*>( g_ptrSelectedFaceBrushes.GetAt( iface ) );
	//strcpy( selected_face->texdef.name, pFaceData->m_TextureName );
	selFace->texdef.SetName( pFaceData->m_TextureName );
	VectorCopy( pFaceData->m_v1, selFace->planepts[0] );
	VectorCopy( pFaceData->m_v2, selFace->planepts[1] );
	VectorCopy( pFaceData->m_v3, selFace->planepts[2] );
	memcpy( &selFace->brushprimit_texdef, &pFaceData->brushprimit_texdef, sizeof( brushprimit_texdef_t ) );
	Brush_Build( selBrush );
	Sys_UpdateWindows( W_ALL );
	return 1;
}

int WINAPI QERApp_ISelectedFace_GetTextureNumber( int iface ){
	if ( iface >= g_ptrSelectedFaces.GetSize() ) {
		Sys_FPrintf( SYS_ERR, "QERApp_ISelectedFace_GetTextureNumber: selected faces count exceeded\n" );
		return 0;
	}
	face_t *selFace = reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( iface ) );
	return selFace->d_texture->texture_number;
}

void WINAPI QERApp_GetTextureSize( int iface, int Size[2] ){
	if ( iface >= g_ptrSelectedFaces.GetSize() ) {
		Sys_FPrintf( SYS_ERR, "QERApp_GetTextureSize: selected faces count exceeded\n" );
		return;
	}
	face_t *selFace = reinterpret_cast<face_t*>( g_ptrSelectedFaces.GetAt( iface ) );
	Size[0] = selFace->d_texture->width;
	Size[1] = selFace->d_texture->height;
}
