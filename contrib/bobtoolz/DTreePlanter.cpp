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
#include "DTreePlanter.h"
#include "funchandlers.h"

bool DTreePlanter::OnMouseMove( guint32 nFlags, gdouble x, gdouble y ) {
	return false;
}

bool DTreePlanter::OnLButtonDown( guint32 nFlags, gdouble x, gdouble y ) {
	VIEWTYPE vt = m_XYWrapper->GetViewType();

	switch ( vt ) {
	case XY:
		break;
	case YZ:
	case XZ:
	default:
		return false;
	}

	vec3_t pt, vhit;

	m_XYWrapper->SnapToGrid( static_cast< int >( x ), static_cast< int >( y ), pt );

	if ( FindDropPoint( pt, vhit ) ) {
		vhit[2] += m_offset;

		char buffer[128];
		DEntity e( m_entType );

		sprintf( buffer, "%i %i %i", (int)vhit[0], (int)vhit[1], (int)vhit[2] );
		e.AddEPair( "origin", buffer );

		if ( m_autoLink ) {
			entity_t* pLastEntity = NULL;
			entity_t* pThisEntity = NULL;

			int entNum = -1, lastEntNum = -1, entpos;
			for ( int i = 0; i < 256; i++ ) {
				sprintf( buffer, m_linkName, i );
				pThisEntity = FindEntityFromTargetname( buffer, &entNum );

				if ( pThisEntity ) {
					entpos = i;
					lastEntNum = entNum;
					pLastEntity = pThisEntity;
				}
			}

			if ( !pLastEntity ) {
				sprintf( buffer, m_linkName, 0 );
			}
			else {
				sprintf( buffer, m_linkName, entpos + 1 );
			}

			e.AddEPair( "targetname", buffer );

			if ( pLastEntity ) {
				DEntity e2;
				e2.LoadFromEntity( lastEntNum, TRUE );
				e2.AddEPair( "target", buffer );
				e2.RemoveFromRadiant();
				e2.BuildInRadiant( FALSE );
			}
		}

		if ( m_setAngles ) {
			int angleYaw = ( rand() % ( m_maxYaw - m_minYaw + 1 ) ) + m_minYaw;
			int anglePitch = ( rand() % ( m_maxPitch - m_minPitch + 1 ) ) + m_minPitch;

			sprintf( buffer, "%i %i 0", anglePitch, angleYaw );
			e.AddEPair( "angles", buffer );
		}

		if ( m_numModels ) {
			int treetype = rand() % m_numModels;
			e.AddEPair( "model", m_trees[treetype].name );
		}

		if ( m_useScale ) {
			float scale = ( ( ( rand() % 1000 ) * 0.001f ) * ( m_maxScale - m_minScale ) ) + m_minScale;

			sprintf( buffer, "%f", scale );
			e.AddEPair( "modelscale", buffer );
		}

		e.BuildInRadiant( FALSE );
	}

	if ( m_autoLink ) {
		DoTrainPathPlot();
	}

	return true;
}

bool DTreePlanter::OnLButtonUp( guint32 nFlags, gdouble x, gdouble y ) {
	return false;
}

bool DTreePlanter::OnRButtonDown( guint32 nFlags, gdouble x, gdouble y ) {
	return false;
}

bool DTreePlanter::OnRButtonUp( guint32 nFlags, gdouble x, gdouble y ) {
	return false;
}

bool DTreePlanter::OnMButtonDown( guint32 nFlags, gdouble x, gdouble y ) {
	return false;
}

bool DTreePlanter::OnMButtonUp( guint32 nFlags, gdouble x, gdouble y ) {
	return false;
}

