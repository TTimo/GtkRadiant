/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
For a list of contributors, see the accompanying CONTRIBUTORS file.

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

//
// Floating dialog that contains a notebook with at least Entities and Group tabs
// I merged the 2 MS Windows dialogs in a single class
//
// Leonardo Zide (leo@lokigames.com)
//

#include "groupdialog.h"

#include "debugging/debugging.h"

#include <vector>

#include <gtk/gtknotebook.h>
#include <gtk/gtktextview.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkscrolledwindow.h>

#include "gtkutil/widget.h"
#include "gtkutil/accelerator.h"
#include "entityinspector.h"
#include "gtkmisc.h"
#include "multimon.h"
#include "console.h"
#include "commands.h"


#include <gtk/gtkwidget.h>
#include "gtkutil/window.h"

class GroupDlg 
{
public:
  GtkWidget* m_pNotebook;
  GtkWindow* m_window;

  GroupDlg();
  void Create(GtkWindow* parent);

  void Show()
  {
    // workaround for strange gtk behaviour - modifying the contents of a window while it is not visible causes the window position to change without sending a configure_event
    m_position_tracker.sync(m_window);
    gtk_widget_show(GTK_WIDGET(m_window));
  }
  void Hide()
  {
    gtk_widget_hide(GTK_WIDGET(m_window));
  }

  WindowPositionTracker m_position_tracker;
};

namespace
{
  GroupDlg g_GroupDlg;

  std::size_t g_current_page;
  std::vector<StringExportCallback> g_pages;
}

void GroupDialog_updatePageTitle(GtkWindow* window, std::size_t pageIndex)
{
  if(pageIndex < g_pages.size())
  {
    g_pages[pageIndex](PointerCaller1<GtkWindow, const char*, gtk_window_set_title>(window));
  }
}

static gboolean switch_page(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, gpointer data)
{
  GroupDialog_updatePageTitle(GTK_WINDOW(data), page_num);
  g_current_page = page_num;

  return FALSE;
}

GroupDlg::GroupDlg() : m_window(0)
{
  m_position_tracker.setPosition(c_default_window_pos);
}

void GroupDlg::Create(GtkWindow* parent)
{
  ASSERT_MESSAGE(m_window == 0, "dialog already created");

  GtkWindow* window = create_persistent_floating_window("Entities", parent);

  global_accel_connect_window(window);

  window_connect_focus_in_clear_focus_widget(window);

  m_window = window;

#ifdef WIN32
  if( g_multimon_globals.m_bStartOnPrimMon )
  {
    WindowPosition pos(m_position_tracker.getPosition());
    PositionWindowOnPrimaryScreen(pos);
    m_position_tracker.setPosition(pos);
  }
#endif
  m_position_tracker.connect(window);

  {
    GtkWidget* notebook = gtk_notebook_new();
    gtk_widget_show(notebook);
    gtk_container_add (GTK_CONTAINER (window), notebook);
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_BOTTOM);
    m_pNotebook = notebook;

    g_signal_connect(G_OBJECT(notebook), "switch_page", G_CALLBACK(switch_page), window);
  }
}


GtkWidget* GroupDialog_addPage(const char* tabLabel, GtkWidget* widget, const StringExportCallback& title)
{
  GtkWidget* w = gtk_label_new(tabLabel);
  gtk_widget_show(w);
  GtkWidget* page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_GroupDlg.m_pNotebook), gtk_notebook_insert_page(GTK_NOTEBOOK(g_GroupDlg.m_pNotebook), widget, w, -1));
  g_pages.push_back(title);

  return page;
}


bool GroupDialog_isShown()
{
  return widget_is_visible(GTK_WIDGET(g_GroupDlg.m_window));
}
void GroupDialog_setShown(bool shown)
{
  shown ? g_GroupDlg.Show() : g_GroupDlg.Hide();
}
void GroupDialog_ToggleShow()
{  
  GroupDialog_setShown(!GroupDialog_isShown());
}

void GroupDialog_constructWindow(GtkWindow* main_window)
{
  g_GroupDlg.Create(main_window);
}
void GroupDialog_destroyWindow()
{
  ASSERT_NOTNULL(g_GroupDlg.m_window);
  destroy_floating_window(g_GroupDlg.m_window);
  g_GroupDlg.m_window = 0;
}


GtkWindow* GroupDialog_getWindow()
{
  return g_GroupDlg.m_window;
}
void GroupDialog_show()
{
  g_GroupDlg.Show();
}

GtkWidget* GroupDialog_getPage()
{
  return gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_GroupDlg.m_pNotebook), gint(g_current_page));
}

void GroupDialog_setPage(GtkWidget* page)
{
  g_current_page = gtk_notebook_page_num(GTK_NOTEBOOK(g_GroupDlg.m_pNotebook), page);
  gtk_notebook_set_current_page(GTK_NOTEBOOK(g_GroupDlg.m_pNotebook), gint(g_current_page));
}

void GroupDialog_showPage(GtkWidget* page)
{
  if(GroupDialog_getPage() == page)
  {
    GroupDialog_ToggleShow();
  }
  else
  {
    gtk_widget_show(GTK_WIDGET(g_GroupDlg.m_window));
    GroupDialog_setPage(page);
  }
}

void GroupDialog_cycle()
{
  g_current_page = (g_current_page + 1) % g_pages.size(); 
  gtk_notebook_set_current_page(GTK_NOTEBOOK(g_GroupDlg.m_pNotebook), gint(g_current_page));
}

void GroupDialog_updatePageTitle(GtkWidget* page)
{
  if(GroupDialog_getPage() == page)
  {
    GroupDialog_updatePageTitle(g_GroupDlg.m_window, g_current_page);
  }
}


#include "preferencesystem.h"

void GroupDialog_Construct()
{
  GlobalPreferenceSystem().registerPreference("EntityWnd", WindowPositionTrackerImportStringCaller(g_GroupDlg.m_position_tracker), WindowPositionTrackerExportStringCaller(g_GroupDlg.m_position_tracker));

  GlobalCommands_insert("ViewEntityInfo", FreeCaller<GroupDialog_ToggleShow>(), Accelerator('N'));
}
void GroupDialog_Destroy()
{
}
