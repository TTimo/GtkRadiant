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

// select.c

#include "qe3.h"

/*
===========
Test_Ray
===========
*/
#define	DIST_START	999999
trace_t Test_Ray (vec3_t origin, vec3_t dir, int flags)
{
	brush_t	*brush;
	face_t	*face;
	float	dist;
	trace_t	t;

	memset (&t, 0, sizeof(t));
	t.dist = DIST_START;

	if (! (flags & SF_SELECTED_ONLY) )
		for (brush = active_brushes.next ; brush != &active_brushes ; brush=brush->next)
		{
			if ( (flags & SF_ENTITIES_FIRST) && brush->owner == world_entity)
				continue;
			if (FilterBrush (brush))
				continue;
			face = Brush_Ray (origin, dir, brush, &dist);
			if (dist > 0 && dist < t.dist)
			{
				t.dist = dist;
				t.brush = brush;
				t.face = face;
				t.selected = false;
			}
		}
	for (brush = selected_brushes.next ; brush != &selected_brushes ; brush=brush->next)
	{
		if ( (flags & SF_ENTITIES_FIRST) && brush->owner == world_entity)
			continue;
		if (FilterBrush (brush))
			continue;
		face = Brush_Ray (origin, dir, brush, &dist);
		if (dist > 0 && dist < t.dist)
		{
			t.dist = dist;
			t.brush = brush;
			t.face = face;
			t.selected = true;
		}
	}

	// if entites first, but didn't find any, check regular

	if ( (flags & SF_ENTITIES_FIRST) && t.brush == NULL)
		return Test_Ray (origin, dir, flags - SF_ENTITIES_FIRST);

	return t;
}


/*
============
Select_Brush

============
*/
void Select_Brush (brush_t *brush)
{
	brush_t	*b;
	entity_t	*e;

	selected_face = NULL;
	if (g_qeglobals.d_select_count < 2)
		g_qeglobals.d_select_order[g_qeglobals.d_select_count] = brush;
	g_qeglobals.d_select_count++;

	e = brush->owner;
	if (e)
	{
		// select complete entity on first click
		if (e != world_entity)
		{
			for (b=selected_brushes.next ; b != &selected_brushes ; b=b->next)
				if (b->owner == e)
					goto singleselect;
			for (b=e->brushes.onext ; b != &e->brushes ; b=b->onext)
			{
				Brush_RemoveFromList (b);
				Brush_AddToList (b, &selected_brushes);
			}
		}
		else
		{
singleselect:
			Brush_RemoveFromList (brush);
			Brush_AddToList (brush, &selected_brushes);
		}

		if (e->eclass)
		{
			UpdateEntitySel(brush->owner->eclass);
		}
	}
}

/*
============
Select_Ray

If the origin is inside a brush, that brush will be ignored.
============
*/
void Select_Ray (vec3_t origin, vec3_t dir, int flags)
{
	trace_t	t;

	t = Test_Ray (origin, dir, flags);
	if (!t.brush)
		return;

	if (flags == SF_SINGLEFACE)
	{
		selected_face = t.face;
		selected_face_brush = t.brush;
		Sys_UpdateWindows (W_ALL);
		g_qeglobals.d_select_mode = sel_brush;
		return;
	}

	// move the brush to the other list

	g_qeglobals.d_select_mode = sel_brush;

	if (t.selected)
	{
		Brush_RemoveFromList (t.brush);
		Brush_AddToList (t.brush, &active_brushes);
	} else
	{
		Select_Brush (t.brush);
	}

	Sys_UpdateWindows (W_ALL);
}


void Select_Delete (void)
{
	brush_t	*brush;

	selected_face = NULL;
	g_qeglobals.d_select_mode = sel_brush;

	g_qeglobals.d_select_count = 0;
	g_qeglobals.d_num_move_points = 0;
	while (selected_brushes.next != &selected_brushes)
	{
		brush = selected_brushes.next;
		Brush_Free (brush);
	}

	// FIXME: remove any entities with no brushes

	Sys_UpdateWindows (W_ALL);
}

