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

#include "cmdlib.h"
#include "inout.h"
#include "lbmlib.h"

// Ups the palette values so no pixels except 0 appear transparent
// Need a value of 8 to cater for 16bit renderers

typedef struct
{
	byte r;
	byte g;
	byte b;
} paletteRGB_t;


void CorrectPalette( byte *pal ){
	paletteRGB_t    *p;

	p = (paletteRGB_t *)pal;
	// Color 0 always transparent
	p->r = 0;
	p->g = 0;
	p->b = 0;
}

/*
   ============================================================================

                        LBM STUFF

   ============================================================================
 */


typedef unsigned char UBYTE;
//conflicts with windows typedef short			WORD;
typedef unsigned short UWORD;
typedef long LONG;

typedef enum
{
	ms_none,
	ms_mask,
	ms_transcolor,
	ms_lasso
} mask_t;

typedef enum
{
	cm_none,
	cm_rle1
} compress_t;

typedef struct
{
	UWORD w,h;
	short x,y;
	UBYTE nPlanes;
	UBYTE masking;
	UBYTE compression;
	UBYTE pad1;
	UWORD transparentColor;
	UBYTE xAspect,yAspect;
	short pageWidth,pageHeight;
} bmhd_t;

extern bmhd_t bmhd;                         // will be in native byte order



#define FORMID ( 'F' + ( 'O' << 8 ) + ( (int)'R' << 16 ) + ( (int)'M' << 24 ) )
#define ILBMID ( 'I' + ( 'L' << 8 ) + ( (int)'B' << 16 ) + ( (int)'M' << 24 ) )
#define PBMID  ( 'P' + ( 'B' << 8 ) + ( (int)'M' << 16 ) + ( (int)' ' << 24 ) )
#define BMHDID ( 'B' + ( 'M' << 8 ) + ( (int)'H' << 16 ) + ( (int)'D' << 24 ) )
#define BODYID ( 'B' + ( 'O' << 8 ) + ( (int)'D' << 16 ) + ( (int)'Y' << 24 ) )
#define CMAPID ( 'C' + ( 'M' << 8 ) + ( (int)'A' << 16 ) + ( (int)'P' << 24 ) )


bmhd_t bmhd;

int    Align( int l ){
	if ( l & 1 ) {
		return l + 1;
	}
	return l;
}



/*
   ================
   LBMRLEdecompress

   Source must be evenly aligned!
   ================
 */
byte  *LBMRLEDecompress( byte *source,byte *unpacked, int bpwidth ){
	int count;
	byte b,rept;

	count = 0;

	do
	{
		rept = *source++;

		if ( rept > 0x80 ) {
			rept = ( rept ^ 0xff ) + 2;
			b = *source++;
			memset( unpacked,b,rept );
			unpacked += rept;
		}
		else if ( rept < 0x80 ) {
			rept++;
			memcpy( unpacked,source,rept );
			unpacked += rept;
			source += rept;
		}
		else{
			rept = 0;               // rept of 0x80 is NOP

		}
		count += rept;

	} while ( count < bpwidth );

	if ( count > bpwidth ) {
		Error( "Decompression exceeded width!\n" );
	}


	return source;
}


/*
   =================
   LoadLBM
   =================
 */
