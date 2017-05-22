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
#include <assert.h>
#include <glib/gi18n.h>
#include "winding.h"
#include <limits.h>
#include "filters.h"

extern MainFrame* g_pParentWnd;

// globals

int g_nBrushId = 0;

#ifdef ENABLE_GROUPS
const char* Brush_Name( brush_t *b ){
	static char cBuff[1024];
	b->numberId = g_nBrushId++;
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		sprintf( cBuff, "Brush %i", b->numberId );
		Brush_SetEpair( b, "Name", cBuff );
	}
	return cBuff;
}
#endif

brush_t *Brush_Alloc(){
	brush_t *b = (brush_t*)qmalloc( sizeof( brush_t ) );
	return b;
}
/*
   void Brush_Free(brush_t *b)
   {
   free(b);
   }
 */
void PrintWinding( winding_t *w ){
	int i;

	Sys_Printf( "-------------\n" );
	for ( i = 0 ; i < w->numpoints ; i++ )
		Sys_Printf( "(%5.2f, %5.2f, %5.2f)\n", w->points[i][0]
					, w->points[i][1], w->points[i][2] );
}

void PrintPlane( plane_t *p ){
	Sys_Printf( "(%5.2f, %5.2f, %5.2f) : %5.2f\n",  p->normal[0],  p->normal[1],
				p->normal[2],  p->dist );
}

void PrintVector( vec3_t v ){
	Sys_Printf( "(%5.2f, %5.2f, %5.2f)\n",  v[0],  v[1], v[2] );
}


/*
   =============================================================================

            TEXTURE COORDINATES

   =============================================================================
 */


/*
   ==================
   textureAxisFromPlane
   ==================
 */
vec3_t baseaxis[18] =
{
	{0,0,1}, {1,0,0}, {0,-1,0},     // floor
	{0,0,-1}, {1,0,0}, {0,-1,0},    // ceiling
	{1,0,0}, {0,1,0}, {0,0,-1},     // west wall
	{-1,0,0}, {0,1,0}, {0,0,-1},    // east wall
	{0,1,0}, {1,0,0}, {0,0,-1},     // south wall
	{0,-1,0}, {1,0,0}, {0,0,-1}     // north wall
};

void TextureAxisFromPlane( plane_t *pln, vec3_t xv, vec3_t yv ) {
	int bestaxis;
	float dot, best;
	int i;

	best = 0;
	bestaxis = 0;

	for ( i = 0 ; i < 6 ; i++ )
	{
		dot = DotProduct( pln->normal, baseaxis[i * 3] );
                // see q3map2 source - added () for clarity
		if ( ( g_PrefsDlg.m_bQ3Map2Texturing && dot > best + 0.0001f ) || dot > best ) {
			best = dot;
			bestaxis = i;
		}
	}

	VectorCopy( baseaxis[bestaxis * 3 + 1], xv );
	VectorCopy( baseaxis[bestaxis * 3 + 2], yv );
}



float lightaxis[3] = {0.6f, 0.8f, 1.0f};
/*
   ================
   SetShadeForPlane

   Light different planes differently to
   improve recognition
   ================
 */

float SetShadeForPlane( plane_t *p ){
	//return ShadeForNormal(p->normal);


	int i;
	float f;

	// axial plane
	for ( i = 0 ; i < 3 ; i++ )
		if ( fabs( p->normal[i] ) > 0.9 ) {
			f = lightaxis[i];
			return f;
		}

	// between two axial planes
	for ( i = 0 ; i < 3 ; i++ )
		if ( fabs( p->normal[i] ) < 0.1 ) {
			f = ( lightaxis[( i + 1 ) % 3] + lightaxis[( i + 2 ) % 3] ) / 2;
			return f;
		}

	// other
	f = ( lightaxis[0] + lightaxis[1] + lightaxis[2] ) / 3;
	return f;

}

vec3_t vecs[2];
float shift[2];

/*
   ================
   Face_Alloc
   ================
 */
face_t *Face_Alloc( void ){
	face_t *f = (face_t*)qmalloc( sizeof( *f ) );
	return f;
}

/*
   ================
   Face_Free
   ================
 */
void Face_Free( face_t *f ){
	assert( f != 0 );

	if ( f->face_winding ) {
		free( f->face_winding );
		f->face_winding = 0;
	}
	f->texdef.~texdef_t();;

	free( f );
}

/*
   ================
   Face_Clone
   ================
 */
face_t  *Face_Clone( face_t *f ){
	face_t  *n;

	n = Face_Alloc();
	n->texdef = f->texdef;
	n->brushprimit_texdef = f->brushprimit_texdef;

	memcpy( n->planepts, f->planepts, sizeof( n->planepts ) );

	// all other fields are derived, and will be set by Brush_Build
	// FIXME: maybe not, for example n->pData!
	return n;
}

/*
   ================
   Face_FullClone

   makes an exact copy of the face
   ================
 */
face_t  *Face_FullClone( face_t *f ){
	face_t  *n;

	n = Face_Alloc();
	n->texdef = f->texdef;
	n->brushprimit_texdef = f->brushprimit_texdef;
	memcpy( n->planepts, f->planepts, sizeof( n->planepts ) );
	memcpy( &n->plane, &f->plane, sizeof( plane_t ) );
	if ( f->face_winding ) {
		n->face_winding = Winding_Clone( f->face_winding );
	}
	else{
		n->face_winding = NULL;
	}
	n->pShader = f->pShader;
	n->pShader->IncRef();
	n->d_texture = n->pShader->getTexture();
	return n;
}

void Face_SetShader( face_t *face, const char *name ){
	if ( face->pShader != NULL ) {
		face->pShader->DecRef();
	}
	face->texdef.SetName( name );
	face->pShader = QERApp_Shader_ForName( name );
	face->pShader->IncRef();
	face->d_texture = face->pShader->getTexture();
	face->texdef.flags = face->pShader->getFlags();
}

void Face_SetShader( face_t *face, IShader *shader ){
	if ( face->pShader != NULL ) {
		face->pShader->DecRef();
	}
	face->texdef.SetName( shader->getName() );
	face->d_texture = shader->getTexture();
	face->texdef.flags = shader->getFlags();
	face->pShader = shader;
	face->pShader->IncRef();
}

/*
   ================
   Clamp
   ================
 */
void Clamp( float& f, int nClamp ){
	float fFrac = f - static_cast<int>( f );
	f = static_cast<int>( f ) % nClamp;
	f += fFrac;
}

/*
   ================
   Face_MoveTexture
   ================
 */
void Face_MoveTexture( face_t *f, vec3_t delta ){
	vec3_t vX, vY;

	if ( g_qeglobals.m_bBrushPrimitMode ) {
		ShiftTextureGeometric_BrushPrimit( f, delta );
	}
	else
	{
		TextureAxisFromPlane( &f->plane, vX, vY );

		vec3_t vDP, vShift;
		vDP[0] = DotProduct( delta, vX );
		vDP[1] = DotProduct( delta, vY );

		double fAngle = f->texdef.rotate  / 180 * Q_PI;
		double c = cos( fAngle );
		double s = sin( fAngle );

		vShift[0] = vDP[0] * c - vDP[1] * s;
		vShift[1] = vDP[0] * s + vDP[1] * c;

		if ( !f->texdef.scale[0] ) {
			f->texdef.scale[0] = g_PrefsDlg.m_fDefTextureScale;
		}
		if ( !f->texdef.scale[1] ) {
			f->texdef.scale[1] = g_PrefsDlg.m_fDefTextureScale;
		}

		f->texdef.shift[0] -= vShift[0] / f->texdef.scale[0];
		f->texdef.shift[1] -= vShift[1] / f->texdef.scale[1];

		// clamp the shifts
		Clamp( f->texdef.shift[0], f->d_texture->width );
		Clamp( f->texdef.shift[1], f->d_texture->height );
	}
}

/*
   ================
   Face_SetColor
   ================
 */
/*!\todo Replace all face_t::d_texture access with face_t::pShader::GetTexture.*/
void Face_SetColor( brush_t *b, face_t *f, float fCurveColor ){
	// set shading for face
	f->d_shade = SetShadeForPlane( &f->plane );
	f->d_color[0] = f->pShader->getTexture()->color[0] * f->d_shade;
	f->d_color[1] = f->pShader->getTexture()->color[1] * f->d_shade;
	f->d_color[2] = f->pShader->getTexture()->color[2] * f->d_shade;
}

/*
   ================
   Face_TextureVectors
   ================
 */
void Face_TextureVectors( face_t *f, float STfromXYZ[2][4] ){
	vec3_t pvecs[2];
	int sv, tv;
	float ang, sinv, cosv;
	float ns, nt;
	int i,j;
	qtexture_t *q;
	texdef_t    *td;

#ifdef _DEBUG
	// this code is not supposed to be used while in BP mode, warning here can help spot the problem
	if ( g_qeglobals.m_bBrushPrimitMode && !g_qeglobals.bNeedConvert ) {
		Sys_FPrintf( SYS_WRN, "Warning : illegal call of Face_TextureVectors in brush primitive mode\n" );
	}
#endif

	td = &f->texdef;
	q = f->d_texture;

	memset( STfromXYZ, 0, 8 * sizeof( float ) );

	if ( !td->scale[0] ) {
		td->scale[0] = g_PrefsDlg.m_fDefTextureScale;
	}
	if ( !td->scale[1] ) {
		td->scale[1] = g_PrefsDlg.m_fDefTextureScale;
	}

	// get natural texture axis
	TextureAxisFromPlane( &f->plane, pvecs[0], pvecs[1] );

	// rotate axis
	if ( td->rotate == 0 ) {
		sinv = 0 ; cosv = 1;
	}
	else if ( td->rotate == 90 ) {
		sinv = 1 ; cosv = 0;
	}
	else if ( td->rotate == 180 ) {
		sinv = 0 ; cosv = -1;
	}
	else if ( td->rotate == 270 ) {
		sinv = -1 ; cosv = 0;
	}
	else
	{
		ang = td->rotate / 180 * Q_PI;
		sinv = sin( ang );
		cosv = cos( ang );
	}

	if ( pvecs[0][0] ) {
		sv = 0;
	}
	else if ( pvecs[0][1] ) {
		sv = 1;
	}
	else{
		sv = 2;
	}

	if ( pvecs[1][0] ) {
		tv = 0;
	}
	else if ( pvecs[1][1] ) {
		tv = 1;
	}
	else{
		tv = 2;
	}

	for ( i = 0 ; i < 2 ; i++ ) {
		ns = cosv * pvecs[i][sv] - sinv * pvecs[i][tv];
		nt = sinv * pvecs[i][sv] +  cosv * pvecs[i][tv];
		STfromXYZ[i][sv] = ns;
		STfromXYZ[i][tv] = nt;
	}

	// scale
	for ( i = 0 ; i < 2 ; i++ )
		for ( j = 0 ; j < 3 ; j++ )
			STfromXYZ[i][j] = STfromXYZ[i][j] / td->scale[i];

	// shift
	STfromXYZ[0][3] = td->shift[0];
	STfromXYZ[1][3] = td->shift[1];

	for ( j = 0 ; j < 4 ; j++ ) {
		STfromXYZ[0][j] /= q->width;
		STfromXYZ[1][j] /= q->height;
	}
}

long double HighestImpactSign( long double a, long double b ){
	// returns the sign of the value with larger abs
	if ( a + b > 0 ) {
		return +1;
	}
	else{
		return -1;
	}
}

void Face_TexdefFromTextureVectors( face_t *f, long double STfromXYZ[2][4], vec3_t pvecs[2], int sv, int tv ){
	texdef_t *td;
	qtexture_t *q;
	int j;
	long double ang;

	td = &f->texdef;
	q = f->d_texture;

	// undo the texture transform
	for ( j = 0 ; j < 4 ; j++ ) {
		STfromXYZ[0][j] *= q->width;
		STfromXYZ[1][j] *= q->height;
	}

	// shift
	td->shift[0] = STfromXYZ[0][3];
	td->shift[1] = STfromXYZ[1][3];

	/**
	 * SOLVE:
	 *  STfromXYZ[0][sv] = (cosv * pvecs[0][sv] - sinv * pvecs[0][tv]) / td->scale[0];
	 *  STfromXYZ[0][tv] = (sinv * pvecs[0][sv] + cosv * pvecs[0][tv]) / td->scale[0];
	 *  STfromXYZ[1][sv] = (cosv * pvecs[1][sv] - sinv * pvecs[1][tv]) / td->scale[1];
	 *  STfromXYZ[1][tv] = (sinv * pvecs[1][sv] + cosv * pvecs[1][tv]) / td->scale[1];
	 * FOR:
	 *  sinv, cosv, td->scale[0], td->scale[1]
	 * WE KNOW:
	 *  sinv^2 + cosv^2 = 1
	 *  pvecs[0][sv] is +/-1
	 *  pvecs[0][tv] is 0
	 *  pvecs[1][sv] is 0
	 *  pvecs[1][tv] is +/-1
	 * THUS:
	 *  STfromXYZ[0][sv] = +cosv * pvecs[0][sv] / td->scale[0];
	 *  STfromXYZ[0][tv] = +sinv * pvecs[0][sv] / td->scale[0];
	 *  STfromXYZ[1][sv] = -sinv * pvecs[1][tv] / td->scale[1];
	 *  STfromXYZ[1][tv] = +cosv * pvecs[1][tv] / td->scale[1];
	 */

	td->scale[0] = sqrt( STfromXYZ[0][sv] * STfromXYZ[0][sv] + STfromXYZ[0][tv] * STfromXYZ[0][tv] );
	td->scale[1] = sqrt( STfromXYZ[1][sv] * STfromXYZ[1][sv] + STfromXYZ[1][tv] * STfromXYZ[1][tv] );

	if ( td->scale[0] ) {
		td->scale[0] = 1 / td->scale[0]; // avoid NaNs
	}
	if ( td->scale[1] ) {
		td->scale[1] = 1 / td->scale[1];
	}

	long double sign0tv = ( STfromXYZ[0][tv] > 0 ) ? +1 : -1;
	ang = atan2( sign0tv * STfromXYZ[0][tv], sign0tv * STfromXYZ[0][sv] ); // atan2(y, x) with y positive is in [0, PI[

	// STOP
	// We have until now ignored the fact that td->scale[0] or td->scale[1] may
	// have either sign (+ or -). Due to roundoff errors, our choice of
	// sign0tv may even have been wrong in a sense.
	// sign0tv may NOT indicate the appropriate sign for td->scale[0] (namely,
	// if cosv is near zero)!
	// let's look at the signs again
	//   sign0sv =  signcosv * pvecs[0][sv] / td->scale[0]sign
	//   sign0tv =             pvecs[0][sv] / td->scale[0]sign
	//   sign1sv = -1        * pvecs[1][tv] / td->scale[1]sign
	//   sign1tv =  signcosv * pvecs[1][tv] / td->scale[1]sign
	// -->
	//   td->scale[1]sign =  sign1tv * signcosv * pvecs[1][tv]
	//   td->scale[1]sign = -sign1sv * signsinv * pvecs[1][tv]
	//   td->scale[0]sign =  sign0tv * signsinv * pvecs[0][sv]
	//   td->scale[0]sign =  sign0sv * signcosv * pvecs[0][sv]
	// which to choose?
	// the one with the larger impact on the original texcoords, of course
	// to minimize the effect of roundoff errors that may flip the signs!

	td->scale[0] *= HighestImpactSign( STfromXYZ[0][tv] * +sin( ang ), STfromXYZ[0][sv] * cos( ang ) ) * pvecs[0][sv];
	td->scale[1] *= HighestImpactSign( STfromXYZ[1][sv] * -sin( ang ), STfromXYZ[1][tv] * cos( ang ) ) * pvecs[1][tv];

	td->rotate = ang * 180 / Q_PI; // FIXME possibly snap this to 0/90/180 (270 can't happen)?
}


