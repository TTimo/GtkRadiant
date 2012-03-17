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

#ifdef _WIN32
 #include <windows.h>
#endif

#include <GL/gl.h>

#if 1
extern char     *g_outputDir;
#endif // _QDATA

char mip_prefix[1024];              // directory to dump the textures in

qboolean colormap_issued;
byte colormap_palette[768];

unsigned total_x = 0;
unsigned total_y = 0;
unsigned total_textures = 0;

#define MAX_IMAGE_SIZE 512

#if 0
/*
   ==============
   RemapZero

   Replaces all 0 bytes in an image with the closest palette entry.
   This is because NT won't let us change index 0, so any palette
   animation leaves those pixels untouched.
   ==============
 */
void RemapZero( byte *pixels, byte *palette, int width, int height ){
	int i, c;
	int alt_zero;
	int value, best;

	alt_zero = 0;
	best = 9999999;
	for ( i = 1 ; i < 255 ; i++ )
	{
		value = palette[i * 3 + 0] + palette[i * 3 + 1] + palette[i * 3 + 2];
		if ( value < best ) {
			best = value;
			alt_zero = i;
		}
	}

	c = width * height;
	for ( i = 0 ; i < c ; i++ )
		if ( pixels[i] == 0 ) {
			pixels[i] = alt_zero;
		}
}

#endif


// ********************************************************************
// **  Mip Map Pre-Processing Routines
// ********************************************************************

#define intensity_value 1

static unsigned image_pal[256];

#define MAX_LAST 25

long palette_r[256], palette_g[256], palette_b[256];
long last_r[MAX_LAST],last_g[MAX_LAST],last_b[MAX_LAST], last_i[MAX_LAST], last_place;

long cached;

void PrepareConvert( unsigned *palette ){
	int i;

	for ( i = 0; i < 256; i++ )
	{
		palette_r[i] = ( palette[i] & 0x00ff0000 ) >> 16;
		palette_g[i] = ( palette[i] & 0x0000ff00 ) >> 8;
		palette_b[i] = ( palette[i] & 0x000000ff );
	}

	for ( i = 0; i < MAX_LAST; i++ )
		last_r[i] = -1;

	last_place = -1;
}

int ConvertTrueColorToPal( unsigned r, unsigned g, unsigned b ){
	int i;
	long min_dist;
	int min_index;
	long dist;
	long dr, dg, db, biggest_delta;

	for ( i = 0; i < MAX_LAST; i++ )
		if ( r == last_r[i] && g == last_g[i] && b == last_b[i] ) {
			cached++;
			return last_i[i];
		}

	min_dist = 256 * 256 + 256 * 256 + 256 * 256;
	biggest_delta = 256 * 256;
	min_index = 0;

	for ( i = 0; i < 256; i++ )
	{
		dr = abs( palette_r[i] - r );
		if ( dr > biggest_delta ) {
			continue;
		}
		dg = abs( palette_g[i] - g );
		if ( dg > biggest_delta ) {
			continue;
		}
		db = abs( palette_b[i] - b );
		if ( db > biggest_delta ) {
			continue;
		}

		dist = dr * dr + dg * dg + db * db;
		if ( dist < min_dist ) {
			min_dist = dist;
			min_index = i;
			if ( min_dist == 0 ) {
				break;
			}

			dist = dr;
			if ( dg > dist ) {
				dist = dg;
			}
			if ( db > dist ) {
				dist = db;
			}
			if ( dist < biggest_delta ) {
				biggest_delta = dist;
			}
		}
	}

	last_place++;
	if ( last_place >= MAX_LAST ) {
		last_place = 0;
	}

	last_r[last_place] = r;
	last_g[last_place] = g;
	last_b[last_place] = b;
	last_i[last_place] = min_index;

	return min_index;
}


void GL_ResampleTexture8P( byte *in, int inwidth, int inheight, byte *out,
						   int outwidth, int outheight, palette_t *palette ){
	int i, j;
	byte    *inrow, *inrow2;
	unsigned frac, fracstep;
	unsigned p1[1024], p2[1024], *p1p, *p2p;
	palette_t   *c1,*c2,*c3,*c4;
	unsigned r,g,b;

	fracstep = inwidth * 0x10000 / outwidth;

	frac = fracstep >> 2;
	for ( i = 0 ; i < outwidth ; i++ )
	{
		p1[i] = frac >> 16;
		frac += fracstep;
	}
	frac = 3 * ( fracstep >> 2 );
	for ( i = 0 ; i < outwidth ; i++ )
	{
		p2[i] = frac >> 16;
		frac += fracstep;
	}

	cached = 0;

	for ( i = 0 ; i < outheight ; i++ ) //, out += outwidth)
	{
		inrow = in + inwidth * (int)( ( i + 0.25 ) * inheight / outheight );
		inrow2 = in + inwidth * (int)( ( i + 0.75 ) * inheight / outheight );

		p1p = p1;
		p2p = p2;
		for ( j = 0 ; j < outwidth ; j++ )
		{
			c1 = &palette[*( (byte *)inrow + ( *p1p ) )];
			c2 = &palette[*( (byte *)inrow + ( *p2p ) )];
			c3 = &palette[*( (byte *)inrow2 + ( *p1p++ ) )];
			c4 = &palette[*( (byte *)inrow2 + ( *p2p++ ) )];

			r = ( (unsigned)c1->r + (unsigned)c2->r + (unsigned)c3->r + (unsigned)c4->r ) >> 2;
			g = ( (unsigned)c1->g + (unsigned)c2->g + (unsigned)c3->g + (unsigned)c4->g ) >> 2;
			b = ( (unsigned)c1->b + (unsigned)c2->b + (unsigned)c3->b + (unsigned)c4->b ) >> 2;

			*out++ = ConvertTrueColorToPal( r,g,b );
		}
	}
}

