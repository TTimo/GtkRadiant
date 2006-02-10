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

#if !defined (INCLUDED_ENTITYLIB_H)
#define INCLUDED_ENTITYLIB_H

#include "ireference.h"
#include "debugging/debugging.h"

#include "ientity.h"
#include "irender.h"
#include "igl.h"
#include "selectable.h"

#include "generic/callback.h"
#include "math/vector.h"
#include "math/aabb.h"
#include "undolib.h"
#include "string/string.h"
#include "generic/referencecounted.h"
#include "scenelib.h"
#include "container/container.h"
#include "eclasslib.h"

#include <list>
#include <set>

inline void arrow_draw(const Vector3& origin, const Vector3& direction)
{
  Vector3 up(0, 0, 1);
  Vector3 left(-direction[1], direction[0], 0);

	Vector3 endpoint(vector3_added(origin, vector3_scaled(direction, 32.0)));

  Vector3 tip1(vector3_added(vector3_added(endpoint, vector3_scaled(direction, -8.0)), vector3_scaled(up, -4.0)));
	Vector3 tip2(vector3_added(tip1, vector3_scaled(up, 8.0)));
  Vector3 tip3(vector3_added(vector3_added(endpoint, vector3_scaled(direction, -8.0)), vector3_scaled(left, -4.0)));
	Vector3 tip4(vector3_added(tip3, vector3_scaled(left, 8.0)));

  glBegin (GL_LINES);

  glVertex3fv(vector3_to_array(origin));
  glVertex3fv(vector3_to_array(endpoint));

  glVertex3fv(vector3_to_array(endpoint));
  glVertex3fv(vector3_to_array(tip1));

  glVertex3fv(vector3_to_array(endpoint));
  glVertex3fv(vector3_to_array(tip2));

  glVertex3fv(vector3_to_array(endpoint));
  glVertex3fv(vector3_to_array(tip3));

  glVertex3fv(vector3_to_array(endpoint));
  glVertex3fv(vector3_to_array(tip4));

  glVertex3fv(vector3_to_array(tip1));
  glVertex3fv(vector3_to_array(tip3));

  glVertex3fv(vector3_to_array(tip3));
  glVertex3fv(vector3_to_array(tip2));

  glVertex3fv(vector3_to_array(tip2));
  glVertex3fv(vector3_to_array(tip4));

  glVertex3fv(vector3_to_array(tip4));
  glVertex3fv(vector3_to_array(tip1));

  glEnd();
}

class SelectionIntersection;

inline void aabb_testselect(const AABB& aabb, SelectionTest& test, SelectionIntersection& best)
{
  const IndexPointer::index_type indices[24] = {
    2, 1, 5, 6,
    1, 0, 4, 5,
    0, 1, 2, 3,
    3, 7, 4, 0,
    3, 2, 6, 7,
    7, 6, 5, 4,
  };

  Vector3 points[8];
  aabb_corners(aabb, points);
  test.TestQuads(VertexPointer(reinterpret_cast<VertexPointer::pointer>(points), sizeof(Vector3)), IndexPointer(indices, 24), best);
}

inline void aabb_draw_wire(const Vector3 points[8])
{
  typedef std::size_t index_t;
  index_t indices[24] = {
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7,
  };
#if 1
  glVertexPointer(3, GL_FLOAT, 0, points);
  glDrawElements(GL_LINES, sizeof(indices)/sizeof(index_t), GL_UNSIGNED_INT, indices);
#else
  glBegin(GL_LINES);
  for(std::size_t i = 0; i < sizeof(indices)/sizeof(index_t); ++i)
  {
    glVertex3fv(points[indices[i]]);
  }
  glEnd();
#endif
}

