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

// To do

// Sound error handling (when sound too short)
// rle b4 huffing
// adpcm encoding of sound

#if 0
#include "qdata.h"
#include "flex.h"
#include "fc.h"
#include "adpcm.h"

#define MIN_REPT    15
#define MAX_REPT    0
#define HUF_TOKENS  ( 256 + MAX_REPT )

#define BLOCKSIZE   8

#define M_PI        3.14159265358979323846  // matches value in gcc v2 math.h
#define SQRT2       1.414213562

typedef struct hnode_s
{
	int count;
	qboolean used;
	int children[2];
} hnode_t;

typedef struct
{
	int rate;
	int width;
	int channels;
	int loopstart;
	int samples;
	int dataofs;                // chunk starts this many bytes from file start
} wavinfo_t;

// These weren`t picked out my ass....
// They were defined at http://www.rahul.net/jfm/dct.html
// However, I think he plucked them out of his ass.....

float Quantise[BLOCKSIZE * BLOCKSIZE];

float LUT_Quantise[BLOCKSIZE * BLOCKSIZE] =
{
	16.0F / 16.0F, 11.0F / 16.0F, 10.0F / 16.0F, 16.0F / 16.0F, 24.0F / 16.0F, 40.0F / 16.0F, 51.0F / 16.0F, 61.0F / 16.0F,
	12.0F / 16.0F, 13.0F / 16.0F, 14.0F / 16.0F, 19.0F / 16.0F, 26.0F / 16.0F, 58.0F / 16.0F, 60.0F / 16.0F, 55.0F / 16.0F,
	14.0F / 16.0F, 13.0F / 16.0F, 16.0F / 16.0F, 24.0F / 16.0F, 40.0F / 16.0F, 57.0F / 16.0F, 69.0F / 16.0F, 56.0F / 16.0F,
	14.0F / 16.0F, 17.0F / 16.0F, 22.0F / 16.0F, 29.0F / 16.0F, 51.0F / 16.0F, 87.0F / 16.0F, 80.0F / 16.0F, 62.0F / 16.0F,
	18.0F / 16.0F, 22.0F / 16.0F, 37.0F / 16.0F, 56.0F / 16.0F, 68.0F / 16.0F,109.0F / 16.0F,103.0F / 16.0F, 77.0F / 16.0F,
	24.0F / 16.0F, 35.0F / 16.0F, 55.0F / 16.0F, 64.0F / 16.0F, 81.0F / 16.0F,104.0F / 16.0F,113.0F / 16.0F, 92.0F / 16.0F,
	49.0F / 16.0F, 64.0F / 16.0F, 78.0F / 16.0F, 87.0F / 16.0F,103.0F / 16.0F,121.0F / 16.0F,120.0F / 16.0F,101.0F / 16.0F,
	72.0F / 16.0F, 92.0F / 16.0F, 95.0F / 16.0F, 98.0F / 16.0F,112.0F / 16.0F,100.0F / 16.0F,103.0F / 16.0F, 99.0F / 16.0F
};

int LUT_ZZ[BLOCKSIZE * BLOCKSIZE] =
{
	0,
	1,  8,
	16,  9,  2,
	3, 10, 17, 24,
	32, 25, 18, 11,  4,
	5, 12, 19, 26, 33, 40,
	48, 41, 34, 27, 20, 13, 6,
	7, 14, 21, 28, 35, 42, 49, 56,
	57, 50, 43, 36, 29, 22, 15,
	23, 30, 37, 44, 51, 58,
	59, 52, 45, 38, 31,
	39, 46, 53, 60,
	61, 54, 47,
	55, 62,
	63
};

char base[32];

byte            *soundtrack;

byte scaled[256][HUF_TOKENS];
unsigned int charbits1[256][HUF_TOKENS];
int charbitscount1[256][HUF_TOKENS];
hnode_t hnodes1[256][HUF_TOKENS * 2];
int numhnodes1[256];
int order0counts[256];
int numhnodes;
hnode_t hnodes[512];
unsigned charbits[256];
int charbitscount[256];

