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

#include "menu.h"

#include <ctype.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenubar.h>
#include <gtk/gtkradiomenuitem.h>
#include <gtk/gtktearoffmenuitem.h>
#include <gtk/gtkaccellabel.h>

#include "generic/callback.h"

#include "accelerator.h"
#include "closure.h"
#include "container.h"
#include "pointer.h"

void menu_add_item(GtkMenu* menu, GtkMenuItem* item)
{
	gtk_container_add(GTK_CONTAINER(menu), GTK_WIDGET(item));
}

GtkMenuItem* menu_separator(GtkMenu* menu)
{
  GtkMenuItem* menu_item = GTK_MENU_ITEM(gtk_menu_item_new());
  container_add_widget(GTK_CONTAINER(menu), GTK_WIDGET(menu_item));
  gtk_widget_set_sensitive(GTK_WIDGET(menu_item), FALSE);
  gtk_widget_show(GTK_WIDGET(menu_item));
  return menu_item;
}

GtkTearoffMenuItem* menu_tearoff(GtkMenu* menu)
{
  GtkTearoffMenuItem* menu_item = GTK_TEAROFF_MENU_ITEM(gtk_tearoff_menu_item_new());
  container_add_widget(GTK_CONTAINER(menu), GTK_WIDGET(menu_item));
// gtk_widget_set_sensitive(GTK_WIDGET(menu_item), FALSE); -- controls whether menu is detachable
  gtk_widget_show(GTK_WIDGET(menu_item));
  return menu_item;
}

GtkMenuItem* new_sub_menu_item_with_mnemonic(const char* mnemonic)
{
  GtkMenuItem* item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(mnemonic));
  gtk_widget_show(GTK_WIDGET(item));

  GtkWidget* sub_menu = gtk_menu_new();
  gtk_menu_item_set_submenu(item, sub_menu);

  return item;
}

GtkMenu* create_sub_menu_with_mnemonic(GtkMenuShell* parent, const char* mnemonic)
{
  GtkMenuItem* item = new_sub_menu_item_with_mnemonic(mnemonic);
  container_add_widget(GTK_CONTAINER(parent), GTK_WIDGET(item));
  return GTK_MENU(gtk_menu_item_get_submenu(item));
}

GtkMenu* create_sub_menu_with_mnemonic(GtkMenuBar* bar, const char* mnemonic)
{
  return create_sub_menu_with_mnemonic(GTK_MENU_SHELL(bar), mnemonic);
}

GtkMenu* create_sub_menu_with_mnemonic(GtkMenu* parent, const char* mnemonic)
{
  return create_sub_menu_with_mnemonic(GTK_MENU_SHELL(parent), mnemonic);
}

void activate_closure_callback(GtkWidget* widget, gpointer data)
{
  (*reinterpret_cast<Callback*>(data))();
}

guint menu_item_connect_callback(GtkMenuItem* item, const Callback& callback)
{
#if 1
  return g_signal_connect_swapped(G_OBJECT(item), "activate", G_CALLBACK(callback.getThunk()), callback.getEnvironment());
#else
  return g_signal_connect_closure(G_OBJECT(item), "activate", create_cclosure(G_CALLBACK(activate_closure_callback), callback), FALSE);
#endif
}

guint check_menu_item_connect_callback(GtkCheckMenuItem* item, const Callback& callback)
{
#if 1
  guint handler = g_signal_connect_swapped(G_OBJECT(item), "toggled", G_CALLBACK(callback.getThunk()), callback.getEnvironment());
#else
  guint handler = g_signal_connect_closure(G_OBJECT(item), "toggled", create_cclosure(G_CALLBACK(activate_closure_callback), callback), TRUE);
#endif
  g_object_set_data(G_OBJECT(item), "handler", gint_to_pointer(handler));
  return handler;
}

GtkMenuItem* new_menu_item_with_mnemonic(const char *mnemonic, const Callback& callback)
{
  GtkMenuItem* item = GTK_MENU_ITEM(gtk_menu_item_new_with_mnemonic(mnemonic));
  gtk_widget_show(GTK_WIDGET(item));
  menu_item_connect_callback(item, callback);
  return item;
}

GtkMenuItem* create_menu_item_with_mnemonic(GtkMenu* menu, const char *mnemonic, const Callback& callback)
{
  GtkMenuItem* item = new_menu_item_with_mnemonic(mnemonic, callback);
  container_add_widget(GTK_CONTAINER(menu), GTK_WIDGET(item));
  return item;
}

GtkCheckMenuItem* new_check_menu_item_with_mnemonic(const char* mnemonic, const Callback& callback)
{
  GtkCheckMenuItem* item = GTK_CHECK_MENU_ITEM(gtk_check_menu_item_new_with_mnemonic(mnemonic));
  gtk_widget_show(GTK_WIDGET(item));
  check_menu_item_connect_callback(item, callback);
  return item;
}

GtkCheckMenuItem* create_check_menu_item_with_mnemonic(GtkMenu* menu, const char* mnemonic, const Callback& callback)
{
  GtkCheckMenuItem* item = new_check_menu_item_with_mnemonic(mnemonic, callback);
  container_add_widget(GTK_CONTAINER(menu), GTK_WIDGET(item));
  return item;
}

GtkRadioMenuItem* new_radio_menu_item_with_mnemonic(GSList** group, const char* mnemonic, const Callback& callback)
{
  GtkRadioMenuItem* item = GTK_RADIO_MENU_ITEM(gtk_radio_menu_item_new_with_mnemonic(*group, mnemonic));
  if(*group == 0)
  {
    gtk_check_menu_item_set_state(GTK_CHECK_MENU_ITEM(item), TRUE);
  }
  *group = gtk_radio_menu_item_group(item);
  gtk_widget_show(GTK_WIDGET(item));
  check_menu_item_connect_callback(GTK_CHECK_MENU_ITEM(item), callback);
  return item;
}

