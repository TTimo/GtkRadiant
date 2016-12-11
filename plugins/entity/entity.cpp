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

#include "plugin.h"
#include "entity.h"
#include "entity_entitymodel.h"
#include "light.h"

int g_entityId = 1;



// internal

static void Entity_FreeEpairs( entity_t *e );

static void SetKeyValue( epair_t *&e, const char *key, const char *value );
static void  DeleteKey( epair_t *&e, const char *key );
static const char *ValueForKey( epair_t *&e, const char *key );

static void Entity_OnKeyValueChanged( entity_t *e, const char* key, const char* value );

// constructor
entity_t *Entity_Alloc(){
	entity_t *e;
	e = (entity_t*)malloc( sizeof( *e ) );
	e->entityId = g_entityId++;
	VectorSet( e->origin, 0, 0, 0 );
	VectorSet( e->color, 1, 1, 1 );
	e->redoId = 0;
	e->undoId = 0;
	e->next = e->prev = NULL;
	e->brushes.onext = e->brushes.oprev = &e->brushes;
	e->epairs = NULL;
	e->eclass = NULL;
	e->model.pRender = NULL;
	e->model.pSelect = NULL;
	e->model.pEdit = NULL;
	return e;
}

// destructor
void Entity_Free( entity_t *e ){
	while ( e->brushes.onext != &e->brushes )
		Brush_Free( e->brushes.onext, true );

	if ( e->next ) {
		e->next->prev = e->prev;
		e->prev->next = e->next;
	}

	Entity_FreeEpairs( e );

	if ( e->model.pRender ) {
		e->model.pRender->DecRef();
		e->model.pRender = NULL;
	}
	if ( e->model.pSelect ) {
		e->model.pSelect->DecRef();
		e->model.pSelect = NULL;
	}
	if ( e->model.pEdit ) {
		e->model.pEdit->DecRef();
		e->model.pEdit = NULL;
	}

	free( e );
}

// construct from entity
entity_t    *Entity_Clone( entity_t *e ){
	entity_t    *n;
	epair_t     *ep;

	n = Entity_Alloc();
	n->eclass = e->eclass;

	for ( ep = e->epairs ; ep ; ep = ep->next ){
		if ( !ep->key || !ep->key[0] ) {
			Sys_FPrintf( SYS_ERR, "ERROR: Entity_Clone: NULL or zero-length key\n" );
			return 0;
		}

		SetKeyValue( n->epairs, ep->key, ep->value );
	}

	for ( ep = n->epairs ; ep ; ep = ep->next ){
		/*!
		   \todo TODO broadcast this through a clean messaging API ;-)
		 */
		Entity_OnKeyValueChanged( n, ep->key, ep->value );
	}

	// copy some misc stuff as well
	VectorCopy( e->origin, n->origin );
//	VectorCopy( e->vRotation, n->vRotation );
//	VectorCopy( e->vScale, n->vScale );

//  n->bDirty = true;

	return n;
}





const char *ValueForKey( epair_t *&e, const char *key ){
	epair_t *ep;
	for ( ep = e ; ep ; ep = ep->next )
	{
		if ( !strcmp( ep->key, key ) ) {
			return ep->value;
		}
	}
	return "";
}

const char *ValueForKey( entity_t *ent, const char *key ){
	return ValueForKey( ent->epairs, key );
}

void    SetKeyValue( epair_t *&e, const char *key, const char *value ){
	epair_t *ep;
	for ( ep = e ; ep ; ep = ep->next )
	{
		if ( !strcmp( ep->key, key ) ) {
			free( ep->value );
			ep->value = (char*)malloc( strlen( value ) + 1 );
			strcpy( ep->value, value );
			return;
		}
	}
	ep = (epair_t*)malloc( sizeof( *ep ) );
	ep->next = e;
	e = ep;
	ep->key = (char*)malloc( strlen( key ) + 1 );
	strcpy( ep->key, key );
	ep->value = (char*)malloc( strlen( value ) + 1 );
	strcpy( ep->value, value );

}

void SetKeyValue( entity_t *ent, const char *key, const char *value ){
	if ( ent == NULL ) {
		Sys_FPrintf( SYS_ERR, "ERROR: SetKeyValue: NULL entity \n" );
		return;
	}

	if ( !key || !key[0] ) {
		Sys_FPrintf( SYS_ERR, "ERROR: SetKeyValue: NULL or zero-length key\n" );
		return;
	}

	SetKeyValue( ent->epairs, key, value );
	/*!
	   \todo TODO broadcast this through a clean messaging API ;-)
	 */
	Entity_OnKeyValueChanged( ent, key, value );
}

void    DeleteKey( epair_t *&e, const char *key ){
	epair_t **ep, *next;

	ep = &e;
	while ( *ep )
	{
		next = *ep;
		if ( !strcmp( next->key, key ) ) {
			*ep = next->next;
			free( next->key );
			free( next->value );
			free( next );
			return;
		}
		ep = &next->next;
	}
}

void    DeleteKey( entity_t *ent, const char *key ){
	DeleteKey( ent->epairs, key );
	Entity_OnKeyValueChanged( ent, key, "" );
}

