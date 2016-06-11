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

/*
   Camera plugin for GtkRadiant
   Copyright (C) 2002 Splash Damage Ltd.
 */

#include "camera.h"

void Sys_ERROR( char* text, ... ){
	va_list argptr;
	char buf[32768];

	va_start( argptr,text );
	vsprintf( buf, text,argptr );
	va_end( argptr );

	Sys_FPrintf( SYS_ERR, "Camera::ERROR->%s", buf );
}

char* UnixToDosPath( char* path ){
#ifndef _WIN32
	return path;
#else
	for ( char* p = path; *p; p++ )
	{
		if ( *p == '/' ) {
			*p = '\\';
		}
	}
	return path;
#endif
}

void CAMERA_ExtractFilePath( const char *path, char *dest ){
	const char *src;

	src = path + strlen( path ) - 1;

//
// back up until a \ or the start
//
	while ( src != path && *( src - 1 ) != '/' && *( src - 1 ) != '\\' )
		src--;

	memcpy( dest, path, src - path );
	dest[src - path] = 0;
}

const char* ExtractFilename( const char* path ){
	char* p = (char *)strrchr( path, '/' );
	if ( !p ) {
		p = (char *)strrchr( path, '\\' );

		if ( !p ) {
			return path;
		}
	}
	return ++p;
}

int Q_stricmp( const char *s1, const char *s2 ) {
	return stricmp( s1, s2 );
}

/*
   ==============
   FileExists
   ==============
 */
bool FileExists( const char *filename ){
	FILE    *f;

	f = fopen( filename, "r" );
	if ( !f ) {
		return false;
	}
	fclose( f );
	return true;
}

//
// command buffer
// empty wrappers, don't really use them here
//
void Cbuf_AddText( const char *text ) {};
void Cbuf_Execute( void ) {};

//
// Common
//

void CDECL Com_Error( int level, const char *error, ... ){
	va_list argptr;
	char buf[32768];

	va_start( argptr,error );
	vsprintf( buf, error,argptr );
	va_end( argptr );

	Sys_FPrintf( SYS_ERR, "Camera::ERROR->%s", buf );
}

void CDECL Com_Printf( const char* msg, ... ){
	va_list argptr;
	char buf[32768];

	va_start( argptr,msg );
	vsprintf( buf, msg,argptr );
	va_end( argptr );

	Sys_Printf( "Camera::%s", buf );
}

void CDECL Com_DPrintf( const char* msg, ... ){
#ifdef _DEBUG
	va_list argptr;
	char buf[32768];

	va_start( argptr,msg );
	vsprintf( buf, msg,argptr );
	va_end( argptr );

	Sys_Printf( "Camera::%s", buf );
#endif
}

void *Com_Allocate( int bytes ) {
	return( malloc( bytes ) );
}

void Com_Dealloc( void *ptr ) {
	free( ptr );
}

//
// Filesystem
//

#ifdef _WIN32
	#pragma warning(disable : 4311)
	#pragma warning(disable : 4312)
#endif

int FS_Read( void *buffer, int len, fileHandle_t f ) {
	return fread( buffer, len, 1, (FILE *)f );
}

int FS_Write( const void *buffer, int len, fileHandle_t h ) {
	return fwrite( buffer, len, 1, (FILE *)h );
}

int FS_ReadFile( const char *qpath, void **buffer ) {
	fileHandle_t h;
	byte*         buf;
	int len;

	buf = NULL;

	len = FS_FOpenFileRead( qpath, &h, qfalse );

	if ( h == 0 ) {
		if ( buffer ) {
			*buffer = NULL;
		}

		return -1;
	}

	buf = (byte *)Com_Allocate( len + 1 );

	*buffer = buf;

	FS_Read( buf, len, h );

	buf[len] = 0;
	FS_FCloseFile( h );

	return len;
}

void FS_FreeFile( void *buffer ) {
	Com_Dealloc( buffer );
}

int FS_FOpenFileRead( const char *filename, fileHandle_t *file, qboolean uniqueFILE ) {
	FILE  *fh;
	long len;

	fh = fopen( filename, "rb" );
	*file = *(fileHandle_t *)&fh;

	if ( file ) {
		fseek( fh, 0, SEEK_END );
		len = ftell( fh );
		rewind( fh );
		return len;
	}
	else{
		return -1;
	}
}

fileHandle_t FS_FOpenFileWrite( const char *filename ) {
	FILE          *fh;
	fileHandle_t f;

	memset( &f, 0, sizeof( f ) );

	fh = fopen( filename, "wb" );

	f = (fileHandle_t)fh;
	return f;
}

void FS_FCloseFile( fileHandle_t f ) {
	fclose( (FILE *)f );
}