inline void aabb_draw_flatshade(const Vector3 points[8])
{
  glBegin(GL_QUADS);

  glNormal3fv(vector3_to_array(aabb_normals[0]));
  glVertex3fv(vector3_to_array(points[2]));
  glVertex3fv(vector3_to_array(points[1]));
  glVertex3fv(vector3_to_array(points[5]));
  glVertex3fv(vector3_to_array(points[6]));

  glNormal3fv(vector3_to_array(aabb_normals[1]));
  glVertex3fv(vector3_to_array(points[1]));
  glVertex3fv(vector3_to_array(points[0]));
  glVertex3fv(vector3_to_array(points[4]));
  glVertex3fv(vector3_to_array(points[5]));

  glNormal3fv(vector3_to_array(aabb_normals[2]));
  glVertex3fv(vector3_to_array(points[0]));
  glVertex3fv(vector3_to_array(points[1]));
  glVertex3fv(vector3_to_array(points[2]));
  glVertex3fv(vector3_to_array(points[3]));

  glNormal3fv(vector3_to_array(aabb_normals[3]));
  glVertex3fv(vector3_to_array(points[0]));
  glVertex3fv(vector3_to_array(points[3]));
  glVertex3fv(vector3_to_array(points[7]));
  glVertex3fv(vector3_to_array(points[4]));

  glNormal3fv(vector3_to_array(aabb_normals[4]));
  glVertex3fv(vector3_to_array(points[3]));
  glVertex3fv(vector3_to_array(points[2]));
  glVertex3fv(vector3_to_array(points[6]));
  glVertex3fv(vector3_to_array(points[7]));

  glNormal3fv(vector3_to_array(aabb_normals[5]));
  glVertex3fv(vector3_to_array(points[7]));
  glVertex3fv(vector3_to_array(points[6]));
  glVertex3fv(vector3_to_array(points[5]));
  glVertex3fv(vector3_to_array(points[4]));

  glEnd();
}

inline void aabb_draw_wire(const AABB& aabb)
{
  Vector3 points[8];
	aabb_corners(aabb, points);
  aabb_draw_wire(points);
}

inline void aabb_draw_flatshade(const AABB& aabb)
{
  Vector3 points[8];
	aabb_corners(aabb, points);
  aabb_draw_flatshade(points);
}

inline void aabb_draw_textured(const AABB& aabb)
{
  Vector3 points[8];
	aabb_corners(aabb, points);

  glBegin(GL_QUADS);

  glNormal3fv(vector3_to_array(aabb_normals[0]));
  glTexCoord2fv(aabb_texcoord_topleft);
  glVertex3fv(vector3_to_array(points[2]));
  glTexCoord2fv(aabb_texcoord_topright);
  glVertex3fv(vector3_to_array(points[1]));
  glTexCoord2fv(aabb_texcoord_botright);
  glVertex3fv(vector3_to_array(points[5]));
  glTexCoord2fv(aabb_texcoord_botleft);
  glVertex3fv(vector3_to_array(points[6]));

  glNormal3fv(vector3_to_array(aabb_normals[1]));
  glTexCoord2fv(aabb_texcoord_topleft);
  glVertex3fv(vector3_to_array(points[1]));
  glTexCoord2fv(aabb_texcoord_topright);
  glVertex3fv(vector3_to_array(points[0]));
  glTexCoord2fv(aabb_texcoord_botright);
  glVertex3fv(vector3_to_array(points[4]));
  glTexCoord2fv(aabb_texcoord_botleft);
  glVertex3fv(vector3_to_array(points[5]));

  glNormal3fv(vector3_to_array(aabb_normals[2]));
  glTexCoord2fv(aabb_texcoord_topleft);
  glVertex3fv(vector3_to_array(points[0]));
  glTexCoord2fv(aabb_texcoord_topright);
  glVertex3fv(vector3_to_array(points[1]));
  glTexCoord2fv(aabb_texcoord_botright);
  glVertex3fv(vector3_to_array(points[2]));
  glTexCoord2fv(aabb_texcoord_botleft);
  glVertex3fv(vector3_to_array(points[3]));

  glNormal3fv(vector3_to_array(aabb_normals[3]));
  glTexCoord2fv(aabb_texcoord_topleft);
  glVertex3fv(vector3_to_array(points[0]));
  glTexCoord2fv(aabb_texcoord_topright);
  glVertex3fv(vector3_to_array(points[3]));
  glTexCoord2fv(aabb_texcoord_botright);
  glVertex3fv(vector3_to_array(points[7]));
  glTexCoord2fv(aabb_texcoord_botleft);
  glVertex3fv(vector3_to_array(points[4]));

  glNormal3fv(vector3_to_array(aabb_normals[4]));
  glTexCoord2fv(aabb_texcoord_topleft);
  glVertex3fv(vector3_to_array(points[3]));
  glTexCoord2fv(aabb_texcoord_topright);
  glVertex3fv(vector3_to_array(points[2]));
  glTexCoord2fv(aabb_texcoord_botright);
  glVertex3fv(vector3_to_array(points[6]));
  glTexCoord2fv(aabb_texcoord_botleft);
  glVertex3fv(vector3_to_array(points[7]));

  glNormal3fv(vector3_to_array(aabb_normals[5]));
  glTexCoord2fv(aabb_texcoord_topleft);
  glVertex3fv(vector3_to_array(points[7]));
  glTexCoord2fv(aabb_texcoord_topright);
  glVertex3fv(vector3_to_array(points[6]));
  glTexCoord2fv(aabb_texcoord_botright);
  glVertex3fv(vector3_to_array(points[5]));
  glTexCoord2fv(aabb_texcoord_botleft);
  glVertex3fv(vector3_to_array(points[4]));

  glEnd();
}

