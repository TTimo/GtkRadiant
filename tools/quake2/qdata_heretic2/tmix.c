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
#include "flex.h"

#define     MAXFILES    2048

typedef struct
{
	int x;
	int y;
	int w;
	int h;
	int cw;
	int ch;
	int rw;
	int index;
	int depth;
	int col;
	int baseline;
	char name[128];
} Coords;

int filenum;
int valid;
Coords in[MAXFILES];
Coords out;
char outscript[256];
char sourcedir[256];
char outusage[256];
char root[32];

int destsize = 0;
byte            *pixels = NULL;             // Buffer to load image
long            *outpixels = NULL;          // Buffer to store combined textures
long            *usagemap = NULL;           // Buffer of usage map
void            *bmptemp = NULL;            // Buffer of usage map
byte            *map = NULL;

int xcharsize;
int ycharsize;
int dosort = 0;
int missed = 0;
int overlap = 0;
int nobaseline = 0;
int percent;

//////////////////////////////////////////////////
// Setting the char based usage map				//
//////////////////////////////////////////////////

byte    TryPlace( Coords *coord ){
	int x, y;
	byte entry = 0;
	byte    *mapitem;

	mapitem = map + ( coord->x / xcharsize ) + ( ( coord->y / ycharsize ) * out.cw );

	for ( y = 0; y < coord->ch; y++, mapitem += out.cw - coord->cw )
	{
		for ( x = 0; x < coord->cw; x++ )
		{
			if ( entry |= *mapitem++ & 8 ) {
				return( entry );
			}
		}
	}
	return( entry );
}

void    SetMap( Coords *coord ){
	int x, y;
	byte    *mapitem;

	mapitem = map + ( coord->x / xcharsize ) + ( ( coord->y / ycharsize ) * out.cw );

	for ( y = 0; y < coord->ch; y++, mapitem += out.cw - coord->cw )
		for ( x = 0; x < coord->cw; x++ )
			*mapitem++ |= 8;
}

//////////////////////////////////////////////////
// Setting the pixel based usage map			//
//////////////////////////////////////////////////

void    CheckOverlap( Coords *coord ){
	int x;
	int y;
	long        *dest;

	x = coord->x;
	y = coord->y;

	dest = (long *)( usagemap + x + ( y * out.w ) );

	for ( y = 0; y < coord->h; y++, dest += out.w - coord->w )
	{
		for ( x = 0; x < coord->w; x++ )
		{
			if ( *dest++ ) {
				overlap++;
				return;
			}
		}
	}
}

void    SetUsageMap( Coords *coord ){
	int x;
	int y;
	long        *dest;

	x = coord->x;
	y = coord->y;

	dest = (long *)( usagemap + x + ( y * out.w ) );

	for ( y = 0; y < coord->h; y++, dest += out.w - coord->w )
	{
		for ( x = 0; x < coord->w; x++ )
		{
			*dest++ = coord->col;
		}
	}
}

//////////////////////////////////////////////////
// Flips the BMP image to the correct way up	//
//////////////////////////////////////////////////

void    CopyLine( byte *dest, byte *src, int size ){
	int x;

	for ( x = 0; x < size; x++ )
		*dest++ = *src++;
}

/****************************************************/
/* Printing headers etc								*/
/****************************************************/

void RemoveLeading( char *name ){
	int i;
	char temp[128];

	for ( i = strlen( name ) - 1; i > 0; i-- )
	{
		if ( ( name[i] == '\\' ) || ( name[i] == '/' ) ) {
			strcpy( temp, name + i + 1 );
			strcpy( name, temp );
			return;
		}
	}
}

void RemoveExt( char *name ){
	while ( ( *name != '.' ) && *name )
		name++;
	*name = 0;
}

/****************************************************/
/* Misc calcualtions								*/
/****************************************************/

int TotalArea(){
	int i;
	int total = 0;

	for ( i = 0; i < ( filenum + 2 ); i++ )
		total += in[i].w * in[i].h;

	return( total );
}

/****************************************************/
/* Setup and checking of all info					*/
/****************************************************/

