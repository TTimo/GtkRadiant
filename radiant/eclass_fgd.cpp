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

#include "eclass_fgd.h"

#include "debugging/debugging.h"

#include <map>

#include "ifilesystem.h"
#include "iscriplib.h"
#include "qerplugin.h"

#include "string/string.h"
#include "eclasslib.h"
#include "os/path.h"
#include "os/dir.h"
#include "stream/stringstream.h"
#include "moduleobservers.h"
#include "stringio.h"
#include "stream/textfilestream.h"

namespace
{
  typedef std::map<const char*, EntityClass*, RawStringLessNoCase> EntityClasses;
  EntityClasses g_EntityClassFGD_classes;
  typedef std::map<const char*, EntityClass*, RawStringLess> BaseClasses;
  BaseClasses g_EntityClassFGD_bases;
  EntityClass	*g_EntityClassFGD_bad = 0;
  typedef std::map<CopiedString, ListAttributeType> ListAttributeTypes;
  ListAttributeTypes g_listTypesFGD;
}


void EntityClassFGD_clear()
{
  for(BaseClasses::iterator i = g_EntityClassFGD_bases.begin(); i != g_EntityClassFGD_bases.end(); ++i)
  {
    (*i).second->free((*i).second);
  }
  g_EntityClassFGD_bases.clear();
  g_listTypesFGD.clear();
}

EntityClass* EntityClassFGD_insertUniqueBase(EntityClass* entityClass)
{
  std::pair<BaseClasses::iterator, bool> result = g_EntityClassFGD_bases.insert(BaseClasses::value_type(entityClass->name(), entityClass));
  if(!result.second)
  {
    globalErrorStream() << "duplicate base class: " << makeQuoted(entityClass->name()) << "\n";
    //eclass_capture_state(entityClass);
    //entityClass->free(entityClass);
  }
  return (*result.first).second;
}

EntityClass* EntityClassFGD_insertUnique(EntityClass* entityClass)
{
  EntityClassFGD_insertUniqueBase(entityClass);
  std::pair<EntityClasses::iterator, bool> result = g_EntityClassFGD_classes.insert(EntityClasses::value_type(entityClass->name(), entityClass));
  if(!result.second)
  {
    globalErrorStream() << "duplicate entity class: " << makeQuoted(entityClass->name()) << "\n";
    eclass_capture_state(entityClass);
    entityClass->free(entityClass);
  }
  return (*result.first).second;
}

void EntityClassFGD_forEach(EntityClassVisitor& visitor)
{
  for(EntityClasses::iterator i = g_EntityClassFGD_classes.begin(); i != g_EntityClassFGD_classes.end(); ++i)
  {
    visitor.visit((*i).second);
  }
}

inline bool EntityClassFGD_parseToken(Tokeniser& tokeniser, const char* token)
{
  return string_equal(tokeniser.getToken(), token);
}

#define PARSE_ERROR "error parsing entity class definition"

void EntityClassFGD_parseSplitString(Tokeniser& tokeniser, CopiedString& string)
{
  StringOutputStream buffer(256);
  for(;;)
  {
    buffer << tokeniser.getToken();
    if(!string_equal(tokeniser.getToken(), "+"))
    {
      tokeniser.ungetToken();
      string = buffer.c_str();
      return;
    }
  }
}

