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

#include "skincache.h"

#include "ifilesystem.h"
#include "iscriplib.h"
#include "iarchive.h"
#include "modelskin.h"

#include <map>

#include "stream/stringstream.h"
#include "generic/callback.h"
#include "container/cache.h"
#include "container/hashfunc.h"
#include "os/path.h"
#include "moduleobservers.h"
#include "modulesystem/singletonmodule.h"
#include "stringio.h"

void parseShaderName(CopiedString& name, const char* token)
{
  StringOutputStream cleaned(256);
  cleaned << PathCleaned(token);
  name = cleaned.c_str();
}

class Doom3ModelSkin
{
  typedef std::map<CopiedString, CopiedString> Remaps;
  Remaps m_remaps;
public:
  bool parseTokens(Tokeniser& tokeniser)
  {
    RETURN_FALSE_IF_FAIL(Tokeniser_parseToken(tokeniser, "{"));
    tokeniser.nextLine();
    for(;;)
    {
      const char* token = tokeniser.getToken();
      if(token == 0)
      {
        return false;
      }
      if(string_equal(token, "}"))
      {
        tokeniser.nextLine();
        return true;
      }
      else if(string_equal(token, "model"))
      {
        //const char* model =
        tokeniser.getToken();
      }
      else
      {
        CopiedString from, to;
        parseShaderName(from, token);

        tokeniser.nextLine(); // hack to handle badly formed skins

        parseShaderName(to, tokeniser.getToken());

        if(!string_equal(from.c_str(), to.c_str()))
        {
          m_remaps.insert(Remaps::value_type(from, to));
        }
      }
      tokeniser.nextLine();
    }
  }
  const char* getRemap(const char* name) const
  {
    Remaps::const_iterator i = m_remaps.find(name);
    if(i != m_remaps.end())
    {
      return (*i).second.c_str();
    }
    return "";
  }
  void forEachRemap(const SkinRemapCallback& callback) const
  {
    for(Remaps::const_iterator i = m_remaps.begin(); i != m_remaps.end(); ++i)
    {
      callback(SkinRemap((*i).first.c_str(), (*i).second.c_str()));
    }
  }
};

class GlobalSkins
{
public:
  typedef std::map<CopiedString, Doom3ModelSkin> SkinMap;
  SkinMap m_skins;
  Doom3ModelSkin g_nullSkin;

  Doom3ModelSkin& getSkin(const char* name)
  {
    SkinMap::iterator i = m_skins.find(name);
    if(i != m_skins.end())
    {
      return (*i).second;
    }
    
    return g_nullSkin;
  }

  bool parseTokens(Tokeniser& tokeniser)
  {
    tokeniser.nextLine();
    for(;;)
    {
      const char* token = tokeniser.getToken();
      if(token == 0)
      {
        // end of token stream
        return true;
      }
      if(!string_equal(token, "skin"))
      {
        Tokeniser_unexpectedError(tokeniser, token, "skin");
        return false;
      }
      const char* other = tokeniser.getToken();
      if(other == 0)
      {
        Tokeniser_unexpectedError(tokeniser, token, "#string");
        return false;
      }
      CopiedString name;
      parseShaderName(name, other);
      Doom3ModelSkin& skin = m_skins[name];
      RETURN_FALSE_IF_FAIL(skin.parseTokens(tokeniser));
    }
  }

  void parseFile(const char* name)
  {
    StringOutputStream relativeName(64);
    relativeName << "skins/" << name;
    ArchiveTextFile* file = GlobalFileSystem().openTextFile(relativeName.c_str());
    if(file != 0)
    {
      globalOutputStream() << "parsing skins from " << makeQuoted(name) << "\n";
      {
        Tokeniser& tokeniser = GlobalScriptLibrary().m_pfnNewSimpleTokeniser(file->getInputStream());
        parseTokens(tokeniser);
        tokeniser.release();
      }
      file->release();
    }
    else
    {
      globalErrorStream() << "failed to open " << makeQuoted(name) << "\n";
    }
  }

  typedef MemberCaller1<GlobalSkins, const char*, &GlobalSkins::parseFile> ParseFileCaller;

  void construct()
  {
    GlobalFileSystem().forEachFile("skins/", "skin", ParseFileCaller(*this));
  }

  void destroy()
  {
    m_skins.clear();
  }