void GL_MipMap8P( byte *out, byte *in, int width, int height, palette_t *palette ){
	int i, j;
	palette_t   *c1,*c2,*c3,*c4;
	unsigned r,g,b;

	cached = 0;
	memset( out, 0, 256 * 256 );
	width <<= 1;
	height <<= 1;

	for ( i = 0; i < height; i += 2, in += width )
	{
		for ( j = 0; j < width; j += 2 )
		{
			c1 = &palette[in[0]];
			c3 = &palette[in[width]];
			in++;
			c2 = &palette[in[0]];
			c4 = &palette[in[width]];
			in++;

			r = ( (unsigned)c1->r + (unsigned)c2->r + (unsigned)c3->r + (unsigned)c4->r ) >> 2;
			g = ( (unsigned)c1->g + (unsigned)c2->g + (unsigned)c3->g + (unsigned)c4->g ) >> 2;
			b = ( (unsigned)c1->b + (unsigned)c2->b + (unsigned)c3->b + (unsigned)c4->b ) >> 2;

			*out++ = ConvertTrueColorToPal( r, g, b );
		}
	}
}


miptex_t *CreateMip( byte *data, unsigned width, unsigned height, byte *palette, int *FinalSize, qboolean mip ){
	int scaled_width, scaled_height;
	int i,j,r,g,b;
	byte intensitytable[256];
	byte scaled[256 * 256];
	byte out[256 * 256];
	int miplevel;
	miptex_t    *mp;
	byte        *pos;
	int size;

	for ( i = 0 ; i < 256 ; i++ )
	{
		j = i * intensity_value;
		if ( j > 255 ) {
			j = 255;
		}
		intensitytable[i] = j;
	}

	for ( scaled_width = 1 ; scaled_width < width ; scaled_width <<= 1 )
		;
	if ( 1 && scaled_width > width && 1 ) {
		scaled_width >>= 1;
	}
	for ( scaled_height = 1 ; scaled_height < height ; scaled_height <<= 1 )
		;
	if ( 1 && scaled_height > height && 1 ) {
		scaled_height >>= 1;
	}

	// don't ever bother with >256 textures
	if ( scaled_width > 256 ) {
		scaled_width = 256;
	}
	if ( scaled_height > 256 ) {
		scaled_height = 256;
	}

	if ( scaled_width < 1 ) {
		scaled_width = 1;
	}
	if ( scaled_height < 1 ) {
		scaled_height = 1;
	}

	size = sizeof( *mp ) + ( scaled_width * scaled_height * 3 );
	mp = (miptex_t *)SafeMalloc( size, "CreateMip" );
	memset( mp,0,size );

	mp->version = MIP_VERSION;

	for ( i = j = 0; i < 256; i++,j += 3 )
	{
		mp->palette[i].r = r = intensitytable[palette[j]];
		mp->palette[i].g = g = intensitytable[palette[j + 1]];
		mp->palette[i].b = b = intensitytable[palette[j + 2]];
		image_pal[i] = 0xff000000 | ( r << 16 ) | ( g << 8 ) | ( b );
	}

	PrepareConvert( image_pal );

	if ( scaled_width == width && scaled_height == height ) {
		memcpy( scaled, data, width * height );
	}
	else{
		GL_ResampleTexture8P( data, width, height, scaled, scaled_width, scaled_height, mp->palette );
	}

	pos = (byte *)( mp + 1 );
	miplevel = 0;

	while ( ( scaled_width >= 1 || scaled_height >= 1 ) && ( miplevel <= MIPLEVELS - 1 ) && ( !miplevel || mip ) )
	{
		if ( scaled_width < 1 ) {
			scaled_width = 1;
		}
		if ( scaled_height < 1 ) {
			scaled_height = 1;
		}

		if ( miplevel > 0 ) {
			GL_MipMap8P( out, (byte *)scaled, scaled_width, scaled_height, mp->palette );
		}
		else{
			memcpy( out, scaled, 256 * 256 );
		}

		mp->width[miplevel] = scaled_width;
		mp->height[miplevel] = scaled_height;
		mp->offsets[miplevel] = pos - ( (byte *)( mp ) );
		memcpy( pos, out, scaled_width * scaled_height );
		memcpy( scaled, out, 256 * 256 );
		pos += scaled_width * scaled_height;

		scaled_width >>= 1;
		scaled_height >>= 1;

		miplevel++;
	}

	*FinalSize = pos - ( (byte *)( mp ) );

	return mp;
}


