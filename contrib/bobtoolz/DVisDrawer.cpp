/*
BobToolz plugin for GtkRadiant
Copyright (C) 2001 Gordon Biggans

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

// BobView.cpp: implementation of the DVisDrawer class.
//
//////////////////////////////////////////////////////////////////////

#include "DVisDrawer.h"

#include "iglrender.h"
#include "math/matrix.h"

#include <list>
#include "str.h"

#include "DPoint.h"
#include "DWinding.h"

#include "misc.h"
#include "funchandlers.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DVisDrawer::DVisDrawer()
{
	m_list = NULL;

  constructShaders();
  GlobalShaderCache().attachRenderable(*this);
}

DVisDrawer::~DVisDrawer()
{
  GlobalShaderCache().detachRenderable(*this);
  destroyShaders();

	g_VisView = NULL;
}

//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////
const char* g_state_solid = "$bobtoolz/visdrawer/solid";
const char* g_state_wireframe = "$bobtoolz/visdrawer/wireframe";

void DVisDrawer::constructShaders()
{
  OpenGLState state;
  GlobalOpenGLStateLibrary().getDefaultState(state);
  state.m_state = RENDER_COLOURWRITE|RENDER_DEPTHWRITE|RENDER_COLOURCHANGE;
  state.m_linewidth = 1;

  GlobalOpenGLStateLibrary().insert(g_state_wireframe, state);

  GlobalOpenGLStateLibrary().getDefaultState(state);
  state.m_state = RENDER_FILL|RENDER_BLEND|RENDER_COLOURWRITE|RENDER_COLOURCHANGE|RENDER_SMOOTH|RENDER_DEPTHWRITE;

  GlobalOpenGLStateLibrary().insert(g_state_solid, state);

  m_shader_solid = GlobalShaderCache().capture(g_state_solid);
  m_shader_wireframe = GlobalShaderCache().capture(g_state_wireframe);
}

void DVisDrawer::destroyShaders()
{
  GlobalShaderCache().release(g_state_solid);
  GlobalShaderCache().release(g_state_wireframe);
  GlobalOpenGLStateLibrary().erase(g_state_solid);
  GlobalOpenGLStateLibrary().erase(g_state_wireframe);
}

void DVisDrawer::render(RenderStateFlags state) const
{
	//bleh
	std::list<DWinding *>::const_iterator l=m_list->begin();

	for(; l != m_list->end(); l++)
	{
		DWinding* w = *l;

		glColor4f(w->clr[0], w->clr[1], w->clr[2], 0.5f);

		glBegin(GL_POLYGON);
		for(int i = 0; i < w->numpoints; i++) {
			glVertex3f((w->p[i])[0], (w->p[i])[1], (w->p[i])[2]);
		}
		glEnd();
	}
}

void DVisDrawer::renderWireframe(Renderer& renderer, const VolumeTest& volume) const
{
	if(!m_list)
		return;

  renderer.SetState(m_shader_wireframe, Renderer::eWireframeOnly);

  renderer.addRenderable(*this, g_matrix4_identity);
}

void DVisDrawer::renderSolid(Renderer& renderer, const VolumeTest& volume) const
{
	if(!m_list)
		return;

  renderer.SetState(m_shader_solid, Renderer::eWireframeOnly);
  renderer.SetState(m_shader_solid, Renderer::eFullMaterials);

  renderer.addRenderable(*this, g_matrix4_identity);
}

void DVisDrawer::SetList(std::list<DWinding*> *pointList)
{
	if(m_list)
		ClearPoints();

	m_list = pointList;
}

void DVisDrawer::ClearPoints()
{
  std::list<DWinding *>::const_iterator deadPoint=m_list->begin();
	for(; deadPoint!=m_list->end(); deadPoint++)
		delete *deadPoint;
	m_list->clear();
}
