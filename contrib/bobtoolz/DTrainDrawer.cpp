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

#include "DTrainDrawer.h"

#include <list>
#include "str.h"

#include "DPoint.h"
#include "DPlane.h"
#include "DBrush.h"
#include "DEPair.h"
#include "DPatch.h"
#include "DEntity.h"

#include "misc.h"
#include "funchandlers.h"

#include "iglrender.h"
#include "ientity.h"
#include "math/matrix.h"

#include "dialogs/dialogs-gtk.h"

DTrainDrawer::DTrainDrawer() {
	m_bDisplay = false;

	BuildPaths();
  constructShaders();
  GlobalShaderCache().attachRenderable(*this);
}

DTrainDrawer::~DTrainDrawer(void) {
  GlobalShaderCache().detachRenderable(*this);
  destroyShaders();

	ClearPoints();
	ClearSplines();
}

void DTrainDrawer::ClearSplines() {
	for(std::list<splinePoint_t *>::const_iterator deadSpline = m_splineList.begin(); deadSpline != m_splineList.end(); deadSpline++) {
		(*deadSpline)->m_pointList.clear();
		(*deadSpline)->m_vertexList.clear();
		delete (*deadSpline);
	}

	m_splineList.clear();
}

void DTrainDrawer::ClearPoints() {
	for(std::list<controlPoint_t *>::const_iterator deadPoint = m_pointList.begin(); deadPoint != m_pointList.end(); deadPoint++) {
		delete *deadPoint;
	}

	m_pointList.clear();
}

void CalculateSpline_r(vec3_t* v, int count, vec3_t out, float tension) {
	vec3_t dist;

	if(count < 2) {
		return;
	}

	if(count == 2) {
		VectorSubtract( v[1], v[0], dist );
		VectorMA(v[0], tension, dist, out);
		return;
	}

	vec3_t* v2 = new vec3_t[count-1];

	for( int i = 0; i < count-1; i++ ) {
		VectorSubtract( v[i+1], v[i], dist );
		VectorMA(v[i], tension, dist, v2[i]);
	}

	CalculateSpline_r( v2, count-1, out, tension);

	delete[] v2;
}

void DTrainDrawer::render(RenderStateFlags state) const
{
	for(std::list<splinePoint_t* >::const_iterator sp = m_splineList.begin(); sp != m_splineList.end(); sp++) {
		splinePoint_t* pSP = (*sp);

		glBegin(GL_LINE_STRIP);
			for(std::list<DPoint >::const_iterator v = pSP->m_vertexList.begin(); v != pSP->m_vertexList.end(); v++) {
				glVertex3fv((*v)._pnt);
			}
		glEnd();

	}
}

const char* DTrainDrawer_state_wireframe = "$bobtoolz/traindrawer/wireframe";
const char* DTrainDrawer_state_solid = "$bobtoolz/traindrawer/solid";

void DTrainDrawer::constructShaders()
{
  OpenGLState state;
  GlobalOpenGLStateLibrary().getDefaultState(state);
  state.m_state = RENDER_COLOURWRITE|RENDER_DEPTHWRITE|RENDER_BLEND;
  state.m_sort = OpenGLState::eSortOverlayFirst;
  state.m_linewidth = 1;
  state.m_colour[0] = 1;
  state.m_colour[1] = 0;
  state.m_colour[2] = 0;
  state.m_colour[3] = 1;
  state.m_linewidth = 1;
  GlobalOpenGLStateLibrary().insert(DTrainDrawer_state_wireframe, state);

  state.m_colour[0] = 1;
  state.m_colour[1] = 1;
  state.m_colour[2] = 1;
  state.m_colour[3] = 1;
  state.m_linewidth = 2;
  GlobalOpenGLStateLibrary().insert(DTrainDrawer_state_solid, state);

  m_shader_wireframe = GlobalShaderCache().capture(DTrainDrawer_state_wireframe);
  m_shader_solid = GlobalShaderCache().capture(DTrainDrawer_state_solid);
}

void DTrainDrawer::destroyShaders()
{
  GlobalOpenGLStateLibrary().erase(DTrainDrawer_state_wireframe);
  GlobalOpenGLStateLibrary().erase(DTrainDrawer_state_solid);
  GlobalShaderCache().release(DTrainDrawer_state_wireframe);
  GlobalShaderCache().release(DTrainDrawer_state_solid);
}


void DTrainDrawer::renderSolid(Renderer& renderer, const VolumeTest& volume) const
{
	if(!m_bDisplay) {
		return;
	}

  renderer.SetState(m_shader_wireframe, Renderer::eWireframeOnly);
  renderer.SetState(m_shader_solid, Renderer::eFullMaterials);
  renderer.addRenderable(*this, g_matrix4_identity);
}
void DTrainDrawer::renderWireframe(Renderer& renderer, const VolumeTest& volume) const
{
  renderSolid(renderer, volume);
}

void AddSplineControl(const char* control, splinePoint_t* pSP) {
	controlPoint_t cp;
	strncpy(cp.strName,	control, 64);

	pSP->m_pointList.push_front(cp);
}