inline void aabb_draw_solid(const AABB& aabb, RenderStateFlags state)
{
  if(state & RENDER_TEXTURE)
  {
    aabb_draw_textured(aabb);
  }
  else
  {
    aabb_draw_flatshade(aabb);
  }
}

inline void aabb_draw(const AABB& aabb, RenderStateFlags state)
{
  if(state & RENDER_FILL)
  {
    aabb_draw_solid(aabb, state);
  }
  else
  {
    aabb_draw_wire(aabb);
  }
}

class RenderableSolidAABB : public OpenGLRenderable
{
  const AABB& m_aabb;
public:
  RenderableSolidAABB(const AABB& aabb) : m_aabb(aabb)
  {
  }
  void render(RenderStateFlags state) const
  {
    aabb_draw_solid(m_aabb, state);
  }
};

class RenderableWireframeAABB : public OpenGLRenderable
{
  const AABB& m_aabb;
public:
  RenderableWireframeAABB(const AABB& aabb) : m_aabb(aabb)
  {
  }
  void render(RenderStateFlags state) const
  {
    aabb_draw_wire(m_aabb);
  }
};


typedef Callback1<const char*> KeyObserver;

/// \brief A key/value pair of strings.
///
/// - Notifies observers when value changes - value changes to "" on destruction.
/// - Provides undo support through the global undo system.
class KeyValue
{
  typedef UnsortedSet<KeyObserver> KeyObservers;

  std::size_t m_refcount;
  KeyObservers m_observers;
  CopiedString m_string;
  const char* m_empty;
  ObservedUndoableObject<CopiedString> m_undo;
  static EntityCreator::KeyValueChangedFunc m_entityKeyValueChanged;
public:

  KeyValue(const char* string, const char* empty)
    : m_refcount(0), m_string(string), m_empty(empty), m_undo(m_string, UndoImportCaller(*this))
  {
    notify();
  }
  ~KeyValue()
  {
    ASSERT_MESSAGE(m_observers.empty(), "KeyValue::~KeyValue: observers still attached");
  }

  static void setKeyValueChangedFunc(EntityCreator::KeyValueChangedFunc func)
  {
    m_entityKeyValueChanged = func;
  }

  void IncRef()
  {
    ++m_refcount;
  }
  void DecRef()
  {
    if(--m_refcount == 0)
    {
      delete this;
    }
  }

  void instanceAttach(MapFile* map)
  {
    m_undo.instanceAttach(map);
  }
  void instanceDetach(MapFile* map)
  {
    m_undo.instanceDetach(map);
  }

  void attach(const KeyObserver& observer)
  {
    (*m_observers.insert(observer))(c_str());
  }
  void detach(const KeyObserver& observer)
  {
    observer(m_empty);
    m_observers.erase(observer);
  }
  const char* c_str() const
  {
    if(string_empty(m_string.c_str()))
    {
      return m_empty;
    }
    return m_string.c_str();
  }
  void assign(const char* other)
  {
    if(!string_equal(m_string.c_str(), other))
    {
      m_undo.save();
      m_string = other;
      notify();
    }
  }

  void notify()
  {
    m_entityKeyValueChanged();
    KeyObservers::reverse_iterator i = m_observers.rbegin();
    while(i != m_observers.rend())
    {
      (*i++)(c_str());
    }
  }

  void importState(const CopiedString& string)
  {
    m_string = string;

    notify();
  }
  typedef MemberCaller1<KeyValue, const CopiedString&, &KeyValue::importState> UndoImportCaller;
};

/// \brief An unsorted list of key/value pairs.
///
/// - Notifies observers when a pair is inserted or removed.
/// - Provides undo support through the global undo system.
/// - New keys are appended to the end of the list.
class EntityKeyValues : public Entity
{
public:
  typedef KeyValue Value;

