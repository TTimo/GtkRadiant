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

#include "accelerator.h"

#include "debugging/debugging.h"

#include <map>
#include <set>
#include <gtk/gtkwindow.h>
#include <gtk/gtkaccelgroup.h>

#include "generic/callback.h"
#include "generic/bitfield.h"

#include "pointer.h"
#include "closure.h"



typedef std::map<Accelerator, Callback> AcceleratorMap;

void accelerator_map_insert(AcceleratorMap& acceleratorMap, Accelerator accelerator, const Callback& callback)
{
  if(accelerator.key != 0)
  {
    ASSERT_MESSAGE(acceleratorMap.find(accelerator) == acceleratorMap.end(), "failed to add accelerator");
    acceleratorMap.insert(AcceleratorMap::value_type(accelerator, callback));
  }
}

void accelerator_map_erase(AcceleratorMap& acceleratorMap, Accelerator accelerator)
{
  if(accelerator.key != 0)
  {
    ASSERT_MESSAGE(acceleratorMap.find(accelerator) != acceleratorMap.end(), "failed to remove accelerator");
    acceleratorMap.erase(accelerator);
  }
}

Accelerator accelerator_for_event_key(guint keyval, guint state)
{
  keyval = gdk_keyval_to_upper(keyval);
  if(keyval == GDK_ISO_Left_Tab)
    keyval = GDK_Tab;
  return Accelerator(keyval, (GdkModifierType)(state & gtk_accelerator_get_default_mod_mask()));
}

bool AcceleratorMap_activate(const AcceleratorMap& acceleratorMap, const Accelerator& accelerator)
{
  AcceleratorMap::const_iterator i = acceleratorMap.find(accelerator);
  if(i != acceleratorMap.end())
  {
    (*i).second();
    return true;
  }

  return false;
}

static gboolean accelerator_key_event(GtkWindow* window, GdkEventKey* event, AcceleratorMap* acceleratorMap)
{
  return AcceleratorMap_activate(*acceleratorMap, accelerator_for_event_key(event->keyval, event->state));
}


AcceleratorMap g_special_accelerators;


namespace MouseButton
{
  enum 
  {
    Left = 1 << 0,
    Right = 1 << 1,
    Middle = 1 << 2,
  };
}

typedef unsigned int ButtonMask;

void print_buttons(ButtonMask mask)
{
  globalOutputStream() << "button state: ";
  if((mask & MouseButton::Left) != 0)
  {
    globalOutputStream() << "Left ";
  }
  if((mask & MouseButton::Right) != 0)
  {
    globalOutputStream() << "Right ";
  }
  if((mask & MouseButton::Middle) != 0)
  {
    globalOutputStream() << "Middle ";
  }
  globalOutputStream() << "\n";
}

ButtonMask ButtonMask_for_event_button(guint button)
{
  switch(button)
  {
  case 1:
    return MouseButton::Left;
  case 2:
    return MouseButton::Middle;
  case 3:
    return MouseButton::Right;
  }
  return 0;
}

bool window_has_accel(GtkWindow* toplevel)
{
  return g_slist_length(gtk_accel_groups_from_object(G_OBJECT(toplevel))) != 0;
}

namespace
{
  bool g_accel_enabled = true;
}

bool global_accel_enabled()
{
  return g_accel_enabled;
}


AcceleratorMap g_queuedAccelerators;

GClosure* accel_group_add_accelerator(GtkAccelGroup* group, Accelerator accelerator, const Callback& callback);

void GlobalQueuedAccelerators_commit()
{
  for(AcceleratorMap::const_iterator i = g_queuedAccelerators.begin(); i != g_queuedAccelerators.end(); ++i)
  {
    accel_group_add_accelerator(global_accel, (*i).first, (*i).second);
  }
  g_queuedAccelerators.clear();
}

