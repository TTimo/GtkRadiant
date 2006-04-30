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

#if !defined(INCLUDED_GTKUTIL_ACCELERATOR_H)
#define INCLUDED_GTKUTIL_ACCELERATOR_H

#include <gdk/gdktypes.h>
#include <gdk/gdkkeysyms.h>

#include "generic/callback.h"

struct Accelerator
{
  Accelerator(guint _key)
    : key(_key), modifiers((GdkModifierType)0)
  {
  }
  Accelerator(guint _key, GdkModifierType _modifiers)
    : key(_key), modifiers(_modifiers)
  {
  }
  bool operator<(const Accelerator& other) const
  {
    return key < other.key || (!(other.key < key) && modifiers < other.modifiers);
  }
  guint key;
  GdkModifierType modifiers;
};

inline Accelerator accelerator_null()
{
  return Accelerator(0, (GdkModifierType)0);
}

const char* global_keys_find(unsigned int key);
unsigned int global_keys_find(const char* name);

class TextOutputStream;
void accelerator_write(const Accelerator& accelerator, TextOutputStream& ostream);

template<typename TextOutputStreamType>
TextOutputStreamType& ostream_write(TextOutputStreamType& ostream, const Accelerator& accelerator)
{
  accelerator_write(accelerator, ostream);
  return ostream;
}

void keydown_accelerators_add(Accelerator accelerator, const Callback& callback);
void keydown_accelerators_remove(Accelerator accelerator);
void keyup_accelerators_add(Accelerator accelerator, const Callback& callback);
void keyup_accelerators_remove(Accelerator accelerator);

typedef struct _GtkWidget GtkWidget;
typedef struct _GtkWindow GtkWindow;
void global_accel_connect_window(GtkWindow* window);
void global_accel_disconnect_window(GtkWindow* window);

void GlobalPressedKeys_releaseAll();

typedef struct _GtkAccelGroup GtkAccelGroup;
extern GtkAccelGroup* global_accel;
void global_accel_init();
void global_accel_destroy();

GClosure* global_accel_group_find(Accelerator accelerator);

void global_accel_group_connect(const Accelerator& accelerator, const Callback& callback);
void global_accel_group_disconnect(const Accelerator& accelerator, const Callback& callback);


class Command
{
public:
  Callback m_callback;
  const Accelerator& m_accelerator;
  Command(const Callback& callback, const Accelerator& accelerator) : m_callback(callback), m_accelerator(accelerator)
  {
  }
};

class Toggle
{
public:
  Command m_command;
  BoolExportCallback m_exportCallback;
  Toggle(const Callback& callback, const Accelerator& accelerator, const BoolExportCallback& exportCallback) : m_command(callback, accelerator), m_exportCallback(exportCallback)
  {
  }
};

class KeyEvent
{
public:
  const Accelerator& m_accelerator;
  Callback m_keyDown;
  Callback m_keyUp;
  KeyEvent(const Accelerator& accelerator, const Callback& keyDown, const Callback& keyUp) : m_accelerator(accelerator), m_keyDown(keyDown), m_keyUp(keyUp)
  {
  }
};



struct PressedButtons;
typedef struct _GtkWidget GtkWidget;
void PressedButtons_connect(PressedButtons& pressedButtons, GtkWidget* widget);

extern PressedButtons g_pressedButtons;

#endif
