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

#include "portals.h"
#include <string.h>
#include <stdlib.h>
#ifndef __APPLE__
#include <search.h>
#endif
#include <stdio.h>

#include "iglrender.h"
#include "cullable.h"

#include "prtview.h"

#define LINE_BUF 1000

CPortals portals;
CPortalsRender render;

int compare( const void *arg1, const void *arg2 )
{

	if(portals.portal[*((int *)arg1)].dist > portals.portal[*((int *)arg2)].dist)
		return -1;
	else if(portals.portal[*((int *)arg1)].dist < portals.portal[*((int *)arg2)].dist)
		return 1;

	return 0;
}


CBspPortal::CBspPortal()
{
	memset(this, 0, sizeof(CBspPortal));
}

CBspPortal::~CBspPortal()
{
	delete[] point;
	delete[] inner_point;
}

bool CBspPortal::Build(char *def)
{
	char *c = def;
	unsigned int n;
	int dummy1, dummy2;
	int res_cnt, i;

	if(portals.hint_flags)
	{
		res_cnt = sscanf(def, "%u %d %d %d", &point_count, &dummy1, &dummy2, (int *)&hint);
	}
	else
	{
		sscanf(def, "%u", &point_count);
		hint = false;
	}

	if(point_count < 3 || (portals.hint_flags && res_cnt < 4))
		return false;

	point = new CBspPoint[point_count];
	inner_point = new CBspPoint[point_count];

	for(n = 0; n < point_count; n++)
	{
		for(; *c != 0 && *c != '('; c++);

		if(*c == 0)
			return false;

		c++;

		sscanf(c, "%f %f %f", point[n].p, point[n].p+1, point[n].p+2);

		center.p[0] += point[n].p[0];
		center.p[1] += point[n].p[1];
		center.p[2] += point[n].p[2];

		if(n == 0)
		{
			for(i = 0; i < 3; i++)
			{
				min[i] = point[n].p[i];
				max[i] = point[n].p[i];
			}
		}
		else
		{
			for(i = 0; i < 3; i++)
			{
				if(min[i] > point[n].p[i])
					min[i] = point[n].p[i];
				if(max[i] < point[n].p[i])
					max[i] = point[n].p[i];
			}
		}
	}

	center.p[0] /= (float)point_count;
	center.p[1] /= (float)point_count;
	center.p[2] /= (float)point_count;

	for(n = 0; n < point_count; n++)
	{
		inner_point[n].p[0] = (0.01f * center.p[0]) + (0.99f * point[n].p[0]);
		inner_point[n].p[1] = (0.01f * center.p[1]) + (0.99f * point[n].p[1]);
		inner_point[n].p[2] = (0.01f * center.p[2]) + (0.99f * point[n].p[2]);
	}

	fp_color_random[0] = (float)(rand() & 0xff) / 255.0f;
	fp_color_random[1] = (float)(rand() & 0xff) / 255.0f;
	fp_color_random[2] = (float)(rand() & 0xff) / 255.0f;
	fp_color_random[3] = 1.0f;

	return true;
}

CPortals::CPortals()
{
	memset(this, 0, sizeof(CPortals));
}

CPortals::~CPortals()
{
	Purge();
}

void CPortals::Purge()
{
	delete[] portal;
	delete[] portal_sort;
	portal = NULL;
	portal_sort = NULL;
	portal_count = 0;

	/*
	delete[] node;
	node = NULL;
	node_count = 0;
	*/
}