void LoadLBM( char *filename, byte **picture, byte **palette ){
	byte    *LBMbuffer, *picbuffer, *cmapbuffer;
	int y;
	byte    *LBM_P, *LBMEND_P;
	byte    *pic_p;
	byte    *body_p;

	int formtype,formlength;
	int chunktype,chunklength;

// qiet compiler warnings
	picbuffer = NULL;
	cmapbuffer = NULL;

//
// load the LBM
//
	LoadFile( filename, (void **)&LBMbuffer );

//
// parse the LBM header
//
	LBM_P = LBMbuffer;
	if ( *(int *)LBMbuffer != LittleLong( FORMID ) ) {
		Error( "No FORM ID at start of file!\n" );
	}

	LBM_P += 4;
	formlength = BigLong( *(int *)LBM_P );
	LBM_P += 4;
	LBMEND_P = LBM_P + Align( formlength );

	formtype = LittleLong( *(int *)LBM_P );

	if ( formtype != ILBMID && formtype != PBMID ) {
		Error( "Unrecognized form type: %c%c%c%c\n", formtype & 0xff
			   ,( formtype >> 8 ) & 0xff,( formtype >> 16 ) & 0xff,( formtype >> 24 ) & 0xff );
	}

	LBM_P += 4;

//
// parse chunks
//

	while ( LBM_P < LBMEND_P )
	{
		chunktype = LBM_P[0] + ( LBM_P[1] << 8 ) + ( LBM_P[2] << 16 ) + ( LBM_P[3] << 24 );
		LBM_P += 4;
		chunklength = LBM_P[3] + ( LBM_P[2] << 8 ) + ( LBM_P[1] << 16 ) + ( LBM_P[0] << 24 );
		LBM_P += 4;

		switch ( chunktype )
		{
		case BMHDID:
			memcpy( &bmhd,LBM_P,sizeof( bmhd ) );
			bmhd.w = BigShort( bmhd.w );
			bmhd.h = BigShort( bmhd.h );
			bmhd.x = BigShort( bmhd.x );
			bmhd.y = BigShort( bmhd.y );
			bmhd.pageWidth = BigShort( bmhd.pageWidth );
			bmhd.pageHeight = BigShort( bmhd.pageHeight );
			break;

		case CMAPID:
			cmapbuffer = malloc( 768 );
			memset( cmapbuffer, 0, 768 );
			memcpy( cmapbuffer, LBM_P, chunklength );
			CorrectPalette( cmapbuffer );
			break;

		case BODYID:
			body_p = LBM_P;

			pic_p = picbuffer = malloc( bmhd.w * bmhd.h );
			if ( formtype == PBMID ) {
				//
				// unpack PBM
				//
				for ( y = 0 ; y < bmhd.h ; y++, pic_p += bmhd.w )
				{
					if ( bmhd.compression == cm_rle1 ) {
						body_p = LBMRLEDecompress( (byte *)body_p
												   , pic_p, bmhd.w );
					}
					else if ( bmhd.compression == cm_none ) {
						memcpy( pic_p,body_p,bmhd.w );
						body_p += Align( bmhd.w );
					}
				}

			}
			else
			{
				//
				// unpack ILBM
				//
				Error( "%s is an interlaced LBM, not packed", filename );
			}
			break;
		}

		LBM_P += Align( chunklength );
	}

	free( LBMbuffer );

	*picture = picbuffer;

	if ( palette ) {
		*palette = cmapbuffer;
	}
}


/*
   ============================================================================

                            WRITE LBM

   ============================================================================
 */

/*
   ==============
   WriteLBMfile
   ==============
 */