class EntityBuildPaths
{
	mutable DEntity e;
  DTrainDrawer& drawer;
public:
  EntityBuildPaths(DTrainDrawer& drawer) : drawer(drawer)
  {
  }
  void operator()(scene::Instance& instance) const
  {
		e.ClearEPairs();
		e.LoadEPairList(Node_getEntity(instance.path().top()));

		const char* classname = e.m_Classname.GetBuffer();
		const char* target;
		const char* control;
		const char* targetname;
		vec3_t		vOrigin;

		e.SpawnString("targetname", NULL, &targetname);
		e.SpawnVector("origin", "0 0 0", vOrigin);

		if(!strcmp(classname, "info_train_spline_main")) {
			if(!targetname) {
				globalOutputStream() << "info_train_spline_main with no targetname";
				return;
			}

			e.SpawnString("target", NULL, &target);

			if(!target) {
				drawer.AddControlPoint( targetname, vOrigin );
			} else {
				splinePoint_t* pSP = drawer.AddSplinePoint( targetname, target, vOrigin );

				e.SpawnString("control", NULL, &control);

				if(control) {
					AddSplineControl( control, pSP );

					for(int j = 2;; j++) {
						char buffer[16];
						sprintf(buffer, "control%i", j);

						e.SpawnString(buffer, NULL, &control);
						if(!control) {
							break;
						}
  				
						AddSplineControl( control, pSP );
					}
				}
			}
		} else if(!strcmp(classname, "info_train_spline_control")) {
			if(!targetname) {
				globalOutputStream() << "info_train_spline_control with no targetname";
				return;
			}

			drawer.AddControlPoint( targetname, vOrigin );
		}
  }
};

void DTrainDrawer::BuildPaths() {
  Scene_forEachEntity(EntityBuildPaths(*this));

  std::list<splinePoint_t* >::const_iterator sp;
	for(sp = m_splineList.begin(); sp != m_splineList.end(); sp++) {
		splinePoint_t* pSP = (*sp);

		controlPoint_t* pTarget = FindControlPoint( pSP->strTarget );

		if(!pTarget) {
			globalOutputStream() << "couldn't find target " << pSP->strTarget;
			return;
//			continue;
		}

		pSP->pTarget = pTarget;


		for(std::list<controlPoint_t >::iterator cp = pSP->m_pointList.begin(); cp != pSP->m_pointList.end(); cp++) {			
			controlPoint_t* pControl = FindControlPoint( (*cp).strName );
			if(!pControl) {
				globalOutputStream() << "couldn't find control " << (*cp).strName;
				return;
			}

			VectorCopy(pControl->vOrigin, (*cp).vOrigin);
		}
	}

	m_bDisplay = true;

	for(sp = m_splineList.begin(); sp != m_splineList.end(); sp++) {
		splinePoint_t* pSP = (*sp);
		DPoint out;

		if(!pSP->pTarget) {
			continue;
		}

    std::size_t count = pSP->m_pointList.size() + 2;
		vec3_t* v = new vec3_t[count];

		VectorCopy(pSP->point.vOrigin, v[0]);

		int i = 1;
		for(std::list<controlPoint_t>::reverse_iterator cp = pSP->m_pointList.rbegin(); cp != pSP->m_pointList.rend(); cp++) {
			VectorCopy((*cp).vOrigin, v[i]);
			i++;
		}
		VectorCopy(pSP->pTarget->vOrigin, v[i]);

		for (float tension = 0.0f; tension <= 1.f; tension += 0.01f) {
			CalculateSpline_r(v, static_cast<int>(count), out._pnt, tension);
			pSP->m_vertexList.push_front(out);
		}

		delete[] v;

		VectorCopy(pSP->pTarget->vOrigin, out._pnt);
		pSP->m_vertexList.push_front(out);
	}

  SceneChangeNotify();
}

void DTrainDrawer::AddControlPoint(const char* name, vec_t* origin)
{
	controlPoint_t* pCP = new controlPoint_t;

	strncpy(pCP->strName, name, 64);
	VectorCopy( origin, pCP->vOrigin );

	m_pointList.push_back( pCP );
}

splinePoint_t* DTrainDrawer::AddSplinePoint(const char* name, const char* target, vec_t* origin)
{
	splinePoint_t* pSP = new splinePoint_t;

	strncpy(pSP->point.strName, name,		64);
	strncpy(pSP->strTarget,		target,		64);
	VectorCopy( origin, pSP->point.vOrigin );
	m_splineList.push_back( pSP );

	return pSP;
}

controlPoint_t* DTrainDrawer::FindControlPoint(const char* name)
{
	for(std::list<controlPoint_t*>::const_iterator cp = m_pointList.begin(); cp != m_pointList.end(); cp++) {
		if(!strcmp(name, (*cp)->strName)) {
			return (*cp);
		}
	}

	for(std::list<splinePoint_t*>::const_iterator sp = m_splineList.begin(); sp != m_splineList.end(); sp++) {
		if(!strcmp(name, (*sp)->point.strName)) {
			return &((*sp)->point);
		}
	}

	return NULL;
}
