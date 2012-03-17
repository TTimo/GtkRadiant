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

#include "stdafx.h"

/*!
   connects two entities creating a unique target/targetname value
 */
void Entity_Connect( entity_t *e1, entity_t *e2 ){
	const char *maptarget;
	char newtarget[16];
	int maxtarget = 0; // highest t# value in the map
	entity_t *e;    // map entities

	if ( e1 == e2 ) {
#ifdef _DEBUG
		Sys_Status( "Entity_Connect: Brushes are from same entity.", 0 );
#endif
		return;
	}

	for ( e = entities.next ; e != &entities ; e = e->next )
	{
		maptarget = ValueForKey( e, "target" );
		if ( maptarget && maptarget[0] ) {
			int targetnum = atoi( maptarget + 1 );
			if ( targetnum > maxtarget ) {
				maxtarget = targetnum;
			}
		}
	}
	sprintf( newtarget, "t%i", maxtarget + 1 );

#ifdef _DEBUG
	Sys_Printf( "Connecting entities with new target/targetname: %s\n", newtarget );
#endif

	SetKeyValue( e1, "target", newtarget );
	SetKeyValue( e2, "targetname", newtarget );
}

int GetUniqueTargetId( int iHint ){
	int iMin, iMax, i;
	bool fFound;
	entity_t *pe;

	fFound = FALSE;
	pe = entities.next;
	iMin = 0;
	iMax = 0;

	for (; pe != NULL && pe != &entities ; pe = pe->next )
	{
		i = IntForKey( pe, "target" );
		if ( i ) {
			iMin = MIN( i, iMin );
			iMax = MAX( i, iMax );
			if ( i == iHint ) {
				fFound = TRUE;
			}
		}
	}

	if ( fFound ) {
		return iMax + 1;
	}
	else{
		return iHint;
	}
}