float   FloatForKey( entity_t *ent, const char *key ){
	const char  *k;

	k = ValueForKey( ent, key );
	return (float) atof( k );
}

int IntForKey( entity_t *ent, const char *key ){
	const char  *k;

	k = ValueForKey( ent, key );
	return atoi( k );
}

void    GetVectorForKey( entity_t *ent, const char *key, vec3_t vec ){
	const char  *k;

	k = ValueForKey( ent, key );
	sscanf( k, "%f %f %f", &vec[0], &vec[1], &vec[2] );
}

/*
   ===============
   Entity_FreeEpairs

   Frees the entity epairs.
   ===============
 */
void Entity_FreeEpairs( entity_t *e ){
	epair_t *ep, *next;

	for ( ep = e->epairs; ep; ep = next )
	{
		next = ep->next;
		free( ep->key );
		free( ep->value );
		free( ep );
	}
	e->epairs = NULL;
}

void Entity_AddToList( entity_t *e, entity_t *elist ){
	if ( e->next || e->prev ) {
		Error( "Entity_AddToList: already linked" );
	}
	//e->next = elist->next;
	//elist->next->prev = e;
	//elist->next = e;
	//e->prev = elist;
	e->next = elist;
	e->prev = elist->prev;
	elist->prev->next = e;
	elist->prev = e;
}

void Entity_RemoveFromList( entity_t *e ){
	if ( !e->next || !e->prev ) {
		Error( "Entity_RemoveFromList: not linked" );
	}
	e->next->prev = e->prev;
	e->prev->next = e->next;
	e->next = e->prev = NULL;
}

void Entity_LinkBrush( entity_t *e, brush_t *b ){
	if ( b->oprev || b->onext ) {
		Error( "Entity_LinkBrush: Already linked" );
	}
	b->owner = e;

//	b->onext = e->brushes.onext;
//	b->oprev = &e->brushes;
//	e->brushes.onext->oprev = b;
//	e->brushes.onext = b;
	/*
	SPoG - changed to add brushes to end of list instead of start - so this can be used by map loader.
	This could concievably cause a problem if someone is traversing e->brushes while calling this function.
	So don't.
	 */
	b->onext = &e->brushes;
	b->oprev = e->brushes.oprev;
	e->brushes.oprev->onext = b;
	e->brushes.oprev = b;
}

void Entity_UnlinkBrush( brush_t *b ){
	if ( !b->onext || !b->oprev ) {
		Error( "Entity_UnlinkBrush: Not currently linked" );
	}
	b->onext->oprev = b->oprev;
	b->oprev->onext = b->onext;
	b->onext = b->oprev = NULL;
	b->owner = NULL;
}

// for undo
int Entity_MemorySize( entity_t *e ){
	epair_t *ep;
	int size = 0;

	for ( ep = e->epairs; ep; ep = ep->next )
	{
		size += strlen( ep->key );
		size += strlen( ep->value );
		size += sizeof( epair_t );
	}
	size += sizeof( entity_t );
	return size;
}

epair_t* Entity_AllocateEpair( const char *key, const char *value ){
	epair_t *ep = (epair_t*)malloc( sizeof( *ep ) );
	ep->key = (char*)malloc( strlen( key ) + 1 );
	strcpy( ep->key, key );
	ep->value = (char*)malloc( strlen( value ) + 1 );
	strcpy( ep->value, value );
	ep->next = NULL;
	return ep;
}

epair_t** Entity_GetKeyValList( entity_t *e ){
	return &e->epairs;
}

void Entity_SetKeyValList( entity_t *e, epair_t* ep ){
	if ( e->epairs ) {
		Sys_FPrintf( SYS_WRN, "Warning : pe->epairs != NULL in Entity_SetKeyValList, will not set\n" );
	}
	else {
		e->epairs = ep;

		for ( epair_t *pe_ep = e->epairs; pe_ep; pe_ep = pe_ep->next )
			Entity_OnKeyValueChanged( e, pe_ep->key, pe_ep->value );
	}
}


/*!
   \todo FIXME TTimo
   this is meant to raise messages instead of calling the IEdit directly
 */
static void Entity_OnKeyValueChanged( entity_t *e, const char *key, const char* value ){
	if ( strcmp( key,"classname" ) == 0 ) {
		e->eclass = Eclass_ForName( value, false );
		Entity_UpdateClass( e, value );
		if ( strcmp( value,"light" ) == 0 ) {
			for ( epair_t* ep = e->epairs; ep != NULL; ep = ep->next )
				Light_OnKeyValueChanged( e, ep->key, ep->value );
		}
		if ( e->model.pEdit ) {
			for ( epair_t* ep = e->epairs; ep != NULL; ep = ep->next )
				e->model.pEdit->OnKeyValueChanged( e, ep->key, ep->value );
		}
	}
	else if ( Entity_IsLight( e ) ) {
		Light_OnKeyValueChanged( e, key, value );
	}
	else if ( e->model.pEdit ) {
		e->model.pEdit->OnKeyValueChanged( e, key, value );
	}

	// update brush mins/maxs for legacy culling system
	if ( e->model.pRender && e->brushes.onext != &e->brushes ) {
		Brush_Build( e->brushes.onext, true, true, false, true );
	}
}
