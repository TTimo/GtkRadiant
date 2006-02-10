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

#if !defined(INCLUDED_GTKUTIL_WINDOW_H)
#define INCLUDED_GTKUTIL_WINDOW_H

#include <gtk/gtkwindow.h>

#include "debugging/debugging.h"
#include "generic/callback.h"
#include "widget.h"

inline gboolean window_focus_in_clear_focus_widget(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
  gtk_window_set_focus(GTK_WINDOW(widget), NULL);
  return FALSE;
}

inline guint window_connect_focus_in_clear_focus_widget(GtkWindow* window)
{
  return g_signal_connect(G_OBJECT(window), "focus_in_event", G_CALLBACK(window_focus_in_clear_focus_widget), NULL);
}


unsigned int connect_floating(GtkWindow* main_window, GtkWindow* floating);
GtkWindow* create_floating_window(const char* title, GtkWindow* parent);
void destroy_floating_window(GtkWindow* window);

GtkWindow* create_persistent_floating_window(const char* title, GtkWindow* main_window);
gboolean persistent_floating_window_delete(GtkWindow* floating, GdkEvent *event, GtkWindow* main_window);

void window_remove_minmax(GtkWindow* window);

typedef struct _GtkScrolledWindow GtkScrolledWindow;
GtkScrolledWindow* create_scrolled_window(GtkPolicyType hscrollbar_policy, GtkPolicyType vscrollbar_policy, int border = 0);


struct WindowPosition
{
  int x, y, w, h;

  WindowPosition()
  {
  }
  WindowPosition(int _x, int _y, int _w, int _h)
    : x(_x), y(_y), w(_w), h(_h)
  {
  }
};

const WindowPosition c_default_window_pos(50, 25, 400, 300);

inline void window_get_position(GtkWindow* window, WindowPosition& position)
{
  ASSERT_MESSAGE(window != 0, "error saving window position");

  gtk_window_get_position(window, &position.x, &position.y);
  gtk_window_get_size(window, &position.w, &position.h);
}

inline void window_set_position(GtkWindow* window, const WindowPosition& position)
{
  gtk_window_set_gravity(window, GDK_GRAVITY_STATIC);

  GdkScreen* screen = gdk_screen_get_default();
  if(position.x < 0
    || position.y < 0
    || position.x > gdk_screen_get_width(screen)
    || position.y > gdk_screen_get_height(screen))
  {
    gtk_window_set_position(window, GTK_WIN_POS_CENTER_ON_PARENT);
  }
  else
  {
    gtk_window_move(window, position.x, position.y);
  }

  gtk_window_set_default_size(window, position.w, position.h);
}

inline void WindowPosition_Parse(WindowPosition& position, const char* value)
{
  if(sscanf(value, "%d %d %d %d", &position.x, &position.y, &position.w, &position.h) != 4)
  {
    position = WindowPosition(c_default_window_pos); // ensure sane default value for window position
  }
}
typedef ReferenceCaller1<WindowPosition, const char*, WindowPosition_Parse> WindowPositionImportStringCaller;

inline void WindowPosition_Write(const WindowPosition& position, const StringImportCallback& importCallback)
{
  char buffer[64];
  sprintf(buffer, "%d %d %d %d", position.x, position.y, position.w, position.h);
  importCallback(buffer);
}
typedef ConstReferenceCaller1<WindowPosition, const StringImportCallback&, WindowPosition_Write> WindowPositionExportStringCaller;



class WindowPositionTracker
{
  WindowPosition m_position;

  static gboolean configure(GtkWidget* widget, GdkEventConfigure *event, WindowPositionTracker* self)
  {
    self->m_position = WindowPosition(event->x, event->y, event->width, event->height);
    return FALSE;
  }

public:
  WindowPositionTracker()
    : m_position(c_default_window_pos)
  {
  }

  void sync(GtkWindow* window)
  {
    window_set_position(window, m_position);
  }

  void connect(GtkWindow* window)
  {
    sync(window);
    g_signal_connect(G_OBJECT(window), "configure_event", G_CALLBACK(configure), this);
  }

  const WindowPosition& getPosition() const
  {
    return m_position;
  }

  //hack
  void setPosition(const WindowPosition& position)
  {
    m_position = position;
  }
};


inline void WindowPositionTracker_importString(WindowPositionTracker& self, const char* value)
{
  WindowPosition position;
  WindowPosition_Parse(position, value);
  self.setPosition(position);
}
typedef ReferenceCaller1<WindowPositionTracker, const char*, WindowPositionTracker_importString> WindowPositionTrackerImportStringCaller;

inline void WindowPositionTracker_exportString(const WindowPositionTracker& self, const StringImportCallback& importer)
{
  WindowPosition_Write(self.getPosition(), importer);
}
typedef ConstReferenceCaller1<WindowPositionTracker, const StringImportCallback&, WindowPositionTracker_exportString> WindowPositionTrackerExportStringCaller;



#endif
