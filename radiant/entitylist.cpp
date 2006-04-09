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

#include "entitylist.h"

#include "iselection.h"

#include <gtk/gtktreemodel.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkcellrenderertext.h>

#include "string/string.h"
#include "scenelib.h"
#include "nameable.h"
#include "signal/isignal.h"
#include "generic/object.h"

#include "gtkutil/widget.h"
#include "gtkutil/window.h"
#include "gtkutil/idledraw.h"
#include "gtkutil/accelerator.h"
#include "gtkutil/closure.h"

#include "treemodel.h"

void RedrawEntityList();
typedef FreeCaller<RedrawEntityList> RedrawEntityListCaller;

typedef struct _GtkTreeView GtkTreeView;

class EntityList
{
public:
  enum EDirty
  {
    eDefault,
    eSelection,
    eInsertRemove,
  };

  EDirty m_dirty;

  IdleDraw m_idleDraw;
  WindowPositionTracker m_positionTracker;

  GtkWindow* m_window;
  GtkTreeView* m_tree_view;
  GraphTreeModel* m_tree_model;
  bool m_selection_disabled;

  EntityList() :
    m_dirty(EntityList::eDefault),
    m_idleDraw(RedrawEntityListCaller()),
    m_window(0),
    m_selection_disabled(false)
  {
  }

  bool visible() const
  {
    return GTK_WIDGET_VISIBLE(GTK_WIDGET(m_window));
  }
};

namespace
{
  EntityList* g_EntityList;

  inline EntityList& getEntityList()
  {
    ASSERT_NOTNULL(g_EntityList);
    return *g_EntityList;
  }
}


inline Nameable* Node_getNameable(scene::Node& node)
{
  return NodeTypeCast<Nameable>::cast(node);
}

const char* node_get_name(scene::Node& node)
{
  Nameable* nameable = Node_getNameable(node);
  return (nameable != 0)
    ? nameable->name()
    : "node";
}

template<typename value_type>
inline void gtk_tree_model_get_pointer(GtkTreeModel* model, GtkTreeIter* iter, gint column, value_type** pointer)
{
  GValue value = GValue_default();
  gtk_tree_model_get_value(model, iter, column, &value);
  *pointer = (value_type*)g_value_get_pointer(&value);
}



void entitylist_treeviewcolumn_celldatafunc(GtkTreeViewColumn* column, GtkCellRenderer* renderer, GtkTreeModel* model, GtkTreeIter* iter, gpointer data)
{
  scene::Node* node;
  gtk_tree_model_get_pointer(model, iter, 0, &node);
  scene::Instance* instance;
  gtk_tree_model_get_pointer(model, iter, 1, &instance);
  if(node != 0)
  {
    gtk_cell_renderer_set_fixed_size(renderer, -1, -1);
    char* name = const_cast<char*>(node_get_name(*node));
    g_object_set(G_OBJECT(renderer), "text", name, "visible", TRUE, 0);

    //globalOutputStream() << "rendering cell " << makeQuoted(name) << "\n";
    GtkStyle* style = gtk_widget_get_style(GTK_WIDGET(getEntityList().m_tree_view));
    if(instance->childSelected())
    {
      g_object_set(G_OBJECT(renderer), "cell-background-gdk", &style->base[GTK_STATE_ACTIVE], 0);
    }
    else
    {
      g_object_set(G_OBJECT(renderer), "cell-background-gdk", &style->base[GTK_STATE_NORMAL], 0);
    }
  }
  else
  {
    gtk_cell_renderer_set_fixed_size(renderer, -1, 0);
    g_object_set(G_OBJECT(renderer), "text", "", "visible", FALSE, 0);
  }
}

