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

#include "treemodel.h"

#include "debugging/debugging.h"

#include <map>
#include <gtk/gtktreemodel.h>
#include <gtk/gtktreednd.h>
#include <gtk/gtkmain.h>

#include "iscenegraph.h"
#include "nameable.h"

#include "generic/callback.h"
#include "scenelib.h"
#include "string/string.h"
#include "generic/reference.h"

inline Nameable* Node_getNameable(scene::Node& node)
{
  return NodeTypeCast<Nameable>::cast(node);
}

#if 0

#include "gtkutil/gtktreestore.h"

template<typename value_type>
inline void gtk_tree_model_get_pointer(GtkTreeModel* model, GtkTreeIter* iter, gint column, value_type** pointer)
{
  GValue value = GValue_default();
  gtk_tree_model_get_value(model, iter, column, &value);
  *pointer = (value_type*)g_value_get_pointer(&value);
}


typedef GtkTreeStore GraphTreeModel;

GtkTreeStore* graph_tree_model_new(graph_type* graph)
{
  return gtk_tree_store_new(2, G_TYPE_POINTER, G_TYPE_POINTER);
}

void graph_tree_model_delete(GraphTreeModel* model)
{
  g_object_unref(G_OBJECT(model));
}


bool graph_tree_model_subtree_find_node(GraphTreeModel* model, GtkTreeIter* parent, const scene::Node& node, GtkTreeIter* iter)
{
  for(gboolean success = gtk_tree_model_iter_children(GTK_TREE_MODEL(model), iter, parent);
    success != FALSE;
    success = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), iter))
  {
    scene::Node* current;
    gtk_tree_model_get_pointer(GTK_TREE_MODEL(model), iter, 0, &current);
    if(current == node)
    {
      return true;
    }
  }
  return false;
}

typedef GtkTreeIter DoubleGtkTreeIter[2];

bool graph_tree_model_find_top(GraphTreeModel* model, const scene::Path& path, GtkTreeIter& iter)
{
  int swap = 0;
  GtkTreeIter* parent_pointer = NULL;
  GtkTreeIter parent;
  for(scene::Path::const_iterator i = path.begin(); i != path.end(); ++i)
  {
    if(!graph_tree_model_subtree_find_node(model, parent_pointer, *i, &iter))
    {
      return false;
    }
    parent = iter;
    parent_pointer = &parent;
  }
  return true;
}

bool graph_tree_model_find_parent(GraphTreeModel* model, const scene::Path& path, GtkTreeIter& iter)
{
  int swap = 0;
  GtkTreeIter* parent_pointer = NULL;
  ASSERT_MESSAGE(path.size() > 1, "path too short");
  for(scene::Path::const_iterator i = path.begin(); i != path.end()-1; ++i)
  {
    GtkTreeIter child;
    if(!graph_tree_model_subtree_find_node(model, parent_pointer, *i, &child))
    {
      return false;
    }
    iter = child;
    parent_pointer = &iter;
  }
  return true;
}

void node_attach_name_changed_callback(scene::Node& node, const Callback& callback)
{
  if(node != 0)
  {
    Nameable* nameable = Node_getNameable(node);
    if(nameable != 0)
    {
      nameable->attach(callback);
    }
  }
}
void node_detach_name_changed_callback(scene::Node& node, const Callback& callback)
{
  if(node != 0)
  {
    Nameable* nameable = Node_getNameable(node);
    if(nameable != 0)
    {
      nameable->detach(callback);
    }
  }
}

GraphTreeModel* scene_graph_get_tree_model(); // temp hack

void graph_tree_model_row_changed(const scene::Instance& instance)
{
  GraphTreeModel* model = scene_graph_get_tree_model();

  GtkTreeIter child;
  ASSERT_MESSAGE(graph_tree_model_find_top(model, instance.path(), child), "RUNTIME ERROR");

  gtk_tree_store_set(GTK_TREE_STORE(model), &child, 0, instance.path().top(), -1);
}

void graph_tree_model_row_inserted(GraphTreeModel* model, const scene::Instance& instance)
{
  GtkTreeIter parent;
  GtkTreeIter* parent_pointer = NULL;
  if(instance.path().size() != 1)
  {
    ASSERT_MESSAGE(graph_tree_model_find_parent(model, instance.path(), parent), "RUNTIME ERROR");
    parent_pointer = &parent;
  }

  gpointer node = instance.path().top();
  gconstpointer selectable = Instance_getSelectable(instance);

  GtkTreeIter child;
  gtk_tree_store_append(GTK_TREE_STORE(model), &child, parent_pointer);
  gtk_tree_store_set(GTK_TREE_STORE(model), &child, 0, node, 1, selectable, -1);

  node_attach_name_changed_callback(instance.path().top(), ConstReferenceCaller<scene::Instance, graph_tree_model_row_changed>(instance));
}

void graph_tree_model_row_deleted(GraphTreeModel* model, const scene::Instance& instance)
{
  GtkTreeIter child;
  ASSERT_MESSAGE(graph_tree_model_find_top(model, instance.path(), child), "RUNTIME ERROR");

  node_detach_name_changed_callback(instance.path().top(), ConstReferenceCaller<scene::Instance, graph_tree_model_row_changed>(instance));

  gtk_tree_store_remove(GTK_TREE_STORE(model), &child);
}

#elif 0

const char* node_get_name(scene::Node& node);

typedef scene::Node* NodePointer;

class NodeNameLess
{
public:
  bool operator()(const NodePointer& self, const NodePointer& other) const
  {
    if(self == 0)
    {
      return true;
    }
    if(other == 0)
    {
      return false;
    }
    int result = string_compare(node_get_name(self), node_get_name(other));
    if(result == 0)
    {
      return self < other;
    }
    return result < 0;
  }
};

class PathNameLess
{
public:
  bool operator()(const PathConstReference& self, const PathConstReference& other) const
  {
    return std::lexicographical_compare(self.get().begin(), self.get().end(), other.get().begin(), other.get().end(), NodeNameLess());
  }
};

typedef std::map<PathConstReference, scene::Instance*, PathNameLess> graph_type;

struct GraphTreeModel
{
  GObject parent;

  graph_type* graph;
};

