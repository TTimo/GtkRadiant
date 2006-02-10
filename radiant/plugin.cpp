/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

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

#include "plugin.h"

#include "debugging/debugging.h"

#include "qerplugin.h"
#include "ifilesystem.h"
#include "ishaders.h"
#include "ientity.h"
#include "ieclass.h"
#include "irender.h"
#include "iscenegraph.h"
#include "iselection.h"
#include "ifilter.h"
#include "iscriplib.h"
#include "igl.h"
#include "iundo.h"
#include "itextures.h"
#include "ireference.h"
#include "ifiletypes.h"
#include "preferencesystem.h"
#include "ibrush.h"
#include "ipatch.h"
#include "iimage.h"
#include "itoolbar.h"
#include "iplugin.h"
#include "imap.h"
#include "namespace.h"

#include "gtkutil/messagebox.h"
#include "gtkutil/filechooser.h"
#include "maplib.h"

#include "error.h"
#include "map.h"
#include "qe3.h"
#include "entityinspector.h"
#include "entitylist.h"
#include "points.h"
#include "gtkmisc.h"
#include "texwindow.h"
#include "mainframe.h"
#include "build.h"
#include "mru.h"
#include "multimon.h"
#include "surfacedialog.h"
#include "groupdialog.h"
#include "patchdialog.h"
#include "camwindow.h"
#include "watchbsp.h"
#include "xywindow.h"
#include "entity.h"
#include "select.h"
#include "preferences.h"
#include "autosave.h"
#include "plugintoolbar.h"
#include "findtexturedialog.h"
#include "nullmodel.h"
#include "grid.h"

#include "modulesystem/modulesmap.h"
#include "modulesystem/singletonmodule.h"

#include "generic/callback.h"

const char* GameDescription_getKeyValue(const char* key)
{
  return g_pGameDescription->getKeyValue(key);
}

const char* GameDescription_getRequiredKeyValue(const char* key)
{
  return g_pGameDescription->getRequiredKeyValue(key);
}

class RadiantCoreAPI
{
  _QERFuncTable_1 m_radiantcore;
public:
  typedef _QERFuncTable_1 Type;
  STRING_CONSTANT(Name, "*");

  RadiantCoreAPI()
  {
    m_radiantcore.getEnginePath = &EnginePath_get;
    m_radiantcore.getAppPath = &AppPath_get;
    m_radiantcore.getGameToolsPath = &GameToolsPath_get;
    m_radiantcore.getSettingsPath = &SettingsPath_get;

    m_radiantcore.getGameName = &gamename_get;
    m_radiantcore.getGameMode = &gamemode_get;

    m_radiantcore.getGameDescriptionKeyValue = &GameDescription_getKeyValue;
    m_radiantcore.getRequiredGameDescriptionKeyValue = &GameDescription_getRequiredKeyValue;

    m_radiantcore.attachGameToolsPathObserver = Radiant_attachGameToolsPathObserver;
    m_radiantcore.detachGameToolsPathObserver = Radiant_detachGameToolsPathObserver;
    m_radiantcore.attachEnginePathObserver = Radiant_attachEnginePathObserver;
    m_radiantcore.detachEnginePathObserver = Radiant_detachEnginePathObserver;
    m_radiantcore.attachGameNameObserver = Radiant_attachGameNameObserver;
    m_radiantcore.detachGameNameObserver = Radiant_detachGameNameObserver;
    m_radiantcore.attachGameModeObserver = Radiant_attachGameModeObserver;
    m_radiantcore.detachGameModeObserver = Radiant_detachGameModeObserver;

    m_radiantcore.m_pfnMessageBox = &gtk_MessageBox;
    m_radiantcore.m_pfnFileDialog = &file_dialog;
    m_radiantcore.m_pfnColorDialog = &color_dialog;
    m_radiantcore.m_pfnDirDialog = &dir_dialog;
    m_radiantcore.m_pfnNewImage = &new_plugin_image;
  }
  _QERFuncTable_1* getTable()
  {
    return &m_radiantcore;
  }
};

typedef SingletonModule<RadiantCoreAPI> RadiantCoreModule;
typedef Static<RadiantCoreModule> StaticRadiantCoreModule;
StaticRegisterModule staticRegisterRadiantCore(StaticRadiantCoreModule::instance());


