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

#include "scenegraph.h"

#include "debugging/debugging.h"

#include <map>
#include <set>
#include <vector>

#include "string/string.h"
#include "generic/callback.h"
#include "scenelib.h"
#include "instancelib.h"
#include "treemodel.h"

class StringEqualPredicate
{
  const char* m_string;
public:
  StringEqualPredicate(const char* string) : m_string(string)
  {
  }
  bool operator()(const char* other) const
  {
    return string_equal(m_string, other);
  }
};

template<std::size_t SIZE>
class TypeIdMap
{
  typedef const char* TypeName;
  typedef TypeName TypeNames[SIZE];
  TypeNames m_typeNames;
  TypeName* m_typeNamesEnd;

public:
  TypeIdMap() : m_typeNamesEnd(m_typeNames)
  {
  }
  TypeId getTypeId(const char* name)
  {
    TypeName* i = std::find_if(m_typeNames, m_typeNamesEnd, StringEqualPredicate(name));
    if(i == m_typeNamesEnd)
    {
      ASSERT_MESSAGE(m_typeNamesEnd != m_typeNames + SIZE, "reached maximum number of type names supported (" << Unsigned(SIZE) << ")");
      *m_typeNamesEnd++ = name;
    }
    return i - m_typeNames;
  }
};

class CompiledGraph : public scene::Graph, public scene::Instantiable::Observer
{
  typedef std::map<PathConstReference, scene::Instance*> InstanceMap;

  InstanceMap m_instances;
  scene::Instantiable::Observer* m_observer;
  typedef std::set<Callback> BoundsChangedCallbacks;
  BoundsChangedCallbacks m_boundsChanged;
  scene::Path m_rootpath;
  std::vector<Callback> m_sceneChangedCallbacks;

  TypeIdMap<NODETYPEID_MAX> m_nodeTypeIds;
  TypeIdMap<INSTANCETYPEID_MAX> m_instanceTypeIds;

public:

  CompiledGraph(scene::Instantiable::Observer* observer)
    : m_observer(observer)
  {
  }

  void addSceneChangedCallback(const Callback& callback)
  {
    m_sceneChangedCallbacks.push_back(callback);
  }
  void sceneChanged()
  {
    std::for_each(m_sceneChangedCallbacks.begin(), m_sceneChangedCallbacks.end(), CallbackInvoke());
  }

  scene::Node& root()
  {
    ASSERT_MESSAGE(!m_rootpath.empty(), "scenegraph root does not exist");
    return *m_rootpath.top();
  }
  void insert_root(scene::Node& root)
  {
    //globalOutputStream() << "insert_root\n";

    ASSERT_MESSAGE(m_rootpath.empty(), "scenegraph root already exists");

    root.IncRef();

    Node_traverseSubgraph(root, InstanceSubgraphWalker(this, scene::Path(), 0));

    m_rootpath.push(makeReference(root));
  }
  void erase_root()
  {
    //globalOutputStream() << "erase_root\n";

    ASSERT_MESSAGE(!m_rootpath.empty(), "scenegraph root does not exist");

    scene::Node& root = *m_rootpath.top();

    m_rootpath.pop();

    Node_traverseSubgraph(root, UninstanceSubgraphWalker(this, scene::Path()));

    root.DecRef();
  }
  void boundsChanged()
  {
    std::for_each(m_boundsChanged.begin(), m_boundsChanged.end(), CallbackInvoke());
  }

  void traverse(const Walker& walker)
  {
    traverse_subgraph(walker, m_instances.begin());
  }

  void traverse_subgraph(const Walker& walker, const scene::Path& start)
  {
    if(!m_instances.empty())
    {
      traverse_subgraph(walker, m_instances.find(PathConstReference(start)));
    }
  }

  scene::Instance* find(const scene::Path& path)
  {
    InstanceMap::iterator i = m_instances.find(PathConstReference(path));
    if(i == m_instances.end())
    {
      return 0;
    }
    return (*i).second;
  }