struct GraphTreeModelClass
{
  GObjectClass parent_class;
};

#define GRAPH_TREE_MODEL(p) (reinterpret_cast<GraphTreeModel*>(p))

static GtkTreeModelFlags graph_tree_model_get_flags (GtkTreeModel* tree_model)
{
  return GTK_TREE_MODEL_ITERS_PERSIST;
}

static gint graph_tree_model_get_n_columns (GtkTreeModel* tree_model)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  GraphTreeModel* graph_tree_model = (GraphTreeModel*) tree_model;
  
  return 2;
}

static const gint c_stamp = 0xabcdef;

inline graph_type::iterator graph_iterator_read_tree_iter(GtkTreeIter* iter)
{
  ASSERT_MESSAGE(iter != 0,  "tree model error");
  ASSERT_MESSAGE(iter->user_data != 0,  "tree model error");
  ASSERT_MESSAGE(iter->stamp == c_stamp,  "tree model error");
  return *reinterpret_cast<graph_type::iterator*>(&iter->user_data);
}

inline void graph_iterator_write_tree_iter(graph_type::iterator i, GtkTreeIter* iter)
{
  ASSERT_MESSAGE(iter != 0,  "tree model error");
  iter->stamp = c_stamp;
  *reinterpret_cast<graph_type::iterator*>(&iter->user_data) = i;
  ASSERT_MESSAGE(iter->user_data != 0,  "tree model error");
}

static GType graph_tree_model_get_column_type (GtkTreeModel *tree_model, gint index)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  GraphTreeModel *graph_tree_model = (GraphTreeModel *) tree_model;
  
  return G_TYPE_POINTER;
}

static gboolean graph_tree_model_get_iter(GtkTreeModel* tree_model, GtkTreeIter* iter, GtkTreePath* path)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  gint* indices = gtk_tree_path_get_indices (path);
  gint depth = gtk_tree_path_get_depth (path);
  
  g_return_val_if_fail (depth > 0, FALSE);

  graph_type& graph = *GRAPH_TREE_MODEL(tree_model)->graph;

  if(graph.empty())
    return FALSE;
  
  GtkTreeIter tmp;
  GtkTreeIter* parent = 0;

  for(gint i = 0; i < depth; i++)
  {
    if (! gtk_tree_model_iter_nth_child (tree_model, iter, parent, indices[i]))
      return FALSE;
    tmp = *iter;
    parent = &tmp;
  }
  
  return TRUE;
}

static GtkTreePath* graph_tree_model_get_path(GtkTreeModel* tree_model, GtkTreeIter* iter)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  graph_type& graph = *GRAPH_TREE_MODEL(tree_model)->graph;
  graph_type::iterator i = graph_iterator_read_tree_iter(iter);

  GtkTreePath* path = gtk_tree_path_new();

  for(std::size_t depth = (*i).first.get().size(); depth != 0; --depth)
  {
    std::size_t index = 0;

    while(i != graph.begin() && (*i).first.get().size() >= depth)
    {
      --i;
      if((*i).first.get().size() == depth)
        ++index;
    }

    gtk_tree_path_prepend_index(path, index);
  }

  return path;
}


static void graph_tree_model_get_value (GtkTreeModel *tree_model, GtkTreeIter  *iter, gint column, GValue *value)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  ASSERT_MESSAGE(column == 0 || column == 1, "tree model error");
  
  graph_type::iterator i = graph_iterator_read_tree_iter(iter);

  g_value_init (value, G_TYPE_POINTER);

  if(column == 0)
    g_value_set_pointer(value, reinterpret_cast<gpointer>((*i).first.get().top()));
  else
    g_value_set_pointer(value, reinterpret_cast<gpointer>(Instance_getSelectable(*(*i).second)));
}

static gboolean graph_tree_model_iter_next (GtkTreeModel  *tree_model, GtkTreeIter   *iter)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  graph_type& graph = *GRAPH_TREE_MODEL(tree_model)->graph;
  graph_type::iterator i = graph_iterator_read_tree_iter(iter);
  std::size_t depth = (*i).first.get().size();

  ++i;

  while(i != graph.end() && (*i).first.get().size() > depth)
  {
    ++i;
  }

  if(i == graph.end() || (*i).first.get().size() != depth)
  {
    return FALSE;
  }

  graph_iterator_write_tree_iter(i, iter);

  return TRUE;
}

static gboolean graph_tree_model_iter_children (GtkTreeModel *tree_model, GtkTreeIter  *iter, GtkTreeIter  *parent)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  graph_type& graph = *GRAPH_TREE_MODEL(tree_model)->graph;
  graph_type::iterator i = (parent == 0) ? graph.begin() : graph_iterator_read_tree_iter(parent);
  std::size_t depth = (parent == 0) ? 1 : (*i).first.get().size() + 1;
  
  if(parent != 0)
    ++i;

  if(i != graph.end() && (*i).first.get().size() == depth)
  {
    graph_iterator_write_tree_iter(i, iter);
    return TRUE;
  }

  return FALSE;
}

static gboolean graph_tree_model_iter_has_child (GtkTreeModel *tree_model, GtkTreeIter  *iter)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  graph_type& graph = *GRAPH_TREE_MODEL(tree_model)->graph;
  graph_type::iterator i = graph_iterator_read_tree_iter(iter);
  std::size_t depth = (*i).first.get().size() + 1;

  return ++i != graph.end() && (*i).first.get().size() == depth;
}

static gint graph_tree_model_iter_n_children (GtkTreeModel *tree_model, GtkTreeIter *parent)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  graph_type& graph = *GRAPH_TREE_MODEL(tree_model)->graph;
  graph_type::iterator i = (parent == 0) ? graph.begin() : graph_iterator_read_tree_iter(parent);
  std::size_t depth = (parent == 0) ? 1 : (*i).first.get().size() + 1;
  
  if(parent != 0)
    ++i;

  gint count = 0;
  while(i != graph.end() && (*i).first.get().size() >= depth)
  {
    ++count;
    ++i;
  }
  
  return count;
}

