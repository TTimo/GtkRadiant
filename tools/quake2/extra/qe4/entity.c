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

#include "qe3.h"

char 	*ValueForKey (entity_t *ent, char *key)
{
	epair_t	*ep;

	for (ep=ent->epairs ; ep ; ep=ep->next)
		if (!strcmp (ep->key, key) )
			return ep->value;
	return "";
}

void 	SetKeyValue (entity_t *ent, char *key, char *value)
{
	epair_t	*ep;

	if (ent == NULL)
		return;

	if (!key || !key[0])
		return;

	for (ep=ent->epairs ; ep ; ep=ep->next)
		if (!strcmp (ep->key, key) )
		{
			free (ep->value);
			ep->value = qmalloc(strlen(value)+1);
			strcpy (ep->value, value);
			return;
		}
	ep = qmalloc (sizeof(*ep));
	ep->next = ent->epairs;
	ent->epairs = ep;
	ep->key = qmalloc(strlen(key)+1);
	strcpy (ep->key, key);
	ep->value = qmalloc(strlen(value)+1);
	strcpy (ep->value, value);
}

void 	DeleteKey (entity_t *ent, char *key)
{
	epair_t	**ep, *next;

	ep = &ent->epairs;
	while (*ep)
	{
		next = *ep;
		if ( !strcmp (next->key, key) )
		{
			*ep = next->next;
			free(next->key);
			free(next->value);
			free(next);
			return;
		}
		ep = &next->next;
	}
}

float	FloatForKey (entity_t *ent, char *key)
{
	char	*k;

	k = ValueForKey (ent, key);
	return atof(k);
}

int IntForKey (entity_t *ent, char *key)
{
	char	*k;

	k = ValueForKey (ent, key);
	return atoi(k);
}

void 	GetVectorForKey (entity_t *ent, char *key, vec3_t vec)
{
	char	*k;

	k = ValueForKey (ent, key);
	sscanf (k, "%f %f %f", &vec[0], &vec[1], &vec[2]);
}


/*
===============
Entity_Free

Frees the entity and any brushes is has.
The entity is removed from the global entities list.
===============
*/
void Entity_Free (entity_t *e)
{
	epair_t	*ep, *next;

	while (e->brushes.onext != &e->brushes)
		Brush_Free (e->brushes.onext);

	if (e->next)
	{
		e->next->prev = e->prev;
		e->prev->next = e->next;
	}

	for (ep = e->epairs ; ep ; ep=next)
	{
		next = ep->next;
		free (ep);
	}
	free (e);
}

/*
=================
ParseEpair
=================
*/
epair_t *ParseEpair (void)
{
	epair_t	*e;

	e = qmalloc (sizeof(*e));

	e->key = qmalloc(strlen(token)+1);
	strcpy (e->key, token);

	GetToken (false);
	e->value = qmalloc(strlen(token)+1);
	strcpy (e->value, token);

	return e;
}

