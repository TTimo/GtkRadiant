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

#ifndef _IBRUSH_H_
#define _IBRUSH_H_

//
// API for brush stuff
//

#define BRUSH_MAJOR "brush"
// {c1c3f567-2541-4aa3-9d5b-031fbe2a013b}
static const GUID QERBrushTable_GUID =
{ 0xc1c3f567, 0x2541, 0x4aa3, { 0x9d, 0x5b, 0x03, 0x1f, 0xbe, 0x2a, 0x01, 0x3b } };

typedef void ( *PFN_BRUSHADDTOLIST )( brush_t *b, brush_t *lst );
typedef void ( *PFN_BRUSHBUILD )( brush_t *b, bool bSnap, bool bMarkMap, bool bConvert, bool bFilterTest );
typedef brush_t*  ( *PFN_BRUSHCREATE )( vec3_t mins, vec3_t maxs, texdef_t *texdef );
typedef void ( *PFN_BRUSHFREE )( brush_t *b, bool bRemoveNode );
typedef void ( *PFN_BRUSHROTATE )( brush_t *b, vec3_t vAngle, vec3_t vOrigin, bool bBuild );
typedef brush_t*  ( *PFN_BRUSHALLOC )();
typedef int ( *PFN_BPMESSAGEBOX )( int );
typedef face_t*   ( *PFN_FACEALLOC )( void );
typedef eclass_t* ( *PFN_HASMODEL )( brush_t *b );

struct _QERBrushTable
{
	int m_nSize;
	PFN_BRUSHADDTOLIST m_pfnBrush_AddToList;
	PFN_BRUSHBUILD m_pfnBrush_Build;
	PFN_BRUSHCREATE m_pfnBrush_Create;
	PFN_BRUSHFREE m_pfnBrush_Free;
	PFN_BRUSHROTATE m_pfnBrush_Rotate;
	PFN_BRUSHALLOC m_pfnBrushAlloc;
	PFN_BPMESSAGEBOX m_pfnBP_MessageBox;
	PFN_FACEALLOC m_pfnFace_Alloc;
	PFN_HASMODEL m_pfnHasModel;
};

#ifdef USE_BRUSHTABLE_DEFINE
#ifndef __BRUSHTABLENAME
#define __BRUSHTABLENAME g_BrushTable
#endif
#define Brush_AddToList __BRUSHTABLENAME.m_pfnBrush_AddToList
#define Brush_Build __BRUSHTABLENAME.m_pfnBrush_Build
#define Brush_Create __BRUSHTABLENAME.m_pfnBrush_Create
#define Brush_Free __BRUSHTABLENAME.m_pfnBrush_Free
#define Brush_Rotate __BRUSHTABLENAME.m_pfnBrush_Rotate
#define Brush_Alloc __BRUSHTABLENAME.m_pfnBrushAlloc
#define BP_MessageBox __BRUSHTABLENAME.m_pfnBP_MessageBox
#define Face_Alloc __BRUSHTABLENAME.m_pfnFace_Alloc
#define HasModel __BRUSHTABLENAME.m_pfnHasModel
#endif

#endif
