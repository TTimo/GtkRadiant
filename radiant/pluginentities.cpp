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

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
// implementation of IPluginEntities specific interface
//

#ifdef USEPLUGINENTITIES

#include "stdafx.h"
#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
#include <dirent.h>
#endif
//#include "qe3.h"

int QERApp_EClassScanDir( const char *path, void* hPlug ){
	char temp[NAME_MAX];
	char filebase[NAME_MAX];
	char filename[NAME_MAX];
	char          *s;
	eclass_t      *e;
	DIR           *dir;
	struct dirent *dirlist;

	QE_ConvertDOSToUnixName( temp, path );
	strcpy( filebase, path );
	s = filebase + strlen( filebase ) - 1;
	while ( *s != '\\' && *s != '/' && s != filebase )
		s--;
	*s = 0;

	dir = opendir( path );
	if ( dir != NULL ) {
		while ( ( dirlist = readdir( dir ) ) != NULL )
		{
			sprintf( filename, "%s/%s", filebase, dirlist->d_name );
			Eclass_ScanFile( filename );

			if ( eclass_found ) {
				e = eclass_e;
				e->modelpath = strdup( dirlist->d_name );
				e->nShowFlags |= ECLASS_PLUGINENTITY;
				e->hPlug = hPlug;
			}
		}
		closedir( dir );
	}
	return 0;
}

#endif // USEPLUGINENTITIES
