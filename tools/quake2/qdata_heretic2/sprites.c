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

#define MAX_SPRFRAMES           MAX_MD2SKINS

dsprite_t sprite;
dsprframe_t frames[MAX_SPRFRAMES];

byte            *byteimage, *lbmpalette;
int byteimagewidth, byteimageheight;

qboolean TrueColorImage;
unsigned        *longimage;
int longimagewidth, longimageheight;

char spritename[1024];


void FinishSprite( void );
void Cmd_Spritename( void );

char spr_prefix[1024];
char pic_prefix[1024];

extern char        *g_outputDir;


/*
   ==============
   FinishSprite
   ==============
 */
void FinishSprite( void ){
	FILE    *spriteouthandle;
	int i, curframe;
	dsprite_t spritetemp;
	char savename[1024];

	if ( sprite.numframes == 0 ) {
		return;
	}

	if ( !strlen( spritename ) ) {
		Error( "Didn't name sprite file" );
	}

	sprintf( savename, "%sSprites/%s/%s.sp2", g_outputDir, spr_prefix, spritename );

	if ( g_release ) {
		char name[1024];

		sprintf( name, "%s.sp2", spritename );
		ReleaseFile( name );
		spritename[0] = 0;      // clear for a new sprite
		sprite.numframes = 0;
		return;
	}


	printf( "saving in %s\n", savename );
	CreatePath( savename );
	spriteouthandle = SafeOpenWrite( savename );


//
// write out the sprite header
//
	spritetemp.ident = LittleLong( IDSPRITEHEADER );
	spritetemp.version = LittleLong( SPRITE_VERSION );
	spritetemp.numframes = LittleLong( sprite.numframes );

	SafeWrite( spriteouthandle, &spritetemp, 12 );

//
// write out the frames
//
	curframe = 0;

	for ( i = 0 ; i < sprite.numframes ; i++ )
	{
		frames[i].width = LittleLong( frames[i].width );
		frames[i].height = LittleLong( frames[i].height );
		frames[i].origin_x = LittleLong( frames[i].origin_x );
		frames[i].origin_y = LittleLong( frames[i].origin_y );
	}
	SafeWrite( spriteouthandle, frames, sizeof( frames[0] ) * sprite.numframes );

	fclose( spriteouthandle );

	spritename[0] = 0;      // clear for a new sprite
	sprite.numframes = 0;
}


/*
   ===============
   Cmd_Load
   ===============
 */
void Cmd_Load( void ){
	char    *name;

	GetScriptToken( false );

	if ( g_release ) {
		return;
	}

	name = ExpandPathAndArchive( token );

	// load the image
	printf( "loading %s\n", name );
	TrueColorImage = LoadAnyImage( name, &byteimage, &lbmpalette, &byteimagewidth, &byteimageheight );

	if ( !TrueColorImage ) {
//		RemapZero (byteimage, lbmpalette, byteimagewidth, byteimageheight);
	}
	else
	{
		if ( longimage ) {
			free( longimage );
		}
		longimage = (unsigned *)byteimage;
		longimagewidth = byteimagewidth;
		longimageheight = byteimageheight;

		byteimage = NULL;
		byteimagewidth = 0;
		byteimageheight = 0;
	}
}


/*
   ===============
   Cmd_SpriteFrame
   ===============
 */

void Cmd_SpriteFrame( void ){
	int x,y,xl,yl,xh,yh,w,h;
	dsprframe_t     *pframe;
	int ox, oy, linedelta, size;
//	byte			*cropped;
	char filename[1024];
	miptex_t        *qtex;
	miptex32_t      *qtex32;
	unsigned        *destl, *sourcel;
	unsigned bufferl[256 * 256];
	byte            *dest, *source;
	byte buffer[256 * 256];

	GetScriptToken( false );
	xl = atoi( token );
	GetScriptToken( false );
	yl = atoi( token );
	GetScriptToken( false );
	w = atoi( token );
	GetScriptToken( false );
	h = atoi( token );

	// origin offset is optional
	if ( ScriptTokenAvailable() ) {
		GetScriptToken( false );
		ox = atoi( token );
		GetScriptToken( false );
		oy = atoi( token );
	}
	else
	{
		ox = w / 2;
		oy = h / 2;
	}

	if ( ( xl & 0x0f ) || ( yl & 0x0f ) || ( w & 0x0f ) || ( h & 0x0f ) ) {
		Error( "Sprite dimensions not multiples of 16\n" );
	}

	if ( ( w > 256 ) || ( h > 256 ) ) {
		Error( "Sprite has a dimension longer than 256" );
	}

	xh = xl + w;
	yh = yl + h;

	if ( sprite.numframes >= MAX_SPRFRAMES ) {
		Error( "Too many frames; increase MAX_SPRFRAMES\n" );
	}

	pframe = &frames[sprite.numframes];
	pframe->width = w;
	pframe->height = h;
	pframe->origin_x = ox;
	pframe->origin_y = oy;

	if ( g_release ) {
		ReleaseFile( pframe->name );
		return;
	}

	if ( TrueColorImage ) {
		sprintf( filename, "%ssprites/%s/%s_%i.m32", g_outputDir, spr_prefix, spritename, sprite.numframes );
		sprintf( pframe->name, "%s/%s_%i.m32", spr_prefix, spritename, sprite.numframes );

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

		qtex32 = CreateMip32( bufferl, w, h, &size, true );

		qtex32->contents = 0;
		qtex32->value = 0;
		strcpy( qtex32->name, pframe->name );
		//
		// write it out
		//
		printf( "writing %s\n", filename );
		SaveFile( filename, (byte *)qtex32, size );

		free( qtex32 );
	}
	else
	{
		sprintf( filename, "%ssprites/%s/%s_%i.m8", g_outputDir, spr_prefix, spritename, sprite.numframes );
		sprintf( pframe->name, "%s/%s_%i.m8", spr_prefix, spritename, sprite.numframes );

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

		qtex = CreateMip( buffer, w, h, lbmpalette, &size, true );

		qtex->flags = 0;
		qtex->contents = 0;
		qtex->value = 0;
		strcpy( qtex->name, pframe->name );
		//
		// write it out
		//
		printf( "writing %s\n", filename );
		SaveFile( filename, (byte *)qtex, size );

		free( qtex );
	}

	sprite.numframes++;
}


/*
   ==============
   Cmd_SpriteName
   ==============
 */
void Cmd_SpriteName( void ){
	if ( sprite.numframes ) {
		FinishSprite();
	}

	GetScriptToken( false );
	strcpy( spritename, token );
	memset( &sprite, 0, sizeof( sprite ) );
	memset( &frames, 0, sizeof( frames ) );
}


/*
   ===============
   Cmd_Sprdir
   ===============
 */
void Cmd_Sprdir( void ){
	char filename[1024];

	GetScriptToken( false );
	strcpy( spr_prefix, token );
	// create the directory if needed
	sprintf( filename, "%sSprites", g_outputDir );
	Q_mkdir( filename );
	sprintf( filename, "%sSprites/%s", g_outputDir, spr_prefix );
	Q_mkdir( filename );
}
