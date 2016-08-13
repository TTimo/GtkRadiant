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

#include "stdafx.h"

// compute a determinant using Sarrus rule
//++timo "inline" this with a macro
// NOTE : the three vec3_t are understood as columns of the matrix
vec_t SarrusDet( vec3_t a, vec3_t b, vec3_t c ){
	return a[0] * b[1] * c[2] + b[0] * c[1] * a[2] + c[0] * a[1] * b[2]
		   - c[0] * b[1] * a[2] - a[1] * b[0] * c[2] - a[0] * b[2] * c[1];
}

// in many case we know three points A,B,C in two axis base B1 and B2
// and we want the matrix M so that A(B1) = T * A(B2)
// NOTE: 2D homogeneous space stuff
// NOTE: we don't do any check to see if there's a solution or we have a particular case .. need to make sure before calling
// NOTE: the third coord of the A,B,C point is ignored
// NOTE: see the commented out section to fill M and D
//++timo TODO: update the other members to use this when possible
void MatrixForPoints( vec3_t M[3], vec3_t D[2], brushprimit_texdef_t *T ){
//	vec3_t M[3]; // columns of the matrix .. easier that way (the indexing is not standard! it's column-line .. later computations are easier that way)
	vec_t det;
//	vec3_t D[2];
	M[2][0] = 1.0f; M[2][1] = 1.0f; M[2][2] = 1.0f;
#if 0
	// fill the data vectors
	M[0][0] = A2[0]; M[0][1] = B2[0]; M[0][2] = C2[0];
	M[1][0] = A2[1]; M[1][1] = B2[1]; M[1][2] = C2[1];
	M[2][0] = 1.0f; M[2][1] = 1.0f; M[2][2] = 1.0f;
	D[0][0] = A1[0];
	D[0][1] = B1[0];
	D[0][2] = C1[0];
	D[1][0] = A1[1];
	D[1][1] = B1[1];
	D[1][2] = C1[1];
#endif
	// solve
	det = SarrusDet( M[0], M[1], M[2] );
	T->coords[0][0] = SarrusDet( D[0], M[1], M[2] ) / det;
	T->coords[0][1] = SarrusDet( M[0], D[0], M[2] ) / det;
	T->coords[0][2] = SarrusDet( M[0], M[1], D[0] ) / det;
	T->coords[1][0] = SarrusDet( D[1], M[1], M[2] ) / det;
	T->coords[1][1] = SarrusDet( M[0], D[1], M[2] ) / det;
	T->coords[1][2] = SarrusDet( M[0], M[1], D[1] ) / det;
}

//++timo replace everywhere texX by texS etc. ( ----> and in q3map !)
// NOTE : ComputeAxisBase here and in q3map code must always BE THE SAME !
// WARNING : special case behaviour of atan2(y,x) <-> atan(y/x) might not be the same everywhere when x == 0
// rotation by (0,RotY,RotZ) assigns X to normal
void ComputeAxisBase( vec3_t normal,vec3_t texS,vec3_t texT ){
	vec_t RotY,RotZ;
	// do some cleaning
	if ( fabs( normal[0] ) < 1e-6 ) {
		normal[0] = 0.0f;
	}
	if ( fabs( normal[1] ) < 1e-6 ) {
		normal[1] = 0.0f;
	}
	if ( fabs( normal[2] ) < 1e-6 ) {
		normal[2] = 0.0f;
	}
	RotY = -atan2( normal[2],sqrt( normal[1] * normal[1] + normal[0] * normal[0] ) );
	RotZ = atan2( normal[1],normal[0] );
	// rotate (0,1,0) and (0,0,1) to compute texS and texT
	texS[0] = -sin( RotZ );
	texS[1] = cos( RotZ );
	texS[2] = 0;
	// the texT vector is along -Z ( T texture coorinates axis )
	texT[0] = -sin( RotY ) * cos( RotZ );
	texT[1] = -sin( RotY ) * sin( RotZ );
	texT[2] = -cos( RotY );
}

