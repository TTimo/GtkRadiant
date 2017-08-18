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

#include <assert.h>
#include "q3data.h"

static int s_resample_width = 256;
static int s_resample_height = 256;

#define OUTPUT_TGAS         1

#define UNCOMPRESSED        0
#define BTC_COMPRESSION     1

static int s_compression_method = BTC_COMPRESSION;

static const char *CIN_EXTENSION = "cn2";
static const int CIN_SIGNATURE = ( 'C' << 24 ) | ( 'I' << 16 ) | ( 'N' << 8 ) | ( '2' );

static byte *s_soundtrack;
static char s_base[32];
static char s_output_base[32];

/*
   ===============================================================================

   WAV loading

   ===============================================================================
 */

typedef struct
{
	int rate;
	int width;
	int channels;
	int loopstart;
	int samples;
	int dataofs;                // chunk starts this many bytes from file start
} wavinfo_t;


byte    *data_p;
byte    *iff_end;
byte    *last_chunk;
byte    *iff_data;
int iff_chunk_len;


static int s_samplecounts[0x10000];
static wavinfo_t s_wavinfo;

short GetLittleShort( void ){
	short val = 0;
	val = *data_p;
	val = val + ( *( data_p + 1 ) << 8 );
	data_p += 2;
	return val;
}

int GetLittleLong( void ){
	int val = 0;
	val = *data_p;
	val = val + ( *( data_p + 1 ) << 8 );
	val = val + ( *( data_p + 2 ) << 16 );
	val = val + ( *( data_p + 3 ) << 24 );
	data_p += 4;
	return val;
}

void FindNextChunk( char *name ){
	while ( 1 )
	{
		data_p = last_chunk;

		if ( data_p >= iff_end ) { // didn't find the chunk
			data_p = NULL;
			return;
		}

		data_p += 4;
		iff_chunk_len = GetLittleLong();
		if ( iff_chunk_len < 0 ) {
			data_p = NULL;
			return;
		}
//		if (iff_chunk_len > 1024*1024)
//			Sys_Error ("FindNextChunk: %i length is past the 1 meg sanity limit", iff_chunk_len);
		data_p -= 8;
		last_chunk = data_p + 8 + ( ( iff_chunk_len + 1 ) & ~1 );
		if ( !strncmp( data_p, name, 4 ) ) {
			return;
		}
	}
}

void FindChunk( char *name ){
	last_chunk = iff_data;
	FindNextChunk( name );
}


void DumpChunks( void ){
	char str[5];

	str[4] = 0;
	data_p = iff_data;
	do
	{
		memcpy( str, data_p, 4 );
		data_p += 4;
		iff_chunk_len = GetLittleLong();
		printf( "0x%x : %s (%d)\n", (int)( data_p - 4 ), str, iff_chunk_len );
		data_p += ( iff_chunk_len + 1 ) & ~1;
	} while ( data_p < iff_end );
}

/*
   ============
   GetWavinfo
   ============
 */
wavinfo_t GetWavinfo( char *name, byte *wav, int wavlength ){
	wavinfo_t info;
	int i;
	int format;
	int samples;

	memset( &info, 0, sizeof( info ) );

	if ( !wav ) {
		return info;
	}

	iff_data = wav;
	iff_end = wav + wavlength;

// find "RIFF" chunk
	FindChunk( "RIFF" );
	if ( !( data_p && !strncmp( data_p + 8, "WAVE", 4 ) ) ) {
		printf( "Missing RIFF/WAVE chunks\n" );
		return info;
	}

// get "fmt " chunk
	iff_data = data_p + 12;
// DumpChunks ();

	FindChunk( "fmt " );
	if ( !data_p ) {
		printf( "Missing fmt chunk\n" );
		return info;
	}
	data_p += 8;
	format = GetLittleShort();
	if ( format != 1 ) {
		printf( "Microsoft PCM format only\n" );
		return info;
	}

	info.channels = GetLittleShort();
	info.rate = GetLittleLong();
	data_p += 4 + 2;
	info.width = GetLittleShort() / 8;

// get cue chunk
	FindChunk( "cue " );
	if ( data_p ) {
		data_p += 32;
		info.loopstart = GetLittleLong();
//		Com_Printf("loopstart=%d\n", sfx->loopstart);

		// if the next chunk is a LIST chunk, look for a cue length marker
		FindNextChunk( "LIST" );
		if ( data_p ) {
			if ( !strncmp( data_p + 28, "mark", 4 ) ) { // this is not a proper parse, but it works with cooledit...
				data_p += 24;
				i = GetLittleLong();    // samples in loop
				info.samples = info.loopstart + i;
			}
		}
	}
	else{
		info.loopstart = -1;
	}

// find data chunk
	FindChunk( "data" );
	if ( !data_p ) {
		printf( "Missing data chunk\n" );
		return info;
	}

	data_p += 4;
	samples = GetLittleLong();

	if ( info.samples ) {
		if ( samples < info.samples ) {
			Error( "Sound %s has a bad loop length", name );
		}
	}
	else{
		info.samples = samples;
	}

	info.dataofs = data_p - wav;

	return info;
}

