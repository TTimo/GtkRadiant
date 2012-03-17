/*
   Copyright (c) 2002 Forest "LordHavoc" Hale

   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list
   of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of Forest Hale nor the names of other contributors may be used
   to endorse or promote products derived from this software without specific prior
   written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT,INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "stdafx.h"
#include "str.h"

static byte *row1 = NULL, *row2 = NULL;
static int rowsize = 0;

void R_ResampleTextureLerpLine( byte *in, byte *out, int inwidth, int outwidth, int bytesperpixel ){
	int j, xi, oldx = 0, f, fstep, endx, lerp;
#define LERPBYTE( i ) out[i] = (byte) ( ( ( ( row2[i] - row1[i] ) * lerp ) >> 16 ) + row1[i] )

	fstep = (int) ( inwidth * 65536.0f / outwidth );
	endx = ( inwidth - 1 );
	if ( bytesperpixel == 4 ) {
		for ( j = 0,f = 0; j < outwidth; j++, f += fstep )
		{
			xi = f >> 16;
			if ( xi != oldx ) {
				in += ( xi - oldx ) * 4;
				oldx = xi;
			}

			if ( xi < endx ) {
				lerp = f & 0xFFFF;
				*out++ = (byte) ( ( ( ( in[4] - in[0] ) * lerp ) >> 16 ) + in[0] );
				*out++ = (byte) ( ( ( ( in[5] - in[1] ) * lerp ) >> 16 ) + in[1] );
				*out++ = (byte) ( ( ( ( in[6] - in[2] ) * lerp ) >> 16 ) + in[2] );
				*out++ = (byte) ( ( ( ( in[7] - in[3] ) * lerp ) >> 16 ) + in[3] );
			}
			else // last pixel of the line has no pixel to lerp to
			{
				*out++ = in[0];
				*out++ = in[1];
				*out++ = in[2];
				*out++ = in[3];
			}
		}
	}
	else if ( bytesperpixel == 3 ) {
		for ( j = 0, f = 0; j < outwidth; j++, f += fstep )
		{
			xi = f >> 16;
			if ( xi != oldx ) {
				in += ( xi - oldx ) * 3;
				oldx = xi;
			}

			if ( xi < endx ) {
				lerp = f & 0xFFFF;
				*out++ = (byte) ( ( ( ( in[3] - in[0] ) * lerp ) >> 16 ) + in[0] );
				*out++ = (byte) ( ( ( ( in[4] - in[1] ) * lerp ) >> 16 ) + in[1] );
				*out++ = (byte) ( ( ( ( in[5] - in[2] ) * lerp ) >> 16 ) + in[2] );
			}
			else // last pixel of the line has no pixel to lerp to
			{
				*out++ = in[0];
				*out++ = in[1];
				*out++ = in[2];
			}
		}
	}
	else{
		Sys_Printf( "R_ResampleTextureLerpLine: unsupported bytesperpixel %i\n", bytesperpixel );
	}
}

/*
   ================
   R_ResampleTexture
   ================
 */
