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

#include "server.h"

#include "debugging/debugging.h"
#include "warnings.h"

#include <vector>
#include <map>
#include "os/path.h"

#include "modulesystem.h"

class RadiantModuleServer : public ModuleServer
{
  typedef std::pair<CopiedString, int> ModuleType;
  typedef std::pair<ModuleType, CopiedString> ModuleKey;
  typedef std::map<ModuleKey, Module*> Modules_;
  Modules_ m_modules;
  bool m_error;

public:
  RadiantModuleServer() : m_error(false)
  {
  }

  void setError(bool error)
  {
    m_error = error;
  }
  bool getError() const
  {
    return m_error;
  }

  TextOutputStream& getOutputStream()
  {
    return globalOutputStream();
  }
  TextOutputStream& getErrorStream()
  {
    return globalErrorStream();
  }
  DebugMessageHandler& getDebugMessageHandler()
  {
    return globalDebugMessageHandler();
  }

  void registerModule(const char* type, int version, const char* name, Module& module)
  {
    ASSERT_NOTNULL(&module);
    if(!m_modules.insert(Modules_::value_type(ModuleKey(ModuleType(type, version), name), &module)).second)
    {
      globalErrorStream() << "module already registered: type=" << makeQuoted(type) << " name=" << makeQuoted(name) << "\n";
    }
    else
    {
      globalOutputStream() << "Module Registered: type=" << makeQuoted(type) << " version=" << makeQuoted(version) << " name=" << makeQuoted(name) << "\n";
    }
  }

  Module* findModule(const char* type, int version, const char* name) const
  {
    Modules_::const_iterator i = m_modules.find(ModuleKey(ModuleType(type, version), name));
    if(i != m_modules.end())
    {
      return (*i).second;
    }
    return 0;
  }

  void foreachModule(const char* type, int version, Visitor& visitor)
  {
    for(Modules_::const_iterator i = m_modules.begin(); i != m_modules.end(); ++i)
    {
      if(string_equal((*i).first.first.first.c_str(), type))
      {
        visitor.visit((*i).first.second.c_str(), *(*i).second);
      }
    }
  }
};


#if defined(WIN32)

#include <windows.h>

#define FORMAT_BUFSIZE 2048
const char* FormatGetLastError()
{
  static char buf[FORMAT_BUFSIZE];
  FormatMessage(
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    buf,
    FORMAT_BUFSIZE,
    NULL 
  );
  return buf;
}

class DynamicLibrary
{
  HMODULE m_library;
public:
  typedef int (__stdcall* FunctionPointer)();

  DynamicLibrary(const char* filename)
  {
    m_library = LoadLibrary(filename);
    if(m_library == 0)
    {
      globalErrorStream() << "LoadLibrary failed: '" << filename << "'\n";
      globalErrorStream() << "GetLastError: " << FormatGetLastError();
    }
  }
  ~DynamicLibrary()
  {
    if(!failed())
    {
      FreeLibrary(m_library);
    }
  }
  bool failed()
  {
    return m_library == 0;
  }
  FunctionPointer findSymbol(const char* symbol)
  {
    FunctionPointer address = GetProcAddress(m_library, symbol);
    if(address == 0)
    {
      globalErrorStream() << "GetProcAddress failed: '" << symbol << "'\n";
      globalErrorStream() << "GetLastError: " << FormatGetLastError();
    }
    return address;
  }
};

#elif defined(__linux__) || defined (__APPLE__)

#include <dlfcn.h>

class DynamicLibrary
{
  void* m_library;
public:
  typedef int (* FunctionPointer)();

  DynamicLibrary(const char* filename)
  {
    m_library = dlopen(filename, RTLD_NOW);
  }
  ~DynamicLibrary()
  {
    if(!failed())
      dlclose(m_library);
  }
  bool failed()
  {
    return m_library == 0;
  }
  FunctionPointer findSymbol(const char* symbol)
  {
    FunctionPointer p = (FunctionPointer)dlsym(m_library, symbol);
    if(p == 0)
    {
      const char* error = reinterpret_cast<const char*>(dlerror());
      if(error != 0)
      {
        globalErrorStream() << error;
      }
    }
    return p;
  }
};

#endif

class DynamicLibraryModule
{
  typedef void (RADIANT_DLLEXPORT* RegisterModulesFunc)(ModuleServer& server);
  DynamicLibrary m_library;
  RegisterModulesFunc m_registerModule;
public:
  DynamicLibraryModule(const char* filename)
    : m_library(filename), m_registerModule(0)
  {
    if(!m_library.failed())
    {
      m_registerModule = reinterpret_cast<RegisterModulesFunc>(m_library.findSymbol("Radiant_RegisterModules"));
    }
  }
  bool failed()
  {
    return m_registerModule == 0;
  }
  void registerModules(ModuleServer& server)
  {
    m_registerModule(server);
  }
};


class Libraries
{
  typedef std::vector<DynamicLibraryModule*> libraries_t;
  libraries_t m_libraries;

public:
  ~Libraries()
  {
    release();
  }
  void registerLibrary(const char* filename, ModuleServer& server)
  {
    DynamicLibraryModule* library = new DynamicLibraryModule(filename);

    if(library->failed())
    {
      delete library;
    }
    else
    {
      m_libraries.push_back(library);
      library->registerModules(server);
    }
  }
  void release()
  {
    for(libraries_t::iterator i = m_libraries.begin(); i != m_libraries.end(); ++i)
    {
      delete *i;
    }
  }
  void clear()
  {
    m_libraries.clear();
  }
};


Libraries g_libraries;
RadiantModuleServer g_server;

ModuleServer& GlobalModuleServer_get()
{
  return g_server;
}

void GlobalModuleServer_loadModule(const char* filename)
{
  g_libraries.registerLibrary(filename, g_server);
}

void GlobalModuleServer_Initialise()
{
}

void GlobalModuleServer_Shutdown()
{
}