//=====================================================================

/*
   ==============
   LoadSoundtrack
   ==============
 */
void LoadSoundtrack( void ){
	char name[1024];
	FILE    *f;
	int len;
	int i, val, j;

	s_soundtrack = NULL;
	sprintf( name, "%svideo/%s/%s.wav", gamedir, s_base, s_base );
	printf( "WAV: %s\n", name );
	f = fopen( name, "rb" );
	if ( !f ) {
		printf( "no soundtrack for %s\n", s_base );
		return;
	}
	len = Q_filelength( f );
	s_soundtrack = malloc( len );
	fread( s_soundtrack, 1, len, f );
	fclose( f );

	s_wavinfo = GetWavinfo( name, s_soundtrack, len );

	// count samples for compression
	memset( s_samplecounts, 0, sizeof( s_samplecounts ) );

	j = s_wavinfo.samples / 2;
	for ( i = 0 ; i < j ; i++ )
	{
		val = ( (unsigned short *)( s_soundtrack + s_wavinfo.dataofs ) )[i];
		s_samplecounts[val]++;
	}
	val = 0;
	for ( i = 0 ; i < 0x10000 ; i++ )
		if ( s_samplecounts[i] ) {
			val++;
		}

	printf( "%i unique sample values\n", val );
}

/*
   ==================
   WriteSound
   ==================
 */
void WriteSound( FILE *output, int frame ){
	int start, end;
	int count;
	int empty = 0;
	int i;
	int sample;
	int width;

	width = s_wavinfo.width * s_wavinfo.channels;

	start = frame * s_wavinfo.rate / 14;
	end = ( frame + 1 ) * s_wavinfo.rate / 14;
	count = end - start;

	for ( i = 0 ; i < count ; i++ )
	{
		sample = start + i;
		if ( sample > s_wavinfo.samples || !s_soundtrack ) {
			fwrite( &empty, 1, width, output );
		}
		else{
			fwrite( s_soundtrack + s_wavinfo.dataofs + sample * width, 1, width,output );
		}
	}
}

//==========================================================================

static float s_resampleXRatio;
static float s_resampleYRatio;

static void BoxFilterHorizontalElements( unsigned char *dst, unsigned char *src, float s0, float s1 ){
	float w;
	float rSum = 0, gSum = 0, bSum = 0;
	float x = s0;
	float sumWeight = 0;

	for ( x = s0; x < s1; x++, src += 4 )
	{
		if ( x == s0 ) {
			w = ( int ) ( s0 + 1 ) - x;
		}
		else if ( x + 1 >= s1 ) {
			w = s1 - ( int ) x;
		}
		else
		{
			w = 1.0f;
		}

		rSum += src[0] * w;
		gSum += src[1] * w;
		bSum += src[2] * w;
		sumWeight += w;
	}

	rSum /= sumWeight;
	gSum /= sumWeight;
	bSum /= sumWeight;

	dst[0] = ( unsigned char ) ( rSum + 0.5 );
	dst[1] = ( unsigned char ) ( gSum + 0.5 );
	dst[2] = ( unsigned char ) ( bSum + 0.5 );
}

static void BoxFilterVerticalElements( unsigned char *dst, // destination of the filter process
									   unsigned char *src, // source pixels
									   int srcStep,        // stride of the source pixels
									   float s0, float s1 ){
	float w;
	float rSum = 0, gSum = 0, bSum = 0;
	float y = s0;
	float sumWeight = 0;

	for ( y = s0; y < ( int ) ( s1 + 1 ) ; y++, src += srcStep )
	{
		if ( y == s0 ) {
			w = ( int ) ( s0 + 1 ) - y;
		}
		else if ( y + 1 >= s1 ) {
			w = s1 - ( int ) y;
		}
		else
		{
			w = 1.0f;
		}

		rSum += src[0] * w;
		gSum += src[1] * w;
		bSum += src[2] * w;
		sumWeight += w;
	}

	rSum /= sumWeight;
	gSum /= sumWeight;
	bSum /= sumWeight;

	dst[0] = ( unsigned char ) ( rSum + 0.5 );
	dst[1] = ( unsigned char ) ( gSum + 0.5 );
	dst[2] = ( unsigned char ) ( bSum + 0.5 );
	dst[3] = 0xff;

}

