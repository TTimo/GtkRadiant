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

#if !defined(INCLUDED_MODULESYSTEM_H)
#define INCLUDED_MODULESYSTEM_H

#include "generic/static.h"

#if defined(WIN32)
#define RADIANT_DLLEXPORT __stdcall
#else
#define RADIANT_DLLEXPORT
#endif


class Module
{
public:
  virtual void capture() = 0;
  virtual void release() = 0;
  virtual void* getTable() = 0;
};

inline void* Module_getTable(Module& module)
{
  return module.getTable();
}

class TextOutputStream;
class DebugMessageHandler;

class ModuleServer
{
public:
  class Visitor
  {
  public:
    virtual void visit(const char* name, Module& module) = 0;
  };

  virtual void setError(bool error) = 0;
  virtual bool getError() const = 0;

  virtual TextOutputStream& getOutputStream() = 0;
  virtual TextOutputStream& getErrorStream() = 0;
  virtual DebugMessageHandler& getDebugMessageHandler() = 0;

  virtual void registerModule(const char* type, int version, const char* name, Module& module) = 0;
  virtual Module* findModule(const char* type, int version, const char* name) const = 0;
  virtual void foreachModule(const char* type, int version, Visitor& visitor) = 0;
};

class ModuleServerHolder
{
  ModuleServer* m_server;
public:
  ModuleServerHolder()
    : m_server(0)
  {
  }
  void set(ModuleServer& server)
  {
    m_server = &server;
  }
  ModuleServer& get()
  {
    return *m_server;
  }
};

typedef Static<ModuleServerHolder> GlobalModuleServer;

inline ModuleServer& globalModuleServer()
{
  return GlobalModuleServer::instance().get();
}


template<typename Type>
class Modules
{
public:
  class Visitor
  {
  public:
    virtual void visit(const char* name, const Type& table) = 0;
  };

  virtual Type* findModule(const char* name) = 0;
  virtual void foreachModule(Visitor& visitor) = 0;
};

#include "debugging/debugging.h"

template<typename Type>
class ModuleRef
{
  Module* m_module;
  Type* m_table;
public:
  ModuleRef(const char* name) : m_table(0)
  {
    if(!globalModuleServer().getError())
    {
      m_module = globalModuleServer().findModule(typename Type::Name(), typename Type::Version(), name);
      if(m_module == 0)
      {
        globalModuleServer().setError(true);
        globalErrorStream() << "ModuleRef::initialise: type=" << makeQuoted(typename Type::Name()) << " version=" << makeQuoted(typename Type::Version()) << " name=" << makeQuoted(name) << " - not found\n";
      }
      else
      {
        m_module->capture();
        if(!globalModuleServer().getError())
        {
          m_table = static_cast<Type*>(m_module->getTable());
        }
      }
    }
  }
  ~ModuleRef()
  {
    if(m_module != 0)
    {
      m_module->release();
    }
  }
  Type* getTable()
  {
#if defined(_DEBUG)
    ASSERT_MESSAGE(m_table != 0, "ModuleRef::getTable: type=" << makeQuoted(typename Type::Name()) << " version=" << makeQuoted(typename Type::Version()) << " - module-reference used without being initialised");
#endif
    return m_table;
  }
};

template<typename Type>
class SingletonModuleRef
{
  Module* m_module;
  Type* m_table;
public:

  SingletonModuleRef()
    : m_module(0), m_table(0)
  {
  }

  bool initialised() const
  {
    return m_module != 0;
  }

  void initialise(const char* name)
  {
    m_module = globalModuleServer().findModule(typename Type::Name(), typename Type::Version(), name);
    if(m_module == 0)
    {
      globalModuleServer().setError(true);
      globalErrorStream() << "SingletonModuleRef::initialise: type=" << makeQuoted(typename Type::Name()) << " version=" << makeQuoted(typename Type::Version()) << " name=" << makeQuoted(name) << " - not found\n";
    }
  }

  Type* getTable()
  {
#if defined(_DEBUG)
    ASSERT_MESSAGE(m_table != 0, "SingletonModuleRef::getTable: type=" << makeQuoted(typename Type::Name()) << " version=" << makeQuoted(typename Type::Version()) << " - module-reference used without being initialised");
#endif
    return m_table;
  }
  void capture()
  {
    if(initialised())
    {
      m_module->capture();
      m_table = static_cast<Type*>(m_module->getTable());
    }
  }
  void release()
  {
    if(initialised())
    {
      m_module->release();
    }
  }
};

template<typename Type>
class GlobalModule
{
  static SingletonModuleRef<Type> m_instance;
public:
  static SingletonModuleRef<Type>& instance()
  {
    return m_instance;
  }
  static Type& getTable()
  {
    return *m_instance.getTable();
  }
};

template<class Type>
SingletonModuleRef<Type> GlobalModule<Type>::m_instance;


template<typename Type>
class GlobalModuleRef
{
public:
  GlobalModuleRef(const char* name = "*")
  {
    if(!globalModuleServer().getError())
    {
      GlobalModule<Type>::instance().initialise(name);
    }
    GlobalModule<Type>::instance().capture();
  }
  ~GlobalModuleRef()
  {
    GlobalModule<Type>::instance().release();
  }
  Type& getTable()
  {
    return GlobalModule<Type>::getTable();
  }
};

#endif