/*
================
Entity_Parse

If onlypairs is set, the classname info will not
be looked up, and the entity will not be added
to the global list.  Used for parsing the project.
================
*/
entity_t	*Entity_Parse (qboolean onlypairs)
{
	entity_t	*ent;
	eclass_t	*e;
	brush_t		*b;
	vec3_t		mins, maxs;
	epair_t		*ep;
	qboolean	has_brushes;

	if (!GetToken (true))
		return NULL;

	if (strcmp (token, "{") )
		Error ("ParseEntity: { not found");

	ent = qmalloc (sizeof(*ent));
	ent->brushes.onext = ent->brushes.oprev = &ent->brushes;

	do
	{
		if (!GetToken (true))
			Error ("ParseEntity: EOF without closing brace");
		if (!strcmp (token, "}") )
			break;
		if (!strcmp (token, "{") )
		{
			b = Brush_Parse ();
			b->owner = ent;

			// add to the end of the entity chain
			b->onext = &ent->brushes;
			b->oprev = ent->brushes.oprev;
			ent->brushes.oprev->onext = b;
			ent->brushes.oprev = b;
		}
		else
		{
			ep = ParseEpair ();
			ep->next = ent->epairs;
			ent->epairs = ep;
		}
	} while (1);

	if (onlypairs)
		return ent;

	if (ent->brushes.onext == &ent->brushes)
		has_brushes = false;
	else
		has_brushes = true;

	GetVectorForKey (ent, "origin", ent->origin);

	e = Eclass_ForName (ValueForKey (ent, "classname"), has_brushes);
	ent->eclass = e;
	if (e->fixedsize)
	{	// fixed size entity
		if (ent->brushes.onext != &ent->brushes)
		{
			printf ("Warning: Fixed size entity with brushes\n");
#if 0
			while (ent->brushes.onext != &ent->brushes)
			{	// FIXME: this will free the entity and crash!
				Brush_Free (b);
			}
#endif
ent->brushes.next = ent->brushes.prev = &ent->brushes;
		}
		// create a custom brush
		VectorAdd (e->mins, ent->origin, mins);
		VectorAdd (e->maxs, ent->origin, maxs);
		b = Brush_Create (mins, maxs, &e->texdef);
		b->owner = ent;

		b->onext = ent->brushes.onext;
		b->oprev = &ent->brushes;
		ent->brushes.onext->oprev = b;
		ent->brushes.onext = b;
	}
	else
	{	// brush entity
		if (ent->brushes.next == &ent->brushes)
			printf ("Warning: Brush entity with no brushes\n");
	}

	// add all the brushes to the main list
	for (b=ent->brushes.onext ; b != &ent->brushes ; b=b->onext)
	{
		b->next = active_brushes.next;
		active_brushes.next->prev = b;
		b->prev = &active_brushes;
		active_brushes.next = b;
	}

	return ent;
}

/*
============
Entity_Write
============
*/
void Entity_Write (entity_t *e, FILE *f, qboolean use_region)
{
	epair_t		*ep;
	brush_t		*b;
	vec3_t		origin;
	char		text[128];
	int			count;

	// if none of the entities brushes are in the region,
	// don't write the entity at all
	if (use_region)
	{
		// in region mode, save the camera position as playerstart
		if ( !strcmp(ValueForKey (e, "classname"), "info_player_start") )
		{
			fprintf (f, "{\n");
			fprintf (f, "\"classname\" \"info_player_start\"\n");
			fprintf (f, "\"origin\" \"%i %i %i\"\n", (int)camera.origin[0],
				(int)camera.origin[1], (int)camera.origin[2]);
			fprintf (f, "\"angle\" \"%i\"\n", (int)camera.angles[YAW]);
			fprintf (f, "}\n");
			return;
		}

		for (b=e->brushes.onext ; b != &e->brushes ; b=b->onext)
			if (!Map_IsBrushFiltered(b))
				break;	// got one

		if (b == &e->brushes)
			return;		// nothing visible
	}

	// if fixedsize, calculate a new origin based on the current
	// brush position
	if (e->eclass->fixedsize)
	{
		VectorSubtract (e->brushes.onext->mins, e->eclass->mins, origin);
		sprintf (text, "%i %i %i", (int)origin[0],
			(int)origin[1], (int)origin[2]);
		SetKeyValue (e, "origin", text);
	}

	fprintf (f, "{\n");
	for (ep = e->epairs ; ep ; ep=ep->next)
		fprintf (f, "\"%s\" \"%s\"\n", ep->key, ep->value);

	if (!e->eclass->fixedsize)
	{
		count = 0;
		for (b=e->brushes.onext ; b != &e->brushes ; b=b->onext)
		{
			if (!use_region || !Map_IsBrushFiltered (b))
			{
				fprintf (f, "// brush %i\n", count);
				count++;
				Brush_Write (b, f);
			}
		}
	}
	fprintf (f, "}\n");
}



