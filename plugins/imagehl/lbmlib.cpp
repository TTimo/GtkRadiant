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

// by Hydra - hydra@hydras-world.com
//
// This module is based on the image module, but just more stripped down.
// it still currently supports TGA file loading, even though this is not
// required for HalfLife support (unless MD2 files use TGA's)
//
// use the #defines in imagehl.h to enable/disable the various formats.
//
// HLW = Half-Life-WAD, I don't know if the actual in data in the WAD files
// has it's own name, so I'm just calling the individal textures .HLW files :)
//
// Thanks to the guys that made Wally for releasing an example WAD loader.
// without it this would not have been possible.

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "imagehl.h"
#include "lbmlib.h"

#include <stdio.h>

#define Sys_Printf g_FuncTable.m_pfnSysPrintf
#define Sys_FPrintf g_FuncTable.m_pfnSysFPrintf


#ifdef USE_IDSP
/*
   ============================================================================

   IDSP IMAGE (.spr files)

   Some code copied straight from the Q1 source, also used the HalfLife SDK as
   a reference.

   ============================================================================
 */

typedef enum {ST_SYNC = 0, ST_RAND } synctype_t;
typedef enum { SPR_SINGLE = 0, SPR_GROUP } spriteframetype_t;

typedef struct dspriteheader_s {
	int ident;
	int version;
} dspriteheader_t;

// Quake1
typedef struct {
	int type;
	float boundingradius;
	int width;
	int height;
	int numframes;
	float beamlength;
	synctype_t synctype;
} dspritev1_t;

// Halflife
typedef struct {
	int type;
	int texFormat;
	float boundingradius;
	int width;
	int height;
	int numframes;
	float beamlength;
	synctype_t synctype;
} dspritev2_t;

typedef struct {
	int origin[2];
	int width;
	int height;
} dspriteframe_t;

typedef struct {
	short type;
} dspriteframetype_t;

/*
   typedef struct {
   byte rgb[256][3];
   } dpalette_t;
 */

#define IDSPRITEHEADER  ( ( 'P' << 24 ) + ( 'S' << 16 ) + ( 'D' << 8 ) + 'I' )
// little-endian "IDSP"

/*
   =============
   LoadIDSP
   =============
 */

