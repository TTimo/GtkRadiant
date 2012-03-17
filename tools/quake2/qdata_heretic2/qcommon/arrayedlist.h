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

#ifndef _ARRAYEDLIST_H
#define _ARRAYEDLIST_H

#include <assert.h>

typedef struct ArrayedListNode_s
{
	int data;
	int next;
	int inUse;
} ArrayedListNode_t;

#define ARRAYEDLISTNODE_NULL -1

static
#ifdef _WIN32
_inline
#else
inline
#endif
int GetFreeNode( ArrayedListNode_t *nodeArray, int max ){
	int i;

	for ( i = 0; i < max; ++i )
	{
		if ( !nodeArray[i].inUse ) {
			nodeArray[i].inUse = 1;
			return i;
		}
	}

	assert( 0 );
	return -1;
}

static
#ifdef _WIN32
_inline
#else
inline
#endif
void FreeNode( ArrayedListNode_t *nodeArray, int index ){
	nodeArray[index].inUse = 0;
}

#endif //_ARRAYEDLIST_H
