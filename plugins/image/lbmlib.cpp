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

// lbmlib.c

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "image.h"
#include "lbmlib.h"
#include "bmp.h"

#define LittleLong( a ) GINT32_FROM_LE( a )
#define LittleShort( a ) GINT16_FROM_LE( a )

#include <stdio.h>

#define Sys_Printf g_FuncTable.m_pfnSysPrintf
#define Sys_FPrintf g_FuncTable.m_pfnSysFPrintf

/*
   ============================================================================

   LOAD PCX

   ============================================================================
 */
typedef struct
{
	char manufacturer;
	char version;
	char encoding;
	char bits_per_pixel;
	unsigned short xmin, ymin, xmax, ymax;
	unsigned short hres, vres;
	unsigned char palette[48];
	char reserved;
	char color_planes;
	unsigned short bytes_per_line;
	unsigned short palette_type;
	char filler[58];
	unsigned char data;     // unbounded
} pcx_t;

/*
   ============================================================================

   TARGA IMAGE

   ============================================================================
 */
typedef struct _TargaHeader
{
	unsigned char id_length, colormap_type, image_type;
	unsigned short colormap_index, colormap_length;
	unsigned char colormap_size;
	unsigned short x_origin, y_origin, width, height;
	unsigned char pixel_size, attributes;
} TargaHeader;

/*
   =========================================================

   BMP LOADING

   =========================================================
 */
typedef struct
{
	char id[2];
	unsigned long fileSize;
	unsigned long reserved0;
	unsigned long bitmapDataOffset;
	unsigned long bitmapHeaderSize;
	unsigned long width;
	unsigned long height;
	unsigned short planes;
	unsigned short bitsPerPixel;
	unsigned long compression;
	unsigned long bitmapDataSize;
	unsigned long hRes;
	unsigned long vRes;
	unsigned long colors;
	unsigned long importantColors;
	unsigned char palette[256][4];
} BMPHeader_t;

