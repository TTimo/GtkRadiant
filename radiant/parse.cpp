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

char token[MAXTOKEN];
qboolean unget;
char*     script_p;
int scriptline;

// Hydra: added support for GetTokenExtra()
const char *currentdelimiters;
qboolean script_keepdelimiter;

void StartTokenParsing( char *data ){
	scriptline = 1;
	script_p = data;
	unget = false;

	// Hydra: added support for GetTokenExtra()
	currentdelimiters = NULL;
	script_keepdelimiter = true;
}


qboolean GetToken( qboolean crossline ){
	char    *token_p;

	if ( unget ) {                   // is a token already waiting?
		unget = false;
		return true;
	}

	//
	// skip space
	//
skipspace:
	while ( *script_p <= 32 )
	{
		if ( !*script_p ) {
			if ( !crossline ) {
				Sys_FPrintf( SYS_WRN, "Warning: Line %i is incomplete [01]\n",scriptline );
			}
			return false;
		}

		if ( *script_p++ == '\n' ) {
			if ( !crossline ) {
				Sys_FPrintf( SYS_WRN, "Warning: Line %i is incomplete [02]\n",scriptline );
			}
			scriptline++;
		}
	}

	if ( script_p[0] == '/' && script_p[1] == '/' ) { // comment field
		if ( !crossline ) {
			Sys_FPrintf( SYS_WRN, "Warning: Line %i is incomplete [03]\n",scriptline );
		}
		while ( *script_p++ != '\n' )
			if ( !*script_p ) {
				if ( !crossline ) {
					Sys_FPrintf( SYS_WRN, "Warning: Line %i is incomplete [04]\n",scriptline );
				}
				return false;
			}
		scriptline++; // Hydra: fixed bad line numbers problem
		goto skipspace;
	}

	//
	// copy token
	//
	token_p = token;

	if ( *script_p == '"' ) {
		script_p++;
		while ( *script_p != '"' )
		{
			if ( !*script_p ) {
				Error( "EOF inside quoted token" );
			}
			*token_p++ = *script_p++;
			if ( token_p == &token[MAXTOKEN] ) {
				Error( "Token too large on line %i",scriptline );
			}
		}
		script_p++;
	}
	else{
		while ( *script_p > 32 )
		{
			// Hydra: added support for GetTokenExtra(), care was taken to maintain speed
			if ( ( currentdelimiters ) && ( !script_keepdelimiter ) && ( strchr( currentdelimiters,*( script_p ) ) ) ) {
				break;
			}

			*token_p++ = *script_p++;
			if ( token_p == &token[MAXTOKEN] ) {
				Error( "Token too large on line %i",scriptline );
			}

			// Hydra: added support for GetTokenExtra()
			if ( ( currentdelimiters ) && ( strchr( currentdelimiters,*( script_p - 1 ) ) ) ) {
				break;
			}

		}
	}

	*token_p = 0;

	return true;
}

void UngetToken( void ){
	unget = true;
}

/*
   ==============
   GetTokenExtra

   This function expands the use of GetToken() so it can be used to parse
   more complex file formats.

   Hydra - Notes:
   You can use this function to split a string like this

   string1:("string2")

   into two strings, like this:
   string1
   string2

   whilst still checking for the brackets and colons, like this:

   GetTokenExtra(false,":",false);// contains "string1"
   GetTokenExtra(false,":",true); // contains ":"
   GetTokenExtra(false,"(",true); // contains "("
   GetToken(false);               // contains "string2"
   GetTokenExtra(false,")",true); // contains ")"

   here's what you get, given the same string, with this code:

   GetToken(false); // contains "string1:("string2")"

   Parsing will end if any character in the script matches any one of the
   characters in the "delimiters" string.

   it's also possible to do things like this:

   source strings:
   1,2
   1:2
   1-2
   1*2

   code:
   GetTokenExtra(false,",:-*",false); // token contains "1"
   GetTokenExtra(false,",:-*",false); // token contains the delimiter that was used
   GetToken(false);                   // contains "2"
   ==============
 */
qboolean GetTokenExtra( qboolean crossline,const char *delimiters, qboolean keepdelimiter ){
	qboolean result;
	const char *olddelimiters = currentdelimiters; // store it

	currentdelimiters = delimiters; // change the delimiters
	script_keepdelimiter = keepdelimiter; // change the global flag

	result = GetToken( crossline );
	currentdelimiters = olddelimiters; // restore it
	return( result );
}

/*
   ==============
   TokenAvailable

   Returns true if there is another token on the line
   ==============
 */
qboolean TokenAvailable( void ){
	char *search_p;

	search_p = script_p;

	while ( *search_p <= 32 )
	{
		if ( *search_p == '\n' ) {
			return false;
		}
		if ( *search_p == 0 ) {
			return false;
		}
		search_p++;
	}

	if ( *search_p == ';' ) {
		return false;
	}

	return true;
}
