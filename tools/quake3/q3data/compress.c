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

#if 0
/*
   ==================
   MTF
   ==================
 */
cblock_t MTF( cblock_t in ){
	int i, j, b, code;
	byte        *out_p;
	int index[256];
	cblock_t out;

	out_p = out.data = malloc( in.count + 4 );

	// write count
	*out_p++ = in.count & 255;
	*out_p++ = ( in.count >> 8 ) & 255;
	*out_p++ = ( in.count >> 16 ) & 255;
	*out_p++ = ( in.count >> 24 ) & 255;

	for ( i = 0 ; i < 256 ; i++ )
		index[i] = i;

	for ( i = 0 ; i < in.count ; i++ )
	{
		b = in.data[i];
		code = index[b];
		*out_p++ = code;

		// shuffle b indexes to 0
		for ( j = 0 ; j < 256 ; j++ )
			if ( index[j] < code ) {
				index[j]++;
			}
		index[b] = 0;
	}

	out.count = out_p - out.data;

	return out;
}


//==========================================================================

int bwt_size;
byte    *bwt_data;

int bwtCompare( const void *elem1, const void *elem2 ){
	int i;
	int i1, i2;
	int b1, b2;

	i1 = *(int *)elem1;
	i2 = *(int *)elem2;

	for ( i = 0 ; i < bwt_size ; i++ )
	{
		b1 = bwt_data[i1];
		b2 = bwt_data[i2];
		if ( b1 < b2 ) {
			return -1;
		}
		if ( b1 > b2 ) {
			return 1;
		}
		if ( ++i1 == bwt_size ) {
			i1 = 0;
		}
		if ( ++i2 == bwt_size ) {
			i2 = 0;
		}
	}

	return 0;
}

/*
   ==================
   BWT
   ==================
 */
cblock_t BWT( cblock_t in ){
	int     *sorted;
	int i;
	byte    *out_p;
	cblock_t out;

	bwt_size = in.count;
	bwt_data = in.data;

	sorted = malloc( in.count * sizeof( *sorted ) );
	for ( i = 0 ; i < in.count ; i++ )
		sorted[i] = i;
	qsort( sorted, in.count, sizeof( *sorted ), bwtCompare );

	out_p = out.data = malloc( in.count + 8 );

	// write count
	*out_p++ = in.count & 255;
	*out_p++ = ( in.count >> 8 ) & 255;
	*out_p++ = ( in.count >> 16 ) & 255;
	*out_p++ = ( in.count >> 24 ) & 255;

	// write head index
	for ( i = 0 ; i < in.count ; i++ )
		if ( sorted[i] == 0 ) {
			break;
		}
	*out_p++ = i & 255;
	*out_p++ = ( i >> 8 ) & 255;
	*out_p++ = ( i >> 16 ) & 255;
	*out_p++ = ( i >> 24 ) & 255;

	// write the L column
	for ( i = 0 ; i < in.count ; i++ )
		*out_p++ = in.data[( sorted[i] + in.count - 1 ) % in.count];

	free( sorted );

	out.count = out_p - out.data;

	return out;
}

//==========================================================================

typedef struct hnode_s
{
	int count;
	qboolean used;
	int children[2];
} hnode_t;

int numhnodes;
hnode_t hnodes[512];
unsigned charbits[256];
int charbitscount[256];

int SmallestNode( void ){
	int i;
	int best, bestnode;

	best = 99999999;
	bestnode = -1;
	for ( i = 0 ; i < numhnodes ; i++ )
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
		return -1;
	}

	hnodes[bestnode].used = true;
	return bestnode;
}

void BuildChars( int nodenum, unsigned bits, int bitcount ){
	hnode_t *node;

	if ( nodenum < 256 ) {
		if ( bitcount > 32 ) {
			Error( "bitcount > 32" );
		}
		charbits[nodenum] = bits;
		charbitscount[nodenum] = bitcount;
		return;
	}

	node = &hnodes[nodenum];
	bits <<= 1;
	BuildChars( node->children[0], bits, bitcount + 1 );
	bits |= 1;
	BuildChars( node->children[1], bits, bitcount + 1 );
}