void FaceToBrushPrimitFace( face_t *f ){
	vec3_t texX,texY;
	vec3_t proj;
	// ST of (0,0) (1,0) (0,1)
	vec_t ST[3][5]; // [ point index ] [ xyz ST ]
	//++timo not used as long as brushprimit_texdef and texdef are static
/*	f->brushprimit_texdef.contents=f->texdef.contents;
    f->brushprimit_texdef.flags=f->texdef.flags;
    f->brushprimit_texdef.value=f->texdef.value;
    strcpy(f->brushprimit_texdef.name,f->texdef.name); */
#ifdef DBG_BP
	if ( f->plane.normal[0] == 0.0f && f->plane.normal[1] == 0.0f && f->plane.normal[2] == 0.0f ) {
		Sys_FPrintf( SYS_WRN, "Warning : f->plane.normal is (0,0,0) in FaceToBrushPrimitFace\n" );
	}
	// check d_texture
	if ( !f->d_texture ) {
		Sys_FPrintf( SYS_WRN, "Warning : f.d_texture is NULL in FaceToBrushPrimitFace\n" );
		return;
	}
#endif
	// compute axis base
	ComputeAxisBase( f->plane.normal,texX,texY );
	// compute projection vector
	VectorCopy( f->plane.normal,proj );
	VectorScale( proj,f->plane.dist,proj );
	// (0,0) in plane axis base is (0,0,0) in world coordinates + projection on the affine plane
	// (1,0) in plane axis base is texX in world coordinates + projection on the affine plane
	// (0,1) in plane axis base is texY in world coordinates + projection on the affine plane
	// use old texture code to compute the ST coords of these points
	VectorCopy( proj,ST[0] );
	EmitTextureCoordinates( ST[0], f->d_texture, f );
	VectorCopy( texX,ST[1] );
	VectorAdd( ST[1],proj,ST[1] );
	EmitTextureCoordinates( ST[1], f->d_texture, f );
	VectorCopy( texY,ST[2] );
	VectorAdd( ST[2],proj,ST[2] );
	EmitTextureCoordinates( ST[2], f->d_texture, f );
	// compute texture matrix
	f->brushprimit_texdef.coords[0][2] = ST[0][3];
	f->brushprimit_texdef.coords[1][2] = ST[0][4];
	f->brushprimit_texdef.coords[0][0] = ST[1][3] - f->brushprimit_texdef.coords[0][2];
	f->brushprimit_texdef.coords[1][0] = ST[1][4] - f->brushprimit_texdef.coords[1][2];
	f->brushprimit_texdef.coords[0][1] = ST[2][3] - f->brushprimit_texdef.coords[0][2];
	f->brushprimit_texdef.coords[1][1] = ST[2][4] - f->brushprimit_texdef.coords[1][2];
}

// compute texture coordinates for the winding points
void EmitBrushPrimitTextureCoordinates( face_t * f, winding_t * w ){
	vec3_t texX,texY;
	vec_t x,y;
	// compute axis base
	ComputeAxisBase( f->plane.normal,texX,texY );
	// in case the texcoords matrix is empty, build a default one
	// same behaviour as if scale[0]==0 && scale[1]==0 in old code
	if ( f->brushprimit_texdef.coords[0][0] == 0 && f->brushprimit_texdef.coords[1][0] == 0 && f->brushprimit_texdef.coords[0][1] == 0 && f->brushprimit_texdef.coords[1][1] == 0 ) {
		f->brushprimit_texdef.coords[0][0] = 1.0f;
		f->brushprimit_texdef.coords[1][1] = 1.0f;
		ConvertTexMatWithQTexture( &f->brushprimit_texdef, NULL, &f->brushprimit_texdef, f->d_texture );
	}
	int i;
	for ( i = 0 ; i < w->numpoints ; i++ )
	{
		x = DotProduct( w->points[i],texX );
		y = DotProduct( w->points[i],texY );
#ifdef DBG_BP
		if ( g_qeglobals.bNeedConvert ) {
			// check we compute the same ST as the traditional texture computation used before
			vec_t S = f->brushprimit_texdef.coords[0][0] * x + f->brushprimit_texdef.coords[0][1] * y + f->brushprimit_texdef.coords[0][2];
			vec_t T = f->brushprimit_texdef.coords[1][0] * x + f->brushprimit_texdef.coords[1][1] * y + f->brushprimit_texdef.coords[1][2];
			if ( fabs( S - w->points[i][3] ) > 1e-2 || fabs( T - w->points[i][4] ) > 1e-2 ) {
				if ( fabs( S - w->points[i][3] ) > 1e-4 || fabs( T - w->points[i][4] ) > 1e-4 ) {
					Sys_FPrintf( SYS_WRN, "Warning : precision loss in brush -> brush primitive texture computation\n" );
				}
				else{
					Sys_FPrintf( SYS_WRN, "Warning : brush -> brush primitive texture computation bug detected\n" );
				}
			}
		}
#endif
		w->points[i][3] = f->brushprimit_texdef.coords[0][0] * x + f->brushprimit_texdef.coords[0][1] * y + f->brushprimit_texdef.coords[0][2];
		w->points[i][4] = f->brushprimit_texdef.coords[1][0] * x + f->brushprimit_texdef.coords[1][1] * y + f->brushprimit_texdef.coords[1][2];
	}
}