/*
   ================
   Face_MakePlane
   ================
 */
void Face_MakePlane( face_t *f ){
	int j;
	vec3_t t1, t2, t3;

	// convert to a vector / dist plane
	for ( j = 0 ; j < 3 ; j++ )
	{
		t1[j] = f->planepts[0][j] - f->planepts[1][j];
		t2[j] = f->planepts[2][j] - f->planepts[1][j];
		t3[j] = f->planepts[1][j];
	}

	CrossProduct( t1,t2, f->plane.normal );
	if ( VectorCompare( f->plane.normal, vec3_origin ) ) {
		Sys_FPrintf( SYS_WRN, "WARNING: brush plane with no normal\n" );
	}
	VectorNormalize( f->plane.normal, f->plane.normal );
	f->plane.dist = DotProduct( t3, f->plane.normal );
}

/*
   ================
   EmitTextureCoordinates
   ================
 */
void EmitTextureCoordinates( float *xyzst, qtexture_t *q, face_t *f ){
	float STfromXYZ[2][4];

	Face_TextureVectors( f,  STfromXYZ );
	xyzst[3] = DotProduct( xyzst, STfromXYZ[0] ) + STfromXYZ[0][3];
	xyzst[4] = DotProduct( xyzst, STfromXYZ[1] ) + STfromXYZ[1][3];
}

long double SarrusDetScalar( long double a1, long double b1, long double c1, long double a2, long double b2, long double c2, long double a3, long double b3, long double c3 ){
	return a1 * b2 * c3 + a2 * b3 * c1 + a3 * b1 * c2
		   - a1 * c2 * b3 - a2 * c3 * b1 - a3 * c1 * b2;
}

void SarrusSolve( long double a1, long double b1, long double c1, long double d1, long double a2, long double b2, long double c2, long double d2, long double a3, long double b3, long double c3, long double d3, long double *a, long double *b, long double *c ){
	long double det;
	det = SarrusDetScalar( a1, b1, c1,
						   a2, b2, c2,
						   a3, b3, c3 );
	*a =  SarrusDetScalar( d1, b1, c1,
						   d2, b2, c2,
						   d3, b3, c3 ) / det;
	*b =  SarrusDetScalar( a1, d1, c1,
						   a2, d2, c2,
						   a3, d3, c3 ) / det;
	*c =  SarrusDetScalar( a1, b1, d1,
						   a2, b2, d2,
						   a3, b3, d3 ) / det;
}

void Face_TexdefFromTextureCoordinates( float *xyzst1, float *xyzst2, float *xyzst3, qtexture_t *q, face_t *f ){
	vec3_t pvecs[2];
	int sv, tv, uv;

	long double STfromXYZ[2][4];

	// get natural texture axis
	TextureAxisFromPlane( &f->plane, pvecs[0], pvecs[1] );

	if ( pvecs[0][0] ) {
		sv = 0;
	}
	else if ( pvecs[0][1] ) {
		sv = 1;
	}
	else{
		sv = 2;
	}

	if ( pvecs[1][0] ) {
		tv = 0;
	}
	else if ( pvecs[1][1] ) {
		tv = 1;
	}
	else{
		tv = 2;
	}

	uv = 3 - sv - tv; // the "other one"

	// find the STfromXYZ 4-vectors
	/*
	   SARRUS-SOLVE:
	    xyzst1[3] == xyzst1[sv] * STfromXYZ[0][sv] + xyzst1[tv] * STfromXYZ[0][tv] + STfromXYZ[0][3];
	    xyzst2[3] == xyzst2[sv] * STfromXYZ[0][sv] + xyzst2[tv] * STfromXYZ[0][tv] + STfromXYZ[0][3];
	    xyzst3[3] == xyzst3[sv] * STfromXYZ[0][sv] + xyzst3[tv] * STfromXYZ[0][tv] + STfromXYZ[0][3];
	   FOR: STfromXYZ[0]
	   GIVEN: one coord of them (uv) is empty (see Face_TextureVectors)
	   SARRUS-SOLVE:
	    xyzst1[4] == xyzst1[sv] * STfromXYZ[1][sv] + xyzst1[tv] * STfromXYZ[1][tv] + STfromXYZ[1][3];
	    xyzst2[4] == xyzst2[sv] * STfromXYZ[1][sv] + xyzst2[tv] * STfromXYZ[1][tv] + STfromXYZ[1][3];
	    xyzst3[4] == xyzst3[sv] * STfromXYZ[1][sv] + xyzst3[tv] * STfromXYZ[1][tv] + STfromXYZ[1][3];
	   FOR: STfromXYZ[1]
	   GIVEN: one coord of them (uv) is empty (see Face_TextureVectors)
	 */

	STfromXYZ[0][uv] = 0;
	SarrusSolve(
		xyzst1[sv],        xyzst1[tv],        1,               xyzst1[3],
		xyzst2[sv],        xyzst2[tv],        1,               xyzst2[3],
		xyzst3[sv],        xyzst3[tv],        1,               xyzst3[3],
		&STfromXYZ[0][sv], &STfromXYZ[0][tv], &STfromXYZ[0][3]
		);

	STfromXYZ[1][uv] = 0;
	SarrusSolve(
		xyzst1[sv],        xyzst1[tv],        1,               xyzst1[4],
		xyzst2[sv],        xyzst2[tv],        1,               xyzst2[4],
		xyzst3[sv],        xyzst3[tv],        1,               xyzst3[4],
		&STfromXYZ[1][sv], &STfromXYZ[1][tv], &STfromXYZ[1][3]
		);

	/*
	   printf("%s\n", q->name);

	   printf("%f == %Lf\n", xyzst1[3], DotProduct (xyzst1, STfromXYZ[0]) + STfromXYZ[0][3]);
	   printf("%f == %Lf\n", xyzst2[3], DotProduct (xyzst2, STfromXYZ[0]) + STfromXYZ[0][3]);
	   printf("%f == %Lf\n", xyzst3[3], DotProduct (xyzst3, STfromXYZ[0]) + STfromXYZ[0][3]);
	   printf("%f == %Lf\n", xyzst1[4], DotProduct (xyzst1, STfromXYZ[1]) + STfromXYZ[1][3]);
	   printf("%f == %Lf\n", xyzst2[4], DotProduct (xyzst2, STfromXYZ[1]) + STfromXYZ[1][3]);
	   printf("%f == %Lf\n", xyzst3[4], DotProduct (xyzst3, STfromXYZ[1]) + STfromXYZ[1][3]);

	   float   newSTfromXYZ[2][4];

	   printf("old: %Lf,%Lf,%Lf,%Lf %Lf,%Lf,%Lf,%Lf\n",
	    STfromXYZ[0][0], STfromXYZ[0][1], STfromXYZ[0][2], STfromXYZ[0][3],
	    STfromXYZ[1][0], STfromXYZ[1][1], STfromXYZ[1][2], STfromXYZ[1][3]);
	 */

	Face_TexdefFromTextureVectors( f,  STfromXYZ, pvecs, sv, tv );

	/*
	   Face_TextureVectors(f, newSTfromXYZ);

	   printf("new: %f,%f,%f,%f %f,%f,%f,%f\n",
	    newSTfromXYZ[0][0], newSTfromXYZ[0][1], newSTfromXYZ[0][2], newSTfromXYZ[0][3],
	    newSTfromXYZ[1][0], newSTfromXYZ[1][1], newSTfromXYZ[1][2], newSTfromXYZ[1][3]);

	   float newxyzst1[5];
	   float newxyzst2[5];
	   float newxyzst3[5];
	   VectorCopy(xyzst1, newxyzst1);
	   VectorCopy(xyzst2, newxyzst2);
	   VectorCopy(xyzst3, newxyzst3);
	   EmitTextureCoordinates (newxyzst1, q, f);
	   EmitTextureCoordinates (newxyzst2, q, f);
	   EmitTextureCoordinates (newxyzst3, q, f);
	   printf("Face_TexdefFromTextureCoordinates: %f,%f %f,%f %f,%f -> %f,%f %f,%f %f,%f\n",
	    xyzst1[3], xyzst1[4],
	    xyzst2[3], xyzst2[4],
	    xyzst3[3], xyzst3[4],
	    newxyzst1[3], newxyzst1[4],
	    newxyzst2[3], newxyzst2[4],
	    newxyzst3[3], newxyzst3[4]);
	   // TODO why do these differ, but not the previous ones? this makes no sense whatsoever
	 */
}



//==========================================================================

/*
   ================
   Brush_MakeFacePlanes
   ================
 */
void Brush_MakeFacePlanes( brush_t *b ){
	face_t  *f;

	for ( f = b->brush_faces ; f ; f = f->next )
	{
		Face_MakePlane( f );
	}
}

/*
   ================
   DrawBrushEntityName
   ================
 */
void DrawBrushEntityName( brush_t *b ){
	const char  *name;
	float a, s, c;
	vec3_t mid;
	int i;

	if ( !b->owner ) {
		return; // during contruction

	}
	if ( b->owner == world_entity ) {
		return;
	}

	if ( b != b->owner->brushes.onext ) {
		return; // not key brush

	}
	// TTimo: Brush_DrawFacingAngle is for camera view rendering, this function is called for 2D views
	// FIXME - spog - not sure who put this here.. Brush_DrawFacingAngle() does this job?
	// Brush_DrawFacingAngle() works when called, but is not being called.
	if ( g_qeglobals.d_savedinfo.show_angles && ( b->owner->eclass->nShowFlags & ECLASS_ANGLE ) ) {
		// draw the angle pointer
		a = FloatForKey( b->owner, "angle" );
		s = sin( a / 180 * Q_PI );
		c = cos( a / 180 * Q_PI );
		for ( i = 0 ; i < 3 ; i++ )
			mid[i] = ( b->mins[i] + b->maxs[i] ) * 0.5;

		qglBegin( GL_LINE_STRIP );
		qglVertex3fv( mid );
		mid[0] += c * 8;
		mid[1] += s * 8;
		mid[2] += s * 8;
		qglVertex3fv( mid );
		mid[0] -= c * 4;
		mid[1] -= s * 4;
		mid[2] -= s * 4;
		mid[0] -= s * 4;
		mid[1] += c * 4;
		mid[2] += c * 4;
		qglVertex3fv( mid );
		mid[0] += c * 4;
		mid[1] += s * 4;
		mid[2] += s * 4;
		mid[0] += s * 4;
		mid[1] -= c * 4;
		mid[2] -= c * 4;
		qglVertex3fv( mid );
		mid[0] -= c * 4;
		mid[1] -= s * 4;
		mid[2] -= s * 4;
		mid[0] += s * 4;
		mid[1] -= c * 4;
		mid[2] -= c * 4;
		qglVertex3fv( mid );
		qglEnd();
	}

	if ( g_qeglobals.d_savedinfo.show_names ) {
		name = ValueForKey( b->owner, "classname" );
		qglRasterPos3f( b->mins[0] + 4, b->mins[1] + 4, b->mins[2] + 4 );
		gtk_glwidget_print_string( name );
	}
}

/*
   =================
   Brush_MakeFaceWinding

   returns the visible polygon on a face
   =================
 */
winding_t *Brush_MakeFaceWinding( brush_t *b, face_t *face ){
	winding_t   *w;
	face_t      *clip;
	plane_t plane;
	qboolean past;

	// get a poly that covers an effectively infinite area
	w = Winding_BaseForPlane( &face->plane );

	// chop the poly by all of the other faces
	past = false;
	for ( clip = b->brush_faces ; clip && w ; clip = clip->next )
	{
		if ( clip == face ) {
			past = true;
			continue;
		}
		if ( DotProduct( face->plane.normal, clip->plane.normal ) > 0.999
			 && fabs( face->plane.dist - clip->plane.dist ) < 0.01 ) { // identical plane, use the later one
			if ( past ) {
				free( w );
				return NULL;
			}
			continue;
		}

		// flip the plane, because we want to keep the back side
		VectorSubtract( vec3_origin,clip->plane.normal, plane.normal );
		plane.dist = -clip->plane.dist;

		w = Winding_Clip( w, &plane, false );
		if ( !w ) {
			return w;
		}
	}

	if ( w->numpoints < 3 ) {
		free( w );
		w = NULL;
	}

	if ( !w ) {
		Sys_FPrintf( SYS_WRN, "unused plane\n" );
	}

	return w;
}

/*
   =================
   Brush_SnapPlanepts
   =================
 */
