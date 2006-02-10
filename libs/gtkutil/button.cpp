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

#include "button.h"

#include <gtk/gtkradiobutton.h>

#include "stream/textstream.h"
#include "stream/stringstream.h"
#include "generic/callback.h"

#include "image.h"
#include "pointer.h"

void clicked_closure_callback(GtkWidget* widget, gpointer data)
{
  (*reinterpret_cast<Callback*>(data))();
}

void button_connect_callback(GtkButton* button, const Callback& callback)
{
#if 1
  g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(callback.getThunk()), callback.getEnvironment());
#else
  g_signal_connect_closure(G_OBJECT(button), "clicked", create_cclosure(G_CALLBACK(clicked_closure_callback), callback), FALSE);
#endif
}
 
guint toggle_button_connect_callback(GtkToggleButton* button, const Callback& callback)
{
#if 1
  guint handler = g_signal_connect_swapped(G_OBJECT(button), "toggled", G_CALLBACK(callback.getThunk()), callback.getEnvironment());
#else
  guint handler = g_signal_connect_closure(G_OBJECT(button), "toggled", create_cclosure(G_CALLBACK(clicked_closure_callback), callback), TRUE);
#endif
  g_object_set_data(G_OBJECT(button), "handler", gint_to_pointer(handler));
  return handler;
}
 
void button_set_icon(GtkButton* button, const char* icon)
{
  GtkImage* image = new_local_image(icon);
  gtk_widget_show(GTK_WIDGET(image));
  gtk_container_add(GTK_CONTAINER(button), GTK_WIDGET(image));
}

void toggle_button_set_active_no_signal(GtkToggleButton* button, gboolean active)
{
  //globalOutputStream() << "set active: " << active << "\n";
  guint handler_id = gpointer_to_int(g_object_get_data(G_OBJECT(button), "handler"));
  //guint signal_id = g_signal_lookup("toggled", G_OBJECT_TYPE (button));
  //globalOutputStream() << "signal_id: " << signal_id << "\n";
  //guint found = g_signal_handler_find(G_OBJECT(button), G_SIGNAL_MATCH_ID, signal_id, 0, 0, 0, 0);
  //globalOutputStream() << " handler found: " << found << "\n";
  g_signal_handler_block(G_OBJECT(button), handler_id);
  gtk_toggle_button_set_active(button, active);
  g_signal_handler_unblock(G_OBJECT(button), handler_id);
}


void radio_button_print_state(GtkRadioButton* button)
{
  globalOutputStream() << "toggle button: ";
  for(GSList* radio = gtk_radio_button_group(button); radio != 0; radio = g_slist_next(radio))
  {
    globalOutputStream() << gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio->data));
  }
  globalOutputStream() << "\n";
}

GtkToggleButton* radio_button_get_nth(GtkRadioButton* radio, int index)
{
  GSList *group = gtk_radio_button_group(radio);
  return GTK_TOGGLE_BUTTON(g_slist_nth_data(group, g_slist_length(group) - index - 1));
}

void radio_button_set_active(GtkRadioButton* radio, int index)
{
  //radio_button_print_state(radio);
  gtk_toggle_button_set_active(radio_button_get_nth(radio, index), TRUE);
  //radio_button_print_state(radio);
}

void radio_button_set_active_no_signal(GtkRadioButton* radio, int index)
{
  {
    for(GSList* l = gtk_radio_button_get_group(radio); l != 0; l = g_slist_next(l))
    {
      g_signal_handler_block(G_OBJECT(l->data), gpointer_to_int(g_object_get_data(G_OBJECT(l->data), "handler")));
    }
  }
  radio_button_set_active(radio, index);
  {
    for(GSList* l = gtk_radio_button_get_group(radio); l != 0; l = g_slist_next(l))
    {
      g_signal_handler_unblock(G_OBJECT(l->data), gpointer_to_int(g_object_get_data(G_OBJECT(l->data), "handler")));
    }
  }
}

int radio_button_get_active(GtkRadioButton* radio)
{
  //radio_button_print_state(radio);
  GSList *group = gtk_radio_button_group(radio);
  int index = g_slist_length(group) - 1;
  for (; group != 0; group = g_slist_next(group))
  {
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(group->data)))
    {
      break;
    }
    else
    {
      index--;
    }
  }
  return index;
}