  class Observer
  {
  public:
    virtual void insert(const char* key, Value& value) = 0;
    virtual void erase(const char* key, Value& value) = 0;
  };

private:
  static EntityCreator::KeyValueChangedFunc m_entityKeyValueChanged;
  static Counter* m_counter;

  EntityClass* m_eclass;

  typedef SmartPointer<KeyValue> KeyValuePtr;
  typedef UnsortedMap<CopiedString, KeyValuePtr > KeyValues;
  KeyValues m_keyValues;

  typedef UnsortedSet<Observer*> Observers;
  Observers m_observers;

  ObservedUndoableObject<KeyValues> m_undo;
  bool m_instanced;

  bool m_observerMutex;

  void notifyInsert(const char* key, Value& value)
  {
    m_observerMutex = true;
    for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i)
    {
      (*i)->insert(key, value);
    }
    m_observerMutex = false;
  }
  void notifyErase(const char* key, Value& value)
  {
    m_observerMutex = true;
    for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i)
    {
      (*i)->erase(key, value);
    }
    m_observerMutex = false;
  }
  void forEachKeyValue_notifyInsert()
  {
    for(KeyValues::const_iterator i = m_keyValues.begin(); i != m_keyValues.end(); ++i)
    {
      notifyInsert((*i).first.c_str(), *(*i).second);
    }
  }
  void forEachKeyValue_notifyErase()
  {
    for(KeyValues::const_iterator i = m_keyValues.begin(); i != m_keyValues.end(); ++i)
    {
      notifyErase((*i).first.c_str(), *(*i).second);
    }
  }

  void insert(const char* key, const KeyValuePtr& keyValue)
  {
    KeyValues::iterator i = m_keyValues.insert(KeyValues::value_type(key, keyValue));
    notifyInsert(key, *(*i).second);

    if(m_instanced)
    {
      (*i).second->instanceAttach(m_undo.map());
    }
  }

  void insert(const char* key, const char* value)
  {
    KeyValues::iterator i = m_keyValues.find(key);
    if(i != m_keyValues.end())
    {
      (*i).second->assign(value);
    }
    else
    {
      m_undo.save();
      insert(key, KeyValuePtr(new KeyValue(value, EntityClass_valueForKey(*m_eclass, key))));
    }
  }

  void erase(KeyValues::iterator i)
  {
    if(m_instanced)
    {
      (*i).second->instanceDetach(m_undo.map());
    }

    CopiedString key((*i).first);
    KeyValuePtr value((*i).second);
    m_keyValues.erase(i);
    notifyErase(key.c_str(), *value);
  }

  void erase(const char* key)
  {
    KeyValues::iterator i = m_keyValues.find(key);
    if(i != m_keyValues.end())
    {
      m_undo.save();
      erase(i);
    }
  }