static void LoadBMP( const char *name, byte ** pic, int *width, int *height ){
	int columns, rows, numPixels;
	byte *pixbuf;
	int row, column;
	byte *buf_p;
	byte *buffer;
	unsigned int length;
	BMPHeader_t bmpHeader;
	byte *bmpRGBA;

	*pic = NULL;

	//
	// load the file
	//
	length = vfsLoadFile( (char *)name, (void **)&buffer, 0 );
	if ( length == (unsigned int) -1 ) {
		return;
	}

	buf_p = buffer;

	bmpHeader.id[0] = *buf_p++;
	bmpHeader.id[1] = *buf_p++;
	bmpHeader.fileSize = LittleLong( *(long *) buf_p );
	buf_p += 4;
	bmpHeader.reserved0 = LittleLong( *(long *) buf_p );
	buf_p += 4;
	bmpHeader.bitmapDataOffset = LittleLong( *(long *) buf_p );
	buf_p += 4;
	bmpHeader.bitmapHeaderSize = LittleLong( *(long *) buf_p );
	buf_p += 4;
	bmpHeader.width = LittleLong( *(long *) buf_p );
	buf_p += 4;
	bmpHeader.height = LittleLong( *(long *) buf_p );
	buf_p += 4;
	bmpHeader.planes = LittleShort( *(short *) buf_p );
	buf_p += 2;
	bmpHeader.bitsPerPixel = LittleShort( *(short *) buf_p );
	buf_p += 2;
	bmpHeader.compression = LittleLong( *(long *) buf_p );
	buf_p += 4;
	bmpHeader.bitmapDataSize = LittleLong( *(long *) buf_p );
	buf_p += 4;
	bmpHeader.hRes = LittleLong( *(long *) buf_p );
	buf_p += 4;
	bmpHeader.vRes = LittleLong( *(long *) buf_p );
	buf_p += 4;
	bmpHeader.colors = LittleLong( *(long *) buf_p );
	buf_p += 4;
	bmpHeader.importantColors = LittleLong( *(long *) buf_p );
	buf_p += 4;

	memcpy( bmpHeader.palette, buf_p, sizeof( bmpHeader.palette ) );

	if ( bmpHeader.bitsPerPixel == 8 ) {
		buf_p += 1024;
	}

	if ( bmpHeader.id[0] != 'B' && bmpHeader.id[1] != 'M' ) {
		Sys_Printf( "LoadBMP: only Windows-style BMP files supported (%s)\n", name );
		return;
	}
	if ( bmpHeader.fileSize != length ) {
		Sys_Printf( "LoadBMP: header size does not match file size (%d vs. %d) (%s)\n",
					bmpHeader.fileSize, length, name );
		return;
	}
	if ( bmpHeader.compression != 0 ) {
		Sys_Printf( "LoadBMP: only uncompressed BMP files supported (%s)\n", name );
		return;
	}
	if ( bmpHeader.bitsPerPixel < 8 ) {
		Sys_Printf( "LoadBMP: monochrome and 4-bit BMP files not supported (%s)\n", name );
		return;
	}

	columns = bmpHeader.width;
	rows = bmpHeader.height;
	if ( rows < 0 ) {
		rows = -rows;
	}
	numPixels = columns * rows;

	if ( width ) {
		*width = columns;
	}
	if ( height ) {
		*height = rows;
	}

	bmpRGBA = reinterpret_cast < unsigned char *>( g_malloc( numPixels * 4 ) );
	*pic = bmpRGBA;


	for ( row = rows - 1; row >= 0; row-- )
	{
		pixbuf = bmpRGBA + row * columns * 4;

		for ( column = 0; column < columns; column++ )
		{
			unsigned char red, green, blue, alpha;
			int palIndex;
			unsigned short shortPixel;

			switch ( bmpHeader.bitsPerPixel )
			{
			case 8:
				palIndex = *buf_p++;
				*pixbuf++ = bmpHeader.palette[palIndex][2];
				*pixbuf++ = bmpHeader.palette[palIndex][1];
				*pixbuf++ = bmpHeader.palette[palIndex][0];
				*pixbuf++ = 0xff;
				break;
			case 16:
				shortPixel = *(unsigned short *) pixbuf;
				pixbuf += 2;
				*pixbuf++ = ( shortPixel & ( 31 << 10 ) ) >> 7;
				*pixbuf++ = ( shortPixel & ( 31 << 5 ) ) >> 2;
				*pixbuf++ = ( shortPixel & ( 31 ) ) << 3;
				*pixbuf++ = 0xff;
				break;
			case 24:
				blue = *buf_p++;
				green = *buf_p++;
				red = *buf_p++;
				*pixbuf++ = red;
				*pixbuf++ = green;
				*pixbuf++ = blue;
				*pixbuf++ = 255;
				break;
			case 32:
				blue = *buf_p++;
				green = *buf_p++;
				red = *buf_p++;
				alpha = *buf_p++;
				*pixbuf++ = red;
				*pixbuf++ = green;
				*pixbuf++ = blue;
				*pixbuf++ = alpha;
				break;
			default:
				Sys_Printf( "LoadBMP: illegal pixel_size '%d' in file '%s'\n", bmpHeader.bitsPerPixel,
							name );
				g_free( *pic );
				*pic = NULL;
				return;
				break;
			}
		}
	}

	vfsFreeFile( buffer );

}


/*
   =================================================================

   PCX LOADING

   =================================================================
 */


/*
   ==============
   LoadPCX
   ==============
 */

/* RR2DO2 */
#define DECODEPCX( b, d, r ) d = *b++; if ( ( d & 0xC0 ) == 0xC0 ) {r = d & 0x3F; d = *b++; }else{r = 1; }

