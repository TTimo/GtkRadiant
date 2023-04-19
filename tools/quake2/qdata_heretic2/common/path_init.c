/* -------------------------------------------------------------------------------

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

/*
   Nurail: Swiped from Q3Map2
 */



/* marker */
#define PATH_INIT_C

#if defined( __linux__ ) || defined( __BSD__ ) || defined( __APPLE__ )
	#define Q_UNIX
#endif

#ifdef Q_UNIX
	#include <unistd.h>
	#include <pwd.h>
	#include <limits.h>
#endif


/* dependencies */
#include "cmdlib.h"
#include "inout.h"



/* path support */
#define MAX_BASE_PATHS  10
#define MAX_GAME_PATHS  10

char                    *homePath;
char installPath[ MAX_OS_PATH ];

int numBasePaths;
char                    *basePaths[ MAX_BASE_PATHS ];
int numGamePaths;
char                    *gamePaths[ MAX_GAME_PATHS ];

/*
   some of this code is based off the original q3map port from loki
   and finds various paths. moved here from bsp.c for clarity.
 */

/*
   PathLokiGetHomeDir()
   gets the user's home dir (for ~/.q3a)
 */

char *LokiGetHomeDir( void ){
	#ifndef Q_UNIX
	return NULL;
	#else
	char            *home;
	uid_t id;
	struct passwd   *pwd;


	/* get the home environment variable */
	home = getenv( "HOME" );
	if ( home == NULL ) {
		/* do some more digging */
		id = getuid();
		setpwent();
		while ( ( pwd = getpwent() ) != NULL )
		{
			if ( pwd->pw_uid == id ) {
				home = pwd->pw_dir;
				break;
			}
		}
		endpwent();
	}

	/* return it */
	return home;
	#endif
}



/*
   PathLokiInitPaths()
   initializes some paths on linux/os x
 */

void LokiInitPaths( char *argv0 ){
	#ifndef Q_UNIX
	/* this is kinda crap, but hey */
	strcpy( installPath, "../" );
	#else
	char temp[ MAX_OS_PATH ];
	char        *home;
	char        *path;
	char        *last;
	qboolean found;


	/* get home dir */
	home = LokiGetHomeDir();
	if ( home == NULL ) {
		home = ".";
	}

	/* do some path divining */
	strcpy( temp, argv0 );
	if ( strrchr( temp, '/' ) ) {
		argv0 = strrchr( argv0, '/' ) + 1;
	}
	else
	{
		/* get path environment variable */
		path = getenv( "PATH" );

		/* minor setup */
		last[ 0 ] = path[ 0 ];
		last[ 1 ] = '\0';
		found = false;

		/* go through each : segment of path */
		while ( last[ 0 ] != '\0' && found == false )
		{
			/* null out temp */
			temp[ 0 ] = '\0';

			/* find next chunk */
			last = strchr( path, ':' );
			if ( last == NULL ) {
				last = path + strlen( path );
			}

			/* found home dir candidate */
			if ( *path == '~' ) {
				strcpy( temp, home );
				path++;
			}

			/* concatenate */
			if ( last > ( path + 1 ) ) {
				strncat( temp, path, ( last - path ) );
				strcat( temp, "/" );
			}
			strcat( temp, "./" );
			strcat( temp, argv0 );

			/* verify the path */
			if ( access( temp, X_OK ) == 0 ) {
				found++;
			}
			path = last + 1;
		}
	}

	/* flake */
	if ( realpath( temp, installPath ) ) {
		/* q3map is in "tools/" */
		*( strrchr( installPath, '/' ) ) = '\0';
		*( strrchr( installPath, '/' ) + 1 ) = '\0';
	}

	/* set home path */
	homePath = home;
	#endif
}



/*
   CleanPath() - ydnar
   cleans a dos path \ -> /
 */

void CleanPath( char *path ){
	while ( *path )
	{
		if ( *path == '\\' ) {
			*path = '/';
		}
		path++;
	}
}

/*
   AddBasePath() - ydnar
   adds a base path to the list
 */

void AddBasePath( char *path ){
	/* dummy check */
	if ( path == NULL || path[ 0 ] == '\0' || numBasePaths >= MAX_BASE_PATHS ) {
		return;
	}

	/* add it to the list */
	basePaths[ numBasePaths ] = safe_malloc( strlen( path ) + 1 );
	strcpy( basePaths[ numBasePaths ], path );
	CleanPath( basePaths[ numBasePaths ] );
	numBasePaths++;
}



/*
   AddHomeBasePath() - ydnar
   adds a base path to the beginning of the list, prefixed by ~/
 */

