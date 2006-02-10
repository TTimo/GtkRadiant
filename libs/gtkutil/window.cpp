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

#include "window.h"

#include <gtk/gtkscrolledwindow.h>

#include "pointer.h"
#include "accelerator.h"

inline void CHECK_RESTORE(GtkWidget* w)
{
  if(gpointer_to_int(g_object_get_data(G_OBJECT(w), "was_mapped")) != 0)
  {
    gtk_widget_show(w);
  }
}

inline void CHECK_MINIMIZE(GtkWidget* w)
{
  g_object_set_data(G_OBJECT(w), "was_mapped", gint_to_pointer(GTK_WIDGET_VISIBLE(w)));
  gtk_widget_hide(w);
}

static gboolean main_window_iconified(GtkWidget* widget, GdkEventWindowState* event, gpointer data)
{
  if((event->changed_mask & (GDK_WINDOW_STATE_ICONIFIED|GDK_WINDOW_STATE_WITHDRAWN)) != 0)
  {
    if((event->new_window_state & (GDK_WINDOW_STATE_ICONIFIED|GDK_WINDOW_STATE_WITHDRAWN)) != 0)
    {
      CHECK_MINIMIZE(GTK_WIDGET(data));
    }
    else
    {
      CHECK_RESTORE(GTK_WIDGET(data));
    }
  }
  return FALSE;
}

unsigned int connect_floating(GtkWindow* main_window, GtkWindow* floating)
{
  return g_signal_connect(G_OBJECT(main_window), "window_state_event", G_CALLBACK(main_window_iconified), floating);
}

gboolean destroy_disconnect_floating(GtkWindow* widget, gpointer data)
{
  g_signal_handler_disconnect(G_OBJECT(data), gpointer_to_int(g_object_get_data(G_OBJECT(widget), "floating_handler")));
  return FALSE;
}

gboolean floating_window_delete_present(GtkWindow* floating, GdkEventFocus *event, GtkWindow* main_window)
{
  if(gtk_window_is_active(floating) || gtk_window_is_active(main_window))
  {
    gtk_window_present(main_window);
  }
  return FALSE;
}

guint connect_floating_window_delete_present(GtkWindow* floating, GtkWindow* main_window)
{
  return g_signal_connect(G_OBJECT(floating), "delete_event", G_CALLBACK(floating_window_delete_present), main_window);
}

gboolean floating_window_destroy_present(GtkWindow* floating, GtkWindow* main_window)
{
  if(gtk_window_is_active(floating) || gtk_window_is_active(main_window))
  {
    gtk_window_present(main_window);
  }
  return FALSE;
}

guint connect_floating_window_destroy_present(GtkWindow* floating, GtkWindow* main_window)
{
  return g_signal_connect(G_OBJECT(floating), "destroy", G_CALLBACK(floating_window_destroy_present), main_window);
}

GtkWindow* create_floating_window(const char* title, GtkWindow* parent)
{
  GtkWindow* window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gtk_window_set_title(window, title);

  if(parent != 0)
  {
    gtk_window_set_transient_for(window, parent);
    connect_floating_window_destroy_present(window, parent);
    g_object_set_data(G_OBJECT(window), "floating_handler", gint_to_pointer(connect_floating(parent, window)));
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_disconnect_floating), parent);
  }

  return window;
}

void destroy_floating_window(GtkWindow* window)
{
  gtk_widget_destroy(GTK_WIDGET(window));
}

gint window_realize_remove_sysmenu(GtkWidget* widget, gpointer data)
{
  gdk_window_set_decorations(widget->window, (GdkWMDecoration)(GDK_DECOR_ALL|GDK_DECOR_MENU));
  return FALSE;
}

gboolean persistent_floating_window_delete(GtkWindow* floating, GdkEvent *event, GtkWindow* main_window)
{
  gtk_widget_hide(GTK_WIDGET(floating));
  return TRUE;
}

GtkWindow* create_persistent_floating_window(const char* title, GtkWindow* main_window)
{
  GtkWindow* window = GTK_WINDOW(create_floating_window(title, main_window));

  gtk_widget_set_events(GTK_WIDGET(window), GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

  connect_floating_window_delete_present(window, main_window);
  g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(persistent_floating_window_delete), 0);

#if 0
  if(g_multimon_globals.m_bStartOnPrimMon && g_multimon_globals.m_bNoSysMenuPopups)
    g_signal_connect(G_OBJECT(window), "realize", G_CALLBACK(window_realize_remove_sysmenu), 0);
#endif

  return window;
}

gint window_realize_remove_minmax(GtkWidget* widget, gpointer data)
{
  gdk_window_set_decorations(widget->window, (GdkWMDecoration)(GDK_DECOR_ALL|GDK_DECOR_MINIMIZE|GDK_DECOR_MAXIMIZE));
  return FALSE;
}

void window_remove_minmax(GtkWindow* window)
{
  g_signal_connect(G_OBJECT(window), "realize", G_CALLBACK(window_realize_remove_minmax), 0);
}


GtkScrolledWindow* create_scrolled_window(GtkPolicyType hscrollbar_policy, GtkPolicyType vscrollbar_policy, int border)
{
  GtkScrolledWindow* scr = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(0, 0));
  gtk_widget_show(GTK_WIDGET(scr));
  gtk_scrolled_window_set_policy(scr, hscrollbar_policy, vscrollbar_policy);
  gtk_scrolled_window_set_shadow_type(scr, GTK_SHADOW_IN);
  gtk_container_set_border_width(GTK_CONTAINER(scr), border);
  return scr;
}