static void BoxFilterRow( unsigned char *dstStart, cblock_t *in, int dstRow, int rowWidth ){
	int i;
	unsigned char *indata = ( unsigned char * ) in->data;

	indata += 4 * dstRow * in->width;

	for ( i = 0; i < rowWidth; i++ )
	{
		float c0 = i * s_resampleXRatio;
		float c1 = ( i + 1 ) * s_resampleXRatio;

		BoxFilterHorizontalElements( &dstStart[i * 4], &indata[( ( int ) c0 ) * 4], c0, c1 );
	}
}

static void BoxFilterColumn( unsigned char *dstStart, unsigned char *srcStart, int dstCol, int dstRowWidth, int dstColHeight, int srcRowWidthInPels ){
	float c0, c1;
	int i;

	for ( i = 0; i < dstColHeight; i++ )
	{
		c0 = i * s_resampleYRatio;
		c1 = ( i + 1 ) * s_resampleYRatio;

		BoxFilterVerticalElements( &dstStart[i * 4 * dstRowWidth], &srcStart[(int)c0 * srcRowWidthInPels * 4], srcRowWidthInPels * 4, c0, c1 );
	}
}

#define DROP_SAMPLE     0
#define BOX_FILTER      1

static void ResampleFrame( cblock_t *in, unsigned char *out, int method, int outWidth, int outHeight ){
	int row, column;
	unsigned char *indata = ( unsigned char * ) in->data;

	s_resampleXRatio = in->width / ( float ) outWidth;
	s_resampleYRatio = in->height / ( float ) outHeight;

	if ( method == DROP_SAMPLE ) {
		for ( row = 0; row < outHeight; row++ )
		{
			int r = ( int ) ( row * s_resampleYRatio );

			for ( column = 0; column < outWidth; column++ )
			{
				int c = ( int ) ( column * s_resampleXRatio );

				out[( row * outWidth + column ) * 4 + 0] = indata[( r * in->width + c ) * 4 + 0];
				out[( row * outWidth + column ) * 4 + 1] = indata[( r * in->width + c ) * 4 + 1];
				out[( row * outWidth + column ) * 4 + 2] = indata[( r * in->width + c ) * 4 + 2];
				out[( row * outWidth + column ) * 4 + 3] = 0xff;
			}
		}
	}
	else if ( method == BOX_FILTER ) {
		unsigned char intermediate[1024 * 1024 * 4];

		assert( in->height <= 1024 );
		assert( in->width <= 1024 );

		//
		// filter our M x N source image into a RESAMPLE_WIDTH x N horizontally filtered image
		//
		for ( row = 0; row < in->height; row++ )
		{
			BoxFilterRow( &intermediate[row * 4 * outWidth], in, row, outWidth );
		}

		//
		// filter our RESAMPLE_WIDTH x N horizontally filtered image into a RESAMPLE_WIDTH x RESAMPLE_HEIGHT filtered image
		//
		for ( column = 0; column < outWidth; column++ )
		{
			BoxFilterColumn( &out[column * 4], &intermediate[column * 4], column, outWidth, outHeight, s_resample_width );
		}
	}
}

static float BTCDistanceSquared( float a[3], float b[3] ){
	return ( b[0] - a[0] ) * ( b[0] - a[0] ) +
		   ( b[1] - a[1] ) * ( b[1] - a[1] ) +
		   ( b[2] - a[2] ) * ( b[2] - a[2] );
}

