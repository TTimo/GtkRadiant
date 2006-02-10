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

#if !defined(INCLUDED_GTKUTIL_NONMODAL_H)
#define INCLUDED_GTKUTIL_NONMODAL_H

#include <gtk/gtkwindow.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkradiobutton.h>
#include <gdk/gdkkeysyms.h>

#include "generic/callback.h"

#include "pointer.h"
#include "button.h"

typedef struct _GtkEntry GtkEntry;


inline gboolean escape_clear_focus_widget(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
  if(event->keyval == GDK_Escape)
  {
    gtk_window_set_focus(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(widget))), NULL);
    return TRUE;
  }
  return FALSE;
}

inline void widget_connect_escape_clear_focus_widget(GtkWidget* widget)
{
  g_signal_connect(G_OBJECT(widget), "key_press_event", G_CALLBACK(escape_clear_focus_widget), 0);
}


class NonModalEntry
{
  bool m_editing;
  Callback m_apply;
  Callback m_cancel;

  static gboolean focus_in(GtkEntry* entry, GdkEventFocus *event, NonModalEntry* self)
  {
    self->m_editing = false;
    return FALSE;
  }

  static gboolean focus_out(GtkEntry* entry, GdkEventFocus *event, NonModalEntry* self)
  {
    if(self->m_editing && GTK_WIDGET_VISIBLE(entry))
    {
      self->m_apply();
    }
    self->m_editing = false;
    return FALSE;
  }

  static gboolean changed(GtkEntry* entry, NonModalEntry* self)
  {
    self->m_editing = true;
    return FALSE;
  }

  static gboolean enter(GtkEntry* entry, GdkEventKey* event, NonModalEntry* self)
  {
    if(event->keyval == GDK_Return)
    {
      self->m_apply();
      self->m_editing = false;
      gtk_window_set_focus(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(entry))), NULL);
      return TRUE;
    }
    return FALSE;
  }

  static gboolean escape(GtkEntry* entry, GdkEventKey* event, NonModalEntry* self)
  {
    if(event->keyval == GDK_Escape)
    {
      self->m_cancel();
      self->m_editing = false;
      gtk_window_set_focus(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(entry))), NULL);
      return TRUE;
    }
    return FALSE;
  }

public:
  NonModalEntry(const Callback& apply, const Callback& cancel) : m_editing(false), m_apply(apply), m_cancel(cancel)
  {
  }
  void connect(GtkEntry* entry)
  {
    g_signal_connect(G_OBJECT(entry), "focus_in_event", G_CALLBACK(focus_in), this);
    g_signal_connect(G_OBJECT(entry), "focus_out_event", G_CALLBACK(focus_out), this);
    g_signal_connect(G_OBJECT(entry), "key_press_event", G_CALLBACK(enter), this);
    g_signal_connect(G_OBJECT(entry), "key_press_event", G_CALLBACK(escape), this);
    g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(changed), this);
  }
};


class NonModalSpinner
{
  Callback m_apply;
  Callback m_cancel;

  static gboolean changed(GtkSpinButton* spin, NonModalSpinner* self)
  {
    self->m_apply();
    return FALSE;
  }

  static gboolean enter(GtkSpinButton* spin, GdkEventKey* event, NonModalSpinner* self)
  {
    if(event->keyval == GDK_Return)
    {
      gtk_window_set_focus(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(spin))), NULL);
      return TRUE;
    }
    return FALSE;
  }

  static gboolean escape(GtkSpinButton* spin, GdkEventKey* event, NonModalSpinner* self)
  {
    if(event->keyval == GDK_Escape)
    {
      self->m_cancel();
      gtk_window_set_focus(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(spin))), NULL);
      return TRUE;
    }
    return FALSE;
  }

public:
  NonModalSpinner(const Callback& apply, const Callback& cancel) : m_apply(apply), m_cancel(cancel)
  {
  }
  void connect(GtkSpinButton* spin)
  {
    guint handler = g_signal_connect(G_OBJECT(gtk_spin_button_get_adjustment(spin)), "value_changed", G_CALLBACK(changed), this);
    g_object_set_data(G_OBJECT(spin), "handler", gint_to_pointer(handler));
    g_signal_connect(G_OBJECT(spin), "key_press_event", G_CALLBACK(enter), this);
    g_signal_connect(G_OBJECT(spin), "key_press_event", G_CALLBACK(escape), this);
  }
};

 
class NonModalRadio
{
  Callback m_changed;

public:
  NonModalRadio(const Callback& changed) : m_changed(changed)
  {
  }
  void connect(GtkRadioButton* radio)
  {
    GSList* group = gtk_radio_button_group(radio);
    for(; group != 0; group = g_slist_next(group))
    {
      toggle_button_connect_callback(GTK_TOGGLE_BUTTON(group->data), m_changed);
    }
  }
};


#endif