void CPortals::Load()
{
	char buf[LINE_BUF+1];

	memset(buf, 0, LINE_BUF + 1);
	
	Purge();

	globalOutputStream() << MSG_PREFIX "Loading portal file " << fn << ".\n";

	FILE *in;

	in = fopen(fn, "rt");

	if(in == NULL)
	{
		globalOutputStream() << "  ERROR - could not open file.\n";

		return;
	}

	if(!fgets(buf, LINE_BUF, in))
	{
		fclose(in);

		globalOutputStream() << "  ERROR - File ended prematurely.\n";

		return;
	}

	if(strncmp("PRT1", buf, 4) != 0)
	{
		fclose(in);

		globalOutputStream() << "  ERROR - File header indicates wrong file type (should be \"PRT1\").\n";

		return;
	}

	if(!fgets(buf, LINE_BUF, in))
	{
		fclose(in);

		globalOutputStream() << "  ERROR - File ended prematurely.\n";

		return;
	}

	sscanf(buf, "%u", &node_count);
/*
	if(node_count > 0xFFFF)
	{
		fclose(in);

		node_count = 0;

		globalOutputStream() << "  ERROR - Extreme number of nodes, aborting.\n";

		return;
	}
	*/

	if(!fgets(buf, LINE_BUF, in))
	{
		fclose(in);

		node_count = 0;

		globalOutputStream() << "  ERROR - File ended prematurely.\n";

		return;
	}

	sscanf(buf, "%u", &portal_count);

	if(portal_count > 0xFFFF)
	{
		fclose(in);

		portal_count = 0;
		node_count = 0;

		globalOutputStream() << "  ERROR - Extreme number of portals, aborting.\n";

		return;
	}

	if(portal_count < 0)
	{
		fclose(in);

		portal_count = 0;
		node_count = 0;

		globalOutputStream() << "  ERROR - number of portals equals 0, aborting.\n";

		return;
	}

//	node = new CBspNode[node_count];
	portal = new CBspPortal[portal_count];
	portal_sort = new int[portal_count];

	unsigned int n;
	bool first = true;
	unsigned test_vals_1, test_vals_2;

	hint_flags = false;

	for(n = 0; n < portal_count; )
	{
		if(!fgets(buf, LINE_BUF, in))
		{
			fclose(in);

			Purge();

			globalOutputStream() << "  ERROR - Could not find information for portal number " << n + 1 << " of " << portal_count << ".\n";

			return;
		}

		if(!portal[n].Build(buf))
		{
			if(first && sscanf(buf, "%d %d", &test_vals_1, &test_vals_2) == 1) // skip additional counts of later data, not needed
			{
				// We can count on hint flags being in the file
				hint_flags = true;
				continue;
			}

			first = false;

			fclose(in);

			Purge();

			globalOutputStream() << "  ERROR - Information for portal number " << n + 1 << " of " << portal_count << " is not formatted correctly.\n";

			return;
		}

		n++;
	}

	fclose(in);

	globalOutputStream() << "  " << node_count << " portals read in.\n";
}

#include "math/matrix.h"

const char* g_state_solid = "$plugins/prtview/solid";
const char* g_state_solid_outline = "$plugins/prtview/solid_outline";
const char* g_state_wireframe = "$plugins/prtview/wireframe";
Shader* g_shader_solid = 0;
Shader* g_shader_solid_outline = 0;
Shader* g_shader_wireframe = 0;

void Portals_constructShaders()
{
  OpenGLState state;
  GlobalOpenGLStateLibrary().getDefaultState(state);
  state.m_state = RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
  state.m_sort = OpenGLState::eSortOverlayFirst;
  state.m_linewidth = portals.width_2d * 0.5f;
  state.m_colour[0] = portals.fp_color_2d[0];
  state.m_colour[1] = portals.fp_color_2d[1];
  state.m_colour[2] = portals.fp_color_2d[2];
  state.m_colour[3] = portals.fp_color_2d[3];
  if(portals.aa_2d)
  {
    state.m_state |= RENDER_BLEND|RENDER_LINESMOOTH;
  }
  GlobalOpenGLStateLibrary().insert(g_state_wireframe, state);

  GlobalOpenGLStateLibrary().getDefaultState(state);
  state.m_state = RENDER_FILL|RENDER_BLEND|RENDER_COLOURWRITE|RENDER_COLOURCHANGE|RENDER_SMOOTH|RENDER_POLYGONSMOOTH;

	if(portals.aa_3d)
  {
		state.m_state |= RENDER_POLYGONSMOOTH;
  }

	switch(portals.zbuffer)
	{
	case 1:
		state.m_state |= RENDER_DEPTHTEST;
		break;
	case 2:
		break;
	default:
		state.m_state |= RENDER_DEPTHTEST;
		state.m_state |= RENDER_DEPTHWRITE;
	}

	if(portals.fog)
	{
		state.m_state |= RENDER_FOG;

		state.m_fog.mode = GL_EXP;
		state.m_fog.density = 0.001f;
		state.m_fog.start = 10.0f;
		state.m_fog.end = 10000.0f;
		state.m_fog.index = 0;
		state.m_fog.colour[0] = portals.fp_color_fog[0];
		state.m_fog.colour[1] = portals.fp_color_fog[1];
		state.m_fog.colour[2] = portals.fp_color_fog[2];
		state.m_fog.colour[3] = portals.fp_color_fog[3];
	}

  GlobalOpenGLStateLibrary().insert(g_state_solid, state);

  GlobalOpenGLStateLibrary().getDefaultState(state);
  state.m_state = RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
  state.m_sort = OpenGLState::eSortOverlayFirst;
	state.m_linewidth = portals.width_3d * 0.5f;
  state.m_colour[0] = portals.fp_color_3d[0];
  state.m_colour[1] = portals.fp_color_3d[1];
  state.m_colour[2] = portals.fp_color_3d[2];
  state.m_colour[3] = portals.fp_color_3d[3];

	if(portals.aa_3d)
  {
		state.m_state |= RENDER_LINESMOOTH;
  }

	switch(portals.zbuffer)
	{
	case 1:
		state.m_state |= RENDER_DEPTHTEST;
		break;
	case 2:
		break;
	default:
		state.m_state |= RENDER_DEPTHTEST;
		state.m_state |= RENDER_DEPTHWRITE;
	}

	if(portals.fog)
	{
		state.m_state |= RENDER_FOG;

		state.m_fog.mode = GL_EXP;
		state.m_fog.density = 0.001f;
		state.m_fog.start = 10.0f;
		state.m_fog.end = 10000.0f;
		state.m_fog.index = 0;
		state.m_fog.colour[0] = portals.fp_color_fog[0];
		state.m_fog.colour[1] = portals.fp_color_fog[1];
		state.m_fog.colour[2] = portals.fp_color_fog[2];
		state.m_fog.colour[3] = portals.fp_color_fog[3];
	}

  GlobalOpenGLStateLibrary().insert(g_state_solid_outline, state);

  g_shader_solid = GlobalShaderCache().capture(g_state_solid);
  g_shader_solid_outline = GlobalShaderCache().capture(g_state_solid_outline);
  g_shader_wireframe = GlobalShaderCache().capture(g_state_wireframe);
}

