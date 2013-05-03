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

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "m8.h"

void LoadM8( const char *name, unsigned char **pic, int *width, int *height ){
//    FILE      *f;
	m8_header_t *m8_header;
	rgb_t   *palette;
	int i, num_pixels, size;
	char text_buf[255] = { 0 };
	char    *text_dot_pos;
	unsigned int length;
	unsigned char   *palette_ent, *buf_temp;
	unsigned char       *buffer, *m8_file_buffer;

	strcpy( text_buf, name );
	text_dot_pos = strchr( text_buf, '.' );
	if ( text_dot_pos ) {
		*text_dot_pos = 0;
	}
	// Fix for .pcx.m8 extention
	strcat( text_buf, ".pcx.m8" );

	// open file
	if ( ( length = vfsLoadFile( (char *) text_buf, (void **) &m8_file_buffer, 0 ) ) == (unsigned int) -1 ) {
		strcpy( text_buf, name );
		for ( i = ( strlen( text_buf ) - 1 ); i > 0; i-- )
		{
			if ( text_buf[i] == '.' ) {
				text_buf[i] = 0;
				break;
			}
		}
		strcat( text_buf, ".m8" );
		if ( ( length = vfsLoadFile( (char *) text_buf, (void **) &m8_file_buffer, 0 ) ) == (unsigned int) -1 ) {
			Sys_Printf( "Unable to open file %s\n",name );
			return;
		}
	}

	m8_header = (m8_header_t *)m8_file_buffer;

	// make sure we have a valid M8 file
	if ( m8_header->version != M8_VERSION ) {
		vfsFreeFile( m8_file_buffer );
		Sys_Printf( "Invalid M8 file %s\n", name );
		return;
	}

	// Get M8 Info
	*width    = m8_header->width[0];        // Only interested in 1st MIP
	*height   = m8_header->height[0];
	num_pixels = ( *width ) * ( *height );
	size = num_pixels * 4;

	// Allocate buffer
	buf_temp = (unsigned char *)( g_malloc( size ) );
	*pic = buf_temp;

	// Load Palette
	palette = m8_header->palette;

	// Image data
	buffer = m8_file_buffer + m8_header->offsets[0];


	// Load texture into buffer
	palette_ent = buffer;
	for ( i = 0; i < num_pixels; i++ )
	{
		*buf_temp++ = palette[*palette_ent].r;
		*buf_temp++ = palette[*palette_ent].g;
		*buf_temp++ = palette[*palette_ent].b;
		*buf_temp++ = 255;  // No alpha
		palette_ent++;
	}

	Sys_Printf( "Loaded file %s\n",text_buf );

	vfsFreeFile( m8_file_buffer );
}