void GlobalQueuedAccelerators_add(Accelerator accelerator, const Callback& callback)
{
  g_queuedAccelerators.insert(AcceleratorMap::value_type(accelerator, callback));
}

void accel_group_test(GtkWindow* toplevel, GtkAccelGroup* accel)
{
  guint n_entries;
  gtk_accel_group_query(accel, '4', (GdkModifierType)0, &n_entries);
  globalOutputStream() << "grid4: " << n_entries << "\n";
  globalOutputStream() << "toplevel accelgroups: " << g_slist_length(gtk_accel_groups_from_object(G_OBJECT(toplevel))) << "\n";
}

typedef std::set<GtkWindow*> WindowSet;
WindowSet g_accel_windows;

bool Buttons_press(ButtonMask& buttons, guint button, guint state)
{
  if(buttons == 0 && bitfield_enable(buttons, ButtonMask_for_event_button(button)) != 0)
  {
    ASSERT_MESSAGE(g_accel_enabled, "Buttons_press: accelerators not enabled");
    g_accel_enabled = false;
    for(WindowSet::iterator i = g_accel_windows.begin(); i != g_accel_windows.end(); ++i)
    {
      GtkWindow* toplevel = *i;
      ASSERT_MESSAGE(window_has_accel(toplevel), "ERROR");
      ASSERT_MESSAGE(GTK_WIDGET_TOPLEVEL(toplevel), "disabling accel for non-toplevel window");
      gtk_window_remove_accel_group(toplevel,  global_accel);
#if 0
      globalOutputStream() << reinterpret_cast<unsigned int>(toplevel) << ": disabled global accelerators\n";
#endif
#if 0
      accel_group_test(toplevel, global_accel);
#endif
    }
  }
  buttons = bitfield_enable(buttons, ButtonMask_for_event_button(button));
#if 0
  globalOutputStream() << "Buttons_press: ";
  print_buttons(buttons);
#endif
  return false;
}

bool Buttons_release(ButtonMask& buttons, guint button, guint state)
{
  if(buttons != 0 && bitfield_disable(buttons, ButtonMask_for_event_button(button)) == 0)
  {
    ASSERT_MESSAGE(!g_accel_enabled, "Buttons_release: accelerators are enabled");
    g_accel_enabled = true;
    for(WindowSet::iterator i = g_accel_windows.begin(); i != g_accel_windows.end(); ++i)
    {
      GtkWindow* toplevel = *i;
      ASSERT_MESSAGE(!window_has_accel(toplevel), "ERROR");
      ASSERT_MESSAGE(GTK_WIDGET_TOPLEVEL(toplevel), "enabling accel for non-toplevel window");
      gtk_window_add_accel_group(toplevel, global_accel);
#if 0
      globalOutputStream() << reinterpret_cast<unsigned int>(toplevel) << ": enabled global accelerators\n";
#endif
#if 0
      accel_group_test(toplevel, global_accel);
#endif
    }
    GlobalQueuedAccelerators_commit();
  }
  buttons = bitfield_disable(buttons, ButtonMask_for_event_button(button));
#if 0
  globalOutputStream() << "Buttons_release: ";
  print_buttons(buttons);
#endif
  return false;
}

bool Buttons_releaseAll(ButtonMask& buttons)
{
  Buttons_release(buttons, MouseButton::Left | MouseButton::Middle | MouseButton::Right, 0);
  return false;
}

struct PressedButtons
{
  ButtonMask buttons;

  PressedButtons() : buttons(0)
  {
  }
};

gboolean PressedButtons_button_press(GtkWidget* widget, GdkEventButton* event, PressedButtons* pressed)
{
  if(event->type == GDK_BUTTON_PRESS)
  {
    return Buttons_press(pressed->buttons, event->button, event->state);
  }
  return FALSE;
}

gboolean PressedButtons_button_release(GtkWidget* widget, GdkEventButton* event, PressedButtons* pressed)
{
  if(event->type == GDK_BUTTON_RELEASE)
  {
    return Buttons_release(pressed->buttons, event->button, event->state);
  }
  return FALSE;
}