void Brush_SnapPlanepts( brush_t *b ){
	int i, j;
	face_t  *f;

	if ( g_PrefsDlg.m_bNoClamp ) {
		return;
	}

	if ( g_qeglobals.d_bSmallGrid ) {
		for ( f = b->brush_faces ; f; f = f->next )
			for ( i = 0 ; i < 3 ; i++ )
				for ( j = 0 ; j < 3 ; j++ )
					f->planepts[i][j] = floor( f->planepts[i][j] / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;
	}
	else
	{
		for ( f = b->brush_faces ; f; f = f->next )
			for ( i = 0 ; i < 3 ; i++ )
				for ( j = 0 ; j < 3 ; j++ )
					f->planepts[i][j] = floor( f->planepts[i][j] + 0.5 );
	}
}

/*
** Brush_Build
**
** Builds a brush rendering data and also sets the min/max bounds
*/
// TTimo
// added a bConvert flag to convert between old and new brush texture formats
// TTimo
// brush grouping: update the group treeview if necessary
void Brush_Build( brush_t *b, bool bSnap, bool bMarkMap, bool bConvert, bool bFilterTest ){
	bool bLocalConvert;


#ifdef _DEBUG
	if ( !g_qeglobals.m_bBrushPrimitMode && bConvert ) {
		Sys_FPrintf( SYS_WRN, "Warning : conversion from brush primitive to old brush format not implemented\n" );
	}
#endif

	// if bConvert is set and g_qeglobals.bNeedConvert is not, that just means we need convert for this brush only
	if ( bConvert && !g_qeglobals.bNeedConvert ) {
#ifdef _DEBUG
		//++timo FIXME: it's not very clear when this can happen, I guess while dealing with plugins that send brushes
		// back and forth in one format or the other .. more when mixing BP / noBP in the same maps.
#endif
		bLocalConvert = true;
		g_qeglobals.bNeedConvert = true;
	}
	else{
		bLocalConvert = false;
	}

	/*
	** build the windings and generate the bounding box
	*/
	Brush_BuildWindings( b, bSnap );

	if ( b->owner->model.pRender ) {
		const aabb_t *aabb = b->owner->model.pRender->GetAABB();
		VectorAdd( aabb->origin, aabb->extents, b->maxs );
		VectorSubtract( aabb->origin, aabb->extents, b->mins );
	}

	//Patch_BuildPoints (b); // does nothing but set b->patchBrush true if the texdef contains SURF_PATCH !

	/*
	** move the points and edges if in select mode
	*/
	if ( g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge ) {
		SetupVertexSelection();
	}

	if ( b->itemOwner == 0 ) { //NULL)
		Group_AddToProperGroup( b );
	}

	if ( bMarkMap ) {
		Sys_MarkMapModified();
	}

	if ( bLocalConvert ) {
		g_qeglobals.bNeedConvert = false;
	}

	// spog - applying filters to brush during brush_build instead of during redraw
	if ( bFilterTest ) {
		b->bFiltered = FilterBrush( b );
	}
}

/*
   ==============
   Brush_SplitBrushByFace

   The incoming brush is NOT freed.
   The incoming face is NOT left referenced.
   ==============
 */
void Brush_SplitBrushByFace( brush_t *in, face_t *f, brush_t **front, brush_t **back, qboolean bCaulk ){
	brush_t *b;
	face_t  *nf;
	vec3_t temp;

	b = Brush_Clone( in );
	nf = Face_Clone( f );

	nf->texdef = b->brush_faces->texdef;
	if ( bCaulk ) {
		nf->texdef.SetName( g_pGameDescription->mCaulkShader.GetBuffer() );
	}
	nf->next = b->brush_faces;
	b->brush_faces = nf;

	Brush_Build( b );
	Brush_RemoveEmptyFaces( b );
	if ( !b->brush_faces ) { // completely clipped away
		Brush_Free( b );
		*back = NULL;
	}
	else
	{
		Entity_LinkBrush( in->owner, b );
		*back = b;
	}

	b = Brush_Clone( in );
	nf = Face_Clone( f );
	// swap the plane winding
	VectorCopy( nf->planepts[0], temp );
	VectorCopy( nf->planepts[1], nf->planepts[0] );
	VectorCopy( temp, nf->planepts[1] );

	nf->texdef = b->brush_faces->texdef;
	if ( bCaulk ) {
		nf->texdef.SetName( g_pGameDescription->mCaulkShader.GetBuffer() );
	}
	nf->next = b->brush_faces;
	b->brush_faces = nf;

	Brush_Build( b );
	Brush_RemoveEmptyFaces( b );
	if ( !b->brush_faces ) { // completely clipped away
		Brush_Free( b );
		*front = NULL;
	}
	else
	{
		Entity_LinkBrush( in->owner, b );
		*front = b;
	}
}

/*
   =================
   Brush_BestSplitFace

   returns the best face to split the brush with.
   return NULL if the brush is convex
   =================
 */
face_t *Brush_BestSplitFace( brush_t *b ){
	face_t *face, *f, *bestface;
	winding_t *front, *back;
	int splits, tinywindings, value, bestvalue;

	bestvalue = 999999;
	bestface = NULL;
	for ( face = b->brush_faces; face; face = face->next )
	{
		splits = 0;
		tinywindings = 0;
		for ( f = b->brush_faces; f; f = f->next )
		{
			if ( f == face ) {
				continue;
			}
			//
			Winding_SplitEpsilon( f->face_winding, face->plane.normal, face->plane.dist, 0.1f, &front, &back );

			if ( !front ) {
				Winding_Free( back );
			}
			else if ( !back ) {
				Winding_Free( front );
			}
			else
			{
				splits++;
				if ( Winding_IsTiny( front ) ) {
					tinywindings++;
				}
				if ( Winding_IsTiny( back ) ) {
					tinywindings++;
				}
			}
		}
		if ( splits ) {
			value = splits + 50 * tinywindings;
			if ( value < bestvalue ) {
				bestvalue = value;
				bestface = face;
			}
		}
	}
	return bestface;
}

/*
   =================
   Brush_MakeConvexBrushes

   MrE FIXME: this doesn't work because the old
           Brush_SplitBrushByFace is used
   Turns the brush into a minimal number of convex brushes.
   If the input brush is convex then it will be returned.
   Otherwise the input brush will be freed.
   NOTE: the input brush should have windings for the faces.
   =================
 */
brush_t *Brush_MakeConvexBrushes( brush_t *b ){
	brush_t *front, *back, *end;
	face_t *face;

	b->next = NULL;
	face = Brush_BestSplitFace( b );
	if ( !face ) {
		return b;
	}
	Brush_SplitBrushByFace( b, face, &front, &back );
	//this should never happen
	if ( !front && !back ) {
		return b;
	}
	Brush_Free( b );
	if ( !front ) {
		return Brush_MakeConvexBrushes( back );
	}
	b = Brush_MakeConvexBrushes( front );
	if ( back ) {
		for ( end = b; end->next; end = end->next ) ;
		end->next = Brush_MakeConvexBrushes( back );
	}
	return b;
}

/*
   =================
   Brush_Convex
   =================
 */
int Brush_Convex( brush_t *b ){
	face_t *face1, *face2;

	for ( face1 = b->brush_faces; face1; face1 = face1->next )
	{
		if ( !face1->face_winding ) {
			continue;
		}
		for ( face2 = b->brush_faces; face2; face2 = face2->next )
		{
			if ( face1 == face2 ) {
				continue;
			}
			if ( !face2->face_winding ) {
				continue;
			}
			if ( Winding_PlanesConcave( face1->face_winding, face2->face_winding,
										face1->plane.normal, face2->plane.normal,
										face1->plane.dist, face2->plane.dist ) ) {
				return false;
			}
		}
	}
	return true;
}

/*
   =================
   Brush_MoveVertexes

   - The input brush must be convex
   - The input brush must have face windings.
   - The output brush will be convex.
   - Returns true if the WHOLE vertex movement is performed.
   =================
 */

// define this to debug the vertex editing mode
#ifdef _DEBUG
//#define DBG_VERT
#endif

#define MAX_MOVE_FACES      64

int Brush_MoveVertex( brush_t *b, vec3_t vertex, vec3_t delta, vec3_t end, bool bSnap ){
	face_t *f, *face, *newface, *lastface, *nextface;
	face_t *movefaces[MAX_MOVE_FACES];
	int movefacepoints[MAX_MOVE_FACES];
	winding_t *w, tmpw;
	vec3_t start, mid;
	plane_t plane;
	int i, j, k, nummovefaces, result, done;
	float dot, front, back, frac, smallestfrac;

#ifdef DBG_VERT
	Sys_Printf( "Bursh_MoveVertex: %p vertex: %g %g %g delta: %g %g %g end: %g %g %g snap: %s\n", b, vertex[0], vertex[1], vertex[2], delta[0], delta[1], delta[2], end[0], end[1], end[2], bSnap ? "true" : "false" );
#endif

	result = true;
	//
	tmpw.numpoints = 3;
	tmpw.maxpoints = 3;
	VectorCopy( vertex, start );
	VectorAdd( vertex, delta, end );
	//snap or not?
	if ( bSnap ) {
		for ( i = 0; i < 3; i++ )
			end[i] = floor( end[i] / g_qeglobals.d_gridsize + 0.1 ) * g_qeglobals.d_gridsize;
	}
	//
	VectorCopy( end, mid );
	//if the start and end are the same
	if ( Point_Equal( start, end, 0.3f ) ) {
		return false;
	}
	//the end point may not be the same as another vertex
	for ( face = b->brush_faces; face; face = face->next )
	{
		w = face->face_winding;
		if ( !w ) {
			continue;
		}
		for ( i = 0; i < w->numpoints; i++ )
		{
			if ( Point_Equal( w->points[i], end, 0.3f ) ) {
				VectorCopy( vertex, end );
				return false;
			}
		}
	}
	//
	done = false;
	while ( !done )
	{
		//chop off triangles from all brush faces that use the to be moved vertex
		//store pointers to these chopped off triangles in movefaces[]
		nummovefaces = 0;
		for ( face = b->brush_faces; face; face = face->next )
		{
			w = face->face_winding;
			if ( !w ) {
				continue;
			}
			for ( i = 0; i < w->numpoints; i++ )
			{
				if ( Point_Equal( w->points[i], start, 0.2f ) ) {
					if ( face->face_winding->numpoints <= 3 ) {
						movefacepoints[nummovefaces] = i;
						movefaces[nummovefaces++] = face;
						break;
					}
					dot = DotProduct( end, face->plane.normal ) - face->plane.dist;
					//if the end point is in front of the face plane
					if ( dot > 0.1 ) {
						//fanout triangle subdivision
						for ( k = i; k < i + w->numpoints - 3; k++ )
						{
							VectorCopy( w->points[i], tmpw.points[0] );
							VectorCopy( w->points[( k + 1 ) % w->numpoints], tmpw.points[1] );
							VectorCopy( w->points[( k + 2 ) % w->numpoints], tmpw.points[2] );
							//
							newface = Face_Clone( face );
							//get the original
							for ( f = face; f->original; f = f->original ) ;
							newface->original = f;
							//store the new winding
							if ( newface->face_winding ) {
								Winding_Free( newface->face_winding );
							}
							newface->face_winding = Winding_Clone( &tmpw );
							//get the texture information
							newface->pShader = face->pShader;
							newface->d_texture = face->d_texture;

							//add the face to the brush
							newface->next = b->brush_faces;
							b->brush_faces = newface;
							//add this new triangle to the move faces
							movefacepoints[nummovefaces] = 0;
							movefaces[nummovefaces++] = newface;
						}
						//give the original face a new winding
						VectorCopy( w->points[( i - 2 + w->numpoints ) % w->numpoints], tmpw.points[0] );
						VectorCopy( w->points[( i - 1 + w->numpoints ) % w->numpoints], tmpw.points[1] );
						VectorCopy( w->points[i], tmpw.points[2] );
						Winding_Free( face->face_winding );
						face->face_winding = Winding_Clone( &tmpw );
						//add the original face to the move faces
						movefacepoints[nummovefaces] = 2;
						movefaces[nummovefaces++] = face;
					}
					else
					{
						//chop a triangle off the face
						VectorCopy( w->points[( i - 1 + w->numpoints ) % w->numpoints], tmpw.points[0] );
						VectorCopy( w->points[i], tmpw.points[1] );
						VectorCopy( w->points[( i + 1 ) % w->numpoints], tmpw.points[2] );
						//remove the point from the face winding
						Winding_RemovePoint( w, i );
						//get texture crap right
						Face_SetColor( b, face, 1.0 );
						for ( j = 0; j < w->numpoints; j++ )
							EmitTextureCoordinates( w->points[j], face->d_texture, face );
						//make a triangle face
						newface = Face_Clone( face );
						//get the original
						for ( f = face; f->original; f = f->original ) ;
						newface->original = f;
						//store the new winding
						if ( newface->face_winding ) {
							Winding_Free( newface->face_winding );
						}
						newface->face_winding = Winding_Clone( &tmpw );
						//get the texture
						newface->pShader = face->pShader;
						newface->d_texture = newface->pShader->getTexture();
//						newface->d_texture = QERApp_Texture_ForName2( newface->texdef.name );
						//add the face to the brush
						newface->next = b->brush_faces;
						b->brush_faces = newface;
						//
						movefacepoints[nummovefaces] = 1;
						movefaces[nummovefaces++] = newface;
					}
					break;
				}
			}
		}
		//now movefaces contains pointers to triangle faces that
		//contain the to be moved vertex
		//
		done = true;
		VectorCopy( end, mid );
		smallestfrac = 1;
		for ( face = b->brush_faces; face; face = face->next )
		{
			//check if there is a move face that has this face as the original
			for ( i = 0; i < nummovefaces; i++ )
			{
				if ( movefaces[i]->original == face ) {
					break;
				}
			}
			if ( i >= nummovefaces ) {
				continue;
			}
			//check if the original is not a move face itself
			for ( j = 0; j < nummovefaces; j++ )
			{
				if ( face == movefaces[j] ) {
					break;
				}
			}
			//if the original is not a move face itself
			if ( j >= nummovefaces ) {
				memcpy( &plane, &movefaces[i]->original->plane, sizeof( plane_t ) );
			}
			else
			{
				k = movefacepoints[j];
				w = movefaces[j]->face_winding;
				VectorCopy( w->points[( k + 1 ) % w->numpoints], tmpw.points[0] );
				VectorCopy( w->points[( k + 2 ) % w->numpoints], tmpw.points[1] );
				//
				k = movefacepoints[i];
				w = movefaces[i]->face_winding;
				VectorCopy( w->points[( k + 1 ) % w->numpoints], tmpw.points[2] );
				if ( !Plane_FromPoints( tmpw.points[0], tmpw.points[1], tmpw.points[2], &plane ) ) {
					VectorCopy( w->points[( k + 2 ) % w->numpoints], tmpw.points[2] );
					if ( !Plane_FromPoints( tmpw.points[0], tmpw.points[1], tmpw.points[2], &plane ) ) {
						//this should never happen otherwise the face merge did a crappy job a previous pass
						continue;
					}
				}
			}
			//now we've got the plane to check agains
			front = DotProduct( start, plane.normal ) - plane.dist;
			back = DotProduct( end, plane.normal ) - plane.dist;
			//if the whole move is at one side of the plane
			if ( front < 0.01 && back < 0.01 ) {
				continue;
			}
			if ( front > -0.01 && back > -0.01 ) {
				continue;
			}
			//if there's no movement orthogonal to this plane at all
			if ( fabs( front - back ) < 0.001 ) {
				continue;
			}
			//ok first only move till the plane is hit
			frac = front / ( front - back );
			if ( frac < smallestfrac ) {
				mid[0] = start[0] + ( end[0] - start[0] ) * frac;
				mid[1] = start[1] + ( end[1] - start[1] ) * frac;
				mid[2] = start[2] + ( end[2] - start[2] ) * frac;
				smallestfrac = frac;
			}
			//
			done = false;
		}

		//move the vertex
		for ( i = 0; i < nummovefaces; i++ )
		{
			//move vertex to end position
			VectorCopy( mid, movefaces[i]->face_winding->points[movefacepoints[i]] );
			//create new face plane
			for ( j = 0; j < 3; j++ )
			{
				VectorCopy( movefaces[i]->face_winding->points[j], movefaces[i]->planepts[j] );
			}
			Face_MakePlane( movefaces[i] );
			if ( VectorLength( movefaces[i]->plane.normal ) < 0.1 ) {
				result = false;
			}
		}
		//if the brush is no longer convex
		if ( !result || !Brush_Convex( b ) ) {
			for ( i = 0; i < nummovefaces; i++ )
			{
				//move the vertex back to the initial position
				VectorCopy( start, movefaces[i]->face_winding->points[movefacepoints[i]] );
				//create new face plane
				for ( j = 0; j < 3; j++ )
				{
					VectorCopy( movefaces[i]->face_winding->points[j], movefaces[i]->planepts[j] );
				}
				Face_MakePlane( movefaces[i] );
			}
			result = false;
			VectorCopy( start, end );
			done = true;
		}
		else
		{
			VectorCopy( mid, start );
		}
		//get texture crap right
		for ( i = 0; i < nummovefaces; i++ )
		{
			Face_SetColor( b, movefaces[i], 1.0 );
			for ( j = 0; j < movefaces[i]->face_winding->numpoints; j++ )
				EmitTextureCoordinates( movefaces[i]->face_winding->points[j], movefaces[i]->d_texture, movefaces[i] );
		}

		//now try to merge faces with their original faces
		lastface = NULL;
		for ( face = b->brush_faces; face; face = nextface )
		{
			nextface = face->next;
			if ( !face->original ) {
				lastface = face;
				continue;
			}
			if ( !Plane_Equal( &face->plane, &face->original->plane, false ) ) {
				lastface = face;
				continue;
			}
			w = Winding_TryMerge( face->face_winding, face->original->face_winding, face->plane.normal, true );
			if ( !w ) {
				lastface = face;
				continue;
			}
			Winding_Free( face->original->face_winding );
			face->original->face_winding = w;
			//get texture crap right
			Face_SetColor( b, face->original, 1.0 );
			for ( j = 0; j < face->original->face_winding->numpoints; j++ )
				EmitTextureCoordinates( face->original->face_winding->points[j], face->original->d_texture, face->original );
			//remove the face that was merged with the original
			if ( lastface ) {
				lastface->next = face->next;
			}
			else{b->brush_faces = face->next; }
			Face_Free( face );
		}
	}
	return result;
}

/*
   =================
   Brush_InsertVertexBetween
   =================
 */
int Brush_InsertVertexBetween( brush_t *b, vec3_t p1, vec3_t p2 ){
	face_t *face;
	winding_t *w, *neww;
	vec3_t point;
	int i, insert;

	if ( Point_Equal( p1, p2, 0.4f ) ) {
		return false;
	}
	VectorAdd( p1, p2, point );
	VectorScale( point, 0.5f, point );
	insert = false;
	//the end point may not be the same as another vertex
	for ( face = b->brush_faces; face; face = face->next )
	{
		w = face->face_winding;
		if ( !w ) {
			continue;
		}
		neww = NULL;
		for ( i = 0; i < w->numpoints; i++ )
		{
			if ( !Point_Equal( w->points[i], p1, 0.1f ) ) {
				continue;
			}
			if ( Point_Equal( w->points[( i + 1 ) % w->numpoints], p2, 0.1f ) ) {
				neww = Winding_InsertPoint( w, point, ( i + 1 ) % w->numpoints );
				break;
			}
			else if ( Point_Equal( w->points[( i - 1 + w->numpoints ) % w->numpoints], p2, 0.3f ) ) {
				neww = Winding_InsertPoint( w, point, i );
				break;
			}
		}
		if ( neww ) {
			Winding_Free( face->face_winding );
			face->face_winding = neww;
			insert = true;
		}
	}
	return insert;
}


/*
   =================
   Brush_ResetFaceOriginals
   =================
 */
void Brush_ResetFaceOriginals( brush_t *b ){
	face_t *face;

	for ( face = b->brush_faces; face; face = face->next )
	{
		face->original = NULL;
	}
}

#ifdef ENABLE_GROUPS
/*
   ==============
   Brush_SetEpair
   sets an epair for the given brush
   ==============
 */
void Brush_SetEpair( brush_t *b, const char *pKey, const char *pValue ){
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		if ( b->patchBrush ) {
			Patch_SetEpair( b->pPatch, pKey, pValue );
		}
		else
		{
			SetKeyValue( b->epairs, pKey, pValue );
		}
	}
	else
	{
		Sys_Printf( "Can only set key/values in Brush primitive mode\n" );
	}
}