void R_ResampleTexture( void *indata, int inwidth, int inheight, void *outdata,  int outwidth, int outheight, int bytesperpixel ){
	if ( rowsize < outwidth * bytesperpixel ) {
		if ( row1 ) {
			free( row1 );
		}
		if ( row2 ) {
			free( row2 );
		}

		rowsize = outwidth * bytesperpixel;
		row1 = (byte *)malloc( rowsize );
		row2 = (byte *)malloc( rowsize );
	}

	if ( bytesperpixel == 4 ) {
		int i, j, yi, oldy, f, fstep, lerp, endy = ( inheight - 1 ), inwidth4 = inwidth * 4, outwidth4 = outwidth * 4;
		byte    *inrow, *out;
		out = (byte *)outdata;
		fstep = (int) ( inheight * 65536.0f / outheight );
#define LERPBYTE( i ) out[i] = (byte) ( ( ( ( row2[i] - row1[i] ) * lerp ) >> 16 ) + row1[i] )

		inrow = (byte *)indata;
		oldy = 0;
		R_ResampleTextureLerpLine( inrow, row1, inwidth, outwidth, bytesperpixel );
		R_ResampleTextureLerpLine( inrow + inwidth4, row2, inwidth, outwidth, bytesperpixel );

		for ( i = 0, f = 0; i < outheight; i++,f += fstep )
		{
			yi = f >> 16;
			if ( yi < endy ) {
				lerp = f & 0xFFFF;
				if ( yi != oldy ) {
					inrow = (byte *)indata + inwidth4 * yi;
					if ( yi == oldy + 1 ) {
						memcpy( row1, row2, outwidth4 );
					}
					else{
						R_ResampleTextureLerpLine( inrow, row1, inwidth, outwidth, bytesperpixel );
					}

					R_ResampleTextureLerpLine( inrow + inwidth4, row2, inwidth, outwidth, bytesperpixel );
					oldy = yi;
				}
				j = outwidth - 4;
				while ( j >= 0 )
				{
					LERPBYTE( 0 );
					LERPBYTE( 1 );
					LERPBYTE( 2 );
					LERPBYTE( 3 );
					LERPBYTE( 4 );
					LERPBYTE( 5 );
					LERPBYTE( 6 );
					LERPBYTE( 7 );
					LERPBYTE( 8 );
					LERPBYTE( 9 );
					LERPBYTE( 10 );
					LERPBYTE( 11 );
					LERPBYTE( 12 );
					LERPBYTE( 13 );
					LERPBYTE( 14 );
					LERPBYTE( 15 );
					out += 16;
					row1 += 16;
					row2 += 16;
					j -= 4;
				}
				if ( j & 2 ) {
					LERPBYTE( 0 );
					LERPBYTE( 1 );
					LERPBYTE( 2 );
					LERPBYTE( 3 );
					LERPBYTE( 4 );
					LERPBYTE( 5 );
					LERPBYTE( 6 );
					LERPBYTE( 7 );
					out += 8;
					row1 += 8;
					row2 += 8;
				}
				if ( j & 1 ) {
					LERPBYTE( 0 );
					LERPBYTE( 1 );
					LERPBYTE( 2 );
					LERPBYTE( 3 );
					out += 4;
					row1 += 4;
					row2 += 4;
				}
				row1 -= outwidth4;
				row2 -= outwidth4;
			}
			else
			{
				if ( yi != oldy ) {
					inrow = (byte *)indata + inwidth4 * yi;
					if ( yi == oldy + 1 ) {
						memcpy( row1, row2, outwidth4 );
					}
					else{
						R_ResampleTextureLerpLine( inrow, row1, inwidth, outwidth, bytesperpixel );
					}

					oldy = yi;
				}
				memcpy( out, row1, outwidth4 );
			}
		}
	}
	else if ( bytesperpixel == 3 ) {
		int i, j, yi, oldy, f, fstep, lerp, endy = ( inheight - 1 ), inwidth3 = inwidth * 3, outwidth3 = outwidth * 3;
		byte    *inrow, *out;
		out = (byte *)outdata;
		fstep = (int) ( inheight * 65536.0f / outheight );
#define LERPBYTE( i ) out[i] = (byte) ( ( ( ( row2[i] - row1[i] ) * lerp ) >> 16 ) + row1[i] )

		inrow = (byte *)indata;
		oldy = 0;
		R_ResampleTextureLerpLine( inrow, row1, inwidth, outwidth, bytesperpixel );
		R_ResampleTextureLerpLine( inrow + inwidth3, row2, inwidth, outwidth, bytesperpixel );
		for ( i = 0, f = 0; i < outheight; i++,f += fstep )
		{
			yi = f >> 16;
			if ( yi < endy ) {
				lerp = f & 0xFFFF;
				if ( yi != oldy ) {
					inrow = (byte *)indata + inwidth3 * yi;
					if ( yi == oldy + 1 ) {
						memcpy( row1, row2, outwidth3 );
					}
					else{
						R_ResampleTextureLerpLine( inrow, row1, inwidth, outwidth, bytesperpixel );
					}

					R_ResampleTextureLerpLine( inrow + inwidth3, row2, inwidth, outwidth, bytesperpixel );
					oldy = yi;
				}
				j = outwidth - 4;
				while ( j >= 0 )
				{
					LERPBYTE( 0 );
					LERPBYTE( 1 );
					LERPBYTE( 2 );
					LERPBYTE( 3 );
					LERPBYTE( 4 );
					LERPBYTE( 5 );
					LERPBYTE( 6 );
					LERPBYTE( 7 );
					LERPBYTE( 8 );
					LERPBYTE( 9 );
					LERPBYTE( 10 );
					LERPBYTE( 11 );
					out += 12;
					row1 += 12;
					row2 += 12;
					j -= 4;
				}
				if ( j & 2 ) {
					LERPBYTE( 0 );
					LERPBYTE( 1 );
					LERPBYTE( 2 );
					LERPBYTE( 3 );
					LERPBYTE( 4 );
					LERPBYTE( 5 );
					out += 6;
					row1 += 6;
					row2 += 6;
				}
				if ( j & 1 ) {
					LERPBYTE( 0 );
					LERPBYTE( 1 );
					LERPBYTE( 2 );
					out += 3;
					row1 += 3;
					row2 += 3;
				}
				row1 -= outwidth3;
				row2 -= outwidth3;
			}
			else
			{
				if ( yi != oldy ) {
					inrow = (byte *)indata + inwidth3 * yi;
					if ( yi == oldy + 1 ) {
						memcpy( row1, row2, outwidth3 );
					}
					else{
						R_ResampleTextureLerpLine( inrow, row1, inwidth, outwidth, bytesperpixel );
					}

					oldy = yi;
				}
				memcpy( out, row1, outwidth3 );
			}
		}
	}
	else{
		Sys_Printf( "R_ResampleTexture: unsupported bytesperpixel %i\n", bytesperpixel );
	}
}

