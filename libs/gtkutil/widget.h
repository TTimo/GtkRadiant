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

#if !defined(INCLUDED_GTKUTIL_WIDGET_H)
#define INCLUDED_GTKUTIL_WIDGET_H

#include <list>
#include <gtk/gtkwidget.h>
#include "generic/callback.h"
#include "warnings.h"
#include "debugging/debugging.h"

inline void widget_set_visible(GtkWidget* widget, bool shown)
{
  if(shown)
  {
    gtk_widget_show(widget);
  }
  else
  {
    gtk_widget_hide(widget);
  }
}

inline bool widget_is_visible(GtkWidget* widget)
{
  return GTK_WIDGET_VISIBLE(widget) != FALSE;
}

inline void widget_toggle_visible(GtkWidget* widget)
{
  widget_set_visible(widget, !widget_is_visible(widget));
}

class ToggleItem
{
  BoolExportCallback m_exportCallback;
  typedef std::list<BoolImportCallback> ImportCallbacks;
  ImportCallbacks m_importCallbacks;
public:
  ToggleItem(const BoolExportCallback& exportCallback) : m_exportCallback(exportCallback)
  {
  }

  void update()
  {
    for(ImportCallbacks::iterator i = m_importCallbacks.begin(); i != m_importCallbacks.end(); ++i)
    {
      m_exportCallback(*i);
    }
  }

  void addCallback(const BoolImportCallback& callback)
  {
    m_importCallbacks.push_back(callback);
    m_exportCallback(callback);
  }
  typedef MemberCaller1<ToggleItem, const BoolImportCallback&, &ToggleItem::addCallback> AddCallbackCaller;
};

class ToggleShown
{
  bool m_shownDeferred;

  ToggleShown(const ToggleShown& other); // NOT COPYABLE
  ToggleShown& operator=(const ToggleShown& other); // NOT ASSIGNABLE

  static gboolean notify_visible(GtkWidget* widget, gpointer dummy, ToggleShown* self)
  {
    self->update();
    return FALSE;
  }
  static gboolean destroy(GtkWidget* widget, ToggleShown* self)
  {
    self->m_shownDeferred = GTK_WIDGET_VISIBLE(self->m_widget) != FALSE;
    self->m_widget = 0;
    return FALSE;
  }
public:
  GtkWidget* m_widget;
  ToggleItem m_item;

  ToggleShown(bool shown)
    : m_shownDeferred(shown), m_widget(0), m_item(ActiveCaller(*this))
  {
  }
  void update()
  {
    m_item.update();
  }
  bool active() const
  {
    if(m_widget == 0)
    {
      return m_shownDeferred;
    }
    else
    {
      return GTK_WIDGET_VISIBLE(m_widget) != FALSE;
    }
  }
  void exportActive(const BoolImportCallback& importCallback)
  {
    importCallback(active());
  }
  typedef MemberCaller1<ToggleShown, const BoolImportCallback&, &ToggleShown::exportActive> ActiveCaller;
  void set(bool shown)
  {
    if(m_widget == 0)
    {
      m_shownDeferred = shown;
    }
    else
    {
      widget_set_visible(m_widget, shown);
    }
  }
  void toggle()
  {
    widget_toggle_visible(m_widget);
  }
  typedef MemberCaller<ToggleShown, &ToggleShown::toggle> ToggleCaller;
  void connect(GtkWidget* widget)
  {
    m_widget = widget;
    widget_set_visible(m_widget, m_shownDeferred);
    g_signal_connect(G_OBJECT(m_widget), "notify::visible", G_CALLBACK(notify_visible), this);
    g_signal_connect(G_OBJECT(m_widget), "destroy", G_CALLBACK(destroy), this);
    update();
  }
};


inline void widget_queue_draw(GtkWidget& widget)
{
  gtk_widget_queue_draw(&widget);
}
typedef ReferenceCaller<GtkWidget, widget_queue_draw> WidgetQueueDrawCaller;


inline void widget_make_default(GtkWidget* widget)
{
  GTK_WIDGET_SET_FLAGS(widget, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(widget);
}

class WidgetFocusPrinter
{
  const char* m_name;

  static gboolean focus_in(GtkWidget *widget, GdkEventFocus *event, WidgetFocusPrinter* self)
  {
    globalOutputStream() << self->m_name << " takes focus\n";
    return FALSE;
  }
  static gboolean focus_out(GtkWidget *widget, GdkEventFocus *event, WidgetFocusPrinter* self)
  {
    globalOutputStream() << self->m_name << " loses focus\n";
    return FALSE;
  }
public:
  WidgetFocusPrinter(const char* name) : m_name(name)
  {
  }
  void connect(GtkWidget* widget)
  {
    g_signal_connect(G_OBJECT(widget), "focus_in_event", G_CALLBACK(focus_in), this);
    g_signal_connect(G_OBJECT(widget), "focus_out_event", G_CALLBACK(focus_out), this);
  }
};

#endif