void WriteLBMfile( char *filename, byte *data,
				   int width, int height, byte *palette ){
	byte    *lbm, *lbmptr;
	int    *formlength, *bmhdlength, *cmaplength, *bodylength;
	int length;
	bmhd_t basebmhd;

	lbm = lbmptr = malloc( width * height + 1000 );

//
// start FORM
//
	*lbmptr++ = 'F';
	*lbmptr++ = 'O';
	*lbmptr++ = 'R';
	*lbmptr++ = 'M';

	formlength = (int*)lbmptr;
	lbmptr += 4;                      // leave space for length

	*lbmptr++ = 'P';
	*lbmptr++ = 'B';
	*lbmptr++ = 'M';
	*lbmptr++ = ' ';

//
// write BMHD
//
	*lbmptr++ = 'B';
	*lbmptr++ = 'M';
	*lbmptr++ = 'H';
	*lbmptr++ = 'D';

	bmhdlength = (int *)lbmptr;
	lbmptr += 4;                      // leave space for length

	memset( &basebmhd,0,sizeof( basebmhd ) );
	basebmhd.w = BigShort( (short)width );
	basebmhd.h = BigShort( (short)height );
	basebmhd.nPlanes = BigShort( 8 );
	basebmhd.xAspect = BigShort( 5 );
	basebmhd.yAspect = BigShort( 6 );
	basebmhd.pageWidth = BigShort( (short)width );
	basebmhd.pageHeight = BigShort( (short)height );

	memcpy( lbmptr,&basebmhd,sizeof( basebmhd ) );
	lbmptr += sizeof( basebmhd );

	length = lbmptr - (byte *)bmhdlength - 4;
	*bmhdlength = BigLong( length );
	if ( length & 1 ) {
		*lbmptr++ = 0;          // pad chunk to even offset

	}
//
// write CMAP
//
	*lbmptr++ = 'C';
	*lbmptr++ = 'M';
	*lbmptr++ = 'A';
	*lbmptr++ = 'P';

	cmaplength = (int *)lbmptr;
	lbmptr += 4;                      // leave space for length

	memcpy( lbmptr,palette,768 );
	lbmptr += 768;

	length = lbmptr - (byte *)cmaplength - 4;
	*cmaplength = BigLong( length );
	if ( length & 1 ) {
		*lbmptr++ = 0;          // pad chunk to even offset

	}
//
// write BODY
//
	*lbmptr++ = 'B';
	*lbmptr++ = 'O';
	*lbmptr++ = 'D';
	*lbmptr++ = 'Y';

	bodylength = (int *)lbmptr;
	lbmptr += 4;                      // leave space for length

	memcpy( lbmptr,data,width * height );
	lbmptr += width * height;

	length = lbmptr - (byte *)bodylength - 4;
	*bodylength = BigLong( length );
	if ( length & 1 ) {
		*lbmptr++ = 0;          // pad chunk to even offset

	}
//
// done
//
	length = lbmptr - (byte *)formlength - 4;
	*formlength = BigLong( length );
	if ( length & 1 ) {
		*lbmptr++ = 0;          // pad chunk to even offset

	}
//
// write output file
//
	SaveFile( filename, lbm, lbmptr - lbm );
	free( lbm );
}


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
	unsigned short xmin,ymin,xmax,ymax;
	unsigned short hres,vres;
	unsigned char palette[48];
	char reserved;
	char color_planes;
	unsigned short bytes_per_line;
	unsigned short palette_type;
	char filler[58];
	unsigned char data;             // unbounded
} pcx_t;

/*
   ==============
   LoadPCX
   ==============
 */
void LoadPCX( char *filename, byte **pic, byte **palette, int *width, int *height ){
	byte    *raw;
	pcx_t   *pcx;
	int x, y;
	int len;
	int dataByte, runLength;
	byte    *out, *pix;

	//
	// load the file
	//
	len = LoadFile( filename, (void **)&raw );

	//
	// parse the PCX file
	//
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
		*palette = malloc( 768 );
		memcpy( *palette, (byte *)pcx + len - 768, 768 );
		CorrectPalette( *palette );
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

	out = malloc( ( pcx->ymax + 1 ) * ( pcx->xmax + 1 ) );
	if ( !out ) {
		Error( "Skin_Cache: couldn't allocate" );
	}

	*pic = out;

	pix = out;

	for ( y = 0 ; y <= pcx->ymax ; y++, pix += pcx->xmax + 1 )
	{
		for ( x = 0 ; x <= pcx->xmax ; )
		{
			dataByte = *raw++;

			if ( ( dataByte & 0xC0 ) == 0xC0 ) {
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			}
			else{
				runLength = 1;
			}

			while ( runLength-- > 0 )
				pix[x++] = dataByte;
		}

	}

	if ( raw - (byte *)pcx > len ) {
		Error( "PCX file %s was malformed", filename );
	}

	free( pcx );
}

/*
   ==============
   WritePCXfile
   ==============
 */

void StuffPackedByte( int curRepCount, byte curByte, byte** packPtr ){
	byte* pack;

	pack = *packPtr;

	while ( curRepCount > 0 )
	{
		if ( curRepCount == 1 ) {
			if ( ( curByte & 0xc0 ) != 0xc0 ) {
				*pack++ = curByte;
			}
			else
			{
				*pack++ = 0xc1;
				*pack++ = curByte;
			}
			break;
		}
		if ( curRepCount < 0x0040 ) {
			*pack++ = ( 0x00c0 | curRepCount );
			curRepCount = 0;
		}
		else
		{
			*pack++ = 0xff;
			curRepCount -= 0x003f;
		}
		*pack++ = curByte;
	}
	*packPtr = pack;
}