void    InitVars(){
	filenum = 0;
	valid = 0;
	dosort = 0;
	missed = 0;
	overlap = 0;
	nobaseline = 0;

	memset( outscript, 0, sizeof( outscript ) );
	memset( outscript, 0, sizeof( sourcedir ) );
	memset( outscript, 0, sizeof( outusage ) );
	memset( outscript, 0, sizeof( root ) );

	memset( in, 0, sizeof( in ) );
	memset( &out, 0, sizeof( out ) );
}
void Cleanup(){
	if ( pixels ) {
		free( pixels );
	}
	if ( usagemap ) {
		free( usagemap );
	}
	if ( outpixels ) {
		free( outpixels );
	}
	if ( bmptemp ) {
		free( bmptemp );
	}
	if ( map ) {
		free( map );
	}
}

typedef struct glxy_s
{
	float xl, yt, xr, yb;
	int w, h, baseline;
} glxy_t;

int SaveScript( char *name ){
	FILE        *fp;
	int i, j;
	glxy_t buff;

	if ( fp = fopen( name, "wb" ) ) {
		for ( j = 0; j < filenum; j++ )
		{
			for ( i = 0; i < filenum; i++ )
			{
				if ( in[i].index == j ) {
					if ( in[i].depth ) {
						buff.xl = (float)in[i].x / (float)out.w;
						buff.yt = (float)in[i].y / (float)out.h;
						buff.xr = ( (float)in[i].w + (float)in[i].x ) / (float)out.w;
						buff.yb = ( (float)in[i].h + (float)in[i].y ) / (float)out.h;
						buff.w = in[i].w;
						buff.h = in[i].h;
						buff.baseline = in[i].baseline;
					}
					else
					{
						memset( &buff, 0, sizeof( glxy_t ) );
					}
					fwrite( &buff, 1, sizeof( glxy_t ), fp );
					i = filenum;
				}
			}
		}
		fclose( fp );
		return( true );
	}
	else{
		return( false );
	}
}

int     GetScriptInfo( char *name ){
	FILE        *fp;
	char buffer[256];
	char tempbuff[256];
	char delims[] = {" \t,\n"};

	printf( "Opening script file %s.\n", name );

	if ( fp = fopen( name, "r" ) ) {
		while ( fgets( buffer, 256, fp ) )
		{
			if ( strncmp( buffer, "//", 2 ) && strncmp( buffer, "\n", 1 ) ) {
				strupr( buffer );
				strcpy( tempbuff, buffer );
				if ( strcmp( strtok( tempbuff, delims ), "OUTPUT" ) == 0 ) {
					strcpy( out.name, strtok( NULL, delims ) );
					strlwr( out.name );
				}

				strcpy( tempbuff, buffer );
				if ( strcmp( strtok( tempbuff, delims ), "SOURCEDIR" ) == 0 ) {
					strcpy( tempbuff, strtok( NULL, delims ) );
					strcpy( sourcedir, ExpandPathAndArchive( tempbuff ) );
				}

				strcpy( tempbuff, buffer );
				if ( strcmp( strtok( tempbuff, delims ), "DOSORT" ) == 0 ) {
					dosort = 1;
				}

				strcpy( tempbuff, buffer );
				if ( strcmp( strtok( tempbuff, delims ), "XCHARSIZE" ) == 0 ) {
					xcharsize = strtol( strtok( NULL, delims ), NULL, 0 );
				}

				strcpy( tempbuff, buffer );
				if ( strcmp( strtok( tempbuff, delims ), "YCHARSIZE" ) == 0 ) {
					ycharsize = strtol( strtok( NULL, delims ), NULL, 0 );
				}

				strcpy( tempbuff, buffer );
				if ( strcmp( strtok( tempbuff, delims ), "OUTSCRIPT" ) == 0 ) {
					strcpy( outscript, strtok( NULL, delims ) );
					strlwr( outscript );
				}

				strcpy( tempbuff, buffer );
				if ( strcmp( strtok( tempbuff, delims ), "OUTUSAGE" ) == 0 ) {
					strcpy( outusage, strtok( NULL, delims ) );
				}

				strcpy( tempbuff, buffer );
				if ( strcmp( strtok( tempbuff, delims ), "POS" ) == 0 ) {
					out.w = strtol( strtok( NULL, delims ), NULL, 0 );
					out.h = strtol( strtok( NULL, delims ), NULL, 0 );
				}

				strcpy( tempbuff, buffer );
				if ( strcmp( strtok( tempbuff, delims ), "FILE" ) == 0 ) {
					strcpy( in[filenum].name, strtok( NULL, delims ) );
					in[filenum].x = strtol( strtok( NULL, delims ), NULL, 0 );
					in[filenum].y = strtol( strtok( NULL, delims ), NULL, 0 );
					in[filenum].col = strtol( strtok( NULL, delims ), NULL, 0 );
					filenum++;
				}
			}
		}
		fclose( fp );
		return( true );
	}
	else
	{
		printf( "ERROR : Could not open script file.\n" );
		return( false );
	}
}

