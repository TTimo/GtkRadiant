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

// parse.h -- text file parsing routines

#define MAXTOKEN    1024

extern char token[MAXTOKEN];
extern int scriptline;

// NOTE: added WINAPI call syntax to export these for plugins in _QERScripLibTable
void        StartTokenParsing( char *data );
qboolean  GetToken( qboolean crossline );
void      UngetToken( void );
qboolean  TokenAvailable( void );
qboolean  GetTokenExtra( qboolean crossline,const char *delimiters,qboolean keepdelimiter ); // Hydra: added support for GetTokenExtra()
