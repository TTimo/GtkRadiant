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

#include "ibrush.h"
#include "ipatch.h"
#include "ifiletypes.h"
#include "ieclass.h"
#include "qerplugin.h"

#include "modulesystem/singletonmodule.h"
#include "typesystem.h"

#include "xmlparse.h"
#include "xmlwrite.h"


class MapXMLDependencies :
  public GlobalRadiantModuleRef,
  public GlobalBrushModuleRef,
  public GlobalPatchModuleRef,
  public GlobalFiletypesModuleRef,
  public GlobalEntityClassManagerModuleRef,
  public GlobalSceneGraphModuleRef
{
public:
  MapXMLDependencies() :
    GlobalBrushModuleRef(GlobalRadiant().getRequiredGameDescriptionKeyValue("brushtypes")),
    GlobalPatchModuleRef(GlobalRadiant().getRequiredGameDescriptionKeyValue("patchtypes")),
    GlobalEntityClassManagerModuleRef(GlobalRadiant().getRequiredGameDescriptionKeyValue("entityclass"))
  {
  }
};

class MapXMLAPI : public TypeSystemRef, public MapFormat
{
public:
  typedef MapFormat Type;
  STRING_CONSTANT(Name, "xmlq3");

  MapXMLAPI()
  {
    GlobalFiletypesModule::getTable().addType(Type::Name(), Name(), filetype_t("xml quake3 maps", "*.xmap"));
  }
  MapFormat* getTable()
  {
    return this;
  }

  void readGraph(scene::Node& root, TextInputStream& inputStream, EntityCreator& entityTable) const
  {
    Map_Read(root, inputStream, entityTable);
  }
  void writeGraph(scene::Node& root, GraphTraversalFunc traverse, TextOutputStream& outputStream) const
  {
    Map_Write(root, traverse, outputStream);
  }
};

typedef SingletonModule<MapXMLAPI, MapXMLDependencies> MapXMLModule;

MapXMLModule g_MapXMLModule;



extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules(ModuleServer& server)
{
  GlobalErrorStream::instance().setOutputStream(server.getErrorStream());
  GlobalOutputStream::instance().setOutputStream(server.getOutputStream());
  GlobalDebugMessageHandler::instance().setHandler(server.getDebugMessageHandler());
  GlobalModuleServer::instance().set(server);

  g_MapXMLModule.selfRegister();
}
