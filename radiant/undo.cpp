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

#include "undo.h"

#include "debugging/debugging.h"
#include "warnings.h"

#include "iundo.h"
#include "preferencesystem.h"
#include "string/string.h"
#include "generic/callback.h"
#include "preferences.h"
#include "stringio.h"

#include <list>
#include <map>
#include <set>

#include "timer.h"

class DebugScopeTimer
{
  Timer m_timer;
  const char* m_operation;
public:
  DebugScopeTimer(const char* operation)
    : m_operation(operation)
  {
    m_timer.start();
  }
  ~DebugScopeTimer()
  {
    unsigned int elapsed = m_timer.elapsed_msec();
    if(elapsed > 0)
    {
      globalOutputStream() << m_operation << ": " << elapsed << " msec\n";
    }
  }
};


class RadiantUndoSystem : public UndoSystem
{
  INTEGER_CONSTANT(MAX_UNDO_LEVELS, 1024);

  class Snapshot
  {
    class StateApplicator
    {
    public:
      Undoable* m_undoable;
    private:
      UndoMemento* m_data;
    public:

      StateApplicator(Undoable* undoable, UndoMemento* data)
        : m_undoable(undoable), m_data(data)
      {
      }
      void restore()
      {
        m_undoable->importState(m_data);
      }
      void release()
      {
        m_data->release();
      }
    };
  
    typedef std::list<StateApplicator> states_t;
    states_t m_states;

  public:
    bool empty() const
    {
      return m_states.empty();
    }
    std::size_t size() const
    {
      return m_states.size();
    }
    void save(Undoable* undoable)
    {
      m_states.push_front(StateApplicator(undoable, undoable->exportState()));
    }
    void restore()
    {
      for(states_t::iterator i = m_states.begin(); i != m_states.end(); ++i)
      {
        (*i).restore();
      }
    }
    void release()
    {
      for(states_t::iterator i = m_states.begin(); i != m_states.end(); ++i)
      {
        (*i).release();
      }
    }
  };

  struct Operation
  {
    Snapshot m_snapshot;
    CopiedString m_command;

    Operation(const char* command)
      : m_command(command)
    {
    }
    ~Operation()
    {
      m_snapshot.release();
    }
  };


  class UndoStack
  {
    //! Note: using std::list instead of vector/deque, to avoid copying of undos
    typedef std::list<Operation*> Operations;

    Operations m_stack;
    Operation* m_pending;

  public:
    UndoStack() : m_pending(0)
    {
    }
    ~UndoStack()
    {
      clear();
    }
    bool empty() const
    {
      return m_stack.empty();
    }
    std::size_t size() const
    {
      return m_stack.size();
    }
    Operation* back()
    {
      return m_stack.back();
    }
    const Operation* back() const
    {
      return m_stack.back();
    }
    Operation* front()
    {
      return m_stack.front();
    }
    const Operation* front() const
    {
      return m_stack.front();
    }
    void pop_front()
    {
      delete m_stack.front();
      m_stack.pop_front();
    }
    void pop_back()
    {
      delete m_stack.back();
      m_stack.pop_back();
    }
    void clear()
    {
      if(!m_stack.empty())
      {
        for(Operations::iterator i = m_stack.begin(); i != m_stack.end(); ++i)
        {
          delete *i;
        }
        m_stack.clear();
      }
    }
    void start(const char* command)
    {
      if(m_pending != 0)
      {
        delete m_pending;
      }
      m_pending = new Operation(command);
    }
    bool finish(const char* command)
    {
      if(m_pending != 0)
      {
        delete m_pending;
        m_pending = 0;
        return false;
      }
      else
      {
        ASSERT_MESSAGE(!m_stack.empty(), "undo stack empty");
        m_stack.back()->m_command = command;
        return true;
      }
    }
    void save(Undoable* undoable)
    {
      if(m_pending != 0)
      {
        m_stack.push_back(m_pending);
        m_pending = 0;
      }
      back()->m_snapshot.save(undoable);
    }
  };

  UndoStack m_undo_stack;
  UndoStack m_redo_stack;

  class UndoStackFiller : public UndoObserver
  {
    UndoStack* m_stack;
  public:

    UndoStackFiller()
      : m_stack(0)
    {
    }
    void save(Undoable* undoable)
    {
      ASSERT_NOTNULL(undoable);

      if(m_stack != 0)
      {
        m_stack->save(undoable);
        m_stack = 0;
      }
    }
    void setStack(UndoStack* stack)
    {
      m_stack = stack;
    }
  };

