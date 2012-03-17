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


#include "qdata.h"

byte            *byteimage, *lbmpalette;
int byteimagewidth, byteimageheight;

qboolean TrueColorImage;
unsigned        *longimage;
int longimagewidth, longimageheight;

char pic_prefix[1024];
extern char        *g_outputDir;

/*
   ===============
   Cmd_Pic
   ===============
 */

void Cmd_Pic( void ){
	int xl,yl,xh,yh,w,h;
	byte            *dest, *source;
	int flags, value, contents;
	char lumpname[128];
	char animname[128];
	byte buffer[256 * 256];
	unsigned bufferl[256 * 256];
	char filename[1024];
	unsigned        *destl, *sourcel;
	int linedelta, x, y;
	int size;
	miptex_t        *qtex;
	miptex32_t      *qtex32;
	float scale_x, scale_y;

	GetScriptToken( false );
	strcpy( lumpname, token );

	GetScriptToken( false );
	xl = atoi( token );
	GetScriptToken( false );
	yl = atoi( token );
	GetScriptToken( false );
	w = atoi( token );
	GetScriptToken( false );
	h = atoi( token );

	total_x += w;
	total_y += h;
	total_textures++;

	if ( ( w & 7 ) || ( h & 7 ) ) {
		Error( "line %i: miptex sizes must be multiples of 8", scriptline );
	}

	flags = 0;
	contents = 0;
	value = 0;

	animname[0] = 0;

	scale_x = scale_y = 0.5;

	if ( TrueColorImage ) {
		sprintf( filename, "%spics/%s/%s.m32", g_outputDir, pic_prefix, lumpname );
		if ( g_release ) {
			return; // textures are only released by $maps

		}
		xh = xl + w;
		yh = yl + h;

		if ( xl >= longimagewidth || xh > longimagewidth ||
			 yl >= longimageheight || yh > longimageheight ) {
			Error( "line %i: bad clip dimmensions (%d,%d) (%d,%d) > image (%d,%d)", scriptline, xl,yl,w,h,longimagewidth,longimageheight );
		}

		sourcel = longimage + ( yl * longimagewidth ) + xl;
		destl = bufferl;
		linedelta = ( longimagewidth - w );

		for ( y = yl ; y < yh ; y++ )
		{
			for ( x = xl ; x < xh ; x++ )
			{
				*destl++ = *sourcel++;  // RGBA
			}
			sourcel += linedelta;
		}

		qtex32 = CreateMip32( bufferl, w, h, &size, false );

		qtex32->flags |= LittleLong( flags );
		qtex32->contents = contents;
		qtex32->value = value;
		qtex32->scale_x = scale_x;
		qtex32->scale_y = scale_y;
		sprintf( qtex32->name, "%s/%s", pic_prefix, lumpname );
		if ( animname[0] ) {
			sprintf( qtex32->animname, "%s/%s", pic_prefix, animname );
		}

		//
		// write it out
		//
		printf( "writing %s\n", filename );
		SaveFile( filename, (byte *)qtex32, size );

		free( qtex32 );
	}
	else
	{
		sprintf( filename, "%spics/%s/%s.m8", g_outputDir, pic_prefix, lumpname );
		if ( g_release ) {
			return; // textures are only released by $maps

		}
		xh = xl + w;
		yh = yl + h;

		if ( xl >= byteimagewidth || xh > byteimagewidth ||
			 yl >= byteimageheight || yh > byteimageheight ) {
			Error( "line %i: bad clip dimmensions (%d,%d) (%d,%d) > image (%d,%d)", scriptline, xl,yl,w,h,byteimagewidth,byteimageheight );
		}

		source = byteimage + yl * byteimagewidth + xl;
		dest = buffer;
		linedelta = byteimagewidth - w;

		for ( y = yl ; y < yh ; y++ )
		{
			for ( x = xl ; x < xh ; x++ )
			{
				*dest++ = *source++;
			}
			source += linedelta;
		}

		qtex = CreateMip( buffer, w, h, lbmpalette, &size, false );

		qtex->flags = flags;
		qtex->contents = contents;
		qtex->value = value;
		sprintf( qtex->name, "%s/%s", pic_prefix, lumpname );
		if ( animname[0] ) {
			sprintf( qtex->animname, "%s/%s", pic_prefix, animname );
		}

		//
		// write it out
		//
		printf( "writing %s\n", filename );
		SaveFile( filename, (byte *)qtex, size );

		free( qtex );
	}
}


/*
   ===============
   Cmd_picdir
   ===============
 */
void Cmd_Picdir( void ){
	char filename[1024];

	GetScriptToken( false );
	strcpy( pic_prefix, token );
	// create the directory if needed
	sprintf( filename, "%sPics", g_outputDir );
	Q_mkdir( filename );
	sprintf( filename, "%sPics/%s", g_outputDir, pic_prefix );
	Q_mkdir( filename );
}
