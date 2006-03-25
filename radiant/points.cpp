/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
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

/*
The following source code is licensed by Id Software and subject to the terms of 
its LIMITED USE SOFTWARE LICENSE AGREEMENT, a copy of which is included with 
GtkRadiant. If you did not receive a LIMITED USE SOFTWARE LICENSE AGREEMENT, 
please contact Id Software immediately at info@idsoftware.com.
*/

#include "points.h"

#include "debugging/debugging.h"

#include "irender.h"
#include "igl.h"
#include "renderable.h"

#include "stream/stringstream.h"
#include "os/path.h"
#include "os/file.h"
#include "cmdlib.h"

#include "map.h"
#include "qe3.h"
#include "camwindow.h"
#include "xywindow.h"
#include "xmlstuff.h"
#include "mainframe.h"
#include "watchbsp.h"
#include "commands.h"


class CPointfile;
void Pointfile_Parse(CPointfile& pointfile);


class CPointfile : public ISAXHandler, public Renderable, public OpenGLRenderable
{
  enum
  {
    MAX_POINTFILE = 8192,
  };
  Vector3 s_pointvecs[MAX_POINTFILE];
  std::size_t s_num_points;
  int m_displaylist;
  static Shader* m_renderstate;
  StringOutputStream m_characters;
public:
  CPointfile()
  {
  }
  ~CPointfile()
  {
  }
  void Init();
  void PushPoint (const Vector3& v);
  void GenerateDisplayList();
  // SAX interface
  void Release()
  {
    // blank because not heap-allocated
  }
  void saxStartElement (message_info_t *ctx, const xmlChar *name, const xmlChar **attrs);
  void saxEndElement (message_info_t *ctx, const xmlChar *name);
  void saxCharacters (message_info_t *ctx, const xmlChar *ch, int len);
  const char* getName();

  typedef const Vector3* const_iterator;

  const_iterator begin() const
  {
    return &s_pointvecs[0];
  }
  const_iterator end() const
  {
    return &s_pointvecs[s_num_points];
  }

  bool shown() const
  {
    return m_displaylist != 0;
  }
  void show(bool show)
  {
    if(show && !shown())
    {
      Pointfile_Parse(*this);
	    GenerateDisplayList();
      SceneChangeNotify();
    }
    else if(!show && shown())
    {
	    glDeleteLists (m_displaylist, 1);
	    m_displaylist = 0;
      SceneChangeNotify();
    }
  }

  void render(RenderStateFlags state) const
  {
    glCallList(m_displaylist);
  }

  void renderSolid(Renderer& renderer, const VolumeTest& volume) const
  {
    if(shown())
    {
      renderer.SetState(m_renderstate, Renderer::eWireframeOnly);
      renderer.SetState(m_renderstate, Renderer::eFullMaterials);
      renderer.addRenderable(*this, g_matrix4_identity);
    }
  }
  void renderWireframe(Renderer& renderer, const VolumeTest& volume) const
  {
    renderSolid(renderer, volume);
  }

  static void constructStatic()
  {
    m_renderstate = GlobalShaderCache().capture("$POINTFILE");
  }

  static void destroyStatic()
  {
    GlobalShaderCache().release("$POINTFILE");
  }
};

Shader* CPointfile::m_renderstate = 0;

namespace
{
  CPointfile s_pointfile;
}

ISAXHandler& g_pointfile = s_pointfile;

static CPointfile::const_iterator s_check_point;

void CPointfile::Init()
{
  s_num_points = 0;
  m_displaylist = 0;
}

void CPointfile::PushPoint(const Vector3& v)
{
	if (s_num_points < MAX_POINTFILE)
	{
		s_pointvecs[s_num_points] = v;
		++s_num_points;
	}
}

// create the display list at the end
void CPointfile::GenerateDisplayList()
{
  m_displaylist = glGenLists(1);

  glNewList (m_displaylist,  GL_COMPILE);

  glBegin(GL_LINE_STRIP);
	for(std::size_t i=0;i<s_num_points;i++)
	  glVertex3fv (vector3_to_array(s_pointvecs[i]));
  glEnd();
  glLineWidth (1);
	
  glEndList();
}

// old (but still relevant) pointfile code -------------------------------------

void Pointfile_Delete (void)
{
  const char* mapname = Map_Name(g_map);
  StringOutputStream name(256);
  name << StringRange(mapname, path_get_filename_base_end(mapname)) << ".lin";
	file_remove(name.c_str());
}

// advance camera to next point
void Pointfile_Next (void)
{
  if(!s_pointfile.shown())
    return;

	if (s_check_point+2 == s_pointfile.end())
	{
		globalOutputStream() << "End of pointfile\n";
		return;
	}

  CPointfile::const_iterator i = ++s_check_point;


  CamWnd& camwnd = *g_pParentWnd->GetCamWnd();
	Camera_setOrigin(camwnd, *i);
	g_pParentWnd->GetXYWnd()->SetOrigin(*i);
  {
	  Vector3 dir(vector3_normalised(vector3_subtracted(*(++i), Camera_getOrigin(camwnd))));
    Vector3 angles(Camera_getAngles(camwnd));
	  angles[CAMERA_YAW] = static_cast<float>(radians_to_degrees(atan2(dir[1], dir[0])));
	  angles[CAMERA_PITCH] = static_cast<float>(radians_to_degrees(asin(dir[2])));
    Camera_setAngles(camwnd, angles);
  }
}