static void BTCFindEndpoints( float inBlock[4][4][3], unsigned int endPoints[2][2] ){
	float longestDistance = -1;

	int bX, bY;

	//
	// find the two points farthest from each other
	//
	for ( bY = 0; bY < 4; bY++ )
	{
		for ( bX = 0; bX < 4; bX++ )
		{
			int cX, cY;
			float d;

			//
			// check the rest of the current row
			//
			for ( cX = bX + 1; cX < 4; cX++ )
			{
				if ( ( d = BTCDistanceSquared( inBlock[bY][bX], inBlock[bY][cX] ) ) > longestDistance ) {
					longestDistance = d;
					endPoints[0][0] = bX;
					endPoints[0][1] = bY;
					endPoints[1][0] = cX;
					endPoints[1][1] = bY;
				}
			}

			//
			// check remaining rows and columns
			//
			for ( cY = bY + 1; cY < 4; cY++ )
			{
				for ( cX = 0; cX < 4; cX++ )
				{
					if ( ( d = BTCDistanceSquared( inBlock[bY][bX], inBlock[cY][cX] ) ) > longestDistance ) {
						longestDistance = d;
						endPoints[0][0] = bX;
						endPoints[0][1] = bY;
						endPoints[1][0] = cX;
						endPoints[1][1] = cY;
					}
				}
			}
		}
	}
}

static float BTCQuantizeBlock( float inBlock[4][4][3], unsigned long endPoints[2][2], int btcQuantizedBlock[4][4], float bestError ){
	int i;
	int blockY, blockX;
	float dR, dG, dB;
	float R, G, B;
	float error = 0;
	float colorLine[4][3];

	//
	// build the color line
	//
	dR = inBlock[endPoints[1][1]][endPoints[1][0]][0] -
		 inBlock[endPoints[0][1]][endPoints[0][0]][0];
	dG = inBlock[endPoints[1][1]][endPoints[1][0]][1] -
		 inBlock[endPoints[0][1]][endPoints[0][0]][1];
	dB = inBlock[endPoints[1][1]][endPoints[1][0]][2] -
		 inBlock[endPoints[0][1]][endPoints[0][0]][2];

	dR *= 0.33f;
	dG *= 0.33f;
	dB *= 0.33f;

	R = inBlock[endPoints[0][1]][endPoints[0][0]][0];
	G = inBlock[endPoints[0][1]][endPoints[0][0]][1];
	B = inBlock[endPoints[0][1]][endPoints[0][0]][2];

	for ( i = 0; i < 4; i++ )
	{
		colorLine[i][0] = R;
		colorLine[i][1] = G;
		colorLine[i][2] = B;

		R += dR;
		G += dG;
		B += dB;
	}

	//
	// quantize each pixel into the appropriate range
	//
	for ( blockY = 0; blockY < 4; blockY++ )
	{
		for ( blockX = 0; blockX < 4; blockX++ )
		{
			float distance = 10000000000;
			int shortest = -1;

			for ( i = 0; i < 4; i++ )
			{
				float d;

				if ( ( d = BTCDistanceSquared( inBlock[blockY][blockX], colorLine[i] ) ) < distance ) {
					distance = d;
					shortest = i;
				}
			}

			error += distance;

			//
			// if bestError is not -1 then that means this is a speculative quantization
			//
			if ( bestError != -1 ) {
				if ( error > bestError ) {
					return error;
				}
			}

			btcQuantizedBlock[blockY][blockX] = shortest;
		}
	}

	return error;
}

