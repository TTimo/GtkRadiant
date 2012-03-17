/*
   Copyright (c) 2001, Loki software, inc.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of Loki software nor the names of its contributors may be used
   to endorse or promote products derived from this software without specific prior
   written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT,INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// Application settings load/save
//
// Leonardo Zide (leo@lokigames.com)
//

#include "stdafx.h"
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "str.h"
#include "file.h"

// =============================================================================
// Static functions

bool read_var( const char *filename, const char *section, const char *key, char *value ){
	char line[1024], *ptr;
	FILE *rc;

	rc = fopen( filename, "rt" );

	if ( rc == NULL ) {
		return false;
	}

	while ( fgets( line, 1024, rc ) != 0 )
	{
		// First we find the section
		if ( line[0] != '[' ) {
			continue;
		}

		ptr = strchr( line, ']' );
		*ptr = '\0';

		if ( strcmp( &line[1], section ) == 0 ) {
			while ( fgets( line, 1024, rc ) != 0 )
			{
				ptr = strchr( line, '=' );

				if ( ptr == NULL ) {
					// reached the end of the section
					fclose( rc );
					return false;
				}
				*ptr = '\0';

				// remove spaces
				while ( line[strlen( line ) - 1] == ' ' )
					line[strlen( line ) - 1] = '\0';

				if ( strcmp( line, key ) == 0 ) {
					strcpy( value, ptr + 1 );
					fclose( rc );

					if ( value[strlen( value ) - 1] == 10 || value[strlen( value ) - 1] == 13 || value[strlen( value ) - 1] == 32 ) {
						value[strlen( value ) - 1] = 0;
					}

					return true;
				}
			}
		}
	}

	fclose( rc );
	return false;
}

static bool save_var( const char *filename, const char *section, const char *key, const char *value ){
	char line[1024], *ptr;
	MemStream old_rc;
	bool found;
	FILE *rc;

	rc = fopen( filename, "rb" );

	if ( rc != NULL ) {
		guint32 len;
		void *buf;

		fseek( rc, 0, SEEK_END );
		len = ftell( rc );
		rewind( rc );
		buf = qmalloc( len );
		fread( buf, len, 1, rc );
		old_rc.Write( buf, len );
		free( buf );
		fclose( rc );
		old_rc.Seek( 0, SEEK_SET );
	}

	// TTimo: changed to binary writing. It doesn't seem to affect linux version, and win32 version was happending a lot of '\n'
	rc = fopen( filename, "wb" );

	if ( rc == NULL ) {
		return false;
	}

	// First we need to find the section
	found = false;
	while ( old_rc.ReadString( line, 1024 ) != NULL )
	{
		fputs( line, rc );

		if ( line[0] == '[' ) {
			ptr = strchr( line, ']' );
			*ptr = '\0';

			if ( strcmp( &line[1], section ) == 0 ) {
				found = true;
				break;
			}
		}
	}

	if ( !found ) {
		fputs( "\n", rc );
		fprintf( rc, "[%s]\n", section );
	}

	fprintf( rc, "%s=%s\n", key, value );

	while ( old_rc.ReadString( line, 1024 ) != NULL )
	{
		ptr = strchr( line, '=' );

		if ( ptr != NULL ) {
			*ptr = '\0';

			if ( strcmp( line, key ) == 0 ) {
				break;
			}

			*ptr = '=';
			fputs( line, rc );
		}
		else
		{
			fputs( line, rc );
			break;
		}
	}

	while ( old_rc.ReadString( line, 1024 ) != NULL )
		fputs( line, rc );

	fclose( rc );
	return true;
}

// =============================================================================
// Global functions

bool WINAPI profile_save_int( const char *filename, const char *section, const char *key, int value ){
	char buf[16];
	sprintf( buf, "%d", value );
	return save_var( filename, section, key, buf );
}

bool WINAPI profile_save_float( const char *filename, const char *section, const char *key, float value ){
	char buf[16];
	sprintf( buf, "%f", value );
	return save_var( filename, section, key, buf );
}

bool WINAPI profile_save_string( const char * filename, const char *section, const char *key, const char *value ){
	return save_var( filename, section, key, value );
}

bool profile_save_buffer( const char * rc_path, const char *name, void *buffer, guint32 size ){
	bool ret = false;
	char filename[PATH_MAX];
	sprintf( filename, "%s/%s.bin", rc_path, name );
	FILE *f;

	f = fopen( filename, "wb" );

	if ( f != NULL ) {
		if ( fwrite( buffer, size, 1, f ) == 1 ) {
			ret = true;
		}

		fclose( f );
	}

	return ret;
}

bool profile_load_buffer( const char * rc_path, const char *name, void *buffer, guint32 *plSize ){
	char filename[PATH_MAX];
	sprintf( filename, "%s/%s.bin", rc_path, name );
	bool ret = false;
	guint32 len;
	FILE *f;

	f = fopen( filename, "rb" );

	if ( f != NULL ) {
		fseek( f, 0, SEEK_END );
		len = ftell( f );
		rewind( f );

		if ( len > *plSize ) {
			len = *plSize;
		}
		else{
			*plSize = len;
		}

		if ( fread( buffer, len, 1, f ) == 1 ) {
			ret = true;
		}

		fclose( f );
	}

	return true;
}

int WINAPI profile_load_int( const char *filename, const char *section, const char *key, int default_value ){
	char value[1024];

	if ( read_var( filename, section, key, value ) ) {
		return atoi( value );
	}
	else{
		return default_value;
	}
}

float WINAPI profile_load_float( const char *filename, const char *section, const char *key, float default_value ){
	char value[1024];

	if ( read_var( filename, section, key, value ) ) {
		return atof( value );
	}
	else{
		return default_value;
	}
}

char* WINAPI profile_load_string( const char *filename, const char *section, const char *key, const char *default_value ){
	static Str ret;
	char value[1024];

	if ( read_var( filename, section, key, value ) ) {
		ret = value;
	}
	else{
		ret = default_value;
	}

	return (char*)ret.GetBuffer();
}