static void LoadPCX( const char *filename, byte **pic, byte **palette, int *width, int *height ){
	byte  *raw;
	pcx_t *pcx;
	int x, y, lsize;
	int len;
	int dataByte, runLength;
	byte  *out, *pix;


	/* load the file */
	len = vfsLoadFile( filename, (void **)&raw, 0 );
	if ( len == -1 ) {
		Error( "LoadPCX: Couldn't read %s", filename );
	}


	/* parse the PCX file */
	pcx = (pcx_t *)raw;
	raw = &pcx->data;

	pcx->xmin = LittleShort( pcx->xmin );
	pcx->ymin = LittleShort( pcx->ymin );
	pcx->xmax = LittleShort( pcx->xmax );
	pcx->ymax = LittleShort( pcx->ymax );
	pcx->hres = LittleShort( pcx->hres );
	pcx->vres = LittleShort( pcx->vres );
	pcx->bytes_per_line = LittleShort( pcx->bytes_per_line );
	pcx->palette_type = LittleShort( pcx->palette_type );

	if ( pcx->manufacturer != 0x0a
		 || pcx->version != 5
		 || pcx->encoding != 1
		 || pcx->bits_per_pixel != 8
		 || pcx->xmax >= 640
		 || pcx->ymax >= 480 ) {
		Error( "Bad pcx file %s", filename );
	}

	if ( palette ) {
		*palette = (byte *)g_malloc( 768 );
		memcpy( *palette, (byte *)pcx + len - 768, 768 );
	}

	if ( width ) {
		*width = pcx->xmax + 1;
	}
	if ( height ) {
		*height = pcx->ymax + 1;
	}

	if ( !pic ) {
		return;
	}

	out = (byte *)g_malloc( ( pcx->ymax + 1 ) * ( pcx->xmax + 1 ) );
	if ( !out ) {
		Error( "LoadPCX: couldn't allocate" );
	}

	*pic = out;
	pix = out;

	/* RR2DO2: pcx fix  */
	lsize = pcx->color_planes * pcx->bytes_per_line;

	/* go scanline by scanline */
	for ( y = 0; y <= pcx->ymax; y++, pix += pcx->xmax + 1 )
	{
		/* do a scanline */
		for ( x = 0; x <= pcx->xmax; )
		{
			/* RR2DO2 */
			DECODEPCX( raw, dataByte, runLength );
			while ( runLength-- > 0 )
				pix[ x++ ] = dataByte;
		}

		/* RR2DO2: discard any other data */
		while ( x < lsize )
		{
			DECODEPCX( raw, dataByte, runLength );
			x++;
		}
		while ( runLength-- > 0 )
			x++;
	}

	/* validity check */
	if ( raw - (byte *) pcx > len ) {
		Error( "PCX file %s was malformed", filename );
	}
	g_free( pcx );
}

/*
   ==============
   LoadPCX32
   ==============
 */
static void LoadPCX32( const char *filename, byte ** pic, int *width, int *height ){
	byte *palette;
	byte *pic8;
	int i, c, p;
	byte *pic32;

	LoadPCX( filename, &pic8, &palette, width, height );
	if ( !pic8 ) {
		*pic = NULL;
		return;
	}

	c = ( *width ) * ( *height );
	pic32 = *pic = reinterpret_cast < unsigned char *>( g_malloc( 4 * c ) );
	for ( i = 0; i < c; i++ )
	{
		p = pic8[i];
		pic32[0] = palette[p * 3];
		pic32[1] = palette[p * 3 + 1];
		pic32[2] = palette[p * 3 + 2];
		pic32[3] = 255;
		pic32 += 4;
	}

	g_free( pic8 );
	g_free( palette );
}

/*
   =========================================================

   TARGA LOADING

   TTimo: added code to get rid of alphachannel from prefs or ignore it if completely empty
    was required since Radiant is using alpha channel when binding the textures for proper curry operation
    can be fully turned off from the prefs though
   =========================================================
 */

/*
   =============
   LoadTGA
   =============
 */
