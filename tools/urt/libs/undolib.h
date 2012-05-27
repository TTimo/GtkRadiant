
#if !defined (INCLUDED_UNDOLIB_H)
#define INCLUDED_UNDOLIB_H

#include "iundo.h"
#include "mapfile.h"
#include "warnings.h"
#include "generic/callback.h"

template<typename Copyable>
class BasicUndoData : public UndoData
{
  Copyable m_data;
public:
  BasicUndoData(const Copyable& data)
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

  UndoData* exportState() const
  {
    return new BasicUndoData<Copyable>(m_object);
  }
  void importState(const UndoData* state)
  {
    save();
    m_importCallback((static_cast<const BasicUndoData<Copyable>*>(state))->get());
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

  UndoData* exportState() const
  {
    return new BasicUndoData<Copyable>(m_object);
  }
  void importState(const UndoData* state)
  {
    save();
    m_object = (static_cast<const BasicUndoData<Copyable>*>(state))->get();
  }
};

#endif
