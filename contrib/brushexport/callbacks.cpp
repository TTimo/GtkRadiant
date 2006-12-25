#include <glib.h>
#include <gtk/gtk.h>
#include <set>

#include "qerplugin.h"
#include "debugging/debugging.h"
#include "support.h"
#include "export.h"

// stuff from interface.cpp
void DestroyWindow();


namespace callbacks {

void OnDestroy(GtkWidget* w, gpointer data)
{
	DestroyWindow();
}

void OnExportClicked(GtkButton* button, gpointer user_data)
{
	GtkWidget* window = lookup_widget(GTK_WIDGET(button), "w_plugplug2");
	ASSERT_NOTNULL(window);
	const char* path = GlobalRadiant().m_pfnFileDialog(window, false, "Save as Obj", 0, 0);
	if(!path)
		return;
  
  // get ignore list from ui
  std::set<std::string> ignore;
  
  GtkTreeView* view = GTK_TREE_VIEW(lookup_widget(GTK_WIDGET(button), "t_materialist"));
  GtkListStore* list = GTK_LIST_STORE(gtk_tree_view_get_model(view));

  GtkTreeIter iter;
  bool valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(list), &iter);
  while(valid)
  {
    gchar* data;
	gtk_tree_model_get(GTK_TREE_MODEL(list), &iter, 0, &data, -1);
	globalOutputStream() << data << "\n";
	ignore.insert(std::string(data));
	g_free(data);
	valid = gtk_tree_model_iter_next (GTK_TREE_MODEL(list), &iter);
  }
  
  for(std::set<std::string>::iterator it(ignore.begin()); it != ignore.end(); ++it)
    globalOutputStream() << it->c_str() << "\n";
  
  // collapse mode
  collapsemode mode = COLLAPSE_NONE;
  
  GtkWidget* radio = lookup_widget(GTK_WIDGET(button), "r_collapse");
  ASSERT_NOTNULL(radio);
  
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)))
    mode = COLLAPSE_ALL;
  else
  {
    radio = lookup_widget(GTK_WIDGET(button), "r_collapsebymaterial");
    ASSERT_NOTNULL(radio);
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)))
      mode = COLLAPSE_BY_MATERIAL;
	else
	{
	  radio = lookup_widget(GTK_WIDGET(button), "r_nocollapse");
	  ASSERT_NOTNULL(radio);
	  ASSERT_NOTNULL(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)));
	  mode = COLLAPSE_NONE;
	}
  }
  
  // export
  ExportSelection(ignore, mode, path);
}

void OnAddMaterial(GtkButton* button, gpointer user_data)
{
  GtkEntry* edit = GTK_ENTRY(lookup_widget(GTK_WIDGET(button), "ed_materialname"));
  ASSERT_NOTNULL(edit);
  
  const gchar* name = gtk_entry_get_text(edit);
  if(g_utf8_strlen(name, -1) > 0)
  {
	GtkListStore* list = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(lookup_widget(GTK_WIDGET(button), "t_materialist"))));
	GtkTreeIter iter;
	gtk_list_store_append(list, &iter);
	gtk_list_store_set(list, &iter, 0, name, -1);
	gtk_entry_set_text(edit, "");
  }
}

void OnRemoveMaterial(GtkButton* button, gpointer user_data)
{
	GtkTreeView* view = GTK_TREE_VIEW(lookup_widget(GTK_WIDGET(button), "t_materialist"));
	GtkListStore* list = GTK_LIST_STORE(gtk_tree_view_get_model(view));
	GtkTreeSelection* sel = gtk_tree_view_get_selection(view);
	
	GtkTreeIter iter;
	if(gtk_tree_selection_get_selected(sel, 0, &iter))
	  gtk_list_store_remove(list, &iter);
}

}// callbacks
