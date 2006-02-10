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

#include "eclass.h"

#include "debugging/debugging.h"

#include <map>

#include "ifilesystem.h"

#include "string/string.h"
#include "eclasslib.h"
#include "os/path.h"
#include "os/dir.h"
#include "stream/stringstream.h"
#include "moduleobservers.h"

#include "cmdlib.h"

#include "preferences.h"
#include "mainframe.h"


namespace
{
  typedef std::map<const char*, EntityClass*, RawStringLessNoCase> EntityClasses;
  EntityClasses g_entityClasses;
  EntityClass	*eclass_bad = 0;
  char		eclass_directory[1024];
  typedef std::map<CopiedString, ListAttributeType> ListAttributeTypes;
  ListAttributeTypes g_listTypes;
}

/*!
implementation of the EClass manager API
*/

void CleanEntityList(EntityClasses& entityClasses)
{
  for(EntityClasses::iterator i = entityClasses.begin(); i != entityClasses.end(); ++i)
  {
    (*i).second->free((*i).second);
  }
  entityClasses.clear();
}

void Eclass_Clear()
{
  CleanEntityList(g_entityClasses);
  g_listTypes.clear();
}

EntityClass* EClass_InsertSortedList(EntityClasses& entityClasses, EntityClass *entityClass)
{
  std::pair<EntityClasses::iterator, bool> result = entityClasses.insert(EntityClasses::value_type(entityClass->name(), entityClass));
  if(!result.second)
  {
    entityClass->free(entityClass);
  }
  return (*result.first).second;
}

EntityClass* Eclass_InsertAlphabetized (EntityClass *e)
{
  return EClass_InsertSortedList(g_entityClasses, e);
}

void Eclass_forEach(EntityClassVisitor& visitor)
{
  for(EntityClasses::iterator i = g_entityClasses.begin(); i != g_entityClasses.end(); ++i)
  {
    visitor.visit((*i).second);
  }
}


class RadiantEclassCollector : public EntityClassCollector
{
public:
  void insert(EntityClass* eclass)
  {
    Eclass_InsertAlphabetized(eclass);
  }
  void insert(const char* name, const ListAttributeType& list)
  {
    g_listTypes.insert(ListAttributeTypes::value_type(name, list));
  }
};

RadiantEclassCollector g_collector;

const ListAttributeType* EntityClass_findListType(const char* name)
{
  ListAttributeTypes::iterator i = g_listTypes.find(name);
  if(i != g_listTypes.end())
  {
    return &(*i).second;
  }
  return 0;
}


class EntityClassFilterMode
{
public:
  bool filter_mp_sp;
  const char* mp_ignore_prefix;
  const char* sp_ignore_prefix;

  EntityClassFilterMode() :
    filter_mp_sp(!string_empty(g_pGameDescription->getKeyValue("eclass_filter_gamemode"))),
    mp_ignore_prefix(g_pGameDescription->getKeyValue("eclass_sp_prefix")),
    sp_ignore_prefix(g_pGameDescription->getKeyValue("eclass_mp_prefix"))
  {
    if(string_empty(mp_ignore_prefix))
    {
      mp_ignore_prefix = "sp_";
    }
    if(string_empty(sp_ignore_prefix))
    {
      sp_ignore_prefix = "mp_";
    }
  }
};

class EntityClassesLoadFile
{
  const char* m_directory;
public:
  EntityClassesLoadFile(const char* directory) : m_directory(directory)
  {
  }
  void operator()(const char* name) const
  {
    EntityClassFilterMode filterMode;

    if(filterMode.filter_mp_sp)
    {
      if(string_empty(GlobalRadiant().getGameMode()) || string_equal(GlobalRadiant().getGameMode(), "sp"))
      {
        if(string_equal_n(name, filterMode.sp_ignore_prefix, strlen(filterMode.sp_ignore_prefix)))
        {
          globalOutputStream() << "Ignoring '" << name << "'\n";
          return;
        }
      }
      else
      {
        if(string_equal_n(name, filterMode.mp_ignore_prefix, strlen(filterMode.mp_ignore_prefix)))
        {
          globalOutputStream() << "Ignoring '" << name << "'\n";
          return;
        }
      }
    }
  
    // for a given name, we grab the first .def in the vfs
    // this allows to override baseq3/scripts/entities.def for instance
    StringOutputStream relPath(256);
    relPath << m_directory << name;

    GlobalEClassLoader().scanFile(g_collector, relPath.c_str());
  }
};

struct PathLess
{
  bool operator()(const CopiedString& path, const CopiedString& other) const
  {
    return path_less(path.c_str(), other.c_str());
  }
};

typedef std::map<CopiedString, const char*, PathLess> Paths;

class PathsInsert
{
  Paths& m_paths;
  const char* m_directory;
public:
  PathsInsert(Paths& paths, const char* directory) : m_paths(paths), m_directory(directory)
  {
  }
  void operator()(const char* name) const
  {
    m_paths.insert(Paths::value_type(name, m_directory));
  }
};

#if 0
void EntityClassQuake3_constructDirectory(const char* directory, const char* extension)
{
  globalOutputStream() << "EntityClass: searching " << makeQuoted(directory) << " for *." << extension << '\n'; 
  Directory_forEach(directory, matchFileExtension(extension, EntityClassesLoadFile(directory)));
}
#else
void EntityClassQuake3_constructDirectory(const char* directory, const char* extension, Paths& paths)
{
  globalOutputStream() << "EntityClass: searching " << makeQuoted(directory) << " for *." << extension << '\n'; 
  Directory_forEach(directory, matchFileExtension(extension, PathsInsert(paths, directory)));
}
#endif

