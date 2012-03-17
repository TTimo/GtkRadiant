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


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>
#include "animcomp.h"


void    *SafeMalloc( size_t n, char *desc );



float *matrix;
float *delta;
float *best;
float *comp;
float *tcomp;
float *bestcomp;
float *frames;
float *base;

int MatWidth;
int MatHeight;
int CFrameSize;
int nFrames;


void AnimCompressInit( int nframes,int nVerts,int CompressedFrameSize ){
	nFrames = nframes;
	MatWidth = nVerts * 3;
	MatHeight = CompressedFrameSize;
	CFrameSize = CompressedFrameSize;
	matrix = (float *)SafeMalloc( MatWidth * MatHeight * sizeof( float ), "AnimCompressInit" );
	best = (float *)SafeMalloc( MatWidth * MatHeight * sizeof( float ), "AnimCompressInit" );
	delta = (float *)SafeMalloc( MatWidth * MatHeight * sizeof( float ), "AnimCompressInit" );
	comp = (float *)SafeMalloc( CFrameSize * nFrames * sizeof( float ), "AnimCompressInit" );
	tcomp = (float *)SafeMalloc( CFrameSize * nFrames * sizeof( float ), "AnimCompressInit" );
	bestcomp = (float *)SafeMalloc( CFrameSize * nFrames * sizeof( float ), "AnimCompressInit" );
	base = (float *)SafeMalloc( MatWidth * sizeof( float ), "AnimCompressInit" );
	frames = (float *)SafeMalloc( MatWidth * nFrames * sizeof( float ), "AnimCompressInit" );
}

void AnimSetFrame( int frame,int index,float x,float y,float z ){
	frames[frame * MatWidth + index * 3] = x;
	frames[frame * MatWidth + index * 3 + 1] = y;
	frames[frame * MatWidth + index * 3 + 2] = z;
}

typedef struct
{
	int index;
	float val;
} SORTP;


#define F_RANDOM ( ( (float)rand() ) / (float)RAND_MAX )

extern void DOsvd( float *a,float *res,float *comp,float *values,int nframes,int framesize,int compressedsize );

void AnimCompressDoit(){
	float compression;
	float *rescale;
	float *ans;
	float maxdev;
	float avedev;
	float tmp;
	int j,k,l,numave;

	for ( k = 0; k < MatWidth; k++ )
		base[k] = 0.0f;
	for ( j = 0; j < nFrames; j++ )
		for ( k = 0; k < MatWidth; k++ )
			base[k] += frames[j * MatWidth + k];
	tmp = 1.0f / (float)nFrames;
	for ( k = 0; k < MatWidth; k++ )
		base[k] *= tmp;
	for ( j = 0; j < nFrames; j++ )
		for ( k = 0; k < MatWidth; k++ )
			frames[j * MatWidth + k] -= base[k];

	ans = (float *)SafeMalloc( sizeof( float ) * MatWidth, "AnimCompressDoit" );
	rescale = (float *)SafeMalloc( sizeof( float ) * CFrameSize, "AnimCompressDoit" );
	DOsvd( frames,best,bestcomp,rescale,nFrames,MatWidth,MatHeight );
	avedev = 0.0;
	for ( l = 0; l < CFrameSize; l++ )
		avedev += rescale[l];
	for ( l = 0; l < CFrameSize; l++ )
		printf( "%3.1f ",100.0f * rescale[l] / avedev );
	printf( "\n" );
	for ( j = 0; j < nFrames; j++ )
	{
		for ( l = 0; l < CFrameSize; l++ )
		{
			bestcomp[j * CFrameSize + l] = 0.0;
			for ( k = 0; k < MatWidth; k++ )
				bestcomp[j * CFrameSize + l] += best[l * MatWidth + k] * frames[j * MatWidth + k];
		}
	}
	numave = 0;
	avedev = 0.0;
	maxdev = 0.0;
	for ( j = 0; j < nFrames; j++ )
	{
		for ( k = 0; k < MatWidth; k++ )
		{
			ans[k] = 0.0;
			for ( l = 0; l < CFrameSize; l++ )
				ans[k] += best[l * MatWidth + k] * bestcomp[j * CFrameSize + l];
			ans[k] -= frames[j * MatWidth + k];
			tmp = (float)fabs( ans[k] );
			if ( tmp > maxdev ) {
				maxdev = tmp;
			}
			avedev += tmp;
			numave++;
		}
	}
	avedev /= (float)numave;
	printf( "%f Max Deviation (inches)  %f Ave Dev. (inches)\n",maxdev,avedev );
	printf( "%d bytes original size\n",MatWidth * nFrames );
	printf( "%d bytes of overhead\n",MatWidth * MatHeight );
	printf( "%d bytes/frame * %d frames = %d bytes\n",CFrameSize,nFrames,CFrameSize * nFrames );
	compression = (float)( MatWidth * MatHeight + CFrameSize * nFrames + MatWidth );
	compression /= (float)( MatWidth * nFrames );
	printf( "Overall compression = %f %%\n",100.0f - 100.0f * compression );
	compression = (float)( CFrameSize );
	compression /= (float)( MatWidth );
	printf( "frame size compression = %f %%\n",100.0f - 100.0f * compression );
	free( rescale );
	free( ans );
}