int CheckVars(){
	int i;

	if ( out.name[0] == 0 ) {
		printf( "ERROR : No output name specified.\n" );
		return( false );
	}
	if ( ( out.w <= 0 ) || ( out.h <= 0 ) ) {
		printf( "ERROR : Invalid VRAM coordinates.\n" );
		return( false );
	}
	if ( filenum == 0 ) {
		printf( "ERROR : No input files specified.\n" );
		return( false );
	}
	for ( i = 0; i < filenum; i++ )
		if ( in[i].name[0] == 0 ) {
			printf( "ERROR : Input filename invalid.\n" );
			return( false );
		}
	return( true );
}

// Makes sure texture is totally within the output area

int CheckCoords( Coords *coord ){
	if ( ( coord->x + coord->w ) > out.w ) {
		return( false );
	}
	if ( ( coord->y + coord->h ) > out.h ) {
		return( false );
	}

	return( true );
}
// Gets the width, height, palette width and palette height of each BMP file

int     GetFileDimensions(){
	int i;
	int width, height;
	char name[128];

	for ( i = 0; i < filenum; i++ )
	{
		in[i].index = i;

		strcpy( name, sourcedir );
		strcat( name, in[i].name );
		printf( "Getting file dimensions, file : %s        \r", in[i].name );
		if ( FileExists( name ) ) {
			LoadAnyImage( name, NULL, NULL, &width, &height );
			in[i].depth = 32;
			in[i].rw = width;
			in[i].w = width;                        // makes it width in
			in[i].h = height;
			in[i].cw = ( in[i].w + ( xcharsize - 1 ) ) / xcharsize;
			in[i].ch = ( in[i].h + ( ycharsize - 1 ) ) / ycharsize;

			if ( !CheckCoords( &in[i] ) && ( in[i].x >= 0 ) ) {
				printf( "Error : texture %s out of bounds.\n", in[i].name );
				return( false );
			}
			valid++;
		}
		else
		{
			in[i].depth = 0;
			in[i].x = -1;
			in[i].y = -1;
			in[i].w = 0;
			in[i].h = 0;
		}
	}
	printf( "\n\n" );
	return( true );
}

// Sorts files into order for optimal space finding
// Fixed position ones first, followed by the others in descending size
// The theory being that it is easier to find space for smaller textures.
// size = (width + height)
// For space finding it is easier to place a 32x32 than a 128x2

#define WEIGHT  0x8000

void    Swap( Coords *a, Coords *b ){
	Coords c;

	c = *a;
	*a = *b;
	*b = c;
}

void    SortInNames(){
	int i, j;
	int largest, largcount;
	int size;

	printf( "Sorting filenames by size.\n\n" );

	for ( j = 0; j < filenum; j++ )
	{
		largest = -1;
		largcount = -1;

		for ( i = j; i < filenum; i++ )
		{
			if ( in[i].depth ) {
				size = in[i].w + in[i].h;

				if ( ( in[i].x < 0 ) && ( size > largest ) ) {
					largcount = i;
					largest = size;
				}
			}
		}
		if ( ( largcount >= 0 ) && ( largcount != j ) ) {
			Swap( &in[j], &in[largcount] );
		}
	}
}

int SetVars( char *name ){
	if ( !GetScriptInfo( name ) ) {
		return( false );
	}

	if ( !CheckVars() ) {
		return( false );
	}

	destsize = out.w * out.h;

	out.cw = out.w / xcharsize;
	out.ch = out.h / ycharsize;

	if ( ( usagemap = (long *)SafeMalloc( destsize * 4, "" ) ) == NULL ) {
		return( false );
	}
	if ( ( outpixels = (long *)SafeMalloc( destsize * 4, "" ) ) == NULL ) {
		return( false );
	}
	if ( ( bmptemp = (void *)SafeMalloc( destsize * 4, "" ) ) == NULL ) {
		return( false );
	}
	if ( ( map = (byte *)SafeMalloc( destsize / ( xcharsize * ycharsize ), "" ) ) == NULL ) {
		return( false );
	}

	if ( GetFileDimensions() == false ) {
		return( false );
	}

	if ( dosort ) {
		SortInNames();
	}

	return( true );
}
/****************************************************/
/* Actual copying routines							*/
/****************************************************/

