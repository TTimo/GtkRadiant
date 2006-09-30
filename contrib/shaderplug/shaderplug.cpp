/*
Copyright (C) 2006, Stefan Greven.
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

#include "shaderplug.h"

#include "debugging/debugging.h"

#include <string>
#include <vector>
#include "string/string.h"
#include "modulesystem/singletonmodule.h"
#include "stream/stringstream.h"
#include "os/file.h"

#include <gtk/gtk.h>

#include "iplugin.h"
#include "qerplugin.h"
#include "ifilesystem.h"
#include "iarchive.h"
#include "ishaders.h"
#include "iscriplib.h"

#include "generic/callback.h"

namespace {
const char SHADERTAG_FILE[] = "shadertags.xml";
}

class ShaderPlugPluginDependencies : public GlobalRadiantModuleRef,
                                     public GlobalFileSystemModuleRef,
                                     public GlobalShadersModuleRef
{
public:
  ShaderPlugPluginDependencies() :
      GlobalShadersModuleRef(GlobalRadiant().getRequiredGameDescriptionKeyValue("shaders"))
  {
  }
};

namespace Shaderplug
{
  GtkWindow* g_window;

  std::vector<const char*> archives;
  std::set<std::string> shaders;
  std::set<std::string> textures;

  XmlTagBuilder TagBuilder;
  void CreateTagFile();

  const char* init(void* hApp, void* pMainWidget)
  {
    g_window = GTK_WINDOW(pMainWidget);
    return "";
  }
  const char* getName()
  {
    return "ShaderPlug";
  }
  const char* getCommandList()
  {
    return "About;Create tag file";
  }
  const char* getCommandTitleList()
  {
    return "";
  }
  void dispatch(const char* command, float* vMin, float* vMax, bool bSingleBrush)
  {
    if(string_equal(command, "About"))
    {
      GlobalRadiant().m_pfnMessageBox(GTK_WIDGET(g_window), "Shaderplug (1.0)\n\n"
                                        "by Shaderman (shaderman@gmx.net)",
                                        "About",
                                        eMB_OK,
                                        eMB_ICONDEFAULT);
    }
    if(string_equal(command, "Create tag file"))
    {
      CreateTagFile();
    }
  }

  void loadArchiveFile(const char* filename)
  {
    archives.push_back(filename);
  }

  typedef FreeCaller1<const char*, loadArchiveFile> LoadArchiveFileCaller;

  void LoadTextureFile(const char* filename)
  {
    std::string s_filename = filename;

    char buffer[256];
    strcpy(buffer, "textures/");

    // append filename without trailing file extension (.tga or .jpg for example)
    strncat(buffer, filename, s_filename.length() - 4);

    std::set<std::string>::iterator iter;
    iter = shaders.find(buffer);

    // a shader with this name already exists
    if(iter == shaders.end())
    {
      textures.insert(buffer);
    }
  }

  typedef FreeCaller1<const char*, LoadTextureFile> LoadTextureFileCaller;

  void GetTextures(char* extension)
  {
    GlobalFileSystem().forEachFile("textures/", extension, LoadTextureFileCaller(), 0);
  }

  void LoadShaderList(const char* filename)
  {
    if(string_equal_prefix(filename, "textures/"))
    {
      shaders.insert(filename);
    }
  }

  typedef FreeCaller1<const char*, LoadShaderList> LoadShaderListCaller;

  void GetAllShaders()
  {
    GlobalShaderSystem().foreachShaderName(LoadShaderListCaller());
  }

  void GetArchiveList()
  {
    GlobalFileSystem().forEachArchive(LoadArchiveFileCaller());
    globalOutputStream() << "Shaderplug: " << (const Unsigned)Shaderplug::archives.size() << " archives found.\n";
  }

  void CreateTagFile()
  {
    const char* shader_type = GlobalRadiant().getGameDescriptionKeyValue("shaders");

    GetAllShaders();
    globalOutputStream() << "Shaderplug: " << (const Unsigned)shaders.size() << " shaders found.\n";

    if(string_equal(shader_type, "quake3"))
    {
      GetTextures("jpg");
      GetTextures("tga");
      GetTextures("png");

      globalOutputStream() << "Shaderplug: " << (const Unsigned)textures.size() << " textures found.\n";
    }

    if(shaders.size() || textures.size() != 0)
    {
      globalOutputStream() << "Shaderplug: Creating XML tag file.\n";

      TagBuilder.CreateXmlDocument();

      std::set<std::string>::reverse_iterator r_iter;

      for(r_iter = textures.rbegin(); r_iter != textures.rend(); ++r_iter)
      {
        TagBuilder.AddShaderNode(const_cast<char*>((*r_iter).c_str()), STOCK, TEXTURE);
      }

      for(r_iter = shaders.rbegin(); r_iter != shaders.rend(); ++r_iter)
      {
        TagBuilder.AddShaderNode(const_cast<char*>((*r_iter).c_str()), STOCK, SHADER);
      }

      // Get the tag file	  
      StringOutputStream tagFileStream(256);
	  tagFileStream << GlobalRadiant().getLocalRcPath() << SHADERTAG_FILE;
      char* tagFile = tagFileStream.c_str();

      char message[256];
      strcpy(message, "Tag file saved to\n");
      strcat(message, tagFile);
      strcat(message, "\nPlease restart Radiant now.\n");

      if(file_exists(tagFile))
      {
        EMessageBoxReturn result = GlobalRadiant().m_pfnMessageBox(GTK_WIDGET(g_window),
            "WARNING! A tag file already exists! Overwrite it?", "Overwrite tag file?",
            eMB_NOYES,
            eMB_ICONWARNING);

        if(result == eIDYES)
        {
          TagBuilder.SaveXmlDoc(tagFile);
          GlobalRadiant().m_pfnMessageBox(GTK_WIDGET(g_window), message, "INFO", eMB_OK, eMB_ICONASTERISK);
        }
      } else {
        TagBuilder.SaveXmlDoc(tagFile);
        GlobalRadiant().m_pfnMessageBox(GTK_WIDGET(g_window), message, "INFO", eMB_OK, eMB_ICONASTERISK);
      }
    } else {
      GlobalRadiant().m_pfnMessageBox(GTK_WIDGET(g_window),
          "No shaders or textures found. No XML tag file created!\n"
          "",
          "ERROR",
          eMB_OK,
          eMB_ICONERROR);
    }
  }
} // namespace

class ShaderPluginModule
{
  _QERPluginTable m_plugin;
public:
  typedef _QERPluginTable Type;
  STRING_CONSTANT(Name, "ShaderPlug");

  ShaderPluginModule()
  {
    m_plugin.m_pfnQERPlug_Init = &Shaderplug::init;
    m_plugin.m_pfnQERPlug_GetName = &Shaderplug::getName;
    m_plugin.m_pfnQERPlug_GetCommandList = &Shaderplug::getCommandList;
    m_plugin.m_pfnQERPlug_GetCommandTitleList = &Shaderplug::getCommandTitleList;
    m_plugin.m_pfnQERPlug_Dispatch = &Shaderplug::dispatch;
  }
  _QERPluginTable* getTable()
  {
    return &m_plugin;
  }
};

typedef SingletonModule<ShaderPluginModule, ShaderPlugPluginDependencies> SingletonShaderPluginModule;

SingletonShaderPluginModule g_ShaderPluginModule;

extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules(ModuleServer& server)
{
  initialiseModule(server);

  g_ShaderPluginModule.selfRegister();
}