void WritePCXfile( char *filename, byte *data,
				   int width, int height, byte *palette ){
	int i, j, length;
	pcx_t       *pcx;
	byte        *pack;
	byte curByte;
	int curRepCount;

	pcx = malloc( width * height * 2 + 1000 );
	memset( pcx, 0, sizeof( *pcx ) );

	pcx->manufacturer = 0x0a;   // PCX id
	pcx->version = 5;           // 256 color
	pcx->encoding = 1;      // RLE
	pcx->bits_per_pixel = 8;        // 256 color
	pcx->xmin = 0;
	pcx->ymin = 0;
	pcx->xmax = LittleShort( (short)( width - 1 ) );
	pcx->ymax = LittleShort( (short)( height - 1 ) );
	pcx->hres = LittleShort( (short)width );
	pcx->vres = LittleShort( (short)height );
	pcx->color_planes = 1;      // chunky image
	pcx->bytes_per_line = LittleShort( (short)width );
	pcx->palette_type = LittleShort( 1 );     // not a grey scale

	// pack the image
	pack = &pcx->data;

/*	for (i=0 ; i<height ; i++)
    {
        for (j=0 ; j<width ; j++)
        {
            if ( (*data & 0xc0) != 0xc0)
   *pack++ = *data++;
            else
            {
   *pack++ = 0xc1;
   *pack++ = *data++;
            }
        }
    }
 */
	for ( i = 0 ; i < height ; i++ )
	{
		curByte = *data;
		curRepCount = 0;
		for ( j = 0 ; j < width ; j++ )
		{
			if ( *data == curByte ) {
				curRepCount++;
				data++;
				continue;
			}
			StuffPackedByte( curRepCount, curByte, &pack );
			curByte = *data++;
			curRepCount = 1;
		}
		StuffPackedByte( curRepCount, curByte, &pack );
	}
	// write the palette
	*pack++ = 0x0c; // palette ID byte
	for ( i = 0 ; i < 768 ; i++ )
		*pack++ = *palette++;

// write output file
	length = pack - (byte *)pcx;
	SaveFile( filename, pcx, length );

	free( pcx );
}


/*
   ============================================================================

   LOAD IMAGE

   ============================================================================
 */

/*
   ==============
   Load256Image

   Will load either an lbm or pcx, depending on extension.
   Any of the return pointers can be NULL if you don't want them.
   ==============
 */
void Load256Image( char *name, byte **pixels, byte **palette,
				   int *width, int *height ){
	char ext[128];

	ExtractFileExtension( name, ext );
	if ( !Q_strcasecmp( ext, "lbm" ) ) {
		LoadLBM( name, pixels, palette );
		if ( width ) {
			*width = bmhd.w;
		}
		if ( height ) {
			*height = bmhd.h;
		}
	}
	else if ( !Q_strcasecmp( ext, "pcx" ) ) {
		LoadPCX( name, pixels, palette, width, height );
	}
	else{
		Error( "%s doesn't have a known image extension", name );
	}
}


/*
   ==============
   Save256Image

   Will save either an lbm or pcx, depending on extension.
   ==============
 */
void Save256Image( char *name, byte *pixels, byte *palette,
				   int width, int height ){
	char ext[128];

	ExtractFileExtension( name, ext );
	if ( !Q_strcasecmp( ext, "lbm" ) ) {
		WriteLBMfile( name, pixels, width, height, palette );
	}
	else if ( !Q_strcasecmp( ext, "pcx" ) ) {
		WritePCXfile( name, pixels, width, height, palette );
	}
	else{
		Error( "%s doesn't have a known image extension", name );
	}
}




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

int fgetLittleShort( FILE *f ){
	byte b1, b2;

	b1 = fgetc( f );
	b2 = fgetc( f );

	return( (short)( b1 + ( b2 << 8 ) ) );
}

int fgetLittleLong( FILE *f ){
	byte b1, b2, b3, b4;

	b1 = fgetc( f );
	b2 = fgetc( f );
	b3 = fgetc( f );
	b4 = fgetc( f );

	return( b1 + ( b2 << 8 ) + ( b3 << 16 ) + ( b4 << 24 ) );
}


/*
   =============
   LoadTGA
   =============
 */
