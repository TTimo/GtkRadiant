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

#include "StdAfx.h"
#include "DPoint.h"

#include "DTrainDrawer.h"
#include "DEPair.h"

#include "misc.h"
#include "funchandlers.h"

#include "dialogs/dialogs-gtk.h"

DTrainDrawer::DTrainDrawer() {
	refCount = 1;
	m_bHooked = FALSE;
	m_bDisplay = FALSE;

	BuildPaths();
}

DTrainDrawer::~DTrainDrawer( void ) {
	if ( m_bHooked ) {
		UnRegister();
	}

	ClearPoints();
	ClearSplines();
}

void DTrainDrawer::ClearSplines() {
	for ( list<splinePoint_t *>::const_iterator deadSpline = m_splineList.begin(); deadSpline != m_splineList.end(); deadSpline++ ) {
		( *deadSpline )->m_pointList.clear();
		( *deadSpline )->m_vertexList.clear();
		delete ( *deadSpline );
	}

	m_splineList.clear();
}

void DTrainDrawer::ClearPoints() {
	for ( list<controlPoint_t *>::const_iterator deadPoint = m_pointList.begin(); deadPoint != m_pointList.end(); deadPoint++ ) {
		delete *deadPoint;
	}

	m_pointList.clear();
}

void DTrainDrawer::Register() {
	g_QglTable.m_pfnHookGL2DWindow( this );
	g_QglTable.m_pfnHookGL3DWindow( this );
	m_bHooked = TRUE;
}

void DTrainDrawer::UnRegister() {
	g_QglTable.m_pfnUnHookGL2DWindow( this );
	g_QglTable.m_pfnUnHookGL3DWindow( this );
	m_bHooked = FALSE;
}

void CalculateSpline_r( vec3_t* v, int count, vec3_t out, float tension ) {
	vec3_t dist;

	if ( count < 2 ) {
		return;
	}

	if ( count == 2 ) {
		VectorSubtract( v[1], v[0], dist );
		VectorMA( v[0], tension, dist, out );
		return;
	}

	vec3_t* v2 = new vec3_t[count - 1];

	for ( int i = 0; i < count - 1; i++ ) {
		VectorSubtract( v[i + 1], v[i], dist );
		VectorMA( v[i], tension, dist, v2[i] );
	}

	CalculateSpline_r( v2, count - 1, out, tension );

	delete[] v2;
}

void DTrainDrawer::Draw3D() {

	if ( !m_bDisplay ) {
		return;
	}

	g_QglTable.m_pfn_qglPushAttrib( GL_ALL_ATTRIB_BITS );

	g_QglTable.m_pfn_qglDisable( GL_BLEND );
	g_QglTable.m_pfn_qglDisable( GL_LINE_SMOOTH );

	g_QglTable.m_pfn_qglPushMatrix();

	g_QglTable.m_pfn_qglLineWidth( 2.0f );
	g_QglTable.m_pfn_qglColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

	g_QglTable.m_pfn_qglEnable( GL_BLEND );
	g_QglTable.m_pfn_qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	g_QglTable.m_pfn_qglDisable( GL_POLYGON_SMOOTH );

	g_QglTable.m_pfn_qglDepthFunc( GL_ALWAYS );

	for ( list<splinePoint_t* >::const_iterator sp = m_splineList.begin(); sp != m_splineList.end(); sp++ ) {
		splinePoint_t* pSP = ( *sp );

		g_QglTable.m_pfn_qglBegin( GL_LINE_STRIP );
		for ( list<DPoint >::const_iterator v = pSP->m_vertexList.begin(); v != pSP->m_vertexList.end(); v++ ) {
			g_QglTable.m_pfn_qglVertex3fv( ( *v )._pnt );
		}
		g_QglTable.m_pfn_qglEnd();

	}

	g_QglTable.m_pfn_qglPopMatrix();
	g_QglTable.m_pfn_qglPopAttrib();
}

void DTrainDrawer::Draw2D( VIEWTYPE vt ) {

	if ( !m_bDisplay ) {
		return;
	}

	g_QglTable.m_pfn_qglPushAttrib( GL_ALL_ATTRIB_BITS );

	g_QglTable.m_pfn_qglDisable( GL_BLEND );
	g_QglTable.m_pfn_qglDisable( GL_LINE_SMOOTH );

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
	g_QglTable.m_pfn_qglColor4f( 1.0f, 0.0f, 0.0f, 0.5f );

	g_QglTable.m_pfn_qglEnable( GL_BLEND );
	g_QglTable.m_pfn_qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	g_QglTable.m_pfn_qglDisable( GL_POLYGON_SMOOTH );

	g_QglTable.m_pfn_qglDepthFunc( GL_ALWAYS );

	g_QglTable.m_pfn_qglColor4f( 1.f, 0.f, 0.f, 1.f );

	for ( list<splinePoint_t* >::const_iterator sp = m_splineList.begin(); sp != m_splineList.end(); sp++ ) {
		splinePoint_t* pSP = ( *sp );

		g_QglTable.m_pfn_qglBegin( GL_LINE_STRIP );
		for ( list<DPoint >::const_iterator v = pSP->m_vertexList.begin(); v != pSP->m_vertexList.end(); v++ ) {
			g_QglTable.m_pfn_qglVertex3fv( ( *v )._pnt );
		}
		g_QglTable.m_pfn_qglEnd();

	}

	g_QglTable.m_pfn_qglPopMatrix();
	g_QglTable.m_pfn_qglPopAttrib();
}

