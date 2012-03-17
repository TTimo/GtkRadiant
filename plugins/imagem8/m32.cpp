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
#include "m32.h"

void LoadM32( const char *name, unsigned char **pic, int *width, int *height ){
//    FILE      *f;
	m32_header_t    *m32_header;
	//rgb_t	*palette;
	int i, num_pixels, size;
//    char	text_buf[255];
	unsigned int length;
	unsigned char   *palette_ent, *buf_temp;
	unsigned char       *buffer, *m32_file_buffer;

	// open file
	if ( ( length = vfsLoadFile( (char *) name, (void **) &m32_file_buffer, 0 ) ) == (unsigned int) -1 ) {
		Sys_Printf( "Unable to open file %s\n",name );
		return;
	}

	m32_header = (m32_header_t *)m32_file_buffer;

	// make sure we have a valid bitmap file
	if ( m32_header->version != M32_VERSION ) {
		vfsFreeFile( m32_file_buffer );
		Sys_Printf( "Invalid M32 file %s\n", name );
	}

	// Get M32 Info
	*width    = m32_header->width[0];       // Only interested in 1st MIP
	*height   = m32_header->height[0];
	num_pixels = ( *width ) * ( *height );
	size = num_pixels * 4;

	// Allocate buffer
	buf_temp = (unsigned char *)( g_malloc( size ) );
	*pic = buf_temp;

	// Image data
	buffer = m32_file_buffer + m32_header->offsets[0];


	// Load texture into buffer
	palette_ent = buffer;
	for ( i = 0; i < size; i++, palette_ent++ )
	{
		*buf_temp++ = *palette_ent;
	}

	vfsFreeFile( m32_file_buffer );
}
