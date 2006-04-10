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


#include "prtview.h"
#include <stdio.h>
#include <stdlib.h>

#include "profile/profile.h"

#include "qerplugin.h"
#include "iscenegraph.h"
#include "iglrender.h"
#include "iplugin.h"
#include "stream/stringstream.h"

#include "portals.h"
#include "AboutDialog.h"
#include "ConfigDialog.h"
#include "LoadPortalFileDialog.h"

#define Q3R_CMD_SPLITTER "-"
#define Q3R_CMD_ABOUT "About Portal Viewer"
#define Q3R_CMD_LOAD "Load .prt file"
#define Q3R_CMD_RELEASE "Unload .prt file"
#define Q3R_CMD_SHOW_3D "Toggle portals (3D)"
#define Q3R_CMD_SHOW_2D "Toggle portals (2D)"
#define Q3R_CMD_OPTIONS "Configure Portal Viewer"

CopiedString INIfn;

/////////////////////////////////////////////////////////////////////////////
// CPrtViewApp construction

#define RENDER_2D "Render2D"
#define WIDTH_2D "Width2D"
#define AA_2D "AntiAlias2D"
#define COLOR_2D "Color2D"

#define RENDER_3D "Render3D"
#define WIDTH_3D "Width3D"
#define AA_3D "AntiAlias3D"
#define COLOR_3D "Color3D"
#define COLOR_FOG "ColorFog"
#define FOG "Fog"
#define ZBUFFER "ZBuffer"
#define POLYGON "Polygons"
#define LINE "Lines"
#define TRANS_3D "Transparency"
#define CLIP_RANGE "ClipRange"
#define CLIP "Clip"


void PrtView_construct()
{
  StringOutputStream tmp(64);
  tmp << GlobalRadiant().getSettingsPath() << "prtview.ini";
  INIfn = tmp.c_str();

  portals.show_2d = INIGetInt(RENDER_2D, FALSE) ? true : false;
  portals.aa_2d = INIGetInt(AA_2D, FALSE) ? true : false;
  portals.width_2d = (float)INIGetInt(WIDTH_2D, 10);
  portals.color_2d = (PackedColour)INIGetInt(COLOR_2D, RGB(0, 0, 255)) & 0xFFFFFF;

  if (portals.width_2d > 40.0f)
    portals.width_2d = 40.0f;
  else if (portals.width_2d < 2.0f)
    portals.width_2d = 2.0f;

  portals.show_3d = INIGetInt(RENDER_3D, TRUE) ? true : false;

  portals.zbuffer = INIGetInt(ZBUFFER, 1);
  portals.fog = INIGetInt(FOG, FALSE) ? true : false;
  portals.polygons = INIGetInt(POLYGON, TRUE);
  portals.lines = INIGetInt(LINE, TRUE);
  portals.aa_3d = INIGetInt(AA_3D, FALSE) ? true : false;
  portals.width_3d = (float)INIGetInt(WIDTH_3D, 4);
  portals.color_3d = (PackedColour)INIGetInt(COLOR_3D, RGB(255, 255, 0)) & 0xFFFFFF;
  portals.color_fog = (PackedColour)INIGetInt(COLOR_FOG, RGB(127, 127, 127)) & 0xFFFFFF;
  portals.trans_3d = (float)INIGetInt(TRANS_3D, 50);
  portals.clip = INIGetInt(CLIP, FALSE) ? true : false;
  portals.clip_range = (float)INIGetInt(CLIP_RANGE, 16);

  if (portals.clip_range < 1)
    portals.clip_range = 1;
  else if (portals.clip_range > 128)
    portals.clip_range = 128;

  if (portals.zbuffer < 0)
    portals.zbuffer = 0;
  else if (portals.zbuffer > 2)
    portals.zbuffer = 0;

  if (portals.width_3d > 40.0f)
    portals.width_3d = 40.0f;
  else if (portals.width_3d < 2.0f)
    portals.width_3d = 2.0f;

  if (portals.trans_3d > 100.0f)
    portals.trans_3d = 100.0f;
  else if (portals.trans_3d < 0.0f)
    portals.trans_3d = 0.0f;

  SaveConfig();

  portals.FixColors();
  
  Portals_constructShaders();
  GlobalShaderCache().attachRenderable(render);
}

void PrtView_destroy()
{
  GlobalShaderCache().detachRenderable(render);
  Portals_destroyShaders();
}

void SaveConfig () 
{
  INISetInt(RENDER_2D, portals.show_2d, "Draw in 2D windows");
  INISetInt(WIDTH_2D, (int)portals.width_2d, "Width of lines in 2D windows (in units of 1/2)");
  INISetInt(COLOR_2D, (int)portals.color_2d, "Color of lines in 2D windows");
  INISetInt(AA_2D, portals.aa_2d, "Draw lines in 2D window anti-aliased");

  INISetInt(ZBUFFER, portals.zbuffer, "ZBuffer level in 3D window");
  INISetInt(FOG, portals.fog, "Use depth cueing in 3D window");
  INISetInt(POLYGON, portals.polygons, "Render using polygons polygons in 3D window");
  INISetInt(LINE, portals.polygons, "Render using lines in 3D window");
  INISetInt(RENDER_3D, portals.show_3d, "Draw in 3D windows");
  INISetInt(WIDTH_3D, (int)portals.width_3d, "Width of lines in 3D window (in units of 1/2)");
  INISetInt(COLOR_3D, (int)portals.color_3d, "Color of lines/polygons in 3D window");
  INISetInt(COLOR_FOG, (int)portals.color_fog, "Color of distant lines/polygons in 3D window");
  INISetInt(AA_3D, portals.aa_3d, "Draw lines in 3D window anti-aliased");
  INISetInt(TRANS_3D, (int)portals.trans_3d, "Transparency in 3d view (0 = solid, 100 = invisible)");
  INISetInt(CLIP, portals.clip, "Cubic clipper active for portal viewer");
  INISetInt(CLIP_RANGE, (int)portals.clip_range, "Portal viewer cubic clip distance (in units of 64)");
}