/*
   =================
   Brush_GetKeyValue
   =================
 */
const char* Brush_GetKeyValue( brush_t *b, const char *pKey ){
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		if ( b->patchBrush ) {
			return Patch_GetKeyValue( b->pPatch, pKey );
		}
		else
		{
			return ValueForKey( b->epairs, pKey );
		}
	}
	else
	{
		Sys_Printf( "Can only set brush/patch key/values in Brush primitive mode\n" );
	}
	return "";
}
#endif
/*
   =================
   CheckName
   temporary stuff, detect potential problems when saving the texture name
   =================
 */
void CheckName( face_t *fa, char *pname ){
	if ( !strlen( fa->texdef.GetName() ) ) {
#ifdef _DEBUG
		Sys_FPrintf( SYS_WRN, "WARNING: unexpected texdef.name is empty in Brush.cpp CheckName\n" );
#endif
		fa->texdef.SetName( SHADER_NOT_FOUND );
		strcpy( pname, SHADER_NOT_FOUND );
		return;
	}

	// some people manage to get long filename textures (with spaces) in their maps
	if ( strchr( fa->texdef.GetName(), ' ' ) ) {
		char Msg1[1024];

		sprintf( Msg1, "Can't save texture with spaces in name. Rename %s\nNOTE: This message may popup several times .. once for each buggy face detected.", fa->texdef.GetName() );

		Sys_Printf( "%s\n", Msg1 );
		gtk_MessageBox( g_pParentWnd->m_pWidget, Msg1, _( "Error saving map" ), MB_OK );
		strcpy( pname, SHADER_NOT_FOUND );
		return;
	}

	//++timo FIXME: bug #103494 detection attempt
	// TODO: clean this detection part when bug will have disappeared
	if ( fa->texdef.GetName()[0] == '(' ) {
		const char *text = "Bug #103494 detected, dropping texture. Please report to timo@qeradiant.com if you have a way to reproduce!\nNOTE: this message may popup several times .. once for each buggy face detected.";
		Sys_Printf( "%s\n", text );
		gtk_MessageBox( g_pParentWnd->m_pWidget, text, _( "Error saving map" ), MB_OK );
		// need to cleanup this dead face name or we may loop endlessly
		fa->texdef.SetName( SHADER_NOT_FOUND );
		strcpy( pname, SHADER_NOT_FOUND );
		return;
	}
	strcpy( pname, fa->texdef.GetName() + 9 ); // remove "textures/"
}

/*
   =============
   Brush_Create

   Create non-textured blocks for entities
   The brush is NOT linked to any list
   =============
 */
brush_t *Brush_Create( vec3_t mins, vec3_t maxs, texdef_t *texdef ){
	int i, j;
	vec3_t pts[4][2];
	face_t  *f;
	brush_t *b;

#if DBG_BP
	// brush primitive mode : convert texdef to brushprimit_texdef ?
	// most of the time texdef is empty
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		// check texdef is empty .. if there are cases it's not we need to write some conversion code
		if ( texdef->shift[0] != 0 || texdef->shift[1] != 0 || texdef->scale[0] != 0 || texdef->scale[1] != 0 || texdef->rotate != 0 ) {
			Sys_FPrintf( SYS_WRN, "Warning : non-zero texdef detected in Brush_Create .. need brush primitive conversion\n" );
		}
	}
#endif

	for ( i = 0 ; i < 3 ; i++ )
	{
		if ( maxs[i] < mins[i] ) {
			Error( "Brush_InitSolid: backwards" );
		}
	}

	b = Brush_Alloc();

	pts[0][0][0] = mins[0];
	pts[0][0][1] = mins[1];

	pts[1][0][0] = mins[0];
	pts[1][0][1] = maxs[1];

	pts[2][0][0] = maxs[0];
	pts[2][0][1] = maxs[1];

	pts[3][0][0] = maxs[0];
	pts[3][0][1] = mins[1];

	for ( i = 0 ; i < 4 ; i++ )
	{
		pts[i][0][2] = mins[2];
		pts[i][1][0] = pts[i][0][0];
		pts[i][1][1] = pts[i][0][1];
		pts[i][1][2] = maxs[2];
	}

	for ( i = 0 ; i < 4 ; i++ )
	{
		f = Face_Alloc();
		f->texdef = *texdef;
		f->texdef.flags &= ~SURF_KEEP;
		f->texdef.contents &= ~CONTENTS_KEEP;
		f->next = b->brush_faces;
		b->brush_faces = f;
		j = ( i + 1 ) % 4;

		VectorCopy( pts[j][1], f->planepts[0] );
		VectorCopy( pts[i][1], f->planepts[1] );
		VectorCopy( pts[i][0], f->planepts[2] );
	}

	f = Face_Alloc();
	f->texdef = *texdef;
	f->texdef.flags &= ~SURF_KEEP;
	f->texdef.contents &= ~CONTENTS_KEEP;
	f->next = b->brush_faces;
	b->brush_faces = f;

	VectorCopy( pts[0][1], f->planepts[0] );
	VectorCopy( pts[1][1], f->planepts[1] );
	VectorCopy( pts[2][1], f->planepts[2] );

	f = Face_Alloc();
	f->texdef = *texdef;
	f->texdef.flags &= ~SURF_KEEP;
	f->texdef.contents &= ~CONTENTS_KEEP;
	f->next = b->brush_faces;
	b->brush_faces = f;

	VectorCopy( pts[2][0], f->planepts[0] );
	VectorCopy( pts[1][0], f->planepts[1] );
	VectorCopy( pts[0][0], f->planepts[2] );

	return b;
}

/*
   =============
   Brush_CreatePyramid

   Create non-textured pyramid for light entities
   The brush is NOT linked to any list
   =============
 */
brush_t *Brush_CreatePyramid( vec3_t mins, vec3_t maxs, texdef_t *texdef ){
	int i;

	//++timo handle new brush primitive ? return here ??
	return Brush_Create( mins, maxs, texdef );

	for ( i = 0 ; i < 3 ; i++ )
		if ( maxs[i] < mins[i] ) {
			Error( "Brush_InitSolid: backwards" );
		}

	brush_t* b = Brush_Alloc();

	vec3_t corners[4];

	float fMid = Rad_rint( mins[2] + ( Rad_rint( ( maxs[2] - mins[2] ) / 2 ) ) );

	corners[0][0] = mins[0];
	corners[0][1] = mins[1];
	corners[0][2] = fMid;

	corners[1][0] = mins[0];
	corners[1][1] = maxs[1];
	corners[1][2] = fMid;

	corners[2][0] = maxs[0];
	corners[2][1] = maxs[1];
	corners[2][2] = fMid;

	corners[3][0] = maxs[0];
	corners[3][1] = mins[1];
	corners[3][2] = fMid;

	vec3_t top, bottom;

	top[0] = Rad_rint( mins[0] + ( ( maxs[0] - mins[0] ) / 2 ) );
	top[1] = Rad_rint( mins[1] + ( ( maxs[1] - mins[1] ) / 2 ) );
	top[2] = Rad_rint( maxs[2] );

	VectorCopy( top, bottom );
	bottom[2] = mins[2];

	// sides
	for ( i = 0; i < 4; i++ )
	{
		face_t* f = Face_Alloc();
		f->texdef = *texdef;
		f->texdef.flags &= ~SURF_KEEP;
		f->texdef.contents &= ~CONTENTS_KEEP;
		f->next = b->brush_faces;
		b->brush_faces = f;
		int j = ( i + 1 ) % 4;

		VectorCopy( top, f->planepts[0] );
		VectorCopy( corners[i], f->planepts[1] );
		VectorCopy( corners[j], f->planepts[2] );

		f = Face_Alloc();
		f->texdef = *texdef;
		f->texdef.flags &= ~SURF_KEEP;
		f->texdef.contents &= ~CONTENTS_KEEP;
		f->next = b->brush_faces;
		b->brush_faces = f;

		VectorCopy( bottom, f->planepts[2] );
		VectorCopy( corners[i], f->planepts[1] );
		VectorCopy( corners[j], f->planepts[0] );
	}

	return b;
}




/*
   =============
   Brush_MakeSided

   Makes the current brush have the given number of 2d sides
   =============
 */
void Brush_MakeSided( int sides ){
	int i, axis = 0;
	vec3_t mins, maxs;
	brush_t *b;
	texdef_t    *texdef;
	face_t  *f;
	vec3_t mid;
	float width;
	float sv, cv;

	if ( sides < 3 ) {
		Sys_Status( "Bad sides number", 0 );
		return;
	}

	if ( sides >= MAX_POINTS_ON_WINDING - 4 ) {
		Sys_Printf( "too many sides.\n" );
		return;
	}

	if ( !QE_SingleBrush() ) {
		Sys_Status( "Must have a single brush selected", 0 );
		return;
	}

	b = selected_brushes.next;
	VectorCopy( b->mins, mins );
	VectorCopy( b->maxs, maxs );
	texdef = &g_qeglobals.d_texturewin.texdef;

	Brush_Free( b );

	if ( g_pParentWnd->ActiveXY() ) {
		switch ( g_pParentWnd->ActiveXY()->GetViewType() )
		{
		case XY: axis = 2; break;
		case XZ: axis = 1; break;
		case YZ: axis = 0; break;
		}
	}
	else
	{
		axis = 2;
	}

	// find center of brush
	width = 8;
	for ( i = 0; i < 3; i++ )
	{
		mid[i] = ( maxs[i] + mins[i] ) * 0.5;
		if ( i == axis ) {
			continue;
		}
		if ( ( maxs[i] - mins[i] ) * 0.5 > width ) {
			width = ( maxs[i] - mins[i] ) * 0.5;
		}
	}

	b = Brush_Alloc();

	// create top face
	f = Face_Alloc();
	f->texdef = *texdef;
	f->next = b->brush_faces;
	b->brush_faces = f;

	f->planepts[2][( axis + 1 ) % 3] = mins[( axis + 1 ) % 3]; f->planepts[2][( axis + 2 ) % 3] = mins[( axis + 2 ) % 3]; f->planepts[2][axis] = maxs[axis];
	f->planepts[1][( axis + 1 ) % 3] = maxs[( axis + 1 ) % 3]; f->planepts[1][( axis + 2 ) % 3] = mins[( axis + 2 ) % 3]; f->planepts[1][axis] = maxs[axis];
	f->planepts[0][( axis + 1 ) % 3] = maxs[( axis + 1 ) % 3]; f->planepts[0][( axis + 2 ) % 3] = maxs[( axis + 2 ) % 3]; f->planepts[0][axis] = maxs[axis];

	// create bottom face
	f = Face_Alloc();
	f->texdef = *texdef;
	f->next = b->brush_faces;
	b->brush_faces = f;

	f->planepts[0][( axis + 1 ) % 3] = mins[( axis + 1 ) % 3]; f->planepts[0][( axis + 2 ) % 3] = mins[( axis + 2 ) % 3]; f->planepts[0][axis] = mins[axis];
	f->planepts[1][( axis + 1 ) % 3] = maxs[( axis + 1 ) % 3]; f->planepts[1][( axis + 2 ) % 3] = mins[( axis + 2 ) % 3]; f->planepts[1][axis] = mins[axis];
	f->planepts[2][( axis + 1 ) % 3] = maxs[( axis + 1 ) % 3]; f->planepts[2][( axis + 2 ) % 3] = maxs[( axis + 2 ) % 3]; f->planepts[2][axis] = mins[axis];

	for ( i = 0 ; i < sides ; i++ )
	{
		f = Face_Alloc();
		f->texdef = *texdef;
		f->next = b->brush_faces;
		b->brush_faces = f;

		sv = sin( i * 3.14159265 * 2 / sides );
		cv = cos( i * 3.14159265 * 2 / sides );

		f->planepts[0][( axis + 1 ) % 3] = floor( mid[( axis + 1 ) % 3] + width * cv + 0.5 );
		f->planepts[0][( axis + 2 ) % 3] = floor( mid[( axis + 2 ) % 3] + width * sv + 0.5 );
		f->planepts[0][axis] = mins[axis];

		f->planepts[1][( axis + 1 ) % 3] = f->planepts[0][( axis + 1 ) % 3];
		f->planepts[1][( axis + 2 ) % 3] = f->planepts[0][( axis + 2 ) % 3];
		f->planepts[1][axis] = maxs[axis];

		f->planepts[2][( axis + 1 ) % 3] = floor( f->planepts[0][( axis + 1 ) % 3] - width * sv + 0.5 );
		f->planepts[2][( axis + 2 ) % 3] = floor( f->planepts[0][( axis + 2 ) % 3] + width * cv + 0.5 );
		f->planepts[2][axis] = maxs[axis];
	}

	Brush_AddToList( b, &selected_brushes );

	Entity_LinkBrush( world_entity, b );

	Brush_Build( b );

	Sys_UpdateWindows( W_ALL );
}



/*
   =============
   Brush_Free

   Frees the brush with all of its faces and display list.
   Unlinks the brush from whichever chain it is in.
   Decrements the owner entity's brushcount.
   Removes owner entity if this was the last brush
   unless owner is the world.
   Removes from groups
   =============
 */