void Select_Deselect (void)
{
	brush_t	*b;

	g_qeglobals.d_workcount++;
	g_qeglobals.d_select_count = 0;
	g_qeglobals.d_num_move_points = 0;
	b = selected_brushes.next;

	if (b == &selected_brushes)
	{
		if (selected_face)
		{
			selected_face = NULL;
			Sys_UpdateWindows (W_ALL);
		}
		return;
	}

	selected_face = NULL;
	g_qeglobals.d_select_mode = sel_brush;

	// grab top / bottom height for new brushes
	if (b->mins[2] < b->maxs[2])
	{
		g_qeglobals.d_new_brush_bottom_z = b->mins[2];
		g_qeglobals.d_new_brush_top_z = b->maxs[2];
	}

	selected_brushes.next->prev = &active_brushes;
	selected_brushes.prev->next = active_brushes.next;
	active_brushes.next->prev = selected_brushes.prev;
	active_brushes.next = selected_brushes.next;
	selected_brushes.prev = selected_brushes.next = &selected_brushes;

	Sys_UpdateWindows (W_ALL);
}

/*
============
Select_Move
============
*/
void Select_Move (vec3_t delta)
{
	brush_t	*b;

// actually move the selected brushes
	for (b = selected_brushes.next ; b != &selected_brushes ; b=b->next)
		Brush_Move (b, delta);
//	Sys_UpdateWindows (W_ALL);
}

/*
============
Select_Clone

Creates an exact duplicate of the selection in place, then moves
the selected brushes off of their old positions
============
*/
void Select_Clone (void)
{
	brush_t		*b, *b2, *n, *next, *next2;
	vec3_t		delta;
	entity_t	*e;

	g_qeglobals.d_workcount++;
	g_qeglobals.d_select_mode = sel_brush;

	delta[0] = g_qeglobals.d_gridsize;
	delta[1] = g_qeglobals.d_gridsize;
	delta[2] = 0;

	for (b=selected_brushes.next ; b != &selected_brushes ; b=next)
	{
		next = b->next;
		// if the brush is a world brush, handle simply
		if (b->owner == world_entity)
		{
			n = Brush_Clone (b);
			Brush_AddToList (n, &active_brushes);
			Entity_LinkBrush (world_entity, n);
			Brush_Build( n );
			Brush_Move (b, delta);
			continue;
		}

		e = Entity_Clone (b->owner);
		// clear the target / targetname
		DeleteKey (e, "target");
		DeleteKey (e, "targetname");

		// if the brush is a fixed size entity, create a new entity
		if (b->owner->eclass->fixedsize)
		{
			n = Brush_Clone (b);
			Brush_AddToList (n, &active_brushes);
			Entity_LinkBrush (e, n);
			Brush_Build( n );
			Brush_Move (b, delta);
			continue;
		}

		// brush is a complex entity, grab all the other ones now

		next = &selected_brushes;

		for ( b2 = b ; b2 != &selected_brushes ; b2=next2)
		{
			next2 = b2->next;
			if (b2->owner != b->owner)
			{
				if (next == &selected_brushes)
					next = b2;
				continue;
			}

			// move b2 to the start of selected_brushes,
			// so it won't be hit again
			Brush_RemoveFromList (b2);
			Brush_AddToList (b2, &selected_brushes);

			n = Brush_Clone (b2);
			Brush_AddToList (n, &active_brushes);
			Entity_LinkBrush (e, n);
			Brush_Build( n );
			Brush_Move (b2, delta);
		}

	}
	Sys_UpdateWindows (W_ALL);
}



/*
============
Select_SetTexture
============
*/
void Select_SetTexture (texdef_t *texdef)
{
	brush_t	*b;

	if (selected_face)
	{
		selected_face->texdef = *texdef;
		Brush_Build(selected_face_brush);
	}
	else
	{
		for (b=selected_brushes.next ; b != &selected_brushes ; b=b->next)
			if (!b->owner->eclass->fixedsize)
				Brush_SetTexture (b, texdef);
	}
	Sys_UpdateWindows (W_ALL);
}


/*
================================================================

  TRANSFORMATIONS

================================================================
*/

