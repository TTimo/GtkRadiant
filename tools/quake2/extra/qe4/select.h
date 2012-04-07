/*
===========================================================================
Copyright (C) 1997-2006 Id Software, Inc.

This file is part of Quake 2 Tools source code.

Quake 2 Tools source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake 2 Tools source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake 2 Tools source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

typedef enum
{
	sel_brush,
	// sel_sticky_brush,
	// sel_face,
	sel_vertex,
	sel_edge
} select_t;

typedef struct
{
	brush_t		*brush;
	face_t		*face;
	float		dist;
	qboolean	selected;
} trace_t;

#define	SF_SELECTED_ONLY	1
#define	SF_ENTITIES_FIRST	2
#define	SF_SINGLEFACE		4


trace_t Test_Ray (vec3_t origin, vec3_t dir, int flags);

void Select_GetBounds (vec3_t mins, vec3_t maxs);
void Select_Brush (brush_t *b);
void Select_Ray (vec3_t origin, vec3_t dir, int flags);
void Select_Delete (void);
void Select_Deselect (void);
void Select_Clone (void);
void Select_Move (vec3_t delta);
void Select_SetTexture (texdef_t *texdef);
void Select_FlipAxis (int axis);
void Select_RotateAxis (int axis, float deg);
void Select_CompleteTall (void);
void Select_PartialTall (void);
void Select_Touching (void);
void Select_Inside (void);
void Select_MakeStructural (void);
void Select_MakeDetail (void);
