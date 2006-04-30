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

#if !defined(INCLUDED_IENTITY_H)
#define INCLUDED_IENTITY_H

#include "generic/constant.h"

#include "string/string.h"
#include "scenelib.h"

class EntityClass;

typedef Callback1<const char*> KeyObserver;

class EntityKeyValue
{
public:
  virtual const char* c_str() const = 0;
  virtual void assign(const char* other) = 0;
  virtual void attach(const KeyObserver& observer) = 0;
  virtual void detach(const KeyObserver& observer) = 0;
};

class Entity
{
public:
  STRING_CONSTANT(Name, "Entity");

  class Observer
  {
  public:
    virtual void insert(const char* key, EntityKeyValue& value) = 0;
    virtual void erase(const char* key, EntityKeyValue& value) = 0;
    virtual void clear() { };
  };

  class Visitor
  {
  public:
    virtual void visit(const char* key, const char* value) = 0;
  };

  virtual const EntityClass& getEntityClass() const = 0;
  virtual void forEachKeyValue(Visitor& visitor) const = 0;
  virtual void setKeyValue(const char* key, const char* value) = 0;
  virtual const char* getKeyValue(const char* key) const = 0;
  virtual bool isContainer() const = 0;
  virtual void attach(Observer& observer) = 0;
  virtual void detach(Observer& observer) = 0;
};

class EntityCopyingVisitor : public Entity::Visitor
{
  Entity& m_entity;
public:
  EntityCopyingVisitor(Entity& entity)
    : m_entity(entity)
  {
  }

  void visit(const char* key, const char* value)
  {
    if(!string_equal(key, "classname"))
    {
      m_entity.setKeyValue(key, value);
    }
  }
};

inline Entity* Node_getEntity(scene::Node& node)
{
  return NodeTypeCast<Entity>::cast(node);
}


template<typename value_type>
class Stack;
template<typename Contained>
class Reference;

namespace scene
{
  class Node;
}

typedef Reference<scene::Node> NodeReference;

namespace scene
{
  typedef Stack<NodeReference> Path;
}

class Counter;

class EntityCreator
{
public:
  INTEGER_CONSTANT(Version, 2);
  STRING_CONSTANT(Name, "entity");

  virtual scene::Node& createEntity(EntityClass* eclass) = 0;

  typedef void (*KeyValueChangedFunc)();
  virtual void setKeyValueChangedFunc(KeyValueChangedFunc func) = 0;

  virtual void setCounter(Counter* counter) = 0;

  virtual void connectEntities(const scene::Path& e1, const scene::Path& e2) = 0;

  virtual void setLightRadii(bool lightRadii) = 0;
  virtual bool getLightRadii() = 0;
  virtual void setShowNames(bool showNames) = 0;
  virtual bool getShowNames() = 0;
  virtual void setShowAngles(bool showAngles) = 0;
  virtual bool getShowAngles() = 0;

  virtual void printStatistics() const = 0;
};

#include "modulesystem.h"

template<typename Type>
class GlobalModule;
typedef GlobalModule<EntityCreator> GlobalEntityModule;

template<typename Type>
class GlobalModuleRef;
typedef GlobalModuleRef<EntityCreator> GlobalEntityModuleRef;

inline EntityCreator& GlobalEntityCreator()
{
  return GlobalEntityModule::getTable();
}

#endif