bool DTreePlanter::FindDropPoint( vec3_t in, vec3_t out ) {
	DPlane p1;
	DPlane p2;

	vec3_t vUp =        { 0, 0, 1 };
	vec3_t vForward =   { 0, 1, 0 };
	vec3_t vLeft =      { 1, 0, 0 };

	in[2] = 65535;

	VectorCopy( in, p1.points[0] );
	VectorCopy( in, p1.points[1] );
	VectorCopy( in, p1.points[2] );
	VectorMA( p1.points[1], 20, vUp,         p1.points[1] );
	VectorMA( p1.points[1], 20, vLeft,       p1.points[2] );

	VectorCopy( in, p2.points[0] );
	VectorCopy( in, p2.points[1] );
	VectorCopy( in, p2.points[2] );
	VectorMA( p1.points[1], 20, vUp,         p2.points[1] );
	VectorMA( p1.points[1], 20, vForward,    p2.points[2] );

	p1.Rebuild();
	p2.Rebuild();

	bool found = false;
	vec3_t temp;
	vec_t dist;
	int cnt = m_world.GetIDMax();
	for ( int i = 0; i < cnt; i++ ) {
		DBrush* pBrush = m_world.GetBrushForID( i );

		if ( pBrush->IntersectsWith( &p1, &p2, temp ) ) {
			vec3_t diff;
			vec_t tempdist;
			VectorSubtract( in, temp, diff );
			tempdist = VectorLength( diff );
			if ( !found || ( tempdist < dist ) ) {
				dist = tempdist;
				VectorCopy( temp, out );
				found  = true;
			}
		}
	}

	return found;
}

void DTreePlanter::DropEntsToGround( void ) {
	// tell Radiant we want to access the selected brushes
	g_FuncTable.m_pfnAllocateSelectedBrushHandles();

	DEntity ent;

	int cnt = g_FuncTable.m_pfnSelectedBrushCount();
	for ( int i = 0; i < cnt; i++ ) {
		brush_t *brush = (brush_t*)g_FuncTable.m_pfnGetSelectedBrushHandle( i );

		ent.LoadFromEntity( brush->owner, TRUE );

		DEPair* pEpair = ent.FindEPairByKey( "origin" );
		if ( !pEpair ) {
			continue;
		}

		vec3_t vec, out;
		sscanf( pEpair->value.GetBuffer(), "%f %f %f", &vec[0], &vec[1], &vec[2] );

		FindDropPoint( vec, out );

		char buffer[256];
		sprintf( buffer, "%f %f %f", out[0], out[1], out[2] );
		ent.AddEPair( "origin", buffer );
		ent.RemoveFromRadiant();
		ent.BuildInRadiant( FALSE );
	}

	g_FuncTable.m_pfnReleaseSelectedBrushHandles();
}

void DTreePlanter::MakeChain( void ) {
	char buffer[256];
	int i;

	for ( i = 0; i < m_linkNum; i++ ) {
		DEntity e( "info_train_spline_main" );

		sprintf( buffer, "%s_pt%i", m_linkName, i );
		e.AddEPair( "targetname", buffer );

		sprintf( buffer, "0 %i 0", i * 64 );
		e.AddEPair( "origin", buffer );

		if ( i != m_linkNum - 1 ) {
			sprintf( buffer, "%s_pt%i", m_linkName, i + 1 );
			e.AddEPair( "target", buffer );

			sprintf( buffer, "%s_ctl%i", m_linkName, i );
			e.AddEPair( "control", buffer );
		}

		e.BuildInRadiant( FALSE );
	}

	for ( i = 0; i < m_linkNum - 1; i++ ) {
		DEntity e( "info_train_spline_control" );

		sprintf( buffer, "%s_ctl%i", m_linkName, i );
		e.AddEPair( "targetname", buffer );

		sprintf( buffer, "0 %i 0", ( i * 64 ) + 32 );
		e.AddEPair( "origin", buffer );

		e.BuildInRadiant( FALSE );
	}
}

void DTreePlanter::SelectChain( void ) {
/*	char buffer[256];

    for(int i = 0; i < m_linkNum; i++) {
        DEntity e("info_train_spline_main");

        sprintf( buffer, "%s_pt%i", m_linkName, i );
        e.AddEPair( "targetname", buffer );

        sprintf( buffer, "0 %i 0", i * 64 );
        e.AddEPair( "origin", buffer );

        if(i != m_linkNum-1) {
            sprintf( buffer, "%s_pt%i", m_linkName, i+1 );
            e.AddEPair( "target", buffer );

            sprintf( buffer, "%s_ctl%i", m_linkName, i );
            e.AddEPair( "control", buffer );
        }

        e.BuildInRadiant( FALSE );
    }

    for(int i = 0; i < m_linkNum-1; i++) {
        DEntity e("info_train_spline_control");

        sprintf( buffer, "%s_ctl%i", m_linkName, i );
        e.AddEPair( "targetname", buffer );

        sprintf( buffer, "0 %i 0", (i * 64) + 32);
        e.AddEPair( "origin", buffer );

        e.BuildInRadiant( FALSE );
    }*/
}