void GL_ResampleTexture( unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight ){
	int i, j;
	unsigned    *inrow, *inrow2;
	unsigned frac, fracstep;
	unsigned p1[1024], p2[1024];
	byte        *pix1, *pix2, *pix3, *pix4;

	fracstep = inwidth * 0x10000 / outwidth;

	frac = fracstep >> 2;
	for ( i = 0 ; i < outwidth ; i++ )
	{
		p1[i] = 4 * ( frac >> 16 );
		frac += fracstep;
	}
	frac = 3 * ( fracstep >> 2 );
	for ( i = 0 ; i < outwidth ; i++ )
	{
		p2[i] = 4 * ( frac >> 16 );
		frac += fracstep;
	}

	for ( i = 0 ; i < outheight ; i++, out += outwidth )
	{
		inrow = in + inwidth * (int)( ( i + 0.25 ) * inheight / outheight );
		inrow2 = in + inwidth * (int)( ( i + 0.75 ) * inheight / outheight );
		frac = fracstep >> 1;
		for ( j = 0 ; j < outwidth ; j++ )
		{
			pix1 = (byte *)inrow + p1[j];
			pix2 = (byte *)inrow + p2[j];
			pix3 = (byte *)inrow2 + p1[j];
			pix4 = (byte *)inrow2 + p2[j];
			( (byte *)( out + j ) )[0] = ( pix1[0] + pix2[0] + pix3[0] + pix4[0] ) >> 2;
			( (byte *)( out + j ) )[1] = ( pix1[1] + pix2[1] + pix3[1] + pix4[1] ) >> 2;
			( (byte *)( out + j ) )[2] = ( pix1[2] + pix2[2] + pix3[2] + pix4[2] ) >> 2;
			( (byte *)( out + j ) )[3] = ( pix1[3] + pix2[3] + pix3[3] + pix4[3] ) >> 2;
		}
	}
}

void GL_MipMap( byte *out, byte *in, int width, int height ){
	int i, j;

	width <<= 3;
	height <<= 1;
	for ( i = 0 ; i < height ; i++, in += width )
	{
		for ( j = 0 ; j < width ; j += 8, out += 4, in += 8 )
		{
			out[0] = ( in[0] + in[4] + in[width + 0] + in[width + 4] ) >> 2;
			out[1] = ( in[1] + in[5] + in[width + 1] + in[width + 5] ) >> 2;
			out[2] = ( in[2] + in[6] + in[width + 2] + in[width + 6] ) >> 2;
			out[3] = ( in[3] + in[7] + in[width + 3] + in[width + 7] ) >> 2;
		}
	}
}

miptex32_t *CreateMip32( unsigned *data, unsigned width, unsigned height, int *FinalSize, qboolean mip ){
	int scaled_width, scaled_height;
	unsigned scaled[MAX_IMAGE_SIZE * MAX_IMAGE_SIZE];
	unsigned out[MAX_IMAGE_SIZE * MAX_IMAGE_SIZE];
	int miplevel;
	miptex32_t      *mp;
	byte            *pos;
	int size;
	paletteRGBA_t   *test;

	for ( scaled_width = 1 ; scaled_width < width ; scaled_width <<= 1 )
		;
	if ( 1 && scaled_width > width && 1 ) {
		scaled_width >>= 1;
	}
	for ( scaled_height = 1 ; scaled_height < height ; scaled_height <<= 1 )
		;
	if ( 1 && scaled_height > height && 1 ) {
		scaled_height >>= 1;
	}

	// don't ever bother with >256 textures
	if ( scaled_width > MAX_IMAGE_SIZE ) {
		scaled_width = MAX_IMAGE_SIZE;
	}
	if ( scaled_height > MAX_IMAGE_SIZE ) {
		scaled_height = MAX_IMAGE_SIZE;
	}

	if ( scaled_width < 1 ) {
		scaled_width = 1;
	}
	if ( scaled_height < 1 ) {
		scaled_height = 1;
	}

	size = sizeof( *mp ) + ( scaled_width * scaled_height * 3 * 4 );
	mp = (miptex32_t *)SafeMalloc( size, "CreateMip" );
	memset( mp,0,size );

	mp->version = MIP32_VERSION;

	size = width * height;
	test = (paletteRGBA_t *)data;
	while ( size )
	{
		if ( test->a != 255 ) {
			mp->flags |= LittleLong( SURF_ALPHA_TEXTURE );
			break;
		}

		size--;
		test++;
	}

	if ( scaled_width == width && scaled_height == height ) {
		memcpy( scaled, data, width * height * 4 );
	}
	else{
		GL_ResampleTexture( data, width, height, scaled, scaled_width, scaled_height );
	}

	pos = (byte *)( mp + 1 );
	miplevel = 0;

	while ( ( scaled_width >= 1 || scaled_height >= 1 ) && ( miplevel <= MIPLEVELS - 1 ) && ( !miplevel || mip ) )
	{
		if ( scaled_width < 1 ) {
			scaled_width = 1;
		}
		if ( scaled_height < 1 ) {
			scaled_height = 1;
		}

		if ( miplevel > 0 ) {
			GL_MipMap( (byte *)out, (byte *)scaled, scaled_width, scaled_height );
		}
		else
		{
			memcpy( out, scaled, MAX_IMAGE_SIZE * MAX_IMAGE_SIZE * 4 );
		}

		mp->width[miplevel] = scaled_width;
		mp->height[miplevel] = scaled_height;
		mp->offsets[miplevel] = pos - ( (byte *)( mp ) );
		memcpy( pos, out, scaled_width * scaled_height * 4 );
		memcpy( scaled, out, MAX_IMAGE_SIZE * MAX_IMAGE_SIZE * 4 );
		pos += scaled_width * scaled_height * 4;

		scaled_width >>= 1;
		scaled_height >>= 1;

		miplevel++;
	}

	*FinalSize = pos - ( (byte *)( mp ) );

	return mp;
}