  void insert(scene::Instance* instance)
  {
    m_instances.insert(InstanceMap::value_type(PathConstReference(instance->path()), instance));

    m_observer->insert(instance);
  }
  void erase(scene::Instance* instance)
  {
    m_observer->erase(instance);

    m_instances.erase(PathConstReference(instance->path()));
  }

  void addBoundsChangedCallback(const Callback& boundsChanged)
  {
    ASSERT_MESSAGE(m_boundsChanged.find(boundsChanged) == m_boundsChanged.end(), "bounds-changed callback already registered");
    m_boundsChanged.insert(boundsChanged);
  }
  void removeBoundsChangedCallback(const Callback& boundsChanged)
  {
    ASSERT_MESSAGE(m_boundsChanged.find(boundsChanged) != m_boundsChanged.end(), "bounds-changed callback not registered");
    m_boundsChanged.erase(boundsChanged);
  }

  TypeId getNodeTypeId(const char* name)
  {
    return m_nodeTypeIds.getTypeId(name);
  }

  TypeId getInstanceTypeId(const char* name)
  {
    return m_instanceTypeIds.getTypeId(name);
  }

private:

  bool pre(const Walker& walker, const InstanceMap::iterator& i)
  {
    return walker.pre(i->first, *i->second);
  }

  void post(const Walker& walker, const InstanceMap::iterator& i)
  {
    walker.post(i->first, *i->second);
  }

  void traverse_subgraph(const Walker& walker, InstanceMap::iterator i)
  {
    Stack<InstanceMap::iterator> stack;
    if(i != m_instances.end())
    {
      const std::size_t startSize = (*i).first.get().size();
      do
      {
        if(i != m_instances.end()
          && stack.size() < ((*i).first.get().size() - startSize + 1))
        {
          stack.push(i);
          ++i;
          if(!pre(walker, stack.top()))
          {
            // skip subgraph
            while(i != m_instances.end()
              && stack.size() < ((*i).first.get().size() - startSize + 1))
            {
              ++i;
            }
          }
        }
        else
        {
          post(walker, stack.top());
          stack.pop();
        }
      }
      while(!stack.empty());
    }
  }
};

namespace
{
  CompiledGraph* g_sceneGraph;
  GraphTreeModel* g_tree_model;
}

GraphTreeModel* scene_graph_get_tree_model()
{
  return g_tree_model;
}


class SceneGraphObserver : public scene::Instantiable::Observer
{
public:
  void insert(scene::Instance* instance)
  {
    g_sceneGraph->sceneChanged();
    graph_tree_model_insert(g_tree_model, *instance);
  }
  void erase(scene::Instance* instance)
  {
    g_sceneGraph->sceneChanged();
    graph_tree_model_erase(g_tree_model, *instance);
  }
};

SceneGraphObserver g_SceneGraphObserver;

void SceneGraph_Construct()
{
  g_tree_model = graph_tree_model_new();

  g_sceneGraph = new CompiledGraph(&g_SceneGraphObserver);
}

void SceneGraph_Destroy()
{
  delete g_sceneGraph;

  graph_tree_model_delete(g_tree_model);
}


#include "modulesystem/singletonmodule.h"
#include "modulesystem/moduleregistry.h"

class SceneGraphAPI
{
  scene::Graph* m_scenegraph;
public:
  typedef scene::Graph Type;
  STRING_CONSTANT(Name, "*");

  SceneGraphAPI()
  {
    SceneGraph_Construct();

    m_scenegraph = g_sceneGraph;
  }
  ~SceneGraphAPI()
  {
    SceneGraph_Destroy();
  }
  scene::Graph* getTable()
  {
    return m_scenegraph;
  }
};

typedef SingletonModule<SceneGraphAPI> SceneGraphModule;
typedef Static<SceneGraphModule> StaticSceneGraphModule;
StaticRegisterModule staticRegisterSceneGraph(StaticSceneGraphModule::instance());

