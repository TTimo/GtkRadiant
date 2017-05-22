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

#ifndef _IMODEL_H_
#define _IMODEL_H_

#define MODEL_MAJOR "model"

/*!
   loads model from model file name specified,
   fills model struct with pointers to model interfaces

   name is a relative path, we'll use VFS to extract a basepath to add to get the absolute path.
 */
typedef void ( *PFN_LOADMODEL )( entity_interfaces_t *model, const char *name );

struct _QERPlugModelTable
{
	int m_nSize;
	PFN_LOADMODEL m_pfnLoadModel;
};


//forward declare entity_t
struct entity_s;
typedef struct entity_s entity_t;

// any module relying on imodel will need to link against the mathlib
#include "mathlib.h"

// state flags
#define DRAW_GL_FILL          0x0001
#define DRAW_GL_LIGHTING      0x0010
#define DRAW_GL_TEXTURE_2D    0x0100
#define DRAW_GL_BLEND         0x1000

// predefined state combinations
#define DRAW_GL_WIRE                    0x0000
#define DRAW_GL_FLAT                    0x0001
#define DRAW_GL_SOLID                   0x0011
#define DRAW_GL_TEXTURED            0x0111

// mode
#define DRAW_WIRE                           0
#define DRAW_SOLID                      1
#define DRAW_TEXTURED                   2

// render flags
#define DRAW_RF_NONE          0x0000
#define DRAW_RF_SEL_OUTLINE   0x0001
#define DRAW_RF_SEL_FILL      0x0010
#define DRAW_RF_XY            0x0011
#define DRAW_RF_CAM           0x0100

class IRender
{
public:
virtual ~IRender() { }
virtual void IncRef() = 0;   // increments the reference counter for this object
virtual void DecRef() = 0;   // decrements the reference counter for this object, deletes the object if reference count is zero
virtual void Draw( int state, int rflags ) const = 0; // render the object - state = the opengl state
virtual const bool IsModelNotNull() const = 0;
virtual const aabb_t *GetAABB() const = 0;
};

class ISelect
{
public:
virtual ~ISelect() { }
virtual void IncRef() = 0;     // increments the reference counter for this object
virtual void DecRef() = 0;     // decrements the reference counter for this object, deletes the object if reference count is zero
virtual bool TestRay( const ray_t *ray, vec_t *dist ) const = 0; // test ray intersection, return bool true if intersects, and store distance to closest point of intersection
//virtual bool TestBox(const aabb_t *aabb) const = 0; // test aabb intersection, return bool true if touching or intersecting
};

class IEdit
{
public:
virtual ~IEdit() { }
virtual void IncRef() = 0;     // increments the reference counter for this object
virtual void DecRef() = 0;     // decrements the reference counter for this object, deletes the object if reference count is zero
virtual void Translate( const vec3_t translation ) = 0;
virtual void Rotate( const vec3_t pivot, const vec3_t rotation ) = 0;
virtual const vec_t *GetTranslation() const = 0;
virtual const vec_t *GetRotation() const = 0;
virtual void OnKeyValueChanged( entity_t *e, const char *key, const char* value ) = 0;
};

#endif /* _IMODEL_H_ */