void EntityClassFGD_parseClass(Tokeniser& tokeniser, bool fixedsize, bool isBase)
{
  EntityClass* entityClass = Eclass_Alloc();
  entityClass->free = &Eclass_Free;
  entityClass->fixedsize = fixedsize;
  entityClass->inheritanceResolved = false;
  entityClass->mins = Vector3(-8, -8, -8);
  entityClass->maxs = Vector3(8, 8, 8);

  for(;;)
  {
    const char* property = tokeniser.getToken();
    if(string_equal(property, "="))
    {
      break;
    }
    else if(string_equal(property, "base"))
    {
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, "("), PARSE_ERROR);
      for(;;)
      {
        const char* base = tokeniser.getToken();
        if(string_equal(base, ")"))
        {
          break;
        }
        else if(!string_equal(base, ","))
        {
          entityClass->m_parent.push_back(base);
        }
      }
    }
    else if(string_equal(property, "size"))
    {
      entityClass->sizeSpecified = true;
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, "("), PARSE_ERROR);
      Tokeniser_getFloat(tokeniser, entityClass->mins.x());
      Tokeniser_getFloat(tokeniser, entityClass->mins.y());
      Tokeniser_getFloat(tokeniser, entityClass->mins.z());
      const char* token = tokeniser.getToken();
      if(string_equal(token, ","))
      {
        Tokeniser_getFloat(tokeniser, entityClass->maxs.x());
        Tokeniser_getFloat(tokeniser, entityClass->maxs.y());
        Tokeniser_getFloat(tokeniser, entityClass->maxs.z());
        ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ")"), PARSE_ERROR);
      }
      else
      {
        entityClass->maxs = entityClass->mins;
        vector3_negate(entityClass->mins);
        ASSERT_MESSAGE(string_equal(token, ")"), "");
      }
    }
    else if(string_equal(property, "color"))
    {
      entityClass->colorSpecified = true;
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, "("), PARSE_ERROR);
      Tokeniser_getFloat(tokeniser, entityClass->color.x());
      entityClass->color.x() /= 256.0;
      Tokeniser_getFloat(tokeniser, entityClass->color.y());
      entityClass->color.y() /= 256.0;
      Tokeniser_getFloat(tokeniser, entityClass->color.z());
      entityClass->color.z() /= 256.0;
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ")"), PARSE_ERROR);
    }
    else if(string_equal(property, "iconsprite"))
    {
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, "("), PARSE_ERROR);
      StringOutputStream buffer(256);
      buffer << PathCleaned(tokeniser.getToken());
      entityClass->m_modelpath = buffer.c_str();
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ")"), PARSE_ERROR);
    }
    else if(string_equal(property, "sprite")
      || string_equal(property, "decal")
      // hl2 below
      || string_equal(property, "overlay")
      || string_equal(property, "light")
      || string_equal(property, "keyframe")
      || string_equal(property, "animator")
      || string_equal(property, "quadbounds"))
    {
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, "("), PARSE_ERROR);
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ")"), PARSE_ERROR);
    }
    // hl2 below
    else if(string_equal(property, "sphere")
      || string_equal(property, "sweptplayerhull")
      || string_equal(property, "studio")
      || string_equal(property, "studioprop")
      || string_equal(property, "lightprop")
      || string_equal(property, "lightcone")
      || string_equal(property, "sidelist"))
    {
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, "("), PARSE_ERROR);
      if(string_equal(tokeniser.getToken(), ")"))
      {
        tokeniser.ungetToken();
      }
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ")"), PARSE_ERROR);
    }
    else if(string_equal(property, "line")
      || string_equal(property, "cylinder"))
    {
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, "("), PARSE_ERROR);
      //const char* r =
      tokeniser.getToken();
      //const char* g =
      tokeniser.getToken();
      //const char* b =
      tokeniser.getToken();
      for(;;)
      {
        if(string_equal(tokeniser.getToken(), ")"))
        {
          tokeniser.ungetToken();
          break;
        }
        //const char* name =
        tokeniser.getToken();
      }
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ")"), PARSE_ERROR);
    }
    else if(string_equal(property, "wirebox"))
    {
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, "("), PARSE_ERROR);
      //const char* mins =
      tokeniser.getToken();
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ","), PARSE_ERROR);
      //const char* maxs =
      tokeniser.getToken();
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ")"), PARSE_ERROR);
    }
    else if(string_equal(property, "halfgridsnap"))
    {
    }
    else
    {
      ERROR_MESSAGE(PARSE_ERROR);
    }
  }

  entityClass->m_name = tokeniser.getToken();

  if(!isBase)
  {
    ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ":"), PARSE_ERROR);

    EntityClassFGD_parseSplitString(tokeniser, entityClass->m_comments);
  }

  tokeniser.nextLine();

  ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, "["), PARSE_ERROR);

  tokeniser.nextLine();

  for(;;)
  {
    CopiedString key = tokeniser.getToken();
    if(string_equal(key.c_str(), "]"))
    {
      tokeniser.nextLine();
      break;
    }

    if(string_equal_nocase(key.c_str(), "input")
      || string_equal_nocase(key.c_str(), "output"))
    {
      const char* name = tokeniser.getToken();
      if(!string_equal(name, "("))
      {
        ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, "("), PARSE_ERROR);
        //const char* type =
        tokeniser.getToken();
        ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ")"), PARSE_ERROR);
        const char* descriptionSeparator = tokeniser.getToken();
        if(string_equal(descriptionSeparator, ":"))
        {
          CopiedString description;
          EntityClassFGD_parseSplitString(tokeniser, description);
        }
        else
        {
          tokeniser.ungetToken();
        }
        tokeniser.nextLine();
        continue;
      }
    }

    ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, "("), PARSE_ERROR);
    CopiedString type = tokeniser.getToken();
    ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ")"), PARSE_ERROR);

    if(string_equal_nocase(type.c_str(), "flags"))
    {
      EntityClassAttribute attribute;

      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, "="), PARSE_ERROR);
      tokeniser.nextLine();
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, "["), PARSE_ERROR);
      tokeniser.nextLine();
      for(;;)
      {
        const char* flag = tokeniser.getToken();
        if(string_equal(flag, "]"))
        {
          tokeniser.nextLine();
          break;
        }
        else
        {
          ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ":"), PARSE_ERROR);
          //const char* name =
          tokeniser.getToken();
          {
            const char* defaultSeparator = tokeniser.getToken();
            if(string_equal(defaultSeparator, ":"))
            {
              tokeniser.getToken();
              {
                const char* descriptionSeparator = tokeniser.getToken();
                if(string_equal(descriptionSeparator, ":"))
                {
                  EntityClassFGD_parseSplitString(tokeniser, attribute.m_description);
                }
                else
                {
                  tokeniser.ungetToken();
                }
              }
            }
            else
            {
              tokeniser.ungetToken();
            }
          }
        }
        tokeniser.nextLine();
      }
      EntityClass_insertAttribute(*entityClass, key.c_str(), attribute);
    }
    else if(string_equal_nocase(type.c_str(), "choices"))
    {
      EntityClassAttribute attribute;

      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ":"), PARSE_ERROR);
      attribute.m_name = tokeniser.getToken();
      const char* valueSeparator = tokeniser.getToken();
      if(string_equal(valueSeparator, ":"))
      {
        const char* value = tokeniser.getToken();
        if(!string_equal(value, ":"))
        {
          attribute.m_value = value;
        }
        else
        {
          tokeniser.ungetToken();
        }
        {
          const char* descriptionSeparator = tokeniser.getToken();
          if(string_equal(descriptionSeparator, ":"))
          {
            EntityClassFGD_parseSplitString(tokeniser, attribute.m_description);
          }
          else
          {
            tokeniser.ungetToken();
          }
        }
      }
      else
      {
        tokeniser.ungetToken();
      }
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, "="), PARSE_ERROR);
      tokeniser.nextLine();
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, "["), PARSE_ERROR);
      tokeniser.nextLine();

      StringOutputStream listTypeName(64);
      listTypeName << entityClass->m_name.c_str() << "_" << attribute.m_name.c_str();
      attribute.m_type = listTypeName.c_str();

      ListAttributeType& listType = g_listTypesFGD[listTypeName.c_str()];

      for(;;)
      {
        const char* value = tokeniser.getToken();
        if(string_equal(value, "]"))
        {
          tokeniser.nextLine();
          break;
        }
        else
        {
          CopiedString tmp(value);
          ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ":"), PARSE_ERROR);
          const char* name = tokeniser.getToken();
          listType.push_back(name, tmp.c_str());
        }
        tokeniser.nextLine();
      }

      for(ListAttributeType::const_iterator i = listType.begin(); i != listType.end(); ++i)
      {
        if(string_equal(attribute.m_value.c_str(), (*i).first.c_str()))
        {
          attribute.m_value = (*i).second.c_str();
        }
      }

      EntityClass_insertAttribute(*entityClass, key.c_str(), attribute);
    }
    else if(string_equal_nocase(type.c_str(), "decal"))
    {
    }
    else if(string_equal_nocase(type.c_str(), "string")
      || string_equal_nocase(type.c_str(), "integer")
      || string_equal_nocase(type.c_str(), "studio")
      || string_equal_nocase(type.c_str(), "sprite")
      || string_equal_nocase(type.c_str(), "color255")
      || string_equal_nocase(type.c_str(), "target_source")
      || string_equal_nocase(type.c_str(), "target_destination")
      || string_equal_nocase(type.c_str(), "sound")
      // hl2 below
      || string_equal_nocase(type.c_str(), "angle")
      || string_equal_nocase(type.c_str(), "origin")
      || string_equal_nocase(type.c_str(), "float")
      || string_equal_nocase(type.c_str(), "node_dest")
      || string_equal_nocase(type.c_str(), "filterclass")
      || string_equal_nocase(type.c_str(), "vector")
      || string_equal_nocase(type.c_str(), "sidelist")
      || string_equal_nocase(type.c_str(), "material")
      || string_equal_nocase(type.c_str(), "vecline")
      || string_equal_nocase(type.c_str(), "axis")
      || string_equal_nocase(type.c_str(), "npcclass")
      || string_equal_nocase(type.c_str(), "target_name_or_class")
      || string_equal_nocase(type.c_str(), "pointentityclass")
      || string_equal_nocase(type.c_str(), "scene"))
    {
      if(!string_equal(tokeniser.getToken(), "readonly"))
      {
        tokeniser.ungetToken();
      }

      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ":"), PARSE_ERROR);
      const char* attributeType = "string";
      if(string_equal_nocase(type.c_str(), "studio"))
      {
        attributeType = "model";
      }

      EntityClassAttribute attribute;
      attribute.m_type = attributeType;
      attribute.m_name = tokeniser.getToken();

      const char* defaultSeparator = tokeniser.getToken();
      if(string_equal(defaultSeparator, ":"))
      {
        const char* value = tokeniser.getToken();
        if(!string_equal(value, ":"))
        {
          attribute.m_value = value;
        }
        else
        {
          tokeniser.ungetToken();
        }

        {
          const char* descriptionSeparator = tokeniser.getToken();
          if(string_equal(descriptionSeparator, ":"))
          {
            EntityClassFGD_parseSplitString(tokeniser, attribute.m_description);
          }
          else
          {
            tokeniser.ungetToken();
          }
        }
      }
      else
      {
        tokeniser.ungetToken();
      }
      EntityClass_insertAttribute(*entityClass, key.c_str(), attribute);
    }
    else
    {
      ERROR_MESSAGE("unknown key type: " << makeQuoted(type.c_str()));
    }
    tokeniser.nextLine();
  }

  if(isBase)
  {
    EntityClassFGD_insertUniqueBase(entityClass);
  }
  else
  {
    EntityClassFGD_insertUnique(entityClass);
  }
}