  typedef std::map<Undoable*, UndoStackFiller> undoables_t;
  undoables_t m_undoables;

  void mark_undoables(UndoStack* stack)
  {
    for(undoables_t::iterator i = m_undoables.begin(); i != m_undoables.end(); ++i)
    {
      (*i).second.setStack(stack);
    }
  }

  std::size_t m_undo_levels;

  typedef std::set<UndoTracker*> Trackers;
  Trackers m_trackers;
public:
  RadiantUndoSystem()
    : m_undo_levels(64)
  {
  }
  ~RadiantUndoSystem()
  {
    clear();
  }
  UndoObserver* observer(Undoable* undoable)
  {
    ASSERT_NOTNULL(undoable);

    return &m_undoables[undoable];
  }
  void release(Undoable* undoable)
  {
    ASSERT_NOTNULL(undoable);

    m_undoables.erase(undoable);
  }
  void setLevels(std::size_t levels)
  {
    if(levels > MAX_UNDO_LEVELS())
    {
      levels = MAX_UNDO_LEVELS();
    }

    while(m_undo_stack.size() > levels)
    {
      m_undo_stack.pop_front();
    }
    m_undo_levels = levels;
  }
  std::size_t getLevels() const
  {
    return m_undo_levels;
  }
  std::size_t size() const
  {
    return m_undo_stack.size();
  }
  void startUndo()
  {
    m_undo_stack.start("unnamedCommand");
    mark_undoables(&m_undo_stack);
  }
  bool finishUndo(const char* command)
  {
    bool changed = m_undo_stack.finish(command);
    mark_undoables(0);
    return changed;
  }
  void startRedo()
  {
    m_redo_stack.start("unnamedCommand");
    mark_undoables(&m_redo_stack);
  }
  bool finishRedo(const char* command)
  {
    bool changed = m_redo_stack.finish(command);
    mark_undoables(0);
    return changed;
  }
  void start()
  {
    m_redo_stack.clear();
    if(m_undo_stack.size() == m_undo_levels)
    {
      m_undo_stack.pop_front();
    }
    startUndo();
    trackersBegin();
  }
  void finish(const char* command)
  {
    if(finishUndo(command))
    {
      globalOutputStream() << command << '\n';
    }
  }
  void undo()
  {
    if(m_undo_stack.empty())
    {
      globalOutputStream() << "Undo: no undo available\n";
    }
    else
    {
      Operation* operation = m_undo_stack.back();
      globalOutputStream() << "Undo: " << operation->m_command.c_str() << "\n";

      startRedo();
      trackersUndo();
      operation->m_snapshot.restore();
      finishRedo(operation->m_command.c_str());
      m_undo_stack.pop_back();
    }
  }
  void redo()
  {
    if(m_redo_stack.empty())
    {
      globalOutputStream() << "Redo: no redo available\n";
    }
    else
    {
      Operation* operation = m_redo_stack.back();
      globalOutputStream() << "Redo: " << operation->m_command.c_str() << "\n";

      startUndo();
      trackersRedo();
      operation->m_snapshot.restore();
      finishUndo(operation->m_command.c_str());
      m_redo_stack.pop_back();
    }
  }
  void clear()
  {
    mark_undoables(0);
    m_undo_stack.clear();
    m_redo_stack.clear();
    trackersClear();
  }
  void trackerAttach(UndoTracker& tracker)
  {
    ASSERT_MESSAGE(m_trackers.find(&tracker) == m_trackers.end(), "undo tracker already attached");
    m_trackers.insert(&tracker);
  }
  void trackerDetach(UndoTracker& tracker)
  {
    ASSERT_MESSAGE(m_trackers.find(&tracker) != m_trackers.end(), "undo tracker cannot be detached");
    m_trackers.erase(&tracker);
  }
  void trackersClear() const
  {
    for(Trackers::const_iterator i = m_trackers.begin(); i != m_trackers.end(); ++i)
    {
      (*i)->clear();
    }
  }
  void trackersBegin() const
  {
    for(Trackers::const_iterator i = m_trackers.begin(); i != m_trackers.end(); ++i)
    {
      (*i)->begin();
    }
  }
  void trackersUndo() const
  {
    for(Trackers::const_iterator i = m_trackers.begin(); i != m_trackers.end(); ++i)
    {
      (*i)->undo();
    }
  }
  void trackersRedo() const
  {
    for(Trackers::const_iterator i = m_trackers.begin(); i != m_trackers.end(); ++i)
    {
      (*i)->redo();
    }
  }
};



