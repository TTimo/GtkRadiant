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

// DPatch.h: interface for the DPatch class.
//
//////////////////////////////////////////////////////////////////////

#if !defined( AFX_DPATCH_H__26C6B083_CE5B_420B_836B_1DDA733C04CE__INCLUDED_ )
#define AFX_DPATCH_H__26C6B083_CE5B_420B_836B_1DDA733C04CE__INCLUDED_

#include "StdAfx.h" // Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct
{
	bool mergable;
	int pos1;
	int pos2;
} patch_merge_t;

class DPatch
{
public:
list<DPatch> Split( bool rows, bool cols );
void Transpose();
void Invert();
DPatch* MergePatches( patch_merge_t merge_info, DPatch* p1, DPatch* p2 );
patch_merge_t IsMergable( DPatch* other );
bool ResetTextures( const char *oldTextureName, const char *newTextureName );
void RemoveFromRadiant( void );
brush_t* QER_brush;
void LoadFromBrush_t( brush_t* brush );
patchMesh_t* QER_patch;
void BuildInRadiant( void* entity = NULL );
void SetTexture( const char* textureName );
char texture[256];
int width, height;
drawVert_t points[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT];
DPatch();
virtual ~DPatch();

};

#endif // !defined(AFX_DPATCH_H__26C6B083_CE5B_420B_836B_1DDA733C04CE__INCLUDED_)
