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

#ifndef _CPICOMODEL_H_
#define _CPICOMODEL_H_

#include "plugin.h"
#include "picomodel.h"

#include "gtkr_vector.h"

class CPicoParent
{
public:
virtual void UpdateShaders( void ) = 0;
};

class CModelManager;  // forward declaration

//typedef std::pair<Str, int> PicoModelKey;
typedef pair<Str, int> PicoModelKey;

class CPicoModel : public IRender, public ISelect
{
friend class CModelManager;
public:
CPicoModel( const PicoModelKey& key );
CPicoModel( const Str& name );
CPicoModel( const Str& name, const int frame );
CPicoModel( const char *name, const int frame );
~CPicoModel();
void load( const char *name, const int frame );

void IncRef(){
	++m_refcount;
}
void DecRef(){
	if ( --m_refcount == 0 ) {
		delete this;
	}
}

void AddParent( CPicoParent *parent );
void RemoveParent( CPicoParent *parent );

void Reload( void );

void Draw( int state, vector<IShader*> shaders, int rflags ) const;
//IRender
virtual void Draw( int state, int rflags ) const;
virtual const bool IsModelNotNull() const { return true; }
virtual const aabb_t *GetAABB() const { return &m_BBox; }

//ISelect
virtual bool TestRay( const ray_t *ray, vec_t *dist ) const;

int GetNumSurfaces( void );
char *GetShaderNameForSurface( const unsigned int surf );

private:
void AccumulateBBox();

char *m_name;
int m_frame;
picoModel_t *m_pModel;
unsigned int m_refcount;
aabb_t m_BBox;
GPtrArray *m_children;   // array of CPicoSurface
GPtrArray *m_parents;   // array of CPicoParent

GPtrArray* m_shaders;

bool m_bReloaded;   // managed by CModelManager
};

#endif // _CPICOMODEL_H_