void Portals_destroyShaders()
{
  GlobalShaderCache().release(g_state_solid);
  GlobalShaderCache().release(g_state_solid_outline);
  GlobalShaderCache().release(g_state_wireframe);
  GlobalOpenGLStateLibrary().erase(g_state_solid);
  GlobalOpenGLStateLibrary().erase(g_state_solid_outline);
  GlobalOpenGLStateLibrary().erase(g_state_wireframe);
}

void Portals_shadersChanged()
{
  Portals_destroyShaders();
  portals.FixColors();
  Portals_constructShaders();
}

void CPortals::FixColors()
{
	fp_color_2d[0] = (float)GetRValue(color_2d) / 255.0f;
	fp_color_2d[1] = (float)GetGValue(color_2d) / 255.0f;
	fp_color_2d[2] = (float)GetBValue(color_2d) / 255.0f;
	fp_color_2d[3] = 1.0f;

	fp_color_3d[0] = (float)GetRValue(color_3d) / 255.0f;
	fp_color_3d[1] = (float)GetGValue(color_3d) / 255.0f;
	fp_color_3d[2] = (float)GetBValue(color_3d) / 255.0f;
	fp_color_3d[3] = 1.0f;

	fp_color_fog[0] = 0.0f;//(float)GetRValue(color_fog) / 255.0f;
	fp_color_fog[1] = 0.0f;//(float)GetGValue(color_fog) / 255.0f;
	fp_color_fog[2] = 0.0f;//(float)GetBValue(color_fog) / 255.0f;
	fp_color_fog[3] = 1.0f;
}

void CPortalsRender::renderWireframe(Renderer& renderer, const VolumeTest& volume) const
{
	if(!portals.show_2d || portals.portal_count < 1)
		return;

  renderer.SetState(g_shader_wireframe, Renderer::eWireframeOnly);

  renderer.addRenderable(m_drawWireframe, g_matrix4_identity);
}

void CPortalsDrawWireframe::render(RenderStateFlags state) const
{
	unsigned int n, p;

	for(n = 0; n < portals.portal_count; n++)
	{
		glBegin(GL_LINE_LOOP);

		for(p = 0; p < portals.portal[n].point_count; p++)
			glVertex3fv(portals.portal[n].point[p].p);

		glEnd();
	}
}

CubicClipVolume calculateCubicClipVolume(const Matrix4& viewproj)
{
  CubicClipVolume clip;
  clip.cam = vector4_projected(
    matrix4_transformed_vector4(
      matrix4_full_inverse(viewproj),
      Vector4(0, 0, -1, 1)
    )
  );
	clip.min[0] = clip.cam[0] + (portals.clip_range * 64.0f);
	clip.min[1] = clip.cam[1] + (portals.clip_range * 64.0f);
	clip.min[2] = clip.cam[2] + (portals.clip_range * 64.0f);
	clip.max[0] = clip.cam[0] - (portals.clip_range * 64.0f);
	clip.max[1] = clip.cam[1] - (portals.clip_range * 64.0f);
	clip.max[2] = clip.cam[2] - (portals.clip_range * 64.0f);
  return clip;
}