/*
   ==================
   Huffman
   ==================
 */
cblock_t Huffman( cblock_t in ){
	int i;
	hnode_t     *node;
	int outbits, c;
	unsigned bits;
	byte        *out_p;
	cblock_t out;
	int max, maxchar;

	// count
	memset( hnodes, 0, sizeof( hnodes ) );
	for ( i = 0 ; i < in.count ; i++ )
		hnodes[in.data[i]].count++;

	// normalize counts
	max = 0;
	maxchar = 0;
	for ( i = 0 ; i < 256 ; i++ )
	{
		if ( hnodes[i].count > max ) {
			max = hnodes[i].count;
			maxchar = i;
		}
	}
	if ( max == 0 ) {
		Error( "Huffman: max == 0" );
	}

	for ( i = 0 ; i < 256 ; i++ )
	{
		hnodes[i].count = ( hnodes[i].count * 255 + max - 1 ) / max;
	}

	// build the nodes
	numhnodes = 256;
	while ( numhnodes != 511 )
	{
		node = &hnodes[numhnodes];

		// pick two lowest counts
		node->children[0] = SmallestNode();
		if ( node->children[0] == -1 ) {
			break;  // no more

		}
		node->children[1] = SmallestNode();
		if ( node->children[1] == -1 ) {
			if ( node->children[0] != numhnodes - 1 ) {
				Error( "Bad smallestnode" );
			}
			break;
		}
		node->count = hnodes[node->children[0]].count +
					  hnodes[node->children[1]].count;
		numhnodes++;
	}

	BuildChars( numhnodes - 1, 0, 0 );

	out_p = out.data = malloc( in.count * 2 + 1024 );
	memset( out_p, 0, in.count * 2 + 1024 );

	// write count
	*out_p++ = in.count & 255;
	*out_p++ = ( in.count >> 8 ) & 255;
	*out_p++ = ( in.count >> 16 ) & 255;
	*out_p++ = ( in.count >> 24 ) & 255;

	// save out the 256 normalized counts so the tree can be recreated
	for ( i = 0 ; i < 256 ; i++ )
		*out_p++ = hnodes[i].count;

	// write bits
	outbits = 0;
	for ( i = 0 ; i < in.count ; i++ )
	{
		c = charbitscount[in.data[i]];
		bits = charbits[in.data[i]];
		while ( c )
		{
			c--;
			if ( bits & ( 1 << c ) ) {
				out_p[outbits >> 3] |= 1 << ( outbits & 7 );
			}
			outbits++;
		}
	}

	out_p += ( outbits + 7 ) >> 3;

	out.count = out_p - out.data;

	return out;
}

//==========================================================================

/*
   ==================
   RLE
   ==================
 */
#define RLE_CODE    0xe8
#define RLE_TRIPPLE 0xe9

int rle_counts[256];
int rle_bytes[256];

cblock_t RLE( cblock_t in ){
	int i;
	byte    *out_p;
	int val;
	int repeat;
	cblock_t out;

	out_p = out.data = malloc( in.count * 2 );

	// write count
	*out_p++ = in.count & 255;
	*out_p++ = ( in.count >> 8 ) & 255;
	*out_p++ = ( in.count >> 16 ) & 255;
	*out_p++ = ( in.count >> 24 ) & 255;

	for ( i = 0 ; i < in.count ; )
	{
		val = in.data[i];
		rle_bytes[val]++;
		repeat = 1;
		i++;
		while ( i < in.count && repeat < 255 && in.data[i] == val )
		{
			repeat++;
			i++;
		}
		if ( repeat < 256 ) {
			rle_counts[repeat]++;
		}
		if ( repeat > 3 || val == RLE_CODE ) {
			*out_p++ = RLE_CODE;
			*out_p++ = val;
			*out_p++ = repeat;
		}
		else
		{
			while ( repeat-- )
				*out_p++ = val;
		}
	}

	out.count = out_p - out.data;
	return out;
}

