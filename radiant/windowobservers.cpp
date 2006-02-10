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

#include "windowobservers.h"

#include <vector>
#include <gdk/gdkevents.h>
#include "generic/bitfield.h"

namespace
{
  ModifierFlags g_modifier_state = c_modifierNone;
}

typedef std::vector<WindowObserver*> WindowObservers;

inline void WindowObservers_OnModifierDown(WindowObservers& observers, ModifierFlags type)
{
  g_modifier_state = bitfield_enable(g_modifier_state, type);
  for(WindowObservers::iterator i = observers.begin(); i != observers.end(); ++i)
  {
    (*i)->onModifierDown(type);
  }
}

inline void WindowObservers_OnModifierUp(WindowObservers& observers, ModifierFlags type)
{
  g_modifier_state = bitfield_disable(g_modifier_state, type);
  for(WindowObservers::iterator i = observers.begin(); i != observers.end(); ++i)
  {
    (*i)->onModifierUp(type);
  }
}

#include <gdk/gdkkeysyms.h>

gboolean selection_modifier_key_press(GtkWidget* widget, GdkEventKey* event, WindowObservers& observers)
{
  switch(event->keyval)
  {
  case GDK_Alt_L:
  case GDK_Alt_R:
    //globalOutputStream() << "Alt PRESSED\n";
    WindowObservers_OnModifierDown(observers, c_modifierAlt);
    break;
  case GDK_Shift_L:
  case GDK_Shift_R:
    //globalOutputStream() << "Shift PRESSED\n";
    WindowObservers_OnModifierDown(observers, c_modifierShift);
    break;
  case GDK_Control_L:
  case GDK_Control_R:
    //globalOutputStream() << "Control PRESSED\n";
    WindowObservers_OnModifierDown(observers, c_modifierControl);
    break;
  }
  return FALSE;
}

gboolean selection_modifier_key_release(GtkWidget* widget, GdkEventKey* event, WindowObservers& observers)
{
  switch(event->keyval)
  {
  case GDK_Alt_L:
  case GDK_Alt_R:
    //globalOutputStream() << "Alt RELEASED\n";
    WindowObservers_OnModifierUp(observers, c_modifierAlt);
    break;
  case GDK_Shift_L:
  case GDK_Shift_R:
    //globalOutputStream() << "Shift RELEASED\n";
    WindowObservers_OnModifierUp(observers, c_modifierShift);
    break;
  case GDK_Control_L:
  case GDK_Control_R:
    //globalOutputStream() << "Control RELEASED\n";
    WindowObservers_OnModifierUp(observers, c_modifierControl);
    break;
  }
  return FALSE;
}

void WindowObservers_UpdateModifier(WindowObservers& observers, ModifierFlags modifiers, ModifierFlags modifier)
{
  if(!bitfield_enabled(g_modifier_state, modifier) && bitfield_enabled(modifiers, modifier))
  {
    WindowObservers_OnModifierDown(observers, modifier);
  }
  if(bitfield_enabled(g_modifier_state, modifier) && !bitfield_enabled(modifiers, modifier))
  {
    WindowObservers_OnModifierUp(observers, modifier);
  }
}

void WindowObservers_UpdateModifiers(WindowObservers& observers, ModifierFlags modifiers)
{
  WindowObservers_UpdateModifier(observers, modifiers, c_modifierAlt);
  WindowObservers_UpdateModifier(observers, modifiers, c_modifierShift);
  WindowObservers_UpdateModifier(observers, modifiers, c_modifierControl);
}

gboolean modifiers_button_press(GtkWidget* widget, GdkEventButton* event, WindowObservers* observers)
{
  if(event->type == GDK_BUTTON_PRESS)
  {
    WindowObservers_UpdateModifiers(*observers, modifiers_for_state(event->state));
  }
  return FALSE;
}

gboolean modifiers_button_release(GtkWidget* widget, GdkEventButton* event, WindowObservers* observers)
{
  if(event->type == GDK_BUTTON_RELEASE)
  {
    WindowObservers_UpdateModifiers(*observers, modifiers_for_state(event->state));
  }
  return FALSE;
}

gboolean modifiers_motion(GtkWidget *widget, GdkEventMotion *event, WindowObservers* observers)
{
  WindowObservers_UpdateModifiers(*observers, modifiers_for_state(event->state));
  return FALSE;
}


WindowObservers g_window_observers;

void GlobalWindowObservers_updateModifiers(ModifierFlags modifiers)
{
  WindowObservers_UpdateModifiers(g_window_observers, modifiers);
}

void GlobalWindowObservers_add(WindowObserver* observer)
{
  g_window_observers.push_back(observer);
}

void GlobalWindowObservers_connectTopLevel(GtkWindow* window)
{
  g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(selection_modifier_key_press), &g_window_observers);
  g_signal_connect(G_OBJECT(window), "key_release_event", G_CALLBACK(selection_modifier_key_release), &g_window_observers);
}

void GlobalWindowObservers_connectWidget(GtkWidget* widget)
{
  g_signal_connect(G_OBJECT(widget), "button_press_event", G_CALLBACK(modifiers_button_press), &g_window_observers);
  g_signal_connect(G_OBJECT(widget), "button_release_event", G_CALLBACK(modifiers_button_release), &g_window_observers);
  g_signal_connect(G_OBJECT(widget), "motion_notify_event", G_CALLBACK(modifiers_motion), &g_window_observers);
}