static gboolean graph_tree_model_iter_nth_child (GtkTreeModel *tree_model, GtkTreeIter  *iter, GtkTreeIter  *parent, gint n)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  graph_type& graph = *GRAPH_TREE_MODEL(tree_model)->graph;
  graph_type::iterator i = (parent == 0) ? graph.begin() : graph_iterator_read_tree_iter(parent);
  std::size_t depth = (parent == 0) ? 1 : (*i).first.get().size() + 1;
  
  if(parent != 0)
    ++i;

  while(i != graph.end() && (*i).first.get().size() >= depth)
  {
    if((*i).first.get().size() == depth && n-- == 0)
    {
      graph_iterator_write_tree_iter(i, iter);
      return TRUE;
    }
    ++i;
  }
  
  return FALSE;
}

static gboolean graph_tree_model_iter_parent(GtkTreeModel *tree_model, GtkTreeIter  *iter, GtkTreeIter  *child)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  graph_type& graph = *GRAPH_TREE_MODEL(tree_model)->graph;
  graph_type::iterator i = graph_iterator_read_tree_iter(child);
  std::size_t depth = (*i).first.get().size();
  if(depth == 1)
  {
    return FALSE;
  }
  else
  {
    do
    {
      --i;
    }
    while((*i).first.get().size() >= depth);
    graph_iterator_write_tree_iter(i, iter);
    return TRUE;
  }
}

static GObjectClass *g_parent_class = 0;

static void graph_tree_model_init (GraphTreeModel *graph_tree_model)
{
  graph_tree_model->graph = 0;
}

static void graph_tree_model_finalize(GObject* object)
{
  GraphTreeModel* graph_tree_model = GRAPH_TREE_MODEL(object);
  
  /* must chain up */
  (* g_parent_class->finalize) (object);
}

static void graph_tree_model_class_init (GraphTreeModelClass *class_)
{
  GObjectClass *object_class;
  
  g_parent_class = (GObjectClass*)g_type_class_peek_parent (class_);
  object_class = (GObjectClass *) class_;
  
  object_class->finalize = graph_tree_model_finalize;
}

static void graph_tree_model_tree_model_init (GtkTreeModelIface *iface)
{
  iface->get_flags = graph_tree_model_get_flags;
  iface->get_n_columns = graph_tree_model_get_n_columns;
  iface->get_column_type = graph_tree_model_get_column_type;
  iface->get_iter = graph_tree_model_get_iter;
  iface->get_path = graph_tree_model_get_path;
  iface->get_value = graph_tree_model_get_value;
  iface->iter_next = graph_tree_model_iter_next;
  iface->iter_children = graph_tree_model_iter_children;
  iface->iter_has_child = graph_tree_model_iter_has_child;
  iface->iter_n_children = graph_tree_model_iter_n_children;
  iface->iter_nth_child = graph_tree_model_iter_nth_child;
  iface->iter_parent = graph_tree_model_iter_parent;
}

static gboolean graph_tree_model_row_draggable(GtkTreeDragSource *drag_source, GtkTreePath *path)
{
#ifdef _DEBUG
  gint depth = gtk_tree_path_get_depth(path);
#endif
  return gtk_tree_path_get_depth(path) > 1;
}