void Select_GetBounds (vec3_t mins, vec3_t maxs)
{
	brush_t	*b;
	int		i;

	for (i=0 ; i<3 ; i++)
	{
		mins[i] = 99999;
		maxs[i] = -99999;
	}

	for (b=selected_brushes.next ; b != &selected_brushes ; b=b->next)
		for (i=0 ; i<3 ; i++)
		{
			if (b->mins[i] < mins[i])
				mins[i] = b->mins[i];
			if (b->maxs[i] > maxs[i])
				maxs[i] = b->maxs[i];
		}
}

void Select_GetMid (vec3_t mid)
{
	vec3_t	mins, maxs;
	int		i;

	Select_GetBounds (mins, maxs);
	for (i=0 ; i<3 ; i++)
		mid[i] = g_qeglobals.d_gridsize*floor ( ( (mins[i] + maxs[i])*0.5 )/g_qeglobals.d_gridsize );
}

vec3_t	select_origin;
vec3_t	select_matrix[3];
qboolean	select_fliporder;

void Select_AplyMatrix (void)
{
	brush_t	*b;
	face_t	*f;
	int		i, j;
	vec3_t	temp;

	for (b=selected_brushes.next ; b != &selected_brushes ; b=b->next)
	{
		for (f=b->brush_faces ; f ; f=f->next)
		{
			for (i=0 ; i<3 ; i++)
			{
				VectorSubtract (f->planepts[i], select_origin, temp);
				for (j=0 ; j<3 ; j++)
					f->planepts[i][j] = DotProduct(temp, select_matrix[j])
						+ select_origin[j];
			}
			if (select_fliporder)
			{
				VectorCopy (f->planepts[0], temp);
				VectorCopy (f->planepts[2], f->planepts[0]);
				VectorCopy (temp, f->planepts[2]);
			}
		}
		Brush_Build( b );
	}
	Sys_UpdateWindows (W_ALL);
}


void Select_FlipAxis (int axis)
{
	int		i;

	Select_GetMid (select_origin);
	for (i=0 ; i<3 ; i++)
	{
		VectorCopy (vec3_origin, select_matrix[i]);
		select_matrix[i][i] = 1;
	}
	select_matrix[axis][axis] = -1;

	select_fliporder = true;
	Select_AplyMatrix ();
}

void Select_RotateAxis (int axis, float deg)
{
	vec3_t	temp;
	int		i, j;
	vec_t	c, s;

	if (deg == 0)
		return;

	Select_GetMid (select_origin);
	select_fliporder = false;

	if (deg == 90)
	{
		for (i=0 ; i<3 ; i++)
		{
			VectorCopy (vec3_origin, select_matrix[i]);
			select_matrix[i][i] = 1;
		}
		i = (axis+1)%3;
		j = (axis+2)%3;
		VectorCopy (select_matrix[i], temp);
		VectorCopy (select_matrix[j], select_matrix[i]);
		VectorSubtract (vec3_origin, temp, select_matrix[j]);
	}
	else
	{
		deg = -deg;
		if (deg == -180)
		{
			c = -1;
			s = 0;
		}
		else if (deg == -270)
		{
			c = 0;
			s = -1;
		}
		else
		{
			c = cos(deg/180*3.14159);
			s = sin (deg/180*3.14159);
		}

		for (i=0 ; i<3 ; i++)
		{
			VectorCopy (vec3_origin, select_matrix[i]);
			select_matrix[i][i] = 1;
		}

		switch (axis)
		{
		case 0:
			select_matrix[1][1] = c;
			select_matrix[1][2] = -s;
			select_matrix[2][1] = s;
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
			select_matrix[0][1] = -s;
			select_matrix[1][0] = s;
			select_matrix[1][1] = c;
			break;
		}
	}

	Select_AplyMatrix ();
}

/*
================================================================

GROUP SELECTIONS

================================================================
*/

void Select_CompleteTall (void)
{
	brush_t	*b, *next;
	int		i;
	vec3_t	mins, maxs;

	if (!QE_SingleBrush ())
		return;

	g_qeglobals.d_select_mode = sel_brush;

	VectorCopy (selected_brushes.next->mins, mins);
	VectorCopy (selected_brushes.next->maxs, maxs);
	Select_Delete ();

	for (b=active_brushes.next ; b != &active_brushes ; b=next)
	{
		next = b->next;
		for (i=0 ; i<2 ; i++)
			if (b->maxs[i] > maxs[i] || b->mins[i] < mins[i])
				break;
		if (i == 2)
		{
			Brush_RemoveFromList (b);
			Brush_AddToList (b, &selected_brushes);
		}
	}
	Sys_UpdateWindows (W_ALL);
}

