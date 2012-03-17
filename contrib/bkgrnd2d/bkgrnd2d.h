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
// bkgrnd2d Plugin
//
// Code by reyalP aka Reed Mideke
//
// Based on spritemodel source code by hydra
//

#include "plugin.h"

class CBackgroundImage {
private:
qtexture_t *m_tex;
VIEWTYPE m_vt;

// which components of a vec3_t correspond to x and y in the image
unsigned m_ix,m_iy;

public:
CBackgroundImage( VIEWTYPE vt );
//  ~CBackgroundImage();

float m_alpha;     // vertex alpha
bool m_bActive;

// x and y axis are in relation to the screen, not world, making rendering
// the same for each view type. Whoever sets them is responsible for
// shuffling.
// units are world units.
// TODO should be private
float m_xmin,m_ymin,m_xmax,m_ymax;

// load file, create new tex, cleanup old tex, set new tex
bool Load( const char *filename );
void Cleanup();     // free texture, free tex, set make tex NULL
bool SetExtentsMM();     // set extents by ET mapcoordsmaxs/mapcoordsmins
bool SetExtentsSel();     // set extents by selection
void Render();
bool Valid() { return ( m_tex && ( m_xmin != m_xmax ) && ( m_ymin != m_ymax ) ); }
};

class CBackgroundRender : public IGL2DWindow {
public:

CBackgroundRender();
virtual ~CBackgroundRender();

protected:
int refCount;

public:

// IGL2DWindow IGL3DWindow interface
void IncRef() { refCount++; }
void DecRef() {
	refCount--; if ( refCount <= 0 ) {
		delete this;
	}
}
void Draw2D( VIEWTYPE vt );
void Register();
};

extern CBackgroundImage backgroundXY,backgroundXZ,backgroundYZ;
extern CBackgroundRender render;