void EntityClassFGD_loadFile(const char* filename);

void EntityClassFGD_parse(TextInputStream& inputStream, const char* path)
{
  Tokeniser& tokeniser = GlobalScriptLibrary().m_pfnNewScriptTokeniser(inputStream);

  tokeniser.nextLine();

  for(;;)
  {
    const char* blockType = tokeniser.getToken();
    if(blockType == 0)
    {
      break;
    }
    if(string_equal(blockType, "@SolidClass"))
    {
      EntityClassFGD_parseClass(tokeniser, false, false);
    }
    else if(string_equal(blockType, "@BaseClass"))
    {
      EntityClassFGD_parseClass(tokeniser, false, true);
    }
    else if(string_equal(blockType, "@PointClass")
      // hl2 below
      || string_equal(blockType, "@KeyFrameClass")
      || string_equal(blockType, "@MoveClass")
      || string_equal(blockType, "@FilterClass")
      || string_equal(blockType, "@NPCClass"))
    {
      EntityClassFGD_parseClass(tokeniser, true, false);
    }
    // hl2 below
    else if(string_equal(blockType, "@include"))
    {
      StringOutputStream includePath(256);
      includePath << StringRange(path, path_get_filename_start(path));
      includePath << tokeniser.getToken();
      EntityClassFGD_loadFile(includePath.c_str());
    }
    else if(string_equal(blockType, "@mapsize"))
    {
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, "("), PARSE_ERROR);
      //const char* min =
      tokeniser.getToken();
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ","), PARSE_ERROR);
      //const char* max =
      tokeniser.getToken();
      ASSERT_MESSAGE(EntityClassFGD_parseToken(tokeniser, ")"), PARSE_ERROR);
    }
    else
    {
      ERROR_MESSAGE("unknown block type: " << makeQuoted(blockType));
    }
  }

  tokeniser.release();
}