public:
  bool m_isContainer;

  EntityKeyValues(EntityClass* eclass) :
    m_eclass(eclass),
    m_undo(m_keyValues, UndoImportCaller(*this)),
    m_instanced(false),
    m_observerMutex(false),
    m_isContainer(!eclass->fixedsize)
  {
  }
  EntityKeyValues(const EntityKeyValues& other) :
    Entity(other),
    m_eclass(&other.getEntityClass()),
    m_undo(m_keyValues, UndoImportCaller(*this)),
    m_instanced(false),
    m_observerMutex(false),
    m_isContainer(other.m_isContainer)
  {
    for(KeyValues::const_iterator i = other.m_keyValues.begin(); i != other.m_keyValues.end(); ++i)
    {
      insert((*i).first.c_str(), (*i).second->c_str());
    }
  }
  ~EntityKeyValues()
  {
    ASSERT_MESSAGE(m_observers.empty(), "EntityKeyValues::~EntityKeyValues: observers still attached");
  }

  static void setKeyValueChangedFunc(EntityCreator::KeyValueChangedFunc func)
  {
    m_entityKeyValueChanged = func;
    KeyValue::setKeyValueChangedFunc(func);
  }
  static void setCounter(Counter* counter)
  {
    m_counter = counter;
  }

  void importState(const KeyValues& keyValues)
  {
    for(KeyValues::iterator i = m_keyValues.begin(); i != m_keyValues.end();)
    {
      erase(i++);
    }

    for(KeyValues::const_iterator i = keyValues.begin(); i != keyValues.end(); ++i)
    {
      insert((*i).first.c_str(), (*i).second);
    }

    m_entityKeyValueChanged();
  }
  typedef MemberCaller1<EntityKeyValues, const KeyValues&, &EntityKeyValues::importState> UndoImportCaller;

  void attach(Observer& observer)
  {
    ASSERT_MESSAGE(!m_observerMutex, "observer cannot be attached during iteration");
    m_observers.insert(&observer);
    for(KeyValues::const_iterator i = m_keyValues.begin(); i != m_keyValues.end(); ++i)
    {
      observer.insert((*i).first.c_str(), *(*i).second);
    }
  }
  void detach(Observer& observer)
  {
    ASSERT_MESSAGE(!m_observerMutex, "observer cannot be detached during iteration");
    m_observers.erase(&observer);
    for(KeyValues::const_iterator i = m_keyValues.begin(); i != m_keyValues.end(); ++i)
    {
      observer.erase((*i).first.c_str(), *(*i).second);
    }
  }

  void forEachKeyValue_instanceAttach(MapFile* map)
  {
    for(KeyValues::const_iterator i = m_keyValues.begin(); i != m_keyValues.end(); ++i)
    {
      (*i).second->instanceAttach(map);
    }
  }
  void forEachKeyValue_instanceDetach(MapFile* map)
  {
    for(KeyValues::const_iterator i = m_keyValues.begin(); i != m_keyValues.end(); ++i)
    {
      (*i).second->instanceDetach(map);
    }
  }

  void instanceAttach(MapFile* map)
  {
    if(m_counter != 0)
    {
      m_counter->increment();
    }

    m_instanced = true;
    forEachKeyValue_instanceAttach(map);
    m_undo.instanceAttach(map);
  }
  void instanceDetach(MapFile* map)
  {
    if(m_counter != 0)
    {
      m_counter->decrement();
    }

    m_undo.instanceDetach(map);
    forEachKeyValue_instanceDetach(map);
    m_instanced = false;
  }

  // entity
  EntityClass& getEntityClass() const
  {
    return *m_eclass;
  }
  void forEachKeyValue(Visitor& visitor) const
  {
    for(KeyValues::const_iterator i = m_keyValues.begin(); i != m_keyValues.end(); ++i)
    {
      visitor.visit((*i).first.c_str(), (*i).second->c_str());
    }
  }
  void setKeyValue(const char* key, const char* value)
  {
    if(value[0] == '\0'
      /*|| string_equal(EntityClass_valueForKey(*m_eclass, key), value)*/) // don't delete values equal to default
    {
      erase(key);
    }
    else
    {
      insert(key, value);
    }
    m_entityKeyValueChanged();
  }
  const char* getKeyValue(const char* key) const
  {
    KeyValues::const_iterator i = m_keyValues.find(key);
    if(i != m_keyValues.end())
    {
      return (*i).second->c_str();
    }

    return EntityClass_valueForKey(*m_eclass, key);
  }

  bool isContainer() const
  {
    return m_isContainer;
  }
};

/// \brief A Resource reference with a controlled lifetime.
/// \brief The resource is released when the ResourceReference is destroyed.
class ResourceReference
{
  CopiedString m_name;
  Resource* m_resource;
public:
  ResourceReference(const char* name)
    : m_name(name)
  {
    capture();
  }
  ResourceReference(const ResourceReference& other)
    : m_name(other.m_name)
  {
    capture();
  }
  ResourceReference& operator=(const ResourceReference& other)
  {
    ResourceReference tmp(other);
    tmp.swap(*this);
    return *this;
  }
  ~ResourceReference()
  {
    release();
  }

  void capture()
  {
    m_resource = GlobalReferenceCache().capture(m_name.c_str());
  }
  void release()
  {
    GlobalReferenceCache().release(m_name.c_str());
  }

  const char* getName() const
  {
    return m_name.c_str();
  }
  void setName(const char* name)
  {
    ResourceReference tmp(name);
    tmp.swap(*this);
  }

  void swap(ResourceReference& other)
  {
    std::swap(m_resource, other.m_resource);
    std::swap(m_name, other.m_name);
  }

  void attach(ModuleObserver& observer)
  {
    m_resource->attach(observer);
  }
  void detach(ModuleObserver& observer)
  {
    m_resource->detach(observer);
  }

  Resource* get()
  {
    return m_resource;
  }
};

namespace std
{
  /// \brief Swaps the values of \p self and \p other.
  /// Overloads std::swap.
  inline void swap(ResourceReference& self, ResourceReference& other)
  {
    self.swap(other);
  }
}

#endif