/*
============
Entity_Create

Creates a new entity out of the selected_brushes list.
If the entity class is fixed size, the brushes are only
used to find a midpoint.  Otherwise, the brushes have
their ownershi[ transfered to the new entity.
============
*/
entity_t	*Entity_Create (eclass_t *c)
{
	entity_t	*e;
	brush_t		*b;
	vec3_t		mins, maxs;
	int			i;

	// check to make sure the brushes are ok

	for (b=selected_brushes.next ; b != &selected_brushes ; b=b->next)
		if (b->owner != world_entity)
		{
			Sys_Printf ("Entity NOT created, brushes not all from world\n");
			Sys_Beep ();
			return NULL;
		}

	// create it

	e = qmalloc(sizeof(*e));
	e->brushes.onext = e->brushes.oprev = &e->brushes;
	e->eclass = c;
	SetKeyValue (e, "classname", c->name);

	// add the entity to the entity list
	e->next = entities.next;
	entities.next = e;
	e->next->prev = e;
	e->prev = &entities;

	if (c->fixedsize)
	{
		//
		// just use the selection for positioning
		//
		b = selected_brushes.next;
		for (i=0 ; i<3 ; i++)
			e->origin[i] = b->mins[i] - c->mins[i];

		// create a custom brush
		VectorAdd (c->mins, e->origin, mins);
		VectorAdd (c->maxs, e->origin, maxs);
		b = Brush_Create (mins, maxs, &c->texdef);

		Entity_LinkBrush (e, b);

		// delete the current selection
		Select_Delete ();

		// select the new brush
		b->next = b->prev = &selected_brushes;
		selected_brushes.next = selected_brushes.prev = b;

		Brush_Build( b );
	}
	else
	{
		//
		// change the selected brushes over to the new entity
		//
		for (b=selected_brushes.next ; b != &selected_brushes ; b=b->next)
		{
			Entity_UnlinkBrush (b);
			Entity_LinkBrush (e, b);
			Brush_Build( b );	// so the key brush gets a name
		}
	}

	Sys_UpdateWindows (W_ALL);
	return e;
}


/*
===========
Entity_LinkBrush
===========
*/
void Entity_LinkBrush (entity_t *e, brush_t *b)
{
	if (b->oprev || b->onext)
		Error ("Entity_LinkBrush: Allready linked");
	b->owner = e;

	b->onext = e->brushes.onext;
	b->oprev = &e->brushes;
	e->brushes.onext->oprev = b;
	e->brushes.onext = b;
}

/*
===========
Entity_UnlinkBrush
===========
*/
void Entity_UnlinkBrush (brush_t *b)
{
	if (!b->owner || !b->onext || !b->oprev)
		Error ("Entity_UnlinkBrush: Not currently linked");
	b->onext->oprev = b->oprev;
	b->oprev->onext = b->onext;
	b->onext = b->oprev = NULL;
	b->owner = NULL;
}



/*
===========
Entity_Clone
===========
*/
entity_t	*Entity_Clone (entity_t *e)
{
	entity_t	*n;
	epair_t		*ep, *np;

	n = qmalloc(sizeof(*n));
	n->brushes.onext = n->brushes.oprev = &n->brushes;
	n->eclass = e->eclass;

	// add the entity to the entity list
	n->next = entities.next;
	entities.next = n;
	n->next->prev = n;
	n->prev = &entities;

	for (ep = e->epairs ; ep ; ep=ep->next)
	{
		np = qmalloc(sizeof(*np));
		np->key = copystring(ep->key);
		np->value = copystring(ep->value);
		np->next = n->epairs;
		n->epairs = np;
	}
	return n;
}

int GetUniqueTargetId(int iHint)
{
	int iMin, iMax, i;
	BOOL fFound;
	entity_t *pe;

	fFound = FALSE;
	pe = entities.next;
	iMin = 0;
	iMax = 0;

	for (; pe != NULL && pe != &entities ; pe = pe->next)
	{
		i = IntForKey(pe, "target");
		if (i)
		{
			iMin = min(i, iMin);
			iMax = max(i, iMax);
			if (i == iHint)
				fFound = TRUE;
		}
	}

	if (fFound)
		return iMax + 1;
	else
		return iHint;
}

entity_t *FindEntity(char *pszKey, char *pszValue)
{
	entity_t *pe;

	pe = entities.next;

	for (; pe != NULL && pe != &entities ; pe = pe->next)
	{
		if (!strcmp(ValueForKey(pe, pszKey), pszValue))
			return pe;
	}

	return NULL;
}

entity_t *FindEntityInt(char *pszKey, int iValue)
{
	entity_t *pe;

	pe = entities.next;

	for (; pe != NULL && pe != &entities ; pe = pe->next)
	{
		if (IntForKey(pe, pszKey) == iValue)
			return pe;
	}

	return NULL;
}