void EntityClassFGD_loadFile(const char* filename)
{
	TextFileInputStream file(filename);
  if(!file.failed())
  {
    globalOutputStream() << "parsing entity classes from " << makeQuoted(filename) << "\n";

    EntityClassFGD_parse(file, filename);
  }
}

EntityClass* EntityClassFGD_findOrInsert(const char *name, bool has_brushes)
{
	ASSERT_NOTNULL(name);

  if(string_empty(name))
  {
    return g_EntityClassFGD_bad;
  }

  EntityClasses::iterator i = g_EntityClassFGD_classes.find(name);
  if(i != g_EntityClassFGD_classes.end()
    //&& string_equal((*i).first, name)
    )
  {
    return (*i).second;
  }

	EntityClass* e = EntityClass_Create_Default(name, has_brushes);
	return EntityClassFGD_insertUnique(e);
}

const ListAttributeType* EntityClassFGD_findListType(const char *name)
{
  ListAttributeTypes::iterator i = g_listTypesFGD.find(name);
  if(i != g_listTypesFGD.end())
  {
    return &(*i).second;
  }
  return 0;

}


void EntityClassFGD_resolveInheritance(EntityClass* derivedClass)
{
  if(derivedClass->inheritanceResolved == false)
  {
    derivedClass->inheritanceResolved = true;
    for(StringList::iterator j = derivedClass->m_parent.begin(); j != derivedClass->m_parent.end(); ++j)
    {
      BaseClasses::iterator i = g_EntityClassFGD_bases.find((*j).c_str());
      if(i == g_EntityClassFGD_bases.end())
      {
        globalErrorStream() << "failed to find entityDef " << makeQuoted((*j).c_str()) << " inherited by "  << makeQuoted(derivedClass->m_name.c_str()) << "\n";
      }
      else
      {
        EntityClass* parentClass = (*i).second;
        EntityClassFGD_resolveInheritance(parentClass);
        if(!derivedClass->colorSpecified)
        {
          derivedClass->colorSpecified = parentClass->colorSpecified;
          derivedClass->color = parentClass->color;
        }
        if(!derivedClass->sizeSpecified)
        {
          derivedClass->sizeSpecified = parentClass->sizeSpecified;
          derivedClass->mins = parentClass->mins;
          derivedClass->maxs = parentClass->maxs;
        }

        for(EntityClassAttributes::iterator k = parentClass->m_attributes.begin(); k != parentClass->m_attributes.end(); ++k)
        {
          EntityClass_insertAttribute(*derivedClass, (*k).first.c_str(), (*k).second);
        }
      }
    }
  }
}