static gboolean entitylist_tree_select(GtkTreeSelection *selection, GtkTreeModel *model, GtkTreePath *path, gboolean path_currently_selected, gpointer data)
{
  GtkTreeIter iter;
  gtk_tree_model_get_iter(model, &iter, path);
  scene::Node* node;
  gtk_tree_model_get_pointer(model, &iter, 0, &node);
  scene::Instance* instance;
  gtk_tree_model_get_pointer(model, &iter, 1, &instance);
  Selectable* selectable = Instance_getSelectable(*instance);

  if(node == 0)
  {
    if(path_currently_selected != FALSE)
    {
      getEntityList().m_selection_disabled = true;
      GlobalSelectionSystem().setSelectedAll(false);
      getEntityList().m_selection_disabled = false;
    }
  }
  else if(selectable != 0)
  {
    getEntityList().m_selection_disabled = true;
    selectable->setSelected(path_currently_selected == FALSE);
    getEntityList().m_selection_disabled = false;
    return TRUE;
  }

  return FALSE;
}

static gboolean entitylist_tree_select_null(GtkTreeSelection *selection, GtkTreeModel *model, GtkTreePath *path, gboolean path_currently_selected, gpointer data)
{
  return TRUE;
}

void EntityList_ConnectSignals(GtkTreeView* view)
{
  GtkTreeSelection* select = gtk_tree_view_get_selection(view);
  gtk_tree_selection_set_select_function(select, entitylist_tree_select, NULL, 0);
}

void EntityList_DisconnectSignals(GtkTreeView* view)
{
  GtkTreeSelection* select = gtk_tree_view_get_selection(view);
  gtk_tree_selection_set_select_function(select, entitylist_tree_select_null, 0, 0);
}



gboolean treemodel_update_selection(GtkTreeModel* model, GtkTreePath* path, GtkTreeIter* iter, gpointer data)
{
  GtkTreeView* view = reinterpret_cast<GtkTreeView*>(data);

  scene::Instance* instance;
  gtk_tree_model_get_pointer(model, iter, 1, &instance);
  Selectable* selectable = Instance_getSelectable(*instance);

  if(selectable != 0)
  {
    GtkTreeSelection* selection = gtk_tree_view_get_selection(view);
    if(selectable->isSelected())
    {
      gtk_tree_selection_select_path(selection, path);
    }
    else
    {
      gtk_tree_selection_unselect_path(selection, path);
    }
  }

  return FALSE;
}

void EntityList_UpdateSelection(GtkTreeModel* model, GtkTreeView* view)
{
  EntityList_DisconnectSignals(view);
  gtk_tree_model_foreach(model, treemodel_update_selection, view);
  EntityList_ConnectSignals(view);
}


void RedrawEntityList()
{
  switch(getEntityList().m_dirty)
  {
  case EntityList::eInsertRemove:
  case EntityList::eSelection:
    EntityList_UpdateSelection(GTK_TREE_MODEL(getEntityList().m_tree_model), getEntityList().m_tree_view);
  default:
    break;
  }
  getEntityList().m_dirty = EntityList::eDefault;
}

void entitylist_queue_draw()
{
  getEntityList().m_idleDraw.queueDraw();
}

void EntityList_SelectionUpdate()
{
  if(getEntityList().m_selection_disabled)
    return;

  if(getEntityList().m_dirty < EntityList::eSelection)
    getEntityList().m_dirty = EntityList::eSelection;
  entitylist_queue_draw();
}

void EntityList_SelectionChanged(const Selectable& selectable)
{
  EntityList_SelectionUpdate();
}

void entitylist_treeview_rowcollapsed(GtkTreeView* view, GtkTreeIter* iter, GtkTreePath* path, gpointer user_data)
{
}

void entitylist_treeview_row_expanded(GtkTreeView* view, GtkTreeIter* iter, GtkTreePath* path, gpointer user_data)
{
  EntityList_SelectionUpdate();
}


void EntityList_SetShown(bool shown)
{
  widget_set_visible(GTK_WIDGET(getEntityList().m_window), shown);
}

void EntityList_toggleShown()
{
  EntityList_SetShown(!getEntityList().visible());
}

gint graph_tree_model_compare_name(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data)
{
  scene::Node* first;
  gtk_tree_model_get(model, a, 0, (gpointer*)&first, -1);
  scene::Node* second;
  gtk_tree_model_get(model, b, 0, (gpointer*)&second, -1);
  int result = 0;
  if(first != 0 && second != 0)
  {
    result = string_compare(node_get_name(*first), node_get_name(*second));
  }
  if(result == 0)
  {
    return (first < second) ? -1 : (second < first) ? 1 : 0;
  }
  return result;
}