gboolean PressedButtons_focus_out(GtkWidget* widget, GdkEventFocus* event, PressedButtons* pressed)
{
  Buttons_releaseAll(pressed->buttons);
  return FALSE;
}

void PressedButtons_connect(PressedButtons& pressedButtons, GtkWidget* widget)
{
  g_signal_connect(G_OBJECT(widget), "button_press_event", G_CALLBACK(PressedButtons_button_press), &pressedButtons);
  g_signal_connect(G_OBJECT(widget), "button_release_event", G_CALLBACK(PressedButtons_button_release), &pressedButtons);
  g_signal_connect(G_OBJECT(widget), "focus_out_event", G_CALLBACK(PressedButtons_focus_out), &pressedButtons);
}

PressedButtons g_pressedButtons;


#include <set>

struct PressedKeys
{
  typedef std::set<guint> Keys;
  Keys keys;
  std::size_t refcount;

  PressedKeys() : refcount(0)
  {
  }
};

AcceleratorMap g_keydown_accelerators;
AcceleratorMap g_keyup_accelerators;

bool Keys_press(PressedKeys::Keys& keys, guint keyval)
{
  if(keys.insert(keyval).second)
  {
    return AcceleratorMap_activate(g_keydown_accelerators, accelerator_for_event_key(keyval, 0));
  }
  return g_keydown_accelerators.find(accelerator_for_event_key(keyval, 0)) != g_keydown_accelerators.end();
}

bool Keys_release(PressedKeys::Keys& keys, guint keyval)
{
  if(keys.erase(keyval) != 0)
  {
    return AcceleratorMap_activate(g_keyup_accelerators, accelerator_for_event_key(keyval, 0));
  }
  return g_keyup_accelerators.find(accelerator_for_event_key(keyval, 0)) != g_keyup_accelerators.end();
}

void Keys_releaseAll(PressedKeys::Keys& keys, guint state)
{
  for(PressedKeys::Keys::iterator i = keys.begin(); i != keys.end(); ++i)
  {
    AcceleratorMap_activate(g_keyup_accelerators, accelerator_for_event_key(*i, state));
  }
  keys.clear();
}

gboolean PressedKeys_key_press(GtkWidget* widget, GdkEventKey* event, PressedKeys* pressedKeys)
{
  //globalOutputStream() << "pressed: " << event->keyval << "\n";
  return event->state == 0 && Keys_press(pressedKeys->keys, event->keyval);
}

gboolean PressedKeys_key_release(GtkWidget* widget, GdkEventKey* event, PressedKeys* pressedKeys)
{
  //globalOutputStream() << "released: " << event->keyval << "\n";
  return Keys_release(pressedKeys->keys, event->keyval);
}

gboolean PressedKeys_focus_in(GtkWidget* widget, GdkEventFocus* event, PressedKeys* pressedKeys)
{
  ++pressedKeys->refcount;
  return FALSE;
}

gboolean PressedKeys_focus_out(GtkWidget* widget, GdkEventFocus* event, PressedKeys* pressedKeys)
{
  if(--pressedKeys->refcount == 0)
  {
    Keys_releaseAll(pressedKeys->keys, 0);
  }
  return FALSE;
}

PressedKeys g_pressedKeys;

void GlobalPressedKeys_releaseAll()
{
  Keys_releaseAll(g_pressedKeys.keys, 0);
}

