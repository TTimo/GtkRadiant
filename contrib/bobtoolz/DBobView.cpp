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

// BobView.cpp: implementation of the DBobView class.
//
//////////////////////////////////////////////////////////////////////

#include "DBobView.h"
//#include "misc.h"
#include "funchandlers.h"

#include <list>

#include "iglrender.h"
#include "qerplugin.h"
#include "str.h"
#include "math/matrix.h"

#include "DEntity.h"
#include "DEPair.h"
#include "misc.h"
#include "dialogs/dialogs-gtk.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DBobView::DBobView()
{
	nPathCount = 0;
	
	path = NULL;

	boundingShow = BOUNDS_APEX;

  constructShaders();
  GlobalShaderCache().attachRenderable(*this);
}

DBobView::~DBobView()
{
  GlobalShaderCache().detachRenderable(*this);
  destroyShaders();

  if(path)
		delete[] path;

	g_PathView = NULL;
}

//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

void DBobView::render(RenderStateFlags state) const
{
	glBegin(GL_LINE_STRIP);

	for(int i = 0; i < nPathCount; i++)
		glVertex3fv(path[i]);

	glEnd();
}

const char* DBobView_state_line = "$bobtoolz/bobview/line";
const char* DBobView_state_box = "$bobtoolz/bobview/box";

void DBobView::constructShaders()
{
  OpenGLState state;
  GlobalOpenGLStateLibrary().getDefaultState(state);
  state.m_state = RENDER_COLOURWRITE|RENDER_DEPTHWRITE|RENDER_BLEND|RENDER_LINESMOOTH;
  state.m_sort = OpenGLState::eSortOpaque;
  state.m_linewidth = 1;
  state.m_colour[0] = 1;
  state.m_colour[1] = 0;
  state.m_colour[2] = 0;
  state.m_colour[3] = 1;
  GlobalOpenGLStateLibrary().insert(DBobView_state_line, state);

  state.m_colour[0] = 0.25f;
  state.m_colour[1] = 0.75f;
  state.m_colour[2] = 0.75f;
  state.m_colour[3] = 1;
  GlobalOpenGLStateLibrary().insert(DBobView_state_box, state);

  m_shader_line = GlobalShaderCache().capture(DBobView_state_line);
  m_shader_box = GlobalShaderCache().capture(DBobView_state_box);
}

void DBobView::destroyShaders()
{
  GlobalOpenGLStateLibrary().erase(DBobView_state_line);
  GlobalOpenGLStateLibrary().erase(DBobView_state_box);
  GlobalShaderCache().release(DBobView_state_line);
  GlobalShaderCache().release(DBobView_state_box);
}

Matrix4 g_transform_box1 = matrix4_translation_for_vec3(Vector3(16.0f, 16.0f, 28.0f));
Matrix4 g_transform_box2 = matrix4_translation_for_vec3(Vector3(-16.0f, 16.0f, 28.0f));
Matrix4 g_transform_box3 = matrix4_translation_for_vec3(Vector3(16.0f, -16.0f, -28.0f));
Matrix4 g_transform_box4 = matrix4_translation_for_vec3(Vector3(-16.0f, -16.0f, -28.0f));

void DBobView::renderSolid(Renderer& renderer, const VolumeTest& volume) const
{
	if(!path)
		return;

  renderer.SetState(m_shader_line, eWireframeOnly);
  renderer.SetState(m_shader_line, eFullMaterials);
  renderer.addRenderable(*this, g_matrix4_identity);

	if(m_bShowExtra)
	{
    renderer.SetState(m_shader_box, eWireframeOnly);
    renderer.SetState(m_shader_box, eFullMaterials);
    renderer.addRenderable(*this, g_transform_box1);
    renderer.addRenderable(*this, g_transform_box2);
    renderer.addRenderable(*this, g_transform_box3);
    renderer.addRenderable(*this, g_transform_box4);
  }
}
void DBobView::renderWireframe(Renderer& renderer, const VolumeTest& volume) const
{
  renderSolid(renderer, volume);
}