static gboolean graph_tree_model_drag_data_delete(GtkTreeDragSource *drag_source, GtkTreePath *path)
{
  GtkTreeIter iter;
  
  if(gtk_tree_model_get_iter(GTK_TREE_MODEL(drag_source), &iter, path))
  {
    graph_type::iterator i = graph_iterator_read_tree_iter(&iter);
    Path_deleteTop((*i).first);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static gboolean graph_tree_model_drag_data_get (GtkTreeDragSource *drag_source, GtkTreePath *path, GtkSelectionData *selection_data)
{
  if(gtk_tree_set_row_drag_data(selection_data, GTK_TREE_MODEL(drag_source), path))
  {
    return TRUE;
  }
  else
  {
    /* FIXME handle text targets at least. */
  }

  return FALSE;
}

static void graph_tree_model_drag_source_init (GtkTreeDragSourceIface *iface)
{
  iface->row_draggable = graph_tree_model_row_draggable;
  iface->drag_data_delete = graph_tree_model_drag_data_delete;
  iface->drag_data_get = graph_tree_model_drag_data_get;
}

static gboolean graph_tree_model_drag_data_received (GtkTreeDragDest *drag_dest, GtkTreePath *dest, GtkSelectionData *selection_data)
{
  GtkTreeModel *tree_model = GTK_TREE_MODEL (drag_dest);
  
  GtkTreeModel *src_model = 0;
  GtkTreePath *src_path = 0;
  if(gtk_tree_get_row_drag_data(selection_data, &src_model, &src_path)
    && src_model == tree_model)
  {
    /* Copy the given row to a new position */
    GtkTreeIter iter;

    if(gtk_tree_model_get_iter(src_model, &iter, src_path))
    {
      int bleh = 0;
    }
  }
  else
  {
  /* FIXME maybe add some data targets eventually, or handle text
  * targets in the simple case.
    */
  }
  
  return FALSE;
}

static gboolean graph_tree_model_row_drop_possible(GtkTreeDragDest *drag_dest, GtkTreePath *dest_path, GtkSelectionData *selection_data)
{
  gboolean retval = FALSE;
  
  GtkTreeModel *src_model = 0;
  GtkTreePath *src_path = 0;
  if(gtk_tree_get_row_drag_data(selection_data, &src_model, &src_path) != FALSE)
  {
    /* can only drag to ourselves */
    if(src_model == GTK_TREE_MODEL(drag_dest))
    {
      /* Can't drop into ourself. */
      if(!gtk_tree_path_is_ancestor(src_path, dest_path))
      {
        /* Can't drop if dest_path's parent doesn't exist */ 
        if (gtk_tree_path_get_depth (dest_path) > 1)
        {
          GtkTreePath* tmp = gtk_tree_path_copy (dest_path);
          gtk_tree_path_up (tmp);
    
          GtkTreeIter iter;
          retval = gtk_tree_model_get_iter (GTK_TREE_MODEL (drag_dest), &iter, tmp);

          gtk_tree_path_free (tmp);
        }
      }
    }
  
    gtk_tree_path_free (src_path);
  }
  
  return retval;
}

static void graph_tree_model_drag_dest_init (GtkTreeDragDestIface *iface)
{
  iface->drag_data_received = graph_tree_model_drag_data_received;
  iface->row_drop_possible = graph_tree_model_row_drop_possible;
}

GType graph_tree_model_get_type (void)
{
  static GType graph_tree_model_type = 0;
  
  if (!graph_tree_model_type)
  {
    static const GTypeInfo graph_tree_model_info =
    {
      sizeof (GraphTreeModelClass),
      0,  /* base_init */
      0,  /* base_finalize */
      (GClassInitFunc) graph_tree_model_class_init,
      0,  /* class_finalize */
      0,  /* class_data */
      sizeof (GraphTreeModel),
      0,              /* n_preallocs */
      (GInstanceInitFunc) graph_tree_model_init
    };
    
    static const GInterfaceInfo tree_model_info =
    {
      (GInterfaceInitFunc) graph_tree_model_tree_model_init,
      0,
      0
    };
    
    static const GInterfaceInfo drag_source_info =
    {
      (GInterfaceInitFunc) graph_tree_model_drag_source_init,
      0,
      0
    };

    static const GInterfaceInfo drag_dest_info =
    {
      (GInterfaceInitFunc) graph_tree_model_drag_dest_init,
      0,
      0
    };

    graph_tree_model_type = g_type_register_static (G_TYPE_OBJECT, "GraphTreeModel",
            &graph_tree_model_info, (GTypeFlags)0);
    
    g_type_add_interface_static (graph_tree_model_type,
      GTK_TYPE_TREE_MODEL,
      &tree_model_info);
    g_type_add_interface_static (graph_tree_model_type,
      GTK_TYPE_TREE_DRAG_SOURCE,
      &drag_source_info);
    g_type_add_interface_static (graph_tree_model_type,
      GTK_TYPE_TREE_DRAG_DEST,
      &drag_dest_info);
  }
  
  return graph_tree_model_type;
}

GraphTreeModel* graph_tree_model_new()
{
  GraphTreeModel* graph_tree_model = GRAPH_TREE_MODEL(g_object_new (graph_tree_model_get_type(), 0));
    
  graph_tree_model->graph = new graph_type;

  return graph_tree_model;
}

void graph_tree_model_delete(GraphTreeModel* model)
{
  delete model->graph;
  g_object_unref(G_OBJECT(model));
}


class TempNameable : public Nameable
{
  const char* m_name;
public:
  TempNameable(const char* name) : m_name(name)
  {
  }
  const char* name() const
  {
    return m_name;
  }
  void attach(const NameCallback& callback)
  {
  }
  void detach(const NameCallback& callback)
  {
  }
};

void node_attach_name_changed_callback(scene::Node& node, const NameCallback& callback)
{
  if(&node != 0)
  {
    Nameable* nameable = Node_getNameable(node);
    if(nameable != 0)
    {
      nameable->attach(callback);
    }
  }
}
void node_detach_name_changed_callback(scene::Node& node, const NameCallback& callback)
{
  if(&node != 0)
  {
    Nameable* nameable = Node_getNameable(node);
    if(nameable != 0)
    {
      nameable->detach(callback);
    }
  }
}

GraphTreeModel* scene_graph_get_tree_model(); // temp hack

void graph_tree_model_row_inserted(GraphTreeModel* model, graph_type::iterator i)
{
  GtkTreeIter iter;
  graph_iterator_write_tree_iter(i, &iter);

  GtkTreePath* tree_path = graph_tree_model_get_path(GTK_TREE_MODEL(model), &iter);

  gint depth = gtk_tree_path_get_depth(tree_path);
  gint* indices = gtk_tree_path_get_indices(tree_path);

  gtk_tree_model_row_inserted(GTK_TREE_MODEL(model), tree_path, &iter);

  gtk_tree_path_free(tree_path);
}

void graph_tree_model_row_deleted(GraphTreeModel* model, graph_type::iterator i)
{
  GtkTreeIter iter;
  graph_iterator_write_tree_iter(i, &iter);

  GtkTreePath* tree_path = graph_tree_model_get_path(GTK_TREE_MODEL(model), &iter);

  gtk_tree_model_row_deleted(GTK_TREE_MODEL(model), tree_path);

  gtk_tree_path_free(tree_path);
}

#include "generic/referencecounted.h"

void graph_tree_model_set_name(const scene::Instance& instance, const char* name)
{
  GraphTreeModel* model = scene_graph_get_tree_model();

  if(string_empty(name)) // hack!
  {
    graph_type::iterator i = model->graph->find(PathConstReference(instance.path()));
    ASSERT_MESSAGE(i != model->graph->end(), "ERROR");

    graph_tree_model_row_deleted(model, i);

    model->graph->erase(i);
  }
  else
  {
    graph_type::iterator i = model->graph->insert(graph_type::value_type(PathConstReference(instance.path()), &const_cast<scene::Instance&>(instance))).first;

    graph_tree_model_row_inserted(model, i);
  }
}

void graph_tree_model_insert(GraphTreeModel* model, const scene::Instance& instance)
{
  graph_type::iterator i = model->graph->insert(graph_type::value_type(PathConstReference(instance.path()), &const_cast<scene::Instance&>(instance))).first;

  graph_tree_model_row_inserted(model, i);

  node_attach_name_changed_callback(instance.path().top(), ConstReferenceCaller1<scene::Instance, const char*, graph_tree_model_set_name>(instance));
}

void graph_tree_model_erase(GraphTreeModel* model, const scene::Instance& instance)
{
  node_detach_name_changed_callback(instance.path().top(), ConstReferenceCaller1<scene::Instance, const char*, graph_tree_model_set_name>(instance));

  graph_type::iterator i = model->graph->find(PathConstReference(instance.path()));
  ASSERT_MESSAGE(i != model->graph->end(), "ERROR");

  graph_tree_model_row_deleted(model, i);

  model->graph->erase(i);
}

#elif 1

class GraphTreeNode;
void graph_tree_model_row_changed(GraphTreeNode& node);

class GraphTreeNode
{
  typedef std::map<std::pair<CopiedString, scene::Node*>, GraphTreeNode*> ChildNodes;
  ChildNodes m_childnodes;
public:
  Reference<scene::Instance> m_instance;
  GraphTreeNode* m_parent;

  typedef ChildNodes::iterator iterator;
  typedef ChildNodes::key_type key_type;
  typedef ChildNodes::value_type value_type;
  typedef ChildNodes::size_type size_type;

  GraphTreeNode(scene::Instance& instance) : m_instance(instance), m_parent(0)
  {
    m_instance.get().setChildSelectedChangedCallback(RowChangedCaller(*this));
  }
  ~GraphTreeNode()
  {
    m_instance.get().setChildSelectedChangedCallback(Callback());
    ASSERT_MESSAGE(empty(), "GraphTreeNode::~GraphTreeNode: memory leak");
  }

  iterator begin()
  {
    return m_childnodes.begin();
  }
  iterator end()
  {
    return m_childnodes.end();
  }

  size_type size() const
  {
    return m_childnodes.size();
  }
  bool empty() const
  {
    return m_childnodes.empty();
  }

  iterator insert(const value_type& value)
  {
    iterator i = m_childnodes.insert(value).first;
    (*i).second->m_parent = this;
    return i;
  }
  void erase(iterator i)
  {
    m_childnodes.erase(i);
  }
  iterator find(const key_type& key)
  {
    return m_childnodes.find(key);
  }

  void swap(GraphTreeNode& other)
  {
    std::swap(m_parent, other.m_parent);
    std::swap(m_childnodes, other.m_childnodes);
    std::swap(m_instance, other.m_instance);
  }

  void rowChanged()
  {
    graph_tree_model_row_changed(*this);
  }
  typedef MemberCaller<GraphTreeNode, &GraphTreeNode::rowChanged> RowChangedCaller;
};

struct GraphTreeModel
{
  GObject parent;

  GraphTreeNode* m_graph;
};

struct GraphTreeModelClass
{
  GObjectClass parent_class;
};

#define GRAPH_TREE_MODEL(p) (reinterpret_cast<GraphTreeModel*>(p))

static GtkTreeModelFlags graph_tree_model_get_flags (GtkTreeModel* tree_model)
{
  return GTK_TREE_MODEL_ITERS_PERSIST;
}

static gint graph_tree_model_get_n_columns (GtkTreeModel* tree_model)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  //GraphTreeModel* graph_tree_model = (GraphTreeModel*) tree_model;
  
  return 2;
}

static const gint c_stamp = 0xabcdef;

inline GraphTreeNode::iterator graph_iterator_read_tree_iter(GtkTreeIter* iter)
{
  ASSERT_MESSAGE(iter != 0,  "tree model error");
  ASSERT_MESSAGE(iter->user_data != 0,  "tree model error");
  ASSERT_MESSAGE(iter->stamp == c_stamp,  "tree model error");
  return *reinterpret_cast<GraphTreeNode::iterator*>(&iter->user_data);
}

inline void graph_iterator_write_tree_iter(GraphTreeNode::iterator i, GtkTreeIter* iter)
{
  ASSERT_MESSAGE(iter != 0,  "tree model error");
  iter->stamp = c_stamp;
  *reinterpret_cast<GraphTreeNode::iterator*>(&iter->user_data) = i;
  ASSERT_MESSAGE(iter->user_data != 0,  "tree model error");
}

static GType graph_tree_model_get_column_type (GtkTreeModel *tree_model, gint index)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  //GraphTreeModel *graph_tree_model = (GraphTreeModel *) tree_model;
  
  return G_TYPE_POINTER;
}