#define CONFIG_SECTION "Configuration"

int INIGetInt(char *key, int def)
{
  char value[1024];

  if (read_var (INIfn.c_str(), CONFIG_SECTION, key, value))
    return atoi (value);
  else
    return def;
}

void INISetInt(char *key, int val, char *comment /* = NULL */)
{
  char s[1000];

  if(comment)
    sprintf(s, "%d        ; %s", val, comment);
  else
    sprintf(s, "%d", val);
  save_var (INIfn.c_str(), CONFIG_SECTION, key, s);
}


// plugin name
static const char *PLUGIN_NAME = "Portal Viewer";
// commands in the menu
static const char *PLUGIN_COMMANDS =
	Q3R_CMD_ABOUT ";"
	Q3R_CMD_SPLITTER ";"
	Q3R_CMD_OPTIONS ";"
	Q3R_CMD_SPLITTER ";"
	Q3R_CMD_SHOW_2D ";"
	Q3R_CMD_SHOW_3D ";"
	Q3R_CMD_SPLITTER ";"
	Q3R_CMD_RELEASE ";"
	Q3R_CMD_LOAD;



const char* QERPlug_Init (void *hApp, void* pMainWidget)
{  
  return "Portal Viewer for Q3Radiant";
}

const char* QERPlug_GetName()
{
  return PLUGIN_NAME;
}

const char* QERPlug_GetCommandList()
{
  return PLUGIN_COMMANDS;
}


const char* QERPlug_GetCommandTitleList()
{
  return "";
}


void QERPlug_Dispatch(const char* p, float* vMin, float* vMax, bool bSingleBrush)
{
  globalOutputStream() << MSG_PREFIX "Command \"" << p << "\"\n";

  if (!strcmp(p,Q3R_CMD_ABOUT))
  {
    DoAboutDlg ();
  }
  else if (!strcmp(p,Q3R_CMD_LOAD))
  {
    if (DoLoadPortalFileDialog () == IDOK)
    {
      portals.Load();
      SceneChangeNotify();
    }
    else
    {
      globalOutputStream() << MSG_PREFIX "Portal file load aborted.\n";
    }
  }
  else if (!strcmp(p,Q3R_CMD_RELEASE))
  {
    portals.Purge();

    SceneChangeNotify();

    globalOutputStream() << MSG_PREFIX "Portals unloaded.\n";
  }
  else if (!strcmp(p,Q3R_CMD_SHOW_2D))
  {
    portals.show_2d = !portals.show_2d;

    SceneChangeNotify();
    SaveConfig();

    if(portals.show_2d)
      globalOutputStream() << MSG_PREFIX "Portals will be rendered in 2D view.\n";
    else
      globalOutputStream() << MSG_PREFIX "Portals will NOT be rendered in 2D view.\n";
  }
  else if (!strcmp(p,Q3R_CMD_SHOW_3D))
  {
    portals.show_3d = !portals.show_3d;
    SaveConfig();

    SceneChangeNotify();

    if (portals.show_3d)
      globalOutputStream() << MSG_PREFIX "Portals will be rendered in 3D view.\n";
    else
      globalOutputStream() << MSG_PREFIX "Portals will NOT be rendered in 3D view.\n";
  }
  else if (!strcmp(p,Q3R_CMD_OPTIONS))
  {
    DoConfigDialog ();
    SaveConfig();

    SceneChangeNotify();
  }
}


#include "modulesystem/singletonmodule.h"

class PrtViewPluginDependencies :
  public GlobalSceneGraphModuleRef,
  public GlobalRadiantModuleRef,
  public GlobalShaderCacheModuleRef,
  public GlobalOpenGLModuleRef, 
  public GlobalOpenGLStateLibraryModuleRef
{
};

class PrtViewPluginModule
{
  _QERPluginTable m_plugin;
public:
  typedef _QERPluginTable Type;
  STRING_CONSTANT(Name, "prtview");

  PrtViewPluginModule()
  {
    m_plugin.m_pfnQERPlug_Init = QERPlug_Init;
    m_plugin.m_pfnQERPlug_GetName = QERPlug_GetName;
    m_plugin.m_pfnQERPlug_GetCommandList = QERPlug_GetCommandList;
    m_plugin.m_pfnQERPlug_GetCommandTitleList = QERPlug_GetCommandTitleList;
    m_plugin.m_pfnQERPlug_Dispatch = QERPlug_Dispatch;

    PrtView_construct();
  }
  ~PrtViewPluginModule()
  {
    PrtView_destroy();
  }
  _QERPluginTable* getTable()
  {
    return &m_plugin;
  }
};

typedef SingletonModule<PrtViewPluginModule, PrtViewPluginDependencies> SingletonPrtViewPluginModule;

SingletonPrtViewPluginModule g_PrtViewPluginModule;


extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules(ModuleServer& server)
{
  initialiseModule(server);

  g_PrtViewPluginModule.selfRegister();
}
