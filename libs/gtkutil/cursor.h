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

#if !defined(INCLUDED_GTKUTIL_CURSOR_H)
#define INCLUDED_GTKUTIL_CURSOR_H

#include <glib/gmain.h>
#include <gdk/gdkevents.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkwindow.h>

#include "debugging/debugging.h"

typedef struct _GdkCursor GdkCursor;
typedef struct _GtkWidget GtkWidget;
typedef struct _GtkWindow GtkWindow;

GdkCursor* create_blank_cursor();
void blank_cursor(GtkWidget* widget);
void default_cursor(GtkWidget* widget);
void Sys_GetCursorPos(GtkWindow* window, int *x, int *y);
void Sys_SetCursorPos(GtkWindow* window, int x, int y);



class DeferredMotion
{
  guint m_handler;
  typedef void(*MotionFunction)(gdouble x, gdouble y, guint state, void* data);
  MotionFunction m_function;
  void* m_data;
  gdouble m_x;
  gdouble m_y;
  guint m_state;

  static gboolean deferred(DeferredMotion* self)
  {
    self->m_handler = 0;
    self->m_function(self->m_x, self->m_y, self->m_state, self->m_data);
    return FALSE;
  }
public:
  DeferredMotion(MotionFunction function, void* data) : m_handler(0), m_function(function), m_data(data)
  {
  }
  void motion(gdouble x, gdouble y, guint state)
  {
    m_x = x;
    m_y = y;
    m_state = state;
    if(m_handler == 0)
    {
      m_handler = g_idle_add((GSourceFunc)deferred, this);
    }
  }
  static gboolean gtk_motion(GtkWidget *widget, GdkEventMotion *event, DeferredMotion* self)
  {
    self->motion(event->x, event->y, event->state);
    return FALSE;
  }
};

class DeferredMotionDelta
{
  int m_delta_x;
  int m_delta_y;
  guint m_motion_handler;
  typedef void (*MotionDeltaFunction)(int x, int y, void* data);
  MotionDeltaFunction m_function;
  void* m_data;

  static gboolean deferred_motion(gpointer data)
  {
    reinterpret_cast<DeferredMotionDelta*>(data)->m_function(
      reinterpret_cast<DeferredMotionDelta*>(data)->m_delta_x,
      reinterpret_cast<DeferredMotionDelta*>(data)->m_delta_y,
      reinterpret_cast<DeferredMotionDelta*>(data)->m_data
    );
    reinterpret_cast<DeferredMotionDelta*>(data)->m_motion_handler = 0;
    reinterpret_cast<DeferredMotionDelta*>(data)->m_delta_x = 0;
    reinterpret_cast<DeferredMotionDelta*>(data)->m_delta_y = 0;
    return FALSE;
  }
public:
  DeferredMotionDelta(MotionDeltaFunction function, void* data) : m_delta_x(0), m_delta_y(0), m_motion_handler(0), m_function(function), m_data(data)
  {
  }
  void flush()
  {
    if(m_motion_handler != 0)
    {
      g_source_remove(m_motion_handler);
      deferred_motion(this);
    }
  }
  void motion_delta(int x, int y, unsigned int state)
  {
    m_delta_x += x;
    m_delta_y += y;
    if(m_motion_handler == 0)
    {
      m_motion_handler = g_idle_add(deferred_motion, this);
    }
  }
};

class FreezePointer
{
  unsigned int handle_motion;
  int recorded_x, recorded_y;
  typedef void (*MotionDeltaFunction)(int x, int y, unsigned int state, void* data);
  MotionDeltaFunction m_function;
  void* m_data;
public:
  FreezePointer() : handle_motion(0), m_function(0), m_data(0)
  {
  }
  static gboolean motion_delta(GtkWidget *widget, GdkEventMotion *event, FreezePointer* self)
  {
    int current_x, current_y;
    Sys_GetCursorPos(GTK_WINDOW(widget), &current_x, &current_y);
    int dx = current_x - self->recorded_x;
    int dy = current_y - self->recorded_y;
    if(dx != 0 || dy != 0)
    {
      //globalOutputStream() << "motion x: " << dx << ", y: " << dy << "\n";
      Sys_SetCursorPos(GTK_WINDOW(widget), self->recorded_x, self->recorded_y);
      self->m_function(dx, dy, event->state, self->m_data);
    }
    return FALSE;
  }

  void freeze_pointer(GtkWindow* window, MotionDeltaFunction function, void* data)
  {
    ASSERT_MESSAGE(m_function == 0, "can't freeze pointer");

    const GdkEventMask mask = static_cast<GdkEventMask>(GDK_POINTER_MOTION_MASK
    | GDK_POINTER_MOTION_HINT_MASK
    | GDK_BUTTON_MOTION_MASK
    | GDK_BUTTON1_MOTION_MASK
    | GDK_BUTTON2_MOTION_MASK
    | GDK_BUTTON3_MOTION_MASK
    | GDK_BUTTON_PRESS_MASK
    | GDK_BUTTON_RELEASE_MASK
    | GDK_VISIBILITY_NOTIFY_MASK);

    GdkCursor* cursor = create_blank_cursor();
   //GdkGrabStatus status = 
    gdk_pointer_grab(GTK_WIDGET(window)->window, TRUE, mask, 0, cursor, GDK_CURRENT_TIME);
    gdk_cursor_unref(cursor);

    Sys_GetCursorPos(window, &recorded_x, &recorded_y);

    Sys_SetCursorPos(window, recorded_x, recorded_y);

    m_function = function;
    m_data = data;

    handle_motion = g_signal_connect(G_OBJECT(window), "motion_notify_event", G_CALLBACK(motion_delta), this);
  }

  void unfreeze_pointer(GtkWindow* window)
  {
    g_signal_handler_disconnect(G_OBJECT(window), handle_motion);

    m_function = 0;
    m_data = 0;

    Sys_SetCursorPos(window, recorded_x, recorded_y);

    gdk_pointer_ungrab(GDK_CURRENT_TIME);
  }
};

#endif