void CPortalsRender::renderSolid(Renderer& renderer, const VolumeTest& volume) const
{
	if(!portals.show_3d || portals.portal_count < 1)
		return;

  CubicClipVolume clip = calculateCubicClipVolume(matrix4_multiplied_by_matrix4(volume.GetProjection(), volume.GetModelview()));

	if(portals.polygons)
	{
    renderer.SetState(g_shader_solid, Renderer::eWireframeOnly);
    renderer.SetState(g_shader_solid, Renderer::eFullMaterials);

    m_drawSolid.clip = clip;
    renderer.addRenderable(m_drawSolid, g_matrix4_identity);
  }

	if(portals.lines)
	{
    renderer.SetState(g_shader_solid_outline, Renderer::eWireframeOnly);
    renderer.SetState(g_shader_solid_outline, Renderer::eFullMaterials);

    m_drawSolidOutline.clip = clip;
    renderer.addRenderable(m_drawSolidOutline, g_matrix4_identity);
  }
}

void CPortalsDrawSolid::render(RenderStateFlags state) const
{
	float trans = (100.0f - portals.trans_3d) / 100.0f;

	unsigned int n, p;

	if(portals.zbuffer != 0)
	{
		float d;

		for(n = 0; n < portals.portal_count; n++)
		{
			d = (float)clip.cam[0] - portals.portal[n].center.p[0];
			portals.portal[n].dist = d * d;

			d = (float)clip.cam[1] - portals.portal[n].center.p[1];
			portals.portal[n].dist += d * d;

			d = (float)clip.cam[2] - portals.portal[n].center.p[2];
			portals.portal[n].dist += d * d;

			portals.portal_sort[n] = n;
		}

		qsort(portals.portal_sort, portals.portal_count, 4, compare);
				
		for(n = 0; n < portals.portal_count; n++)
		{
			if(portals.polygons == 2 && !portals.portal[portals.portal_sort[n]].hint)
				continue;

			if(portals.clip)
			{
				if(clip.min[0] < portals.portal[portals.portal_sort[n]].min[0])
					continue;
				else if(clip.min[1] < portals.portal[portals.portal_sort[n]].min[1])
					continue;
				else if(clip.min[2] < portals.portal[portals.portal_sort[n]].min[2])
					continue;
				else if(clip.max[0] > portals.portal[portals.portal_sort[n]].max[0])
					continue;
				else if(clip.max[1] > portals.portal[portals.portal_sort[n]].max[1])
					continue;
				else if(clip.max[2] > portals.portal[portals.portal_sort[n]].max[2])
					continue;
			}

			glColor4f(portals.portal[portals.portal_sort[n]].fp_color_random[0], portals.portal[portals.portal_sort[n]].fp_color_random[1],
				portals.portal[portals.portal_sort[n]].fp_color_random[2], trans);

			glBegin(GL_POLYGON);

				for(p = 0; p < portals.portal[portals.portal_sort[n]].point_count; p++)
					glVertex3fv(portals.portal[portals.portal_sort[n]].point[p].p);

			glEnd();
		}
	}
	else
	{
		for(n = 0; n < portals.portal_count; n++)
		{
			if(portals.polygons == 2 && !portals.portal[n].hint)
				continue;

			if(portals.clip)
			{
				if(clip.min[0] < portals.portal[n].min[0])
					continue;
				else if(clip.min[1] < portals.portal[n].min[1])
					continue;
				else if(clip.min[2] < portals.portal[n].min[2])
					continue;
				else if(clip.max[0] > portals.portal[n].max[0])
					continue;
				else if(clip.max[1] > portals.portal[n].max[1])
					continue;
				else if(clip.max[2] > portals.portal[n].max[2])
					continue;
			}

			glColor4f(portals.portal[n].fp_color_random[0], portals.portal[n].fp_color_random[1],
				portals.portal[n].fp_color_random[2], trans);

			glBegin(GL_POLYGON);

				for(p = 0; p < portals.portal[n].point_count; p++)
					glVertex3fv(portals.portal[n].point[p].p);

			glEnd();
		}
	}
}

void CPortalsDrawSolidOutline::render(RenderStateFlags state) const
{
  for(int n = 0; n < portals.portal_count; n++)
	{
		if(portals.lines == 2 && !portals.portal[n].hint)
			continue;

		if(portals.clip)
		{
			if(clip.min[0] < portals.portal[n].min[0])
				continue;
			else if(clip.min[1] < portals.portal[n].min[1])
				continue;
			else if(clip.min[2] < portals.portal[n].min[2])
				continue;
			else if(clip.max[0] > portals.portal[n].max[0])
				continue;
			else if(clip.max[1] > portals.portal[n].max[1])
				continue;
			else if(clip.max[2] > portals.portal[n].max[2])
				continue;
		}

		glBegin(GL_LINE_LOOP);

		for(int p = 0; p < portals.portal[n].point_count; p++)
			glVertex3fv(portals.portal[n].inner_point[p].p);

		glEnd();
	}
}