// compute a fake shift scale rot representation from the texture matrix
// these shift scale rot values are to be understood in the local axis base
void TexMatToFakeTexCoords( vec_t texMat[2][3], float shift[2], float *rot, float scale[2] ){
#ifdef DBG_BP
	// check this matrix is orthogonal
	if ( fabs( texMat[0][0] * 1.0L * texMat[0][1] + texMat[1][0] * 1.0L * texMat[1][1] ) > ZERO_EPSILON ) {
		Sys_FPrintf( SYS_WRN, "Warning : non orthogonal texture matrix in TexMatToFakeTexCoords\n" );
	}
#endif
	scale[0] = sqrt( texMat[0][0] * 1.0L * texMat[0][0] + texMat[1][0] * 1.0L * texMat[1][0] );
	scale[1] = sqrt( texMat[0][1] * 1.0L * texMat[0][1] + texMat[1][1] * 1.0L * texMat[1][1] );
#ifdef DBG_BP
	if ( scale[0] < ZERO_EPSILON || scale[1] < ZERO_EPSILON ) {
		Sys_FPrintf( SYS_WRN, "Warning : unexpected scale==0 in TexMatToFakeTexCoords\n" );
	}
#endif
	// compute rotate value
	if ( fabs( texMat[0][0] ) < ZERO_EPSILON ) {
#ifdef DBG_BP
		// check brushprimit_texdef[1][0] is not zero
		if ( fabs( texMat[1][0] ) < ZERO_EPSILON ) {
			Sys_FPrintf( SYS_WRN, "Warning : unexpected texdef[1][0]==0 in TexMatToFakeTexCoords\n" );
		}
#endif
		// rotate is +-90
		if ( texMat[1][0] > 0 ) {
			*rot = 90.0f;
		}
		else{
			*rot = -90.0f;
		}
	}
	else{
		*rot = RAD2DEG( atan2( texMat[1][0] * 1.0L, texMat[0][0] * 1.0L ) );
	}
	shift[0] = -texMat[0][2];
	shift[1] = texMat[1][2];
}

// compute back the texture matrix from fake shift scale rot
// the matrix returned must be understood as a qtexture_t with width=2 height=2 ( the default one )
void FakeTexCoordsToTexMat( float shift[2], float rot, float scale[2], vec_t texMat[2][3] ){
	texMat[0][0] = scale[0] * 1.0L * cos( DEG2RAD( 1.0L * rot ) );
	texMat[1][0] = scale[0] * 1.0L * sin( DEG2RAD( 1.0L * rot ) );
	texMat[0][1] = -scale[1] * 1.0L * sin( DEG2RAD( 1.0L * rot ) );
	texMat[1][1] = scale[1] * 1.0L * cos( DEG2RAD( 1.0L * rot ) );
	texMat[0][2] = -shift[0];
	texMat[1][2] = shift[1];
}

// convert a texture matrix between two qtexture_t
// if NULL for qtexture_t, basic 2x2 texture is assumed ( straight mapping between s/t coordinates and geometric coordinates )
void ConvertTexMatWithQTexture( vec_t texMat1[2][3], qtexture_t *qtex1, vec_t texMat2[2][3], qtexture_t *qtex2 ){
	float s1,s2;
	s1 = ( qtex1 ? static_cast<float>( qtex1->width ) : 2.0f ) / ( qtex2 ? static_cast<float>( qtex2->width ) : 2.0f );
	s2 = ( qtex1 ? static_cast<float>( qtex1->height ) : 2.0f ) / ( qtex2 ? static_cast<float>( qtex2->height ) : 2.0f );
	texMat2[0][0] = s1 * texMat1[0][0];
	texMat2[0][1] = s1 * texMat1[0][1];
	texMat2[0][2] = s1 * texMat1[0][2];
	texMat2[1][0] = s2 * texMat1[1][0];
	texMat2[1][1] = s2 * texMat1[1][1];
	texMat2[1][2] = s2 * texMat1[1][2];
}

