/*
   PrtView plugin for GtkRadiant
   Copyright (C) 2001 Geoffrey Dewan, Loki software and qeradiant.com

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#ifndef __APPLE__
#include <search.h>
#endif
#include <stdio.h>

#define LINE_BUF 1000

CPortals portals;
CPortalsRender render;

int compare( const void *arg1, const void *arg2 ){

	if ( portals.portal[*( (int *)arg1 )].dist > portals.portal[*( (int *)arg2 )].dist ) {
		return -1;
	}
	else if ( portals.portal[*( (int *)arg1 )].dist < portals.portal[*( (int *)arg2 )].dist ) {
		return 1;
	}

	return 0;
}


CBspPortal::CBspPortal(){
	memset( this, 0, sizeof( CBspPortal ) );
}

CBspPortal::~CBspPortal(){
	delete[] point;
	delete[] inner_point;
}

qboolean CBspPortal::Build( char *def ){
	char *c = def;
	unsigned int n;
	int dummy1, dummy2;
	int i;

	if ( portals.hint_flags ) {
		int res_cnt = sscanf( def, "%u %d %d %d", &point_count, &dummy1, &dummy2, (int *)&hint );
		if ( res_cnt < 4 ) {
			return FALSE;
		}
	}
	else
	{
		sscanf( def, "%u", &point_count );
		hint = FALSE;
	}

	if ( point_count < 3 ) {
		return FALSE;
	}

	point = new CBspPoint[point_count];
	inner_point = new CBspPoint[point_count];

	for ( n = 0; n < point_count; n++ )
	{
		for (; *c != 0 && *c != '('; c++ ) ;

		if ( *c == 0 ) {
			return FALSE;
		}

		c++;

		sscanf( c, "%f %f %f", point[n].p, point[n].p + 1, point[n].p + 2 );

		center.p[0] += point[n].p[0];
		center.p[1] += point[n].p[1];
		center.p[2] += point[n].p[2];

		if ( n == 0 ) {
			for ( i = 0; i < 3; i++ )
			{
				min[i] = point[n].p[i];
				max[i] = point[n].p[i];
			}
		}
		else
		{
			for ( i = 0; i < 3; i++ )
			{
				if ( min[i] > point[n].p[i] ) {
					min[i] = point[n].p[i];
				}
				if ( max[i] < point[n].p[i] ) {
					max[i] = point[n].p[i];
				}
			}
		}
	}

	center.p[0] /= (float)point_count;
	center.p[1] /= (float)point_count;
	center.p[2] /= (float)point_count;

	for ( n = 0; n < point_count; n++ )
	{
		inner_point[n].p[0] = ( 0.01f * center.p[0] ) + ( 0.99f * point[n].p[0] );
		inner_point[n].p[1] = ( 0.01f * center.p[1] ) + ( 0.99f * point[n].p[1] );
		inner_point[n].p[2] = ( 0.01f * center.p[2] ) + ( 0.99f * point[n].p[2] );
	}

	fp_color_random[0] = (float)( rand() & 0xff ) / 255.0f;
	fp_color_random[1] = (float)( rand() & 0xff ) / 255.0f;
	fp_color_random[2] = (float)( rand() & 0xff ) / 255.0f;
	fp_color_random[3] = 1.0f;

	return TRUE;
}

CPortals::CPortals(){
	memset( this, 0, sizeof( CPortals ) );
}

CPortals::~CPortals(){
	Purge();
}

void CPortals::Purge(){
	delete[] portal;
	delete[] portal_sort;
	portal = NULL;
	portal_sort = NULL;
	portal_count = 0;

	/*
	   delete[] node;
	   node = NULL;
	   node_count = 0;
	 */
}

