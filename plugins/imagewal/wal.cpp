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
#include "wal.h"

#include "q2_palette.h"

#define Sys_Printf g_FuncTable.m_pfnSysPrintf

void LoadWAL( const char *name, unsigned char **pic, int *width, int *height ){
//    FILE      *f;
	miptex_t    *wal_header;
//    rgb_t	*palette;
	int i, num_pixels, size;
//    char	text_buf[255];
	unsigned int length;
	unsigned char   *palette_ent, *buf_temp;
	unsigned char       *buffer, *wal_file_buffer;

	// open file
	if ( ( length = vfsLoadFile( (char *) name, (void **) &wal_file_buffer, 0 ) ) == (unsigned int) -1 ) {
		Sys_Printf( "Unable to open file %s\n",name );
		return;
	}

	wal_header = (miptex_t *)wal_file_buffer;

	// make sure we have a valid bitmap file
	if ( wal_header->width & 15 ) {
		vfsFreeFile( wal_file_buffer );
		Sys_Printf( "Invalid WAL file %s: Width not multiple of 16!\n", name );
		return;
	}

	if ( wal_header->height & 15 ) {
		vfsFreeFile( wal_file_buffer );
		Sys_Printf( "Invalid WAL file %s: Height not multiple of 16!\n", name );
		return;
	}


	// Get WAL Info
	*width    = wal_header->width;      // Only interested in 1st MIP
	*height   = wal_header->height;
	num_pixels = ( *width ) * ( *height );
	size = num_pixels * 4;

	// Allocate buffer
	buf_temp = (unsigned char *)( g_malloc( size ) );
	*pic = buf_temp;

	// Image data
	buffer = wal_file_buffer + wal_header->offsets[0];


	// Load texture into buffer
	palette_ent = buffer;
	for ( i = 0; i < num_pixels; i++ )
	{
		*buf_temp++ = quake2_palette[*palette_ent][0];
		*buf_temp++ = quake2_palette[*palette_ent][1];
		*buf_temp++ = quake2_palette[*palette_ent][2];
		*buf_temp++ = 255;      // No alpha
		palette_ent++;
	}

	vfsFreeFile( wal_file_buffer );
}
