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

#include "entitymodel.h"

extern CModelManager g_model_cache;

//
// CEntityMiscModel implementation
//

CEntityMiscModel::CEntityMiscModel (){
	refCount = 1;
	m_name = NULL;
	m_model = NULL;
	m_entity = NULL;
	m_frame = 0;
	m_remaps = g_ptr_array_new();
	m_shaders = g_ptr_array_new();
	VectorSet( m_translate, 0,0,0 );
	VectorSet( m_euler, 0,0,0 );
	VectorSet( m_scale, 1,1,1 );
	VectorSet( m_pivot, 0,0,0 );
	m4x4_identity( m_transform );
	m4x4_identity( m_inverse_transform );
}

typedef struct remap_s {
	char m_key[64];
	char m_remapbuff[64 + 1024];
	char *m_remap[2];
} remap_t;

CEntityMiscModel::~CEntityMiscModel (){
	unsigned int i;

	if ( m_name && *m_name != '\0' ) {
		if ( !g_model_cache.DeleteByNameAndFrame( m_name,m_frame ) && m_model ) {
			m_model->RemoveParent( this );
		}
		m_model = NULL;
		delete [] m_name;
	}

	for ( i = 0; i < m_remaps->len; i++ )
		delete (remap_t*)m_remaps->pdata[i];
	g_ptr_array_free( m_remaps, FALSE );

	for ( i = 0; i < m_shaders->len; i++ )
	{
		( *(IShader**)m_shaders->pdata[i] )->DecRef();
		delete (IShader**)m_shaders->pdata[i];
	}
	g_ptr_array_free( m_shaders, FALSE );

	if ( m_entity ) {
		// This might be just an evasion of the actual problem
		m_entity->model.pRender = NULL;
		m_entity->model.pSelect = NULL;
		m_entity->model.pEdit = NULL;
	}
}

// IRender

void CEntityMiscModel::Draw( int state, int rflags ) const {
	m4x4_t matrix;
	vec3_t pivot;

	memcpy( matrix, m_transform, sizeof( m4x4_t ) );
	m4x4_transpose( matrix );

	VectorAdd( m_pivot, m_translate, pivot );
	pivot_draw( pivot );

	// push the current modelview matrix
	// FIXME: put in a check for stack recursion depth..
	// or avoid recursion of opengl matrix stack
	g_QglTable.m_pfn_qglPushMatrix();
	// apply the parent-to-local transform
	g_QglTable.m_pfn_qglMultMatrixf( matrix );

	// draw children
	if ( m_model ) {
		m_model->Draw( state, m_shaders, rflags );
	}

	g_QglTable.m_pfn_qglPopMatrix();
}

// ISelect

bool CEntityMiscModel::TestRay( const ray_t *ray, vec_t *dist ) const {
	vec_t dist_start = *dist;
	vec_t dist_local = *dist;
	ray_t ray_local = *ray;

	if ( !aabb_intersect_ray( &m_BBox, &ray_local, &dist_local ) ) {
		return false;
	}

	if ( m_model ) {
		ray_transform( &ray_local, m_inverse_transform );
		dist_local = dist_start;
		if ( m_model->TestRay( &ray_local, &dist_local ) ) {
			*dist = dist_local;
		}
	}
	else{*dist = dist_local; }

	return *dist < dist_start;
}


//IEdit

void CEntityMiscModel::Translate( const vec3_t translation ){
	VectorIncrement( translation, m_translate );
	UpdateCachedData();
}

void CEntityMiscModel::Rotate( const vec3_t pivot, const vec3_t rotation ){
	m4x4_t rotation_matrix;

	m4x4_identity( rotation_matrix );
	m4x4_pivoted_rotate_by_vec3( rotation_matrix, rotation, pivot );
	m4x4_transform_point( rotation_matrix, m_translate );

	VectorIncrement( rotation, m_euler );

	UpdateCachedData();
}