static gboolean graph_tree_model_get_iter(GtkTreeModel* tree_model, GtkTreeIter* iter, GtkTreePath* path)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  gint* indices = gtk_tree_path_get_indices (path);
  gint depth = gtk_tree_path_get_depth (path);
  
  g_return_val_if_fail (depth > 0, FALSE);

  GraphTreeNode* graph = GRAPH_TREE_MODEL(tree_model)->m_graph;

  if(graph->empty())
    return FALSE;
  
  GtkTreeIter tmp;
  GtkTreeIter* parent = 0;

  for(gint i = 0; i < depth; i++)
  {
    if (! gtk_tree_model_iter_nth_child (tree_model, iter, parent, indices[i]))
      return FALSE;
    tmp = *iter;
    parent = &tmp;
  }
  
  return TRUE;
}

static GtkTreePath* graph_tree_model_get_path(GtkTreeModel* tree_model, GtkTreeIter* iter)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  GraphTreeNode* graph = GRAPH_TREE_MODEL(tree_model)->m_graph;

  GtkTreePath* path = gtk_tree_path_new();

  for(GraphTreeNode* node = (*graph_iterator_read_tree_iter(iter)).second; node != graph; node = node->m_parent)
  {
    std::size_t index = 0;
    for(GraphTreeNode::iterator i = node->m_parent->begin(); i != node->m_parent->end(); ++i, ++index)
    {
      if((*i).second == node)
      {
        gtk_tree_path_prepend_index(path, gint(index));
        break;
      }
    }
    ASSERT_MESSAGE(index != node->m_parent->size(), "error resolving tree path");
  }

  return path;
}


static void graph_tree_model_get_value (GtkTreeModel *tree_model, GtkTreeIter  *iter, gint column, GValue *value)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  ASSERT_MESSAGE(column == 0 || column == 1, "tree model error");
  
  GraphTreeNode::iterator i = graph_iterator_read_tree_iter(iter);

  g_value_init (value, G_TYPE_POINTER);

  if(column == 0)
  {
    g_value_set_pointer(value, reinterpret_cast<gpointer>((*i).first.second));
  }
  else
  {
    g_value_set_pointer(value, reinterpret_cast<gpointer>(&(*i).second->m_instance.get()));
  }
}

static gboolean graph_tree_model_iter_next (GtkTreeModel  *tree_model, GtkTreeIter   *iter)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  GraphTreeNode::iterator i = graph_iterator_read_tree_iter(iter);
  GraphTreeNode& parent = *(*i).second->m_parent;
  
  ASSERT_MESSAGE(i != parent.end(), "RUNTIME ERROR");

  if(++i == parent.end())
  {
    return FALSE;
  }

  graph_iterator_write_tree_iter(i, iter);

  return TRUE;
}