void AnimCompressToBytes( float *trans,float *scale,char *mat,char *ccomp,unsigned char *cbase,float *cscale,float *coffset,float *bmin,float *bmax ){
	int k,l,nv,j;
	float maxdev;
	float avedev;
	float tmp;
	int numave;
	float t,mx;
	float *ans;


	nv = MatWidth / 3;

	trans[0] = 1E30f;
	scale[0] = -1E30f;
	trans[1] = 1E30f;
	scale[1] = -1E30f;
	trans[2] = 1E30f;
	scale[2] = -1E30f;
	for ( k = 0; k < MatWidth; k += 3 )
	{
		if ( base[k] > scale[0] ) {
			scale[0] = base[k];
		}
		if ( base[k] < trans[0] ) {
			trans[0] = base[k];
		}

		if ( base[k + 1] > scale[1] ) {
			scale[1] = base[k + 1];
		}
		if ( base[k + 1] < trans[1] ) {
			trans[1] = base[k + 1];
		}

		if ( base[k + 2] > scale[2] ) {
			scale[2] = base[k + 2];
		}
		if ( base[k + 2] < trans[2] ) {
			trans[2] = base[k + 2];
		}
	}

	scale[0] -= trans[0];
	scale[1] -= trans[1];
	scale[2] -= trans[2];
	scale[0] /= 255.0f;
	scale[1] /= 255.0f;
	scale[2] /= 255.0f;
	for ( k = 0; k < MatWidth; k += 3 )
	{
		t = ( base[k] - trans[0] ) / scale[0];
		if ( t < 0.0f ) {
			t = 0.0f;
		}
		if ( t > 255.0f ) {
			t = 255.0f;
		}
		cbase[k] = (unsigned char)t;

		t = ( base[k + 1] - trans[1] ) / scale[1];
		if ( t < 0.0f ) {
			t = 0.0f;
		}
		if ( t > 255.0f ) {
			t = 255.0f;
		}
		cbase[k + 1] = (unsigned char)t;

		t = ( base[k + 2] - trans[2] ) / scale[2];
		if ( t < 0.0f ) {
			t = 0.0f;
		}
		if ( t > 255.0f ) {
			t = 255.0f;
		}
		cbase[k + 2] = (unsigned char)t;
	}
	for ( l = 0; l < MatHeight; l++ )
	{
		mx = 0.0;
		for ( k = 0; k < MatWidth; k++ )
		{
			if ( fabs( best[l * MatWidth + k] ) > mx ) {
				mx = (float)fabs( best[l * MatWidth + k] );
			}
		}
		if ( mx > 1E-8 ) {
			mx /= 127.0f;
			coffset[l] = 1E30f;
			cscale[l] = -1E30f;
			for ( j = 0; j < nFrames; j++ )
			{
				bestcomp[j * MatHeight + l] *= mx;
				if ( bestcomp[j * MatHeight + l] > cscale[l] ) {
					cscale[l] = bestcomp[j * MatHeight + l];
				}
				if ( bestcomp[j * MatHeight + l] < coffset[l] ) {
					coffset[l] = bestcomp[j * MatHeight + l];
				}
			}
			cscale[l] -= coffset[l];
			if ( cscale[l] > 1E-10 ) {
				for ( j = 0; j < nFrames; j++ )
				{
					tmp = 254.0f * ( bestcomp[j * MatHeight + l] - coffset[l] ) / cscale[l] - 127.0f;
					if ( tmp > 127.0f ) {
						tmp = 127.0f;
					}
					if ( tmp < -127.0f ) {
						tmp = -127.0f;
					}
					ccomp[j * MatHeight + l] = (char)floor( tmp + 0.5 );
				}
				coffset[l] += cscale[l] * 127.0f / 254.0f;
				cscale[l] /= 254.0f;
			}
			else
			{
				cscale[l] = 1.0f;
				coffset[l] = 0.0f;
				for ( j = 0; j < nFrames; j++ )
					ccomp[j * MatHeight + l] = 0;
			}
			mx = 1.0f / mx;
			for ( k = 0; k < MatWidth; k++ )
			{
				tmp = best[l * MatWidth + k] * mx;
				if ( tmp > 127.0f ) {
					tmp = 127.0f;
				}
				if ( tmp < -127.0f ) {
					tmp = -127.0f;
				}
				mat[k * MatHeight + l] = (char)floor( tmp + 0.5 );
			}
		}
		else
		{
			cscale[l] = 1.0f;
			coffset[l] = 0.0f;
			for ( j = 0; j < nFrames; j++ )
				ccomp[j * MatHeight + l] = 0;
			for ( k = 0; k < MatWidth; k++ )
				mat[k * MatHeight + l] = 0;
		}
	}
	bmin[0] = 1E30f;
	bmin[1] = 1E30f;
	bmin[2] = 1E30f;
	bmax[0] = -1E30f;
	bmax[1] = -1E30f;
	bmax[2] = -1E30f;
	numave = 0;
	avedev = 0.0;
	maxdev = 0.0;
	ans = (float *)SafeMalloc( sizeof( float ) * MatWidth, "AnimCompressToBytes" );
	for ( j = 0; j < nFrames; j++ )
	{
		for ( k = 0; k < MatWidth; k++ )
		{
			ans[k] = 0.0;
			for ( l = 0; l < CFrameSize; l++ )
				ans[k] += (float)( mat[l + k * MatHeight] ) * ( (float)( ccomp[j * CFrameSize + l] ) * cscale[l] + coffset[l] );
			ans[k] += (float)( cbase[k] ) * scale[k % 3] + trans[k % 3];
			tmp = (float)fabs( ans[k] - frames[j * MatWidth + k] - base[k] );
			if ( tmp > maxdev ) {
				maxdev = tmp;
			}
			avedev += tmp;
			numave++;

			if ( bmin[k % 3] > ans[k] ) {
				bmin[k % 3] = ans[k];
			}
			if ( bmax[k % 3] < ans[k] ) {
				bmax[k % 3] = ans[k];
			}
		}
	}
	avedev /= (float)numave;
	printf( "%f Max Deviation (inches)  %f Ave Dev. (inches)\n",maxdev,avedev );
	free( ans );
}

void AnimCompressGetMatrix( float *mat ){
	int k,l;
	for ( k = 0; k < MatWidth; k++ )
		for ( l = 0; l < MatHeight; l++ )
			mat[k * MatHeight + l] = best[l * MatWidth + k];
}

void AnimCompressGetFrames( float *mat ){
	memcpy( mat,bestcomp,CFrameSize * nFrames * sizeof( float ) );
}

void AnimCompressGetBase( int i,float *x,float *y,float *z ){
	*x = base[i * 3];
	*y = base[i * 3 + 1];
	*z = base[i * 3 + 2];
}

void AnimCompressEnd(){
	free( matrix );
	free( best );
	free( delta );
	free( comp );
	free( tcomp );
	free( bestcomp );
	free( base );
	free( frames );
}
