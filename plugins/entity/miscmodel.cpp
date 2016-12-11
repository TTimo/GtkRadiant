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
#include <assert.h>

#include "entity_entitymodel.h"
#include "entity.h"

//
// CEntityMiscModel implementation
//

CEntityMiscModel::CEntityMiscModel ( entity_t *e ){
	refCount = 1;
	m_entity = e;
	m_model = NULL;
	VectorSet( m_translate, 0,0,0 );
	VectorSet( m_euler, 0,0,0 );
	VectorSet( m_scale, 1,1,1 );
	VectorSet( m_pivot, 0,0,0 );
	m4x4_identity( m_transform );
	m4x4_identity( m_inverse_transform );
}

CEntityMiscModel::~CEntityMiscModel (){
	if ( m_cachereq.GetBuffer()[0] != ':'
		 && m_version.c_str()[0] != '\0' ) {
		GetModelCache()->DeleteByID( m_cachereq.GetBuffer(), m_version.c_str() );
	}
}


// IRender

void CEntityMiscModel::Draw( int state, int rflags ) const {
	// push the current modelview matrix
	// FIXME: put in a check for stack recursion depth..
	// or avoid recursion of opengl matrix stack
	g_QglTable.m_pfn_qglPushMatrix();
	// apply the parent-to-local transform
	g_QglTable.m_pfn_qglMultMatrixf( m_transform );

	pivot_draw( m_pivot );

	// draw children
	if ( m_model && m_model->pRender ) {
		m_model->pRender->Draw(state, rflags);
	}

	g_QglTable.m_pfn_qglPopMatrix();
}

// ISelect

bool CEntityMiscModel::TestRay( const ray_t *ray, vec_t *dist ) const {
	vec_t dist_start = *dist;
	vec_t dist_local = *dist;
	ray_t ray_local = *ray;

	if ( aabb_test_ray( &m_BBox, ray ) == 0 ) {
		return false;
	}

	ray_transform( &ray_local, m_inverse_transform );

	if ( m_model && m_model->pSelect ) {
		if ( m_model->pSelect->TestRay( &ray_local, &dist_local ) ) {
			*dist = dist_local;
		}
	}
	else{ *dist = dist_local; }

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
	m4x4_pivoted_rotate_by_vec3( rotation_matrix, rotation, eXYZ, pivot );
	m4x4_transform_point( rotation_matrix, m_translate );

	VectorIncrement( rotation, m_euler );

	UpdateCachedData();
}

void CEntityMiscModel::OnKeyValueChanged( entity_t *e, const char *key, const char* value ){
	if ( strcmp( key, "model" ) == 0 ) {
		SetName( value );
	}
	else if ( strcmp( key, "_frame" ) == 0 ) {
		SetName( ValueForKey( e, "model" ) );
	}
	else if ( strcmp( key, "angle" ) == 0 ) {
		VectorSet( m_euler, 0.f, 0.f, 0.f );
		m_euler[2] = atof( value );
		UpdateCachedData();
	}
	else if ( strcmp( key, "angles" ) == 0 ) {
		VectorSet( m_euler, 0.f, 0.f, 0.f );
		if ( value[0] != '\0' ) {
			sscanf( value, "%f %f %f", &m_euler[1], &m_euler[2], &m_euler[0] );
		}
		UpdateCachedData();
	}
	else if ( strcmp( key, "modelscale" ) == 0 || strcmp( key,"modelscale_vec" ) == 0 ) {
		const char *s;
		VectorSet( m_scale, 1.f, 1.f, 1.f );
		s = ValueForKey( e,"modelscale" );
		if ( s[0] != '\0' ) {
			float f = atof( s );
			if ( f != 0 ) {
				VectorSet( m_scale, f, f, f );
			}
			else{
				Sys_FPrintf( SYS_WRN, "WARNING: ignoring 0 modelscale key\n" );
			}
		}
		s = ValueForKey( e,"modelscale_vec" );
		if ( s[0] != '\0' ) {
			sscanf( s, "%f %f %f", &m_scale[0], &m_scale[1], &m_scale[2] );
			if ( m_scale[0] == 0.0 && m_scale[1] == 0.0 && m_scale[2] == 0.0 ) {
				VectorSet( m_scale, 1,1,1 );
				Sys_FPrintf( SYS_WRN, "WARNING: ignoring 0 0 0 modelscale_vec key\n" );
			}
		}
		UpdateCachedData();
	}
	else if ( strcmp( key, "origin" ) == 0 ) {
		sscanf( value, "%f %f %f", &m_translate[0], &m_translate[1], &m_translate[2] );
		UpdateCachedData();
	}
	else if ( strncmp( key,"_remap",6 ) == 0 ) {
		SetName( ValueForKey( e, "model" ) );
	}
}

//
// CEntityMiscModel
//

// private:

void CEntityMiscModel::BuildCacheRequestString( const char *name ){
	bool hasRemaps = false;

	m_cachereq.Format( "%s:%i", name, IntForKey( m_entity,"_frame" ) );

	for ( epair_t* ep = m_entity->epairs ; ep ; ep = ep->next )
	{
		if ( strncmp( ep->key,"_remap",6 ) == 0 ) {
			if ( !hasRemaps ) {
				hasRemaps = true;
				m_cachereq += "?";
			}
			else {
				m_cachereq += "&";
			}
			m_cachereq += ep->value;
		}
	}
}

void CEntityMiscModel::SetName( const char *name ){
	Str m_oldcachereq = m_cachereq;

	if ( name[0] == '\0' ) {
		return;
	}

	BuildCacheRequestString( name );

	if ( strcmp( m_oldcachereq, m_cachereq ) == 0 ) {
		return;
	}

	if ( m_oldcachereq.GetBuffer()[0] != ':'
		 && m_version.c_str()[0] != '\0' ) {
		GetModelCache()->DeleteByID( m_oldcachereq.GetBuffer(), m_version.c_str() );
	} else {
		assert(m_model == NULL);
	}

	m_model = NULL;

	if ( name[0] != '\0' ) {
		const char* dot = strrchr( name, '.' );
		if ( dot != NULL ) {
			m_version = ++dot;
			m_model = GetModelCache()->GetByID( m_cachereq.GetBuffer(), m_version.c_str() );
		}
	}

	UpdateCachedData();
}


void CEntityMiscModel::UpdateCachedData(){
	aabb_t aabb_temp;

	m4x4_identity( m_transform );
	m4x4_pivoted_transform_by_vec3( m_transform, m_translate, m_euler, eXYZ, m_scale, m_pivot );
	memcpy( m_inverse_transform, m_transform, sizeof( m4x4_t ) );
	m4x4_invert( m_inverse_transform );

	aabb_clear( &aabb_temp );

	if ( m_model && m_model->pRender ) {
		aabb_extend_by_aabb( &aabb_temp, m_model->pRender->GetAABB() );
	}
	else{
		VectorSet( aabb_temp.extents, 8, 8, 8 );
	}

	aabb_for_transformed_aabb( &m_BBox, &aabb_temp, m_transform );
}