void ConvertTexMatWithQTexture( brushprimit_texdef_t *texMat1, qtexture_t *qtex1, brushprimit_texdef_t *texMat2, qtexture_t *qtex2 ){
	ConvertTexMatWithQTexture( texMat1->coords, qtex1, texMat2->coords, qtex2 );
}

// used for texture locking
// will move the texture according to a geometric vector
void ShiftTextureGeometric_BrushPrimit( face_t *f, vec3_t delta ){
	vec3_t texS,texT;
	vec_t tx,ty;
	vec3_t M[3]; // columns of the matrix .. easier that way
	vec_t det;
	vec3_t D[2];
	// compute plane axis base ( doesn't change with translation )
	ComputeAxisBase( f->plane.normal, texS, texT );
	// compute translation vector in plane axis base
	tx = DotProduct( delta, texS );
	ty = DotProduct( delta, texT );
	// fill the data vectors
	M[0][0] = tx; M[0][1] = 1.0f + tx; M[0][2] = tx;
	M[1][0] = ty; M[1][1] = ty; M[1][2] = 1.0f + ty;
	M[2][0] = 1.0f; M[2][1] = 1.0f; M[2][2] = 1.0f;
	D[0][0] = f->brushprimit_texdef.coords[0][2];
	D[0][1] = f->brushprimit_texdef.coords[0][0] + f->brushprimit_texdef.coords[0][2];
	D[0][2] = f->brushprimit_texdef.coords[0][1] + f->brushprimit_texdef.coords[0][2];
	D[1][0] = f->brushprimit_texdef.coords[1][2];
	D[1][1] = f->brushprimit_texdef.coords[1][0] + f->brushprimit_texdef.coords[1][2];
	D[1][2] = f->brushprimit_texdef.coords[1][1] + f->brushprimit_texdef.coords[1][2];
	// solve
	det = SarrusDet( M[0], M[1], M[2] );
	f->brushprimit_texdef.coords[0][0] = SarrusDet( D[0], M[1], M[2] ) / det;
	f->brushprimit_texdef.coords[0][1] = SarrusDet( M[0], D[0], M[2] ) / det;
	f->brushprimit_texdef.coords[0][2] = SarrusDet( M[0], M[1], D[0] ) / det;
	f->brushprimit_texdef.coords[1][0] = SarrusDet( D[1], M[1], M[2] ) / det;
	f->brushprimit_texdef.coords[1][1] = SarrusDet( M[0], D[1], M[2] ) / det;
	f->brushprimit_texdef.coords[1][2] = SarrusDet( M[0], M[1], D[1] ) / det;
}

// shift a texture (texture adjustments) along it's current texture axes
// x and y are geometric values, which we must compute as ST increments
// this depends on the texture size and the pixel/texel ratio
void ShiftTextureRelative_BrushPrimit( face_t *f, float x, float y ){
	float s,t;
	// as a ratio against texture size
	// the scale of the texture is not relevant here (we work directly on a transformation from the base vectors)
	s = ( x * 2.0 ) / (float)f->d_texture->width;
	t = ( y * 2.0 ) / (float)f->d_texture->height;
	f->brushprimit_texdef.coords[0][2] -= s;
	f->brushprimit_texdef.coords[1][2] -= t;
}

// TTimo: FIXME: I don't like that, it feels broken
//   (and it's likely that it's not used anymore)
// best fitted 2D vector is x.X+y.Y
void ComputeBest2DVector( vec3_t v, vec3_t X, vec3_t Y, int &x, int &y ){
	double sx,sy;
	sx = DotProduct( v, X );
	sy = DotProduct( v, Y );
	if ( fabs( sy ) > fabs( sx ) ) {
		x = 0;
		if ( sy > 0.0 ) {
			y =  1;
		}
		else{
			y = -1;
		}
	}
	else
	{
		y = 0;
		if ( sx > 0.0 ) {
			x =  1;
		}
		else{
			x = -1;
		}
	}
}

