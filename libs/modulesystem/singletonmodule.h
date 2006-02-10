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

#if !defined(INCLUDED_MODULESYSTEM_SINGLETONMODULE_H)
#define INCLUDED_MODULESYSTEM_SINGLETONMODULE_H

#include "modulesystem.h"
#include <cstddef>
#include "debugging/debugging.h"
#include "modulesystem/moduleregistry.h"
#include "generic/reference.h"

template<typename API, typename Dependencies>
class DefaultAPIConstructor
{
public:
  const char* getName()
  {
    return typename API::Name();
  }

  API* constructAPI(Dependencies& dependencies)
  {
    return new API;
  }
  void destroyAPI(API* api)
  {
    delete api;
  }
};

template<typename API, typename Dependencies>
class DependenciesAPIConstructor
{
public:
  const char* getName()
  {
    return typename API::Name();
  }

  API* constructAPI(Dependencies& dependencies)
  {
    return new API(dependencies);
  }
  void destroyAPI(API* api)
  {
    delete api;
  }
};

class NullDependencies
{
};


template<typename API, typename Dependencies = NullDependencies, typename APIConstructor = DefaultAPIConstructor<API, Dependencies> >
class SingletonModule : public APIConstructor, public Module, public ModuleRegisterable
{
  Dependencies* m_dependencies;
  API* m_api;
  std::size_t m_refcount;
  bool m_dependencyCheck;
  bool m_cycleCheck;
public:
  typedef typename API::Type Type;

  SingletonModule()
    : m_dependencies(0), m_api(0), m_refcount(0), m_dependencyCheck(false), m_cycleCheck(false)
  {
  }
  explicit SingletonModule(const APIConstructor& constructor)
    : APIConstructor(constructor), m_dependencies(0), m_api(0), m_refcount(0), m_dependencyCheck(false), m_cycleCheck(false)
  {
  }
  ~SingletonModule()
  {
    ASSERT_MESSAGE(m_refcount == 0, "module still referenced at shutdown");
  }

  void selfRegister()
  {
    globalModuleServer().registerModule(typename Type::Name(), typename Type::Version(), APIConstructor::getName(), *this);
  }
  
  Dependencies& getDependencies()
  {
    return *m_dependencies;
  }
  void* getTable()
  {
    if(m_api != 0)
    {
      return m_api->getTable();
    }
    return 0;
  }
  void capture()
  {
    if(++m_refcount == 1)
    {
      globalOutputStream() << "Module Initialising: '" << typename Type::Name() << "' '" << APIConstructor::getName() << "'\n";
      m_dependencies = new Dependencies();
      m_dependencyCheck = !globalModuleServer().getError();
      if(m_dependencyCheck)
      {
        m_api = APIConstructor::constructAPI(*m_dependencies);
        globalOutputStream() << "Module Ready: '" << typename Type::Name() << "' '" << APIConstructor::getName() << "'\n";
      }
      else
      {
        globalOutputStream() << "Module Dependencies Failed: '" << typename Type::Name() << "' '" << APIConstructor::getName() << "'\n";
      }
      m_cycleCheck = true;
    }

    ASSERT_MESSAGE(m_cycleCheck, "cyclic dependency detected");
  }
  void release()
  {
    if(--m_refcount == 0)
    {
      if(m_dependencyCheck)
      {
        APIConstructor::destroyAPI(m_api);
      }
      delete m_dependencies;
    }
  }
};


#endif
