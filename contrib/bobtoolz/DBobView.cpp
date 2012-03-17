/*
   BobToolz plugin for GtkRadiant
   Copyright (C) 2001 Gordon Biggans

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

// BobView.cpp: implementation of the DBobView class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DBobView.h"
#include "DListener.h"
#include "misc.h"
#include "funchandlers.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DBobView::DBobView(){
	nPathCount = 0;
	refCount = 1;

	m_bHooked = FALSE;

	path = NULL;
	eyes = NULL;

	boundingShow = BOUNDS_APEX;
}

DBobView::~DBobView(){
	if ( path ) {
		delete[] path;
	}

	// oops forgot to remove our eyes, was causing access violation when it tried
	// to talk to it's parent
	if ( eyes ) {
		delete eyes;
	}

	if ( m_bHooked ) {
		UnRegister();
	}

	g_PathView = NULL;
}

//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

void DBobView::Draw2D( VIEWTYPE vt ){
	if ( !path ) {
		return;
	}

	g_QglTable.m_pfn_qglPushAttrib( GL_ALL_ATTRIB_BITS );

	g_QglTable.m_pfn_qglDisable( GL_BLEND );
	g_QglTable.m_pfn_qglEnable( GL_LINE_SMOOTH );

	g_QglTable.m_pfn_qglPushMatrix();

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

	g_QglTable.m_pfn_qglLineWidth( 1.0f );
	g_QglTable.m_pfn_qglColor4f( 1.0f, 0.0f, 0.0f, 1.0f );

	int i;

	g_QglTable.m_pfn_qglBegin( GL_LINE_STRIP );

	for ( i = 0; i < nPathCount; i++ )
		g_QglTable.m_pfn_qglVertex3fv( path[i] );

	g_QglTable.m_pfn_qglEnd();

	if ( m_bShowExtra ) {
		// +mars
		// for the bounding box stuff
		g_QglTable.m_pfn_qglColor4f( 0.25f, 0.75f, 0.75f, 1.0f );

		g_QglTable.m_pfn_qglTranslatef( 16.0f, 16.0f, 28.0f );

		g_QglTable.m_pfn_qglBegin( GL_LINE_STRIP );

		for ( i = 0; i < nPathCount; i++ )
			g_QglTable.m_pfn_qglVertex3fv( path[i] );

		g_QglTable.m_pfn_qglEnd();

		// ---------------

		g_QglTable.m_pfn_qglTranslatef( -16.0f, -16.0f, -28.0f );   // back to where we were
		g_QglTable.m_pfn_qglTranslatef( -16.0f, 16.0f, 28.0f );     // move to new postion

		g_QglTable.m_pfn_qglBegin( GL_LINE_STRIP );

		for ( i = 0; i < nPathCount; i++ )
			g_QglTable.m_pfn_qglVertex3fv( path[i] );

		g_QglTable.m_pfn_qglEnd();

		// --------------

		g_QglTable.m_pfn_qglTranslatef( 16.0f, -16.0f, -28.0f );        // back to where we were
		g_QglTable.m_pfn_qglTranslatef( 16.0f, -16.0f, -28.0f );        // new pos

		g_QglTable.m_pfn_qglBegin( GL_LINE_STRIP );

		for ( i = 0; i < nPathCount; i++ )
			g_QglTable.m_pfn_qglVertex3fv( path[i] );

		g_QglTable.m_pfn_qglEnd();

		// ----------------

		g_QglTable.m_pfn_qglTranslatef( -16.0f, 16.0f, 28.0f );     // back to where we were

/*		g_QglTable.m_pfn_qglTranslatef( -16.0f, -16.0f, -28.0f );		// new pos

        g_QglTable.m_pfn_qglBegin( GL_LINE_STRIP );

        if ( boundingShow == BOUNDS_ALL )
        {
            for ( i = 0; i < nPathCount; i++ )
                g_QglTable.m_pfn_qglVertex3fv( path[i] );
        }
        else if ( boundingShow == BOUNDS_APEX )
        {
            for ( i = (nPathCount/4); i < (nPathCount/4) * 3; i++ )
                g_QglTable.m_pfn_qglVertex3fv( path[i] );
        }

        g_QglTable.m_pfn_qglEnd();*/                                                                                                                                                                                                                                                                                                                                                                                                                   // djbob: er, um doesn't really seem to do anyhting
	}

	// -mars

	g_QglTable.m_pfn_qglPopMatrix();

	g_QglTable.m_pfn_qglPopAttrib();
}