void Select_PartialTall (void)
{
	brush_t	*b, *next;
	int		i;
	vec3_t	mins, maxs;

	if (!QE_SingleBrush ())
		return;

	g_qeglobals.d_select_mode = sel_brush;

	VectorCopy (selected_brushes.next->mins, mins);
	VectorCopy (selected_brushes.next->maxs, maxs);
	Select_Delete ();

	for (b=active_brushes.next ; b != &active_brushes ; b=next)
	{
		next = b->next;
		for (i=0 ; i<2 ; i++)
			if (b->mins[i] > maxs[i] || b->maxs[i] < mins[i])
				break;
		if (i == 2)
		{
			Brush_RemoveFromList (b);
			Brush_AddToList (b, &selected_brushes);
		}
	}
	Sys_UpdateWindows (W_ALL);
}

void Select_Touching (void)
{
	brush_t	*b, *next;
	int		i;
	vec3_t	mins, maxs;

	if (!QE_SingleBrush ())
		return;

	g_qeglobals.d_select_mode = sel_brush;

	VectorCopy (selected_brushes.next->mins, mins);
	VectorCopy (selected_brushes.next->maxs, maxs);

	for (b=active_brushes.next ; b != &active_brushes ; b=next)
	{
		next = b->next;
		for (i=0 ; i<3 ; i++)
			if (b->mins[i] > maxs[i]+1 || b->maxs[i] < mins[i]-1)
				break;
		if (i == 3)
		{
			Brush_RemoveFromList (b);
			Brush_AddToList (b, &selected_brushes);
		}
	}
	Sys_UpdateWindows (W_ALL);
}

void Select_Inside (void)
{
	brush_t	*b, *next;
	int		i;
	vec3_t	mins, maxs;

	if (!QE_SingleBrush ())
		return;

	g_qeglobals.d_select_mode = sel_brush;

	VectorCopy (selected_brushes.next->mins, mins);
	VectorCopy (selected_brushes.next->maxs, maxs);
	Select_Delete ();

	for (b=active_brushes.next ; b != &active_brushes ; b=next)
	{
		next = b->next;
		for (i=0 ; i<3 ; i++)
			if (b->maxs[i] > maxs[i] || b->mins[i] < mins[i])
				break;
		if (i == 3)
		{
			Brush_RemoveFromList (b);
			Brush_AddToList (b, &selected_brushes);
		}
	}
	Sys_UpdateWindows (W_ALL);
}

/*
=============
Select_Ungroup

Turn the currently selected entity back into normal brushes
=============
*/
void  Select_Ungroup (void)
{
	entity_t	*e;
	brush_t		*b;

	e = selected_brushes.next->owner;

	if (!e || e == world_entity || e->eclass->fixedsize)
	{
		Sys_Status ("Not a grouped entity.", 0);
		return;
	}

	for (b=e->brushes.onext ; b != &e->brushes ; b=e->brushes.onext)
	{
		Brush_RemoveFromList (b);
		Brush_AddToList (b, &active_brushes);
		Entity_UnlinkBrush (b);
		Entity_LinkBrush (world_entity, b);
		Brush_Build( b );
		b->owner = world_entity;
	}

	Entity_Free (e);
	Sys_UpdateWindows (W_ALL);
}

/*
====================
Select_MakeStructural
====================
*/
void Select_MakeStructural (void)
{
	brush_t	*b;
	face_t	*f;

	for (b=selected_brushes.next ; b != &selected_brushes ; b=b->next)
		for (f=b->brush_faces ; f ; f=f->next)
			f->texdef.contents &= ~CONTENTS_DETAIL;
	Select_Deselect ();
	Sys_UpdateWindows (W_ALL);
}

void Select_MakeDetail (void)
{
	brush_t	*b;
	face_t	*f;

	for (b=selected_brushes.next ; b != &selected_brushes ; b=b->next)
		for (f=b->brush_faces ; f ; f=f->next)
			f->texdef.contents |= CONTENTS_DETAIL;
	Select_Deselect ();
	Sys_UpdateWindows (W_ALL);
}