static void LoadIDSP( const char *name, byte ** pic, int *width, int *height ){
	byte *buffer;
	byte *buf_p;
	unsigned int length;
	int columns, rows, numPixels;
	byte *pixbuf;

	int row, column;
	byte *bmpRGBA;
	byte *palette;
	unsigned char red, green, blue, alphabyte;

	dspriteheader_t *header;
	dspritev1_t         *pinv1;
	dspritev2_t         *pinv2;
	dspriteframetype_t  *pframetype;
	int version;
	int numframes;
	dspriteframe_t *spriteframe;

	*pic = NULL;

	//
	// load the file
	//
	length = vfsLoadFile( (char *) name, (void **) &buffer, 0 );
	if ( length == (unsigned int) -1 ) {
		return;
	}

	header = (dspriteheader_t *)buffer;

	if ( header->ident != IDSPRITEHEADER ) {
		Sys_FPrintf( SYS_WRN, "WARNING: %s has wrong header\n" );
		vfsFreeFile( buffer );
		return;
	}

	version = header->version;
	if ( version != 1 && version != 2 ) {
		Sys_FPrintf( SYS_WRN, "WARNING: %s has wrong version number "
					"(%i should be 1 or 2)\n", name, version );
		vfsFreeFile( buffer );
		return;
	}

	// initialise variables depending on the sprite version.
	switch ( version )
	{
	case 1:
		pinv1 = (dspritev1_t *)( header + 1 );
		numframes = pinv1->numframes;
		columns = pinv1->width;
		rows = pinv1->height;
		pframetype = (dspriteframetype_t *)( pinv1 + 1 );
		break;
	case 2:
		pinv2 = (dspritev2_t *)( header + 1 );
		numframes = pinv2->numframes;
		columns = pinv2->width;
		rows = pinv2->height;
		pframetype = (dspriteframetype_t *)( pinv2 + 1 );
		break;
	}
	if ( numframes > 1 ) {
		Sys_FPrintf( SYS_WRN, "WARNING: %s has multiple frames, only the first frame will be used.\n", name );
	}

	// palette = buffer+mipdatasize+2;
	// buf_p = buffer+lpMip->offsets[0];

	numPixels = columns * rows;

	if ( width ) {
		*width = columns;
	}
	if ( height ) {
		*height = rows;
	}

	bmpRGBA = reinterpret_cast < unsigned char *>( g_malloc( numPixels * 4 ) );
	*pic = bmpRGBA;

#ifdef DEBUG
	spriteframetype_t frametype = spriteframetype_t( LittleLong( pframetype->type ) );
	if ( frametype == SPR_SINGLE ) {
		Sys_Printf( "Single Frame\n" );
	}
	else if ( frametype == SPR_GROUP ) {
		Sys_Printf( "Group of Frames\n" );
	}
	else
	{
		Sys_Printf( "Bleh!\n" ); // <-- we always get this, wtf!
	}
#endif

	palette = (byte *)( pframetype + 1 );
	spriteframe = (dspriteframe_t *)( palette + ( 256 * 3 ) + 4 ); // what are those 4 extra bytes ? what's missing ?
	buf_p = (byte *)( spriteframe + 1 );

	int temp;

	temp = buf_p - buffer;

	for ( row = 0; row < rows; row++ )
	{
		pixbuf = bmpRGBA + row * columns * 4;

		for ( column = 0; column < columns; column++ )
		{
			int palIndex;

			palIndex = *buf_p++;

			red = *( palette + ( palIndex * 3 ) );
			green = *( palette + ( palIndex * 3 ) + 1 );
			blue = *( palette + ( palIndex * 3 ) + 2 );

			// HalfLife engine makes pixels that are BLUE transparent. (RGB = 0x0000FF)
			// So show them that way in the editor.
			if ( blue == 0xff && red == 0x00 && green == 0x00 ) {
				alphabyte = 0xff; //FIXME: backwards? (so sprite models to render correctly)
				blue = 0x00; // don't set the resulting pixel to blue
			}
			else
			{
				alphabyte = 0x00; //FIXME: backwards? (so sprite models to render correctly)
			}

			*pixbuf++ = red;
			*pixbuf++ = green;
			*pixbuf++ = blue;

			*pixbuf++ = alphabyte;
		}
	}

	vfsFreeFile( buffer );
}
#endif

#ifdef USE_HLW
/*
   ============================================================================

   HLW IMAGE

   HalfLife WAD files contain files that look like this:

    Mip section
        First mip
            Mip header
            First mip (width * height)
            Second mip (width * height / 4)
            Third mip (width * height / 16)
            Fourth mip (width * height / 64)
            Palette size (WORD)
            Palette (Palette size * 3)
            Padding (WORD)

   ============================================================================
 */

#define GET_MIP_DATA_SIZE( WIDTH, HEIGHT ) ( sizeof( WAD3_MIP ) + ( WIDTH * HEIGHT ) + ( WIDTH * HEIGHT / 4 ) + ( WIDTH * HEIGHT / 16 ) + ( WIDTH * HEIGHT / 64 ) )

typedef struct
{
	char name[16];
	DWORD width, height;
	DWORD offsets[4];           // four mip maps stored
} WAD3_MIP, *LPWAD3_MIP;

/*
   =========================================================

   HLW LOADING

   Hydra: this code isn't bullet proof and probably won't
   like corrupt WAD files, but it works for now.

   TODO: make it more robust.
   =========================================================
 */

/*
   =============
   LoadHLW
   =============
 */