void CPortals::Load(){
	char buf[LINE_BUF + 1];

	memset( buf, 0, LINE_BUF + 1 );

	Purge();

	Sys_Printf( MSG_PREFIX "Loading portal file %s.\n", fn );

	FILE *in;

	in = fopen( fn, "rt" );

	if ( in == NULL ) {
		Sys_FPrintf( SYS_ERR, "ERROR - could not open file.\n" );

		return;
	}

	if ( !fgets( buf, LINE_BUF, in ) ) {
		fclose( in );

		Sys_FPrintf( SYS_ERR, "ERROR - File ended prematurely.\n" );

		return;
	}

	if ( strncmp( "PRT1", buf, 4 ) != 0 ) {
		fclose( in );

		Sys_FPrintf( SYS_ERR, "ERROR - File header indicates wrong file type (should be \"PRT1\").\n" );

		return;
	}

	if ( !fgets( buf, LINE_BUF, in ) ) {
		fclose( in );

		Sys_FPrintf( SYS_ERR, "ERROR - File ended prematurely.\n" );

		return;
	}

	sscanf( buf, "%u", &node_count );
/*
    if(node_count > 0xFFFF)
    {
        fclose(in);

        node_count = 0;

        Sys_FPrintf( SYS_ERR, "ERROR - Extreme number of nodes, aborting.\n");

        return;
    }
 */

	if ( !fgets( buf, LINE_BUF, in ) ) {
		fclose( in );

		node_count = 0;

		Sys_FPrintf( SYS_ERR, "ERROR - File ended prematurely.\n" );

		return;
	}

	sscanf( buf, "%u", &portal_count );

	if ( portal_count > 0xFFFF ) {
		fclose( in );

		portal_count = 0;
		node_count = 0;

		Sys_FPrintf( SYS_ERR, "ERROR - Extreme number of portals, aborting.\n" );

		return;
	}

	if ( portal_count <= 0 ) {
		fclose( in );

		portal_count = 0;
		node_count = 0;

		Sys_FPrintf( SYS_ERR, "ERROR - number of portals equals 0, aborting.\n" );

		return;
	}

//	node = new CBspNode[node_count];
	portal = new CBspPortal[portal_count];
	portal_sort = new int[portal_count];

	unsigned int n;
	qboolean first = TRUE;
	unsigned test_vals_1, test_vals_2;

	hint_flags = FALSE;

	for ( n = 0; n < portal_count; )
	{
		if ( !fgets( buf, LINE_BUF, in ) ) {
			fclose( in );

			Purge();

			Sys_FPrintf( SYS_ERR, "ERROR - Could not find information for portal number %d of %d.\n", n + 1, portal_count );

			return;
		}

		if ( !portal[n].Build( buf ) ) {
			if ( first && sscanf( buf, "%d %d", &test_vals_1, &test_vals_2 ) == 1 ) { // skip additional counts of later data, not needed
				// We can count on hint flags being in the file
				hint_flags = TRUE;
				continue;
			}

			first = FALSE;

			fclose( in );

			Purge();

			Sys_FPrintf( SYS_ERR, "ERROR - Information for portal number %d of %d is not formatted correctly.\n", n + 1, portal_count );

			return;
		}

		n++;
	}

	fclose( in );

	Sys_Printf( "  %u portals read in.\n", node_count, portal_count );
}

void CPortals::FixColors(){
	fp_color_2d[0] = (float)GetRValue( color_2d ) / 255.0f;
	fp_color_2d[1] = (float)GetGValue( color_2d ) / 255.0f;
	fp_color_2d[2] = (float)GetBValue( color_2d ) / 255.0f;
	fp_color_2d[3] = 1.0f;

	fp_color_3d[0] = (float)GetRValue( color_3d ) / 255.0f;
	fp_color_3d[1] = (float)GetGValue( color_3d ) / 255.0f;
	fp_color_3d[2] = (float)GetBValue( color_3d ) / 255.0f;
	fp_color_3d[3] = 1.0f;

	fp_color_fog[0] = 0.0f; //(float)GetRValue(color_fog) / 255.0f;
	fp_color_fog[1] = 0.0f; //(float)GetGValue(color_fog) / 255.0f;
	fp_color_fog[2] = 0.0f; //(float)GetBValue(color_fog) / 255.0f;
	fp_color_fog[3] = 1.0f;
}

CPortalsRender::CPortalsRender(){
	refCount = 1;
}

CPortalsRender::~CPortalsRender(){
}

void CPortalsRender::Register(){
	g_QglTable.m_pfnHookGL2DWindow( this );
	g_QglTable.m_pfnHookGL3DWindow( this );
}