//++timo FIXME quick'n dirty hack, doesn't care about current texture settings (angle)
// can be improved .. bug #107311
// mins and maxs are the face bounding box
//++timo fixme: we use the face info, mins and maxs are irrelevant
void Face_FitTexture_BrushPrimit( face_t *f, vec3_t mins, vec3_t maxs, int nHeight, int nWidth ){
	vec3_t BBoxSTMin, BBoxSTMax;
	winding_t *w;
	int i,j;
	vec_t val;
	vec3_t M[3],D[2];
//	vec3_t N[2],Mf[2];
	brushprimit_texdef_t N;
	vec3_t Mf[2];


	// we'll be working on a standardized texture size
//	ConvertTexMatWithQTexture( &f->brushprimit_texdef, f->d_texture, &f->brushprimit_texdef, NULL );
	// compute the BBox in ST coords
	EmitBrushPrimitTextureCoordinates( f, f->face_winding );
	ClearBounds( BBoxSTMin, BBoxSTMax );
	w = f->face_winding;
	for ( i = 0 ; i < w->numpoints ; i++ )
	{
		// AddPointToBounds in 2D on (S,T) coordinates
		for ( j = 0 ; j < 2 ; j++ )
		{
			val = w->points[i][j + 3];
			if ( val < BBoxSTMin[j] ) {
				BBoxSTMin[j] = val;
			}
			if ( val > BBoxSTMax[j] ) {
				BBoxSTMax[j] = val;
			}
		}
	}
	// we have the three points of the BBox (BBoxSTMin[0].BBoxSTMin[1]) (BBoxSTMax[0],BBoxSTMin[1]) (BBoxSTMin[0],BBoxSTMax[1]) in ST space
	// the BP matrix we are looking for gives (0,0) (nwidth,0) (0,nHeight) coordinates in (Sfit,Tfit) space to these three points
	// we have A(Sfit,Tfit) = (0,0) = Mf * A(TexS,TexT) = N * M * A(TexS,TexT) = N * A(S,T)
	// so we solve the system for N and then Mf = N * M
	M[0][0] = BBoxSTMin[0]; M[0][1] = BBoxSTMax[0]; M[0][2] = BBoxSTMin[0];
	M[1][0] = BBoxSTMin[1]; M[1][1] = BBoxSTMin[1]; M[1][2] = BBoxSTMax[1];
	D[0][0] = 0.0f; D[0][1] = nWidth; D[0][2] = 0.0f;
	D[1][0] = 0.0f; D[1][1] = 0.0f; D[1][2] = nHeight;
	MatrixForPoints( M, D, &N );

#if 0
	// FIT operation gives coordinates of three points of the bounding box in (S',T'), our target axis base
	// A(S',T')=(0,0) B(S',T')=(nWidth,0) C(S',T')=(0,nHeight)
	// and we have them in (S,T) axis base: A(S,T)=(BBoxSTMin[0],BBoxSTMin[1]) B(S,T)=(BBoxSTMax[0],BBoxSTMin[1]) C(S,T)=(BBoxSTMin[0],BBoxSTMax[1])
	// we compute the N transformation so that: A(S',T') = N * A(S,T)
	VectorSet( N[0], ( BBoxSTMax[0] - BBoxSTMin[0] ) / (float)nWidth, 0.0f, BBoxSTMin[0] );
	VectorSet( N[1], 0.0f, ( BBoxSTMax[1] - BBoxSTMin[1] ) / (float)nHeight, BBoxSTMin[1] );
#endif

	// the final matrix is the product (Mf stands for Mfit)
	Mf[0][0] = N.coords[0][0] * f->brushprimit_texdef.coords[0][0] + N.coords[0][1] * f->brushprimit_texdef.coords[1][0];
	Mf[0][1] = N.coords[0][0] * f->brushprimit_texdef.coords[0][1] + N.coords[0][1] * f->brushprimit_texdef.coords[1][1];
	Mf[0][2] = N.coords[0][0] * f->brushprimit_texdef.coords[0][2] + N.coords[0][1] * f->brushprimit_texdef.coords[1][2] + N.coords[0][2];
	Mf[1][0] = N.coords[1][0] * f->brushprimit_texdef.coords[0][0] + N.coords[1][1] * f->brushprimit_texdef.coords[1][0];
	Mf[1][1] = N.coords[1][0] * f->brushprimit_texdef.coords[0][1] + N.coords[1][1] * f->brushprimit_texdef.coords[1][1];
	Mf[1][2] = N.coords[1][0] * f->brushprimit_texdef.coords[0][2] + N.coords[1][1] * f->brushprimit_texdef.coords[1][2] + N.coords[1][2];
	// copy back
	VectorCopy( Mf[0], f->brushprimit_texdef.coords[0] );
	VectorCopy( Mf[1], f->brushprimit_texdef.coords[1] );
	// handle the texture size
//	ConvertTexMatWithQTexture( &f->brushprimit_texdef, NULL, &f->brushprimit_texdef, f->d_texture );
}