static gboolean graph_tree_model_iter_children (GtkTreeModel *tree_model, GtkTreeIter  *iter, GtkTreeIter  *parent)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  GraphTreeNode& node = (parent == 0) ? *GRAPH_TREE_MODEL(tree_model)->m_graph : *(*graph_iterator_read_tree_iter(parent)).second;
  if(!node.empty())
  {
    graph_iterator_write_tree_iter(node.begin(), iter);
    return TRUE;
  }
 
  return FALSE;
}

static gboolean graph_tree_model_iter_has_child (GtkTreeModel *tree_model, GtkTreeIter  *iter)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  GraphTreeNode& node = *(*graph_iterator_read_tree_iter(iter)).second;
  return !node.empty();
}

static gint graph_tree_model_iter_n_children (GtkTreeModel *tree_model, GtkTreeIter *parent)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  GraphTreeNode& node = (parent == 0) ? *GRAPH_TREE_MODEL(tree_model)->m_graph : *(*graph_iterator_read_tree_iter(parent)).second;
  return static_cast<gint>(node.size());
}

static gboolean graph_tree_model_iter_nth_child (GtkTreeModel *tree_model, GtkTreeIter  *iter, GtkTreeIter  *parent, gint n)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  GraphTreeNode& node = (parent == 0) ? *GRAPH_TREE_MODEL(tree_model)->m_graph : *(*graph_iterator_read_tree_iter(parent)).second;
  if(static_cast<std::size_t>(n) < node.size())
  {
    GraphTreeNode::iterator i = node.begin();
    std::advance(i, n);
    graph_iterator_write_tree_iter(i, iter);
    return TRUE;
  }
  
  return FALSE;
}

static gboolean graph_tree_model_iter_parent(GtkTreeModel *tree_model, GtkTreeIter  *iter, GtkTreeIter  *child)
{
  ASSERT_MESSAGE(tree_model != 0, "RUNTIME ERROR");
  GraphTreeNode& node = *(*graph_iterator_read_tree_iter(child)).second;
  if(node.m_parent != GRAPH_TREE_MODEL(tree_model)->m_graph)
  {
    GraphTreeNode& parentParent = *node.m_parent->m_parent;
    for(GraphTreeNode::iterator i = parentParent.begin(); i != parentParent.end(); ++i)
    {
      if((*i).second == node.m_parent)
      {
        graph_iterator_write_tree_iter(i, iter);
        return TRUE;
      }
    }
  }
  return FALSE;
}

static GObjectClass *g_parent_class = 0;

namespace
{
  scene::Node* g_null_node = 0;
}

class NullInstance : public scene::Instance
{
public:
  NullInstance() : scene::Instance(scene::Path(makeReference(*g_null_node)), 0, 0, Static<InstanceTypeCastTable>::instance())
  {
  }
};

namespace
{
  NullInstance g_null_instance;
}

static void graph_tree_model_init (GraphTreeModel *graph_tree_model)
{
  graph_tree_model->m_graph = new GraphTreeNode(g_null_instance);
}

static void graph_tree_model_finalize(GObject* object)
{
  GraphTreeModel* graph_tree_model = GRAPH_TREE_MODEL(object);

  delete graph_tree_model->m_graph;
  
  /* must chain up */
  (* g_parent_class->finalize) (object);
}

static void graph_tree_model_class_init (GraphTreeModelClass *class_)
{
  GObjectClass *object_class;
  
  g_parent_class = (GObjectClass*)g_type_class_peek_parent (class_);
  object_class = (GObjectClass *) class_;
  
  object_class->finalize = graph_tree_model_finalize;
}

static void graph_tree_model_tree_model_init (GtkTreeModelIface *iface)
{
  iface->get_flags = graph_tree_model_get_flags;
  iface->get_n_columns = graph_tree_model_get_n_columns;
  iface->get_column_type = graph_tree_model_get_column_type;
  iface->get_iter = graph_tree_model_get_iter;
  iface->get_path = graph_tree_model_get_path;
  iface->get_value = graph_tree_model_get_value;
  iface->iter_next = graph_tree_model_iter_next;
  iface->iter_children = graph_tree_model_iter_children;
  iface->iter_has_child = graph_tree_model_iter_has_child;
  iface->iter_n_children = graph_tree_model_iter_n_children;
  iface->iter_nth_child = graph_tree_model_iter_nth_child;
  iface->iter_parent = graph_tree_model_iter_parent;
}

GType graph_tree_model_get_type (void)
{
  static GType graph_tree_model_type = 0;
  
  if (!graph_tree_model_type)
  {
    static const GTypeInfo graph_tree_model_info =
    {
      sizeof (GraphTreeModelClass),
      0,    /* base_init */
      0,    /* base_finalize */
      (GClassInitFunc) graph_tree_model_class_init,
      0,    /* class_finalize */
      0,    /* class_data */
      sizeof (GraphTreeModel),
      0,              /* n_preallocs */
      (GInstanceInitFunc) graph_tree_model_init,
      0
    };
    
    static const GInterfaceInfo tree_model_info =
    {
      (GInterfaceInitFunc) graph_tree_model_tree_model_init,
      0,
      0
    };
    
    graph_tree_model_type = g_type_register_static (G_TYPE_OBJECT, "GraphTreeModel",
            &graph_tree_model_info, (GTypeFlags)0);
    
    g_type_add_interface_static (graph_tree_model_type,
      GTK_TYPE_TREE_MODEL,
      &tree_model_info);
  }
  
  return graph_tree_model_type;
}

GraphTreeModel* graph_tree_model_new()
{
  GraphTreeModel* graph_tree_model = GRAPH_TREE_MODEL(g_object_new (graph_tree_model_get_type(), 0));

  return graph_tree_model;
}

void graph_tree_model_delete(GraphTreeModel* model)
{
  g_object_unref(G_OBJECT(model));
}

void graph_tree_model_row_changed(GraphTreeModel* model, GraphTreeNode::iterator i)
{
  GtkTreeIter iter;
  graph_iterator_write_tree_iter(i, &iter);

  GtkTreePath* tree_path = graph_tree_model_get_path(GTK_TREE_MODEL(model), &iter);

  gtk_tree_model_row_changed(GTK_TREE_MODEL(model), tree_path, &iter);

  gtk_tree_path_free(tree_path);
}