static void LoadHLW( const char *name, byte ** pic, int *width, int *height ){
	byte *buffer;
	byte *buf_p;
	unsigned int length;
	unsigned long mipdatasize;
	int columns, rows, numPixels;
	byte *pixbuf;
	int row, column;
	byte *bmpRGBA;
	byte *palette;
	LPWAD3_MIP lpMip;
	unsigned char red, green, blue, alphabyte;

	*pic = NULL;

	//
	// load the file
	//
	length = vfsLoadFile( (char *) name, (void **) &buffer, 0 );
	if ( length == (unsigned int) -1 ) {
		return;
	}

	lpMip = (LPWAD3_MIP)buffer;

	mipdatasize = GET_MIP_DATA_SIZE( lpMip->width,lpMip->height );

	palette = buffer + mipdatasize + 2;

	buf_p = buffer + lpMip->offsets[0];

	columns = lpMip->width;
	rows = lpMip->height;
	numPixels = columns * rows;

	if ( width ) {
		*width = columns;
	}
	if ( height ) {
		*height = rows;
	}

	bmpRGBA = reinterpret_cast < unsigned char *>( g_malloc( numPixels * 4 ) );
	*pic = bmpRGBA;

	for ( row = 0; row < rows; row++ )
	{
		pixbuf = bmpRGBA + row * columns * 4;

		for ( column = 0; column < columns; column++ )
		{
			int palIndex;

			palIndex = *buf_p++;

			red = *( palette + ( palIndex * 3 ) );
			green = *( palette + ( palIndex * 3 ) + 1 );
			blue = *( palette + ( palIndex * 3 ) + 2 );

			// HalfLife engine makes pixels that are BLUE transparent.
			// So show them that way in the editor.
			if ( blue == 0xff && red == 0x00 && green == 0x00 ) {
				alphabyte = 0x00;
				blue = 0x00; // don't set the resulting pixel to blue
			}
			else
			{
				alphabyte = 0xff;
			}

			*pixbuf++ = red;
			*pixbuf++ = green;
			*pixbuf++ = blue;

			*pixbuf++ = alphabyte;
		}
	}

	vfsFreeFile( buffer );
}
#endif

#ifdef USE_MIP
/*
   ============================================================================

   MIP IMAGE

   Quake WAD files contain miptex files that look like this:

    Mip section
        First mip
            Mip header
            First mip (width * height)
            Second mip (width * height / 4)
            Third mip (width * height / 16)
            Fourth mip (width * height / 64)

   ============================================================================
 */

/*
   =========================================================

   MIP LOADING

   LordHavoc: this code is based on the HLW code above.
   =========================================================
 */