void DBobView::SetPath(vec3_t *pPath)
{
	if(path)
		delete[] path;

	path = pPath;
}

#define LOCAL_GRAVITY -800.0f

bool DBobView::CalculateTrajectory(vec3_t start, vec3_t apex, float multiplier, int points, float varGravity)
{
	if(apex[2] <= start[2])
	{
		SetPath(NULL);
		return false;
	}
	// ----think q3a actually would allow these
	//scrub that, coz the plugin wont :]

	vec3_t dist, speed;
	VectorSubtract(apex, start, dist);

	vec_t speed_z = (float)sqrt(-2*LOCAL_GRAVITY*dist[2]);
	float flight_time = -speed_z/LOCAL_GRAVITY;


	VectorScale(dist, 1/flight_time, speed);
	speed[2] = speed_z;

//	Sys_Printf("Speed: (%.4f %.4f %.4f)\n", speed[0], speed[1], speed[2]);

	vec3_t* pPath = new vec3_t[points];

	float interval = multiplier*flight_time/points;
	for(int i = 0; i < points; i++)
	{
		float ltime = interval*i;

		VectorScale(speed, ltime, pPath[i]);
		VectorAdd(pPath[i], start, pPath[i]);

		// could do this all with vectors
		// vGrav = {0, 0, -800.0f}
		// VectorScale(vGrav, 0.5f*ltime*ltime, vAdd);
		// VectorScale(speed, ltime, pPath[i]);
		// _VectorAdd(pPath[i], start, pPath[i])
		// _VectorAdd(pPath[i], vAdd, pPath[i])

		pPath[i][2] = start[2] + (speed_z*ltime) + (varGravity*0.5f*ltime*ltime);
	}

	SetPath(pPath);
	return true;
}

void DBobView::Begin(const char* trigger, const char *target, float multiplier, int points, float varGravity, bool bNoUpdate, bool bShowExtra)
{
	strcpy(entTrigger, trigger);
	strcpy(entTarget, target);

	fMultiplier = multiplier;
	fVarGravity = varGravity;
	nPathCount = points;
	m_bShowExtra = bShowExtra;

	if(!UpdatePath())
	{
		globalErrorStream() << "Initialization Failure in DBobView::Begin";
		delete this;
	}
}

bool DBobView::UpdatePath()
{
	vec3_t start, apex;

	if(GetEntityCentre(entTrigger, start))
	{
		if(GetEntityCentre(entTarget, apex))
		{
			CalculateTrajectory(start, apex, fMultiplier, nPathCount, fVarGravity);
			return true;
		}
	}
	return false;
}

void DBobView_setEntity(Entity& entity, float multiplier, int points, float varGravity, bool bNoUpdate, bool bShowExtra)
{
	DEntity trigger;
	trigger.LoadEPairList(&entity);

	DEPair* trigger_ep = trigger.FindEPairByKey("targetname");

	if(trigger_ep)
	{
		if(!strcmp(trigger.m_Classname, "trigger_push"))
		{
			DEPair* target_ep = trigger.FindEPairByKey("target");
			if(target_ep)
			{
        scene::Path* entTarget = FindEntityFromTargetname(target_ep->value, NULL);
				if(entTarget)
				{
					if(g_PathView)
						delete g_PathView;
					g_PathView = new DBobView;

          Entity* target = Node_getEntity(entTarget->top());
          if(target != 0)
          {
            if(!bNoUpdate)
            {
              g_PathView->trigger = &entity;
              entity.attach(*g_PathView);
              g_PathView->target = target;
              target->attach(*g_PathView);
            }
					  g_PathView->Begin(trigger_ep->value, target_ep->value, multiplier, points, varGravity, bNoUpdate, bShowExtra);
          }
				}
				else
					DoMessageBox("trigger_push target could not be found.", "Error", eMB_OK);
			}
			else
				DoMessageBox("trigger_push has no target.", "Error", eMB_OK);
		}
		else
			DoMessageBox("You must select a 'trigger_push' entity.", "Error", eMB_OK);
	}	
	else
		DoMessageBox("Entity must have a targetname", "Error", eMB_OK);
}