void graph_tree_model_row_inserted(GraphTreeModel* model, GraphTreeNode::iterator i)
{
  GtkTreeIter iter;
  graph_iterator_write_tree_iter(i, &iter);

  GtkTreePath* tree_path = graph_tree_model_get_path(GTK_TREE_MODEL(model), &iter);

  gtk_tree_model_row_inserted(GTK_TREE_MODEL(model), tree_path, &iter);

  gtk_tree_path_free(tree_path);
}

void graph_tree_model_row_deleted(GraphTreeModel* model, GraphTreeNode::iterator i)
{
  GtkTreeIter iter;
  graph_iterator_write_tree_iter(i, &iter);

  GtkTreePath* tree_path = graph_tree_model_get_path(GTK_TREE_MODEL(model), &iter);

  gtk_tree_model_row_deleted(GTK_TREE_MODEL(model), tree_path);

  gtk_tree_path_free(tree_path);
}

void graph_tree_model_row_inserted(GraphTreeModel& model, GraphTreeNode::iterator i)
{
  graph_tree_model_row_inserted(&model, i);
}

void graph_tree_model_row_deleted(GraphTreeModel& model, GraphTreeNode::iterator i)
{
  graph_tree_model_row_deleted(&model, i);
}

const char* node_get_name(scene::Node& node);

const char* node_get_name_safe(scene::Node& node)
{
  if(&node == 0)
  {
    return "";
  }
  return node_get_name(node);
}

GraphTreeNode* graph_tree_model_find_parent(GraphTreeModel* model, const scene::Path& path)
{
  GraphTreeNode* parent = model->m_graph;
  for(scene::Path::const_iterator i = path.begin(); i != path.end()-1; ++i)
  {
    GraphTreeNode::iterator child = parent->find(GraphTreeNode::key_type(node_get_name_safe((*i).get()), (*i).get_pointer()));
    ASSERT_MESSAGE(child != parent->end(), "ERROR");
    parent = (*child).second;
  }
  return parent;
}

void node_attach_name_changed_callback(scene::Node& node, const NameCallback& callback)
{
  if(&node != 0)
  {
    Nameable* nameable = Node_getNameable(node);
    if(nameable != 0)
    {
      nameable->attach(callback);
    }
  }
}
void node_detach_name_changed_callback(scene::Node& node, const NameCallback& callback)
{
  if(&node != 0)
  {
    Nameable* nameable = Node_getNameable(node);
    if(nameable != 0)
    {
      nameable->detach(callback);
    }
  }
}

GraphTreeModel* scene_graph_get_tree_model(); // temp hack

void graph_tree_node_foreach_pre(GraphTreeNode::iterator root, const Callback1<GraphTreeNode::iterator>& callback)
{
  callback(root);
  for(GraphTreeNode::iterator i = (*root).second->begin(); i != (*root).second->end(); ++i)
  {
    graph_tree_node_foreach_pre(i, callback);
  }
}

void graph_tree_node_foreach_post(GraphTreeNode::iterator root, const Callback1<GraphTreeNode::iterator>& callback)
{
  for(GraphTreeNode::iterator i = (*root).second->begin(); i != (*root).second->end(); ++i)
  {
    graph_tree_node_foreach_post(i, callback);
  }
  callback(root);
}

void graph_tree_model_row_changed(GraphTreeNode& node)
{
  GraphTreeModel* model = scene_graph_get_tree_model();
  const scene::Instance& instance = node.m_instance.get();
  
  GraphTreeNode::iterator i = node.m_parent->find(GraphTreeNode::key_type(node_get_name_safe(instance.path().top().get()), instance.path().top().get_pointer()));

  graph_tree_model_row_changed(model, i);
}

void graph_tree_model_set_name(const scene::Instance& instance, const char* name)
{
  GraphTreeModel* model = scene_graph_get_tree_model();
  GraphTreeNode* parent = graph_tree_model_find_parent(model, instance.path());

  GraphTreeNode::iterator oldNode = parent->find(GraphTreeNode::key_type(node_get_name_safe(instance.path().top().get()), instance.path().top().get_pointer()));
  graph_tree_node_foreach_post(oldNode, ReferenceCaller1<GraphTreeModel, GraphTreeNode::iterator, graph_tree_model_row_deleted>(*model));
  GraphTreeNode* node((*oldNode).second);
  parent->erase(oldNode);

  GraphTreeNode::iterator newNode = parent->insert(GraphTreeNode::value_type(GraphTreeNode::key_type(name, &instance.path().top().get()), node));
  graph_tree_node_foreach_pre(newNode, ReferenceCaller1<GraphTreeModel, GraphTreeNode::iterator, graph_tree_model_row_inserted>(*model));
}

void graph_tree_model_insert(GraphTreeModel* model, const scene::Instance& instance)
{
  GraphTreeNode* parent = graph_tree_model_find_parent(model, instance.path());

  GraphTreeNode::iterator i = parent->insert(GraphTreeNode::value_type(GraphTreeNode::key_type(node_get_name_safe(instance.path().top().get()), instance.path().top().get_pointer()), new GraphTreeNode(const_cast<scene::Instance&>(instance))));

  graph_tree_model_row_inserted(model, i);

  node_attach_name_changed_callback(instance.path().top(), ConstReferenceCaller1<scene::Instance, const char*, graph_tree_model_set_name>(instance));
}

void graph_tree_model_erase(GraphTreeModel* model, const scene::Instance& instance)
{
  node_detach_name_changed_callback(instance.path().top(), ConstReferenceCaller1<scene::Instance, const char*, graph_tree_model_set_name>(instance));

  GraphTreeNode* parent = graph_tree_model_find_parent(model, instance.path());

  GraphTreeNode::iterator i = parent->find(GraphTreeNode::key_type(node_get_name_safe(instance.path().top().get()), instance.path().top().get_pointer()));

  graph_tree_model_row_deleted(model, i);

  GraphTreeNode* node((*i).second);
  parent->erase(i);
  delete node;
}



#endif




