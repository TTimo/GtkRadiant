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

// DPatch.cpp: implementation of the DPatch class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DPatch.h"
#include "misc.h"
#include "./dialogs/dialogs-gtk.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//		Added patch merging, wahey!

//
//		problem is, you cant put patches into entities as yet :(
//

DPatch::DPatch(){
	width = MIN_PATCH_WIDTH;
	height = MIN_PATCH_HEIGHT;
	QER_patch = NULL;
	QER_brush = NULL;
}

DPatch::~DPatch(){

}

void DPatch::SetTexture( const char *textureName ){
	strcpy( texture, textureName );
}

void CopyDrawVert( const drawVert_t* in, drawVert_t* out ){
	out->lightmap[0] = in->lightmap[0];
	out->lightmap[1] = in->lightmap[1];
	out->st[0] = in->st[0];
	out->st[1] = in->st[1];
	VectorCopy( in->normal, out->normal );
	VectorCopy( in->xyz, out->xyz );
}

void DPatch::BuildInRadiant( void* entity ){
	int nIndex = g_FuncTable.m_pfnCreatePatchHandle();
	//$ FIXME: m_pfnGetPatchHandle
	patchMesh_t* pm = g_FuncTable.m_pfnGetPatchData( nIndex );

	pm->height = height;
	pm->width = width;

	for ( int x = 0; x < width; x++ )
		for ( int y = 0; y < height; y++ )
			CopyDrawVert( &points[x][y], &pm->ctrl[x][y] );

	QER_patch = pm;

/*	if(entity)
    {
   //		strcpy(pm->d_texture->name, texture);

        brush_t* brush = (brush_t*)g_FuncTable.m_pfnCreateBrushHandle();
        brush->patchBrush = TRUE;
        brush->pPatch = pm;

        pm->pSymbiot = brush;
        pm->bSelected = false;
        pm->bOverlay = false;	// bleh, f*cks up, just have to wait for a proper function
        pm->bDirty = true;		// or get my own patch out....
        pm->nListID = -1;

        g_FuncTable.m_pfnCommitBrushHandleToEntity(brush, entity);
    }
    else*/                                                                                                                                                                                                                                                                                                                                                                                                                                                                // patch to entity just plain dont work atm

	if ( entity ) {
		g_FuncTable.m_pfnCommitPatchHandleToEntity( nIndex, pm, texture, entity );
	}
	else{
		g_FuncTable.m_pfnCommitPatchHandleToMap( nIndex, pm, texture );
	}

	QER_brush = pm->pSymbiot;
}

void DPatch::LoadFromBrush_t( brush_t* brush ){
	QER_brush = brush;
	QER_patch = brush->pPatch;

	SetTexture( QER_patch->pShader->getName() );

	for ( int x = 0; x < QER_patch->width; x++ )
		for ( int y = 0; y < QER_patch->height; y++ )
			CopyDrawVert( &QER_patch->ctrl[x][y], &points[x][y] );

	width = QER_patch->width;
	height = QER_patch->height;
}

void DPatch::RemoveFromRadiant(){
	if ( QER_brush ) {
		g_FuncTable.m_pfnDeleteBrushHandle( QER_brush );
	}
}

bool DPatch::ResetTextures( const char *oldTextureName, const char *newTextureName ){
	if ( !oldTextureName || !strcmp( texture, oldTextureName ) ) {
		strcpy( texture, newTextureName );
		return TRUE;
	}

	return FALSE;
}

void Build1dArray( vec3_t* array, drawVert_t points[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT],
				   int startX, int startY, int number, bool horizontal, bool inverse ){
	int x = startX, y = startY, i, step;

	if ( inverse ) {
		step = -1;
	}
	else{
		step = 1;
	}

	for ( i = 0; i < number; i++ )
	{
		VectorCopy( points[x][y].xyz, array[i] );

		if ( horizontal ) {
			x += step;
		}
		else{
			y += step;
		}
	}
}

void Print1dArray( vec3_t* array, int size ){
	for ( int i = 0; i < size; i++ )
		Sys_Printf( "(%.0f %.0f %.0f)\t", array[i][0], array[i][1], array[i][2] );
	Sys_Printf( "\n" );
}

bool Compare1dArrays( vec3_t* a1, vec3_t* a2, int size ){
	int i;
	bool equal = true;

	for ( i = 0; i < size; i++ )
	{
		if ( !VectorCompare( a1[i], a2[size - i - 1] ) ) {
			equal = false;
			break;
		}
	}
	return equal;
}

patch_merge_t DPatch::IsMergable( DPatch *other ){
	int i, j;
	vec3_t p1Array[4][MAX_PATCH_HEIGHT];
	vec3_t p2Array[4][MAX_PATCH_HEIGHT];

	int p1ArraySizes[4];
	int p2ArraySizes[4];

	patch_merge_t merge_info;

	Build1dArray( p1Array[0], this->points, 0,               0,              this->width,    true,   false );
	Build1dArray( p1Array[1], this->points, this->width - 1,   0,              this->height,   false,  false );
	Build1dArray( p1Array[2], this->points, this->width - 1,   this->height - 1, this->width,    true,   true );
	Build1dArray( p1Array[3], this->points, 0,               this->height - 1, this->height,   false,  true );

	Build1dArray( p2Array[0], other->points, 0,              0,                  other->width,   true,   false );
	Build1dArray( p2Array[1], other->points, other->width - 1, 0,                  other->height,  false,  false );
	Build1dArray( p2Array[2], other->points, other->width - 1, other->height - 1,    other->width,   true,   true );
	Build1dArray( p2Array[3], other->points, 0,              other->height - 1,    other->height,  false,  true );

	p1ArraySizes[0] = this->width;
	p1ArraySizes[1] = this->height;
	p1ArraySizes[2] = this->width;
	p1ArraySizes[3] = this->height;

	p2ArraySizes[0] = other->width;
	p2ArraySizes[1] = other->height;
	p2ArraySizes[2] = other->width;
	p2ArraySizes[3] = other->height;

	for ( i = 0; i < 4; i++ )
	{
		for ( j = 0; j < 4; j++ )
		{
			if ( p1ArraySizes[i] == p2ArraySizes[j] ) {
				if ( Compare1dArrays( p1Array[i], p2Array[j], p1ArraySizes[i] ) ) {
					merge_info.pos1 = i;
					merge_info.pos2 = j;
					merge_info.mergable = true;
					return merge_info;
				}
			}
		}
	}

	merge_info.mergable = false;
	return merge_info;
}

