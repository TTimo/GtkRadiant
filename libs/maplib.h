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

#if !defined (INCLUDED_MAPLIB_H)
#define INCLUDED_MAPLIB_H

#include "nameable.h"
#include "mapfile.h"

#include "traverselib.h"
#include "transformlib.h"
#include "scenelib.h"
#include "string/string.h"
#include "instancelib.h"
#include "selectionlib.h"
#include "generic/callback.h"


class NameableString : public Nameable
{
  CopiedString m_name;
public:
  NameableString(const char* name)
    : m_name(name)
  {
  }

  const char* name() const
  {
    return m_name.c_str();
  }
  void attach(const NameCallback& callback)
  {
  }
  void detach(const NameCallback& callback)
  {
  }
};


class UndoFileChangeTracker : public UndoTracker, public MapFile
{
  std::size_t m_size;
  std::size_t m_saved;
  typedef void (UndoFileChangeTracker::*Pending)();
  Pending m_pending;
  Callback m_changed;

public:
  UndoFileChangeTracker() : m_size(0), m_saved(MAPFILE_MAX_CHANGES), m_pending(0)
  {
  }
  void print()
  {
    globalOutputStream() << "saved: " << Unsigned(m_saved) << " size: " << Unsigned(m_size) << "\n";
  }

  void push()
  {
    ++m_size;
    m_changed();
    //print();
  }
  void pop()
  {
    --m_size;
    m_changed();
    //print();
  }
  void pushOperation()
  {
    if(m_size < m_saved)
    {
      // redo queue has been flushed.. it is now impossible to get back to the saved state via undo/redo
      m_saved = MAPFILE_MAX_CHANGES;
    }
    push();
  }
  void clear()
  {
    m_size = 0;
    m_changed();
    //print();
  }
  void begin()
  {
    m_pending = Pending(&UndoFileChangeTracker::pushOperation);
  }
  void undo()
  {
    m_pending = Pending(&UndoFileChangeTracker::pop);
  }
  void redo()
  {
    m_pending = Pending(&UndoFileChangeTracker::push);
  }

  void changed()
  {
    if(m_pending != 0)
    {
      ((*this).*m_pending)();
      m_pending = 0;
    }
  }

  void save()
  {
    m_saved = m_size;
    m_changed();
  }
  bool saved() const
  {
    return m_saved == m_size;
  }

  void setChangedCallback(const Callback& changed)
  {
    m_changed = changed;
    m_changed();
  }

  std::size_t changes() const
  {
    return m_size;
  }
};


class MapRoot : public scene::Node::Symbiot, public scene::Instantiable, public scene::Traversable::Observer
{
  class TypeCasts
  {
    NodeTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      NodeStaticCast<MapRoot, scene::Instantiable>::install(m_casts);
      NodeContainedCast<MapRoot, scene::Traversable>::install(m_casts);
      NodeContainedCast<MapRoot, TransformNode>::install(m_casts);
      NodeContainedCast<MapRoot, Nameable>::install(m_casts);
      NodeContainedCast<MapRoot, MapFile>::install(m_casts);
    }
    NodeTypeCastTable& get()
    {
      return m_casts;
    }
  };

  scene::Node m_node;
  IdentityTransform m_transform;
  TraversableNodeSet m_traverse;
  InstanceSet m_instances;
  typedef SelectableInstance Instance;
  NameableString m_name;
  UndoFileChangeTracker m_changeTracker;
public:
  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  scene::Traversable& get(NullType<scene::Traversable>)
  {
    return m_traverse;
  }
  TransformNode& get(NullType<TransformNode>)
  {
    return m_transform;
  }
  Nameable& get(NullType<Nameable>)
  {
    return m_name;
  }
  MapFile& get(NullType<MapFile>)
  {
    return m_changeTracker;
  }

  MapRoot(const char* name) : m_node(this, this, StaticTypeCasts::instance().get()), m_name(name)
  {
    m_node.m_isRoot = true;

    m_traverse.attach(this);

    GlobalUndoSystem().trackerAttach(m_changeTracker);
  }
  ~MapRoot()
  {
  }
  void release()
  {
    GlobalUndoSystem().trackerDetach(m_changeTracker);

    m_traverse.detach(this);
    delete this;
  }
  scene::Node& node()
  {
    return m_node;
  }

  InstanceCounter m_instanceCounter;
  void instanceAttach(const scene::Path& path)
  {
    if(++m_instanceCounter.m_count == 1)
    {
      m_traverse.instanceAttach(path_find_mapfile(path.begin(), path.end()));
    }
  }
  void instanceDetach(const scene::Path& path)
  {
    if(--m_instanceCounter.m_count == 0)
    {
      m_traverse.instanceDetach(path_find_mapfile(path.begin(), path.end()));
    }
  }

  void insert(scene::Node& child)
  {
    m_instances.insert(child);
  }
  void erase(scene::Node& child)
  {
    m_instances.erase(child);
  }

  scene::Node& clone() const
  {
    return (new MapRoot(*this))->node();
  }

  scene::Instance* create(const scene::Path& path, scene::Instance* parent)
  {
    return new Instance(path, parent);
  }
  void forEachInstance(const scene::Instantiable::Visitor& visitor)
  {
    m_instances.forEachInstance(visitor);
  }
  void insert(scene::Instantiable::Observer* observer, const scene::Path& path, scene::Instance* instance)
  {
    m_instances.insert(observer, path, instance);
    instanceAttach(path);
  }
  scene::Instance* erase(scene::Instantiable::Observer* observer, const scene::Path& path)
  {
    instanceDetach(path);
    return m_instances.erase(observer, path);
  }
};

inline void MapRoot_construct()
{
}

inline void MapRoot_destroy()
{
}

inline NodeSmartReference NewMapRoot(const char* name)
{
  return NodeSmartReference((new MapRoot(name))->node());
}


#endif
