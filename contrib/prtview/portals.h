/*
   PrtView plugin for GtkRadiant
   Copyright (C) 2001 Geoffrey Dewan, Loki software and qeradiant.com

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

#ifndef _PORTALS_H_
#define _PORTALS_H_

class CBspPoint {
public:
float p[3];
};

class CBspPortal {
public:
CBspPortal();
~CBspPortal();

protected:

public:
CBspPoint center;
unsigned point_count;
CBspPoint *point;
CBspPoint *inner_point;
float fp_color_random[4];
float min[3];
float max[3];
float dist;
qboolean hint;

qboolean Build( char *def );
};

class CPortals {
public:

CPortals();
~CPortals();

protected:


public:

void Load();     // use filename in fn
void Purge();

void FixColors();

char fn[_MAX_PATH];

int zbuffer;
int polygons;
int lines;
qboolean show_3d;
qboolean aa_3d;
qboolean fog;
COLORREF color_3d;
float width_3d;      // in 8'ths
float fp_color_3d[4];
COLORREF color_fog;
float fp_color_fog[4];
float trans_3d;
float clip_range;
qboolean clip;

qboolean show_2d;
qboolean aa_2d;
COLORREF color_2d;
float width_2d;      // in 8'ths
float fp_color_2d[4];

CBspPortal *portal;
int *portal_sort;
qboolean hint_flags;
//	CBspNode *node;

unsigned int node_count;
unsigned int portal_count;
};

class CPortalsRender : public IGL2DWindow, public IGL3DWindow {
public:

CPortalsRender();
virtual ~CPortalsRender();

protected:

int refCount;
#ifdef _WIN32
CRITICAL_SECTION protect;
#endif

public:

// IGL2DWindow IGL3DWindow interface
void IncRef() { refCount++; }
void DecRef() {
	refCount--; if ( refCount <= 0 ) {
		delete this;
	}
}
void Draw2D( VIEWTYPE vt );
void Draw3D();
void Register();
};

// void Sys_Printf (char *text, ...);

extern CPortals portals;
extern CPortalsRender render;

#endif // _PORTALS_H_