static const byte quakepalette[768] =
{
	0x00,0x00,0x00, 0x0f,0x0f,0x0f, 0x1f,0x1f,0x1f, 0x2f,0x2f,0x2f,
	0x3f,0x3f,0x3f, 0x4b,0x4b,0x4b, 0x5b,0x5b,0x5b, 0x6b,0x6b,0x6b,
	0x7b,0x7b,0x7b, 0x8b,0x8b,0x8b, 0x9b,0x9b,0x9b, 0xab,0xab,0xab,
	0xbb,0xbb,0xbb, 0xcb,0xcb,0xcb, 0xdb,0xdb,0xdb, 0xeb,0xeb,0xeb,
	0x0f,0x0b,0x07, 0x17,0x0f,0x0b, 0x1f,0x17,0x0b, 0x27,0x1b,0x0f,
	0x2f,0x23,0x13, 0x37,0x2b,0x17, 0x3f,0x2f,0x17, 0x4b,0x37,0x1b,
	0x53,0x3b,0x1b, 0x5b,0x43,0x1f, 0x63,0x4b,0x1f, 0x6b,0x53,0x1f,
	0x73,0x57,0x1f, 0x7b,0x5f,0x23, 0x83,0x67,0x23, 0x8f,0x6f,0x23,
	0x0b,0x0b,0x0f, 0x13,0x13,0x1b, 0x1b,0x1b,0x27, 0x27,0x27,0x33,
	0x2f,0x2f,0x3f, 0x37,0x37,0x4b, 0x3f,0x3f,0x57, 0x47,0x47,0x67,
	0x4f,0x4f,0x73, 0x5b,0x5b,0x7f, 0x63,0x63,0x8b, 0x6b,0x6b,0x97,
	0x73,0x73,0xa3, 0x7b,0x7b,0xaf, 0x83,0x83,0xbb, 0x8b,0x8b,0xcb,
	0x00,0x00,0x00, 0x07,0x07,0x00, 0x0b,0x0b,0x00, 0x13,0x13,0x00,
	0x1b,0x1b,0x00, 0x23,0x23,0x00, 0x2b,0x2b,0x07, 0x2f,0x2f,0x07,
	0x37,0x37,0x07, 0x3f,0x3f,0x07, 0x47,0x47,0x07, 0x4b,0x4b,0x0b,
	0x53,0x53,0x0b, 0x5b,0x5b,0x0b, 0x63,0x63,0x0b, 0x6b,0x6b,0x0f,
	0x07,0x00,0x00, 0x0f,0x00,0x00, 0x17,0x00,0x00, 0x1f,0x00,0x00,
	0x27,0x00,0x00, 0x2f,0x00,0x00, 0x37,0x00,0x00, 0x3f,0x00,0x00,
	0x47,0x00,0x00, 0x4f,0x00,0x00, 0x57,0x00,0x00, 0x5f,0x00,0x00,
	0x67,0x00,0x00, 0x6f,0x00,0x00, 0x77,0x00,0x00, 0x7f,0x00,0x00,
	0x13,0x13,0x00, 0x1b,0x1b,0x00, 0x23,0x23,0x00, 0x2f,0x2b,0x00,
	0x37,0x2f,0x00, 0x43,0x37,0x00, 0x4b,0x3b,0x07, 0x57,0x43,0x07,
	0x5f,0x47,0x07, 0x6b,0x4b,0x0b, 0x77,0x53,0x0f, 0x83,0x57,0x13,
	0x8b,0x5b,0x13, 0x97,0x5f,0x1b, 0xa3,0x63,0x1f, 0xaf,0x67,0x23,
	0x23,0x13,0x07, 0x2f,0x17,0x0b, 0x3b,0x1f,0x0f, 0x4b,0x23,0x13,
	0x57,0x2b,0x17, 0x63,0x2f,0x1f, 0x73,0x37,0x23, 0x7f,0x3b,0x2b,
	0x8f,0x43,0x33, 0x9f,0x4f,0x33, 0xaf,0x63,0x2f, 0xbf,0x77,0x2f,
	0xcf,0x8f,0x2b, 0xdf,0xab,0x27, 0xef,0xcb,0x1f, 0xff,0xf3,0x1b,
	0x0b,0x07,0x00, 0x1b,0x13,0x00, 0x2b,0x23,0x0f, 0x37,0x2b,0x13,
	0x47,0x33,0x1b, 0x53,0x37,0x23, 0x63,0x3f,0x2b, 0x6f,0x47,0x33,
	0x7f,0x53,0x3f, 0x8b,0x5f,0x47, 0x9b,0x6b,0x53, 0xa7,0x7b,0x5f,
	0xb7,0x87,0x6b, 0xc3,0x93,0x7b, 0xd3,0xa3,0x8b, 0xe3,0xb3,0x97,
	0xab,0x8b,0xa3, 0x9f,0x7f,0x97, 0x93,0x73,0x87, 0x8b,0x67,0x7b,
	0x7f,0x5b,0x6f, 0x77,0x53,0x63, 0x6b,0x4b,0x57, 0x5f,0x3f,0x4b,
	0x57,0x37,0x43, 0x4b,0x2f,0x37, 0x43,0x27,0x2f, 0x37,0x1f,0x23,
	0x2b,0x17,0x1b, 0x23,0x13,0x13, 0x17,0x0b,0x0b, 0x0f,0x07,0x07,
	0xbb,0x73,0x9f, 0xaf,0x6b,0x8f, 0xa3,0x5f,0x83, 0x97,0x57,0x77,
	0x8b,0x4f,0x6b, 0x7f,0x4b,0x5f, 0x73,0x43,0x53, 0x6b,0x3b,0x4b,
	0x5f,0x33,0x3f, 0x53,0x2b,0x37, 0x47,0x23,0x2b, 0x3b,0x1f,0x23,
	0x2f,0x17,0x1b, 0x23,0x13,0x13, 0x17,0x0b,0x0b, 0x0f,0x07,0x07,
	0xdb,0xc3,0xbb, 0xcb,0xb3,0xa7, 0xbf,0xa3,0x9b, 0xaf,0x97,0x8b,
	0xa3,0x87,0x7b, 0x97,0x7b,0x6f, 0x87,0x6f,0x5f, 0x7b,0x63,0x53,
	0x6b,0x57,0x47, 0x5f,0x4b,0x3b, 0x53,0x3f,0x33, 0x43,0x33,0x27,
	0x37,0x2b,0x1f, 0x27,0x1f,0x17, 0x1b,0x13,0x0f, 0x0f,0x0b,0x07,
	0x6f,0x83,0x7b, 0x67,0x7b,0x6f, 0x5f,0x73,0x67, 0x57,0x6b,0x5f,
	0x4f,0x63,0x57, 0x47,0x5b,0x4f, 0x3f,0x53,0x47, 0x37,0x4b,0x3f,
	0x2f,0x43,0x37, 0x2b,0x3b,0x2f, 0x23,0x33,0x27, 0x1f,0x2b,0x1f,
	0x17,0x23,0x17, 0x0f,0x1b,0x13, 0x0b,0x13,0x0b, 0x07,0x0b,0x07,
	0xff,0xf3,0x1b, 0xef,0xdf,0x17, 0xdb,0xcb,0x13, 0xcb,0xb7,0x0f,
	0xbb,0xa7,0x0f, 0xab,0x97,0x0b, 0x9b,0x83,0x07, 0x8b,0x73,0x07,
	0x7b,0x63,0x07, 0x6b,0x53,0x00, 0x5b,0x47,0x00, 0x4b,0x37,0x00,
	0x3b,0x2b,0x00, 0x2b,0x1f,0x00, 0x1b,0x0f,0x00, 0x0b,0x07,0x00,
	0x00,0x00,0xff, 0x0b,0x0b,0xef, 0x13,0x13,0xdf, 0x1b,0x1b,0xcf,
	0x23,0x23,0xbf, 0x2b,0x2b,0xaf, 0x2f,0x2f,0x9f, 0x2f,0x2f,0x8f,
	0x2f,0x2f,0x7f, 0x2f,0x2f,0x6f, 0x2f,0x2f,0x5f, 0x2b,0x2b,0x4f,
	0x23,0x23,0x3f, 0x1b,0x1b,0x2f, 0x13,0x13,0x1f, 0x0b,0x0b,0x0f,
	0x2b,0x00,0x00, 0x3b,0x00,0x00, 0x4b,0x07,0x00, 0x5f,0x07,0x00,
	0x6f,0x0f,0x00, 0x7f,0x17,0x07, 0x93,0x1f,0x07, 0xa3,0x27,0x0b,
	0xb7,0x33,0x0f, 0xc3,0x4b,0x1b, 0xcf,0x63,0x2b, 0xdb,0x7f,0x3b,
	0xe3,0x97,0x4f, 0xe7,0xab,0x5f, 0xef,0xbf,0x77, 0xf7,0xd3,0x8b,
	0xa7,0x7b,0x3b, 0xb7,0x9b,0x37, 0xc7,0xc3,0x37, 0xe7,0xe3,0x57,
	0x7f,0xbf,0xff, 0xab,0xe7,0xff, 0xd7,0xff,0xff, 0x67,0x00,0x00,
	0x8b,0x00,0x00, 0xb3,0x00,0x00, 0xd7,0x00,0x00, 0xff,0x00,0x00,
	0xff,0xf3,0x93, 0xff,0xf7,0xc7, 0xff,0xff,0xff, 0x9f,0x5b,0x53
};