void DBobView::Draw3D(){
	if ( !path ) {
		return;
	}

	g_QglTable.m_pfn_qglPushAttrib( GL_ALL_ATTRIB_BITS );

	g_QglTable.m_pfn_qglDisable( GL_BLEND );
	g_QglTable.m_pfn_qglEnable( GL_LINE_SMOOTH );

	g_QglTable.m_pfn_qglLineWidth( 1.0f );
	g_QglTable.m_pfn_qglColor4f( 1.0f, 0.0f, 0.0f, 1.0f );

	g_QglTable.m_pfn_qglBegin( GL_LINE_STRIP );

	for ( int i = 0; i < nPathCount; i++ )
		g_QglTable.m_pfn_qglVertex3fv( path[i] );

	g_QglTable.m_pfn_qglEnd();

	if ( m_bShowExtra ) {
		// +mars
		// ahhh -- a nice C&P job :)
		// for the bounding box stuff
		g_QglTable.m_pfn_qglColor4f( 0.25f, 0.75f, 0.75f, 1.0f );

		g_QglTable.m_pfn_qglTranslatef( 16.0f, 16.0f, 28.0f );

		g_QglTable.m_pfn_qglBegin( GL_LINE_STRIP );

		int i;
		for ( i = 0; i < nPathCount; i++ )
			g_QglTable.m_pfn_qglVertex3fv( path[i] );

		g_QglTable.m_pfn_qglEnd();

		// ---------------

		g_QglTable.m_pfn_qglTranslatef( -16.0f, -16.0f, -28.0f );   // back to where we were
		g_QglTable.m_pfn_qglTranslatef( -16.0f, 16.0f, 28.0f );     // move to new postion

		g_QglTable.m_pfn_qglBegin( GL_LINE_STRIP );

		for ( i = 0; i < nPathCount; i++ )
			g_QglTable.m_pfn_qglVertex3fv( path[i] );

		g_QglTable.m_pfn_qglEnd();

		// --------------

		g_QglTable.m_pfn_qglTranslatef( 16.0f, -16.0f, -28.0f );        // back to where we were
		g_QglTable.m_pfn_qglTranslatef( 16.0f, -16.0f, -28.0f );        // new pos

		g_QglTable.m_pfn_qglBegin( GL_LINE_STRIP );

		for ( i = 0; i < nPathCount; i++ )
			g_QglTable.m_pfn_qglVertex3fv( path[i] );

		g_QglTable.m_pfn_qglEnd();

		// ----------------

		g_QglTable.m_pfn_qglTranslatef( -16.0f, 16.0f, 28.0f );     // back to where we were
		g_QglTable.m_pfn_qglTranslatef( -16.0f, -16.0f, -28.0f );       // new pos

		g_QglTable.m_pfn_qglBegin( GL_LINE_STRIP );

		for ( i = 0; i < nPathCount; i++ )
			g_QglTable.m_pfn_qglVertex3fv( path[i] );

		g_QglTable.m_pfn_qglEnd();
	}
	// -mars

	g_QglTable.m_pfn_qglPopAttrib();
}

void DBobView::Register(){
	g_QglTable.m_pfnHookGL2DWindow( this );
	g_QglTable.m_pfnHookGL3DWindow( this );
	m_bHooked = TRUE;
}

void DBobView::UnRegister(){
	g_QglTable.m_pfnUnHookGL2DWindow( this );
	g_QglTable.m_pfnUnHookGL3DWindow( this );
	m_bHooked = FALSE;
}

void DBobView::SetPath( vec3_t *pPath ){
	if ( path ) {
		delete[] path;
	}

	path = pPath;
}

#define LOCAL_GRAVITY -800.0f

bool DBobView::CalculateTrajectory( vec3_t start, vec3_t apex, float multiplier, int points, float varGravity ){
	if ( apex[2] <= start[2] ) {
		SetPath( NULL );
		return FALSE;
	}
	// ----think q3a actually would allow these
	//scrub that, coz the plugin wont :]

	vec3_t dist, speed;
	VectorSubtract( apex, start, dist );

	vec_t speed_z = (float)sqrt( -2 * LOCAL_GRAVITY * dist[2] );
	float flight_time = -speed_z / LOCAL_GRAVITY;


	VectorScale( dist, 1 / flight_time, speed );
	speed[2] = speed_z;

//	Sys_Printf("Speed: (%.4f %.4f %.4f)\n", speed[0], speed[1], speed[2]);

	vec3_t* pPath = new vec3_t[points];

	float interval = multiplier * flight_time / points;
	for ( int i = 0; i < points; i++ )
	{
		float ltime = interval * i;

		VectorScale( speed, ltime, pPath[i] );
		VectorAdd( pPath[i], start, pPath[i] );

		// could do this all with vectors
		// vGrav = {0, 0, -800.0f}
		// VectorScale(vGrav, 0.5f*ltime*ltime, vAdd);
		// VectorScale(speed, ltime, pPath[i]);
		// _VectorAdd(pPath[i], start, pPath[i])
		// _VectorAdd(pPath[i], vAdd, pPath[i])

		pPath[i][2] = start[2] + ( speed_z * ltime ) + ( varGravity * 0.5f * ltime * ltime );
	}

	SetPath( pPath );
	return TRUE;
}

void DBobView::Begin( const char* trigger, const char *target, float multiplier, int points, float varGravity, bool bNoUpdate, bool bShowExtra ){
	strcpy( entTrigger, trigger );
	strcpy( entTarget, target );

	fMultiplier = multiplier;
	fVarGravity = varGravity;
	nPathCount = points;
	m_bShowExtra = bShowExtra;

	Register();

	if ( UpdatePath() ) {
		if ( !bNoUpdate ) {
			eyes = new DListener;
			eyes->parent = this;
			eyes->Register();
		}
	}
	else
	{
		Sys_ERROR( "Initialization Failure in DBobView::Begin" );
		delete this;
	}
}

bool DBobView::UpdatePath(){
	vec3_t start, apex;

	if ( GetEntityCentre( entTrigger, start ) ) {
		if ( GetEntityCentre( entTarget, apex ) ) {
			CalculateTrajectory( start, apex, fMultiplier, nPathCount, fVarGravity );
			return TRUE;
		}
	}
	return FALSE;
}