int FindCoords( Coords *coord ){
	int tx, ty;

	if ( coord->x >= 0 ) {
		SetMap( coord );
		return( true );
	}
	else
	{
		for ( ty = 0; ty < out.ch; ty++ )
		{
			for ( tx = 0; tx < out.cw; tx++ )
			{
				coord->x = ( tx * xcharsize );
				coord->y = ( ty * ycharsize );

				if ( CheckCoords( coord ) && !TryPlace( coord ) ) {
					SetMap( coord );
					return( true );
				}
			}
		}
	}
	coord->x = -1;
	coord->y = -1;

	return( false );
}

void CheckBaseline( int i ){
	int y;
	long    *pix;

	in[i].baseline = -1;
	pix = (long *)pixels;

	for ( y = 0; y < in[i].h; y++, pix += in[i].w )
	{
		if ( ( *pix & 0x00ffffff ) == 0x00ff00ff ) {
			in[i].baseline = y;
			break;
		}
	}
	pix = (long *)pixels;
	for ( y = 0; y < in[i].w * in[i].h; y++, pix++ )
	{
		if ( ( *pix & 0x00ffffff ) == 0x00ff00ff ) {
			*pix = 0;
		}
	}

	if ( in[i].baseline == -1 ) {
		printf( "\nERROR : %s has no baseline\n", in[i].name );
		nobaseline++;
	}
}

void    CopyToMain32( Coords *coord ){
	int x;
	int y;
	long        *source;
	long        *dest;

	x = coord->x;
	y = coord->y;

	source = (long *)pixels;
	dest = (long *)( outpixels + x + ( y * out.w ) );

	for ( y = 0; y < coord->h; y++, dest += out.w - coord->w )
	{
		for ( x = 0; x < coord->w; x++ )
		{
			*dest++ = *source++;
		}
	}
}

void CreateMain(){
	int i, count;
	int width, height;
	char name[128];

	for ( i = 0, count = 0; i < filenum; i++ )
	{
		if ( in[i].depth ) {
			printf( "\rProcessing %d of %d (%d missed, %d overlapping, %d nobase)\r", count + 1, valid, missed, overlap, nobaseline );
			count++;
			if ( !FindCoords( &in[i] ) ) {
				missed++;
			}
			else
			{
				strcpy( name, sourcedir );
				strcat( name, in[i].name );
				LoadAnyImage( name, &pixels, NULL, &width, &height );
				CheckBaseline( i );
				CheckOverlap( &in[i] );
				CopyToMain32( &in[i] );
				SetUsageMap( &in[i] );
			}
		}
	}
}

void Cmd_TextureMix(){
	miptex32_t      *qtex32;
	char filename[1024];
	int size;

	InitVars();

	GetScriptToken( false );

	strcpy( root, token );
	RemoveExt( root );
	RemoveLeading( root );

	strcpy( filename, ExpandPathAndArchive( token ) );
	if ( SetVars( filename ) ) {
		// Create combined texture
		percent = ( ( TotalArea() * 100 ) / ( out.w * out.h ) );
		printf( "Total area consumed : %d%%\n", percent );
		printf( "Texture resolution  : %dx%d pixels.\n", xcharsize, ycharsize );
		CreateMain();

		// Save image as m32
		sprintf( filename, "%spics/misc/%s.m32", gamedir, out.name );
		qtex32 = CreateMip32( (unsigned *)outpixels, out.w, out.h, &size, false );

		qtex32->contents = 0;
		qtex32->value = 0;
		qtex32->scale_x = 1.0;
		qtex32->scale_y = 1.0;
		sprintf( qtex32->name, "misc/%s", out.name );

		printf( "\n\nwriting %s\n", filename );
		SaveFile( filename, (byte *)qtex32, size );
		free( qtex32 );

		// Save out script file
		sprintf( filename, "%spics/misc/%s.fnt", gamedir, outscript );
		printf( "Writing %s as script file\n", filename );
		if ( !SaveScript( filename ) ) {
			printf( "Unable to save output script.\n" );
		}
	}
	printf( "Everythings groovy.\n" );
	Cleanup();
}

// end
