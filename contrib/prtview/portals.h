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

#include <glib.h>
#include "irender.h"
#include "renderable.h"
#include "math/vector.h"


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
	bool hint;

	bool Build(char *def);
};

#define PRTVIEW_PATH_MAX 260
typedef guint32 PackedColour;
#define RGB(r, g, b) ((guint32)(((guint8) (r) | ((guint16) (g) << 8))|(((guint32) (guint8) (b)) << 16)))
#define GetRValue(rgb)      ((guint8)(rgb))
#define GetGValue(rgb)      ((guint8)(((guint16)(rgb)) >> 8))
#define GetBValue(rgb)      ((guint8)((rgb)>>16))


class CPortals {
public:

  CPortals();
  ~CPortals();

protected:


public:

	void Load(); // use filename in fn
	void Purge();

	void FixColors();

	char fn[PRTVIEW_PATH_MAX];

	int zbuffer;
	int polygons;
	int lines;
	bool show_3d;
	bool aa_3d;
	bool fog;
	PackedColour color_3d;
	float width_3d;  // in 8'ths
	float fp_color_3d[4];
	PackedColour color_fog;
	float fp_color_fog[4];
	float trans_3d;
	float clip_range;
	bool clip;

	bool show_2d;
	bool aa_2d;
	PackedColour color_2d;
	float width_2d;  // in 8'ths
	float fp_color_2d[4];

	CBspPortal *portal;
	int *portal_sort;
	bool hint_flags;
//	CBspNode *node;

	unsigned int node_count;
	unsigned int portal_count;
};

class CubicClipVolume
{
public:
  Vector3 cam, min, max;
};

class CPortalsDrawSolid : public OpenGLRenderable
{
public:
  mutable CubicClipVolume clip;
  void render(RenderStateFlags state) const;
};

class CPortalsDrawSolidOutline : public OpenGLRenderable
{
public:
  mutable CubicClipVolume clip;
  void render(RenderStateFlags state) const;
};

class CPortalsDrawWireframe : public OpenGLRenderable
{
public:
  void render(RenderStateFlags state) const;
};

class CPortalsRender : public Renderable
{
public:
  CPortalsDrawSolid m_drawSolid;
  CPortalsDrawSolidOutline m_drawSolidOutline;
  CPortalsDrawWireframe m_drawWireframe;

  void renderSolid(Renderer& renderer, const VolumeTest& volume) const;
  void renderWireframe(Renderer& renderer, const VolumeTest& volume) const;
};

extern CPortals portals;
extern CPortalsRender render;

void Portals_constructShaders();
void Portals_destroyShaders();

void Portals_shadersChanged();


#endif // _PORTALS_H_