class EntityClassFGD : public ModuleObserver
{
  std::size_t m_unrealised;
  ModuleObservers m_observers;
public:
  EntityClassFGD() : m_unrealised(3)
  {
  }
  void realise()
  {
    if(--m_unrealised == 0)
    {
      StringOutputStream filename(256);
      filename << GlobalRadiant().getGameToolsPath() << GlobalRadiant().getGameName() << "/halflife.fgd";
      EntityClassFGD_loadFile(filename.c_str());

      {
        for(EntityClasses::iterator i = g_EntityClassFGD_classes.begin(); i != g_EntityClassFGD_classes.end(); ++i)
        {
          EntityClassFGD_resolveInheritance((*i).second);
          if((*i).second->fixedsize && string_empty((*i).second->m_modelpath.c_str()))
          {
            if(!(*i).second->sizeSpecified)
            {
              globalErrorStream() << "size not specified for entity class: " << makeQuoted((*i).second->m_name.c_str()) << '\n';
            }
            if(!(*i).second->colorSpecified)
            {
              globalErrorStream() << "color not specified for entity class: " << makeQuoted((*i).second->m_name.c_str()) << '\n';
            }
          }
        }
      }
      {
        for(BaseClasses::iterator i = g_EntityClassFGD_bases.begin(); i != g_EntityClassFGD_bases.end(); ++i)
        {
          eclass_capture_state((*i).second);
        }
      }

      m_observers.realise();
    }
  }
  void unrealise()
  {
    if(++m_unrealised == 1)
    {
      m_observers.unrealise();
      EntityClassFGD_clear();
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

EntityClassFGD g_EntityClassFGD;

void EntityClassFGD_attach(ModuleObserver& observer)
{
  g_EntityClassFGD.attach(observer);
}
void EntityClassFGD_detach(ModuleObserver& observer)
{
  g_EntityClassFGD.detach(observer);
}

void EntityClassFGD_realise()
{
  g_EntityClassFGD.realise();
}
void EntityClassFGD_unrealise()
{
  g_EntityClassFGD.unrealise();
}

void EntityClassFGD_construct()
{
  // start by creating the default unknown eclass
  g_EntityClassFGD_bad = EClass_Create("UNKNOWN_CLASS", Vector3(0.0f, 0.5f, 0.0f), "");

  EntityClassFGD_realise();
}

void EntityClassFGD_destroy()
{
  EntityClassFGD_unrealise();

  g_EntityClassFGD_bad->free(g_EntityClassFGD_bad);
}

class EntityClassFGDDependencies : public GlobalFileSystemModuleRef, public GlobalShaderCacheModuleRef, public GlobalRadiantModuleRef
{
};

class EntityClassFGDAPI
{
  EntityClassManager m_eclassmanager;
public:
  typedef EntityClassManager Type;
  STRING_CONSTANT(Name, "halflife");

  EntityClassFGDAPI()
  {
    EntityClassFGD_construct();

    m_eclassmanager.findOrInsert = &EntityClassFGD_findOrInsert;
    m_eclassmanager.findListType = &EntityClassFGD_findListType;
    m_eclassmanager.forEach = &EntityClassFGD_forEach;
    m_eclassmanager.attach = &EntityClassFGD_attach;
    m_eclassmanager.detach = &EntityClassFGD_detach;
    m_eclassmanager.realise = &EntityClassFGD_realise;
    m_eclassmanager.unrealise = &EntityClassFGD_unrealise;

    GlobalRadiant().attachGameToolsPathObserver(g_EntityClassFGD);
    GlobalRadiant().attachGameNameObserver(g_EntityClassFGD);
  }
  ~EntityClassFGDAPI()
  {
    GlobalRadiant().detachGameNameObserver(g_EntityClassFGD);
    GlobalRadiant().detachGameToolsPathObserver(g_EntityClassFGD);

    EntityClassFGD_destroy();
  }
  EntityClassManager* getTable()
  {
    return &m_eclassmanager;
  }
};

#include "modulesystem/singletonmodule.h"
#include "modulesystem/moduleregistry.h"

typedef SingletonModule<EntityClassFGDAPI, EntityClassFGDDependencies> EntityClassFGDModule;
typedef Static<EntityClassFGDModule> StaticEntityClassFGDModule;
StaticRegisterModule staticRegisterEntityClassFGD(StaticEntityClassFGDModule::instance());
