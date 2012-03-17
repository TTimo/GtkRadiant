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
int longimagewidth, longimageheight;

char book_prefix[1024];
byte buffer[640 * 480];
unsigned long bufferl[640 * 480];

miptex_t *CreateBook8( byte *buffer, int w, int h, byte *palette, int *FinalSize ){
	miptex_t    *mp;
	int i, j;
	byte        *pos;
	int size;

	size = sizeof( *mp ) + ( w * h );
	mp = (miptex_t *)SafeMalloc( size, "CreateBook8" );
	memset( mp, 0, size );

	mp->version = MIP_VERSION;

	for ( i = j = 0; i < 256; i++,j += 3 )
	{
		mp->palette[i].r = palette[j];
		mp->palette[i].g = palette[j + 1];
		mp->palette[i].b = palette[j + 2];
	}
	pos = (byte *)( mp + 1 );

	mp->width[0] = w;
	mp->height[0] = h;
	mp->offsets[0] = sizeof( *mp );
	memcpy( pos, buffer, w * h );

	*FinalSize = size;
	return( mp );
}

miptex32_t *CreateBook32( long *buffer, int w, int h, int *FinalSize ){
	miptex32_t  *mp;
	byte        *pos;
	int size;

	size = sizeof( *mp ) + ( w * h * 4 );
	mp = (miptex32_t *)SafeMalloc( size, "CreateBook32" );
	memset( mp, 0, size );

	mp->version = MIP32_VERSION;

	pos = (byte *)( mp + 1 );

	mp->width[0] = w;
	mp->height[0] = h;
	mp->offsets[0] = sizeof( *mp );
	memcpy( pos, buffer, w * h * 4 );

	*FinalSize = size;
	return( mp );
}


// Routines to chop a random sized image into gl texture friendly chunks

typedef struct rect_s
{
	int x, y;
	int w, h;
	char name[4];
} rect_t;

int GetCoords( int x, int store[MAX_MD2SKINS] ){
	int index, start, delta;

	index = 0;
	start = 0;
	delta = 256;

	store[index++] = start;
	while ( x )
	{
		if ( x >= delta ) {
			start += delta;
			store[index++] = start;
			x -= delta;
		}
		else
		{
			delta >>= 1;
		}
	}
	return( index );
}

int ChopImage( int w, int h, rect_t coords[MAX_MD2SKINS] ){
	int xs[MAX_MD2SKINS], ys[MAX_MD2SKINS];
	int xcount, ycount, x, y, index;

	index = 0;
	xcount = GetCoords( w, xs ) - 1;
	ycount = GetCoords( h, ys ) - 1;

	for ( y = 0; y < ycount; y++ )
	{
		for ( x = 0; x < xcount; x++, index++ )
		{
			coords[index].x = xs[x];
			coords[index].y = ys[y];
			coords[index].w = xs[x + 1] - xs[x];
			coords[index].h = ys[y + 1] - ys[y];
			coords[index].name[0] = x + '0';
			coords[index].name[1] = y + '0';
			coords[index].name[2] = 0;
		}
	}
	return( index );
}

/*
   ===============
   Cmd_Pic
   ===============
 */