void GlobalPressedKeys_connect(GtkWindow* window)
{
  unsigned int key_press_handler = g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(PressedKeys_key_press), &g_pressedKeys);
  unsigned int key_release_handler = g_signal_connect(G_OBJECT(window), "key_release_event", G_CALLBACK(PressedKeys_key_release), &g_pressedKeys);
  g_object_set_data(G_OBJECT(window), "key_press_handler", gint_to_pointer(key_press_handler));
  g_object_set_data(G_OBJECT(window), "key_release_handler", gint_to_pointer(key_release_handler));
  unsigned int focus_in_handler = g_signal_connect(G_OBJECT(window), "focus_in_event", G_CALLBACK(PressedKeys_focus_in), &g_pressedKeys);
  unsigned int focus_out_handler = g_signal_connect(G_OBJECT(window), "focus_out_event", G_CALLBACK(PressedKeys_focus_out), &g_pressedKeys);
  g_object_set_data(G_OBJECT(window), "focus_in_handler", gint_to_pointer(focus_in_handler));
  g_object_set_data(G_OBJECT(window), "focus_out_handler", gint_to_pointer(focus_out_handler));
}

void GlobalPressedKeys_disconnect(GtkWindow* window)
{
  g_signal_handler_disconnect(G_OBJECT(window), gpointer_to_int(g_object_get_data(G_OBJECT(window), "key_press_handler")));
  g_signal_handler_disconnect(G_OBJECT(window), gpointer_to_int(g_object_get_data(G_OBJECT(window), "key_release_handler")));
  g_signal_handler_disconnect(G_OBJECT(window), gpointer_to_int(g_object_get_data(G_OBJECT(window), "focus_in_handler")));
  g_signal_handler_disconnect(G_OBJECT(window), gpointer_to_int(g_object_get_data(G_OBJECT(window), "focus_out_handler")));
}



void special_accelerators_add(Accelerator accelerator, const Callback& callback)
{
  accelerator_map_insert(g_special_accelerators, accelerator, callback);
}
void special_accelerators_remove(Accelerator accelerator)
{
  accelerator_map_erase(g_special_accelerators, accelerator);
}

void keydown_accelerators_add(Accelerator accelerator, const Callback& callback)
{
  accelerator_map_insert(g_keydown_accelerators, accelerator, callback);
}
void keydown_accelerators_remove(Accelerator accelerator)
{
  accelerator_map_erase(g_keydown_accelerators, accelerator);
}

void keyup_accelerators_add(Accelerator accelerator, const Callback& callback)
{
  accelerator_map_insert(g_keyup_accelerators, accelerator, callback);
}
void keyup_accelerators_remove(Accelerator accelerator)
{
  accelerator_map_erase(g_keyup_accelerators, accelerator);
}


gboolean accel_closure_callback(GtkAccelGroup* group, GtkWidget* widget, guint key, GdkModifierType modifiers, gpointer data)
{
  (*reinterpret_cast<Callback*>(data))();
  return TRUE;
}

GClosure* accel_group_add_accelerator(GtkAccelGroup* group, Accelerator accelerator, const Callback& callback)
{
  if(accelerator.key != 0 && gtk_accelerator_valid(accelerator.key, accelerator.modifiers))
  {
    //globalOutputStream() << "adding accelerator: " << accelerator.key << " " << accelerator.modifiers << "\n";
    GClosure* closure = create_cclosure(G_CALLBACK(accel_closure_callback), callback);
    gtk_accel_group_connect(group, accelerator.key, accelerator.modifiers, GTK_ACCEL_VISIBLE, closure);
    return closure;
  }
  else
  {
    special_accelerators_add(accelerator, callback);
    return 0;
  }
}

void accel_group_remove_accelerator(GtkAccelGroup* group, Accelerator accelerator)
{
  if(accelerator.key != 0 && gtk_accelerator_valid(accelerator.key, accelerator.modifiers))
  {
    gtk_accel_group_disconnect_key(group, accelerator.key, accelerator.modifiers);
  }
  else
  {
    special_accelerators_remove(accelerator);
  }
}

GtkAccelGroup* global_accel = 0;

void global_accel_init()
{
  global_accel = gtk_accel_group_new();
}

void global_accel_destroy()
{
  g_object_unref(global_accel);
}