/*
   ==============
   Cmd_Grab

   $grab filename x y width height
   ==============
 */
void Cmd_Grab( void ){
	int xl,yl,w,h,y;
	byte            *cropped;
	char savename[1024];
	char dest[1024];

	GetScriptToken( false );

	if ( token[0] == '/' || token[0] == '\\' ) {
		sprintf( savename, "%s%s.pcx", gamedir, token + 1 );
	}
	else{
		sprintf( savename, "%spics/%s.pcx", gamedir, token );
	}

	if ( g_release ) {
		if ( token[0] == '/' || token[0] == '\\' ) {
			sprintf( dest, "%s.pcx", token + 1 );
		}
		else{
			sprintf( dest, "pics/%s.pcx", token );
		}

		ReleaseFile( dest );
		return;
	}

	GetScriptToken( false );
	xl = atoi( token );
	GetScriptToken( false );
	yl = atoi( token );
	GetScriptToken( false );
	w = atoi( token );
	GetScriptToken( false );
	h = atoi( token );

	if ( xl < 0 || yl < 0 || w < 0 || h < 0 || xl + w > byteimagewidth || yl + h > byteimageheight ) {
		Error( "GrabPic: Bad size: %i, %i, %i, %i",xl,yl,w,h );
	}

	// crop it to the proper size
	cropped = (byte *) SafeMalloc( w * h, "Cmd_Grab" );
	for ( y = 0 ; y < h ; y++ )
	{
		memcpy( cropped + y * w, byteimage + ( y + yl ) * byteimagewidth + xl, w );
	}

	// save off the new image
	printf( "saving %s\n", savename );
	CreatePath( savename );
	WritePCXfile( savename, cropped, w, h, lbmpalette );

	free( cropped );
}

/*
   ==============
   Cmd_Raw

   $grab filename x y width height
   ==============
 */
void Cmd_Raw( void ){
	int xl,yl,w,h,y;
	byte            *cropped;
	char savename[1024];
	char dest[1024];

	GetScriptToken( false );

	sprintf( savename, "%s%s.lmp", gamedir, token );

	if ( g_release ) {
		sprintf( dest, "%s.lmp", token );
		ReleaseFile( dest );
		return;
	}

	GetScriptToken( false );
	xl = atoi( token );
	GetScriptToken( false );
	yl = atoi( token );
	GetScriptToken( false );
	w = atoi( token );
	GetScriptToken( false );
	h = atoi( token );

	if ( xl < 0 || yl < 0 || w < 0 || h < 0 || xl + w > byteimagewidth || yl + h > byteimageheight ) {
		Error( "GrabPic: Bad size: %i, %i, %i, %i",xl,yl,w,h );
	}

	// crop it to the proper size
	cropped = (byte *) SafeMalloc( w * h, "Cmd_Raw" );
	for ( y = 0 ; y < h ; y++ )
	{
		memcpy( cropped + y * w, byteimage + ( y + yl ) * byteimagewidth + xl, w );
	}

	// save off the new image
	printf( "saving %s\n", savename );
	CreatePath( savename );

	SaveFile( savename, cropped, w * h );

	free( cropped );
}

/*
   =============================================================================

   COLORMAP GRABBING

   =============================================================================
 */

/*
   ===============
   BestColor
   ===============
 */
byte BestColor( int r, int g, int b, int start, int stop ){
	int i;
	int dr, dg, db;
	int bestdistortion, distortion;
	int bestcolor;
	byte    *pal;

//
// let any color go to 0 as a last resort
//
	bestdistortion = 256 * 256 * 4;
	bestcolor = 0;

	pal = colormap_palette + start * 3;
	for ( i = start ; i <= stop ; i++ )
	{
		dr = r - (int)pal[0];
		dg = g - (int)pal[1];
		db = b - (int)pal[2];
		pal += 3;
		distortion = dr * dr + dg * dg + db * db;
		if ( distortion < bestdistortion ) {
			if ( !distortion ) {
				return i;       // perfect match

			}
			bestdistortion = distortion;
			bestcolor = i;
		}
	}

	return bestcolor;
}


/*
   ==============
   Cmd_Colormap

   $colormap filename

   the brightes colormap is first in the table (FIXME: reverse this now?)

   64 rows of 256 : lightmaps
   256 rows of 256 : translucency table
   ==============
 */