/*
** float BTCCompressBlock
*/
static float BTCCompressBlock( float inBlock[4][4][3], unsigned long out[2] ){
	int i;
	int btcQuantizedBlock[4][4];    // values should be [0..3]
	unsigned long encodedEndPoints, encodedBitmap;
	unsigned long endPoints[2][2];       // endPoints[0] = color start, endPoints[1] = color end
	int blockY, blockX;
	float error = 0;
	float bestError = 10000000000;
	unsigned long bestEndPoints[2][2];

#if 0
	//
	// find the "ideal" end points for the color vector
	//
	BTCFindEndpoints( inBlock, endPoints );
	error = BTCQuantizeBlock( inBlock, endPoints, btcQuantizedBlock );
	memcpy( bestEndPoints, endPoints, sizeof( bestEndPoints ) );
#else
	for ( blockY = 0; blockY < 4; blockY++ )
	{
		for ( blockX = 0; blockX < 4; blockX++ )
		{
			int x2, y2;

			for ( y2 = 0; y2 < 4; y2++ )
			{
				for ( x2 = 0; x2 < 4; x2++ )
				{
					if ( ( x2 == blockX ) && ( y2 == blockY ) ) {
						continue;
					}

					endPoints[0][0] = blockX;
					endPoints[0][1] = blockY;
					endPoints[1][0] = x2;
					endPoints[1][1] = y2;

					error = BTCQuantizeBlock( inBlock, endPoints, btcQuantizedBlock, -1 ); //bestError );

					if ( error < bestError ) {
						bestError = error;
						memcpy( bestEndPoints, endPoints, sizeof( bestEndPoints ) );
					}
				}
			}
		}
	}

	error = BTCQuantizeBlock( inBlock, bestEndPoints, btcQuantizedBlock, -1.0f );
#endif

	//
	// encode the results
	//
	encodedBitmap = 0;
	for ( blockY = 0; blockY < 4; blockY++ )
	{
		for ( blockX = 0; blockX < 4; blockX++ )
		{
			int shift = ( blockX + blockY * 4 ) * 2;
			encodedBitmap |= btcQuantizedBlock[blockY][blockX] << shift;
		}
	}

	//
	// encode endpoints
	//
	encodedEndPoints = 0;
	for ( i = 0; i < 2; i++ )
	{
		int iR, iG, iB;

		iR = ( ( int ) inBlock[bestEndPoints[i][1]][bestEndPoints[i][0]][0] );
		if ( iR > 255 ) {
			iR = 255;
		}
		else if ( iR < 0 ) {
			iR = 0;
		}
		iR >>= 3;

		iG = ( ( int ) inBlock[bestEndPoints[i][1]][bestEndPoints[i][0]][1] );
		if ( iG > 255 ) {
			iG = 255;
		}
		else if ( iG < 0 ) {
			iG = 0;
		}
		iG >>= 2;

		iB = ( ( int ) inBlock[bestEndPoints[i][1]][bestEndPoints[i][0]][2] );
		if ( iB > 255 ) {
			iB = 255;
		}
		else if ( iB < 0 ) {
			iB = 0;
		}
		iB >>= 3;


		encodedEndPoints |= ( ( ( iR << 11 ) | ( iG << 5 ) | ( iB ) ) << ( i * 16 ) );
	}

	//
	// store
	//
	out[0] = encodedBitmap;
	out[1] = encodedEndPoints;

	return error;
}

/*
** void BTCDecompressFrame
*/
static void BTCDecompressFrame( unsigned long *src, unsigned char *dst ){
	int x, y;
	int iR, iG, iB;
	int dstX, dstY;
	float colorStart[3], colorEnd[3];
	unsigned char colorRampABGR[4][4];
	unsigned encoded;

	memset( colorRampABGR, 0xff, sizeof( colorRampABGR ) );

	for ( y = 0; y < s_resample_height / 4; y++ )
	{
		for ( x = 0; x < s_resample_width / 4; x++ )
		{
			unsigned colorStartPacked = src[( y * s_resample_width / 4 + x ) * 2 + 1] & 0xffff;
			unsigned colorEndPacked = src[( y * s_resample_width / 4 + x ) * 2 + 1] >> 16;

			//
			// grab the end points
			//   0 = color start
			//   1 = color end
			//
			iR = ( ( colorStartPacked >> 11 ) & ( ( 1 << 5 ) - 1 ) );
			iR = ( iR << 3 ) | ( iR >> 2 );
			iG = ( ( colorStartPacked >> 5 ) & ( ( 1 << 6 )  - 1 ) );
			iG = ( iG << 2 ) | ( iG >> 4 );
			iB = ( ( colorStartPacked ) & ( ( 1 << 5  ) - 1 ) );
			iB = ( iB << 3 ) | ( iB >> 2 );

			colorStart[0] = iR;
			colorStart[1] = iG;
			colorStart[2] = iB;
			colorRampABGR[0][0] = iR;
			colorRampABGR[0][1] = iG;
			colorRampABGR[0][2] = iB;

			iR = ( ( colorEndPacked >> 11 ) & ( ( 1 << 5 ) - 1 ) );
			iR = ( iR << 3 ) | ( iR >> 2 );
			iG = ( ( colorEndPacked >> 5 ) & ( ( 1 << 6 )  - 1 ) );
			iG = ( iG << 2 ) | ( iG >> 4 );
			iB = ( colorEndPacked & ( ( 1 << 5  ) - 1 ) );
			iB = ( iB << 3 ) | ( iB >> 2 );

			colorEnd[0] = iR;
			colorEnd[1] = iG;
			colorEnd[2] = iB;
			colorRampABGR[3][0] = iR;
			colorRampABGR[3][1] = iG;
			colorRampABGR[3][2] = iB;

			//
			// compute this block's color ramp
			// FIXME: This needs to be reversed on big-endian machines
			//

			colorRampABGR[1][0] = colorStart[0] * 0.66f + colorEnd[0] * 0.33f;
			colorRampABGR[1][1] = colorStart[1] * 0.66f + colorEnd[1] * 0.33f;
			colorRampABGR[1][2] = colorStart[2] * 0.66f + colorEnd[2] * 0.33f;

			colorRampABGR[2][0] = colorStart[0] * 0.33f + colorEnd[0] * 0.66f;
			colorRampABGR[2][1] = colorStart[1] * 0.33f + colorEnd[1] * 0.66f;
			colorRampABGR[2][2] = colorStart[2] * 0.33f + colorEnd[2] * 0.66f;

			//
			// decode the color data
			// information is encoded in 2-bit pixels, with low order bits corresponding
			// to upper left pixels.  These 2-bit values are indexed into the block's
			// computer color ramp.
			//
			encoded = src[( y * s_resample_width / 4 + x ) * 2 + 0];

			for ( dstY = 0; dstY < 4; dstY++ )
			{
				for ( dstX = 0; dstX < 4; dstX++ )
				{
					memcpy( &dst[( y * 4 + dstY ) * s_resample_width * 4 + x * 4 * 4 + dstX * 4], colorRampABGR[encoded & 3], sizeof( colorRampABGR[0] ) );
					encoded >>= 2;
				}
			}
		}
	}
}

