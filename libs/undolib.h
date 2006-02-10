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

#if !defined (INCLUDED_UNDOLIB_H)
#define INCLUDED_UNDOLIB_H

#include "iundo.h"
#include "mapfile.h"
#include "warnings.h"
#include "generic/callback.h"

template<typename Copyable>
class BasicUndoMemento : public UndoMemento
{
  Copyable m_data;
public:
  BasicUndoMemento(const Copyable& data)
    : m_data(data)
  {
  }

  void release()
  {
    delete this;
  }

  const Copyable& get() const
  {
    return m_data;
  }
};


template<typename Copyable>
class ObservedUndoableObject : public Undoable
{
  typedef Callback1<const Copyable&> ImportCallback;

  Copyable& m_object;
  ImportCallback m_importCallback;
  UndoObserver* m_undoQueue;
  MapFile* m_map;
public:

  ObservedUndoableObject<Copyable>(Copyable& object, const ImportCallback& importCallback)
    : m_object(object), m_importCallback(importCallback), m_undoQueue(0), m_map(0)
  {
  }
  ~ObservedUndoableObject()
  {
  }

  MapFile* map()
  {
    return m_map;
  }

  void instanceAttach(MapFile* map)
  {
    m_map = map;
    m_undoQueue = GlobalUndoSystem().observer(this);
  }
  void instanceDetach(MapFile* map)
  {
    m_map = 0;
    m_undoQueue = 0;
    GlobalUndoSystem().release(this);
  }

  void save()
  {
    if(m_map != 0)
    {
      m_map->changed();
    }
    if(m_undoQueue != 0)
    {
      m_undoQueue->save(this);
    }
  }

  UndoMemento* exportState() const
  {
    return new BasicUndoMemento<Copyable>(m_object);
  }
  void importState(const UndoMemento* state)
  {
    save();
    m_importCallback((static_cast<const BasicUndoMemento<Copyable>*>(state))->get());
  }
};

template<typename Copyable>
class UndoableObject : public Undoable
{
  Copyable& m_object;
  UndoObserver* m_undoQueue;
  MapFile* m_map;

public:
  UndoableObject(Copyable& object)
    : m_object(object), m_undoQueue(0), m_map(0)
  {}
  ~UndoableObject()
  {
  }

  void instanceAttach(MapFile* map)
  {
    m_map = map;
    m_undoQueue = GlobalUndoSystem().observer(this);
  }
  void instanceDetach(MapFile* map)
  {
    m_map = 0;
    m_undoQueue = 0;
    GlobalUndoSystem().release(this);
  }

  void save()
  {
    if(m_map != 0)
    {
      m_map->changed();
    }
    if(m_undoQueue != 0)
    {
      m_undoQueue->save(this);
    }
  }

  UndoMemento* exportState() const
  {
    return new BasicUndoMemento<Copyable>(m_object);
  }
  void importState(const UndoMemento* state)
  {
    save();
    m_object = (static_cast<const BasicUndoMemento<Copyable>*>(state))->get();
  }
};

#endif