  void realise()
  {
    construct();
  }
  void unrealise()
  {
    destroy();
  }
};

GlobalSkins g_skins;


class Doom3ModelSkinCacheElement : public ModelSkin
{
  ModuleObservers m_observers;
  Doom3ModelSkin* m_skin;
public:
  Doom3ModelSkinCacheElement() : m_skin(0)
  {
  }
  void attach(ModuleObserver& observer)
  {
    m_observers.attach(observer);
    if(realised())
    {
      observer.realise();
    }
  }
  void detach(ModuleObserver& observer)
  {
    if(realised())
    {
      observer.unrealise();
    }
    m_observers.detach(observer);
  }
  bool realised() const
  {
    return m_skin != 0;
  }
  void realise(const char* name)
  {
    ASSERT_MESSAGE(!realised(), "Doom3ModelSkinCacheElement::realise: already realised");
    m_skin = &g_skins.getSkin(name);
    m_observers.realise();
  }
  void unrealise()
  {
    ASSERT_MESSAGE(realised(), "Doom3ModelSkinCacheElement::unrealise: not realised");
    m_observers.unrealise();
    m_skin = 0;
  }
  const char* getRemap(const char* name) const
  {
    ASSERT_MESSAGE(realised(), "Doom3ModelSkinCacheElement::getRemap: not realised");
    return m_skin->getRemap(name);
  }
  void forEachRemap(const SkinRemapCallback& callback) const
  {
    ASSERT_MESSAGE(realised(), "Doom3ModelSkinCacheElement::forEachRemap: not realised");
    m_skin->forEachRemap(callback);
  }
};

class Doom3ModelSkinCache : public ModelSkinCache, public ModuleObserver
{
  class CreateDoom3ModelSkin
  {
    Doom3ModelSkinCache& m_cache;
  public:
    explicit CreateDoom3ModelSkin(Doom3ModelSkinCache& cache)
      : m_cache(cache)
    {
    }
    Doom3ModelSkinCacheElement* construct(const CopiedString& name)
    {
      Doom3ModelSkinCacheElement* skin = new Doom3ModelSkinCacheElement;
      if(m_cache.realised())
      {
        skin->realise(name.c_str());
      }
      return skin;
    }
    void destroy(Doom3ModelSkinCacheElement* skin)
    {
      if(m_cache.realised())
      {
        skin->unrealise();
      }
      delete skin;
    }
  };

  typedef HashedCache<CopiedString, Doom3ModelSkinCacheElement, HashString, std::equal_to<CopiedString>, CreateDoom3ModelSkin> Cache;
  Cache m_cache;
  bool m_realised;

public:
  typedef ModelSkinCache Type;
  STRING_CONSTANT(Name, "*");
  ModelSkinCache* getTable()
  {
    return this;
  }

  Doom3ModelSkinCache() : m_cache(CreateDoom3ModelSkin(*this)), m_realised(false)
  {
    GlobalFileSystem().attach(*this);
  }
  ~Doom3ModelSkinCache()
  {
    GlobalFileSystem().detach(*this);
  }

  ModelSkin& capture(const char* name)
  {
    return *m_cache.capture(name);
  }
  void release(const char* name)
  {
    m_cache.release(name);
  }

  bool realised() const
  {
    return m_realised;
  }
  void realise()
  {
    g_skins.realise();
    m_realised = true;
    for(Cache::iterator i = m_cache.begin(); i != m_cache.end(); ++i)
    {
      (*i).value->realise((*i).key.c_str());
    }
  }
  void unrealise()
  {
    m_realised = false;
    for(Cache::iterator i = m_cache.begin(); i != m_cache.end(); ++i)
    {
      (*i).value->unrealise();
    }
    g_skins.unrealise();
  }
};

class Doom3ModelSkinCacheDependencies : public GlobalFileSystemModuleRef, public GlobalScripLibModuleRef
{
};

typedef SingletonModule<Doom3ModelSkinCache, Doom3ModelSkinCacheDependencies> Doom3ModelSkinCacheModule;

Doom3ModelSkinCacheModule g_Doom3ModelSkinCacheModule;

void Doom3ModelSkinCacheModule_selfRegister(ModuleServer& server)
{
  g_Doom3ModelSkinCacheModule.selfRegister();
}

