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
// ResourceManager.c
//

#include <stdio.h>
#include "resourcemanager.h"
#include <assert.h>

typedef struct ResMngr_Block_s
{
	char *start;
	unsigned int size;
	struct ResMngr_Block_s *next;
} ResMngr_Block_t;

static void ResMngr_CreateBlock( ResourceManager_t *resource ){
	unsigned int _blockSize;
	char *block;
	char **current;
	ResMngr_Block_t *temp;
	unsigned int i;

	_blockSize = resource->nodeSize * resource->resPerBlock;

	block = malloc( _blockSize );

	assert( block );

	temp = malloc( sizeof( *temp ) );

	temp->start = block;
	temp->size = _blockSize;
	temp->next = resource->blockList;

	resource->blockList = temp;

	resource->free = (char **)( block );

	current = resource->free;

	for ( i = 1; i < resource->resPerBlock; ++i )
	{
		// set current->next to point to next node
		*current = (char *)( current ) + resource->nodeSize;

		// set current node to current->next
		current = (char **)( *current );
	}

	*current = NULL;
}

H2COMMON_API void ResMngr_Con( ResourceManager_t *resource, size_t init_resSize, unsigned int init_resPerBlock, char *resman_name ){
	resource->resSize = init_resSize;

	resource->resPerBlock = init_resPerBlock;

	resource->nodeSize = resource->resSize + sizeof( *resource->free );

	resource->blockList = NULL;

	resource->numResourcesAllocated = 0;

	ResMngr_CreateBlock( resource );
}

H2COMMON_API void ResMngr_Des( ResourceManager_t *resource ){
	ResMngr_Block_t *toDelete;

#if 0
	if ( resource->numResourcesAllocated ) {
		char mess[100];
		sprintf( mess,"Potential memory leak %d bytes unfreed\n",resource->resSize * resource->numResourcesAllocated );
		OutputDebugString( mess );
	}
#endif

	while ( resource->blockList )
	{
		toDelete = resource->blockList;
		resource->blockList = resource->blockList->next;
		free( toDelete->start );
		free( toDelete );
	}
}

H2COMMON_API void *ResMngr_AllocateResource( ResourceManager_t *resource, size_t size ){
	char **toPop;

	assert( size == resource->resSize );

	++resource->numResourcesAllocated;

	assert( resource->free ); // constructor not called; possibly due to a static object
	// containing a static ResourceManagerFastLarge member being
	// constructed before its own static members

	toPop = resource->free;

	// set unallocated to the next node and check for NULL (end of list)
	if ( !( resource->free = (char **)( *resource->free ) ) ) { // if at end create new block
		ResMngr_CreateBlock( resource );
	}

	// set next to NULL
	*toPop = NULL;

	// return the resource for the node
	return (void *)( toPop + 1 );
}

H2COMMON_API void ResMngr_DeallocateResource( ResourceManager_t *resource, void *toDeallocate, size_t size ){
	char **toPush;

	assert( size == resource->resSize );

	--resource->numResourcesAllocated;

	toPush = (char **)( toDeallocate ) - 1;

	assert( resource->free ); // see same assert at top of AllocateResource

	// set toPop->next to current unallocated front
	*toPush = (char *)( resource->free );

	// set unallocated to the node removed from allocated
	resource->free = toPush;
}

// end