// advance camera to previous point
void Pointfile_Prev (void)
{
  if(!s_pointfile.shown())
    return;

	if (s_check_point == s_pointfile.begin())
	{
		globalOutputStream() << "Start of pointfile\n";
		return;
	}

	CPointfile::const_iterator i = --s_check_point;

  CamWnd& camwnd = *g_pParentWnd->GetCamWnd();
	Camera_setOrigin(camwnd, *i);
	g_pParentWnd->GetXYWnd()->SetOrigin(*i);
  {
	  Vector3 dir(vector3_normalised(vector3_subtracted(*(++i), Camera_getOrigin(camwnd))));
    Vector3 angles(Camera_getAngles(camwnd));
	  angles[CAMERA_YAW] = static_cast<float>(radians_to_degrees(atan2(dir[1], dir[0])));
	  angles[CAMERA_PITCH] = static_cast<float>(radians_to_degrees(asin(dir[2])));
    Camera_setAngles(camwnd, angles);
  }
}

int LoadFile (const char *filename, void **bufferptr)
{
  FILE *f;
  long len;

  f = fopen (filename, "rb");
  if (f == 0)
    return -1;

  fseek (f, 0, SEEK_END);
  len = ftell (f);
  rewind (f);

  *bufferptr = malloc (len+1);
  if (*bufferptr == 0)
    return -1;

  fread (*bufferptr, 1, len, f);
  fclose (f);

  // we need to end the buffer with a 0
  ((char*) (*bufferptr))[len] = 0;

  return len;
}

void Pointfile_Parse(CPointfile& pointfile)
{
	int		size;
	char	*data;
  char  *text;
  int   line = 1;

  const char* mapname = Map_Name(g_map);
  StringOutputStream name(256);
  name << StringRange(mapname, path_get_filename_base_end(mapname)) << ".lin";

	size = LoadFile (name.c_str(), (void**)&data);
  if (size == -1)
  {
    globalErrorStream() << "Pointfile " << name.c_str() << " not found\n";
		return;
  }

  // store a pointer
  text = data;

	globalOutputStream() << "Reading pointfile " << name.c_str() << "\n";

  pointfile.Init();

	while (*data)
	{
	  Vector3 v;
    if (sscanf(data,"%f %f %f", &v[0], &v[1], &v[2]) != 3)
    {
      globalOutputStream() << "Corrupt point file, line " << line << "\n";
			break;
    }

  	while (*data && *data != '\n')
    {
      if (*(data-1) == ' ' && *(data) == '-' && *(data+1) == ' ')
        break;
		  data++;
    }
    // deal with zhlt style point files.
    if (*data == '-')
    {
      if (sscanf(data,"- %f %f %f", &v[0], &v[1], &v[2]) != 3)
      {
        globalOutputStream() << "Corrupt point file, line " << line << "\n";
        break;
      }

      while (*data && *data != '\n')
		    data++;

    }
    while (*data == '\n')
    {
      data++; // skip the \n
      line++;
    }
		pointfile.PushPoint (v);
	}

  g_free(text);
}

void Pointfile_Clear()
{
  s_pointfile.show(false);
}

void Pointfile_Toggle()
{
  s_pointfile.show(!s_pointfile.shown());

  s_check_point = s_pointfile.begin();
}

void Pointfile_Construct()
{
  CPointfile::constructStatic();

  GlobalShaderCache().attachRenderable(s_pointfile);

  GlobalCommands_insert("TogglePointfile", FreeCaller<Pointfile_Toggle>());
  GlobalCommands_insert("NextLeakSpot", FreeCaller<Pointfile_Next>(), Accelerator('K', (GdkModifierType)(GDK_SHIFT_MASK|GDK_CONTROL_MASK)));
  GlobalCommands_insert("PrevLeakSpot", FreeCaller<Pointfile_Prev>(), Accelerator('L', (GdkModifierType)(GDK_SHIFT_MASK|GDK_CONTROL_MASK)));
}

void Pointfile_Destroy()
{
  GlobalShaderCache().detachRenderable(s_pointfile);

  CPointfile::destroyStatic();
}



// CPointfile implementation for SAX-specific stuff -------------------------------
void CPointfile::saxStartElement (message_info_t *ctx, const xmlChar *name, const xmlChar **attrs)
{
  if(string_equal(reinterpret_cast<const char*>(name), "polyline"))
  {
    Init();
    // there's a prefs setting to avoid stopping on leak
    if (!g_WatchBSP_LeakStop)
      ctx->stop_depth = 0;
  }
}

void CPointfile::saxEndElement (message_info_t *ctx, const xmlChar *name)
{
  if(string_equal(reinterpret_cast<const char*>(name), "polyline"))
  {
    // we are done
    GenerateDisplayList();
    SceneChangeNotify();
    s_check_point = begin();
  }
  else if(string_equal(reinterpret_cast<const char*>(name), "point"))
  {
    Vector3 v;  
    sscanf(m_characters.c_str(), "%f %f %f\n", &v[0], &v[1], &v[2]);
    PushPoint(v);
    m_characters.clear();
  }
}

// only "point" is expected to have characters around here
void CPointfile::saxCharacters (message_info_t *ctx, const xmlChar *ch, int len)
{
  m_characters.write(reinterpret_cast<const char*>(ch), len);
}

const char* CPointfile::getName()
{
  return "Map leaked";
}
