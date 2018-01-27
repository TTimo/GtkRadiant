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

#include "q3data.h"

byte            *byteimage, *lbmpalette;
int byteimagewidth, byteimageheight;


char mip_prefix[1024];              // directory to dump the textures in

qboolean colormap_issued;
byte colormap_palette[768];

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

	GetToken( qfalse );

	if ( token[0] == '/' || token[0] == '\\' ) {
		sprintf( savename, "%s%s.pcx", writedir, token + 1 );
	}
	else{
		sprintf( savename, "%spics/%s.pcx", writedir, token );
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

	GetToken( qfalse );
	xl = atoi( token );
	GetToken( qfalse );
	yl = atoi( token );
	GetToken( qfalse );
	w = atoi( token );
	GetToken( qfalse );
	h = atoi( token );

	if ( xl < 0 || yl < 0 || w < 0 || h < 0 || xl + w > byteimagewidth || yl + h > byteimageheight ) {
		Error( "GrabPic: Bad size: %i, %i, %i, %i",xl,yl,w,h );
	}

	// crop it to the proper size
	cropped = malloc( w * h );
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

	GetToken( qfalse );

	sprintf( savename, "%s%s.lmp", writedir, token );

	if ( g_release ) {
		sprintf( dest, "%s.lmp", token );
		ReleaseFile( dest );
		return;
	}

	GetToken( qfalse );
	xl = atoi( token );
	GetToken( qfalse );
	yl = atoi( token );
	GetToken( qfalse );
	w = atoi( token );
	GetToken( qfalse );
	h = atoi( token );

	if ( xl < 0 || yl < 0 || w < 0 || h < 0 || xl + w > byteimagewidth || yl + h > byteimageheight ) {
		Error( "GrabPic: Bad size: %i, %i, %i, %i",xl,yl,w,h );
	}

	// crop it to the proper size
	cropped = malloc( w * h );
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

	colormap_issued = qtrue;
	if ( !g_release ) {
		memcpy( colormap_palette, lbmpalette, 768 );
	}

	if ( !TokenAvailable() ) { // just setting colormap_issued
		return;
	}

	GetToken( qfalse );
	sprintf( savename, "%spics/%s.pcx", writedir, token );

	if ( g_release ) {
		sprintf( dest, "pics/%s.pcx", token );
		ReleaseFile( dest );
		return;
	}

	range = 2;
	levels = 64;
	brights = 1;    // ignore 255 (transparent)

	cropped = malloc( ( levels + 256 ) * 256 );
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
	palmap_built = qtrue;

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

	vis = 0;
	r = g = b = 0;
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

	GetToken( qfalse );

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
		sprintf( name, "%senv/%s%s.pcx", writedir, token, suf[i] );
		if ( FileTime( name ) != -1 ) {
			printf( "%s already exists, not overwriting.\n", name );
		}
		else{
			WritePCXfile( name, image, 256, 256, colormap_palette );
		}
	}
}