#if 0
class TestGraphTreeModel
{
public:
  TestGraphTreeModel()
  {
    gtk_init(0, 0);

    graph_type graph;

    scene::Node* root = *(scene::Node*)0xa0000000;
    scene::Node* node1 = (scene::Node*)0xa0000001;
    scene::Node* node2 = (scene::Node*)0xa0000002;
    scene::Node* node3 = (scene::Node*)0xa0000003;
    scene::Node* node4 = (scene::Node*)0xa0000004;
    scene::Instance* instance = (scene::Instance*)0xaaaaaaaa;

    scene::Path rootpath(root);

    graph.insert(graph_type::value_type(rootpath, instance));

    rootpath.push(node1);
    graph.insert(graph_type::value_type(rootpath, instance));
    rootpath.pop();

    rootpath.push(node2);
    graph.insert(graph_type::value_type(rootpath, instance));
    rootpath.push(node3);
    graph.insert(graph_type::value_type(rootpath, instance));
    rootpath.pop();
    rootpath.push(node4);
    graph.insert(graph_type::value_type(rootpath, instance));
    rootpath.pop();
    rootpath.pop();

    GtkTreeModel* model = GTK_TREE_MODEL(graph_tree_model_new(&graph));

    {
      gint n_columns = gtk_tree_model_get_n_columns(model);
      ASSERT_MESSAGE(n_columns == 2, "test failed!");
    }

    {
      GType type = gtk_tree_model_get_column_type(model, 0);
      ASSERT_MESSAGE(type == G_TYPE_POINTER, "test failed!");
    }

    {
      GType type = gtk_tree_model_get_column_type(model, 1);
      ASSERT_MESSAGE(type == G_TYPE_POINTER, "test failed!");
    }

    
    {
      GtkTreeIter iter;
      gtk_tree_model_get_iter_first(model, &iter);

      graph_type::iterator i = graph_iterator_read_tree_iter(&iter);
      ASSERT_MESSAGE((*i).first.get().size() == 2 && (*i).first.get().top() == node1, "test failed!");
    }

    {
      GtkTreeIter iter;
      gtk_tree_model_get_iter_first(model, &iter);

      ASSERT_MESSAGE(gtk_tree_model_iter_has_child(model, &iter) == FALSE, "test failed!");

      ASSERT_MESSAGE(gtk_tree_model_iter_n_children(model, &iter) == 0, "test failed!");

      gtk_tree_model_iter_next(model, &iter);

      ASSERT_MESSAGE(gtk_tree_model_iter_has_child(model, &iter) != FALSE, "test failed!");

      ASSERT_MESSAGE(gtk_tree_model_iter_n_children(model, &iter) == 2, "test failed!");

      {
        GtkTreeIter child;
        gtk_tree_model_iter_nth_child(model, &child, &iter, 0);

        scene::Node* test;
        gtk_tree_model_get_value(model, &child, 0, (GValue*)&test);
        ASSERT_MESSAGE(test == node3, "test failed!");

        {
          GtkTreeIter parent;
          gtk_tree_model_iter_parent(model, &parent, &child);

          scene::Node* test;
          gtk_tree_model_get_value(model, &parent, 0, (GValue*)&test);
          ASSERT_MESSAGE(test == node2, "test failed!");
        }
      }

      {
        GtkTreeIter child;
        gtk_tree_model_iter_nth_child(model, &child, &iter, 1);

        scene::Node* test;
        gtk_tree_model_get_value(model, &child, 0, (GValue*)&test);
        ASSERT_MESSAGE(test == node4, "test failed!");
      }
    }

    {
      GtkTreeIter iter;
      std::size_t count = 0;
      for(gboolean good = gtk_tree_model_get_iter_first(model, &iter); good; good = gtk_tree_model_iter_next(model, &iter))
      {
        scene::Node* test;
        gtk_tree_model_get_value(model, &iter, 0, (GValue*)&test);

        ASSERT_MESSAGE((count == 0 && test == node1) || (count == 1 && test == node2), "test failed!");
        ++count;
      }

      ASSERT_MESSAGE(count == 2, "test failed!");

    }

    {
      GtkTreeIter iter;
      gtk_tree_model_get_iter_first(model, &iter);

      scene::Node* test;
      gtk_tree_model_get_value(model, &iter, 0, (GValue*)&test);
      ASSERT_MESSAGE(test == node1, "test failed!");
    }

    {
      GtkTreeIter iter;
      GtkTreePath* path = gtk_tree_path_new_from_string("0");
      gtk_tree_model_get_iter(model, &iter, path);
      gtk_tree_path_free(path);

      graph_type::iterator i = graph_iterator_read_tree_iter(&iter);
      ASSERT_MESSAGE((*i).first.get().size() == 2 && (*i).first.get().top() == node1, "test failed!");
    }

    {
      GtkTreeIter iter;
      GtkTreePath* path = gtk_tree_path_new_from_string("1");
      gtk_tree_model_get_iter(model, &iter, path);
      gtk_tree_path_free(path);

      graph_type::iterator i = graph_iterator_read_tree_iter(&iter);
      ASSERT_MESSAGE((*i).first.get().size() == 2 && (*i).first.get().top() == node2, "test failed!");
    }

    {
      GtkTreeIter iter;
      graph_type::iterator i = graph.begin();
      ++i;
      graph_iterator_write_tree_iter(i, &iter);

      GtkTreePath* path = gtk_tree_model_get_path(model, &iter);

      gint depth = gtk_tree_path_get_depth(path);
      gint* indices = gtk_tree_path_get_indices(path);

      ASSERT_MESSAGE(depth == 1 && indices[0] == 0, "test failed!");

      gtk_tree_path_free(path);
    }

    {
      GtkTreeIter iter;
      graph_type::iterator i = graph.begin();
      ++i;
      ++i;
      graph_iterator_write_tree_iter(i, &iter);

      GtkTreePath* path = gtk_tree_model_get_path(model, &iter);

      gint depth = gtk_tree_path_get_depth(path);
      gint* indices = gtk_tree_path_get_indices(path);

      ASSERT_MESSAGE(depth == 1 && indices[0] == 1, "test failed!");

      gtk_tree_path_free(path);
    }
  }
};


TestGraphTreeModel g_TestGraphTreeModel;

#endif
