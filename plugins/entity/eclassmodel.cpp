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

#include "entity_entitymodel.h"

//
// CEntityEclassModel implementation
//

CEntityEclassModel::CEntityEclassModel (){
	refCount = 1;
	m_eclass = NULL;
	m_model = NULL;
	VectorSet( m_translate, 0,0,0 );
	VectorSet( m_euler, 0,0,0 );
	VectorSet( m_scale, 1,1,1 );
	VectorSet( m_pivot, 0,0,0 );
	m4x4_identity( m_transform );
	m4x4_identity( m_inverse_transform );
}

CEntityEclassModel::~CEntityEclassModel (){
	if ( m_name.c_str()[0] != '\0'
		 && m_version.c_str()[0] != '\0' ) {
		GetModelCache()->DeleteByID( m_name.c_str(), m_version.c_str() );
	}
}


// IRender

void CEntityEclassModel::Draw( int state, int rflags ) const {
	// push the current modelview matrix
	// FIXME: put in a check for stack recursion depth..
	// or avoid recursion of opengl matrix stack
	g_QglTable.m_pfn_qglPushMatrix();
	// apply the parent-to-local transform
	g_QglTable.m_pfn_qglMultMatrixf( m_transform );

	// draw children
	if ( m_model && m_model->pRender ) {
		m_model->pRender->Draw( state, rflags );
	}

	g_QglTable.m_pfn_qglPopMatrix();
}

// ISelect

bool CEntityEclassModel::TestRay( const ray_t *ray, vec_t *dist ) const {
	vec_t dist_start = *dist;
	vec_t dist_local = *dist;
	ray_t ray_local = *ray;

	if ( aabb_intersect_ray( &m_BBox, &ray_local, &dist_local ) ) {
		*dist = dist_local;
	}
	return *dist < dist_start;
}


//IEdit

void CEntityEclassModel::Translate( const vec3_t translation ){
	VectorIncrement( translation, m_translate );
	UpdateCachedData();
}

void CEntityEclassModel::Rotate( const vec3_t pivot, const vec3_t rotation ){
	m4x4_t rotation_matrix;

	m4x4_identity( rotation_matrix );
	m4x4_pivoted_rotate_by_vec3( rotation_matrix, rotation, eXYZ, pivot );
	m4x4_transform_point( rotation_matrix, m_translate );

	VectorIncrement( rotation, m_euler );

	UpdateCachedData();
}

void CEntityEclassModel::OnKeyValueChanged( entity_t *e, const char *key, const char* value ){
	if ( strcmp( key,"origin" ) == 0 ) {
		sscanf( value, "%f %f %f", &m_translate[0], &m_translate[1], &m_translate[2] );
		UpdateCachedData();
	}
	else if ( strcmp( key,"angle" ) == 0 ) {
		VectorSet( m_euler, 0, 0, (float) atof( value ) );
		UpdateCachedData();
	}
}

void CEntityEclassModel::SetEclass( const eclass_t* eclass ){
	m_eclass = eclass;
}

void CEntityEclassModel::SetName( const char *name ){
	if ( strcmp( m_name.c_str(), name ) == 0 ) {
		return;
	}

	if ( m_name.c_str()[0] != '\0'
		 && m_version.c_str()[0] != '\0' ) {
		GetModelCache()->DeleteByID( m_name.c_str(), m_version.c_str() );
	}

	m_model = NULL;
	m_name = name;

	if ( m_name.c_str()[0] != '\0' ) {
		const char* dot = strrchr( m_name.c_str(), '.' );
		if ( dot != NULL ) {
			m_version = ++dot;
			m_model = GetModelCache()->GetByID( m_name.c_str(), m_version.c_str() );
		}
	}

	UpdateCachedData();
}

//
// CEntityEclassModel
//

// private:

void CEntityEclassModel::UpdateCachedData(){
	aabb_t aabb_temp;

	aabb_clear( &aabb_temp );

	m4x4_identity( m_transform );
	m4x4_pivoted_transform_by_vec3( m_transform, m_translate, m_euler, eXYZ, m_scale, m_pivot );
	memcpy( m_inverse_transform, m_transform, sizeof( m4x4_t ) );
	if ( m4x4_invert( m_inverse_transform ) == 1 ) {
		Sys_FPrintf( SYS_ERR, "ERROR: Singular Matrix, cannot invert" );
	}

	if ( m_eclass ) {
		aabb_construct_for_vec3( &aabb_temp, m_eclass->mins, m_eclass->maxs );
	}
	else{
		VectorSet( aabb_temp.extents, 8, 8, 8 );
	}

	aabb_for_transformed_aabb( &m_BBox, &aabb_temp, m_transform );
}