void CPortalsRender::Draw2D( VIEWTYPE vt ){
	if ( !portals.show_2d || portals.portal_count < 1 ) {
		return;
	}

	g_QglTable.m_pfn_qglPushAttrib( GL_ALL_ATTRIB_BITS );

	if ( portals.aa_2d ) {
		g_QglTable.m_pfn_qglEnable( GL_BLEND );
		g_QglTable.m_pfn_qglEnable( GL_LINE_SMOOTH );
	}
	else
	{
		g_QglTable.m_pfn_qglDisable( GL_BLEND );
		g_QglTable.m_pfn_qglEnable( GL_LINE_SMOOTH );
	}

	switch ( vt )
	{
	case XY:
		break;
	case XZ:
		g_QglTable.m_pfn_qglRotatef( 270.0f, 1.0f, 0.0f, 0.0f );
		break;
	case YZ:
		g_QglTable.m_pfn_qglRotatef( 270.0f, 1.0f, 0.0f, 0.0f );
		g_QglTable.m_pfn_qglRotatef( 270.0f, 0.0f, 0.0f, 1.0f );
		break;
	}

	g_QglTable.m_pfn_qglLineWidth( portals.width_2d * 0.5f );

	g_QglTable.m_pfn_qglColor4fv( portals.fp_color_2d );

	unsigned int n, p;

	for ( n = 0; n < portals.portal_count; n++ )
	{
		g_QglTable.m_pfn_qglBegin( GL_LINE_LOOP );

		for ( p = 0; p < portals.portal[n].point_count; p++ )
			g_QglTable.m_pfn_qglVertex3fv( portals.portal[n].point[p].p );

		g_QglTable.m_pfn_qglEnd();
	}

	g_QglTable.m_pfn_qglPopAttrib();
}

/*
 * Transform a point (column vector) by a 4x4 matrix.  I.e.  out = m * in
 * Input:  m - the 4x4 matrix
 *         in - the 4x1 vector
 * Output:  out - the resulting 4x1 vector.
 */
static void transform_point( GLdouble out[4], const GLdouble m[16],
							 const GLdouble in[4] ){
#define M( row,col )  m[col * 4 + row]
	out[0] = M( 0,0 ) * in[0] + M( 0,1 ) * in[1] + M( 0,2 ) * in[2] + M( 0,3 ) * in[3];
	out[1] = M( 1,0 ) * in[0] + M( 1,1 ) * in[1] + M( 1,2 ) * in[2] + M( 1,3 ) * in[3];
	out[2] = M( 2,0 ) * in[0] + M( 2,1 ) * in[1] + M( 2,2 ) * in[2] + M( 2,3 ) * in[3];
	out[3] = M( 3,0 ) * in[0] + M( 3,1 ) * in[1] + M( 3,2 ) * in[2] + M( 3,3 ) * in[3];
#undef M
}

#include <math.h>


/*
 * Perform a 4x4 matrix multiplication  (product = a x b).
 * Input:  a, b - matrices to multiply
 * Output:  product - product of a and b
 */
static void matmul( GLdouble *product, const GLdouble *a, const GLdouble *b ){
	/* This matmul was contributed by Thomas Malik */
	GLdouble temp[16];
	GLint i;

#define A( row,col )  a[( col << 2 ) + row]
#define B( row,col )  b[( col << 2 ) + row]
#define T( row,col )  temp[( col << 2 ) + row]

	/* i-te Zeile */
	for ( i = 0; i < 4; i++ )
	{
		T( i, 0 ) = A( i, 0 ) * B( 0, 0 ) + A( i, 1 ) * B( 1, 0 ) + A( i, 2 ) * B( 2, 0 ) + A( i, 3 ) * B( 3, 0 );
		T( i, 1 ) = A( i, 0 ) * B( 0, 1 ) + A( i, 1 ) * B( 1, 1 ) + A( i, 2 ) * B( 2, 1 ) + A( i, 3 ) * B( 3, 1 );
		T( i, 2 ) = A( i, 0 ) * B( 0, 2 ) + A( i, 1 ) * B( 1, 2 ) + A( i, 2 ) * B( 2, 2 ) + A( i, 3 ) * B( 3, 2 );
		T( i, 3 ) = A( i, 0 ) * B( 0, 3 ) + A( i, 1 ) * B( 1, 3 ) + A( i, 2 ) * B( 2, 3 ) + A( i, 3 ) * B( 3, 3 );
	}

#undef A
#undef B
#undef T
	memcpy( product, temp, 16 * sizeof( GLdouble ) );
}