void AddHomeBasePath( char *path ){
	#ifdef Q_UNIX
	int i;
	char temp[ MAX_OS_PATH ];


	/* dummy check */
	if ( path == NULL || path[ 0 ] == '\0' ) {
		return;
	}

	/* make a hole */
	for ( i = 0; i < ( MAX_BASE_PATHS - 1 ); i++ )
		basePaths[ i + 1 ] = basePaths[ i ];

	/* concatenate home dir and path */
	sprintf( temp, "%s/%s", homePath, path );

	/* add it to the list */
	basePaths[ 0 ] = safe_malloc( strlen( temp ) + 1 );
	strcpy( basePaths[ 0 ], temp );
	CleanPath( basePaths[ 0 ] );
	numBasePaths++;
	#endif
}



/*
   AddGamePath() - ydnar
   adds a game path to the list
 */

void AddGamePath( char *path ){
	/* dummy check */
	if ( path == NULL || path[ 0 ] == '\0' || numGamePaths >= MAX_GAME_PATHS ) {
		return;
	}

	/* add it to the list */
	gamePaths[ numGamePaths ] = safe_malloc( strlen( path ) + 1 );
	strcpy( gamePaths[ numGamePaths ], path );
	CleanPath( gamePaths[ numGamePaths ] );
	numGamePaths++;
}




/*
   InitPaths() - ydnar
   cleaned up some of the path initialization code from bsp.c
   will remove any arguments it uses
 */

void InitPaths( int *argc, char **argv ){
	int i, j, k, len, len2;
	char temp[ MAX_OS_PATH ];
	char gamePath[MAX_OS_PATH], homeBasePath[MAX_OS_PATH], game_magic[10];

	strcpy( gamePath, "base" );
	strcpy( game_magic, "h" );
	strcpy( homeBasePath, ".heretic2" );

	/* note it */
	Sys_FPrintf( SYS_VRB, "--- InitPaths ---\n" );

	/* get the install path for backup */
	LokiInitPaths( argv[ 0 ] );

	/* set game to default (q3a) */
	numBasePaths = 0;
	numGamePaths = 0;

	/* parse through the arguments and extract those relevant to paths */
	for ( i = 0; i < *argc; i++ )
	{
		/* check for null */
		if ( argv[ i ] == NULL ) {
			continue;
		}

		/* -fs_basepath */
		if ( strcmp( argv[ i ], "-fs_basepath" ) == 0 ) {
			if ( ++i >= *argc ) {
				Error( "Out of arguments: No path specified after %s.", argv[ i - 1 ] );
			}
			argv[ i - 1 ] = NULL;
			AddBasePath( argv[ i ] );
			argv[ i ] = NULL;
		}

	}

	/* remove processed arguments */
	for ( i = 0, j = 0, k = 0; i < *argc && j < *argc; i++, j++ )
	{
		for ( j; j < *argc && argv[ j ] == NULL; j++ ) ;
		argv[ i ] = argv[ j ];
		if ( argv[ i ] != NULL ) {
			k++;
		}
	}
	*argc = k;

	/* add standard game path */
	AddGamePath( gamePath );

	/* if there is no base path set, figure it out */
	if ( numBasePaths == 0 ) {
		/* this is another crappy replacement for SetQdirFromPath() */
		len2 = strlen( game_magic );
		for ( i = 0; i < *argc && numBasePaths == 0; i++ )
		{
			/* extract the arg */
			strcpy( temp, argv[ i ] );
			CleanPath( temp );
			len = strlen( temp );
			Sys_FPrintf( SYS_VRB, "Searching for \"%s\" in \"%s\" (%d)...\n", game_magic, temp, i );

			/* this is slow, but only done once */
			for ( j = 0; j < ( len - len2 ); j++ )
			{
				/* check for the game's magic word */
				if ( Q_strncasecmp( &temp[ j ], game_magic, len2 ) == 0 ) {
					/* now find the next slash and nuke everything after it */
					while ( temp[ ++j ] != '/' && temp[ j ] != '\0' ) ;
					temp[ j ] = '\0';

					/* add this as a base path */
					AddBasePath( temp );
					break;
				}
			}
		}

		/* add install path */
		if ( numBasePaths == 0 ) {
			AddBasePath( installPath );
		}

		/* check again */
		if ( numBasePaths == 0 ) {
			Error( "Failed to find a valid base path." );
		}
	}

	/* this only affects unix */
	AddHomeBasePath( homeBasePath );

	/* initialize vfs paths */
	if ( numBasePaths > MAX_BASE_PATHS ) {
		numBasePaths = MAX_BASE_PATHS;
	}
	if ( numGamePaths > MAX_GAME_PATHS ) {
		numGamePaths = MAX_GAME_PATHS;
	}

	/* walk the list of game paths */
	//for( j = 0; j < numGamePaths; j++ )
	//{
	/* walk the list of base paths */
	//	for( i = 0; i < numBasePaths; i++ )
	//	{
	/* create a full path and initialize it */
	//		sprintf( temp, "%s/%s/", basePaths[ i ], gamePaths[ j ] );
	//		vfsInitDirectory( temp );
	//	}
	//}

	/* done */
	Sys_Printf( "\n" );
}