/*
   =============
   LoadMIP
   =============
 */

static void LoadMIP( const char *name, byte ** pic, int *width, int *height ){
	byte *buffer;
	byte *buf_p;
	unsigned int length, palettelength;
	unsigned long mipdatasize;
	int columns, rows, numPixels;
	byte *pixbuf;
	int i;
	byte *bmpRGBA;
	byte *loadedpalette;
	const byte *palette;
	LPWAD3_MIP lpMip;

	*pic = NULL;
	loadedpalette = NULL;

	//
	// load the file
	//
	length = vfsLoadFile( (char *) name, (void **) &buffer, 0 );
	if ( length == (unsigned int) -1 ) {
		return;
	}

	lpMip = (LPWAD3_MIP)buffer;

	mipdatasize = GET_MIP_DATA_SIZE( lpMip->width,lpMip->height );

	palettelength = vfsLoadFile( "textures/palette.lmp", (void **) &loadedpalette, 0 );
	if ( palettelength == 768 ) {
		palette = loadedpalette;
	}
	else
	{
		loadedpalette = NULL;
		palette = quakepalette;
	}

	buf_p = buffer + lpMip->offsets[0];

	columns = lpMip->width;
	rows = lpMip->height;
	numPixels = columns * rows;

	if ( width ) {
		*width = columns;
	}
	if ( height ) {
		*height = rows;
	}

	//Sys_Printf("lpMip->width = %i, lpMip->height = %i, lpMip->offsets[0] = %i, lpMip->offsets[1] = %i, lpMip->offsets[2] = %i, lpMip->offsets[3] = %i, numPixels = %i\n", lpMip->width, lpMip->height, lpMip->offsets[0], lpMip->offsets[1], lpMip->offsets[2], lpMip->offsets[3], numPixels);
	//for (i = 0; i < sizeof(*lpMip); i++)
	//  Sys_Printf("%02x", (int) ((unsigned char *)lpMip)[i]);

	bmpRGBA = reinterpret_cast < unsigned char *>( g_malloc( numPixels * 4 ) );
	*pic = bmpRGBA;
	pixbuf = bmpRGBA;

	for ( i = 0; i < numPixels; i++ )
	{
		int palIndex = *buf_p++;
		*pixbuf++ = palette[palIndex * 3];
		*pixbuf++ = palette[palIndex * 3 + 1];
		*pixbuf++ = palette[palIndex * 3 + 2];
		*pixbuf++ = 0xff;
	}

	vfsFreeFile( buffer );
	if ( loadedpalette != NULL ) {
		vfsFreeFile( loadedpalette );
	}
}
#endif

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

#ifdef USE_HLW
	if ( *pic == NULL && !g_ascii_strcasecmp( name + len - 4, ".hlw" ) ) {
		LoadHLW( name, pic, width, height );
	}
#endif

#ifdef USE_MIP
	if ( *pic == NULL && !g_ascii_strcasecmp( name + len - 4, ".mip" ) ) {
		LoadMIP( name, pic, width, height );
	}
#endif

#ifdef USE_IDSP
	if ( *pic == NULL && !g_ascii_strcasecmp( name + len - 4, ".spr" ) ) {
		LoadIDSP( name, pic, width, height );
	}
#endif
}