/*
 * Compute inverse of 4x4 transformation matrix.
 * Code contributed by Jacques Leroy jle@star.be
 * Return GL_TRUE for success, GL_FALSE for failure (singular matrix)
 */
static GLboolean invert_matrix( const GLdouble *m, GLdouble *out ){
/* NB. OpenGL Matrices are COLUMN major. */
#define SWAP_ROWS( a, b ) { GLdouble *_tmp = a; ( a ) = ( b ); ( b ) = _tmp; }
#define MAT( m,r,c ) ( m )[( c ) * 4 + ( r )]

	GLdouble wtmp[4][8];
	GLdouble m0, m1, m2, m3, s;
	GLdouble *r0, *r1, *r2, *r3;

	r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

	r0[0] = MAT( m,0,0 ), r0[1] = MAT( m,0,1 ),
	r0[2] = MAT( m,0,2 ), r0[3] = MAT( m,0,3 ),
	r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,

	r1[0] = MAT( m,1,0 ), r1[1] = MAT( m,1,1 ),
	r1[2] = MAT( m,1,2 ), r1[3] = MAT( m,1,3 ),
	r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,

	r2[0] = MAT( m,2,0 ), r2[1] = MAT( m,2,1 ),
	r2[2] = MAT( m,2,2 ), r2[3] = MAT( m,2,3 ),
	r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,

	r3[0] = MAT( m,3,0 ), r3[1] = MAT( m,3,1 ),
	r3[2] = MAT( m,3,2 ), r3[3] = MAT( m,3,3 ),
	r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

	/* choose pivot - or die */
	if ( fabs( r3[0] ) > fabs( r2[0] ) ) {
		SWAP_ROWS( r3, r2 );
	}
	if ( fabs( r2[0] ) > fabs( r1[0] ) ) {
		SWAP_ROWS( r2, r1 );
	}
	if ( fabs( r1[0] ) > fabs( r0[0] ) ) {
		SWAP_ROWS( r1, r0 );
	}
	if ( 0.0 == r0[0] ) {
		return GL_FALSE;
	}

	/* eliminate first variable     */
	m1 = r1[0] / r0[0]; m2 = r2[0] / r0[0]; m3 = r3[0] / r0[0];
	s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
	s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
	s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
	s = r0[4];
	if ( s != 0.0 ) {
		r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s;
	}
	s = r0[5];
	if ( s != 0.0 ) {
		r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s;
	}
	s = r0[6];
	if ( s != 0.0 ) {
		r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s;
	}
	s = r0[7];
	if ( s != 0.0 ) {
		r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s;
	}

	/* choose pivot - or die */
	if ( fabs( r3[1] ) > fabs( r2[1] ) ) {
		SWAP_ROWS( r3, r2 );
	}
	if ( fabs( r2[1] ) > fabs( r1[1] ) ) {
		SWAP_ROWS( r2, r1 );
	}
	if ( 0.0 == r1[1] ) {
		return GL_FALSE;
	}

	/* eliminate second variable */
	m2 = r2[1] / r1[1]; m3 = r3[1] / r1[1];
	r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
	r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
	s = r1[4]; if ( 0.0 != s ) {
		r2[4] -= m2 * s; r3[4] -= m3 * s;
	}
	s = r1[5]; if ( 0.0 != s ) {
		r2[5] -= m2 * s; r3[5] -= m3 * s;
	}
	s = r1[6]; if ( 0.0 != s ) {
		r2[6] -= m2 * s; r3[6] -= m3 * s;
	}
	s = r1[7]; if ( 0.0 != s ) {
		r2[7] -= m2 * s; r3[7] -= m3 * s;
	}

	/* choose pivot - or die */
	if ( fabs( r3[2] ) > fabs( r2[2] ) ) {
		SWAP_ROWS( r3, r2 );
	}
	if ( 0.0 == r2[2] ) {
		return GL_FALSE;
	}

	/* eliminate third variable */
	m3 = r3[2] / r2[2];
	r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
	r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
	r3[7] -= m3 * r2[7];

	/* last check */
	if ( 0.0 == r3[3] ) {
		return GL_FALSE;
	}

	s = 1.0 / r3[3];         /* now back substitute row 3 */
	r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

	m2 = r2[3];              /* now back substitute row 2 */
	s  = 1.0 / r2[2];
	r2[4] = s * ( r2[4] - r3[4] * m2 ), r2[5] = s * ( r2[5] - r3[5] * m2 ),
	r2[6] = s * ( r2[6] - r3[6] * m2 ), r2[7] = s * ( r2[7] - r3[7] * m2 );
	m1 = r1[3];
	r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
	r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
	m0 = r0[3];
	r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
	r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

	m1 = r1[2];              /* now back substitute row 1 */
	s  = 1.0 / r1[1];
	r1[4] = s * ( r1[4] - r2[4] * m1 ), r1[5] = s * ( r1[5] - r2[5] * m1 ),
	r1[6] = s * ( r1[6] - r2[6] * m1 ), r1[7] = s * ( r1[7] - r2[7] * m1 );
	m0 = r0[2];
	r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
	r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

	m0 = r0[1];              /* now back substitute row 0 */
	s  = 1.0 / r0[0];
	r0[4] = s * ( r0[4] - r1[4] * m0 ), r0[5] = s * ( r0[5] - r1[5] * m0 ),
	r0[6] = s * ( r0[6] - r1[6] * m0 ), r0[7] = s * ( r0[7] - r1[7] * m0 );

	MAT( out,0,0 ) = r0[4]; MAT( out,0,1 ) = r0[5],
	MAT( out,0,2 ) = r0[6]; MAT( out,0,3 ) = r0[7],
	MAT( out,1,0 ) = r1[4]; MAT( out,1,1 ) = r1[5],
	MAT( out,1,2 ) = r1[6]; MAT( out,1,3 ) = r1[7],
	MAT( out,2,0 ) = r2[4]; MAT( out,2,1 ) = r2[5],
	MAT( out,2,2 ) = r2[6]; MAT( out,2,3 ) = r2[7],
	MAT( out,3,0 ) = r3[4]; MAT( out,3,1 ) = r3[5],
	MAT( out,3,2 ) = r3[6]; MAT( out,3,3 ) = r3[7];

	return GL_TRUE;

#undef MAT
#undef SWAP_ROWS
}