void CEntityMiscModel::OnKeyChanged( entity_t *e, const char *key ){
	const char *value;

	// FIXME: keys are case-sensitive?

	m_entity = e;

	if ( strcmp( key,"model" ) == 0 ) {
		SetName( ValueForKey( e,"model" ) );
	}
	else if ( strcmp( key,"_frame" ) == 0 ) {
		SetFrame( IntForKey( e,"_frame" ) );
	}
	else if ( strcmp( key,"angle" ) == 0 || strcmp( key,"angles" ) == 0 ) {
		VectorSet( m_euler, 0.f, 0.f, 0.f );
		m_euler[2] = FloatForKey( e,"angle" );
		value = ValueForKey( e,"angles" );
		if ( value[0] != '\0' ) {
			sscanf( value, "%f %f %f", &m_euler[0], &m_euler[2], &m_euler[1] );
		}
		UpdateCachedData();
	}
	else if ( strcmp( key,"modelscale" ) == 0 || strcmp( key,"modelscale_vec" ) == 0 ) {
		VectorSet( m_scale, 1.f, 1.f, 1.f );
		value = ValueForKey( e,"modelscale" );
		if ( value[0] != '\0' ) {
			float f = atof( value );
			if ( f != 0 ) {
				VectorSet( m_scale, f, f, f );
			}
			else{
				Sys_FPrintf( SYS_WRN, "WARNING: ignoring 0 modelscale key\n" );
			}
		}
		value = ValueForKey( e,"modelscale_vec" );
		if ( value[0] != '\0' ) {
			sscanf( value, "%f %f %f", &m_scale[0], &m_scale[1], &m_scale[2] );
			if ( m_scale[0] == 0.0 && m_scale[1] == 0.0 && m_scale[2] == 0.0 ) {
				VectorSet( m_scale, 1,1,1 );
				Sys_FPrintf( SYS_WRN, "WARNING: ignoring 0 0 0 modelscale_vec key\n" );
			}
		}
		UpdateCachedData();
	}
	else if ( strcmp( key,"origin" ) == 0 ) {
		value = ValueForKey( e,"origin" );
		sscanf( value, "%f %f %f", &m_translate[0], &m_translate[1], &m_translate[2] );
		UpdateCachedData();
	}
	else if ( strncmp( key,"_remap",6 ) == 0 ) {
		unsigned int i;
		remap_t *pRemap;
		char *ch;

		value = ValueForKey( e,key );

		for ( i = 0; i < m_remaps->len; i++ )
		{
			pRemap = (remap_t*)m_remaps->pdata[i];
			if ( strcmp( key,pRemap->m_key ) == 0 ) {
				break;
			}
		}

		if ( i == m_remaps->len ) {
			if ( value[0] == '\0' ) {
				return;
			}

			pRemap = new remap_t;
			g_ptr_array_add( m_remaps, pRemap );
		}
		else if ( value[0] == '\0' ) {
			g_ptr_array_remove_index_fast( m_remaps, i );
			delete pRemap;

			UpdateShaders();
			return;
		}

		strncpy( pRemap->m_remapbuff,value,sizeof( pRemap->m_remapbuff ) );
		strncpy( pRemap->m_key,key,sizeof( pRemap->m_key ) );

		pRemap->m_remap[0] = ch = pRemap->m_remapbuff;

		while ( *ch && *ch != ';' )
			ch++;

		if ( *ch == '\0' ) {
			// bad remap
			Sys_FPrintf( SYS_WRN, "WARNING: Shader _remap key found in misc_model without a ; character\n" );
			g_ptr_array_remove_index_fast( m_remaps, i );
			delete pRemap;
			return;
		}
		else
		{
			*ch = '\0';
			pRemap->m_remap[1] = ch + 1;
		}

		UpdateShaders();
	}
}

//
// CEntityMiscModel
//

// private:

void CEntityMiscModel::SetName( const char *name ){
	if ( m_name && *m_name != '\0' ) {
		if ( strcmp( m_name, name ) == 0 ) {
			return;
		}
		if ( !g_model_cache.DeleteByNameAndFrame( m_name,m_frame ) && m_model ) {
			m_model->RemoveParent( this );
		}
		delete [] m_name;
	}

	m_model = NULL;
	m_name = new char[strlen( name ) + 1];
	strcpy( m_name,name );

	if ( *m_name != '\0' ) {
		m_model = g_model_cache.GetByNameAndFrame( m_name, m_frame );
		m_model->AddParent( this );
	}

	UpdateCachedData();
	UpdateShaders();
}

void CEntityMiscModel::SetFrame( const int frame ){
	if ( m_frame == frame ) {
		return;
	}

	if ( m_name && *m_name != '\0' ) {
		if ( !g_model_cache.DeleteByNameAndFrame( m_name,m_frame ) && m_model ) {
			m_model->RemoveParent( this );
		}
	}

	m_model = NULL;

	m_frame = frame;

	if ( *m_name != '\0' ) {
		m_model = g_model_cache.GetByNameAndFrame( m_name, m_frame );
		m_model->AddParent( this );
	}

	UpdateCachedData();
}