// in can be the same as out
void GL_MipReduce( byte *in, byte *out, int width, int height, int destwidth, int destheight ){
	int x, y, width2, height2, nextrow;
	if ( width > destwidth ) {
		if ( height > destheight ) {
			// reduce both
			width2 = width >> 1;
			height2 = height >> 1;
			nextrow = width << 2;
			for ( y = 0; y < height2; y++ )
			{
				for ( x = 0; x < width2; x++ )
				{
					out[0] = (byte) ( ( in[0] + in[4] + in[nextrow  ] + in[nextrow + 4] ) >> 2 );
					out[1] = (byte) ( ( in[1] + in[5] + in[nextrow + 1] + in[nextrow + 5] ) >> 2 );
					out[2] = (byte) ( ( in[2] + in[6] + in[nextrow + 2] + in[nextrow + 6] ) >> 2 );
					out[3] = (byte) ( ( in[3] + in[7] + in[nextrow + 3] + in[nextrow + 7] ) >> 2 );
					out += 4;
					in += 8;
				}
				in += nextrow; // skip a line
			}
		}
		else
		{
			// reduce width
			width2 = width >> 1;
			for ( y = 0; y < height; y++ )
			{
				for ( x = 0; x < width2; x++ )
				{
					out[0] = (byte) ( ( in[0] + in[4] ) >> 1 );
					out[1] = (byte) ( ( in[1] + in[5] ) >> 1 );
					out[2] = (byte) ( ( in[2] + in[6] ) >> 1 );
					out[3] = (byte) ( ( in[3] + in[7] ) >> 1 );
					out += 4;
					in += 8;
				}
			}
		}
	}
	else
	{
		if ( height > destheight ) {
			// reduce height
			height2 = height >> 1;
			nextrow = width << 2;
			for ( y = 0; y < height2; y++ )
			{
				for ( x = 0; x < width; x++ )
				{
					out[0] = (byte) ( ( in[0] + in[nextrow  ] ) >> 1 );
					out[1] = (byte) ( ( in[1] + in[nextrow + 1] ) >> 1 );
					out[2] = (byte) ( ( in[2] + in[nextrow + 2] ) >> 1 );
					out[3] = (byte) ( ( in[3] + in[nextrow + 3] ) >> 1 );
					out += 4;
					in += 4;
				}
				in += nextrow; // skip a line
			}
		}
		else{
			Sys_Printf( "GL_MipReduce: desired size already achieved\n" );
		}
	}
}