//==========================================================================

unsigned lzss_head[256];
unsigned lzss_next[0x20000];

/*
   ==================
   LZSS
   ==================
 */
#define BACK_WINDOW     0x10000
#define BACK_BITS       16
#define FRONT_WINDOW    16
#define FRONT_BITS      4
cblock_t LZSS( cblock_t in ){
	int i;
	byte    *out_p;
	cblock_t out;
	int val;
	int j, start, max;
	int bestlength, beststart;
	int outbits;

	if ( in.count >= sizeof( lzss_next ) / 4 ) {
		Error( "LZSS: too big" );
	}

	memset( lzss_head, -1, sizeof( lzss_head ) );

	out_p = out.data = malloc( in.count * 2 );
	memset( out.data, 0, in.count * 2 );

	// write count
	*out_p++ = in.count & 255;
	*out_p++ = ( in.count >> 8 ) & 255;
	*out_p++ = ( in.count >> 16 ) & 255;
	*out_p++ = ( in.count >> 24 ) & 255;

	outbits = 0;
	for ( i = 0 ; i < in.count ; )
	{
		val = in.data[i];
#if 1
// chained search
		bestlength = 0;
		beststart = 0;

		max = FRONT_WINDOW;
		if ( i + max > in.count ) {
			max = in.count - i;
		}

		start = lzss_head[val];
		while ( start != -1 && start >= i - BACK_WINDOW )
		{
			// count match length
			for ( j = 0 ; j < max ; j++ )
				if ( in.data[start + j] != in.data[i + j] ) {
					break;
				}
			if ( j > bestlength ) {
				bestlength = j;
				beststart = start;
			}
			start = lzss_next[start];
		}

#else
// slow simple search
		// search for a match
		max = FRONT_WINDOW;
		if ( i + max > in.count ) {
			max = in.count - i;
		}

		start = i - BACK_WINDOW;
		if ( start < 0 ) {
			start = 0;
		}
		bestlength = 0;
		beststart = 0;
		for ( ; start < i ; start++ )
		{
			if ( in.data[start] != val ) {
				continue;
			}
			// count match length
			for ( j = 0 ; j < max ; j++ )
				if ( in.data[start + j] != in.data[i + j] ) {
					break;
				}
			if ( j > bestlength ) {
				bestlength = j;
				beststart = start;
			}
		}
#endif
		beststart = BACK_WINDOW - ( i - beststart );

		if ( bestlength < 3 ) { // output a single char
			bestlength = 1;

			out_p[outbits >> 3] |= 1 << ( outbits & 7 );    // set bit to mark char
			outbits++;
			for ( j = 0 ; j < 8 ; j++, outbits++ )
				if ( val & ( 1 << j ) ) {
					out_p[outbits >> 3] |= 1 << ( outbits & 7 );
				}
		}
		else
		{   // output a phrase
			outbits++;  // leave a 0 bit to mark phrase
			for ( j = 0 ; j < BACK_BITS ; j++, outbits++ )
				if ( beststart & ( 1 << j ) ) {
					out_p[outbits >> 3] |= 1 << ( outbits & 7 );
				}
			for ( j = 0 ; j < FRONT_BITS ; j++, outbits++ )
				if ( bestlength & ( 1 << j ) ) {
					out_p[outbits >> 3] |= 1 << ( outbits & 7 );
				}
		}

		while ( bestlength-- )
		{
			val = in.data[i];
			lzss_next[i] = lzss_head[val];
			lzss_head[val] = i;
			i++;
		}
	}

	out_p += ( outbits + 7 ) >> 3;
	out.count = out_p - out.data;
	return out;
}

//==========================================================================