void LoadTGA( const char *name, byte ** pic, int *width, int *height ){
	int columns, rows, numPixels;
	byte *pixbuf;
	int row, column;
	byte *buf_p;
	byte *buffer;
	TargaHeader targa_header;
	byte *targa_rgba;

	*pic = NULL;

	//
	// load the file
	//
	int nLen = vfsLoadFile( (char *)name, (void **)&buffer, 0 );
	if ( nLen == -1 ) {
		return;
	}

	buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;

	targa_header.colormap_index = LittleShort( *(short *) buf_p );
	buf_p += 2;
	targa_header.colormap_length = LittleShort( *(short *) buf_p );
	buf_p += 2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = LittleShort( *(short *) buf_p );
	buf_p += 2;
	targa_header.y_origin = LittleShort( *(short *) buf_p );
	buf_p += 2;
	targa_header.width = LittleShort( *(short *) buf_p );
	buf_p += 2;
	targa_header.height = LittleShort( *(short *) buf_p );
	buf_p += 2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	bool bAlphaOK = false;

	if ( targa_header.image_type != 2 && targa_header.image_type != 10
		 && targa_header.image_type != 3 && targa_header.image_type != 11 ) {
		Sys_Printf( "LoadTGA: TGA type %d not supported\n", targa_header.image_type );
		Sys_Printf( "LoadTGA: Only type 2 (RGB), 3 (gray), 10 (RGB), and 11 (gray) TGA images supported\n" );
		return;
	}

	if ( targa_header.colormap_type != 0 ) {
		Sys_Printf( "LoadTGA: colormaps not supported\n" );
		return;
	}

	if ( ( ( targa_header.image_type == 2 || targa_header.image_type == 10 ) &&
	         targa_header.pixel_size != 32 && targa_header.pixel_size != 24 ) ||
	     ( ( targa_header.image_type == 3 || targa_header.image_type == 11 ) &&
	         targa_header.pixel_size != 8 ) ) {
		Sys_Printf( "LoadTGA: Only 32, 24 or 8 bit images supported (no colormaps)\n" );
		return;
	}

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if ( width ) {
		*width = columns;
	}
	if ( height ) {
		*height = rows;
	}

	targa_rgba = reinterpret_cast < unsigned char *>( g_malloc( numPixels * 4 ) );
	*pic = targa_rgba;

	if ( targa_header.id_length != 0 ) {
		buf_p += targa_header.id_length; // skip TARGA image comment

	}
	if ( targa_header.image_type == 2 || targa_header.image_type == 3 ) {
		// Uncompressed RGB or gray scale image
		for ( row = rows - 1; row >= 0; row-- )
		{
			pixbuf = targa_rgba + row * columns * 4;
			for ( column = 0; column < columns; column++ )
			{
				unsigned char red, green, blue, alphabyte;
				switch ( targa_header.pixel_size )
				{
				case 8:
					blue = *buf_p++;
					green = blue;
					red = blue;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;

				case 24:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;
				case 32:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					alphabyte = *buf_p++;
					// detect if the whole alpha channel is 0
					if ( alphabyte != 0 ) {
						bAlphaOK = true;
					}
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = alphabyte;
					break;
				default:
					Sys_Printf( "LoadTGA: illegal pixel_size '%d' in file '%s'\n", targa_header.pixel_size,
								name );
					g_free( *pic );
					*pic = NULL;
					return;
					break;
				}
			}
		}

		if ( !bAlphaOK ) {
			if ( targa_header.pixel_size == 32 ) {
				Sys_FPrintf( SYS_WRN, "WARNING: %s has empty alpha channel\n", name );
			}
			// disable the alpha value
			for ( row = rows - 1; row >= 0; row-- )
			{
				pixbuf = targa_rgba + row * columns * 4;
				for ( column = 0; column < columns; column++ )
				{
					// 32 bit
					pixbuf += 3;
					*pixbuf++ = 255;
				}
			}
		}
	}
	else if ( targa_header.image_type == 10 || targa_header.image_type == 11 ) {
		// Runlength encoded RGB or gray scale images
		unsigned char red, green, blue, alphabyte, packetHeader, packetSize, j;

		red = 0;
		green = 0;
		blue = 0;
		alphabyte = 0xff;

		for ( row = rows - 1; row >= 0; row-- )
		{
			pixbuf = targa_rgba + row * columns * 4;
			for ( column = 0; column < columns; )
			{
				packetHeader = *buf_p++;
				packetSize = 1 + ( packetHeader & 0x7f );
				if ( packetHeader & 0x80 ) { // run-length packet
					switch ( targa_header.pixel_size )
					{
					case 8:
						blue = *buf_p++;
						green = blue;
						red = blue;
						alphabyte = 255;
						break;
					case 24:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = 255;
						break;
					case 32:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = *buf_p++;
						// detect if the whole alpha channel is 0
						if ( alphabyte != 0 ) {
							bAlphaOK = true;
						}
						break;
					default:
						Sys_Printf( "LoadTGA: illegal pixel_size '%d' in file '%s'\n", targa_header.pixel_size,
									name );
						g_free( *pic );
						*pic = NULL;
						return;
						break;
					}

					for ( j = 0; j < packetSize; j++ )
					{
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = alphabyte;
						column++;
						if ( column == columns ) { // run spans across rows
							column = 0;
							if ( row > 0 ) {
								row--;
							}
							else{
								goto breakOut;
							}
							pixbuf = targa_rgba + row * columns * 4;
						}
					}
				}
				else
				{   // non run-length packet
					for ( j = 0; j < packetSize; j++ )
					{
						switch ( targa_header.pixel_size )
						{
						case 8:
							blue = *buf_p++;
							green = blue;
							red = blue;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
						case 24:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
						case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							// detect if the whole alpha channel is 0
							if ( alphabyte != 0 ) {
								bAlphaOK = true;
							}
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							break;
						default:
							Sys_Printf( "LoadTGA: illegal pixel_size '%d' in file '%s'\n",
										targa_header.pixel_size, name );
							g_free( *pic );
							*pic = NULL;
							return;
							break;
						}
						column++;
						if ( column == columns ) { // pixel packet run spans across rows
							column = 0;
							if ( row > 0 ) {
								row--;
							}
							else{
								goto breakOut;
							}
							pixbuf = targa_rgba + row * columns * 4;
						}
					}
				}
			}
breakOut:;
		}

		if ( !bAlphaOK ) {
			if ( targa_header.pixel_size == 32 ) {
				Sys_FPrintf( SYS_WRN, "WARNING: %s has empty alpha channel\n", name );
			}
			// disable the alpha value
			for ( row = rows - 1; row >= 0; row-- )
			{
				pixbuf = targa_rgba + row * columns * 4;
				for ( column = 0; column < columns; column++ )
				{
					// 32 bit
					pixbuf += 3;
					*pixbuf++ = 255;
				}
			}
		}

	}

	// vertically flipped
	if ( ( targa_header.attributes & ( 1 << 5 ) ) ) {
		int flip;
		for ( row = 0; row < .5f * rows; row++ )
		{
			for ( column = 0; column < columns; column++ )
			{
				flip = *( (int*)targa_rgba + row * columns + column );
				*( (int*)targa_rgba + row * columns + column ) = *( (int*)targa_rgba + ( ( rows - 1 ) - row ) * columns + column );
				*( (int*)targa_rgba + ( ( rows - 1 ) - row ) * columns + column ) = flip;
			}
		}
	}

	vfsFreeFile( buffer );
}

//===================================================================

/*
   =================
   LoadImage

   Loads any of the supported image types into a cannonical
   32 bit format.
   =================
 */
void LoadImage( const char *name, byte ** pic, int *width, int *height ){
	int len;
	*pic = NULL;
	*width = 0;
	*height = 0;

	len = strlen( name );
	if ( len < 5 ) {
		return;
	}

	if ( vfsGetFileCount( name, 0 ) == 0 ) {
		return;
	}

	if ( !g_ascii_strcasecmp( name + len - 4, ".tga" ) ) {
		LoadTGA( name, pic, width, height );
	}
	else if ( !g_ascii_strcasecmp( name + len - 4, ".pcx" ) ) {
		LoadPCX32( name, pic, width, height );
	}
	else if ( !g_ascii_strcasecmp( name + len - 4, ".bmp" ) ) {
		LoadBMP( name, pic, width, height );
	}
	/*
	   else if (!g_ascii_strcasecmp (name + len - 4, ".jpg"))
	   {
	   LoadJPG (name, pic, width, height);
	   }
	 */
}