void Brush_Free( brush_t *b, bool bRemoveNode ){
	face_t  *f, *next;
	epair_t *ep, *enext;

	// remove from group
	if ( bRemoveNode ) {
		Group_RemoveBrush( b );
	}

	// free the patch if it's there
	if ( b->patchBrush ) {
		Patch_Delete( b->pPatch );
	}

	// free faces
	for ( f = b->brush_faces ; f ; f = next )
	{
		next = f->next;
		Face_Free( f );
	}

	// TTimo : free brush epairs
	for ( ep = b->epairs ; ep ; ep = enext )
	{
		enext = ep->next;
		free( ep->key );
		free( ep->value );
		free( ep );
	}

	// unlink from active/selected list
	if ( b->next ) {
		Brush_RemoveFromList( b );
	}

	// unlink from entity list
	if ( b->onext ) {
		Entity_UnlinkBrush( b );
	}

	free( b );
}

/*
   =============
   Face_MemorySize
   =============
 */
int Face_MemorySize( face_t *f ){
	int size = 0;

	if ( f->face_winding ) {
//    size += _msize(f->face_winding);
		size += sizeof( vec3_t ) * f->face_winding->numpoints + 2 * sizeof( int );
	}
//  size += _msize(f);
	size += sizeof( face_t );
	return size;
}

/*
   =============
   Brush_MemorySize
   =============
 */
int Brush_MemorySize( brush_t *b ){
	face_t  *f;
	epair_t *ep;
	int size = 0;

	//
	if ( b->patchBrush ) {
		size += Patch_MemorySize( b->pPatch );
	}
	//
	for ( f = b->brush_faces; f; f = f->next )
	{
		size += Face_MemorySize( f );
	}
	//
	for ( ep = b->epairs; ep; ep = ep->next )
	{
//		size += _msize(ep->key);
		size += strlen( ep->key );
//		size += _msize(ep->value);
		size += strlen( ep->value );
//		size += _msize(ep);
		size += sizeof( epair_t );
	}
//	size += _msize(b);
	size += sizeof( brush_t );
	return size;
}


/*
   ============
   Brush_Clone

   Does NOT add the new brush to any lists
   ============
 */
brush_t *Brush_Clone( brush_t *b ){
	brush_t *n = NULL;
	face_t  *f, *nf;

	if ( b->patchBrush ) {
		patchMesh_t *p = Patch_Duplicate( b->pPatch );
		Brush_RemoveFromList( p->pSymbiot );
		Entity_UnlinkBrush( p->pSymbiot );
		n = p->pSymbiot;
	}
	else
	{
		n = Brush_Alloc();
		n->numberId = g_nBrushId++;
		n->owner = b->owner;
		for ( f = b->brush_faces ; f ; f = f->next )
		{
			nf = Face_Clone( f );
			nf->next = n->brush_faces;
			n->brush_faces = nf;
		}
	}

	return n;
}

/*
   ============
   Brush_Clone

   Does NOT add the new brush to any lists
   ============
 */
brush_t *Brush_FullClone( brush_t *b ){
	brush_t *n = NULL;
	face_t *f, *nf, *f2, *nf2;
	int j;

	if ( b->patchBrush ) {
		patchMesh_t *p = Patch_Duplicate( b->pPatch );
		Brush_RemoveFromList( p->pSymbiot );
		Entity_UnlinkBrush( p->pSymbiot );
		n = p->pSymbiot;
		n->owner = b->owner;
		Brush_Build( n );
	}
	else
	{
		n = Brush_Alloc();
		n->numberId = g_nBrushId++;
		n->owner = b->owner;
		VectorCopy( b->mins, n->mins );
		VectorCopy( b->maxs, n->maxs );
		//
		for ( f = b->brush_faces; f; f = f->next )
		{
			if ( f->original ) {
				continue;
			}
			nf = Face_FullClone( f );
			nf->next = n->brush_faces;
			n->brush_faces = nf;
			//copy all faces that have the original set to this face
			for ( f2 = b->brush_faces; f2; f2 = f2->next )
			{
				if ( f2->original == f ) {
					nf2 = Face_FullClone( f2 );
					nf2->next = n->brush_faces;
					n->brush_faces = nf2;
					//set original
					nf2->original = nf;
				}
			}
		}
		for ( nf = n->brush_faces; nf; nf = nf->next )
		{
			Face_SetColor( n, nf, 1.0 );
			if ( nf->face_winding ) {
				if ( g_qeglobals.m_bBrushPrimitMode ) {
					EmitBrushPrimitTextureCoordinates( nf,nf->face_winding );
				}
				else
				{
					for ( j = 0; j < nf->face_winding->numpoints; j++ )
						EmitTextureCoordinates( nf->face_winding->points[j], nf->d_texture, nf );
				}
			}
		}
	}
	return n;
}

// FIXME - spog - finish this later..
/*
   bool Triangle_Ray(vec3_t origin, vec3_t dir, vec3_t p1, vec3_t p2, vec3_t p3)
   {
   int i;
   vec3_t v1, v2, normal[3];
   float d;

   //Sys_Printf("p1: %f %f %f\n",p1[0],p1[1],p1[2]);
   //Sys_Printf("p2: %f %f %f\n",p2[0],p2[1],p2[2]);
   //Sys_Printf("p3: %f %f %f\n",p3[0],p3[1],p3[2]);
   //Sys_Printf("origin: %f %f %f\n",origin[0],origin[1],origin[2]);

   // test ray against triangle
   // get triangle plane normal
   //VectorSubtract(p1, p2, v1);
   //VectorSubtract(p1, p3, v2);
   //CrossProduct(v1, v2, v1);
   // check normal against direction
   //if (DotProduct(dir, v1) >= 0)
   //{
       // generate cone normals
       VectorSubtract(origin, p1, v1);
       VectorSubtract(origin, p2, v2);
       CrossProduct(v1, v2, normal[0]);
       VectorSubtract(origin, p2, v1);
       VectorSubtract(origin, p3, v2);
       CrossProduct(v1, v2, normal[1]);
       VectorSubtract(origin, p3, v1);
       VectorSubtract(origin, p1, v2);
       CrossProduct(v1, v2, normal[2]);
   //}
   //else
   //{
       // flip normals if triangle faces away
   //	Sys_Printf("flipped\n");
   //	VectorSubtract(origin, p1, v1);
   //	VectorSubtract(origin, p3, v2);
   //	CrossProduct(v1, v2, normal[0]);
   //	VectorSubtract(origin, p3, v1);
   //	VectorSubtract(origin, p2, v2);
   //	CrossProduct(v1, v2, normal[1]);
   //	VectorSubtract(origin, p2, v1);
   //	VectorSubtract(origin, p1, v2);
   //	CrossProduct(v1, v2, normal[2]);
   //}

   for (i=0; i<3; i++)
   {
       VectorNormalize(normal[i]);
       //Sys_Printf("direction: %f %f %f\n",dir[0],dir[1],dir[2]);
       //Sys_Printf("normal: %f %f %f\n",normal[i][0],normal[i][1],normal[i][2]);
       d = DotProduct(dir, normal[i]);
       //Sys_Printf("dotproduct: %f\n",d);
       if (d < 0)
           return false;
   }
   return true;
   }
 */

/*
   extern int Triangle_Ray(float orig[3], float dir[3], bool bCullBack,
                 float vert0[3], float vert1[3], float vert2[3],
                 double *t, double *u, double *v);

   bool Model_Ray(brush_t *b, vec3_t origin, vec3_t dir, double *t, double *u, double *v)
   {
   bool bIntersect = false;
   float tBest = FLT_MAX;
   int i, j;
   vec3_t xyz[3];
   vec3_t vRay[2];

   float angle = FloatForKey (b->owner, "angle"); // FIXME: should be set when this entity key is set

   VectorSubtract (origin, b->owner->origin, vRay[0]);
   VectorCopy (dir, vRay[1]);

   if (angle > 0)
   {
    int i;
    float s, c;
    float x, y;

    s = sin (-angle/180*Q_PI);
    c = cos (-angle/180*Q_PI);

    for (i=0; i<2; i++)
    {
      x = vRay[i][0];
      y = vRay[i][1];
      vRay[i][0] = (x * c) - (y * s);
      vRay[i][1] = (x * s) + (y * c);
    }
   }

   entitymodel *model = b->owner->md3Class->model;

   while (model != NULL)
   {
    for (i = 0; i < model->nTriCount; i++)
    {
      for (j = 0; j < 3; j++)
        VectorCopy(model->pVertList[model->pTriList[i].indexes[j]].v, xyz[j]);

      if (Triangle_Ray(vRay[0], vRay[1], true, xyz[0], xyz[2], xyz[1], t, u, v))
      {
        bIntersect = true;
        if (*t < tBest)
          tBest = *t;
      }
    }
    model = model->pNext;
   }
   if (bIntersect)
   {
   *t = tBest;
    return true;
   }
   else
   {
   *t = 0;
    return false;
   }
   }
 */

/*
   ==============
   Brush_Ray

   Itersects a ray with a brush
   Returns the face hit and the distance along the ray the intersection occured at
   Returns NULL and 0 if not hit at all
   ==============
 */
extern bool Patch_Ray( patchMesh_t *patch, vec3_t origin, vec3_t dir, double *t, double *u, double *v );
face_t *Brush_Ray( vec3_t origin, vec3_t dir, brush_t *b, float *dist, int nFlags ){
	face_t  *f, *firstface = NULL;
	vec3_t p1, p2;
	float frac, d1, d2;
	int i;

	if ( b->owner->eclass->fixedsize
		 && b->owner->model.pSelect
		 && !( !IsBrushSelected( b ) && ( g_PrefsDlg.m_nEntityShowState & ENTITY_SELECTED_ONLY ) )
		 && g_PrefsDlg.m_nEntityShowState != ENTITY_BOX
		 && b->owner->model.pRender->IsModelNotNull() ) {
		ray_t ray_local;
		vec_t dist_local = FLT_MAX;
		ray_construct_for_vec3( &ray_local, origin, dir );
		if ( b->owner->model.pSelect->TestRay( &ray_local, &dist_local ) ) {
			*dist = dist_local;
			return b->brush_faces;
		}
		else
		{
			*dist = 0.0f;
			return NULL;
		}
	}

	VectorCopy( origin, p1 );
	for ( i = 0 ; i < 3 ; i++ )
		p2[i] = p1[i] + dir[i] * 2 * g_MaxWorldCoord;

	for ( f = b->brush_faces ; f ; f = f->next )
	{
		d1 = DotProduct( p1, f->plane.normal ) - f->plane.dist;
		d2 = DotProduct( p2, f->plane.normal ) - f->plane.dist;
		if ( d1 >= 0 && d2 >= 0 ) {
			*dist = 0;
			return NULL;    // ray is on front side of face
		}
		if ( d1 <= 0 && d2 <= 0 ) {
			continue;
		}
		// clip the ray to the plane
		frac = d1 / ( d1 - d2 );
		if ( d1 > 0 ) {
			firstface = f;
			for ( i = 0 ; i < 3 ; i++ )
				p1[i] = p1[i] + frac * ( p2[i] - p1[i] );
		}
		else {
			for ( i = 0 ; i < 3 ; i++ )
				p2[i] = p1[i] + frac * ( p2[i] - p1[i] );
		}
	}

	// find distance p1 is along dir
	VectorSubtract( p1, origin, p1 );
	d1 = DotProduct( p1, dir );

	*dist = d1;

	// new test stuff for patches
	if ( !g_PrefsDlg.m_bPatchBBoxSelect && b->patchBrush ) {
		double t, u, v; // t is the distance from origin to point-of-intersection.. er.. i don't know what u and v are
		if ( !Patch_Ray( b->pPatch, origin, dir, &t, &u, &v ) ) {
			*dist = 0;
			return NULL;
		}
		else
		{
			*dist = (float)t;
			//Sys_Printf("t: %f, u: %f, v: %f\n", t, u, v);
		}
	}

	// IMPORTANT NOTE:
	// modifications to the discarding code here should be matched in the selection code
	// see Brush_Draw

	// do some last minute filtering
	if ( firstface && ( nFlags & SF_CAMERA ) ) {
		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_CAULK ) {
			if ( strstr( firstface->texdef.GetName(), "caulk" ) ) {
				*dist = 0;
				return NULL;
			}
		}
		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_BOTCLIP ) {
			if ( strstr( firstface->texdef.GetName(), "botclip" ) || strstr( firstface->texdef.GetName(), "clipmonster" ) ) {
				*dist = 0;
				return NULL;
			}
		}
		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_CLIP ) {
			if ( strstr( firstface->texdef.GetName(), "clip" ) ) {
				*dist = 0;
				return NULL;
			}
		}
	}

	return firstface;
}

//PGM
face_t *Brush_Point( vec3_t origin, brush_t *b ){
	face_t  *f;
	float d1;

	for ( f = b->brush_faces ; f ; f = f->next )
	{
		d1 = DotProduct( origin, f->plane.normal ) - f->plane.dist;
		if ( d1 > 0 ) {
			return NULL;    // point is on front side of face
		}
	}

	return b->brush_faces;
}
//PGM


void    Brush_AddToList( brush_t *b, brush_t *blist ){
	if ( b->next || b->prev ) {
		Error( "Brush_AddToList: already linked" );
	}

	if ( blist == &selected_brushes || blist == &active_brushes ) {
		if ( b->patchBrush && blist == &selected_brushes ) {
			Patch_Select( b->pPatch );
		}
	}
	b->next = blist->next;
	blist->next->prev = b;
	blist->next = b;
	b->prev = blist;

	// TTimo messaging
	DispatchRadiantMsg( RADIANT_SELECTION );
}

void    Brush_RemoveFromList( brush_t *b ){
	if ( !b->next || !b->prev ) {
		Error( "Brush_RemoveFromList: not linked" );
	}

	if ( b->patchBrush ) {
		Patch_Deselect( b->pPatch );
	}
	b->next->prev = b->prev;
	b->prev->next = b->next;
	b->next = b->prev = NULL;
}

/*
   ===============
   SetFaceTexdef

   Doesn't set the curve flags

   NOTE : ( TTimo )
    never trust f->d_texture here, f->texdef and f->d_texture are out of sync when called by Brush_SetTexture
    use Texture_ForName() to find the right shader
    FIXME : send the right shader ( qtexture_t * ) in the parameters ?

   TTimo: surface plugin, added an IPluginTexdef* parameter
        if not NULL, get ->Copy() of it into the face ( and remember to hook )
        if NULL, ask for a default

   TTimo - shader code cleanup
   added IShader* parameter
   ===============
 */
void SetFaceTexdef2( brush_t *b, face_t *f, IShader *pShader, texdef_t *texdef, brushprimit_texdef_t *brushprimit_texdef, bool bFitScale, IPluginTexdef* pPlugTexdef ) {
	int oldFlags;
	int oldContents;
	face_t  *tf;

	oldFlags = f->texdef.flags;
	oldContents = f->texdef.contents;
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		f->texdef = *texdef;
		ConvertTexMatWithQTexture( brushprimit_texdef, NULL, &f->brushprimit_texdef, QERApp_Shader_ForName( f->texdef.GetName() )->getTexture() );
	}
	else
	if ( bFitScale ) {
		f->texdef = *texdef;
		// fit the scaling of the texture on the actual plane
		vec3_t p1,p2,p3;     // absolute coordinates
		// compute absolute coordinates
		ComputeAbsolute( f,p1,p2,p3 );
		// compute the scale
		vec3_t vx,vy;
		VectorSubtract( p2,p1,vx );
		VectorNormalize( vx, vx );
		VectorSubtract( p3,p1,vy );
		VectorNormalize( vy, vy );
		// assign scale
		VectorScale( vx,texdef->scale[0],vx );
		VectorScale( vy,texdef->scale[1],vy );
		VectorAdd( p1,vx,p2 );
		VectorAdd( p1,vy,p3 );
		// compute back shift scale rot
		AbsoluteToLocal( f->plane,f,p1,p2,p3 );
	}
	else{
		f->texdef = *texdef;
	}
	f->texdef.flags = ( f->texdef.flags & ~SURF_KEEP ) | ( oldFlags & SURF_KEEP );
	f->texdef.contents = ( f->texdef.contents & ~CONTENTS_KEEP ) | ( oldContents & CONTENTS_KEEP );

	// if this is a curve face, set all other curve faces to the same texdef
	if ( f->texdef.flags & SURF_CURVE ) {
		for ( tf = b->brush_faces ; tf ; tf = tf->next )
		{
			if ( tf->texdef.flags & SURF_CURVE ) {
				tf->texdef = f->texdef;
			}
		}
	}
}