/*
** BTCCompressFrame
**
** Perform a BTC compression using a 2-bit encoding at each pixel.  This
** compression method is performed by decomposing the incoming image into
** a sequence of 4x4 blocks.  At each block two color values are computed
** that define the endpoints of a vector in color space that represent
** the two colors "farthest apart".
*/
static float BTCCompressFrame( unsigned char *src, unsigned long *dst ){
	int x, y;
	int bX, bY;
	float btcBlock[4][4][3];

	float error = 0;

	for ( y = 0; y < s_resample_height / 4; y++ )
	{
		for ( x = 0; x < s_resample_width / 4; x++ )
		{
			//
			// fill in the BTC block with raw values
			//
			for ( bY = 0; bY < 4; bY++ )
			{
				for ( bX = 0; bX < 4; bX++ )
				{
					btcBlock[bY][bX][0] = src[( y * 4 + bY ) * s_resample_width * 4 + ( x * 4 + bX ) * 4 + 0];
					btcBlock[bY][bX][1] = src[( y * 4 + bY ) * s_resample_width * 4 + ( x * 4 + bX ) * 4 + 1];
					btcBlock[bY][bX][2] = src[( y * 4 + bY ) * s_resample_width * 4 + ( x * 4 + bX ) * 4 + 2];
				}
			}

			error += BTCCompressBlock( btcBlock, &dst[( y * s_resample_width / 4 + x ) * 2] );
		}
	}

	return error / ( ( s_resample_width / 4 ) * ( s_resample_height / 4 ) );
}

/*
   ===================
   LoadFrame
   ===================
 */
cblock_t LoadFrame( char *base, int frame, int digits, byte **palette ){
	int ten3, ten2, ten1, ten0;
	cblock_t in;
	int width, height;
	char name[1024];
	FILE        *f;

	in.data = NULL;
	in.count = -1;

	ten3 = frame / 1000;
	ten2 = ( frame - ten3 * 1000 ) / 100;
	ten1 = ( frame - ten3 * 1000 - ten2 * 100 ) / 10;
	ten0 = frame % 10;

	if ( digits == 4 ) {
		sprintf( name, "%svideo/%s/%s%i%i%i%i.tga", gamedir, base, base, ten3, ten2, ten1, ten0 );
	}
	else{
		sprintf( name, "%svideo/%s/%s%i%i%i.tga", gamedir, base, base, ten2, ten1, ten0 );
	}

	f = fopen( name, "rb" );
	if ( !f ) {
		in.data = NULL;
		return in;
	}
	fclose( f );

	printf( "%s", name );
	LoadTGA( name, ( unsigned char ** ) &in.data, &width, &height );
	if ( palette ) {
		*palette = 0;
	}
//	Load256Image (name, &in.data, palette, &width, &height);
	in.count = width * height;
	in.width = width;
	in.height = height;
// FIXME: map 0 and 255!

#if 0
	// rle compress
	rle = RLE( in );
	free( in.data );

	return rle;
#endif

	return in;
}

/*
   ===============
   Cmd_Video

   video <directory> <framedigits>
   ===============
 */
