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

#ifndef _ENTITYMODEL_H_
#define _ENTITYMODEL_H_

#include "plugin.h"

/*! simulates misc_model entity behaviours for rendering/selection/editing */
class CEntityMiscModel : public IRender, public ISelect, public IEdit
{
	public:
	CEntityMiscModel( entity_t *e );
	virtual ~CEntityMiscModel ();

	void IncRef() { refCount++; }
	void DecRef() {
		if ( --refCount == 0 ) {
			delete this;
		}
	}

	// IRender
	void Draw( int state, int rflags ) const;
	const bool IsModelNotNull() const { return m_model && m_model->pRender; }
	const aabb_t *GetAABB() const { return &m_BBox; }

	// ISelect
	bool TestRay( const ray_t *ray, vec_t *dist ) const;
	//bool TestBox(const aabb_t aabb) const;

	// ITransform
	void Translate( const vec3_t translation );
	void Rotate( const vec3_t pivot, const vec3_t rotation );
	const vec_t *GetTranslation() const { return m_translate; }
	const vec_t *GetRotation() const { return m_euler; }
	void OnKeyValueChanged( entity_t *e, const char *key, const char* value );

	void SetName( const char *name );
	private:
	void BuildCacheRequestString( const char *name );
	/*! updates the AABB and transformation matrix */
	void UpdateCachedData();
	entity_interfaces_t *m_model;

	entity_t *m_entity;

	int refCount;
	string_t m_version;

	Str m_cachereq;

	/*! AABB in local space */
	aabb_t m_BBox;

	/*! worldspace-to-localspace translation */
	vec3_t m_translate;

	/*! worldspace-to-localspace euler rotation angles */
	vec3_t m_euler;

	/*! worldspace-to-localspace scale */
	vec3_t m_scale;

	/*! localspace origin, effectively rotation & scale pivot point */
	vec3_t m_pivot;

	/*! worldspace-to-localspace transform, generated from translate/euler/scale/pivot */
	m4x4_t m_transform;

	/*! localspace-to-worldspace transform */
	m4x4_t m_inverse_transform;
};

/*! simulates eclass-model entity behaviours for rendering/selection/editing */
class CEntityEclassModel : public IRender, public ISelect, public IEdit
{
	public:
	CEntityEclassModel();
	virtual ~CEntityEclassModel();

	void IncRef() { refCount++; }
	void DecRef() {
		if ( --refCount == 0 ) {
			delete this;
		}
	}

	// IRender
	void Draw( int state, int rflags ) const;
	const bool IsModelNotNull() const { return m_model && m_model->pRender; }
	const aabb_t *GetAABB() const { return &m_BBox; }

	// ISelect
	bool TestRay( const ray_t *ray, vec_t *dist ) const;
	//bool TestBox(const aabb_t aabb) const;

	// ITransform
	void Translate( const vec3_t translation );
	void Rotate( const vec3_t pivot, const vec3_t rotation );
	const vec_t *GetTranslation() const { return m_translate; }
	const vec_t *GetRotation() const { return m_euler; }
	void OnKeyValueChanged( entity_t *e, const char *key, const char* value );

	void SetName( const char *name );
	void SetEclass( const eclass_t* eclass );
	private:
	/*! updates the AABB and transformation matrix */
	void UpdateCachedData();
	entity_interfaces_t *m_model;

	int refCount;
	string_t m_name;
	string_t m_version;
	const eclass_t *m_eclass;

	/*! AABB in local space */
	aabb_t m_BBox;

	/*! worldspace-to-localspace translation */
	vec3_t m_translate;

	/*! worldspace-to-localspace euler rotation angles */
	vec3_t m_euler;

	/*! worldspace-to-localspace scale */
	vec3_t m_scale;

	/*! localspace origin, effectively rotation & scale pivot point */
	vec3_t m_pivot;

	/*! worldspace-to-localspace transform, generated from translate/euler/scale/pivot */
	m4x4_t m_transform;

	/*! localspace-to-worldspace transform */
	m4x4_t m_inverse_transform;
};

void pivot_draw( const vec3_t pivot );
void Entity_UpdateClass( entity_t *e, const char* value );

#endif /* _ENTITYMODEL_H_ */