/*
   ===============
   SetFaceTexdef

   Doesn't set the curve flags

   NOTE : ( TTimo )
    never trust f->d_texture here, f->texdef and f->d_texture are out of sync when called by Brush_SetTexture
    use Texture_ForName() to find the right shader
    FIXME : send the right shader ( qtexture_t * ) in the parameters ?

   TTimo: surface plugin, added an IPluginTexdef* parameter
        if not NULL, get ->Copy() of it into the face ( and remember to hook )
        if NULL, ask for a default
   ===============
 */
void SetFaceTexdef( face_t *f, texdef_t *texdef, brushprimit_texdef_t *brushprimit_texdef, bool bFitScale, IPluginTexdef* pPlugTexdef ) {
	int oldFlags;
	int oldContents;

	oldFlags = f->texdef.flags;
	oldContents = f->texdef.contents;

	if ( strcmp( f->texdef.GetName(), texdef->GetName() ) != 0 ) { // set shader here instead of Brush_Build
		Face_SetShader( f, texdef->GetName() );
	}

	if ( g_qeglobals.m_bBrushPrimitMode ) {
		f->texdef = *texdef;
		ConvertTexMatWithQTexture( brushprimit_texdef, NULL, &f->brushprimit_texdef, QERApp_Shader_ForName( f->texdef.GetName() )->getTexture() );
	}
	else
	{
		if ( bFitScale ) {
			f->texdef = *texdef;
			// fit the scaling of the texture on the actual plane
			vec3_t p1,p2,p3; // absolute coordinates
			// compute absolute coordinates
			ComputeAbsolute( f,p1,p2,p3 );
			// compute the scale
			vec3_t vx,vy;
			VectorSubtract( p2,p1,vx );
			VectorNormalize( vx, vx );
			VectorSubtract( p3,p1,vy );
			VectorNormalize( vy, vy );
			// assign scale
			VectorScale( vx,texdef->scale[0],vx );
			VectorScale( vy,texdef->scale[1],vy );
			VectorAdd( p1,vx,p2 );
			VectorAdd( p1,vy,p3 );
			// compute back shift scale rot
			AbsoluteToLocal( f->plane,f,p1,p2,p3 );
		}
		else
		{
			f->texdef = *texdef;
		}
	}
	f->texdef.flags = ( f->texdef.flags & ~SURF_KEEP ) | ( oldFlags & SURF_KEEP );
	f->texdef.contents = ( f->texdef.contents & ~CONTENTS_KEEP ) | ( oldContents & CONTENTS_KEEP );
}

#ifdef _DEBUG
void Brush_SetTexture2( brush_t *b, IShader *pShader, texdef_t *texdef, brushprimit_texdef_t *brushprimit_texdef, bool bFitScale, IPluginTexdef* pTexdef ){
	for ( face_t* f = b->brush_faces ; f ; f = f->next )
		SetFaceTexdef2( b, f, pShader, texdef, brushprimit_texdef, bFitScale, pTexdef );
	Brush_Build( b );
	if ( b->patchBrush ) {
		Patch_SetTexture( b->pPatch, texdef, pTexdef );
		b->bFiltered = FilterBrush( b );
	}
}
#endif

void Brush_SetTexture( brush_t *b, texdef_t *texdef, brushprimit_texdef_t *brushprimit_texdef, bool bFitScale, IPluginTexdef* pTexdef ){
	for ( face_t* f = b->brush_faces ; f ; f = f->next )
		SetFaceTexdef( f, texdef, brushprimit_texdef, bFitScale, pTexdef );
	Brush_Build( b );
	if ( b->patchBrush ) {
		Patch_SetTexture( b->pPatch, texdef, pTexdef );
		b->bFiltered = FilterBrush( b );
	}
}


qboolean ClipLineToFace( vec3_t p1, vec3_t p2, face_t *f ){
	float d1, d2, fr;
	int i;
	float   *v;

	d1 = DotProduct( p1, f->plane.normal ) - f->plane.dist;
	d2 = DotProduct( p2, f->plane.normal ) - f->plane.dist;

	if ( d1 >= 0 && d2 >= 0 ) {
		return false;       // totally outside
	}
	if ( d1 <= 0 && d2 <= 0 ) {
		return true;        // totally inside

	}
	fr = d1 / ( d1 - d2 );

	if ( d1 > 0 ) {
		v = p1;
	}
	else{
		v = p2;
	}

	for ( i = 0 ; i < 3 ; i++ )
		v[i] = p1[i] + fr * ( p2[i] - p1[i] );

	return true;
}


int AddPlanept( float *f ){
	int i;

	for ( i = 0 ; i < g_qeglobals.d_num_move_points ; i++ )
		if ( g_qeglobals.d_move_points[i] == f ) {
			return 0;
		}
	g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = f;
	return 1;
}

/*
   ==============
   Brush_SelectFaceForDragging

   Adds the faces planepts to move_points, and
   rotates and adds the planepts of adjacent face if shear is set
   ==============
 */
void Brush_SelectFaceForDragging( brush_t *b, face_t *f, qboolean shear ){
	int i;
	face_t  *f2;
	winding_t   *w;
	float d;
	brush_t *b2;
	int c;

	if ( b->owner->eclass->fixedsize ) {
		return;
	}

	c = 0;
	for ( i = 0 ; i < 3 ; i++ )
		c += AddPlanept( f->planepts[i] );
	if ( c == 0 ) {
		return;     // already completely added

	}
	// select all points on this plane in all brushes the selection
	for ( b2 = selected_brushes.next ; b2 != &selected_brushes ; b2 = b2->next )
	{
		if ( b2 == b ) {
			continue;
		}
		for ( f2 = b2->brush_faces ; f2 ; f2 = f2->next )
		{
			for ( i = 0 ; i < 3 ; i++ )
				if ( fabs( DotProduct( f2->planepts[i], f->plane.normal )
						   - f->plane.dist ) > ON_EPSILON ) {
					break;
				}
			if ( i == 3 ) { // move this face as well
				Brush_SelectFaceForDragging( b2, f2, shear );
				break;
			}
		}
	}


	// if shearing, take all the planes adjacent to
	// selected faces and rotate their points so the
	// edge clipped by a selcted face has two of the points
	if ( !shear ) {
		return;
	}

	for ( f2 = b->brush_faces ; f2 ; f2 = f2->next )
	{
		if ( f2 == f ) {
			continue;
		}
		w = Brush_MakeFaceWinding( b, f2 );
		if ( !w ) {
			continue;
		}

		// any points on f will become new control points
		for ( i = 0 ; i < w->numpoints ; i++ )
		{
			d = DotProduct( w->points[i], f->plane.normal )
				- f->plane.dist;
			if ( d > -ON_EPSILON && d < ON_EPSILON ) {
				break;
			}
		}

		//
		// if none of the points were on the plane,
		// leave it alone
		//
		if ( i != w->numpoints ) {
			// see if the first clockwise point was the
			// last point on the winding
			if ( i == 0 ) {
				d = DotProduct( w->points[w->numpoints - 1]
								, f->plane.normal ) - f->plane.dist;
				if ( d > -ON_EPSILON && d < ON_EPSILON ) {
					i = w->numpoints - 1;
				}
			}

			AddPlanept( f2->planepts[0] );

			VectorCopy( w->points[i], f2->planepts[0] );
			if ( ++i == w->numpoints ) {
				i = 0;
			}

			// see if the next point is also on the plane
			d = DotProduct( w->points[i]
							, f->plane.normal ) - f->plane.dist;
			if ( d > -ON_EPSILON && d < ON_EPSILON ) {
				AddPlanept( f2->planepts[1] );
			}

			VectorCopy( w->points[i], f2->planepts[1] );
			if ( ++i == w->numpoints ) {
				i = 0;
			}

			// the third point is never on the plane

			VectorCopy( w->points[i], f2->planepts[2] );
		}

		free( w );
	}
}

/*
   ==============
   Brush_SideSelect

   The mouse click did not hit the brush, so grab one or more side
   planes for dragging
   ==============
 */
void Brush_SideSelect( brush_t *b, vec3_t origin, vec3_t dir
					   , qboolean shear ){
	face_t  *f, *f2;
	vec3_t p1, p2;

	for ( f = b->brush_faces ; f ; f = f->next )
	{
		VectorCopy( origin, p1 );
		VectorMA( origin, 2 * g_MaxWorldCoord, dir, p2 );

		for ( f2 = b->brush_faces ; f2 ; f2 = f2->next )
		{
			if ( f2 == f ) {
				continue;
			}
			ClipLineToFace( p1, p2, f2 );
		}

		if ( f2 ) {
			continue;
		}

		if ( VectorCompare( p1, origin ) ) {
			continue;
		}
		if ( ClipLineToFace( p1, p2, f ) ) {
			continue;
		}

		Brush_SelectFaceForDragging( b, f, shear );
	}
}

bool g_bBuildWindingsNoTexBuild = false;

void Brush_SetBuildWindingsNoTexBuild( bool bBuild ){
	g_bBuildWindingsNoTexBuild = bBuild;
}

// TTimo: don't rebuild pShader and d_texture if it doesn't seem necessary
//    saves quite a lot of time, but on the other hand we've gotta make sure we clean the d_texture in some cases
//    ie when we want to update a shader
//    default will make Radiant rebuild the texture, but it can be turned off by setting the flag g_bBuildWindingsNoTexBuild
void Brush_BuildWindings( brush_t *b, bool bSnap ){
	winding_t *w;
	face_t    *face;
	vec_t v;

	if ( bSnap ) {
		Brush_SnapPlanepts( b );
	}

	// clear the mins/maxs bounds
	b->mins[0] = b->mins[1] = b->mins[2] = 99999;
	b->maxs[0] = b->maxs[1] = b->maxs[2] = -99999;

	Brush_MakeFacePlanes( b );

	face = b->brush_faces;

	float fCurveColor = 1.0;

	for ( ; face ; face = face->next )
	{
		int i, j;
		free( face->face_winding );
		w = face->face_winding = Brush_MakeFaceWinding( b, face );

		if ( !g_bBuildWindingsNoTexBuild || !face->d_texture ) {
#ifdef _DEBUG
			// if there's no d_texture, then we expect pShader to be empty
			if ( !face->d_texture && face->pShader ) {
				Sys_FPrintf( SYS_ERR, "ERROR: unexpected face->pShader != NULL with face->d_texture == NULL in Brush_BuildWindings\n" );
			}
#endif
			if ( ( !face->d_texture && !face->pShader ) || !face->pShader ) {
				// NOTE TTimo
				// patch 84 for bug 253 doesn't dec ref the potential face->pShader
				// add a debug check to make sure this is actually not necessary
#ifdef _DEBUG
				if ( face->pShader ) {
					Sys_FPrintf( SYS_ERR, "ERROR: face->pShader != NULL in Brush_BuildWindings\n" );
				}
#endif
				face->pShader = QERApp_Shader_ForName( face->texdef.GetName() );
				face->pShader->IncRef();
				face->d_texture = face->pShader->getTexture();
			}
		}

		if ( !w ) {
			continue;
		}

		for ( i = 0 ; i < w->numpoints ; i++ )
		{
			// add to bounding box
			for ( j = 0 ; j < 3 ; j++ )
			{
				v = w->points[i][j];
				if ( v > b->maxs[j] ) {
					b->maxs[j] = v;
				}
				if ( v < b->mins[j] ) {
					b->mins[j] = v;
				}
			}
		}
		Face_SetColor( b, face, fCurveColor );

		fCurveColor -= .10f;
		if ( fCurveColor <= 0 ) {
			fCurveColor = 1.0f;
		}

		// computing ST coordinates for the windings
		if ( g_qeglobals.m_bBrushPrimitMode ) {
			if ( g_qeglobals.bNeedConvert ) {
				// we have parsed old brushes format and need conversion
				// convert old brush texture representation to new format
				FaceToBrushPrimitFace( face );
#ifdef _DEBUG
				// use old texture coordinates code to check against
				for ( i = 0 ; i < w->numpoints ; i++ )
					EmitTextureCoordinates( w->points[i], face->d_texture, face );
#endif
			}
			// use new texture representation to compute texture coordinates
			// in debug mode we will check against old code and warn if there are differences
			EmitBrushPrimitTextureCoordinates( face,w );
		}
		else
		{
			if ( g_qeglobals.bNeedConvert ) {
				BrushPrimitFaceToFace( face );
/*
        // we have parsed brush primitives and need conversion back to standard format
        // NOTE: converting back is a quick hack, there's some information lost and we can't do anything about it
                // FIXME: if we normalize the texture matrix to a standard 2x2 size, we end up with wrong scaling
                // I tried various tweaks, no luck .. seems shifting is lost
        brushprimit_texdef_t aux;
        ConvertTexMatWithQTexture( &face->brushprimit_texdef, face->d_texture, &aux, NULL );
        TexMatToFakeTexCoords( aux.coords, face->texdef.shift, &face->texdef.rotate, face->texdef.scale );
                face->texdef.scale[0]/=2.0;
                face->texdef.scale[1]/=2.0;
 */
			}
			for ( i = 0 ; i < w->numpoints ; i++ )
				EmitTextureCoordinates( w->points[i], face->d_texture, face );
		}
	}
}

/*
   ==================
   Brush_RemoveEmptyFaces

   Frees any overconstraining faces
   ==================
 */
void Brush_RemoveEmptyFaces( brush_t *b ){
	face_t  *f, *next;

	f = b->brush_faces;
	b->brush_faces = NULL;

	for ( ; f ; f = next )
	{
		next = f->next;
		if ( !f->face_winding ) {
			Face_Free( f );
		}
		else
		{
			f->next = b->brush_faces;
			b->brush_faces = f;
		}

	}
}

void Brush_SnapToGrid( brush_t *pb ){
	face_t *f;
	vec3_t temp;
	vec3_t diff[2];
	int mult[3];
	int i, j, n;
	// TTimo: some brushes are "special" and should not be snapped
	// specially fixed-size entity ones
	if ( pb->owner->eclass->fixedsize ) {
		// save current origin
		VectorCopy( pb->owner->origin, temp );
		// snap the origin
		VectorFSnap( pb->owner->origin, g_qeglobals.d_gridsize );
		// return if amount is zero
		if ( VectorCompare( pb->owner->origin, temp ) ) {
			return;
		}
		// transform brush faces same amount
		VectorSubtract( pb->owner->origin, temp, temp );
		for ( f = pb->brush_faces; f; f = f->next )
		{
			for ( i = 0 ; i < 3 ; i++ )
				VectorAdd( f->planepts[i], temp, f->planepts[i] );
		}
	}
	else
	{
		for ( f = pb->brush_faces ; f; f = f->next )
		{
			for ( j = 0; j < 2; j++ )
			{
				// spog - move planepts apart just far enough to avoid snapping two together
				VectorSubtract( f->planepts[j + 1], f->planepts[j], diff[j] );
				for ( i = 0; i < 3; i++ )
				{
					if ( diff[j][i] == 0.0f ) {
						mult[i] = 2; // next value up from 1
					}
					else{ // multiplier = gridsize / component difference, rounded up
						mult[i] = (int)ceil( fabs( g_qeglobals.d_gridsize / diff[j][i] ) );
					}
				}

				if ( mult[0] > 1 && mult[1] > 1 && mult[2] > 1 ) { // if all multipliers are greater than 1
					n = ( mult[0] >= mult[1] && mult[0] >= mult[2] ) ? 0 : ( mult[1] >= mult[0] && mult[1] >= mult[2] ) ? 1 : 2;
					for ( i = 0; i < 3; i++ )
						diff[j][i] *= mult[n];  // multiply difference by multiplier of smallest component
				}
				VectorAdd( f->planepts[j], diff[j], f->planepts[j + 1] );
			}

			for ( i = 0; i < 3; i++ )
				VectorFSnap( f->planepts[i], g_qeglobals.d_gridsize );

		}
	}
	Brush_Build( pb,true,true,false,false ); // don't filter
}

