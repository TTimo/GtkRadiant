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

#include "../common/cmdlib.h"
#include "p3dlib.h"

#ifdef WIN32
#include <io.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_POLYSETS 64

#if defined( __linux__ ) || defined( __FreeBSD__ ) || defined( __APPLE__ )
#define _strcmpi Q_stricmp
#define filelength Q_filelength
#define strlwr strlower
#endif
typedef struct
{
	long len;

	int numPairs;
	char polysetNames[MAX_POLYSETS][256];
	char shaders[MAX_POLYSETS][256];

	char *buffer, *curpos;
} p3d_t;

static p3d_t p3d;

static int P3DProcess();
static int P3DGetToken( int restOfLine );

static char s_token[1024];
static int s_curpair;

/*
** P3DLoad
**
*/
int P3DLoad( const char *filename ){
	FILE *fp = fopen( filename, "rb" );

	if ( !fp ) {
		return 0;
	}

	memset( &p3d, 0, sizeof( p3d ) );

	p3d.len = filelength( fileno( fp ) );

	p3d.curpos = p3d.buffer = malloc( p3d.len );

	if ( fread( p3d.buffer, p3d.len, 1, fp ) != 1 ) {
		fclose( fp );
		return 0;
	}

	fclose( fp );

	return P3DProcess();
}

/*
** P3DClose
**
*/
void P3DClose(){
	if ( p3d.buffer ) {
		free( p3d.buffer );
		p3d.buffer = 0;
	}
}

int CharIsTokenDelimiter( int ch ){
	if ( ch <= 32 ) {
		return 1;
	}
	return 0;
}

int P3DSkipToToken( const char *name ){
	while ( P3DGetToken( 0 ) )
	{
		if ( !_strcmpi( s_token, name ) ) {
			return 1;
		}
	}

	return 0;
}

/*
** P3DGetToken
**
*/
int P3DGetToken( int restOfLine ){
	int i = 0;

	if ( p3d.buffer == 0 ) {
		return 0;
	}

	if ( ( p3d.curpos - p3d.buffer ) == p3d.len ) {
		return 0;
	}

	// skip over crap
	while ( ( ( p3d.curpos - p3d.buffer ) < p3d.len ) &&
			( *p3d.curpos <= 32 ) )
	{
		p3d.curpos++;
	}

	while ( ( p3d.curpos - p3d.buffer ) < p3d.len )
	{
		s_token[i] = *p3d.curpos;

		p3d.curpos++;
		i++;

		if ( ( CharIsTokenDelimiter( s_token[i - 1] ) && !restOfLine ) ||
			 ( ( s_token[i - 1] == '\n' ) ) ) {
			s_token[i - 1] = 0;
			break;
		}
	}

	s_token[i] = 0;

	return 1;
}

int P3DGetNextPair( char **psetName, char **associatedShader ){
	if ( s_curpair < p3d.numPairs ) {
		*psetName = p3d.polysetNames[s_curpair];
		*associatedShader = p3d.shaders[s_curpair];
		s_curpair++;
		return 1;
	}

	return 0;
}

int P3DSkipToTokenInBlock( const char *name ){
	int iLevel = 0;

	while ( P3DGetToken( 0 ) )
	{
		if ( !_strcmpi( s_token, "}" ) ) {
			iLevel--;
		}
		else if ( !_strcmpi( s_token, "{" ) ) {
			iLevel++;
		}

		if ( !_strcmpi( s_token, name ) ) {
			return 1;
		}

		if ( iLevel == 0 ) {
			return 0;
		}
	}

	return 0;
}

/*
** P3DProcess
**
** Nothing fancy here.
*/
int P3DProcess(){

	s_curpair = 0;

	// first token should be a string
	P3DGetToken( 1 );       // Voodoo Ascii File

	// skip to the first Obj declaration
	while ( P3DGetToken( 0 ) )
	{
		if ( !_strcmpi( s_token, "Obj" ) ) {
			int j = 0, k = 0;

			if ( P3DSkipToToken( "Text" ) ) {
				if ( P3DSkipToTokenInBlock( "TMap" ) ) {
					char *p;

					if ( !P3DSkipToToken( "Path" ) ) {
						return 0;
					}

					if ( !P3DGetToken( 1 ) ) {
						return 0;
					}

					while ( s_token[j] != 0 )
					{
						if ( s_token[j] == '\\' ) {
							j++;
							p3d.shaders[p3d.numPairs][k] = '/';
						}
						else
						{
							p3d.shaders[p3d.numPairs][k] = s_token[j];
						}
						j++;
						k++;
					}
					p3d.shaders[p3d.numPairs][k] = 0;

					//
					// strip off any explicit extensions
					//
					if ( ( p = strrchr( p3d.shaders[p3d.numPairs], '/' ) ) != 0 ) {
						while ( *p )
						{
							if ( *p == '.' ) {
								*p = 0;
								break;
							}
							p++;
						}
					}

					//
					// skip to the end of the Object and grab its name
					//
					if ( !P3DSkipToToken( "Name" ) ) {
						return 0;
					}

					if ( P3DGetToken( 0 ) ) {
						// strip off leading 'Obj_' if it exists
						if ( strstr( s_token, "Obj_" ) == s_token ) {
							strcpy( p3d.polysetNames[p3d.numPairs], s_token + strlen( "Obj_" ) );
						}
						else{
							strcpy( p3d.polysetNames[p3d.numPairs], s_token );
						}

						// strip off trailing unused color information
//						if ( strrchr( p3d.polysetNames[p3d.numPairs], '_' ) != 0 )
//							*strrchr( p3d.polysetNames[p3d.numPairs], '_' ) = 0;

						p3d.numPairs++;
					}
					else
					{
						return 0;
					}
				}
			}
		}
	}

	s_curpair = 0;

	return 1;
}

#if 0
void SkinFromP3D( const char *file ){
	char filename[1024];
	char *psetName, *associatedShader;

	/*
	** a P3D file contains a list of polysets, each with a list of associated
	** texture names that constitute it's
	**
	** Thus:
	**
	** P3D file -> skin
	** polyset  -> polyset
	**   texture -> texture.SHADER becomes polyset's shader
	*/
	sprintf( filename, "%s/%s", g_cddir, file );

	if ( !P3DLoad( filename ) ) {
		Error( "unable to load '%s'", filename );
	}

	while ( P3DGetNextPair( &psetName, &associatedShader ) )
	{
		int i;

		// find the polyset in the object that this particular pset/shader pair
		// corresponds to and append the shader to it
		for ( i = 0; i < g_data.model.numSurfaces; i++ )
		{
			if ( !_strcmpi( g_data.surfData[i].header.name, psetName ) ) {
				char *p;

				if ( strstr( associatedShader, gamedir + 1 ) ) {
					p = strstr( associatedShader, gamedir + 1 ) + strlen( gamedir ) - 1;
				}
				else
				{
					p = associatedShader;
				}

				strcpy( g_data.surfData[i].shaders[g_data.surfData[i].header.numShaders].name, p );

				g_data.surfData[i].header.numShaders++;
			}
		}

	}

	P3DClose();
}
#endif
