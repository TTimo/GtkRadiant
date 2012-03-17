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

#include "qfiles.h"
#include "scriplib.h"
//#include <windows.h>

materialtype_t defaultmaterialtypes[] =
{
	{"gravel",  MATERIAL_GRAVEL},
	{"metal",   MATERIAL_METAL},
	{"stone",   MATERIAL_STONE},
	{"wood",    MATERIAL_WOOD},
	{NULL,      0}
};

materialtype_t  *materialtypes;

void QFile_ReadMaterialTypes( char* filename ){
	int i;
	FILE    *f;

	f = fopen( filename, "rb" );
	if ( !f ) {
		materialtypes = defaultmaterialtypes;
		return;
	}
	fclose( f );

	free( materialtypes );
	materialtypes = (materialtype_t*)malloc( 256 * sizeof( materialtype_t ) );

	LoadScriptFile( filename );
	i = 0;

	while ( i < 255 )
	{
		GetScriptToken( true );
		if ( endofscript ) {
			break;
		}
		if ( strcmp( token, "material" ) != 0 ) {
			while ( ScriptTokenAvailable() )
			{
				GetScriptToken( false );
			}
		}
		else
		{
			GetScriptToken( false );
			materialtypes[i].name = (char*)malloc( strlen( token ) + 1 );
			strcpy( materialtypes[i].name, token );
			GetScriptToken( false );
			materialtypes[i].value = atoi( token );
		}
		i++;
	}
	materialtypes[i].name = NULL;
	materialtypes[i].value = 0;
}
