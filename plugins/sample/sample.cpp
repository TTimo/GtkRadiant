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

#include "sample.h"

#include "debugging/debugging.h"

#include "iplugin.h"

#include "string/string.h"
#include "modulesystem/singletonmodule.h"


namespace Sample
{
  const char* init(void* hApp, void* pMainWidget)
  {
    return "";
  }
  const char* getName()
  {
    return "Sample Plugin";
  }
  const char* getCommandList()
  {
    return "About;Do Something";
  }
  const char* getCommandTitleList()
  {
    return "";
  }
  void dispatch(const char* command, float* vMin, float* vMax, bool bSingleBrush)
  {
    if(string_equal(command, "About"))
    {
      globalOutputStream() << "Sample Demo Plugin\n";
    }
    if(string_equal(command, "Do Something"))
    {
      globalOutputStream() << "Sample Command\n";
    }
  }

} // namespace

class SamplePluginModule
{
  _QERPluginTable m_plugin;
public:
  typedef _QERPluginTable Type;
  STRING_CONSTANT(Name, "sample");

  SamplePluginModule()
  {
    m_plugin.m_pfnQERPlug_Init = &Sample::init;
    m_plugin.m_pfnQERPlug_GetName = &Sample::getName;
    m_plugin.m_pfnQERPlug_GetCommandList = &Sample::getCommandList;
    m_plugin.m_pfnQERPlug_GetCommandTitleList = &Sample::getCommandTitleList;
    m_plugin.m_pfnQERPlug_Dispatch = &Sample::dispatch;
  }
  _QERPluginTable* getTable()
  {
    return &m_plugin;
  }
};

typedef SingletonModule<SamplePluginModule> SingletonSamplePluginModule;

SingletonSamplePluginModule g_SamplePluginModule;


extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules(ModuleServer& server)
{
  GlobalErrorStream::instance().setOutputStream(server.getErrorStream());
  GlobalOutputStream::instance().setOutputStream(server.getOutputStream());
  GlobalDebugMessageHandler::instance().setHandler(server.getDebugMessageHandler());
  GlobalModuleServer::instance().set(server);

  g_SamplePluginModule.selfRegister();
}