void UndoLevels_importString(RadiantUndoSystem& undo, const char* value)
{
  int levels;
  Int_importString(levels, value);
  undo.setLevels(levels);
}
typedef ReferenceCaller1<RadiantUndoSystem, const char*, UndoLevels_importString> UndoLevelsImportStringCaller;
void UndoLevels_exportString(const RadiantUndoSystem& undo, const StringImportCallback& importer)
{
  Int_exportString(static_cast<int>(undo.getLevels()), importer);
}
typedef ConstReferenceCaller1<RadiantUndoSystem, const StringImportCallback&, UndoLevels_exportString> UndoLevelsExportStringCaller;

#include "generic/callback.h"

void UndoLevelsImport(RadiantUndoSystem& self, int value)
{
  self.setLevels(value);
}
typedef ReferenceCaller1<RadiantUndoSystem, int, UndoLevelsImport> UndoLevelsImportCaller;
void UndoLevelsExport(const RadiantUndoSystem& self, const IntImportCallback& importCallback)
{
  importCallback(static_cast<int>(self.getLevels()));
}
typedef ConstReferenceCaller1<RadiantUndoSystem, const IntImportCallback&, UndoLevelsExport> UndoLevelsExportCaller;


void Undo_constructPreferences(RadiantUndoSystem& undo, PreferencesPage& page)
{
  page.appendSpinner("Undo Queue Size", 64, 0, 1024, IntImportCallback(UndoLevelsImportCaller(undo)), IntExportCallback(UndoLevelsExportCaller(undo)));
}
void Undo_constructPage(RadiantUndoSystem& undo, PreferenceGroup& group)
{
  PreferencesPage page(group.createPage("Undo", "Undo Queue Settings"));
  Undo_constructPreferences(undo, page);
}
void Undo_registerPreferencesPage(RadiantUndoSystem& undo)
{
  PreferencesDialog_addSettingsPage(ReferenceCaller1<RadiantUndoSystem, PreferenceGroup&, Undo_constructPage>(undo));
}

class UndoSystemDependencies : public GlobalPreferenceSystemModuleRef
{
};

class UndoSystemAPI
{
  RadiantUndoSystem m_undosystem;
public:
  typedef UndoSystem Type;
  STRING_CONSTANT(Name, "*");

  UndoSystemAPI()
  {
    GlobalPreferenceSystem().registerPreference("UndoLevels", makeIntStringImportCallback(UndoLevelsImportCaller(m_undosystem)), makeIntStringExportCallback(UndoLevelsExportCaller(m_undosystem)));

    Undo_registerPreferencesPage(m_undosystem);
  }
  UndoSystem* getTable()
  {
    return &m_undosystem;
  }
};

#include "modulesystem/singletonmodule.h"
#include "modulesystem/moduleregistry.h"

typedef SingletonModule<UndoSystemAPI, UndoSystemDependencies> UndoSystemModule;
typedef Static<UndoSystemModule> StaticUndoSystemModule;
StaticRegisterModule staticRegisterUndoSystem(StaticUndoSystemModule::instance());










class undoable_test : public Undoable
{
  struct state_type : public UndoMemento
  {
    state_type() : test_data(0)
    {
    }
    state_type(const state_type& other) : UndoMemento(other), test_data(other.test_data)
    {
    }
    void release()
    {
      delete this;
    }

    int test_data;
  };
  state_type m_state;
  UndoObserver* m_observer;
public:
  undoable_test()
    : m_observer(GlobalUndoSystem().observer(this))
  {
  }
  ~undoable_test()
  {
    GlobalUndoSystem().release(this);
  }
  UndoMemento* exportState() const
  {
    return new state_type(m_state);
  }
  void importState(const UndoMemento* state)
  {
    ASSERT_NOTNULL(state);

    m_observer->save(this);
    m_state = *(static_cast<const state_type*>(state));
  }

  void mutate(unsigned int data)
  {
    m_observer->save(this);
    m_state.test_data = data;
  }
};

#if 0

class TestUndo
{
public:
  TestUndo()
  {
    undoable_test test;
    GlobalUndoSystem().begin("bleh");
    test.mutate(3);
    GlobalUndoSystem().begin("blah");
    test.mutate(4);
    GlobalUndoSystem().undo();
    GlobalUndoSystem().undo();
    GlobalUndoSystem().redo();
    GlobalUndoSystem().redo();
  }
};

TestUndo g_TestUndo;

#endif