extern GraphTreeModel* scene_graph_get_tree_model();
void AttachEntityTreeModel()
{
  getEntityList().m_tree_model = scene_graph_get_tree_model();

  gtk_tree_view_set_model(getEntityList().m_tree_view, GTK_TREE_MODEL(getEntityList().m_tree_model));
}

void DetachEntityTreeModel()
{
  getEntityList().m_tree_model = 0;

  gtk_tree_view_set_model(getEntityList().m_tree_view, 0);
}
      
void EntityList_constructWindow(GtkWindow* main_window)
{
  ASSERT_MESSAGE(getEntityList().m_window == 0, "error");

  GtkWindow* window = create_persistent_floating_window("Entity List", main_window);  

  gtk_window_add_accel_group(window, global_accel);

  getEntityList().m_positionTracker.connect(window);
  

  getEntityList().m_window = window;

  {
    GtkScrolledWindow* scr = create_scrolled_window(GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(scr));

    {
      GtkWidget* view = gtk_tree_view_new();
      gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);

      GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
      GtkTreeViewColumn* column = gtk_tree_view_column_new();
      gtk_tree_view_column_pack_start(column, renderer, TRUE);
      gtk_tree_view_column_set_cell_data_func(column, renderer, entitylist_treeviewcolumn_celldatafunc, 0, 0);

      GtkTreeSelection* select = gtk_tree_view_get_selection (GTK_TREE_VIEW(view));
      gtk_tree_selection_set_mode(select, GTK_SELECTION_MULTIPLE);

      g_signal_connect(G_OBJECT(view), "row_expanded", G_CALLBACK(entitylist_treeview_row_expanded), 0);
      g_signal_connect(G_OBJECT(view), "row_collapsed", G_CALLBACK(entitylist_treeview_rowcollapsed), 0);

      gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

      gtk_widget_show(view);
      gtk_container_add (GTK_CONTAINER(scr), view);
      getEntityList().m_tree_view = GTK_TREE_VIEW(view);
    }
  }

  EntityList_ConnectSignals(getEntityList().m_tree_view);
  AttachEntityTreeModel();
}

void EntityList_destroyWindow()
{
  DetachEntityTreeModel();
  EntityList_DisconnectSignals(getEntityList().m_tree_view);
  destroy_floating_window(getEntityList().m_window);
}

#include "preferencesystem.h"

#include "iselection.h"

namespace
{
  scene::Node* nullNode = 0;
}

class NullSelectedInstance : public scene::Instance, public Selectable
{
  class TypeCasts
  {
    InstanceTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      InstanceStaticCast<NullSelectedInstance, Selectable>::install(m_casts);
    }
    InstanceTypeCastTable& get()
    {
      return m_casts;
    }
  };

public:
  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  NullSelectedInstance() : Instance(scene::Path(makeReference(*nullNode)), 0, this, StaticTypeCasts::instance().get())
  {
  }

  void setSelected(bool select)
  {
    ERROR_MESSAGE("error");
  }
  bool isSelected() const
  {
    return true;
  }
};

typedef LazyStatic<NullSelectedInstance> StaticNullSelectedInstance;


void EntityList_Construct()
{
  graph_tree_model_insert(scene_graph_get_tree_model(), StaticNullSelectedInstance::instance());

  g_EntityList = new EntityList;

  getEntityList().m_positionTracker.setPosition(c_default_window_pos);

  GlobalPreferenceSystem().registerPreference("EntityInfoDlg", WindowPositionTrackerImportStringCaller(getEntityList().m_positionTracker), WindowPositionTrackerExportStringCaller(getEntityList().m_positionTracker));

  GlobalSelectionSystem().addSelectionChangeCallback(FreeCaller1<const Selectable&, EntityList_SelectionChanged>());
}
void EntityList_Destroy()
{
  delete g_EntityList;

  graph_tree_model_erase(scene_graph_get_tree_model(), StaticNullSelectedInstance::instance());
}