void EntityClassQuake3_Construct()
{
#if 1
  StringOutputStream baseDirectory(256);
  StringOutputStream gameDirectory(256);
  const char* basegame = GlobalRadiant().getRequiredGameDescriptionKeyValue("basegame");
  const char* gamename = GlobalRadiant().getGameName();
  baseDirectory << GlobalRadiant().getGameToolsPath() << basegame << '/';
  gameDirectory << GlobalRadiant().getGameToolsPath() << gamename << '/';

  Paths paths;
  EntityClassQuake3_constructDirectory(baseDirectory.c_str(), GlobalEClassLoader().getExtension(), paths);
  if(!string_equal(basegame, gamename))
  {
    EntityClassQuake3_constructDirectory(gameDirectory.c_str(), GlobalEClassLoader().getExtension(), paths);
  }

  for(Paths::iterator i = paths.begin(); i != paths.end(); ++i)
  {
    EntityClassesLoadFile((*i).second)((*i).first.c_str());
  }
#else
  StringOutputStream directory(256);
  directory << GlobalRadiant().getGameToolsPath() << GlobalRadiant().getGameName() << '/';
  EntityClassQuake3_constructDirectory(directory.c_str(), GlobalEClassLoader().getExtension());
#endif
}

EntityClass *Eclass_ForName(const char *name, bool has_brushes)
{
	ASSERT_NOTNULL(name);

  if(string_empty(name))
  {
    return eclass_bad;
  }

  EntityClasses::iterator i = g_entityClasses.find(name);
  if(i != g_entityClasses.end() && string_equal((*i).first, name))
  {
    return (*i).second;
  }

	EntityClass* e = EntityClass_Create_Default(name, has_brushes);
	return Eclass_InsertAlphabetized(e);
}

class EntityClassQuake3 : public ModuleObserver
{
  std::size_t m_unrealised;
  ModuleObservers m_observers;
public:
  EntityClassQuake3() : m_unrealised(4)
  {
  }
  void realise()
  {
    if(--m_unrealised == 0)
    {
      //globalOutputStream() << "Entity Classes: realise\n";
      EntityClassQuake3_Construct();
      m_observers.realise();
    }
  }
  void unrealise()
  {
    if(++m_unrealised == 1)
    {
      m_observers.unrealise();
      //globalOutputStream() << "Entity Classes: unrealise\n";
      Eclass_Clear();
    }
  }
  void attach(ModuleObserver& observer)
  {
    m_observers.attach(observer);
  }
  void detach(ModuleObserver& observer)
  {
    m_observers.detach(observer);
  }
};

EntityClassQuake3 g_EntityClassQuake3;

void EntityClass_attach(ModuleObserver& observer)
{
  g_EntityClassQuake3.attach(observer);
}
void EntityClass_detach(ModuleObserver& observer)
{
  g_EntityClassQuake3.detach(observer);
}

void EntityClass_realise()
{
  g_EntityClassQuake3.realise();
}
void EntityClass_unrealise()
{
  g_EntityClassQuake3.unrealise();
}

void EntityClassQuake3_construct()
{
  // start by creating the default unknown eclass
  eclass_bad = EClass_Create("UNKNOWN_CLASS", Vector3(0.0f, 0.5f, 0.0f), "");

  EntityClass_realise();
}

void EntityClassQuake3_destroy()
{
  EntityClass_unrealise();

  eclass_bad->free(eclass_bad);
}

class EntityClassQuake3Dependencies :
  public GlobalRadiantModuleRef,
  public GlobalFileSystemModuleRef,
  public GlobalShaderCacheModuleRef,
  public GlobalEClassModuleRef
{
public:
  EntityClassQuake3Dependencies() :
    GlobalEClassModuleRef(GlobalRadiant().getRequiredGameDescriptionKeyValue("entityclasstype"))
  {
  }
};

class EclassManagerAPI
{
  EntityClassManager m_eclassmanager;
public:
  typedef EntityClassManager Type;
  STRING_CONSTANT(Name, "quake3");

  EclassManagerAPI()
  {
    EntityClassQuake3_construct();

    m_eclassmanager.findOrInsert = &Eclass_ForName;
    m_eclassmanager.findListType = &EntityClass_findListType;
    m_eclassmanager.forEach = &Eclass_forEach;
    m_eclassmanager.attach = &EntityClass_attach;
    m_eclassmanager.detach = &EntityClass_detach;
    m_eclassmanager.realise = &EntityClass_realise;
    m_eclassmanager.unrealise = &EntityClass_unrealise;

    GlobalRadiant().attachGameToolsPathObserver(g_EntityClassQuake3);
    GlobalRadiant().attachGameModeObserver(g_EntityClassQuake3);
    GlobalRadiant().attachGameNameObserver(g_EntityClassQuake3);
  }
  ~EclassManagerAPI()
  {
    GlobalRadiant().detachGameNameObserver(g_EntityClassQuake3);
    GlobalRadiant().detachGameModeObserver(g_EntityClassQuake3);
    GlobalRadiant().detachGameToolsPathObserver(g_EntityClassQuake3);

    EntityClassQuake3_destroy();
  }
  EntityClassManager* getTable()
  {
    return &m_eclassmanager;
  }
};

#include "modulesystem/singletonmodule.h"
#include "modulesystem/moduleregistry.h"

typedef SingletonModule<EclassManagerAPI, EntityClassQuake3Dependencies> EclassManagerModule;
typedef Static<EclassManagerModule> StaticEclassManagerModule;
StaticRegisterModule staticRegisterEclassManager(StaticEclassManagerModule::instance());