GLint UnProject( GLdouble winx,GLdouble winy,GLdouble winz,
				 const GLdouble model[16],const GLdouble proj[16],
				 const GLint viewport[4],
				 GLdouble *objx,GLdouble *objy,GLdouble *objz ){
	/* matrice de transformation */
	GLdouble m[16], A[16];
	GLdouble in[4],out[4];

	/* transformation coordonnees normalisees entre -1 et 1 */
	in[0] = ( winx - viewport[0] ) * 2 / viewport[2] - 1.0;
	in[1] = ( winy - viewport[1] ) * 2 / viewport[3] - 1.0;
	in[2] = 2 * winz - 1.0;
	in[3] = 1.0;

	/* calcul transformation inverse */
	matmul( A,proj,model );
	invert_matrix( A,m );

	/* d'ou les coordonnees objets */
	transform_point( out,m,in );
	if ( out[3] == 0.0 ) {
		return GL_FALSE;
	}
	*objx = out[0] / out[3];
	*objy = out[1] / out[3];
	*objz = out[2] / out[3];
	return GL_TRUE;
}

void CPortalsRender::Draw3D(){
	if ( !portals.show_3d || portals.portal_count < 1 ) {
		return;
	}

	g_QglTable.m_pfn_qglPushAttrib( GL_ALL_ATTRIB_BITS );

	double cam[3];
	double proj_m[16];
	double model_m[16];
	float min_check[3];
	float max_check[3];
	float trans = ( 100.0f - portals.trans_3d ) / 100.0f;
	int view[4];

	g_QglTable.m_pfn_qglGetDoublev( GL_PROJECTION_MATRIX, proj_m );
	g_QglTable.m_pfn_qglGetDoublev( GL_MODELVIEW_MATRIX, model_m );
	g_QglTable.m_pfn_qglGetIntegerv( GL_VIEWPORT, view );

	UnProject( 0.5 * (double)view[2], 0.5 * (double)view[3], 0.0, model_m, proj_m, view, cam, cam + 1, cam + 2 );

	min_check[0] = (float)cam[0] + ( portals.clip_range * 64.0f );
	min_check[1] = (float)cam[1] + ( portals.clip_range * 64.0f );
	min_check[2] = (float)cam[2] + ( portals.clip_range * 64.0f );
	max_check[0] = (float)cam[0] - ( portals.clip_range * 64.0f );
	max_check[1] = (float)cam[1] - ( portals.clip_range * 64.0f );
	max_check[2] = (float)cam[2] - ( portals.clip_range * 64.0f );

	g_QglTable.m_pfn_qglHint( GL_FOG_HINT, GL_NICEST );

	g_QglTable.m_pfn_qglDisable( GL_CULL_FACE );

	g_QglTable.m_pfn_qglDisable( GL_LINE_SMOOTH );
	g_QglTable.m_pfn_qglDisable( GL_POLYGON_SMOOTH );

	g_QglTable.m_pfn_qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	g_QglTable.m_pfn_qglShadeModel( GL_SMOOTH );

	g_QglTable.m_pfn_qglEnable( GL_BLEND );
	g_QglTable.m_pfn_qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	g_QglTable.m_pfn_qglEnable( GL_POLYGON_SMOOTH );

	if ( portals.aa_3d ) {
		g_QglTable.m_pfn_qglEnable( GL_LINE_SMOOTH );
	}
	else{
		g_QglTable.m_pfn_qglDisable( GL_LINE_SMOOTH );
	}

	if ( portals.fog ) {
		g_QglTable.m_pfn_qglEnable( GL_FOG );

		g_QglTable.m_pfn_qglFogi( GL_FOG_MODE, GL_EXP );
		g_QglTable.m_pfn_qglFogf( GL_FOG_DENSITY, 0.001f );
		g_QglTable.m_pfn_qglFogf( GL_FOG_START, 10.0f );
		g_QglTable.m_pfn_qglFogf( GL_FOG_END, 10000.0f );
		g_QglTable.m_pfn_qglFogi( GL_FOG_INDEX, 0 );
		g_QglTable.m_pfn_qglFogfv( GL_FOG_COLOR, portals.fp_color_fog );
	}
	else
	{
		g_QglTable.m_pfn_qglDisable( GL_FOG );
	}

	switch ( portals.zbuffer )
	{
	case 1:
		g_QglTable.m_pfn_qglEnable( GL_DEPTH_TEST );
		g_QglTable.m_pfn_qglDepthMask( GL_FALSE );
		break;
	case 2:
		g_QglTable.m_pfn_qglDisable( GL_DEPTH_TEST );
		break;
	default:
		g_QglTable.m_pfn_qglEnable( GL_DEPTH_TEST );
		g_QglTable.m_pfn_qglDepthMask( GL_TRUE );
	}

	g_QglTable.m_pfn_qglLineWidth( portals.width_3d * 0.5f );

	unsigned int n, p;

	if ( portals.polygons ) {
		if ( portals.zbuffer != 0 ) {
			float d;

			for ( n = 0; n < portals.portal_count; n++ )
			{
				d = (float)cam[0] - portals.portal[n].center.p[0];
				portals.portal[n].dist = d * d;

				d = (float)cam[1] - portals.portal[n].center.p[1];
				portals.portal[n].dist += d * d;

				d = (float)cam[2] - portals.portal[n].center.p[2];
				portals.portal[n].dist += d * d;

				portals.portal_sort[n] = n;
			}

			qsort( portals.portal_sort, portals.portal_count, 4, compare );

			for ( n = 0; n < portals.portal_count; n++ )
			{
				if ( portals.polygons == 2 && !portals.portal[portals.portal_sort[n]].hint ) {
					continue;
				}

				if ( portals.clip ) {
					if ( min_check[0] < portals.portal[portals.portal_sort[n]].min[0] ) {
						continue;
					}
					else if ( min_check[1] < portals.portal[portals.portal_sort[n]].min[1] ) {
						continue;
					}
					else if ( min_check[2] < portals.portal[portals.portal_sort[n]].min[2] ) {
						continue;
					}
					else if ( max_check[0] > portals.portal[portals.portal_sort[n]].max[0] ) {
						continue;
					}
					else if ( max_check[1] > portals.portal[portals.portal_sort[n]].max[1] ) {
						continue;
					}
					else if ( max_check[2] > portals.portal[portals.portal_sort[n]].max[2] ) {
						continue;
					}
				}

				g_QglTable.m_pfn_qglColor4f( portals.portal[portals.portal_sort[n]].fp_color_random[0], portals.portal[portals.portal_sort[n]].fp_color_random[1],
											 portals.portal[portals.portal_sort[n]].fp_color_random[2], trans );

				g_QglTable.m_pfn_qglBegin( GL_POLYGON );

				for ( p = 0; p < portals.portal[portals.portal_sort[n]].point_count; p++ )
					g_QglTable.m_pfn_qglVertex3fv( portals.portal[portals.portal_sort[n]].point[p].p );

				g_QglTable.m_pfn_qglEnd();
			}
		}
		else
		{
			for ( n = 0; n < portals.portal_count; n++ )
			{
				if ( portals.polygons == 2 && !portals.portal[n].hint ) {
					continue;
				}

				if ( portals.clip ) {
					if ( min_check[0] < portals.portal[n].min[0] ) {
						continue;
					}
					else if ( min_check[1] < portals.portal[n].min[1] ) {
						continue;
					}
					else if ( min_check[2] < portals.portal[n].min[2] ) {
						continue;
					}
					else if ( max_check[0] > portals.portal[n].max[0] ) {
						continue;
					}
					else if ( max_check[1] > portals.portal[n].max[1] ) {
						continue;
					}
					else if ( max_check[2] > portals.portal[n].max[2] ) {
						continue;
					}
				}

				g_QglTable.m_pfn_qglColor4f( portals.portal[n].fp_color_random[0], portals.portal[n].fp_color_random[1],
											 portals.portal[n].fp_color_random[2], trans );

				g_QglTable.m_pfn_qglBegin( GL_POLYGON );

				for ( p = 0; p < portals.portal[n].point_count; p++ )
					g_QglTable.m_pfn_qglVertex3fv( portals.portal[n].point[p].p );

				g_QglTable.m_pfn_qglEnd();
			}
		}
	}

	if ( portals.lines ) {
		g_QglTable.m_pfn_qglColor4fv( portals.fp_color_3d );

		for ( n = 0; n < portals.portal_count; n++ )
		{
			if ( portals.lines == 2 && !portals.portal[n].hint ) {
				continue;
			}

			if ( portals.clip ) {
				if ( min_check[0] < portals.portal[n].min[0] ) {
					continue;
				}
				else if ( min_check[1] < portals.portal[n].min[1] ) {
					continue;
				}
				else if ( min_check[2] < portals.portal[n].min[2] ) {
					continue;
				}
				else if ( max_check[0] > portals.portal[n].max[0] ) {
					continue;
				}
				else if ( max_check[1] > portals.portal[n].max[1] ) {
					continue;
				}
				else if ( max_check[2] > portals.portal[n].max[2] ) {
					continue;
				}
			}

			g_QglTable.m_pfn_qglBegin( GL_LINE_LOOP );

			for ( p = 0; p < portals.portal[n].point_count; p++ )
				g_QglTable.m_pfn_qglVertex3fv( portals.portal[n].inner_point[p].p );

			g_QglTable.m_pfn_qglEnd();
		}
	}

	g_QglTable.m_pfn_qglPopAttrib();
}