void Brush_Rotate( brush_t *b, vec3_t vAngle, vec3_t vOrigin, bool bBuild ){
	for ( face_t* f = b->brush_faces ; f ; f = f->next )
	{
		for ( int i = 0 ; i < 3 ; i++ )
		{
			VectorRotateOrigin( f->planepts[i], vAngle, vOrigin, f->planepts[i] );
		}
	}
	if ( bBuild ) {
		Brush_Build( b,false,false,false,false ); // don't filter
	}
}

void Brush_Center( brush_t *b, vec3_t vNewCenter ){
	vec3_t vMid;
	// get center of the brush
	for ( int j = 0; j < 3; j++ )
	{
		vMid[j] = b->mins[j] + fabs( ( b->maxs[j] - b->mins[j] ) * 0.5 );
	}
	// calc distance between centers
	VectorSubtract( vNewCenter, vMid, vMid );
	Brush_Move( b, vMid, true );

}

void Brush_Resize( brush_t *b, vec3_t vMin, vec3_t vMax ){
	face_t *f;
	texdef_t texdef;
	int i;
	short box[3][2] = { { 0, 1 }, { 2, 0 }, { 1, 2 } };

	for ( i = 0 ; i < 3 ; i++ )
		if ( vMax[i] < vMin[i] ) {
			Error( "Brush_Resize: invalid input" );
		}

	if ( b->brush_faces != NULL ) {
		texdef = b->brush_faces->texdef;
	}
	else{
		texdef = g_qeglobals.d_texturewin.texdef;
	}

	while ( b->brush_faces != NULL )
	{
		f = b->brush_faces->next;
		Face_Free( b->brush_faces );
		b->brush_faces = f;
	}

	for ( i = 0; i < 3; i++ )
	{
		f = b->brush_faces;
		b->brush_faces = Face_Alloc();
		b->brush_faces->next = f;
		f = b->brush_faces;
		f->texdef = texdef;
		VectorCopy( vMax, f->planepts[0] );
		VectorCopy( vMax, f->planepts[1] );
		VectorCopy( vMax, f->planepts[2] );
		f->planepts[2][box[i][0]] = vMin[box[i][0]];
		f->planepts[1][box[i][1]] = vMin[box[i][1]];
	}
	for ( i = 0; i < 3; i++ )
	{
		f = b->brush_faces;
		b->brush_faces = Face_Alloc();
		b->brush_faces->next = f;
		f = b->brush_faces;
		f->texdef = texdef;
		VectorCopy( vMin, f->planepts[0] );
		VectorCopy( vMin, f->planepts[1] );
		VectorCopy( vMin, f->planepts[2] );
		f->planepts[1][box[i][0]] = vMax[box[i][0]];
		f->planepts[2][box[i][1]] = vMax[box[i][1]];
	}
}

void FacingVectors( entity_t *e, vec3_t forward, vec3_t right, vec3_t up ){
	int angleVal;
	vec3_t angles;

	angleVal = IntForKey( e, "angle" );
	if ( angleVal == -1 ) {           // up
		VectorSet( angles, 270, 0, 0 );
	}
	else if ( angleVal == -2 ) {   // down
		VectorSet( angles, 90, 0, 0 );
	}
	else
	{
		VectorSet( angles, 0, angleVal, 0 );
	}

	AngleVectors( angles, forward, right, up );
}

void Brush_DrawFacingAngle( brush_t *b, entity_t *e ){
	vec3_t forward, right, up;
	vec3_t endpoint, tip1, tip2;
	vec3_t start;
	float dist;

	VectorAdd( e->brushes.onext->mins, e->brushes.onext->maxs, start );
	VectorScale( start, 0.5, start );
	dist = ( b->maxs[0] - start[0] ) * 2.5;

	FacingVectors( e, forward, right, up );
	VectorMA( start, dist, forward, endpoint );

	dist = ( b->maxs[0] - start[0] ) * 0.5;
	VectorMA( endpoint, -dist, forward, tip1 );
	VectorMA( tip1, -dist, up, tip1 );
	VectorMA( tip1, 2 * dist, up, tip2 );

	qglColor4f( 1, 1, 1, 1 );
	qglLineWidth( 4 );
	qglBegin( GL_LINES );
	qglVertex3fv( start );
	qglVertex3fv( endpoint );
	qglVertex3fv( endpoint );
	qglVertex3fv( tip1 );
	qglVertex3fv( endpoint );
	qglVertex3fv( tip2 );
	qglEnd();
	qglLineWidth( 1 );
}

void Brush_FaceDraw( face_t *face, int nGLState ){
	const winding_t *w = face->face_winding;
	if ( w == NULL ) {
		return;
	}
	if ( ( nGLState & DRAW_GL_LIGHTING ) && g_PrefsDlg.m_bGLLighting ) {
		qglNormal3fv( face->plane.normal );
	}
	/*
	   if (mode & DRAW_GL_TEXTURE_2D)
	      qglTexCoordPointer(2, GL_FLOAT, 5, &w->points[3]);
	   qglVertexPointer(3, GL_FLOAT, 5, w->points);

	   if (mode & DRAW_GL_FILL)
	      qglDrawArrays(GL_TRIANGLE_FAN, 0, w->numpoints);
	   else
	      qglDrawArrays(GL_POLYGON, 0, w->numpoints);
	 */

	if ( nGLState & DRAW_GL_FILL ) {
		qglBegin( GL_TRIANGLE_FAN );
	}
	else{
		qglBegin( GL_POLYGON );
	}

	for ( int i = 0 ; i < w->numpoints ; i++ )
	{
		if ( nGLState & DRAW_GL_TEXTURE_2D ) {
			qglTexCoord2fv( &w->points[i][3] );
		}
		qglVertex3fv( w->points[i] );
	}
	qglEnd();
}

#define Q2_SURF_TRANS33   0x00000010
#define Q2_SURF_TRANS66   0x00000020

void Brush_Draw( brush_t *b ){
	face_t          *face;
	int order;
	qtexture_t      *prev = 0;
	winding_t *w;

	int nDrawMode = g_pParentWnd->GetCamWnd()->Camera()->draw_mode;
	int nGLState = g_pParentWnd->GetCamWnd()->Camera()->draw_glstate;

	GLfloat material[4], identity[4];
	VectorSet( identity, 0.8f, 0.8f, 0.8f );
	IShader *pShader;
	qglPushClientAttrib( GL_CLIENT_VERTEX_ARRAY_BIT );
	qglDisableClientState( GL_NORMAL_ARRAY );

	// guarantee the texture will be set first
	bool bTrans;
	float transVal;
	prev = NULL;
	for ( face = b->brush_faces,order = 0 ; face ; face = face->next, order++ )
	{
		w = face->face_winding;
		if ( !w ) {
			continue;       // freed face
		}

		bTrans = ( face->pShader->getFlags() & QER_TRANS );
		transVal = face->pShader->getTrans();
		// try to read the texture def surface flags to get trans
		if ( !bTrans ) {
			if ( face->texdef.flags & Q2_SURF_TRANS33 ) {
				bTrans = true;
				transVal = 0.33f;
			}
			else if ( face->texdef.flags & Q2_SURF_TRANS66 ) {
				bTrans = true;
				transVal = 0.66f;
			}
		}

		if ( bTrans && !( nGLState & DRAW_GL_BLEND ) ) {
			continue;
		}
		if ( !bTrans && ( nGLState & DRAW_GL_BLEND ) ) {
			continue;
		}

		// IMPORTANT NOTE:
		// modifications to the discarding code here should be matched in the selection code
		// see Brush_Ray

		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_CAULK ) {
			if ( strstr( face->texdef.GetName(), "caulk" ) ) {
				continue;
			}
		}

		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_BOTCLIP ) {
			if ( strstr( face->texdef.GetName(), "botclip" ) || strstr( face->texdef.GetName(), "clipmonster" ) ) {
				continue;
			}
		}

		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_CLIP ) {
			if ( strstr( face->texdef.GetName(), "clip" ) ) {
				continue;
			}
		}

		if ( ( nGLState & DRAW_GL_TEXTURE_2D ) && face->d_texture->name[0] == '(' ) {
			prev = NULL;
			qglDisable( GL_TEXTURE_2D );
		}
		else if ( ( nGLState & DRAW_GL_TEXTURE_2D ) && ( nDrawMode == cd_texture || nDrawMode == cd_light ) && face->d_texture != prev ) {
			// set the texture for this face
			prev = face->d_texture;
			qglBindTexture( GL_TEXTURE_2D, face->d_texture->texture_number );
		}

		if ( ( nGLState & DRAW_GL_LIGHTING ) && !g_PrefsDlg.m_bGLLighting ) {
			if ( !b->owner->eclass->fixedsize ) {
				material[3] = transVal;
			}
			else{
				material[3] = 1;
			}
			VectorCopy( face->d_color, material );

			if ( nGLState & DRAW_GL_TEXTURE_2D ) {
				qglColor4f( face->d_shade, face->d_shade, face->d_shade, material[3] );
			}
			else{
				qglColor4fv( material );
			}
		}
		else if ( !b->owner->eclass->fixedsize ) {
			pShader = face->pShader;
			VectorCopy( pShader->getTexture()->color, material );
			material[3] = identity[3] = transVal;

			if ( nGLState & DRAW_GL_TEXTURE_2D ) {
				qglColor4fv( identity );
			}
			else{
				qglColor4fv( material );
			}
		}

		// draw the polygon

		Brush_FaceDraw( face, nGLState );
	}
	qglPopClientAttrib();
}

void Face_Draw( face_t *f ){
	int i;

	if ( f->face_winding == 0 ) {
		return;
	}
	qglBegin( GL_POLYGON );
	for ( i = 0 ; i < f->face_winding->numpoints; i++ )
		qglVertex3fv( f->face_winding->points[i] );
	qglEnd();
}

entity_t *FindEntity( const char *pszKey, const char *pszValue ){
	entity_t *pe;

	pe = entities.next;

	for (; pe != NULL && pe != &entities ; pe = pe->next )
	{
		if ( !strcmp( ValueForKey( pe, pszKey ), pszValue ) ) {
			return pe;
		}
	}

	return NULL;
}

void Brush_DrawXY( brush_t *b, int nViewType ){
	face_t *face;
	int order;
	winding_t *w;
	int i;

	if ( b->patchBrush ) {
		Patch_DrawXY( b->pPatch );
		if ( !g_bPatchShowBounds ) {
			return;
		}
	}

	if ( b->owner->eclass->fixedsize ) {
		if ( g_PrefsDlg.m_bNewLightDraw && ( b->owner->eclass->nShowFlags & ECLASS_LIGHT ) ) {
#if 1 // requires vertex arrays enabled
			DrawLight( b->owner, DRAW_GL_WIRE, ( IsBrushSelected( b ) ) ? g_PrefsDlg.m_nLightRadiuses : 0, nViewType );
#else
			vec3_t vCorners[4];
			float fMid = b->mins[2] + ( b->maxs[2] - b->mins[2] ) / 2;

			vCorners[0][0] = b->mins[0];
			vCorners[0][1] = b->mins[1];
			vCorners[0][2] = fMid;

			vCorners[1][0] = b->mins[0];
			vCorners[1][1] = b->maxs[1];
			vCorners[1][2] = fMid;

			vCorners[2][0] = b->maxs[0];
			vCorners[2][1] = b->maxs[1];
			vCorners[2][2] = fMid;

			vCorners[3][0] = b->maxs[0];
			vCorners[3][1] = b->mins[1];
			vCorners[3][2] = fMid;

			vec3_t vTop, vBottom;

			vTop[0] = b->mins[0] + ( ( b->maxs[0] - b->mins[0] ) / 2 );
			vTop[1] = b->mins[1] + ( ( b->maxs[1] - b->mins[1] ) / 2 );
			vTop[2] = b->maxs[2];

			VectorCopy( vTop, vBottom );
			vBottom[2] = b->mins[2];

			qglBegin( GL_LINES );
			qglVertex3fv( vTop );
			qglVertex3fv( vCorners[0] );
			qglVertex3fv( vTop );
			qglVertex3fv( vCorners[1] );
			qglVertex3fv( vTop );
			qglVertex3fv( vCorners[2] );
			qglVertex3fv( vTop );
			qglVertex3fv( vCorners[3] );
			qglEnd();

			qglBegin( GL_LINES );
			qglVertex3fv( vBottom );
			qglVertex3fv( vCorners[0] );
			qglVertex3fv( vBottom );
			qglVertex3fv( vCorners[1] );
			qglVertex3fv( vBottom );
			qglVertex3fv( vCorners[2] );
			qglVertex3fv( vBottom );
			qglVertex3fv( vCorners[3] );
			qglEnd();

			qglBegin( GL_LINE_LOOP );
			qglVertex3fv( vCorners[0] );
			qglVertex3fv( vCorners[1] );
			qglVertex3fv( vCorners[2] );
			qglVertex3fv( vCorners[3] );
			qglEnd();
#endif
			DrawBrushEntityName( b );
			return;
		}
		else if ( b->owner->model.pRender && !( !IsBrushSelected( b ) && ( g_PrefsDlg.m_nEntityShowState & ENTITY_SELECTED_ONLY ) ) ) {
			qglPushAttrib( GL_CURRENT_BIT ); // save brush colour
			qglColor3fv( b->owner->eclass->color );
			if ( g_PrefsDlg.m_nEntityShowState != ENTITY_BOX ) {
				b->owner->model.pRender->Draw( DRAW_GL_WIRE, DRAW_RF_XY );
			}
			aabb_draw( b->owner->model.pRender->GetAABB(), DRAW_GL_WIRE );
			qglPopAttrib();
			return;
		}
		//}
	}

	for ( face = b->brush_faces,order = 0 ; face ; face = face->next, order++ )
	{
		// moved so check occurs earlier
		w = face->face_winding;
		if ( !w ) {
			continue;
		}
		// only draw polygons facing in a direction we care about
		if ( nViewType == XY ) {
			if ( face->plane.normal[2] <= 0 ) {
				continue;
			}
		}
		else
		{
			if ( nViewType == XZ ) {
				if ( face->plane.normal[1] >= 0 ) { // stop axes being mirrored
					continue;
				}
			}
			else
			{
				if ( face->plane.normal[0] <= 0 ) {
					continue;
				}
			}
		}

		// draw the polygon
		qglBegin( GL_LINE_LOOP );
		for ( i = 0 ; i < w->numpoints ; i++ )
			qglVertex3fv( w->points[i] );
		qglEnd();
	}

	DrawBrushEntityName( b );

}

/*
   ============
   Brush_Move
   ============
 */
