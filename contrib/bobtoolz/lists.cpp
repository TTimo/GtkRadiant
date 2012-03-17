/*
   BobToolz plugin for GtkRadiant
   Copyright (C) 2001 Gordon Biggans

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "StdAfx.h"

#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include "lists.h"
#include "misc.h"

bool LoadExclusionList( char* filename, list<Str>* exclusionList ){
	FILE* eFile = fopen( filename, "r" );
	if ( eFile ) {
		char buffer[256];
		int cnt = 0;
		while ( !feof( eFile ) )
		{
			memset( buffer, 0, 256 );
			fscanf( eFile, "%s\n", buffer );

			if ( strlen( buffer ) > 0 ) {
				exclusionList->push_back( buffer );
			}
			else{
				cnt++;
			}
		}

		fclose( eFile );

		return TRUE;
	}

	Sys_ERROR( "Failed To Load Exclusion List: %s\n", filename );
	return FALSE;
}

bool LoadGList( char* filename, GList** loadlist ){
	FILE* eFile = fopen( filename, "r" );
	if ( eFile ) {
		char buffer[256];
		int cnt = 0;
		while ( !feof( eFile ) )
		{
			memset( buffer, 0, 256 );
			fscanf( eFile, "%s\n", buffer );

			if ( strlen( buffer ) > 0 ) {
				char* buffer2 = new char[strlen( buffer ) + 1];
				strcpy( buffer2, buffer );
				*loadlist = g_list_append( *loadlist, buffer2 );
			}
			else{
				cnt++;
			}
		}

		fclose( eFile );

		return TRUE;
	}

	Sys_ERROR( "Failed To Load GList: %s\n", filename );
	return FALSE;
}