GtkRadioMenuItem* create_radio_menu_item_with_mnemonic(GtkMenu* menu, GSList** group, const char* mnemonic, const Callback& callback)
{
  GtkRadioMenuItem* item = new_radio_menu_item_with_mnemonic(group, mnemonic, callback);
  container_add_widget(GTK_CONTAINER(menu), GTK_WIDGET(item));
  return item;
}

void check_menu_item_set_active_no_signal(GtkCheckMenuItem* item, gboolean active)
{
  guint handler_id = gpointer_to_int(g_object_get_data(G_OBJECT(item), "handler"));
  g_signal_handler_block(G_OBJECT(item), handler_id);
  gtk_check_menu_item_set_active(item, active);
  g_signal_handler_unblock(G_OBJECT(item), handler_id);
}



void radio_menu_item_set_active_no_signal(GtkRadioMenuItem* item, gboolean active)
{
  {
    for(GSList* l = gtk_radio_menu_item_get_group(item); l != 0; l = g_slist_next(l))
    {
      g_signal_handler_block(G_OBJECT(l->data), gpointer_to_int(g_object_get_data(G_OBJECT(l->data), "handler")));
    }
  }
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), active);
  {
    for(GSList* l = gtk_radio_menu_item_get_group(item); l != 0; l = g_slist_next(l))
    {
      g_signal_handler_unblock(G_OBJECT(l->data), gpointer_to_int(g_object_get_data(G_OBJECT(l->data), "handler")));
    }
  }
}


void menu_item_set_accelerator(GtkMenuItem* item, GClosure* closure)
{
  GtkAccelLabel* accel_label = GTK_ACCEL_LABEL(gtk_bin_get_child(GTK_BIN(item)));
  gtk_accel_label_set_accel_closure(accel_label, closure);
}

void accelerator_name(const Accelerator& accelerator, GString* gstring)
{
  gboolean had_mod = FALSE;
  if (accelerator.modifiers & GDK_SHIFT_MASK)
  {
    g_string_append (gstring, "Shift");
    had_mod = TRUE;
  }
  if (accelerator.modifiers & GDK_CONTROL_MASK)
  {
    if (had_mod)
      g_string_append (gstring, "+");
    g_string_append (gstring, "Ctrl");
    had_mod = TRUE;
  }
  if (accelerator.modifiers & GDK_MOD1_MASK)
  {
    if (had_mod)
      g_string_append (gstring, "+");
    g_string_append (gstring, "Alt");
    had_mod = TRUE;
  }

  if (had_mod)
    g_string_append (gstring, "+");
  if (accelerator.key < 0x80 || (accelerator.key > 0x80 && accelerator.key <= 0xff))
  {
    switch (accelerator.key)
    {
    case ' ':
      g_string_append (gstring, "Space");
      break;
    case '\\':
      g_string_append (gstring, "Backslash");
      break;
    default:
      g_string_append_c (gstring, gchar(toupper(accelerator.key)));
      break;
    }
  }
  else
  {
    gchar *tmp;

    tmp = gtk_accelerator_name (accelerator.key, (GdkModifierType)0);
    if (tmp[0] != 0 && tmp[1] == 0)
      tmp[0] = gchar(toupper(tmp[0]));
    g_string_append (gstring, tmp);
    g_free (tmp);
  }
}

void menu_item_set_accelerator(GtkMenuItem* item, Accelerator accelerator)
{
  GtkAccelLabel* accel_label = GTK_ACCEL_LABEL(gtk_bin_get_child(GTK_BIN(item)));

  g_free (accel_label->accel_string);
  accel_label->accel_string = 0;

  GString* gstring = g_string_new (accel_label->accel_string);
  g_string_append (gstring, "   ");

  accelerator_name(accelerator, gstring);

  g_free (accel_label->accel_string);
  accel_label->accel_string = gstring->str;
  g_string_free (gstring, FALSE);

  if (!accel_label->accel_string)
    accel_label->accel_string = g_strdup ("");

  gtk_widget_queue_resize (GTK_WIDGET (accel_label));
}

void menu_item_add_accelerator(GtkMenuItem* item, Accelerator accelerator)
{
  if(accelerator.key != 0)
  {
    GClosure* closure = global_accel_group_find(accelerator);
    if(closure != 0)
    {
      menu_item_set_accelerator(item, closure);
    }
    else
    {
      menu_item_set_accelerator(item, accelerator);
    }
  }
}

GtkMenuItem* create_menu_item_with_mnemonic(GtkMenu* menu, const char* mnemonic, const Command& command)
{
  GtkMenuItem* item = create_menu_item_with_mnemonic(menu, mnemonic, command.m_callback);
  menu_item_add_accelerator(item, command.m_accelerator);
  return item;
}

void check_menu_item_set_active_callback(GtkCheckMenuItem& item, bool enabled)
{
  check_menu_item_set_active_no_signal(&item, enabled);
}
typedef ReferenceCaller1<GtkCheckMenuItem, bool, check_menu_item_set_active_callback> CheckMenuItemSetActiveCaller;

GtkCheckMenuItem* create_check_menu_item_with_mnemonic(GtkMenu* menu, const char* mnemonic, const Toggle& toggle)
{
  GtkCheckMenuItem* item = create_check_menu_item_with_mnemonic(menu, mnemonic, toggle.m_command.m_callback);
  menu_item_add_accelerator(GTK_MENU_ITEM(item), toggle.m_command.m_accelerator);
  toggle.m_exportCallback(CheckMenuItemSetActiveCaller(*item));
  return item;
}