void BrushPrimitFaceToFace( face_t *f ){
#if 0
	// we have parsed brush primitives and need conversion back to standard format
	// NOTE: converting back is a quick hack, there's some information lost and we can't do anything about it
	// FIXME: if we normalize the texture matrix to a standard 2x2 size, we end up with wrong scaling
	// I tried various tweaks, no luck .. seems shifting is lost
	brushprimit_texdef_t aux;
	ConvertTexMatWithQTexture( &face->brushprimit_texdef, face->d_texture, &aux, NULL );
	TexMatToFakeTexCoords( aux.coords, face->texdef.shift, &face->texdef.rotate, face->texdef.scale );
	face->texdef.scale[0] /= 2.0;
	face->texdef.scale[1] /= 2.0;
#else
	// new method by divVerent@alientrap.org: Shift and scale no longer get lost when opening a BP map in texdef mode.
	vec3_t texX,texY;
	vec3_t proj;
	vec_t ST[3][5];

	ComputeAxisBase( f->plane.normal,texX,texY );
	VectorCopy( f->plane.normal,proj );
	VectorScale( proj,f->plane.dist,proj );
	VectorCopy( proj,ST[0] );
	VectorCopy( texX,ST[1] );
	VectorAdd( ST[1],proj,ST[1] );
	VectorCopy( texY,ST[2] );
	VectorAdd( ST[2],proj,ST[2] );

	ST[0][3] = f->brushprimit_texdef.coords[0][2];
	ST[0][4] = f->brushprimit_texdef.coords[1][2];
	ST[1][3] = f->brushprimit_texdef.coords[0][0] + ST[0][3];
	ST[1][4] = f->brushprimit_texdef.coords[1][0] + ST[0][4];
	ST[2][3] = f->brushprimit_texdef.coords[0][1] + ST[0][3];
	ST[2][4] = f->brushprimit_texdef.coords[1][1] + ST[0][4];

	Face_TexdefFromTextureCoordinates( ST[0], ST[1], ST[2], f->d_texture, f );
#endif
}

// TEXTURE LOCKING -----------------------------------------------------------------------------------------------------
// (Relevant to the editor only?)

// internally used for texture locking on rotation and flipping
// the general algorithm is the same for both lockings, it's only the geometric transformation part that changes
// so I wanted to keep it in a single function
// if there are more linear transformations that need the locking, going to a C++ or code pointer solution would be best
// (but right now I want to keep brush_primit.cpp striclty C)

qboolean txlock_bRotation;

// rotation locking params
int txl_nAxis;
float txl_fDeg;
vec3_t txl_vOrigin;

// flip locking params
vec3_t txl_matrix[3];
vec3_t txl_origin;