CineHead_t cinehead;

byte            *data_p;
byte            *iff_end;
byte            *last_chunk;
byte            *iff_data;
int iff_chunk_len;

float dctbase[BLOCKSIZE][BLOCKSIZE];
float red[BLOCKSIZE * BLOCKSIZE];
float green[BLOCKSIZE * BLOCKSIZE];
float blue[BLOCKSIZE * BLOCKSIZE];
float temp[BLOCKSIZE * BLOCKSIZE];

wavinfo_t wavinfo;
adpcm_t adpcm;

/*
   ===============================================================================

   WAV loading

   ===============================================================================
 */

/* Intel ADPCM step variation table */
static int indexTable[16] =
{
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8,
};

static int stepsizeTable[89] =
{
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

#if 0
static void adpcm_decoder( char *indata, short *outdata, int len, adpcm_state_t *state ){
	signed char *inp;       /* Input buffer pointer */
	short *outp;        /* output buffer pointer */
	int sign;           /* Current adpcm sign bit */
	int delta;          /* Current adpcm output value */
	int step;           /* Stepsize */
	int valpred;        /* Predicted value */
	int vpdiff;         /* Current change to valpred */
	int index;          /* Current step change index */
	int inputbuffer;        /* place to keep next 4-bit value */
	int bufferstep;     /* toggle between inputbuffer/input */

	outp = outdata;
	inp = (signed char *)indata;

	valpred = state->valprev;
	index = state->index;
	step = stepsizeTable[index];

	bufferstep = 0;

	for (; len > 0; len-- )
	{
		/* Step 1 - get the delta value */
		if ( bufferstep ) {
			delta = inputbuffer & 0xf;
		}
		else
		{
			inputbuffer = *inp++;
			delta = ( inputbuffer >> 4 ) & 0xf;
		}
		bufferstep = !bufferstep;

		/* Step 2 - Find new index value (for later) */
		index += indexTable[delta];
		if ( index < 0 ) {
			index = 0;
		}
		if ( index > 88 ) {
			index = 88;
		}

		/* Step 3 - Separate sign and magnitude */
		sign = delta & 8;
		delta = delta & 7;

		/* Step 4 - Compute difference and new predicted value */
		/*
		** Computes 'vpdiff = (delta+0.5)*step/4', but see comment
		** in adpcm_coder.
		*/
		vpdiff = step >> 3;
		if ( delta & 4 ) {
			vpdiff += step;
		}
		if ( delta & 2 ) {
			vpdiff += step >> 1;
		}
		if ( delta & 1 ) {
			vpdiff += step >> 2;
		}

		if ( sign ) {
			valpred -= vpdiff;
		}
		else{
			valpred += vpdiff;
		}

		/* Step 5 - clamp output value */
		if ( valpred > 32767 ) {
			valpred = 32767;
		}
		else if ( valpred < -32768 ) {
			valpred = -32768;
		}

		/* Step 6 - Update step value */
		step = stepsizeTable[index];

		/* Step 7 - Output value */
		*outp++ = valpred;
	}

	state->valprev = valpred;
	state->index = index;
}
#endif

void adpcm_coder( short *inp, adpcm_t *adpcm ){
	int val;                        /* Current input sample value */
	int sign;                       /* Current adpcm sign bit */
	int delta;                      /* Current adpcm output value */
	int diff;                       /* Difference between val and valprev */
	int step;                       /* Stepsize */
	int valpred;                    /* Predicted output value */
	int vpdiff;                     /* Current change to valpred */
	int index;                      /* Current step change index */
	int outputbuffer;               /* place to keep previous 4-bit value */
	int bufferstep;                 /* toggle between outputbuffer/output */
	adpcm_state_t   *state;
	char            *outp;
	int len;

	state = &adpcm->state;
	len = state->count;
	outp = adpcm->adpcm;

	valpred = state->in_valprev;
	index = state->in_index;
	step = stepsizeTable[index];

	bufferstep = 1;
	while ( len-- )
	{
		val = *inp++;

		/* Step 1 - compute difference with previous value */
		diff = val - valpred;
		sign = ( diff < 0 ) ? 8 : 0;
		if ( sign ) {
			diff = -diff;
		}

		/* Step 2 - Divide and clamp */
		/* Note:
		** This code *approximately* computes:
		**	  delta = diff*4/step;
		**	  vpdiff = (delta+0.5)*step/4;
		** but in shift step bits are dropped. The net result of this is
		** that even if you have fast mul/div hardware you cannot put it to
		** good use since the fixup would be too expensive.
		*/
		delta = 0;
		vpdiff = ( step >> 3 );

		if ( diff >= step ) {
			delta = 4;
			diff -= step;
			vpdiff += step;
		}
		step >>= 1;
		if ( diff >= step ) {
			delta |= 2;
			diff -= step;
			vpdiff += step;
		}
		step >>= 1;
		if ( diff >= step ) {
			delta |= 1;
			vpdiff += step;
		}

		/* Step 3 - Update previous value */
		if ( sign ) {
			valpred -= vpdiff;
		}
		else{
			valpred += vpdiff;
		}

		/* Step 4 - Clamp previous value to 16 bits */
		if ( valpred > 32767 ) {
			valpred = 32767;
		}
		else if ( valpred < -32768 ) {
			valpred = -32768;
		}

		/* Step 5 - Assemble value, update index and step values */
		delta |= sign;

		index += indexTable[delta];
		if ( index < 0 ) {
			index = 0;
		}
		if ( index > 88 ) {
			index = 88;
		}
		step = stepsizeTable[index];

		/* Step 6 - Output value */
		if ( bufferstep ) {
			outputbuffer = ( delta << 4 ) & 0xf0;
		}
		else{
			*outp++ = ( delta & 0x0f ) | outputbuffer;
		}

		bufferstep = !bufferstep;
	}

	/* Output last step, if needed */
	if ( !bufferstep ) {
		*outp++ = outputbuffer;
	}

	state->out_valprev = valpred;
	state->out_index = index;
}

void FindNextChunk( char *name ){
	while ( 1 )
	{
		data_p = last_chunk;

		if ( data_p >= iff_end ) {         // didn't find the chunk
			data_p = NULL;
			return;
		}

		data_p += 4;
		iff_chunk_len = *(long *)data_p;
		data_p += 4;
		if ( iff_chunk_len < 0 ) {
			data_p = NULL;
			return;
		}

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
		iff_chunk_len = *(long *)data_p;
		data_p += 4;
		printf( "0x%x : %s (%d)\n", (int)( data_p - 4 ), str, iff_chunk_len );
		data_p += ( iff_chunk_len + 1 ) & ~1;
	}
	while ( data_p < iff_end );
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
		return( info );
	}

	iff_data = wav;
	iff_end = wav + wavlength;

// find "RIFF" chunk
	FindChunk( "RIFF" );
	if ( !( data_p && !strncmp( data_p + 8, "WAVE", 4 ) ) ) {
		printf( "Missing RIFF/WAVE chunks\n" );
		return( info );
	}

// get "fmt " chunk
	iff_data = data_p + 12;

	FindChunk( "fmt " );
	if ( !data_p ) {
		printf( "Missing fmt chunk\n" );
		return( info );
	}
	data_p += 8;
	format = *(short *)data_p;
	data_p += 2;
	if ( format != 1 ) {
		printf( "Microsoft PCM format only\n" );
		return( info );
	}

	info.channels = *(short *)data_p;
	data_p += 2;
	info.rate = *(long *)data_p;
	data_p += 4;
	data_p += 6;
	info.width = *(short *)data_p / 8;
	data_p += 2;

// get cue chunk
	FindChunk( "cue " );
	if ( data_p ) {
		data_p += 32;
		info.loopstart = *(long *)data_p;
		data_p += 4;

// if the next chunk is a LIST chunk, look for a cue length marker
		FindNextChunk( "LIST" );
		if ( data_p ) {
// this is not a proper parse, but it works with cooledit...
			if ( !strncmp( data_p + 28, "mark", 4 ) ) {
				data_p += 24;
				i = *(long *)data_p;                    // samples in loop
				data_p += 4;
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
		return( info );
	}

	data_p += 4;
	samples = *(long *)data_p;
	data_p += 4;

	if ( info.samples ) {
		if ( samples < info.samples ) {
			Error( "Sound %s has a bad loop length", name );
		}
	}
	else{
		info.samples = samples;
	}

	info.dataofs = data_p - wav;
	return( info );
}

// ==============
// LoadSoundtrack
// ==============

void LoadSoundtrack(){
	char name[1024];
	FILE    *f;
	int len;

	soundtrack = NULL;
	sprintf( name, "%svideo/%s/%s.wav", gamedir, base, base );
	printf( "\nLoading sound    : %s\n", name );
	f = fopen( name, "rb" );
	if ( !f ) {
		printf( "\nNo soundtrack for %s\n", base );
		return;
	}
	len = Q_filelength( f );
	soundtrack = SafeMalloc( len, "LoadSoundtrack" );
	fread( soundtrack, 1, len, f );
	fclose( f );

	wavinfo = GetWavinfo( name, soundtrack, len );
	adpcm.state.out_valprev = 0;
	adpcm.state.out_index = 0;
}

// ==================
// WriteSound
// ==================

int WriteSound( FILE *output, int frame, int numframes ){
	int start, end;
	int count;
	int empty = 0;
	int width;
	char    *work;

	width = wavinfo.width * wavinfo.channels;
	start = ( ( frame * wavinfo.rate / 14 ) + 31 ) & 0xffffffe0;                // start sample
	end = ( ( ( frame + numframes ) * wavinfo.rate / 14 ) + 31 ) & 0xffffffe0;    // end sample
	count = end - start;

	work = soundtrack + wavinfo.dataofs + ( start * width );
	adpcm.state.count = count * wavinfo.channels;           // Number of samples
	adpcm.state.in_valprev = adpcm.state.out_valprev;
	adpcm.state.in_index = adpcm.state.out_index;
	adpcm_coder( (short *)work, &adpcm );
	WriteHeader( output, FC_SOUND_22KMADPCM, FC_ADPCM_VERSION, ( adpcm.state.count / 2 ) + sizeof( adpcm_state_t ), (char *)&adpcm );
	return( count / 2 );
}
// ==============================
// Basic run length encoder
// ==============================

char *RLEZZ( char *in, char *out ){
	int srun;
	char count;
	int idx = 0;

	while ( idx < 64 )
	{
		srun = idx;                             // Start of run

		while ( idx < 63 )
		{
			if ( in[LUT_ZZ[idx]] != in[LUT_ZZ[idx + 1]] ) {
				break;
			}
			idx++;
		}
		count = (char)( idx - srun );             // count of repeated bytes

		if ( !count ) {
			while ( idx < 63 )
			{
				if ( in[LUT_ZZ[idx]] == in[LUT_ZZ[idx + 1]] ) {
					break;
				}
				idx++;
			}
			if ( idx == 63 ) {
				idx++;
			}

			count = (char)( idx - srun );         // count of unique bytes
			*out++ = count;
			while ( count-- )
				*out++ = in[LUT_ZZ[srun++]];
		}
		else
		{
			*out++ = -( count + 1 );
			*out++ = in[LUT_ZZ[idx]];
			idx++;
		}
	}
	return( out );
}

// ==============================
// Discrete Cosine Transformation
// ==============================

void init_base( float quant ){
	int y, x;

	for ( y = 0; y < BLOCKSIZE; y++ )
		for ( x = 0; x < BLOCKSIZE; x++ )
		{
			if ( y == 0 ) {
				dctbase[y][x] = 1;
			}
			else{
				dctbase[y][x] = SQRT2 * cos( ( ( x * 2 + 1 ) * y * M_PI ) / ( BLOCKSIZE * 2 ) );
			}
		}

	for ( y = 0; y < BLOCKSIZE * BLOCKSIZE; y++ )
		Quantise[y] = LUT_Quantise[y] / quant;
}

void SplitComponents( byte *src, int width, int height ){
	int i, j;
	float   *tr = red;
	float   *tg = green;
	float   *tb = blue;

	for ( i = 0; i < BLOCKSIZE; i++, src += ( width - BLOCKSIZE ) * 4 )
		for ( j = 0; j < BLOCKSIZE; j++ )
		{
			*tr++ = ( (float)*src++ ) - 128.0F;
			*tg++ = ( (float)*src++ ) - 128.0F;
			*tb++ = ( (float)*src++ ) - 128.0F;
			src++;
		}
}

void transferH( float *src, float *dst ){
	int y, dx, dy;
	float sum;
	float   *work;

	for ( y = 0; y < BLOCKSIZE; y++, src += BLOCKSIZE )
	{
		for ( dy = 0; dy < BLOCKSIZE; dy++ )
		{
			sum = 0;
			work = src;
			for ( dx = 0; dx < BLOCKSIZE; dx++, work++ )
				sum += dctbase[dy][dx] * *work;

			*dst++ = sum / BLOCKSIZE;
		}
	}
}

void transferV( float *src, float *dst ){
	int x, dy, fy;
	float sum;
	float   *work;

	for ( x = 0; x < BLOCKSIZE; x++, src++, dst++ )
	{
		for ( fy = 0; fy < BLOCKSIZE; fy++ )
		{
			sum = 0;
			work = src;
			for ( dy = 0; dy < BLOCKSIZE; dy++, work += BLOCKSIZE )
				sum += dctbase[fy][dy] * *work;

			dst[fy * BLOCKSIZE] = sum / BLOCKSIZE;
		}
	}
}

char *Combine( byte *dst, float *p, float *q ){
	int i, j;
	byte rlesrc[BLOCKSIZE * BLOCKSIZE];
	int c;
	byte    *work;

	work = rlesrc;
	for ( j = 0; j < BLOCKSIZE; j++ )
		for ( i = 0; i < BLOCKSIZE; i++ )
		{
			c = (int)( ( *p++ / *q++ ) + 128.5F );
			c -= 128;

			if ( c < -128 ) {
				c = -128;
			}
			if ( c > 127 ) {
				c = 127;
			}

			*work++ = (char)c;
		}

	dst = RLEZZ( rlesrc, dst );
	return( dst );
}

char *CombineComponents( char *dst, int width, int height ){
	dst = Combine( dst, red, Quantise );
	dst = Combine( dst, green, Quantise );
	dst = Combine( dst, blue, Quantise );
	return( dst );
}

void DCT( cblock_t *out, cblock_t in, int width, int height ){
	int x, y;
	char    *cursrc;
	char    *curdst;

	curdst = out->data;
	for ( y = 0; y < height; y += BLOCKSIZE )
		for ( x = 0; x < width; x += BLOCKSIZE )
		{
			cursrc = in.data + ( ( y * width ) + x ) * 4;
			SplitComponents( cursrc, width, height );
			transferH( red, temp );
			transferV( temp, red );
			transferH( green, temp );
			transferV( temp, green );
			transferH( blue, temp );
			transferV( temp, blue );
			curdst = CombineComponents( curdst, width, height );
		}
	out->count = curdst - out->data;
}

// ==================
// BuildChars1
// ==================

void BuildChars1( int prev, int nodenum, unsigned bits, int bitcount ){
	hnode_t     *node;

	if ( nodenum < HUF_TOKENS ) {
		if ( bitcount > 32 ) {
			Error( "bitcount > 32" );
		}
		charbits1[prev][nodenum] = bits;
		charbitscount1[prev][nodenum] = bitcount;
		return;
	}

	node = &hnodes1[prev][nodenum];
	bits <<= 1;
	BuildChars1( prev, node->children[0], bits, bitcount + 1 );
	bits |= 1;
	BuildChars1( prev, node->children[1], bits, bitcount + 1 );
}

// ==================
// SmallestNode1
// ==================

int SmallestNode1( hnode_t *hnodes, int numhnodes ){
	int i;
	int best, bestnode;

	best = 99999999;
	bestnode = -1;
	for ( i = 0; i < numhnodes; i++ )
	{
		if ( hnodes[i].used ) {
			continue;
		}
		if ( !hnodes[i].count ) {
			continue;
		}
		if ( hnodes[i].count < best ) {
			best = hnodes[i].count;
			bestnode = i;
		}
	}

	if ( bestnode == -1 ) {
		return( -1 );
	}

	hnodes[bestnode].used = true;
	return( bestnode );
}

// ==================
// BuildTree1
// ==================

void BuildTree1( int prev ){
	hnode_t     *node, *nodebase;
	int numhnodes;

	// build the nodes
	numhnodes = HUF_TOKENS;
	nodebase = hnodes1[prev];
	while ( 1 )
	{
		node = &nodebase[numhnodes];

		// pick two lowest counts
		node->children[0] = SmallestNode1( nodebase, numhnodes );
		if ( node->children[0] == -1 ) {
			break;  // no more

		}
		node->children[1] = SmallestNode1( nodebase, numhnodes );
		if ( node->children[1] == -1 ) {
			break;
		}

		node->count = nodebase[node->children[0]].count +
					  nodebase[node->children[1]].count;
		numhnodes++;
	}
	numhnodes1[prev] = numhnodes - 1;
	BuildChars1( prev, numhnodes - 1, 0, 0 );
}

// ==================
// Huffman1_Count
// ==================

void Huffman1_Count( cblock_t in ){
	int i;
	int prev;
	int v;
	int rept;

	prev = 0;
	for ( i = 0; i < in.count; i++ )
	{
		v = in.data[i];
		order0counts[v]++;
		hnodes1[prev][v].count++;
		prev = v;

		for ( rept = 1; ( i + rept < in.count ) && ( rept < MAX_REPT ); rept++ )
			if ( in.data[i + rept] != v ) {
				break;
			}
		if ( rept > MIN_REPT ) {
			hnodes1[prev][255 + rept].count++;
			i += rept - 1;
		}
	}
}

// ==================
// Huffman1_Build
// ==================

void Huffman1_Build(){
	int i, j, v;
	int max;
	int total;

	for ( i = 0; i < 256; i++ )
	{
// normalize and save the counts
		max = 0;
		for ( j = 0; j < HUF_TOKENS; j++ )
		{
			if ( hnodes1[i][j].count > max ) {
				max = hnodes1[i][j].count;
			}
		}
		if ( max == 0 ) {
			max = 1;
		}
		total = 0;
// easy to overflow 32 bits here!
		for ( j = 0; j < HUF_TOKENS; j++ )
		{
			v = ( hnodes1[i][j].count * (double) 255 + max - 1 ) / max;
			if ( v > 255 ) {
				Error( "v > 255" );
			}
			scaled[i][j] = hnodes1[i][j].count = v;
			if ( v ) {
				total++;
			}
		}
		if ( total == 1 ) { // must have two tokens
			if ( !scaled[i][0] ) {
				scaled[i][0] = hnodes1[i][0].count = 1;
			}
			else{
				scaled[i][1] = hnodes1[i][1].count = 1;
			}
		}
		BuildTree1( i );
	}
}

// ==================
// Huffman1
// Order 1 compression with pre-built table
// ==================

cblock_t Huffman1( cblock_t in ){
	int i;
	int outbits, c;
	unsigned bits;
	byte        *out_p;
	cblock_t out;
	int prev;
	int v;
	int rept;

	out_p = out.data = SafeMalloc( ( in.count * 2 ) + 1024 + 4, "Huffman" );
	memset( out_p, 0, ( in.count * 2 ) + 1024 + 4 );

	// leave space for compressed count
	out_p += 4;
	// write count
	*(long *)out_p = in.count;
	out_p += 4;

	// write bits
	outbits = 0;
	prev = 0;
	for ( i = 0; i < in.count; i++ )
	{
		v = in.data[i];

		c = charbitscount1[prev][v];
		bits = charbits1[prev][v];
		if ( !c ) {
			Error( "!bits" );
		}
		while ( c )
		{
			c--;
			if ( bits & ( 1 << c ) ) {
				out_p[outbits >> 3] |= 1 << ( outbits & 7 );
			}
			outbits++;
		}

		prev = v;
		// check for repeat encodes
		for ( rept = 1; ( i + rept < in.count ) && ( rept < MAX_REPT ); rept++ )
			if ( in.data[i + rept] != v ) {
				break;
			}
		if ( rept > MIN_REPT ) {
			c = charbitscount1[prev][255 + rept];
			bits = charbits1[prev][255 + rept];
			if ( !c ) {
				Error( "!bits" );
			}
			while ( c )
			{
				c--;
				if ( bits & ( 1 << c ) ) {
					out_p[outbits >> 3] |= 1 << ( outbits & 7 );
				}
				outbits++;
			}
			i += rept - 1;
		}
	}
	out_p += ( outbits + 7 ) >> 3;
	out.count = out_p - out.data;

	out_p = out.data;
	*(long *)out_p = out.count;
	return( out );
}
// ===================
// LoadFrame
// ===================

void LoadFrame( cblock_t *out, char *base, int frame ){
	cblock_t in;
	int width, height;
	char name[1024];
	FILE        *f;

	in.data = NULL;
	in.count = -1;
	sprintf( name, "%svideo/%s/%s%04i.tga", gamedir, base, base, frame );

	f = fopen( name, "rb" );
	if ( !f ) {
		out->data = NULL;
		return;
	}
	fclose( f );

	LoadTGA( name, &in.data, &width, &height );
	if ( ( width != cinehead.Width ) || ( height != cinehead.Height ) ) {
		free( in.data );
		printf( "Invalid picture size\n" );
		out->data = NULL;
		return;
	}
	out->data = SafeMalloc( width * height * 3, "LoadFrame" );        // rle could possibly expand file so this not 100% safe (however DCT should force a lot of compression)
	DCT( out, in, width, height );
	free( in.data );
}

// ==================================
// Cmd_Video
//
// video <directory> <framedigits>
// ==================================

void Cmd_Video(){
	char savename[256];
	char name[256];
	FILE        *output;
	int frame;
	int width, height;
	cblock_t in, huffman;
	int size;
	float dctconst;
	int maxsize, ssize;
	int min_rle_size, warnings;
	int ave_image, ave_sound;

	GetScriptToken( false );
	strcpy( base, token );
	if ( g_release ) {
		return;
	}

	GetScriptToken( false );
	dctconst = atof( token );
	GetScriptToken( false );
	maxsize = atoi( token );

	sprintf( savename, "%svideo/%s.cin", gamedir, base );

	// clear stuff
	memset( charbits1, 0, sizeof( charbits1 ) );
	memset( charbitscount1, 0, sizeof( charbitscount1 ) );
	memset( hnodes1, 0, sizeof( hnodes1 ) );
	memset( numhnodes1, 0, sizeof( numhnodes1 ) );
	memset( order0counts, 0, sizeof( order0counts ) );

	// load the entire sound wav file if present
	LoadSoundtrack();

	cinehead.SndRate = wavinfo.rate;
	cinehead.SndWidth = wavinfo.width;
	cinehead.SndChannels = wavinfo.channels;

	sprintf( name, "%svideo/%s/%s0000.tga", gamedir, base, base );
	printf( "Loading sequence : %s\n", name );
	printf( "DCT constant     : %f\n", dctconst );

	LoadTGA( name, NULL, &width, &height );

	output = fopen( savename, "wb" );
	if ( !output ) {
		Error( "Can't open %s", savename );
	}

	if ( ( width % BLOCKSIZE ) || ( height % BLOCKSIZE ) ) {
		Error( "Width and height must be a multiple of %d", BLOCKSIZE );
	}

	cinehead.Width = width;
	cinehead.Height = height;
	init_base( dctconst );

	// build the dictionary
	printf( "Counting         : " );
	min_rle_size = 0;
	for ( frame = 0;  ; frame++ )
	{
		printf( "." );
		LoadFrame( &in, base, frame );
		if ( !in.data ) {
			break;
		}
		Huffman1_Count( in );
		if ( in.count > min_rle_size ) {
			min_rle_size = in.count;
		}
		free( in.data );
	}
	printf( "\n" );
	cinehead.NumFrames = frame;
	printf( "Num Frames       : %d\n", frame );
	cinehead.MaxRleSize = ( min_rle_size + 0x1f ) & 0xfffffe0;
	cinehead.MaxSndSize = ( ( 4 * wavinfo.rate * wavinfo.channels / 14 ) + 0x1f ) & 0xffffffe0;

	WriteHeader( output, FC_HEADER_NAME, FC_HEADER_VERSION, sizeof( CineHead_t ), &cinehead );

	// build nodes and write counts
	Huffman1_Build();
	WriteHeader( output, FC_HUFFBITS_NAME, FC_HUFFBITS_VERSION, sizeof( scaled ), scaled );
	WriteHeader( output, FC_QUANT_NAME, FC_QUANT_VERSION, sizeof( Quantise ), Quantise );

	ave_image = 0;
	ave_sound = 0;
	warnings = 0;
	// compress it with the dictionary
	if ( soundtrack ) {
		ssize = WriteSound( output, frame, 4 );
		ave_sound += ssize;
	}

	for ( frame = 0; frame < cinehead.NumFrames; frame++ )
	{
		// save some sound samples
		printf( "Packing          : ", frame );
		LoadFrame( &in, base, frame );

		// save the image
		huffman = Huffman1( in );
		printf( "%d bytes rle, %d bytes huffman", in.count, huffman.count );
		size = ( huffman.count + 3 ) & 0xfffffffc;                    // round up to longwords
		if ( size > maxsize ) {
			printf( " ** WARNING **" );
			warnings++;
		}
		printf( "\n" );
		ave_image += huffman.count;

		WriteHeader( output, FC_IMAGE_NAME, FC_IMAGE_VERSION, size, huffman.data );
		if ( soundtrack ) {
			ssize = WriteSound( output, frame + 4, 1 );
			ave_sound += ssize;
		}

		free( in.data );
		free( huffman.data );
	}
	printf( "\nTotal size: %d (headers + %d image + %d sound)\n", ftell( output ), ave_image, ave_sound );
	printf( "Data rate : %d bytes per sec (image and sound)\n", ( ave_image + ave_sound ) / cinehead.NumFrames );
	printf( "Cin created ok with %d warnings.\n", warnings );
	fclose( output );

	if ( soundtrack ) {
		free( soundtrack );
	}
}
#endif

void Cmd_Video(){
}

// end
