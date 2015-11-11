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

//
// Sprite Model Plugin
//
// Code by Hydra aka Dominic Clifton
//
// Based on MD3Model source code by SPoG
//

#include "plugin.h"

/*! i guess a description should go here... */
class CSpriteModel : public IRender //, public ISelect
{
public:
CSpriteModel();
~CSpriteModel();

void IncRef() { refCount++; }
void DecRef() {
	if ( --refCount == 0 ) {
		delete this;
	}
}

//IRender
void Draw( int state, int rflags ) const;
const bool IsModelNotNull() const { return true; }
const aabb_t *GetAABB() const { return &m_BBox; }

//ISelect
//bool TestRay (const ray_t *ray, vec_t *dist) const;

void Construct( IShader *pShader );

protected:
IShader *m_pShader;

private:
int refCount;
aabb_t m_BBox;
};