void Brush_Move( brush_t *b, const vec3_t move, bool bSnap ){
	int i;
	face_t *f;

	for ( f = b->brush_faces ; f ; f = f->next )
		for ( i = 0 ; i < 3 ; i++ )
			VectorAdd( f->planepts[i], move, f->planepts[i] );

	if ( g_PrefsDlg.m_bTextureLock && !b->owner->eclass->fixedsize ) {
		for ( f = b->brush_faces ; f ; f = f->next )
		{
			vec3_t vTemp;
			VectorCopy( move, vTemp );
			Face_MoveTexture( f, vTemp );
		}
	}

	Brush_Build( b, bSnap,true,false,false ); // don't filter


	if ( b->patchBrush ) {
		//Patch_Move(b->nPatchID, move);
		Patch_Move( b->pPatch, move );
	}


	// PGM - keep the origin vector up to date on fixed size entities.
	if ( b->owner->eclass->fixedsize ) {
		char text[64];
		VectorAdd( b->owner->origin, move, b->owner->origin );
		sprintf( text, "%i %i %i",
				 (int)b->owner->origin[0], (int)b->owner->origin[1], (int)b->owner->origin[2] );
		SetKeyValue( b->owner, "origin", text );
		//VectorAdd(b->maxs, b->mins, b->owner->origin);
		//VectorScale(b->owner->origin, 0.5, b->owner->origin);
	}
}



void Brush_Print( brush_t* b ){
	int nFace = 0;
	for ( face_t* f = b->brush_faces ; f ; f = f->next )
	{
		Sys_Printf( "Face %i\n", nFace++ );
		Sys_Printf( "%f %f %f\n", f->planepts[0][0], f->planepts[0][1], f->planepts[0][2] );
		Sys_Printf( "%f %f %f\n", f->planepts[1][0], f->planepts[1][1], f->planepts[1][2] );
		Sys_Printf( "%f %f %f\n", f->planepts[2][0], f->planepts[2][1], f->planepts[2][2] );
	}
}



/*
   =============
   Brush_MakeSided

   Makes the current brushhave the given number of 2d sides and turns it into a cone
   =============
 */
void Brush_MakeSidedCone( int sides ){
	int i;
	vec3_t mins, maxs;
	brush_t *b;
	texdef_t    *texdef;
	face_t  *f;
	vec3_t mid;
	float width;
	float sv, cv;

	if ( sides < 3 || sides > 32 ) {
		Sys_Status( "Bad sides number", 0 );
		return;
	}

	if ( !QE_SingleBrush() ) {
		Sys_Status( "Must have a single brush selected", 0 );
		return;
	}

	b = selected_brushes.next;
	VectorCopy( b->mins, mins );
	VectorCopy( b->maxs, maxs );
	texdef = &g_qeglobals.d_texturewin.texdef;

	Brush_Free( b );

	// find center of brush
	width = 8;
	for ( i = 0 ; i < 2 ; i++ )
	{
		mid[i] = ( maxs[i] + mins[i] ) * 0.5;
		if ( maxs[i] - mins[i] > width ) {
			width = maxs[i] - mins[i];
		}
	}
	width /= 2;

	b = Brush_Alloc();

	// create bottom face
	f = Face_Alloc();
	f->texdef = *texdef;
	f->next = b->brush_faces;
	b->brush_faces = f;

	f->planepts[0][0] = mins[0]; f->planepts[0][1] = mins[1]; f->planepts[0][2] = mins[2];
	f->planepts[1][0] = maxs[0]; f->planepts[1][1] = mins[1]; f->planepts[1][2] = mins[2];
	f->planepts[2][0] = maxs[0]; f->planepts[2][1] = maxs[1]; f->planepts[2][2] = mins[2];

	for ( i = 0 ; i < sides ; i++ )
	{
		f = Face_Alloc();
		f->texdef = *texdef;
		f->next = b->brush_faces;
		b->brush_faces = f;

		sv = sin( i * 3.14159265 * 2 / sides );
		cv = cos( i * 3.14159265 * 2 / sides );


		f->planepts[0][0] = floor( mid[0] + width * cv + 0.5 );
		f->planepts[0][1] = floor( mid[1] + width * sv + 0.5 );
		f->planepts[0][2] = mins[2];

		f->planepts[1][0] = mid[0];
		f->planepts[1][1] = mid[1];
		f->planepts[1][2] = maxs[2];

		f->planepts[2][0] = floor( f->planepts[0][0] - width * sv + 0.5 );
		f->planepts[2][1] = floor( f->planepts[0][1] + width * cv + 0.5 );
		f->planepts[2][2] = maxs[2];

	}

	Brush_AddToList( b, &selected_brushes );

	Entity_LinkBrush( world_entity, b );

	Brush_Build( b );

	Sys_UpdateWindows( W_ALL );
}

/*
   =============
   Brush_MakeSided

   Makes the current brushhave the given number of 2d sides and turns it into a sphere
   =============

 */
void Brush_MakeSidedSphere( int sides ){
	int i,j;
	vec3_t mins, maxs;
	brush_t *b;
	texdef_t    *texdef;
	face_t  *f;
	vec3_t mid;

	if ( sides < 4 || sides > 32 ) {
		Sys_Status( "Bad sides number", 0 );
		return;
	}

	if ( !QE_SingleBrush() ) {
		Sys_Status( "Must have a single brush selected", 0 );
		return;
	}

	b = selected_brushes.next;
	VectorCopy( b->mins, mins );
	VectorCopy( b->maxs, maxs );
	texdef = &g_qeglobals.d_texturewin.texdef;

	Brush_Free( b );

	// find center of brush
	float radius = 8;
	for ( i = 0 ; i < 2 ; i++ )
	{
		mid[i] = ( maxs[i] + mins[i] ) * 0.5;
		if ( maxs[i] - mins[i] > radius ) {
			radius = maxs[i] - mins[i];
		}
	}
	radius /= 2;

	b = Brush_Alloc();

	float dt = float(2 * Q_PI / sides);
	float dp = float(Q_PI / sides);
	float t,p;
	for ( i = 0; i <= sides - 1; i++ )
	{
		for ( j = 0; j <= sides - 2; j++ )
		{
			t = i * dt;
			p = float(j * dp - Q_PI / 2);

			f = Face_Alloc();
			f->texdef = *texdef;
			f->next = b->brush_faces;
			b->brush_faces = f;

			VectorPolar( f->planepts[0], radius, t, p );
			VectorPolar( f->planepts[1], radius, t, p + dp );
			VectorPolar( f->planepts[2], radius, t + dt, p + dp );

			for ( int k = 0; k < 3; k++ )
				VectorAdd( f->planepts[k], mid, f->planepts[k] );
		}
	}

	p = float( ( sides - 1 ) * dp - Q_PI / 2);
	for ( i = 0; i <= sides - 1; i++ )
	{
		t = i * dt;

		f = Face_Alloc();
		f->texdef = *texdef;
		f->next = b->brush_faces;
		b->brush_faces = f;

		VectorPolar( f->planepts[0], radius, t, p );
		VectorPolar( f->planepts[1], radius, t + dt, p + dp );
		VectorPolar( f->planepts[2], radius, t + dt, p );

		for ( int k = 0; k < 3; k++ )
			VectorAdd( f->planepts[k], mid, f->planepts[k] );
	}

	Brush_AddToList( b, &selected_brushes );

	Entity_LinkBrush( world_entity, b );

	Brush_Build( b );

	Sys_UpdateWindows( W_ALL );
}

void Face_FitTexture( face_t * face, int nHeight, int nWidth ){
	winding_t *w;
	vec3_t mins,maxs;
	int i;
	float width, height, temp;
	float rot_width, rot_height;
	float cosv,sinv,ang;
	float min_t, min_s, max_t, max_s;
	float s,t;
	vec3_t vecs[2];
	vec3_t coords[4];
	texdef_t  *td;

	if ( nHeight < 1 ) {
		nHeight = 1;
	}
	if ( nWidth < 1 ) {
		nWidth = 1;
	}

	ClearBounds( mins, maxs );

	w = face->face_winding;
	if ( !w ) {
		return;
	}
	for ( i = 0 ; i < w->numpoints ; i++ )
	{
		AddPointToBounds( w->points[i], mins, maxs );
	}

	if ( g_qeglobals.m_bBrushPrimitMode ) {
		Face_FitTexture_BrushPrimit( face, mins, maxs, nHeight, nWidth );
	}
	else
	{

		td = &face->texdef;
		//
		// get the current angle
		//
		ang = td->rotate / 180 * Q_PI;
		sinv = sin( ang );
		cosv = cos( ang );

		// get natural texture axis
		TextureAxisFromPlane( &face->plane, vecs[0], vecs[1] );

		min_s = DotProduct( mins, vecs[0] );
		min_t = DotProduct( mins, vecs[1] );
		max_s = DotProduct( maxs, vecs[0] );
		max_t = DotProduct( maxs, vecs[1] );
		width = max_s - min_s;
		height = max_t - min_t;
		coords[0][0] = min_s;
		coords[0][1] = min_t;
		coords[1][0] = max_s;
		coords[1][1] = min_t;
		coords[2][0] = min_s;
		coords[2][1] = max_t;
		coords[3][0] = max_s;
		coords[3][1] = max_t;
		min_s = min_t = 99999;
		max_s = max_t = -99999;
		for ( i = 0; i < 4; i++ )
		{
			s = cosv * coords[i][0] - sinv * coords[i][1];
			t = sinv * coords[i][0] + cosv * coords[i][1];
			if ( i & 1 ) {
				if ( s > max_s ) {
					max_s = s;
				}
			}
			else
			{
				if ( s < min_s ) {
					min_s = s;
				}
				if ( i < 2 ) {
					if ( t < min_t ) {
						min_t = t;
					}
				}
				else
				{
					if ( t > max_t ) {
						max_t = t;
					}
				}
			}
		}
		rot_width =  ( max_s - min_s );
		rot_height = ( max_t - min_t );
		td->scale[0] = -( rot_width / ( (float)( face->d_texture->width * nWidth ) ) );
		td->scale[1] = -( rot_height / ( (float)( face->d_texture->height * nHeight ) ) );

		td->shift[0] = min_s / td->scale[0];
		temp = (int)( td->shift[0] / ( face->d_texture->width * nWidth ) );
		temp = ( temp + 1 ) * face->d_texture->width * nWidth;
		td->shift[0] = (int)( temp - td->shift[0] ) % ( face->d_texture->width * nWidth );

		td->shift[1] = min_t / td->scale[1];
		temp = (int)( td->shift[1] / ( face->d_texture->height * nHeight ) );
		temp = ( temp + 1 ) * ( face->d_texture->height * nHeight );
		td->shift[1] = (int)( temp - td->shift[1] ) % ( face->d_texture->height * nHeight );

		td->shift[1] = min_t / td->scale[1];
		temp = (int)( td->shift[1] / ( face->d_texture->height * nHeight ) );
		temp = ( temp + 1 ) * ( face->d_texture->height * nHeight );
		td->shift[1] = (int)( temp - td->shift[1] ) % ( face->d_texture->height * nHeight );

	}
}

void Brush_FitTexture( brush_t *b, int nHeight, int nWidth ){
	face_t *face;

	for ( face = b->brush_faces ; face ; face = face->next )
	{
		Face_FitTexture( face, nHeight, nWidth );
	}
}

void aabb_draw( const aabb_t *aabb, int mode ){
	vec3_t normals[6] = { { 1, 0, 0}, { 0, 1, 0 }, { 0, 0, 1 }, {-1, 0, 0}, { 0,-1, 0 }, { 0, 0,-1 } };
	vec3_t points[8];
	vec3_t vMin, vMax;
	VectorSubtract( aabb->origin, aabb->extents, vMin );
	VectorAdd( aabb->origin, aabb->extents, vMax );
	VectorSet( points[0], vMin[0], vMax[1], vMax[2] );
	VectorSet( points[1], vMax[0], vMax[1], vMax[2] );
	VectorSet( points[2], vMax[0], vMin[1], vMax[2] );
	VectorSet( points[3], vMin[0], vMin[1], vMax[2] );
	VectorSet( points[4], vMin[0], vMax[1], vMin[2] );
	VectorSet( points[5], vMax[0], vMax[1], vMin[2] );
	VectorSet( points[6], vMax[0], vMin[1], vMin[2] );
	VectorSet( points[7], vMin[0], vMin[1], vMin[2] );

	qglBegin( GL_QUADS );

	qglNormal3fv( normals[0] );
	qglVertex3fv( points[2] );
	qglVertex3fv( points[1] );
	qglVertex3fv( points[5] );
	qglVertex3fv( points[6] );

	qglNormal3fv( normals[1] );
	qglVertex3fv( points[1] );
	qglVertex3fv( points[0] );
	qglVertex3fv( points[4] );
	qglVertex3fv( points[5] );

	qglNormal3fv( normals[2] );
	qglVertex3fv( points[0] );
	qglVertex3fv( points[1] );
	qglVertex3fv( points[2] );
	qglVertex3fv( points[3] );

	qglNormal3fv( normals[3] );
	qglVertex3fv( points[3] );
	qglVertex3fv( points[7] );
	qglVertex3fv( points[4] );
	qglVertex3fv( points[0] );

	qglNormal3fv( normals[4] );
	qglVertex3fv( points[3] );
	qglVertex3fv( points[2] );
	qglVertex3fv( points[6] );
	qglVertex3fv( points[7] );

	qglNormal3fv( normals[5] );
	qglVertex3fv( points[7] );
	qglVertex3fv( points[6] );
	qglVertex3fv( points[5] );
	qglVertex3fv( points[4] );

	qglEnd();

/*


   vec3_t Coords[8];

    vec3_t vMin, vMax;
   VectorSubtract(aabb->origin, aabb->extents, vMin);
   VectorAdd(aabb->origin, aabb->extents, vMax);
   VectorSet(Coords[0], vMin[0], vMax[1], vMax[2]);
   VectorSet(Coords[1], vMax[0], vMax[1], vMax[2]);
   VectorSet(Coords[2], vMax[0], vMin[1], vMax[2]);
   VectorSet(Coords[3], vMin[0], vMin[1], vMax[2]);
   VectorSet(Coords[4], vMin[0], vMax[1], vMin[2]);
   VectorSet(Coords[5], vMax[0], vMax[1], vMin[2]);
   VectorSet(Coords[6], vMax[0], vMin[1], vMin[2]);
   VectorSet(Coords[7], vMin[0], vMin[1], vMin[2]);

    vec3_t Normals[8] = { {-1, 0, 0 },
                                            { 0, 0, 0 },
                                            { 0, 0, 0 },
                                            { 0, 0, 1 },
                                            { 0, 0,-1 },
                                            { 0, 1, 0 },
                                            { 1, 0, 0 },
                                            { 0,-1, 0 } };

    unsigned short Indices[24] = { 2, 1, 5, 6,
                                                                 1, 0, 4, 5,
                                                                 0, 1, 2, 3,
                                                                 3, 7, 4, 0,
                                                                 3, 2, 6, 7,
                                                                 7, 6, 5, 4 };

   qglVertexPointer(3, GL_FLOAT, 0, Coords);         // filling the arrays
   qglNormalPointer(GL_FLOAT, 0, Normals);

   //glLockArraysEXT(0, count);                // extension GL_EXT_compiled_vertex_array

   qglDrawElements(GL_QUADS, 24, GL_UNSIGNED_SHORT, Indices);

   //glUnlockArraysEXT;                        // extension GL_EXT_compiled_vertex_array
 */
}

qboolean IsBrushSelected( brush_t* bSel ){
	for ( brush_t* b = selected_brushes.next ; b != NULL && b != &selected_brushes; b = b->next )
	{
		if ( b == bSel ) {
			return true;
		}
	}
	return false;
}
