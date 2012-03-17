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

//
// ResourceManager.h
//

#include "h2common.h"
#include <stdlib.h>     // needed here for size_t

typedef struct ResourceManager_s
{
	size_t resSize;
	unsigned int resPerBlock;
	unsigned int nodeSize;
	struct ResMngr_Block_s *blockList;
	char **free;
	char *ResMan_Name;

	unsigned numResourcesAllocated;

} ResourceManager_t;

extern H2COMMON_API void ResMngr_Con( ResourceManager_t *resource, size_t init_resSize, unsigned int init_resPerBlock, char *resman_name );
extern H2COMMON_API void ResMngr_Des( ResourceManager_t *resource );
extern H2COMMON_API void *ResMngr_AllocateResource( ResourceManager_t *resource, size_t size );
extern H2COMMON_API void ResMngr_DeallocateResource( ResourceManager_t *resource, void *toDeallocate, size_t size );
