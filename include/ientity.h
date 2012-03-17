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

#ifndef _IENTITY_H_
#define _IENTITY_H_

//
// API for entity manip stuff
//

// FIXME TTimo this prolly needs to merge with iepairs.h?

/*!
   SPoG
   generic "entity" module...
   at first, there will only be one implementation for all entities,
   perhaps later there will be one for each entity type?
   it would probably make more sense to have a single implementation,
   a generic one that is very flexible and can adapt the visualisation of
   itself depending on an xml config specified in the entity definitions file
 */
#define ENTITY_MAJOR "entity"
// {A1C9F9FD-75D5-4e4d-9D65-235D6D3F254C}
static const GUID QEREntityTable_GUID =
{ 0xa1c9f9fd, 0x75d5, 0x4e4d, { 0x9d, 0x65, 0x23, 0x5d, 0x6d, 0x3f, 0x25, 0x4c } };

typedef entity_t* ( *PFN_ENTITYALLOC )();
typedef void ( *PFN_ENTITYFREE )( entity_t *e );
typedef entity_t* ( *PFN_ENTITYCREATE )( eclass_t *c );
typedef entity_t* ( *PFN_ENTITYCLONE )( entity_t *e );
typedef void ( *PFN_ENTITYSETKEYVALUE )( entity_t *ent, const char *key, const char *value );
typedef void ( *PFN_ENTITYDELETEKEY )( entity_t *ent, const char *key );
typedef const char* ( *PFN_ENTITYVALUEFORKEY )( entity_t *ent, const char *key );
typedef float ( *PFN_ENTITYFLOATFORKEY )( entity_t *ent, const char *key );
typedef int ( *PFN_ENTITYINTFORKEY )( entity_t *ent, const char *key );
typedef void ( *PFN_ENTITYVECTORFORKEY )( entity_t *ent, const char *key, vec3_t vec );
typedef void ( *PFN_ENTITYADDTOLIST )( entity_t *e, entity_t *lst );
typedef void ( *PFN_ENTITYREMOVEFROMLIST )( entity_t *e );
typedef void ( *PFN_ENTITYLINKBRUSH )( entity_t *e, brush_t *b );
typedef void ( *PFN_ENTITYUNLINKBRUSH )( brush_t *b );
typedef void ( *PFN_ENTITYDRAWLIGHT )( entity_t* e, int nGLState, int pref, int nViewType );
typedef int ( *PFN_ENTITYMEMORYSIZE )( entity_t *e );
typedef void ( *PFN_ENTITYUPDATEMODEL )( entity_t *e );
typedef epair_t* ( *PFN_ALLOCATEEPAIR )( const char *key, const char *value );
typedef epair_t** ( *PFN_GETENTITYKEYVALLIST )( entity_t *e );
typedef void ( *PFN_SETENTITYKEYVALLIST )( entity_t *e, epair_t* ep );


struct _QEREntityTable
{
	int m_nSize;
	PFN_ENTITYALLOC m_pfnEntity_Alloc;
	PFN_ENTITYFREE m_pfnEntity_Free;
	PFN_ENTITYCREATE m_pfnEntity_Create;
	PFN_ENTITYCLONE m_pfnEntity_Clone;
	PFN_ENTITYSETKEYVALUE m_pfnSetKeyValue;
	PFN_ENTITYDELETEKEY m_pfnDeleteKey;
	PFN_ENTITYVALUEFORKEY m_pfnValueForKey;
	PFN_ENTITYFLOATFORKEY m_pfnFloatForKey;
	PFN_ENTITYINTFORKEY m_pfnIntForKey;
	PFN_ENTITYVECTORFORKEY m_pfnGetVectorForKey;
	PFN_ENTITYADDTOLIST m_pfnEntity_AddToList;
	PFN_ENTITYREMOVEFROMLIST m_pfnEntity_RemoveFromList;
	PFN_ENTITYLINKBRUSH m_pfnEntity_LinkBrush;
	PFN_ENTITYUNLINKBRUSH m_pfnEntity_UnlinkBrush;
	PFN_ENTITYDRAWLIGHT m_pfnDrawLight;
	PFN_ENTITYMEMORYSIZE m_pfnEntity_MemorySize;
	PFN_ALLOCATEEPAIR m_pfnAllocateEpair;
	PFN_GETENTITYKEYVALLIST m_pfnGetEntityKeyValList;
	PFN_SETENTITYKEYVALLIST m_pfnSetEntityKeyValList;
};

#ifdef USE_ENTITYTABLE_DEFINE
#ifndef __ENTITYTABLENAME
#define __ENTITYTABLENAME g_EntityTable
#endif
#define Entity_Alloc __ENTITYTABLENAME.m_pfnEntity_Alloc
#define Entity_Free __ENTITYTABLENAME.m_pfnEntity_Free
#define Entity_Clone __ENTITYTABLENAME.m_pfnEntity_Clone
#define SetKeyValue __ENTITYTABLENAME.m_pfnSetKeyValue
#define DeleteKey __ENTITYTABLENAME.m_pfnDeleteKey
#define ValueForKey __ENTITYTABLENAME.m_pfnValueForKey
#define FloatForKey __ENTITYTABLENAME.m_pfnFloatForKey
#define IntForKey __ENTITYTABLENAME.m_pfnIntForKey
#define GetVectorForKey __ENTITYTABLENAME.m_pfnGetVectorForKey
#define Entity_AddToList __ENTITYTABLENAME.m_pfnEntity_AddToList
#define Entity_RemoveFromList __ENTITYTABLENAME.m_pfnEntity_RemoveFromList
#define Entity_LinkBrush __ENTITYTABLENAME.m_pfnEntity_LinkBrush
#define Entity_UnlinkBrush __ENTITYTABLENAME.m_pfnEntity_UnlinkBrush
#define DrawLight __ENTITYTABLENAME.m_pfnDrawLight
#define Entity_MemorySize __ENTITYTABLENAME.m_pfnEntity_MemorySize
#define Entity_AllocateEpair __ENTITYTABLENAME.m_pfnAllocateEpair
#define Entity_GetKeyValList __ENTITYTABLENAME.m_pfnGetEntityKeyValList
#define Entity_SetKeyValList __ENTITYTABLENAME.m_pfnSetEntityKeyValList
#endif

#endif
