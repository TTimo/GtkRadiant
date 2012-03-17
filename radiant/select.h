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

typedef struct
{
	brush_t     *brush;
	face_t      *face;
	float dist;
	qboolean selected;
} trace_t;

#define SF_SELECTED_ONLY    0x0001
#define SF_ENTITIES_FIRST   0x0002
#define SF_SINGLEFACE           0x0004
#define SF_IGNORECURVES     0x0008
#define SF_IGNOREGROUPS     0x0010
#define SF_CYCLE                    0x0020
#define SF_CAMERA         0x0040 // set when the operation happens through camera view, otherwise XY
#define SF_DRAG_ON              0x0080
#define SF_DRAG_OFF             0x0100
#define SF_DRAG                     ( SF_DRAG_ON | SF_DRAG_OFF )

trace_t Test_Ray( vec3_t origin, vec3_t dir, int flags );

void Select_GetBounds( vec3_t mins, vec3_t maxs );
void Select_GetMid( vec3_t mid );
void Select_Brush( brush_t *b, bool bComplete = true, bool bStatus = true );
void Select_Ray( vec3_t origin, vec3_t dir, int flags );
void Select_Delete( void );
void Select_Deselect( bool bDeselectFaces = true );
void Select_Invert( void );
void Select_Clone( void );
void Select_Move( vec3_t delta, bool bSnap = true );
void Select_NudgePoint( vec3_t delta, qboolean bSnap = true );
void WINAPI Select_SetTexture( texdef_t *texdef, brushprimit_texdef_t *brushprimit_texdef, bool bFitScale = false, void* pPlugTexdef = NULL );
void Select_FlipAxis( int axis );
void Select_RotateAxis( int axis, float deg, bool bPaint = true, bool bMouse = false );
void Select_RealCompleteTall( vec3_t mins, vec3_t maxs );
void Select_CompleteTall( void );
void Select_PartialTall( void );
void Select_Touching( void );
void Select_Inside( void );
void Select_Seperate( void );
void Select_MakeStructural( void );
void Select_MakeDetail( void );
void Select_AllOfType();
void Select_Reselect();
void Select_FitTexture( int nHeight = 1, int nWidth = 1 );

void Select_SelectGroup( entity_t* group );

// absolute texture coordinates
// TTimo NOTE: this is stuff for old brushes format and rotation texture lock .. sort of in-between with bush primitives
void ComputeAbsolute( face_t* f, vec3_t& p1, vec3_t& p2, vec3_t& p3 );
void AbsoluteToLocal( plane_t normal2, face_t* f, vec3_t& p1, vec3_t& p2, vec3_t& p3 );
void Select_Hide();
void Select_ShowAllHidden();
// add selected brushes to a group, update the tree
//void Select_AddToGroup(const char *pName);
//void Select_Name(const char *pName);

// updating workzone to a given brush (depends on current view)
void UpdateWorkzone_ForBrush( brush_t* b );

void Select_GroupEntity( entity_t* e );
void Select_MergeEntity();