void Cmd_Video( void ){
	float sumError = 0, error = 0, maxError = 0;
	char savename[1024];
	char name[1024];
	FILE    *output;
	int startframe, frame;
	int width, height;
	int i;
	int digits;
	int minutes;
	float fseconds;
	int remSeconds;
	cblock_t in;
	unsigned char *resampled;
	unsigned long *compressed;
	clock_t start, stop;

	GetToken( qfalse );
	strcpy( s_base, token );
	if ( g_release ) {
//		sprintf (savename, "video/%s.cin", token);
//		ReleaseFile (savename);
		return;
	}

	GetToken( qfalse );
	strcpy( s_output_base, token );

	GetToken( qfalse );
	digits = atoi( token );

	GetToken( qfalse );

	if ( !strcmp( token, "btc" ) ) {
		s_compression_method = BTC_COMPRESSION;
		printf( "Compression: BTC\n" );
	}
	else if ( !strcmp( token, "uc" ) ) {
		s_compression_method = UNCOMPRESSED;
		printf( "Compression: none\n" );
	}
	else
	{
		Error( "Uknown compression method '%s'\n", token );
	}

	GetToken( qfalse );
	s_resample_width = atoi( token );

	GetToken( qfalse );
	s_resample_height = atoi( token );

	resampled = malloc( sizeof( unsigned char ) * 4 * s_resample_width * s_resample_height );
	compressed = malloc( sizeof( long ) * 2 * ( s_resample_width / 4 ) * ( s_resample_height / 4 ) );

	printf( "Resample width: %d\n", s_resample_width );
	printf( "Resample height: %d\n", s_resample_height );

	// optionally skip frames
	if ( TokenAvailable() ) {
		GetToken( qfalse );
		startframe = atoi( token );
	}
	else{
		startframe = 0;
	}

	sprintf( savename, "%svideo/%s.%s", writedir, s_output_base, CIN_EXTENSION );

	// load the entire sound wav file if present
	LoadSoundtrack();

	if ( digits == 4 ) {
		sprintf( name, "%svideo/%s/%s0000.tga", gamedir, s_base, s_base );
	}
	else{
		sprintf( name, "%svideo/%s/%s000.tga", gamedir, s_base, s_base );
	}

	printf( "%s\n", name );
	LoadTGA( name, NULL, &width, &height );

	output = fopen( savename, "wb" );
	if ( !output ) {
		Error( "Can't open %s", savename );
	}

	// write header info
	i = LittleLong( CIN_SIGNATURE );
	fwrite( &i, 4, 1, output );
	i = LittleLong( s_resample_width );
	fwrite( &i, 4, 1, output );
	i = LittleLong( s_resample_height );
	fwrite( &i, 4, 1, output );
	i = LittleLong( s_wavinfo.rate );
	fwrite( &i, 4, 1, output );
	i = LittleLong( s_wavinfo.width );
	fwrite( &i, 4, 1, output );
	i = LittleLong( s_wavinfo.channels );
	fwrite( &i, 4, 1, output );
	i = LittleLong( s_compression_method );
	fwrite( &i, 4, 1, output );

	start = clock();

	// perform compression on a per frame basis
	for ( frame = startframe ;  ; frame++ )
	{
		printf( "%02d: ", frame );
		in = LoadFrame( s_base, frame, digits, 0 );
		if ( !in.data ) {
			break;
		}

		ResampleFrame( &in, ( unsigned char * ) resampled, BOX_FILTER, s_resample_width, s_resample_height );

		if ( s_compression_method == UNCOMPRESSED ) {
			printf( "\n" );
			fwrite( resampled, 1, sizeof( unsigned char ) * s_resample_width * s_resample_height * 4, output );

#if OUTPUT_TGAS
			{
				int x, y;
				char buffer[1000];

				for ( y = 0; y < s_resample_height / 2; y++ )
				{
					for ( x = 0; x < s_resample_width; x++ )
					{
						unsigned char tmp[4];

						tmp[0] = resampled[( s_resample_height - 1 - y ) * s_resample_width * 4 + x * 4 + 0];
						tmp[1] = resampled[( s_resample_height - 1 - y ) * s_resample_width * 4 + x * 4 + 1];
						tmp[2] = resampled[( s_resample_height - 1 - y ) * s_resample_width * 4 + x * 4 + 2];
						tmp[3] = resampled[( s_resample_height - 1 - y ) * s_resample_width * 4 + x * 4 + 3];

						resampled[( s_resample_height - 1 - y ) * s_resample_width * 4 + x * 4 + 0] = resampled[y * s_resample_width * 4 + x * 4 + 0];
						resampled[( s_resample_height - 1 - y ) * s_resample_width * 4 + x * 4 + 1] = resampled[y * s_resample_width * 4 + x * 4 + 1];
						resampled[( s_resample_height - 1 - y ) * s_resample_width * 4 + x * 4 + 2] = resampled[y * s_resample_width * 4 + x * 4 + 2];
						resampled[( s_resample_height - 1 - y ) * s_resample_width * 4 + x * 4 + 3] = resampled[y * s_resample_width * 4 + x * 4 + 3];

						resampled[y * s_resample_width * 4 + x * 4 + 0] = tmp[0];
						resampled[y * s_resample_width * 4 + x * 4 + 1] = tmp[1];
						resampled[y * s_resample_width * 4 + x * 4 + 2] = tmp[2];
						resampled[y * s_resample_width * 4 + x * 4 + 3] = tmp[3];
					}
				}

				sprintf( buffer, "%svideo/%s/uc%04d.tga", gamedir, s_base, frame );
				WriteTGA( buffer, resampled, s_resample_width, s_resample_height );
			}
#endif
		}
		else if ( s_compression_method == BTC_COMPRESSION ) {
			error = BTCCompressFrame( resampled, compressed );

			sumError += error;

			if ( error > maxError ) {
				maxError = error;
			}

			printf( " (error = %f)\n", error );
			fwrite( compressed, 1, 2 * sizeof( long ) * ( s_resample_width / 4 ) * ( s_resample_height / 4 ), output );

#if OUTPUT_TGAS
			{
				int x, y;
				unsigned char *uncompressed;
				char buffer[1000];

				uncompressed = malloc( sizeof( unsigned char ) * 4 * s_resample_width * s_resample_height );
				BTCDecompressFrame( compressed, uncompressed );

				for ( y = 0; y < s_resample_height / 2; y++ )
				{
					for ( x = 0; x < s_resample_width; x++ )
					{
						unsigned char tmp[4];

						tmp[0] = uncompressed[( s_resample_height - 1 - y ) * s_resample_width * 4 + x * 4 + 0];
						tmp[1] = uncompressed[( s_resample_height - 1 - y ) * s_resample_width * 4 + x * 4 + 1];
						tmp[2] = uncompressed[( s_resample_height - 1 - y ) * s_resample_width * 4 + x * 4 + 2];
						tmp[3] = uncompressed[( s_resample_height - 1 - y ) * s_resample_width * 4 + x * 4 + 3];

						uncompressed[( s_resample_height - 1 - y ) * s_resample_width * 4 + x * 4 + 0] = uncompressed[y * s_resample_width * 4 + x * 4 + 0];
						uncompressed[( s_resample_height - 1 - y ) * s_resample_width * 4 + x * 4 + 1] = uncompressed[y * s_resample_width * 4 + x * 4 + 1];
						uncompressed[( s_resample_height - 1 - y ) * s_resample_width * 4 + x * 4 + 2] = uncompressed[y * s_resample_width * 4 + x * 4 + 2];
						uncompressed[( s_resample_height - 1 - y ) * s_resample_width * 4 + x * 4 + 3] = uncompressed[y * s_resample_width * 4 + x * 4 + 3];

						uncompressed[y * s_resample_width * 4 + x * 4 + 0] = tmp[0];
						uncompressed[y * s_resample_width * 4 + x * 4 + 1] = tmp[1];
						uncompressed[y * s_resample_width * 4 + x * 4 + 2] = tmp[2];
						uncompressed[y * s_resample_width * 4 + x * 4 + 3] = tmp[3];
					}
				}


				sprintf( buffer, "%svideo/%s/btc%04d.tga", gamedir, s_base, frame );
				WriteTGA( buffer, uncompressed, s_resample_width, s_resample_height );

				free( uncompressed );
			}
#endif
		}

		WriteSound( output, frame );

		free( in.data );
	}
	stop = clock();

	printf( "\n" );

	printf( "Total size: %ld\n", ftell( output ) );
	printf( "Average error: %f\n", sumError / ( frame - startframe ) );
	printf( "Max error: %f\n", maxError );

	fseconds = ( stop - start ) / 1000.0f;
	minutes = fseconds / 60;
	remSeconds = fseconds - minutes * 60;

	printf( "Total time: %d s (%d m %d s)\n", ( int ) fseconds, minutes, remSeconds );
	printf( "Time/frame: %.2f seconds\n", fseconds / ( frame - startframe ) );

	fclose( output );

	if ( s_soundtrack ) {
		free( s_soundtrack );
		s_soundtrack = 0;
	}
}