class RadiantDependencies :
  public GlobalRadiantModuleRef,
  public GlobalFileSystemModuleRef,
  public GlobalEntityModuleRef,
  public GlobalShadersModuleRef,
  public GlobalBrushModuleRef,
  public GlobalSceneGraphModuleRef,
  public GlobalShaderCacheModuleRef,
  public GlobalFiletypesModuleRef,
  public GlobalSelectionModuleRef,
  public GlobalReferenceModuleRef,
  public GlobalOpenGLModuleRef,
  public GlobalEntityClassManagerModuleRef,
  public GlobalUndoModuleRef,
  public GlobalScripLibModuleRef,
  public GlobalNamespaceModuleRef
{
  ImageModulesRef m_image_modules;
  MapModulesRef m_map_modules;
  ToolbarModulesRef m_toolbar_modules;
  PluginModulesRef m_plugin_modules;

public:
  RadiantDependencies() :
    GlobalEntityModuleRef(GlobalRadiant().getRequiredGameDescriptionKeyValue("entities")),
    GlobalShadersModuleRef(GlobalRadiant().getRequiredGameDescriptionKeyValue("shaders")),
    GlobalBrushModuleRef(GlobalRadiant().getRequiredGameDescriptionKeyValue("brushtypes")),
    GlobalEntityClassManagerModuleRef(GlobalRadiant().getRequiredGameDescriptionKeyValue("entityclass")),
    m_image_modules(GlobalRadiant().getRequiredGameDescriptionKeyValue("texturetypes")),
    m_map_modules(GlobalRadiant().getRequiredGameDescriptionKeyValue("maptypes")),
    m_toolbar_modules("*"),
    m_plugin_modules("*")
  {
  }

  ImageModules& getImageModules()
  {
    return m_image_modules.get();
  }
  MapModules& getMapModules()
  {
    return m_map_modules.get();
  }
  ToolbarModules& getToolbarModules()
  {
    return m_toolbar_modules.get();
  }
  PluginModules& getPluginModules()
  {
    return m_plugin_modules.get();
  }
};

class Radiant : public TypeSystemRef
{
public:
  Radiant()
  {
    Preferences_Init();

    GlobalFiletypes().addType("sound", "wav", filetype_t("PCM sound files", "*.wav"));

    Selection_construct();
    HomePaths_Construct();
    VFS_Construct();
    Grid_construct();
    MultiMon_Construct();
    MRU_Construct();
    Pointfile_Construct();
    GLWindow_Construct();
    BuildMenu_Construct();
    Map_Construct();
    EntityList_Construct();
    MainFrame_Construct();
    GroupDialog_Construct();
    SurfaceInspector_Construct();
    PatchInspector_Construct();
    CamWnd_Construct();
    XYWindow_Construct();
    BuildMonitor_Construct();
    TextureBrowser_Construct();
    Entity_Construct();
    Autosave_Construct();
    EntityInspector_construct();
    FindTextureDialog_Construct();
    NullModel_construct();
    MapRoot_construct();

    EnginePath_verify();
    EnginePath_Realise();
  }
  ~Radiant()
  {
    EnginePath_Unrealise();

    MapRoot_destroy();
    NullModel_destroy();
    FindTextureDialog_Destroy();
    EntityInspector_destroy();
    Autosave_Destroy();
    Entity_Destroy();
    TextureBrowser_Destroy();
    BuildMonitor_Destroy();
    XYWindow_Destroy();
    CamWnd_Destroy();
    PatchInspector_Destroy();
    SurfaceInspector_Destroy();
    GroupDialog_Destroy();
    MainFrame_Destroy();
    EntityList_Destroy();
    Map_Destroy();
    BuildMenu_Destroy();
    GLWindow_Destroy();
    Pointfile_Destroy();
    MRU_Destroy();
    MultiMon_Destroy();
    Grid_destroy();
    VFS_Destroy();
    HomePaths_Destroy();
    Selection_destroy();
  }
};

namespace
{
  bool g_RadiantInitialised = false;
  RadiantDependencies* g_RadiantDependencies;
  Radiant* g_Radiant;
}



bool Radiant_Construct(ModuleServer& server)
{
  GlobalModuleServer::instance().set(server);
  StaticModuleRegistryList().instance().registerModules();

  g_RadiantDependencies = new RadiantDependencies();

  g_RadiantInitialised = !server.getError();

  if(g_RadiantInitialised)
  {
    g_Radiant = new Radiant;
  }

  return g_RadiantInitialised;
}
void Radiant_Destroy()
{
  if(g_RadiantInitialised)
  {
    delete g_Radiant;
  }

  delete g_RadiantDependencies;
}

ImageModules& Radiant_getImageModules()
{
  return g_RadiantDependencies->getImageModules();
}
MapModules& Radiant_getMapModules()
{
  return g_RadiantDependencies->getMapModules();
}
ToolbarModules& Radiant_getToolbarModules()
{
  return g_RadiantDependencies->getToolbarModules();
}
PluginModules& Radiant_getPluginModules()
{
  return g_RadiantDependencies->getPluginModules();
}