void CEntityMiscModel::UpdateCachedData(){
	aabb_t aabb_temp;
	bbox_t bbox_temp;

	m4x4_identity( m_transform );
	m4x4_pivoted_transform_by_vec3( m_transform, m_translate, m_euler, m_scale, m_pivot );
	memcpy( m_inverse_transform, m_transform, sizeof( m4x4_t ) );
	if ( m4x4_invert( m_inverse_transform ) == 1 ) {
		Sys_FPrintf( SYS_ERR, "ERROR: Singular Matrix, cannot invert" );
	}

	aabb_clear( &aabb_temp );

	if ( m_model ) {
		aabb_extend_by_aabb( &aabb_temp, m_model->GetAABB() );
	}
	else
	{
		if ( m_entity->eclass ) {
			VectorSet( aabb_temp.extents, m_entity->eclass->maxs[0], m_entity->eclass->maxs[1], m_entity->eclass->maxs[2] );
		}
		else{
			VectorSet( aabb_temp.extents, 8, 8, 8 );
		}
	}

	// create an oriented BBox in world-space
	bbox_for_oriented_aabb( &bbox_temp, &aabb_temp, m_transform, m_euler, m_scale );
	// create an axis aligned bbox in world-space
	aabb_for_bbox( &m_BBox, &bbox_temp );

	aabb_update_radius( &m_BBox );
}

void CEntityMiscModel::UpdateShaders(){
	unsigned int i, j, numSurfaces;
	remap_t *pRemap, *pGlobRemap = NULL;
	char *surfShaderName;
	IShader **pShader;

	if ( !m_model ) {
		if ( m_shaders->len ) {
			// free our shaders
			for ( i = 0; i < m_shaders->len; i++ )
			{
				g_ptr_array_remove_index_fast( m_shaders, i );
				( *(IShader**)m_shaders->pdata[i] )->DecRef();
				delete (IShader**)m_shaders->pdata[i];
			}
		}
		return;
	}

	numSurfaces = m_model->GetNumSurfaces();

	if ( numSurfaces < m_shaders->len ) {
		// free unneeded shader pointers
		for ( i = m_shaders->len - 1; i >= numSurfaces; i-- )
		{
			g_ptr_array_remove_index_fast( m_shaders, i );
			( *(IShader**)m_shaders->pdata[i] )->DecRef();
			delete (IShader**)m_shaders->pdata[i];
		}
	}

	// now go through our surface and find our shaders, remap if needed
	for ( j = 0; j < numSurfaces; j++ )
	{
		surfShaderName = m_model->GetShaderNameForSurface( j );

		if ( j < m_shaders->len ) {
			pShader = (IShader **)m_shaders->pdata[j];
		}
		else
		{
			pShader = new (IShader *);
			*pShader = NULL;
			g_ptr_array_add( m_shaders, pShader );
		}

		if ( m_remaps->len ) {
			for ( i = 0; i < m_remaps->len; i++ )
			{
				pRemap = (remap_t*)m_remaps->pdata[i];
				if ( stricmp( pRemap->m_remap[0],surfShaderName ) == 0 ) {
					// only do the shader lookups if really needed
					if ( !( *pShader ) || stricmp( pRemap->m_remap[1],( *pShader )->getName() ) ) {
						if ( *pShader ) {
							( *pShader )->DecRef();
						}
						*pShader = QERApp_Shader_ForName( pRemap->m_remap[1] );
					}

					pGlobRemap = NULL;
					break;
				}
				else if ( pRemap->m_remap[0][0] == '*' && pRemap->m_remap[0][1] == '\0' ) {
					pGlobRemap = pRemap;
				}
			}

			if ( pGlobRemap ) {
				if ( !( *pShader ) || stricmp( pGlobRemap->m_remap[1],( *pShader )->getName() ) ) {
					if ( *pShader ) {
						( *pShader )->DecRef();
					}
					*pShader = QERApp_Shader_ForName( pGlobRemap->m_remap[1] );
				}
			}
			else if ( i == m_remaps->len ) {
				// Back to the default one, if needed
				if ( !( *pShader ) || ( stricmp( surfShaderName,( *pShader )->getName() ) && !( surfShaderName[0] == '\0' ) ) ) {
					if ( *pShader ) {
						( *pShader )->DecRef();
					}
					*pShader = QERApp_Shader_ForName( surfShaderName );
				}
			}
		}
		else
		{
			// Model specified shader, if needed
			if ( !( *pShader ) || ( stricmp( surfShaderName,( *pShader )->getName() ) && !( surfShaderName[0] == '\0' ) ) ) {
				if ( *pShader ) {
					( *pShader )->DecRef();
				}
				*pShader = QERApp_Shader_ForName( surfShaderName );
			}
		}
	}
}
