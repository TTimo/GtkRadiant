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

#include "iscenegraph.h"
#include "irender.h"
#include "iselection.h"
#include "ientity.h"
#include "iundo.h"
#include "ieclass.h"
#include "igl.h"
#include "ireference.h"
#include "ifilter.h"
#include "preferencesystem.h"
#include "qerplugin.h"
#include "namespace.h"
#include "modelskin.h"

#include "typesystem.h"

#include "entity.h"
#include "skincache.h"

#include "modulesystem/singletonmodule.h"

class EntityDependencies :
  public GlobalRadiantModuleRef,
  public GlobalOpenGLModuleRef,
  public GlobalUndoModuleRef,
  public GlobalSceneGraphModuleRef,
  public GlobalShaderCacheModuleRef,
  public GlobalSelectionModuleRef,
  public GlobalReferenceModuleRef,
  public GlobalFilterModuleRef,
  public GlobalPreferenceSystemModuleRef,
  public GlobalNamespaceModuleRef,
  public GlobalModelSkinCacheModuleRef
{
};

class EntityQ3API : public TypeSystemRef
{
  EntityCreator* m_entityq3;
public:
  typedef EntityCreator Type;
  STRING_CONSTANT(Name, "quake3");

  EntityQ3API()
  {
    Entity_Construct();

    m_entityq3 = &GetEntityCreator();

    GlobalReferenceCache().setEntityCreator(*m_entityq3);
  }
  ~EntityQ3API()
  {
    Entity_Destroy();
  }
  EntityCreator* getTable()
  {
    return m_entityq3;
  }
};

typedef SingletonModule<EntityQ3API, EntityDependencies> EntityQ3Module;

EntityQ3Module g_EntityQ3Module;


class EntityWolfAPI : public TypeSystemRef
{
  EntityCreator* m_entitywolf;
public:
  typedef EntityCreator Type;
  STRING_CONSTANT(Name, "wolf");

  EntityWolfAPI()
  {
    Entity_Construct(eGameTypeRTCW);

    m_entitywolf = &GetEntityCreator();

    GlobalReferenceCache().setEntityCreator(*m_entitywolf);
  }
  ~EntityWolfAPI()
  {
    Entity_Destroy();
  }
  EntityCreator* getTable()
  {
    return m_entitywolf;
  }
};

typedef SingletonModule<EntityWolfAPI, EntityDependencies> EntityWolfModule;

EntityWolfModule g_EntityWolfModule;


class EntityDoom3API : public TypeSystemRef
{
  EntityCreator* m_entitydoom3;
public:
  typedef EntityCreator Type;
  STRING_CONSTANT(Name, "doom3");

  EntityDoom3API()
  {
    Entity_Construct(eGameTypeDoom3);

    m_entitydoom3 = &GetEntityCreator();

    GlobalReferenceCache().setEntityCreator(*m_entitydoom3);
  }
  ~EntityDoom3API()
  {
    Entity_Destroy();
  }
  EntityCreator* getTable()
  {
    return m_entitydoom3;
  }
};

typedef SingletonModule<EntityDoom3API, EntityDependencies> EntityDoom3Module;

EntityDoom3Module g_EntityDoom3Module;


extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules(ModuleServer& server)
{
  GlobalErrorStream::instance().setOutputStream(server.getErrorStream());
  GlobalOutputStream::instance().setOutputStream(server.getOutputStream());
  GlobalDebugMessageHandler::instance().setHandler(server.getDebugMessageHandler());
  GlobalModuleServer::instance().set(server);

  g_EntityQ3Module.selfRegister();
  g_EntityWolfModule.selfRegister();
  g_EntityDoom3Module.selfRegister();
  Doom3ModelSkinCacheModule_selfRegister(server);
}