void Cmd_Colormap( void ){
	int levels, brights;
	int l, c;
	float frac, red, green, blue;
	float range;
	byte    *cropped, *lump_p;
	char savename[1024];
	char dest[1024];

	colormap_issued = true;
	if ( !g_release ) {
		memcpy( colormap_palette, lbmpalette, 768 );
	}

	if ( !ScriptTokenAvailable() ) { // just setting colormap_issued
		return;
	}

	GetScriptToken( false );
	sprintf( savename, "%spics/%s.pcx", gamedir, token );

	if ( g_release ) {
		sprintf( dest, "pics/%s.pcx", token );
		ReleaseFile( dest );
		return;
	}

	range = 2;
	levels = 64;
	brights = 1;    // ignore 255 (transparent)

	cropped = (byte *) SafeMalloc( ( levels + 256 ) * 256, "Cmd_ColorMap" );
	lump_p = cropped;

// shaded levels
	for ( l = 0; l < levels; l++ )
	{
		frac = range - range * (float)l / ( levels - 1 );
		for ( c = 0 ; c < 256 - brights ; c++ )
		{
			red = lbmpalette[c * 3];
			green = lbmpalette[c * 3 + 1];
			blue = lbmpalette[c * 3 + 2];

			red = (int)( red * frac + 0.5 );
			green = (int)( green * frac + 0.5 );
			blue = (int)( blue * frac + 0.5 );

//
// note: 254 instead of 255 because 255 is the transparent color, and we
// don't want anything remapping to that
// don't use color 0, because NT can't remap that (or 255)
//
			*lump_p++ = BestColor( red,green,blue, 1, 254 );
		}

		// fullbrights allways stay the same
		for ( ; c < 256 ; c++ )
			*lump_p++ = c;
	}

// 66% transparancy table
	for ( l = 0; l < 255; l++ )
	{
		for ( c = 0 ; c < 255 ; c++ )
		{
			red = lbmpalette[c * 3] * 0.33 + lbmpalette[l * 3] * 0.66;
			green = lbmpalette[c * 3 + 1] * 0.33 + lbmpalette[l * 3 + 1] * 0.66;
			blue = lbmpalette[c * 3 + 2] * 0.33 + lbmpalette[l * 3 + 2] * 0.66;

			*lump_p++ = BestColor( red,green,blue, 1, 254 );
		}
		*lump_p++ = 255;
	}
	for ( c = 0 ; c < 256 ; c++ )
		*lump_p++ = 255;

	// save off the new image
	printf( "saving %s\n", savename );
	CreatePath( savename );
	WritePCXfile( savename, cropped, 256, levels + 256, lbmpalette );

	free( cropped );
}

/*
   =============================================================================

   MIPTEX GRABBING

   =============================================================================
 */

byte pixdata[256];

int d_red, d_green, d_blue;

byte palmap[32][32][32];
qboolean palmap_built;

/*
   =============
   FindColor
   =============
 */
int FindColor( int r, int g, int b ){
	int bestcolor;

	if ( r > 255 ) {
		r = 255;
	}
	if ( r < 0 ) {
		r = 0;
	}
	if ( g > 255 ) {
		g = 255;
	}
	if ( g < 0 ) {
		g = 0;
	}
	if ( b > 255 ) {
		b = 255;
	}
	if ( b < 0 ) {
		b = 0;
	}
#ifndef TABLECOLORS
	bestcolor = BestColor( r, g, b, 0, 254 );
#else
	bestcolor = palmap[r >> 3][g >> 3][b >> 3];
#endif

	return bestcolor;
}


void BuildPalmap( void ){
#ifdef TABLECOLORS
	int r, g, b;
	int bestcolor;

	if ( palmap_built ) {
		return;
	}
	palmap_built = true;

	for ( r = 4 ; r < 256 ; r += 8 )
	{
		for ( g = 4 ; g < 256 ; g += 8 )
		{
			for ( b = 4 ; b < 256 ; b += 8 )
			{
				bestcolor = BestColor( r, g, b, 1, 254 );
				palmap[r >> 3][g >> 3][b >> 3] = bestcolor;
			}
		}
	}
#endif

	if ( !colormap_issued ) {
		Error( "You must issue a $colormap command first" );
	}

}

/*
   =============
   AveragePixels
   =============
 */
byte AveragePixels( int count ){
	int r,g,b;
	int i;
	int vis;
	int pix;
	int bestcolor;
	byte    *pal;
	int fullbright;

	vis = 0;
	r = g = b = 0;
	fullbright = 0;
	for ( i = 0 ; i < count ; i++ )
	{
		pix = pixdata[i];

		r += lbmpalette[pix * 3];
		g += lbmpalette[pix * 3 + 1];
		b += lbmpalette[pix * 3 + 2];
		vis++;
	}

	r /= vis;
	g /= vis;
	b /= vis;

	// error diffusion
	r += d_red;
	g += d_green;
	b += d_blue;

//
// find the best color
//
	bestcolor = FindColor( r, g, b );

	// error diffusion
	pal = colormap_palette + bestcolor * 3;
	d_red = r - (int)pal[0];
	d_green = g - (int)pal[1];
	d_blue = b - (int)pal[2];

	return bestcolor;
}