void Cmd_Book(){
	int xl,yl,xh,yh,w,h;
	byte            *dest, *source;
	int flags, value, contents;
	char lumpname[64];
	char filename[1024];
	unsigned long   *destl, *sourcel;
	int linedelta, x, y;
	int size;
	miptex_t        *qtex;
	miptex32_t      *qtex32;
	float scale_x, scale_y;
	int numrects, i;
	rect_t coords[MAX_MD2SKINS];
	bookframe_t bframes[MAX_MD2SKINS];
	bookframe_t     *bf;
	book_t book;

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

	scale_x = scale_y = 0.5;

	if ( g_release ) {
		return;
	}

	if ( TrueColorImage ) {
		xh = xl + w;
		yh = yl + h;

		if ( xl >= longimagewidth || xh > longimagewidth ||
			 yl >= longimageheight || yh > longimageheight ) {
			Error( "line %i: bad clip dimmensions (%d,%d) (%d,%d) > image (%d,%d)", scriptline, xl,yl,w,h,longimagewidth,longimageheight );
		}

		sourcel = (unsigned long *) longimage + ( yl * longimagewidth ) + xl;
		destl = (unsigned long *) longimage;
		linedelta = ( longimagewidth - w );

		for ( y = yl; y < yh; y++ )
		{
			for ( x = xl; x < xh; x++ )
			{
				*destl++ = *sourcel++;  // RGBA
			}
			sourcel += linedelta;
		}

		// Get rectangles to chop into
		numrects = ChopImage( w, h, coords );

		bf = bframes;
		for ( i = 0; i < numrects; i++, bf++ )
		{
			// Copy section of image to buffer
			sourcel = (unsigned long *) longimage + ( coords[i].y * w ) + coords[i].x;
			destl = bufferl;
			linedelta = w - coords[i].w;

			for ( y = 0; y < coords[i].h; y++ )
			{
				for ( x = 0; x < coords[i].w; x++ )
				{
					*destl++ = *sourcel++;
				}
				sourcel += linedelta;
			}

			qtex32 = CreateBook32( bufferl, coords[i].w, coords[i].h, &size );

			qtex32->flags = flags;
			qtex32->contents = contents;
			qtex32->value = value;
			qtex32->scale_x = scale_x;
			qtex32->scale_y = scale_y;

			sprintf( filename, "%sbook/%s/%s_%s.m32", gamedir, book_prefix, lumpname, coords[i].name );
			sprintf( qtex32->name, "%s/%s_%s.m32", book_prefix, lumpname, coords[i].name );

			strcpy( bf->name, qtex32->name );
			bf->x = coords[i].x;
			bf->y = coords[i].y;
			bf->w = coords[i].w;
			bf->h = coords[i].h;
			//
			// write it out
			//
			printf( "writing %s\n", filename );
			SaveFile( filename, (byte *)qtex32, size );

			free( qtex32 );
		}
	}
	else
	{
		xh = xl + w;
		yh = yl + h;

		if ( xl >= byteimagewidth || xh > byteimagewidth ||
			 yl >= byteimageheight || yh > byteimageheight ) {
			Error( "line %i: bad clip dimmensions (%d,%d) (%d,%d) > image (%d,%d)", scriptline, xl,yl,w,h,byteimagewidth,byteimageheight );
		}

		// Copy image to top left
		source = byteimage + yl * byteimagewidth + xl;
		dest = byteimage;
		linedelta = byteimagewidth - w;

		for ( y = yl; y < yh; y++ )
		{
			for ( x = xl; x < xh; x++ )
			{
				*dest++ = *source++;
			}
			source += linedelta;
		}

		// Get rectangles to chop into
		numrects = ChopImage( w, h, coords );

		bf = bframes;
		for ( i = 0; i < numrects; i++, bf++ )
		{
			// Copy section of image to buffer
			source = byteimage + ( coords[i].y * w ) + coords[i].x;
			dest = buffer;
			linedelta = w - coords[i].w;

			for ( y = 0; y < coords[i].h; y++ )
			{
				for ( x = 0; x < coords[i].w; x++ )
				{
					*dest++ = *source++;
				}
				source += linedelta;
			}

			qtex = CreateBook8( buffer, coords[i].w, coords[i].h, lbmpalette, &size );

			qtex->flags = flags;
			qtex->contents = contents;
			qtex->value = value;

			sprintf( filename, "%sbook/%s/%s_%s.m8", gamedir, book_prefix, lumpname, coords[i].name );
			sprintf( qtex->name, "%s/%s_%s.m8", book_prefix, lumpname, coords[i].name );

			strcpy( bf->name, qtex->name );
			bf->x = coords[i].x;
			bf->y = coords[i].y;
			bf->w = coords[i].w;
			bf->h = coords[i].h;
			//
			// write it out
			//
			printf( "writing %s\n", filename );
			SaveFile( filename, (byte *)qtex, size );

			free( qtex );
		}
	}
	// Set up descriptor
	size = sizeof( bookframe_t ) * numrects;

	book.bheader.ident = IDBOOKHEADER;
	book.bheader.version = BOOK_VERSION;
	book.bheader.num_segments = numrects;
	book.bheader.total_w = w;
	book.bheader.total_h = h;
	memcpy( book.bframes, bframes, size );

	// Save out segment descriptor
	sprintf( filename, "%sBook/%s/%s.bk", gamedir, book_prefix, lumpname );
	printf( "writing %s\n", filename );
	SaveFile( filename, (byte *)&book, size + sizeof( bookheader_t ) );
}

/*
   ===============
   Cmd_picdir
   ===============
 */
void Cmd_Bookdir( void ){
	char filename[1024];

	GetScriptToken( false );
	strcpy( book_prefix, token );
	// create the directory if needed
	sprintf( filename, "%sBook", gamedir );
	Q_mkdir( filename );
	sprintf( filename, "%sBook/%s", gamedir, book_prefix );
	Q_mkdir( filename );
}

// end