void LoadTGA( char *name, byte **pixels, int *width, int *height ){
	int columns, rows, numPixels;
	byte            *pixbuf;
	byte            *rowBuf;
	int row, column;
	FILE            *fin;
	byte            *targa_rgba;
	TargaHeader targa_header;
	unsigned char red, green, blue, alphabyte;
	unsigned char packetHeader, packetSize, j;
	int flip;
	int mirror;
	int rowOffset;
	int pixDirection;

	fin = fopen( name, "rb" );
	if ( !fin ) {
		Error( "Couldn't read %s", name );
	}

	targa_header.id_length = fgetc( fin );
	targa_header.colormap_type = fgetc( fin );
	targa_header.image_type = fgetc( fin );

	targa_header.colormap_index = fgetLittleShort( fin );
	targa_header.colormap_length = fgetLittleShort( fin );
	targa_header.colormap_size = fgetc( fin );
	targa_header.x_origin = fgetLittleShort( fin );
	targa_header.y_origin = fgetLittleShort( fin );
	targa_header.width = fgetLittleShort( fin );
	targa_header.height = fgetLittleShort( fin );
	targa_header.pixel_size = fgetc( fin );
	targa_header.attributes = fgetc( fin );
	flip = ( targa_header.attributes & 0x020 ) == 0;
	mirror = ( targa_header.attributes & 0x010 ) != 0;

	if ( ( targa_header.image_type != 2 ) && ( targa_header.image_type != 10 ) ) {
		Error( "LoadTGA: Only type 2 and 10 targa RGB images supported\n" );
	}

	if ( targa_header.colormap_type || ( ( targa_header.pixel_size != 32 ) && ( targa_header.pixel_size != 24 ) ) ) {
		Error( "Texture_LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n" );
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

	if ( !pixels ) {
		return;
	}

	targa_rgba = malloc( numPixels * 4 );
	*pixels = targa_rgba;

	if ( flip ) {
		pixbuf = targa_rgba + ( ( rows - 1 ) * columns * 4 );
		rowOffset = -columns * 4;
	}
	else
	{
		pixbuf = targa_rgba;
		rowOffset = columns * 4;
	}
	if ( mirror ) {
		pixDirection = -4;
		pixbuf += ( ( columns - 1 ) * 4 );
	}
	else
	{
		pixDirection = 4;
	}

	if ( targa_header.id_length ) {
		fseek( fin, targa_header.id_length, SEEK_CUR );  // skip TARGA image comment

	}
	if ( targa_header.image_type == 2 ) {                 // Uncompressed, RGB images
		for ( row = 0; row < rows; row++ )
		{
			rowBuf = pixbuf;
			for ( column = 0; column < columns; column++ )
			{
				switch ( targa_header.pixel_size )
				{
				case 24:
					blue = getc( fin );
					green = getc( fin );
					red = getc( fin );
					rowBuf[0] = red;
					rowBuf[1] = green;
					rowBuf[2] = blue;
					rowBuf[3] = 255;
					rowBuf += pixDirection;
					break;
				case 32:
					blue = getc( fin );
					green = getc( fin );
					red = getc( fin );
					alphabyte = getc( fin );
					rowBuf[0] = red;
					rowBuf[1] = green;
					rowBuf[2] = blue;
					rowBuf[3] = alphabyte;
					rowBuf += pixDirection;
					break;
				}
			}
			pixbuf += rowOffset;
		}
	}
	else if ( targa_header.image_type == 10 ) {        // Runlength encoded RGB images
		for ( row = 0; row < rows; row++ )
		{
			rowBuf = pixbuf;
			for ( column = 0; column < columns; )
			{
				packetHeader = getc( fin );
				packetSize = 1 + ( packetHeader & 0x7f );
				if ( packetHeader & 0x80 ) {          // run-length packet
					switch ( targa_header.pixel_size )
					{
					case 24:
						blue = getc( fin );
						green = getc( fin );
						red = getc( fin );
						alphabyte = 255;
						break;
					case 32:
						blue = getc( fin );
						green = getc( fin );
						red = getc( fin );
						alphabyte = getc( fin );
						break;
					}

					for ( j = 0; j < packetSize; j++ )
					{
						rowBuf[0] = red;
						rowBuf[1] = green;
						rowBuf[2] = blue;
						rowBuf[3] = alphabyte;
						rowBuf += pixDirection;
						column++;
						if ( column == columns ) {             // run spans across rows
							column = 0;
							row++;
							if ( row >= rows ) {
								goto breakOut;
							}
							pixbuf += rowOffset;
							rowBuf = pixbuf;
						}
					}
				}
				else
				{                                       // non run-length packet
					for ( j = 0; j < packetSize; j++ )
					{
						switch ( targa_header.pixel_size )
						{
						case 24:
							blue = getc( fin );
							green = getc( fin );
							red = getc( fin );
							rowBuf[0] = red;
							rowBuf[1] = green;
							rowBuf[2] = blue;
							rowBuf[3] = 255;
							rowBuf += pixDirection;
							break;
						case 32:
							blue = getc( fin );
							green = getc( fin );
							red = getc( fin );
							alphabyte = getc( fin );
							rowBuf[0] = red;
							rowBuf[1] = green;
							rowBuf[2] = blue;
							rowBuf[3] = alphabyte;
							rowBuf += pixDirection;
							break;
						}
						column++;
						if ( column == columns ) {    // pixel packet run spans across rows
							column = 0;
							row++;
							if ( row >= rows ) {
								goto breakOut;
							}
							pixbuf += rowOffset;
							rowBuf = pixbuf;
						}
					}
				}
			}
breakOut:;
			pixbuf += rowOffset;
		}
	}
	fclose( fin );
}

void MergeAlpha( byte *pix, byte *alpha, byte *pal, byte **out, int width, int height ){
	int size, i;
	byte    *data, *src, *srca;

	size = width * height;
	data = malloc( size * 4 );
	if ( !data ) {
		Error( "Could not allocate memory for true color image" );
	}

	*out = data;
	src = pix;
	srca = alpha;

	for ( i = 0; i < size; i++, src++, srca++ )
	{
		*data++ = pal[*src * 3 + 0];      // r
		*data++ = pal[*src * 3 + 1];      // g
		*data++ = pal[*src * 3 + 2];      // b
		*data++ = *srca;                  // a
	}
	free( pix );
	free( alpha );
	free( pal );
}

/*
   ==============
   LoadAnyImage

   Return Value:
    false: paletted texture
    true:  true color RGBA image (no palette)
   ==============
 */
qboolean LoadAnyImage( char *name, byte **pixels, byte **palette, int *width, int *height ){
	char ext[128];
	int len;
	int alpha_width, alpha_height;
	char alpha_name[128];
	byte    *alpha_pixels;

	ExtractFileExtension( name, ext );

	if ( palette ) {
		*palette = NULL;
	}

	if ( !Q_strcasecmp( ext, "lbm" ) ) {
		LoadLBM( name, pixels, palette );
		if ( width ) {
			*width = bmhd.w;
		}
		if ( height ) {
			*height = bmhd.h;
		}
		return false;
	}
	else if ( !Q_strcasecmp( ext, "pcx" ) ) {
		len = strlen( name );
		strcpy( alpha_name, name );
		strcpy( &alpha_name[len - 4], "_a.pcx" );                 // Alpha map name (may not exist)

		if ( FileExists( alpha_name ) ) {
			LoadPCX( name, pixels, palette, width, height );                         // Load in image
			LoadPCX( alpha_name, &alpha_pixels, NULL, &alpha_width, &alpha_height ); // Load in alpha map
			if ( ( *width != alpha_width ) || ( *height != alpha_height ) ) {
				Error( "Alpha image dimensions not equal to graphic image dimensions" );
			}
			MergeAlpha( *pixels, alpha_pixels, *palette, pixels, *width, *height );
			*palette = NULL; //Merge Frees pal
			return true;
		}
		else
		{
			LoadPCX( name, pixels, palette, width, height );         // Load in image
			return false;
		}
	}
	else if ( !Q_strcasecmp( ext, "tga" ) ) {
		LoadTGA( name, pixels, width, height );
		if ( palette ) {
			*palette = NULL;
		}

		return true;
	}
	else{
		Error( "%s doesn't have a known image extension", name );
	}

	return false;
}