DPatch* DPatch::MergePatches( patch_merge_t merge_info, DPatch *p1, DPatch *p2 ){
	while ( merge_info.pos1 != 2 )
	{
		p1->Transpose();
		merge_info.pos1--;
		if ( merge_info.pos1 < 0 ) {
			merge_info.pos1 += 4;
		}
	}

	while ( merge_info.pos2 != 0 )
	{
		p2->Transpose();
		merge_info.pos2--;
		if ( merge_info.pos2 < 0 ) {
			merge_info.pos2 += 3;
		}
	}

	int newHeight = p1->height + p2->height - 1;
	if ( newHeight > MAX_PATCH_HEIGHT ) {
		return NULL;
	}

	DPatch* newPatch = new DPatch();

	newPatch->height    = newHeight;
	newPatch->width     = p1->width;
	newPatch->SetTexture( p1->texture );

	int y = 0;
	int i;
	for ( i = 0; i < p1->height; i++, y++ )
		for ( int x = 0; x < p1->width; x++ )
			memcpy( &newPatch->points[x][y], &p1->points[x][i],  sizeof( drawVert_t ) );

	for ( i = 1; i < p2->height; i++, y++ )
		for ( int x = 0; x < p2->width; x++ )
			memcpy( &newPatch->points[x][y], &p2->points[x][i],  sizeof( drawVert_t ) );

//	newPatch->Invert();

	return newPatch;
}

void DPatch::Invert(){
	drawVert_t vertTemp;
	int i, j;

	for ( i = 0 ; i < width ; i++ )
	{
		for ( j = 0; j < height / 2; j++ )
		{
			memcpy( &vertTemp, &points[i][height - 1 - j], sizeof( drawVert_t ) );
			memcpy( &points[i][height - 1 - j], &points[i][j], sizeof( drawVert_t ) );
			memcpy( &points[i][j], &vertTemp, sizeof( drawVert_t ) );
		}
	}
}

void DPatch::Transpose(){
	int i, j, w;
	drawVert_t dv;

	if ( width > height ) {
		for ( i = 0 ; i < height ; i++ )
		{
			for ( j = i + 1 ; j < width ; j++ )
			{
				if ( j < height ) {
					// swap the value
					memcpy( &dv,             &points[j][i],  sizeof( drawVert_t ) );
					memcpy( &points[j][i],   &points[i][j],  sizeof( drawVert_t ) );
					memcpy( &points[i][j],   &dv,            sizeof( drawVert_t ) );
				}
				else
				{
					// just copy
					memcpy( &points[i][j],   &points[j][i],  sizeof( drawVert_t ) );
				}
			}
		}
	}
	else
	{
		for ( i = 0 ; i < width ; i++ )
		{
			for ( j = i + 1 ; j < height ; j++ )
			{
				if ( j < width ) {
					// swap the value
					memcpy( &dv,             &points[i][j],  sizeof( drawVert_t ) );
					memcpy( &points[i][j],   &points[j][i],  sizeof( drawVert_t ) );
					memcpy( &points[j][i],   &dv,            sizeof( drawVert_t ) );
				}
				else
				{
					// just copy
					memcpy( &points[j][i],   &points[i][j],  sizeof( drawVert_t ) );
				}
			}
		}
	}

	w = width;
	width = height;
	height = w;

	Invert();
}

list<DPatch> DPatch::Split( bool rows, bool cols ){
	list<DPatch> patchList;
	int i;
	int x, y;

	if ( rows && height >= 5 ) {
		for ( i = 0; i < ( height - 1 ) / 2; i++ )
		{
			DPatch p;

			p.width = width;
			p.height = 3;
			p.SetTexture( texture );

			for ( y = 0; y < 3; y++ )
			{
				for ( x = 0; x < p.width; x++ )
				{
					memcpy( &p.points[x][y], &points[x][( i * 2 ) + y],    sizeof( drawVert_t ) );
				}
			}
			patchList.push_back( p );
		}

		if ( cols && width >= 5 ) {
			list<DPatch> patchList2;

			for ( list<DPatch>::iterator patches = patchList.begin(); patches != patchList.end(); patches++ )
			{
				list<DPatch> patchList3 = ( *patches ).Split( false, true );

				for ( list<DPatch>::iterator patches2 = patchList3.begin(); patches2 != patchList3.end(); patches2++ )
					patchList2.push_front( *patches2 );
			}

			return patchList2;
		}
	}
	else if ( cols && width >= 5 ) {
		for ( i = 0; i < ( width - 1 ) / 2; i++ )
		{
			DPatch p;

			p.height = height;
			p.width = 3;
			p.SetTexture( texture );

			for ( x = 0; x < 3; x++ )
			{
				for ( y = 0; y < p.height; y++ )
				{
					memcpy( &p.points[x][y], &points[( i * 2 ) + x][y],    sizeof( drawVert_t ) );
				}
			}

			patchList.push_back( p );
		}
	}

	return patchList;
}