void AddSplineControl( const char* control, splinePoint_t* pSP ) {
	controlPoint_t cp;
	strncpy( cp.strName, control, 64 );

	pSP->m_pointList.push_front( cp );
}

void DTrainDrawer::BuildPaths() {
	int count = g_FuncTable.m_pfnGetEntityCount();

	DEntity e;

	for ( int i = 0; i < count; i++ ) {
		entity_s* ent = (entity_s*)g_FuncTable.m_pfnGetEntityHandle( i );
		e.ClearEPairs();
		e.LoadEPairList( *g_EntityTable.m_pfnGetEntityKeyValList( ent ) );

		const char* classname = e.m_Classname.GetBuffer();
		const char* target;
		const char* control;
		const char* targetname;
		vec3_t vOrigin;

		e.SpawnString( "targetname", NULL, &targetname );
		e.SpawnVector( "origin", "0 0 0", vOrigin );

		if ( !strcmp( classname, "info_train_spline_main" ) ) {
			if ( !targetname ) {
				Sys_Printf( "info_train_spline_main with no targetname" );
				return;
			}

			e.SpawnString( "target", NULL, &target );

			if ( !target ) {
				AddControlPoint( targetname, vOrigin );
			}
			else {
				splinePoint_t* pSP = AddSplinePoint( targetname, target, vOrigin );

				e.SpawnString( "control", NULL, &control );

				if ( control ) {
					AddSplineControl( control, pSP );

					for ( int j = 2;; j++ ) {
						char buffer[16];
						sprintf( buffer, "control%i", j );

						e.SpawnString( buffer, NULL, &control );
						if ( !control ) {
							break;
						}

						AddSplineControl( control, pSP );
					}
				}
			}
		}
		else if ( !strcmp( classname, "info_train_spline_control" ) ) {
			if ( !targetname ) {
				Sys_Printf( "info_train_spline_control with no targetname" );
				return;
			}

			AddControlPoint( targetname, vOrigin );
		}
	}

	list<splinePoint_t* >::const_iterator sp;
	for ( sp = m_splineList.begin(); sp != m_splineList.end(); sp++ ) {
		splinePoint_t* pSP = ( *sp );

		controlPoint_t* pTarget = FindControlPoint( pSP->strTarget );

		if ( !pTarget ) {
			Sys_Printf( "couldn't find target %s", pSP->strTarget );
			return;
//			continue;
		}

		pSP->pTarget = pTarget;


		for ( list<controlPoint_t >::iterator cp = pSP->m_pointList.begin(); cp != pSP->m_pointList.end(); cp++ ) {
			controlPoint_t* pControl = FindControlPoint( ( *cp ).strName );
			if ( !pControl ) {
				Sys_Printf( "couldn't find control %s", ( *cp ).strName );
				return;
			}

			VectorCopy( pControl->vOrigin, ( *cp ).vOrigin );
		}
	}

	m_bDisplay = TRUE;
	Register();

	for ( sp = m_splineList.begin(); sp != m_splineList.end(); sp++ ) {
		splinePoint_t* pSP = ( *sp );
		DPoint out;

		if ( !pSP->pTarget ) {
			continue;
		}

		int count = pSP->m_pointList.size() + 2;
		vec3_t* v = new vec3_t[count];

		VectorCopy( pSP->point.vOrigin, v[0] );

		int i = 1;
		for ( list<controlPoint_t>::reverse_iterator cp = pSP->m_pointList.rbegin(); cp != pSP->m_pointList.rend(); cp++ ) {
			VectorCopy( ( *cp ).vOrigin, v[i] );
			i++;
		}
		VectorCopy( pSP->pTarget->vOrigin, v[i] );

		for ( float tension = 0.0f; tension <= 1.f; tension += 0.01f ) {
			CalculateSpline_r( v, count, out._pnt, tension );
			pSP->m_vertexList.push_front( out );
		}

		delete[] v;

		VectorCopy( pSP->pTarget->vOrigin, out._pnt );
		pSP->m_vertexList.push_front( out );
	}


}

void DTrainDrawer::AddControlPoint( const char* name, vec_t* origin ){
	controlPoint_t* pCP = new controlPoint_t;

	strncpy( pCP->strName, name, 64 );
	VectorCopy( origin, pCP->vOrigin );

	m_pointList.push_back( pCP );
}

splinePoint_t* DTrainDrawer::AddSplinePoint( const char* name, const char* target, vec_t* origin ){
	splinePoint_t* pSP = new splinePoint_t;

	strncpy( pSP->point.strName, name,       64 );
	strncpy( pSP->strTarget,     target,     64 );
	VectorCopy( origin, pSP->point.vOrigin );
	m_splineList.push_back( pSP );

	return pSP;
}

controlPoint_t* DTrainDrawer::FindControlPoint( const char* name ){
	for ( list<controlPoint_t*>::const_iterator cp = m_pointList.begin(); cp != m_pointList.end(); cp++ ) {
		if ( !strcmp( name, ( *cp )->strName ) ) {
			return ( *cp );
		}
	}

	for ( list<splinePoint_t*>::const_iterator sp = m_splineList.begin(); sp != m_splineList.end(); sp++ ) {
		if ( !strcmp( name, ( *sp )->point.strName ) ) {
			return &( ( *sp )->point );
		}
	}

	return NULL;
}
