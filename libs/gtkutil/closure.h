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

#if !defined(INCLUDED_GTKUTIL_CLOSURE_H)
#define INCLUDED_GTKUTIL_CLOSURE_H

#include <glib-object.h>
#include "generic/callback.h"

inline void closure_destroy(gpointer data, GClosure* closure)
{
  delete reinterpret_cast<Callback*>(data);
}

inline GClosure* create_cclosure(GCallback func, const Callback& callback)
{
  return g_cclosure_new(func, new Callback(callback), closure_destroy);
}

inline GValue GValue_default()
{
  GValue value;
  value.g_type = 0;
  return value;
}

inline gint object_get_int_property(GObject* object, const char* property)
{
  GValue gvalue = GValue_default();
  g_value_init(&gvalue, G_TYPE_INT);
  g_object_get_property(object, property, &gvalue);
  return g_value_get_int(&gvalue);
}

inline void object_set_int_property(GObject* object, const char* property, gint value)
{
  GValue gvalue = GValue_default();
  g_value_init(&gvalue, G_TYPE_INT);
  g_value_set_int(&gvalue, value);
  g_object_set_property(object, property, &gvalue);
}

inline gboolean object_get_boolean_property(GObject* object, const char* property)
{
  GValue gvalue = GValue_default();
  g_value_init(&gvalue, G_TYPE_BOOLEAN);
  g_object_get_property(object, property, &gvalue);
  return g_value_get_boolean(&gvalue);
}

inline void object_set_boolean_property(GObject* object, const char* property, gboolean value)
{
  GValue gvalue = GValue_default();
  g_value_init(&gvalue, G_TYPE_BOOLEAN);
  g_value_set_boolean(&gvalue, value);
  g_object_set_property(object, property, &gvalue);
}

#endif
