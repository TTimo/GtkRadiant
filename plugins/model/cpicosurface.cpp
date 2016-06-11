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

#include "cpicosurface.h"

// public

CPicoSurface::CPicoSurface( picoSurface_t *pSurface ){
	refCount = 1;

	m_pSurface = pSurface;

	// PicoFixSurfaceNormals( pSurface );

	AccumulateBBox();

	m_shader = QERApp_Shader_ForName( GetShaderName() );

	// jdolan: If the shader fails to load, try skin.tga in the model's
	// directory as a fall-back. This is a hack for malformed models.

	if ( m_shader->IsDefault() ) {
		Sys_FPrintf( SYS_WRN, "WARNING: Failed to load shader %s for %s\n", m_shader->getName(), m_pSurface->model->name );

		gchar *dir = g_path_get_dirname( m_pSurface->model->name );
		gchar *skin = g_strdup_printf( "%s/skin.tga", dir );

		m_shader->DecRef();
		m_shader = QERApp_Shader_ForName( skin );

		g_free( skin );
		g_free( dir );
	}
}

CPicoSurface::~CPicoSurface(){
	m_shader->DecRef();
}

void CPicoSurface::Draw( int state, int rflags ){
	Draw( state, m_shader, rflags );
}

void CPicoSurface::Draw( int state, IShader *pShader, int rflags ){
	int j;

	if ( !( rflags & ( DRAW_RF_SEL_OUTLINE | DRAW_RF_SEL_FILL | DRAW_RF_XY ) ) ) {
		if ( state & DRAW_GL_TEXTURE_2D ) {
			bool bTrans = ( pShader->getFlags() & QER_TRANS ) == QER_TRANS;
			bool bDrawBlend = ( state & DRAW_GL_BLEND ) == DRAW_GL_BLEND;
			//only draw transparent stuff when in transparent stuff pass and vice versa
			if(bTrans != bDrawBlend) {
				return;
			}
			g_QglTable.m_pfn_qglBindTexture( GL_TEXTURE_2D, pShader->getTexture()->texture_number );
			if ( ( rflags & DRAW_RF_CAM ) && ( pShader->getFlags() & QER_ALPHAFUNC ) ) {
				int nFunc = 0;
				float fRef = 0.f;

				g_QglTable.m_pfn_qglColor4f( 1.f, 1.f, 1.f, pShader->getTrans() ); // transparency

				g_QglTable.m_pfn_qglEnable( GL_ALPHA_TEST );

				pShader->getAlphaFunc( &nFunc, &fRef );
				g_QglTable.m_pfn_qglAlphaFunc( nFunc, fRef );
			}
		}
		else
		{
			//g_QglTable.m_pfn_qglColor3fv( pShader->getTexture()->color );
/*      g_QglTable.m_pfn_qglEnableClientState(GL_COLOR_ARRAY);*/
		}

		if ( !( state & DRAW_GL_WIRE ) && ( pShader->getFlags() & QER_CULL ) ) {
			if ( pShader->getCull() == 2 ) {
				g_QglTable.m_pfn_qglDisable( GL_CULL_FACE );
				g_QglTable.m_pfn_qglPolygonMode( GL_FRONT, GL_FILL );
			}
			else // is 1
			{
				g_QglTable.m_pfn_qglCullFace( GL_BACK );
			}
		}
	}

	switch ( PicoGetSurfaceType( m_pSurface ) )
	{
	case PICO_TRIANGLES:  g_QglTable.m_pfn_qglBegin( GL_TRIANGLES );
		for ( j = 0; j < PicoGetSurfaceNumIndexes( m_pSurface ); j++ )
		{
			g_QglTable.m_pfn_qglNormal3fv( PicoGetSurfaceNormal( m_pSurface,PicoGetSurfaceIndex( m_pSurface,j ) ) );

			if ( !( rflags & ( DRAW_RF_SEL_OUTLINE | DRAW_RF_SEL_FILL | DRAW_RF_XY ) ) ) {
				if ( state & DRAW_GL_TEXTURE_2D ) {
					g_QglTable.m_pfn_qglTexCoord2fv( PicoGetSurfaceST( m_pSurface,0,PicoGetSurfaceIndex( m_pSurface,j ) ) );
				}
				else {
					picoByte_t *vertexColor = PicoGetSurfaceColor( m_pSurface,0,PicoGetSurfaceIndex( m_pSurface,j ) );
					//% g_QglTable.m_pfn_qglColor4f( vertexColor[ 0 ] / 255.f,
					//%                              vertexColor[ 1 ] / 255.f,
					//%                              vertexColor[ 2 ] / 255.f,
					//%                              vertexColor[ 3 ] / 255.f );
					g_QglTable.m_pfn_qglColor4ubv( vertexColor );
				}
			}
			g_QglTable.m_pfn_qglVertex3fv( PicoGetSurfaceXYZ( m_pSurface, PicoGetSurfaceIndex( m_pSurface, j ) ) );
		}
		g_QglTable.m_pfn_qglEnd();
		/*g_QglTable.m_pfn_qglVertexPointer( 3, GL_FLOAT, 0, PicoGetSurfaceXYZ( m_pSurface, 0 ) );
		   g_QglTable.m_pfn_qglNormalPointer( GL_FLOAT, 0, PicoGetSurfaceNormal( m_pSurface, 0 ) );
		   if( !(rflags & (DRAW_RF_SEL_OUTLINE|DRAW_RF_SEL_FILL|DRAW_RF_XY)) ) {
		   if( state & DRAW_GL_TEXTURE_2D ) {
		    g_QglTable.m_pfn_qglTexCoordPointer( 2, GL_FLOAT, 0, PicoGetSurfaceST( m_pSurface, 0, 0 ) );
		   } else {
		    g_QglTable.m_pfn_qglColorPointer( 4, GL_UNSIGNED_BYTE, 0, PicoGetSurfaceColor( m_pSurface, 0, 0 ) );
		   }
		   }
		   g_QglTable.m_pfn_qglDrawElements( GL_TRIANGLES, PicoGetSurfaceNumIndexes( m_pSurface ), GL_UNSIGNED_INT, PicoGetSurfaceIndexes( m_pSurface, 0 ) );*/

		/*g_QglTable.m_pfn_qglColor3f( 0.f, .5f, 1.f );
		   g_QglTable.m_pfn_qglBegin( GL_LINES );
		   for( int i = 0; i < PicoGetSurfaceNumIndexes( m_pSurface ); i++ ) {
		   vec3_t outerpoint;
		   VectorMA( PicoGetSurfaceXYZ( m_pSurface, PicoGetSurfaceIndex( m_pSurface, i ) ), .3f, PicoGetSurfaceNormal( m_pSurface, PicoGetSurfaceIndex( m_pSurface, i ) ), outerpoint );
		   g_QglTable.m_pfn_qglVertex3fv( PicoGetSurfaceXYZ( m_pSurface, PicoGetSurfaceIndex( m_pSurface, i ) ) );
		   g_QglTable.m_pfn_qglVertex3fv( outerpoint );
		   }
		   g_QglTable.m_pfn_qglEnd();*/

		break;
	default:              Sys_FPrintf( SYS_ERR, "ERROR: Unsupported Pico Surface Type: %i", PicoGetSurfaceType( m_pSurface ) );
		break;
	}

	if ( !( rflags & ( DRAW_RF_SEL_OUTLINE | DRAW_RF_SEL_FILL | DRAW_RF_XY ) ) ) {
		if ( ( state & DRAW_GL_TEXTURE_2D ) && ( rflags & DRAW_RF_CAM ) && ( pShader->getFlags() & QER_ALPHAFUNC ) ) {
			g_QglTable.m_pfn_qglDisable( GL_ALPHA_TEST );
		}

/*	if(!(state & DRAW_GL_TEXTURE_2D)) {
      g_QglTable.m_pfn_qglDisableClientState(GL_COLOR_ARRAY);
    }*/

		if ( !( state & DRAW_GL_WIRE ) && ( pShader->getFlags() & QER_CULL ) ) {
			if ( pShader->getCull() == 2 ) {
				g_QglTable.m_pfn_qglPolygonMode( GL_FRONT, GL_LINE );
				g_QglTable.m_pfn_qglEnable( GL_CULL_FACE );
			}
			else // is 1
			{
				g_QglTable.m_pfn_qglCullFace( GL_FRONT );
			}
		}
	}
}