void TextureLockTransformation_BrushPrimit( face_t *f ){
	vec3_t Orig,texS,texT;        // axis base of initial plane
	// used by transformation algo
	vec3_t temp; int j;
	vec3_t vRotate;                     // rotation vector

	vec3_t rOrig,rvecS,rvecT;     // geometric transformation of (0,0) (1,0) (0,1) { initial plane axis base }
	vec3_t rNormal,rtexS,rtexT;   // axis base for the transformed plane
	vec3_t lOrig,lvecS,lvecT;   // [2] are not used ( but usefull for debugging )
	vec3_t M[3];
	vec_t det;
	vec3_t D[2];

	// compute plane axis base
	ComputeAxisBase( f->plane.normal, texS, texT );
	VectorSet( Orig, 0.0f, 0.0f, 0.0f );

	// compute coordinates of (0,0) (1,0) (0,1) ( expressed in initial plane axis base ) after transformation
	// (0,0) (1,0) (0,1) ( expressed in initial plane axis base ) <-> (0,0,0) texS texT ( expressed world axis base )
	// input: Orig, texS, texT (and the global locking params)
	// ouput: rOrig, rvecS, rvecT, rNormal
	if ( txlock_bRotation ) {
		// rotation vector
		VectorSet( vRotate, 0.0f, 0.0f, 0.0f );
		vRotate[txl_nAxis] = txl_fDeg;
		VectorRotateOrigin( Orig, vRotate, txl_vOrigin, rOrig );
		VectorRotateOrigin( texS, vRotate, txl_vOrigin, rvecS );
		VectorRotateOrigin( texT, vRotate, txl_vOrigin, rvecT );
		// compute normal of plane after rotation
		VectorRotate( f->plane.normal, vRotate, rNormal );
	}
	else
	{
		VectorSubtract( Orig, txl_origin, temp );
		for ( j = 0 ; j < 3 ; j++ )
			rOrig[j] = DotProduct( temp, txl_matrix[j] ) + txl_origin[j];
		VectorSubtract( texS, txl_origin, temp );
		for ( j = 0 ; j < 3 ; j++ )
			rvecS[j] = DotProduct( temp, txl_matrix[j] ) + txl_origin[j];
		VectorSubtract( texT, txl_origin, temp );
		for ( j = 0 ; j < 3 ; j++ )
			rvecT[j] = DotProduct( temp, txl_matrix[j] ) + txl_origin[j];
		// we also need the axis base of the target plane, apply the transformation matrix to the normal too..
		for ( j = 0 ; j < 3 ; j++ )
			rNormal[j] = DotProduct( f->plane.normal, txl_matrix[j] );
	}

	// compute rotated plane axis base
	ComputeAxisBase( rNormal, rtexS, rtexT );
	// compute S/T coordinates of the three points in rotated axis base ( in M matrix )
	lOrig[0] = DotProduct( rOrig, rtexS );
	lOrig[1] = DotProduct( rOrig, rtexT );
	lvecS[0] = DotProduct( rvecS, rtexS );
	lvecS[1] = DotProduct( rvecS, rtexT );
	lvecT[0] = DotProduct( rvecT, rtexS );
	lvecT[1] = DotProduct( rvecT, rtexT );
	M[0][0] = lOrig[0]; M[1][0] = lOrig[1]; M[2][0] = 1.0f;
	M[0][1] = lvecS[0]; M[1][1] = lvecS[1]; M[2][1] = 1.0f;
	M[0][2] = lvecT[0]; M[1][2] = lvecT[1]; M[2][2] = 1.0f;
	// fill data vector
	D[0][0] = f->brushprimit_texdef.coords[0][2];
	D[0][1] = f->brushprimit_texdef.coords[0][0] + f->brushprimit_texdef.coords[0][2];
	D[0][2] = f->brushprimit_texdef.coords[0][1] + f->brushprimit_texdef.coords[0][2];
	D[1][0] = f->brushprimit_texdef.coords[1][2];
	D[1][1] = f->brushprimit_texdef.coords[1][0] + f->brushprimit_texdef.coords[1][2];
	D[1][2] = f->brushprimit_texdef.coords[1][1] + f->brushprimit_texdef.coords[1][2];
	// solve
	det = SarrusDet( M[0], M[1], M[2] );
	f->brushprimit_texdef.coords[0][0] = SarrusDet( D[0], M[1], M[2] ) / det;
	f->brushprimit_texdef.coords[0][1] = SarrusDet( M[0], D[0], M[2] ) / det;
	f->brushprimit_texdef.coords[0][2] = SarrusDet( M[0], M[1], D[0] ) / det;
	f->brushprimit_texdef.coords[1][0] = SarrusDet( D[1], M[1], M[2] ) / det;
	f->brushprimit_texdef.coords[1][1] = SarrusDet( M[0], D[1], M[2] ) / det;
	f->brushprimit_texdef.coords[1][2] = SarrusDet( M[0], M[1], D[1] ) / det;
}

// texture locking
// called before the points on the face are actually rotated
void RotateFaceTexture_BrushPrimit( face_t *f, int nAxis, float fDeg, vec3_t vOrigin ){
	// this is a placeholder to call the general texture locking algorithm
	txlock_bRotation = true;
	txl_nAxis = nAxis;
	txl_fDeg = fDeg;
	VectorCopy( vOrigin, txl_vOrigin );
	TextureLockTransformation_BrushPrimit( f );
}