typedef enum
{
	pt_contents,
	pt_flags,
	pt_animvalue,
	pt_altnamevalue,
	pt_damagenamevalue,
	pt_flagvalue,
	pt_materialvalue,
	pt_scale,
	pt_mip,
	pt_detail,
	pt_gl,
	pt_nomip,
	pt_detailer,
} parmtype_t;

typedef struct
{
	char    *name;
	int flags;
	parmtype_t type;
} mipparm_t;

mipparm_t mipparms[] =
{
	// utility content attributes
	{"pushpull",CONTENTS_PUSHPULL, pt_contents},
	{"water",   CONTENTS_WATER, pt_contents},
	{"slime",   CONTENTS_SLIME, pt_contents},       // mildly damaging
	{"lava",    CONTENTS_LAVA, pt_contents},        // very damaging
	{"window",  CONTENTS_WINDOW, pt_contents},  // solid, but doesn't eat internal textures
	{"mist",    CONTENTS_MIST, pt_contents},    // non-solid window
	{"origin",  CONTENTS_ORIGIN, pt_contents},  // center of rotating brushes
	{"playerclip",  CONTENTS_PLAYERCLIP, pt_contents},
	{"monsterclip", CONTENTS_MONSTERCLIP, pt_contents},

	// utility surface attributes
	{"hint",    SURF_HINT, pt_flags},
	{"skip",    SURF_SKIP, pt_flags},
	{"light",   SURF_LIGHT, pt_flagvalue},      // value is the light quantity

	{"animspeed",SURF_ANIMSPEED, pt_flagvalue},     // value will hold the anim speed in fps

	// texture chaining
	{"anim",    0,          pt_animvalue},      // animname is the next animation
	{"alt",     0,          pt_altnamevalue},   // altname is the alternate texture
	{"damage",  0,          pt_damagenamevalue},    // damagename is the damage texture
	{"scale",   0,          pt_scale},      // next two values are for scale
	{"mip",     0,          pt_mip},
	{"detail",  0,          pt_detail},

	{"GL_ZERO",                 GL_ZERO,                pt_gl},
	{"GL_ONE",                  GL_ONE,                 pt_gl},
	{"GL_SRC_COLOR",            GL_SRC_COLOR,           pt_gl},
	{"GL_ONE_MINUS_SRC_COLOR",  GL_ONE_MINUS_SRC_COLOR, pt_gl},
	{"GL_DST_COLOR",            GL_DST_COLOR,           pt_gl},
	{"GL_ONE_MINUS_DST_COLOR",  GL_ONE_MINUS_DST_COLOR, pt_gl},
	{"GL_SRC_ALPHA",            GL_SRC_ALPHA,           pt_gl},
	{"GL_ONE_MINUS_SRC_ALPHA",  GL_ONE_MINUS_SRC_ALPHA, pt_gl},
	{"GL_DST_ALPHA",            GL_DST_ALPHA,           pt_gl},
	{"GL_ONE_MINUS_DST_ALPHA",  GL_ONE_MINUS_DST_ALPHA, pt_gl},
	{"GL_SRC_ALPHA_SATURATE",   GL_SRC_ALPHA_SATURATE,  pt_gl},

	// server attributes
	{"slick",   SURF_SLICK, pt_flags},

	// drawing attributes
	{"sky",     SURF_SKY, pt_flags},
	{"warping", SURF_WARP, pt_flags},       // only valid with 64x64 textures
	{"trans33", SURF_TRANS33, pt_flags},    // translucent should allso set fullbright
	{"trans66", SURF_TRANS66, pt_flags},
	{"flowing", SURF_FLOWING, pt_flags},    // flow direction towards angle 0
	{"nodraw",  SURF_NODRAW, pt_flags}, // for clip textures and trigger textures
	{"alpha",   SURF_ALPHA_TEXTURE, pt_flags},
	{"undulate",    SURF_UNDULATE, pt_flags},       // rock surface up and down...
	{"skyreflect",  SURF_SKYREFLECT, pt_flags},     // liquid will somewhat reflect the sky - not quite finished....

	{"material", SURF_MATERIAL, pt_materialvalue},
	{"metal",   SURF_TYPE_METAL, pt_flags},
	{"stone",   SURF_TYPE_STONE, pt_flags},
	{"wood",    SURF_TYPE_WOOD, pt_flags},

	{"m_nomip", 0, pt_nomip},
	{"m_detail", 0, pt_detailer},

	{NULL, 0, pt_contents}
};

/*
   ==============
   Cmd_Mip

   $mip filename x y width height <OPTIONS>
   must be multiples of sixteen
   SURF_WINDOW
   ==============
 */