#define MIN_REPT    15
#define MAX_REPT    0
#define HUF_TOKENS  ( 256 + MAX_REPT )

unsigned charbits1[256][HUF_TOKENS];
int charbitscount1[256][HUF_TOKENS];

hnode_t hnodes1[256][HUF_TOKENS * 2];
int numhnodes1[256];

int order0counts[256];

/*
   ==================
   SmallestNode1
   ==================
 */
int SmallestNode1( hnode_t *hnodes, int numhnodes ){
	int i;
	int best, bestnode;

	best = 99999999;
	bestnode = -1;
	for ( i = 0 ; i < numhnodes ; i++ )
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
		return -1;
	}

	hnodes[bestnode].used = true;
	return bestnode;
}


/*
   ==================
   BuildChars1
   ==================
 */
void BuildChars1( int prev, int nodenum, unsigned bits, int bitcount ){
	hnode_t *node;

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


/*
   ==================
   BuildTree1
   ==================
 */
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


/*
   ==================
   Huffman1_Count
   ==================
 */
void Huffman1_Count( cblock_t in ){
	int i;
	int prev;
	int v;
	int rept;

	prev = 0;
	for ( i = 0 ; i < in.count ; i++ )
	{
		v = in.data[i];
		order0counts[v]++;
		hnodes1[prev][v].count++;
		prev = v;
#if 1
		for ( rept = 1 ; i + rept < in.count && rept < MAX_REPT ; rept++ )
			if ( in.data[i + rept] != v ) {
				break;
			}
		if ( rept > MIN_REPT ) {
			hnodes1[prev][255 + rept].count++;
			i += rept - 1;
		}
#endif
	}
}


/*
   ==================
   Huffman1_Build
   ==================
 */
byte scaled[256][HUF_TOKENS];
void Huffman1_Build( FILE *f ){
	int i, j, v;
	int max;
	int total;

	for ( i = 0 ; i < 256 ; i++ )
	{
		// normalize and save the counts
		max = 0;
		for ( j = 0 ; j < HUF_TOKENS ; j++ )
		{
			if ( hnodes1[i][j].count > max ) {
				max = hnodes1[i][j].count;
			}
		}
		if ( max == 0 ) {
			max = 1;
		}
		total = 0;
		for ( j = 0 ; j < HUF_TOKENS ; j++ )
		{   // easy to overflow 32 bits here!
			v = ( hnodes1[i][j].count * (double)255 + max - 1 ) / max;
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

#if 0
	// count up the total bits
	total = 0;
	for ( i = 0 ; i < 256 ; i++ )
		for ( j = 0 ; j < 256 ; j++ )
			total += charbitscount1[i][j] * hnodes1[i][j].count;

	total = ( total + 7 ) / 8;
	printf( "%i bytes huffman1 compressed\n", total );
#endif

	fwrite( scaled, 1, sizeof( scaled ), f );
}

/*
   ==================
   Huffman1

   Order 1 compression with pre-built table
   ==================
 */
cblock_t Huffman1( cblock_t in ){
	int i;
	int outbits, c;
	unsigned bits;
	byte        *out_p;
	cblock_t out;
	int prev;
	int v;
	int rept;

	out_p = out.data = malloc( in.count * 2 + 1024 );
	memset( out_p, 0, in.count * 2 + 1024 );

	// write count
	*out_p++ = in.count & 255;
	*out_p++ = ( in.count >> 8 ) & 255;
	*out_p++ = ( in.count >> 16 ) & 255;
	*out_p++ = ( in.count >> 24 ) & 255;

	// write bits
	outbits = 0;
	prev = 0;
	for ( i = 0 ; i < in.count ; i++ )
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
#if 1
		// check for repeat encodes
		for ( rept = 1 ; i + rept < in.count && rept < MAX_REPT ; rept++ )
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
#endif
	}

	out_p += ( outbits + 7 ) >> 3;

	out.count = out_p - out.data;

	return out;
}

#endif
