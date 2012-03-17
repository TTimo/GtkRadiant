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

// entity.h

// external API

// construct
entity_t* Entity_Alloc();
// destruct
void Entity_Free( entity_t *e );
// construct from entity
entity_t* Entity_Clone( entity_t *e );

// epair interface
void  SetKeyValue( entity_t *ent, const char *key, const char *value );
void  DeleteKey( entity_t *ent, const char *key );
const char* ValueForKey( entity_t *ent, const char *key );
float FloatForKey( entity_t *ent, const char *key );
int   IntForKey( entity_t *ent, const char *key );
void  GetVectorForKey( entity_t *ent, const char *key, vec3_t vec );

void Entity_AddToList( entity_t *e, entity_t *lst );
void Entity_RemoveFromList( entity_t *e );

void Entity_LinkBrush( entity_t *e, brush_t *b );
void Entity_UnlinkBrush( brush_t *b );

// for undo
int Entity_MemorySize( entity_t *e );

epair_t* Entity_AllocateEpair( const char *key, const char *value );
epair_t** Entity_GetKeyValList( entity_t *e );
void Entity_SetKeyValList( entity_t *e, epair_t* ep );