GClosure* global_accel_group_add_accelerator(Accelerator accelerator, const Callback& callback)
{
  if(!global_accel_enabled())
  {
    // workaround: cannot add to GtkAccelGroup while it is disabled
    GlobalQueuedAccelerators_add(accelerator, callback);
    return 0;
  }
  return accel_group_add_accelerator(global_accel, accelerator, callback);
}
void global_accel_group_remove_accelerator(Accelerator accelerator)
{
  //ASSERT_MESSAGE(global_accel_enabled(), "removing accelerator while global accel is disabled");
  accel_group_remove_accelerator(global_accel, accelerator);
}

/// \brief Propagates key events to the focus-widget, overriding global accelerators.
static gboolean override_global_accelerators(GtkWindow* window, GdkEventKey* event, gpointer data)
{
  return gtk_window_propagate_key_event(window, event);
}

void global_accel_connect_window(GtkWindow* window)
{
#if 1
  unsigned int override_handler = g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(override_global_accelerators), 0);
  g_object_set_data(G_OBJECT(window), "override_handler", gint_to_pointer(override_handler));

  unsigned int special_key_press_handler = g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(accelerator_key_event), &g_special_accelerators);
  g_object_set_data(G_OBJECT(window), "special_key_press_handler", gint_to_pointer(special_key_press_handler));

  GlobalPressedKeys_connect(window);
#else
  unsigned int key_press_handler = g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(accelerator_key_event), &g_keydown_accelerators);
  unsigned int key_release_handler = g_signal_connect(G_OBJECT(window), "key_release_event", G_CALLBACK(accelerator_key_event), &g_keyup_accelerators);
  g_object_set_data(G_OBJECT(window), "key_press_handler", gint_to_pointer(key_press_handler));
  g_object_set_data(G_OBJECT(window), "key_release_handler", gint_to_pointer(key_release_handler));
#endif
  g_accel_windows.insert(window);
  gtk_window_add_accel_group(window, global_accel);
}
void global_accel_disconnect_window(GtkWindow* window)
{
#if 1
  GlobalPressedKeys_disconnect(window);

  g_signal_handler_disconnect(G_OBJECT(window), gpointer_to_int(g_object_get_data(G_OBJECT(window), "override_handler")));
  g_signal_handler_disconnect(G_OBJECT(window), gpointer_to_int(g_object_get_data(G_OBJECT(window), "special_key_press_handler")));
#else
  g_signal_handler_disconnect(G_OBJECT(window), gpointer_to_int(g_object_get_data(G_OBJECT(window), "key_press_handler")));
  g_signal_handler_disconnect(G_OBJECT(window), gpointer_to_int(g_object_get_data(G_OBJECT(window), "key_release_handler")));
#endif
  gtk_window_remove_accel_group(window, global_accel);
  std::size_t count = g_accel_windows.erase(window);
  ASSERT_MESSAGE(count == 1, "failed to remove accel group\n");
}


GClosure* global_accel_group_find(Accelerator accelerator)
{
  guint numEntries = 0;
  GtkAccelGroupEntry* entry = gtk_accel_group_query(global_accel, accelerator.key, accelerator.modifiers, &numEntries);
  if(numEntries != 0)
  {
    if(numEntries != 1)
    {
      char* name = gtk_accelerator_name(accelerator.key, accelerator.modifiers);
      globalErrorStream() << "accelerator already in-use: " << name << "\n";
      g_free(name);
    }
    return entry->closure;
  }
  return 0;
}

void global_accel_group_connect(const Accelerator& accelerator, const Callback& callback)
{
  if(accelerator.key != 0)
  {
    global_accel_group_add_accelerator(accelerator, callback);
  }
}

void global_accel_group_disconnect(const Accelerator& accelerator, const Callback& callback)
{
  if(accelerator.key != 0)
  {
    global_accel_group_remove_accelerator(accelerator);
  }
}