// private

void CPicoSurface::AccumulateBBox(){
	int i;
	picoVec_t *p;
	aabb_clear( &m_BBox );
	for ( i = 0; i < PicoGetSurfaceNumVertexes( m_pSurface ); i++ )
	{
		p = PicoGetSurfaceXYZ( m_pSurface,i );
		aabb_extend_by_point( &m_BBox, p );
	}
	aabb_update_radius( &m_BBox );
}

bool CPicoSurface::TestRay( const ray_t *ray, vec_t *dist ) const {
	int i;
	vec_t start_dist = *dist;
	vec_t local_dist = *dist;
	if ( aabb_intersect_ray( &m_BBox, ray, &local_dist ) ) {
		switch ( PicoGetSurfaceType( m_pSurface ) )
		{
		case PICO_TRIANGLES:
			for ( i = 0; i < PicoGetSurfaceNumIndexes( m_pSurface ); i += 3 )
			{
				local_dist = ray_intersect_triangle( ray, true, PicoGetSurfaceXYZ( m_pSurface,PicoGetSurfaceIndex( m_pSurface,i + 2 ) ),
													 PicoGetSurfaceXYZ( m_pSurface,PicoGetSurfaceIndex( m_pSurface,i + 1 ) ),
													 PicoGetSurfaceXYZ( m_pSurface,PicoGetSurfaceIndex( m_pSurface,i ) ) );
				if ( local_dist < *dist ) {
					*dist = local_dist;
				}
			}
			break;
		default:
			Sys_FPrintf( SYS_ERR, "ERROR: Unsupported Pico Surface Type: %i", PicoGetSurfaceType( m_pSurface ) );
			break;
		}
	}
	return ( *dist < start_dist );
}