void Cmd_Mip( void ){
	int xl,yl,xh,yh,w,h;
	byte            *dest, *source;
	int flags, value, contents;
	mipparm_t       *mp;
	char lumpname[128];
	char altname[128];
	char animname[128];
	char damagename[128];
	byte buffer[MAX_IMAGE_SIZE * MAX_IMAGE_SIZE];
	unsigned bufferl[MAX_IMAGE_SIZE * MAX_IMAGE_SIZE];
	materialtype_t  *mat;
	char filename[1024];
	unsigned        *destl, *sourcel;
	int linedelta, x, y;
	int size;
	miptex_t        *qtex;
	miptex32_t      *qtex32;
	float scale_x, scale_y;
	int mip_scale;
	// detail texturing
	char dt_name[128];
	float dt_scale_x, dt_scale_y;
	float dt_u, dt_v;
	float dt_alpha;
	int dt_src_blend_mode, dt_dst_blend_mode;
	int flags2;


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

	if ( ( w & 15 ) || ( h & 15 ) ) {
		Error( "line %i: miptex sizes must be multiples of 16", scriptline );
	}

	flags = 0;
	flags2 = 0;
	contents = 0;
	value = 0;
	mip_scale = 0;

	altname[0] = animname[0] = damagename[0] = 0;

	scale_x = scale_y = 0.5;

	// detail texturing
	dt_name[0] = 0;
	dt_scale_x = dt_scale_y = 0.0;
	dt_u = dt_v = 0.0;
	dt_alpha = 0.0;
	dt_src_blend_mode = dt_dst_blend_mode = 0;

	// get optional flags and values
	while ( ScriptTokenAvailable() )
	{
		GetScriptToken( false );

		for ( mp = mipparms ; mp->name ; mp++ )
		{
			if ( !strcmp( mp->name, token ) ) {
				switch ( mp->type )
				{
				case pt_animvalue:
					GetScriptToken( false );    // specify the next animation frame
					strcpy( animname, token );
					break;
				case pt_altnamevalue:
					GetScriptToken( false );    // specify the alternate texture
					strcpy( altname, token );
					break;
				case pt_damagenamevalue:
					GetScriptToken( false );    // specify the damage texture
					strcpy( damagename, token );
					break;
				case pt_flags:
					flags |= mp->flags;
					break;
				case pt_contents:
					contents |= mp->flags;
					break;
				case pt_flagvalue:
					flags |= mp->flags;
					GetScriptToken( false );    // specify the light value
					value = atoi( token );
					break;
				case pt_materialvalue:
					GetScriptToken( false );
					for ( mat = materialtypes ; mat->name ; mat++ )
					{
						if ( !strcmp( mat->name, token ) ) {
							// assumes SURF_MATERIAL is in top 8 bits
							flags = ( flags & 0x0FFFFFF ) | ( mat->value << 24 );
							break;
						}
					}
					break;
				case pt_scale:
					GetScriptToken( false );    // specify the x scale
					scale_x = atof( token );
					GetScriptToken( false );    // specify the y scale
					scale_y = atof( token );
					break;

				case pt_mip:
					mip_scale = 1;
					break;

				case pt_detailer:
					flags2 |= MIP32_DETAILER_FLAG2;
					break;

				case pt_nomip:
					flags2 |= MIP32_NOMIP_FLAG2;
					break;

				case pt_detail:
					GetScriptToken( false );
					strcpy( dt_name, token );
					GetScriptToken( false );
					dt_scale_x = atof( token );
					GetScriptToken( false );
					dt_scale_y = atof( token );
					GetScriptToken( false );
					dt_u = atof( token );
					GetScriptToken( false );
					dt_v = atof( token );
					GetScriptToken( false );
					dt_alpha = atof( token );
					GetScriptToken( false );
					for ( mp = mipparms ; mp->name ; mp++ )
					{
						if ( !strcmp( mp->name, token ) ) {
							if ( mp->type == pt_gl ) {
								dt_src_blend_mode = mp->flags;
								break;
							}
						}
					}
					if ( !mp->name ) {
						Error( "line %i: invalid gl blend mode %s", scriptline, token );
					}
					GetScriptToken( false );
					for ( mp = mipparms ; mp->name ; mp++ )
					{
						if ( !strcmp( mp->name, token ) ) {
							if ( mp->type == pt_gl ) {
								dt_dst_blend_mode = mp->flags;
								break;
							}
						}
					}
					if ( !mp->name ) {
						Error( "line %i: invalid gl blend mode %s", scriptline, token );
					}
					break;
				}
				break;
			}
		}
		if ( !mp->name ) {
			Error( "line %i: unknown parm %s", scriptline, token );
		}
	}

	if ( g_release ) {
		return; // textures are only released by $maps

	}
	xh = xl + w;
	yh = yl + h;
	if ( xh * yh > MAX_IMAGE_SIZE * MAX_IMAGE_SIZE ) {
		Error( "line %i image %s: image is too big!", scriptline, lumpname );
	}

	if ( TrueColorImage ) {
		if ( xl >= longimagewidth || xh > longimagewidth ||
			 yl >= longimageheight || yh > longimageheight ) {
			Error( "line %i image %s: bad clip dimmensions (%d,%d) (%d,%d) > image (%d,%d)", scriptline, lumpname, xl,yl,w,h,longimagewidth,longimageheight );
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

		qtex32 = CreateMip32( bufferl, w, h, &size, true );

		qtex32->flags |= LittleLong( flags );
		qtex32->flags2 |= LittleLong( flags2 );
		qtex32->contents = LittleLong( contents );
		qtex32->value = LittleLong( value );
		qtex32->scale_x = scale_x;
		qtex32->scale_y = scale_y;
		qtex32->mip_scale = mip_scale;
		sprintf( qtex32->name, "%s/%s", mip_prefix, lumpname );
		if ( animname[0] ) {
			sprintf( qtex32->animname, "%s/%s", mip_prefix, animname );
		}
		if ( altname[0] ) {
			sprintf( qtex32->altname, "%s/%s", mip_prefix, altname );
		}
		if ( damagename[0] ) {
			sprintf( qtex32->damagename, "%s/%s", mip_prefix, damagename );
		}
		if ( dt_name[0] & ( ( flags2 & MIP32_DETAILER_FLAG2 ) == 0 ) ) {
			sprintf( qtex32->dt_name, "%s/%s", mip_prefix, dt_name );
			qtex32->dt_scale_x = dt_scale_x;
			qtex32->dt_scale_y = dt_scale_y;
			qtex32->dt_u = dt_u;
			qtex32->dt_v = dt_v;
			qtex32->dt_alpha = dt_alpha;
			qtex32->dt_src_blend_mode = dt_src_blend_mode;
			qtex32->dt_dst_blend_mode = dt_dst_blend_mode;
		}

		//
		// write it out
		//
		sprintf( filename, "%stextures/%s/%s.m32", g_outputDir, mip_prefix, lumpname );
		if ( qtex32->flags & ( SURF_ALPHA_TEXTURE ) ) {
			printf( "writing %s with ALPHA\n", filename );
		}
		else{
			printf( "writing %s\n", filename );
		}
		SaveFile( filename, (byte *)qtex32, size );

		free( qtex32 );
	}
	else
	{
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

		qtex = CreateMip( buffer, w, h, lbmpalette, &size, true );

		qtex->flags = LittleLong( flags );
		qtex->contents = LittleLong( contents );
		qtex->value = LittleLong( value );
		sprintf( qtex->name, "%s/%s", mip_prefix, lumpname );
		if ( animname[0] ) {
			sprintf( qtex->animname, "%s/%s", mip_prefix, animname );
		}

		//
		// write it out
		//
		sprintf( filename, "%stextures/%s/%s.m8", g_outputDir, mip_prefix, lumpname );
		printf( "writing %s\n", filename );
		SaveFile( filename, (byte *)qtex, size );

		free( qtex );
	}
}

/*
   ===============
   Cmd_Mippal
   ===============
 */
void Cmd_Mippal( void ){
	colormap_issued = true;
	if ( g_release ) {
		return;
	}

	memcpy( colormap_palette, lbmpalette, 768 );

	BuildPalmap();
}


/*
   ===============
   Cmd_Mipdir
   ===============
 */
void Cmd_Mipdir( void ){
	char filename[1024];

	GetScriptToken( false );
	strcpy( mip_prefix, token );
	// create the directory if needed
	sprintf( filename, "%stextures", g_outputDir );
	Q_mkdir( filename );
	sprintf( filename, "%stextures/%s", g_outputDir, mip_prefix );
	Q_mkdir( filename );
}


/*
   =============================================================================

   ENVIRONMENT MAP GRABBING

   Creates six pcx files from tga files without any palette edge seams
   also copies the tga files for GL rendering.
   =============================================================================
 */

// 3dstudio environment map suffixes
char    *suf[6] = {"rt", "ft", "lf", "bk", "up", "dn"};

/*
   =================
   Cmd_Environment
   =================
 */
void Cmd_Environment( void ){
	char name[1024];
	int i, x, y;
	byte image[256 * 256];
	byte    *tga;

	GetScriptToken( false );

	if ( g_release ) {
		for ( i = 0 ; i < 6 ; i++ )
		{
			sprintf( name, "env/%s%s.pcx", token, suf[i] );
			ReleaseFile( name );
			sprintf( name, "env/%s%s.tga", token, suf[i] );
			ReleaseFile( name );
		}
		return;
	}
	// get the palette
	BuildPalmap();

	sprintf( name, "%senv/", gamedir );
	CreatePath( name );

	// convert the images
	for ( i = 0 ; i < 6 ; i++ )
	{
		sprintf( name, "%senv/%s%s.tga", gamedir, token, suf[i] );
		printf( "loading %s...\n", name );
		LoadTGA( name, &tga, NULL, NULL );

		for ( y = 0 ; y < 256 ; y++ )
		{
			for ( x = 0 ; x < 256 ; x++ )
			{
				image[y * 256 + x] = FindColor( tga[( y * 256 + x ) * 4 + 0],tga[( y * 256 + x ) * 4 + 1],tga[( y * 256 + x ) * 4 + 2] );
			}
		}
		free( tga );
		sprintf( name, "%senv/%s%s.pcx", gamedir, token, suf[i] );
		if ( FileTime( name ) != -1 ) {
			printf( "%s already exists, not overwriting.\n", name );
		}
		else{
			WritePCXfile( name, image, 256, 256, colormap_palette );
		}
	}
}
