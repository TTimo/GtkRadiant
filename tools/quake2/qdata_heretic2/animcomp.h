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

#if !defined( ANIMCOMP_INC )
#define ANIMCOMP_INC

#ifdef  __cplusplus
extern "C"
{
#endif
void AnimCompressInit( int nFrames,int nVerts,int CompressedFrameSize );
void AnimSetFrame( int frame,int index,float x,float y,float z );
void AnimCompressDoit();
void AnimCompressToBytes( float *trans,float *scale,char *mat,char *ccomp,unsigned char *cbase,float *cscale,float *coffset,float *bmin,float *bmax );
void AnimCompressGetMatrix( float *mat );
void AnimCompressGetFrames( float *mat );
void AnimCompressGetBase( int i,float *x,float *y,float *z );
void AnimCompressEnd();
void DOsvdPlane( float *pnts,int npnts,float *n,float *base );
#ifdef  __cplusplus
}
#endif

#endif