// compute the new brush primit texture matrix for a transformation matrix and a flip order flag (change plane orientation)
// this matches the select_matrix algo used in select.cpp
// this needs to be called on the face BEFORE any geometric transformation
// it will compute the texture matrix that will represent the same texture on the face after the geometric transformation is done
void ApplyMatrix_BrushPrimit( face_t *f, vec3_t matrix[3], vec3_t origin ){
	// this is a placeholder to call the general texture locking algorithm
	txlock_bRotation = false;
	VectorCopy( matrix[0], txl_matrix[0] );
	VectorCopy( matrix[1], txl_matrix[1] );
	VectorCopy( matrix[2], txl_matrix[2] );
	VectorCopy( origin, txl_origin );
	TextureLockTransformation_BrushPrimit( f );
}

// don't do C==A!
void BPMatMul( vec_t A[2][3], vec_t B[2][3], vec_t C[2][3] ){
	C[0][0] = A[0][0] * B[0][0] + A[0][1] * B[1][0];
	C[1][0] = A[1][0] * B[0][0] + A[1][1] * B[1][0];
	C[0][1] = A[0][0] * B[0][1] + A[0][1] * B[1][1];
	C[1][1] = A[1][0] * B[0][1] + A[1][1] * B[1][1];
	C[0][2] = A[0][0] * B[0][2] + A[0][1] * B[1][2] + A[0][2];
	C[1][2] = A[1][0] * B[0][2] + A[1][1] * B[1][2] + A[1][2];
}

void BPMatDump( vec_t A[2][3] ){
	Sys_Printf( "%g %g %g\n%g %g %g\n0 0 1\n", A[0][0], A[0][1], A[0][2], A[1][0], A[1][1], A[1][2] );
}

void BPMatRotate( vec_t A[2][3], float theta ){
	vec_t m[2][3];
	vec_t aux[2][3];
	memset( &m, 0, sizeof( vec_t ) * 6 );
	m[0][0] = cos( theta * Q_PI / 180.0 );
	m[0][1] = -sin( theta * Q_PI / 180.0 );
	m[1][0] = -m[0][1];
	m[1][1] = m[0][0];
	BPMatMul( A, m, aux );
	BPMatCopy( aux,A );
}

// get the relative axes of the current texturing
void BrushPrimit_GetRelativeAxes( face_t *f, vec3_t vecS, vec3_t vecT ){
	vec_t vS[2],vT[2];
	// first we compute them as expressed in plane axis base
	// BP matrix has coordinates of plane axis base expressed in geometric axis base
	// so we use the line vectors
	vS[0] = f->brushprimit_texdef.coords[0][0];
	vS[1] = f->brushprimit_texdef.coords[0][1];
	vT[0] = f->brushprimit_texdef.coords[1][0];
	vT[1] = f->brushprimit_texdef.coords[1][1];
	// now compute those vectors in geometric space
	vec3_t texS, texT; // axis base of the plane (geometric)
	ComputeAxisBase( f->plane.normal, texS, texT );
	// vecS[] = vS[0].texS[] + vS[1].texT[]
	// vecT[] = vT[0].texS[] + vT[1].texT[]
	vecS[0] = vS[0] * texS[0] + vS[1] * texT[0];
	vecS[1] = vS[0] * texS[1] + vS[1] * texT[1];
	vecS[2] = vS[0] * texS[2] + vS[1] * texT[2];
	vecT[0] = vT[0] * texS[0] + vT[1] * texT[0];
	vecT[1] = vT[0] * texS[1] + vT[1] * texT[1];
	vecT[2] = vT[0] * texS[2] + vT[1] * texT[2];
}

// GL matrix 4x4 product (3D homogeneous matrix)
// NOTE: the crappy thing is that GL doesn't follow the standard convention [line][column]
//   otherwise it's all good
void GLMatMul( vec_t M[4][4], vec_t A[4], vec_t B[4] ){
	unsigned short i,j;
	for ( i = 0; i < 4; i++ )
	{
		B[i] = 0.0;
		for ( j = 0; j < 4; j++ )
		{
			B[i] += M[j][i] * A[j];
		}
	}
}

qboolean IsBrushPrimitMode(){
	return( g_qeglobals.m_bBrushPrimitMode );
}
