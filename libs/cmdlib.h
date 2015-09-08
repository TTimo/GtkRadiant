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
// start of shared cmdlib stuff
//

#ifndef __CMDLIB__
#define __CMDLIB__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <limits.h>
#ifdef _WIN32
  #define PATH_MAX 260
#endif

#include "bytebool.h"

// some easy portability crap
#ifdef _WIN32
  #include <direct.h>
  #define Q_mkdir( a,b ) _mkdir( a )
#else
  #include <sys/stat.h>
  #define Q_mkdir( a,b ) mkdir( a,b )
#endif

#if defined(_MSC_VER) && _MSC_VER<1900 && !(defined safe_malloc)
void *safe_malloc( size_t size );
void *safe_malloc_info( size_t size, char* info );
#endif

#if defined(_MSC_VER) && _MSC_VER<1900 && !(defined snprintf)
#define snprintf _snprintf
#endif
#ifndef Q_strncpyz
#define Q_strncpyz(_dst, _source, _len) do { strncpy((_dst), (_source), (_len) - 1); (_dst)[(_len) - 1] = 0; } while( 0 )
#endif

void    DefaultExtension( char *path, char *extension, size_t length );
void    DefaultPath( char *path, const char *basepath, size_t length );
void    StripFilename( char *path );
void    StripExtension( char *path );
void    ExtractFilePath( const char *path, char *dest, size_t size );
void    ExtractFileName( const char *path, char *dest, size_t size );
void    ExtractFileBase( const char *path, char *dest, size_t size );
void    ExtractFileExtension( const char *path, char *dest, size_t size );
/*!
   \brief create all directories leading to a file path. if you pass a directory, terminate it with a '/'
 */
void  CreateDirectoryPath( const char *path );

short   BigShort( short l );
short   LittleShort( short l );
int     BigLong( int l );
int     LittleLong( int l );
float   BigFloat( float l );
float   LittleFloat( float l );
void *qmalloc( size_t size );
void* qblockmalloc( size_t nSize );

void ConvertDOSToUnixName( char *dst, const char *src );
#ifdef __cplusplus
char* StrDup( char* pStr );
#endif
char* StrDup( const char* pStr );

// TTimo started adding portability code:
// return true if spawning was successful, false otherwise
// on win32 we have a bCreateConsole flag to create a new console or run inside the current one
//boolean Q_Exec(const char* pCmd, boolean bCreateConsole);
// execute a system command:
//   cmd: the command to run
//   cmdline: the command line
// NOTE TTimo following are win32 specific:
//   execdir: the directory to execute in
//   bCreateConsole: spawn a new console or not
// return values;
//   if the spawn was fine
//   TODO TTimo add functionality to track the process until it dies
bool Q_Exec( const char *cmd, char *cmdline, const char *execdir, bool bCreateConsole );

#endif
