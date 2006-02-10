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

#include "pluginmenu.h"

#include "stream/textstream.h"

#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>

#include "gtkutil/pointer.h"
#include "gtkutil/menu.h"

#include "pluginmanager.h"
#include "mainframe.h"
#include "preferences.h"


int m_nNextPlugInID = 0;

void plugin_activated(GtkWidget* widget, gpointer data)
{
  const char* str = (const char*)g_object_get_data(G_OBJECT(widget),"command");
  GetPlugInMgr().Dispatch(gpointer_to_int(data), str);
}

#include <stack>
typedef std::stack<GtkWidget*> WidgetStack;

void PlugInMenu_Add(GtkMenu* plugin_menu, IPlugIn* pPlugIn)
{
  GtkWidget *menu, *item, *parent, *subMenu;
  const char *menuText, *menuCommand;
  WidgetStack menuStack;
  
  parent = gtk_menu_item_new_with_label (pPlugIn->getMenuName());
  gtk_widget_show (parent);
  gtk_container_add (GTK_CONTAINER (plugin_menu), parent);

  std::size_t nCount = pPlugIn->getCommandCount();
  if (nCount > 0)
  {
    menu = gtk_menu_new();
    while (nCount > 0)
    {
      menuText = pPlugIn->getCommandTitle(--nCount);
      menuCommand = pPlugIn->getCommand(nCount);
      
      if (menuText != 0 && strlen(menuText) > 0)
      {
        if (!strcmp(menuText, "-"))
        {
          item = gtk_menu_item_new();
          gtk_widget_set_sensitive (item, FALSE);
        }
        else if (!strcmp(menuText, ">"))
        {
          menuText = pPlugIn->getCommandTitle(--nCount);
          menuCommand = pPlugIn->getCommand(nCount);
          if (!strcmp(menuText, "-") || !strcmp(menuText, ">") || !strcmp(menuText, "<"))
          {
            globalErrorStream() << pPlugIn->getMenuName() << " Invalid title (" << menuText << ") for submenu.\n";
            continue;
          }
          
          item = gtk_menu_item_new_with_label(menuText);
          gtk_widget_show (item);
          gtk_container_add (GTK_CONTAINER (menu), item);
          
          subMenu = gtk_menu_new();
          gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), subMenu);
          menuStack.push(menu);
          menu = subMenu;
          continue;
        }
        else if (!strcmp(menuText, "<"))
        {
          if (!menuStack.empty())
          {
              menu = menuStack.top();
              menuStack.pop();
          }
          else
          {
            globalErrorStream() << pPlugIn->getMenuName() << ": Attempt to end non-existent submenu ignored.\n";
          }
          continue;
        }
        else
        {
          item = gtk_menu_item_new_with_label (menuText);
          g_object_set_data(G_OBJECT(item),"command", const_cast<gpointer>(static_cast<const void*>(menuCommand)));
          g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(plugin_activated), gint_to_pointer(m_nNextPlugInID));
        }
        gtk_widget_show (item);
        gtk_container_add (GTK_CONTAINER (menu), item);
        pPlugIn->addMenuID(m_nNextPlugInID++);
      }
    }
    if (!menuStack.empty())
    {
      std::size_t size = menuStack.size();
      if (size != 0)
      {
        globalErrorStream() << pPlugIn->getMenuName() << " mismatched > <. " << Unsigned(size) << " submenu(s) not closed.\n";
      }
      for (std::size_t i = 0; i < (size -1); i++)
      {
        menuStack.pop();
      }
      menu = menuStack.top();
      menuStack.pop();
    }
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (parent), menu);
  }
}

GtkMenu* g_plugins_menu = 0;
GtkMenuItem* g_plugins_menu_separator = 0;

void PluginsMenu_populate()
{
  class PluginsMenuConstructor : public PluginsVisitor
  {
    GtkMenu* m_menu;
  public:
    PluginsMenuConstructor(GtkMenu* menu) : m_menu(menu)
    {
    }
    void visit(IPlugIn& plugin)
    {
      PlugInMenu_Add(m_menu, &plugin);
    }
  };
  
  PluginsMenuConstructor constructor(g_plugins_menu);
  GetPlugInMgr().constructMenu(constructor);
}

void PluginsMenu_clear()
{
  m_nNextPlugInID = 0;

  GList* lst = g_list_find (gtk_container_children(GTK_CONTAINER(g_plugins_menu)), GTK_WIDGET(g_plugins_menu_separator));
  while (lst->next)
  {
    gtk_container_remove (GTK_CONTAINER(g_plugins_menu), GTK_WIDGET (lst->next->data));
    lst = g_list_find (gtk_container_children(GTK_CONTAINER(g_plugins_menu)),  GTK_WIDGET(g_plugins_menu_separator));
  }
}

GtkMenuItem* create_plugins_menu()
{
  // Plugins menu
  GtkMenuItem* plugins_menu_item = new_sub_menu_item_with_mnemonic("_Plugins");
  GtkMenu* menu = GTK_MENU(gtk_menu_item_get_submenu(plugins_menu_item));
  if (g_Layout_enableDetachableMenus.m_value)
  {
    menu_tearoff(menu);
  }

  g_plugins_menu = menu;

  //TODO: some modules/plugins do not yet support refresh
#if 0
  create_menu_item_with_mnemonic(menu, "Refresh", FreeCaller<Restart>());

  // NOTE: the seperator is used when doing a refresh of the list, everything past the seperator is removed
  g_plugins_menu_separator = menu_separator(menu);
#endif

  PluginsMenu_populate();

  return plugins_menu_item;
}

