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

#include "commands.h"

#include "debugging/debugging.h"
#include "warnings.h"

#include <map>
#include "string/string.h"
#include "versionlib.h"
#include "gtkutil/accelerator.h"

typedef std::pair<Accelerator, bool> ShortcutValue; // accelerator, isRegistered
typedef std::map<CopiedString, ShortcutValue> Shortcuts;

void Shortcuts_foreach(Shortcuts& shortcuts, CommandVisitor& visitor)
{
  for(Shortcuts::iterator i = shortcuts.begin(); i != shortcuts.end(); ++i)
  {
    visitor.visit((*i).first.c_str(), (*i).second.first);
  }
}

Shortcuts g_shortcuts;

const Accelerator& GlobalShortcuts_insert(const char* name, const Accelerator& accelerator)
{
  return (*g_shortcuts.insert(Shortcuts::value_type(name, ShortcutValue(accelerator, false))).first).second.first;
}

void GlobalShortcuts_foreach(CommandVisitor& visitor)
{
  Shortcuts_foreach(g_shortcuts, visitor);
}

void GlobalShortcuts_register(const char* name)
{
  Shortcuts::iterator i = g_shortcuts.find(name);
  if(i != g_shortcuts.end())
  {
    (*i).second.second = true;
  }
}

void GlobalShortcuts_reportUnregistered()
{
  for(Shortcuts::iterator i = g_shortcuts.begin(); i != g_shortcuts.end(); ++i)
  {
    if((*i).second.first.key != 0 && !(*i).second.second)
    {
      globalOutputStream() << "shortcut not registered: " << (*i).first.c_str() << "\n";
    }
  }
}

typedef std::map<CopiedString, Command> Commands;

Commands g_commands;

void GlobalCommands_insert(const char* name, const Callback& callback, const Accelerator& accelerator)
{
  bool added = g_commands.insert(Commands::value_type(name, Command(callback, GlobalShortcuts_insert(name, accelerator)))).second;
  ASSERT_MESSAGE(added, "command already registered: " << makeQuoted(name));
}

const Command& GlobalCommands_find(const char* command)
{
  Commands::iterator i = g_commands.find(command);
  ASSERT_MESSAGE(i != g_commands.end(), "failed to lookup command " << makeQuoted(command));
  return (*i).second;
}

typedef std::map<CopiedString, Toggle> Toggles;


Toggles g_toggles;

void GlobalToggles_insert(const char* name, const Callback& callback, const BoolExportCallback& exportCallback, const Accelerator& accelerator)
{
  bool added = g_toggles.insert(Toggles::value_type(name, Toggle(callback, GlobalShortcuts_insert(name, accelerator), exportCallback))).second;
  ASSERT_MESSAGE(added, "toggle already registered: " << makeQuoted(name));
}
const Toggle& GlobalToggles_find(const char* name)
{
  Toggles::iterator i = g_toggles.find(name);
  ASSERT_MESSAGE(i != g_toggles.end(), "failed to lookup toggle " << makeQuoted(name));
  return (*i).second;
}

typedef std::map<CopiedString, KeyEvent> KeyEvents;


KeyEvents g_keyEvents;

void GlobalKeyEvents_insert(const char* name, const Accelerator& accelerator, const Callback& keyDown, const Callback& keyUp)
{
  bool added = g_keyEvents.insert(KeyEvents::value_type(name, KeyEvent(GlobalShortcuts_insert(name, accelerator), keyDown, keyUp))).second;
  ASSERT_MESSAGE(added, "command already registered: " << makeQuoted(name));
}
const KeyEvent& GlobalKeyEvents_find(const char* name)
{
  KeyEvents::iterator i = g_keyEvents.find(name);
  ASSERT_MESSAGE(i != g_keyEvents.end(), "failed to lookup keyEvent " << makeQuoted(name));
  return (*i).second;
}




#include <cctype>

#include <gtk/gtkbox.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtktreemodel.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkcellrenderertext.h>

#include "gtkutil/dialog.h"
#include "mainframe.h"

#include "stream/textfilestream.h"
#include "stream/stringstream.h"


struct command_list_dialog_t : public ModalDialog
{
  command_list_dialog_t()
    : m_close_button(*this, eIDCANCEL)
  {
  }
  ModalDialogButton m_close_button;
};

void DoCommandListDlg()
{
  command_list_dialog_t dialog;

  GtkWindow* window = create_modal_dialog_window(MainFrame_getWindow(), "Mapped Commands", dialog, -1, 400);

  GtkAccelGroup* accel = gtk_accel_group_new();
  gtk_window_add_accel_group(window, accel);

  GtkHBox* hbox = create_dialog_hbox(4, 4);
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(hbox));

  {
    GtkScrolledWindow* scr = create_scrolled_window(GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(scr), TRUE, TRUE, 0);

    {
      GtkListStore* store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

      GtkWidget* view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

      {
        GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("Command", renderer, "text", 0, 0);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
      }

      {
        GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes("Key", renderer, "text", 1, 0);
        gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
      }

      gtk_widget_show(view);
      gtk_container_add(GTK_CONTAINER (scr), view);

      {
        // Initialize dialog
        StringOutputStream path(256);
        path << SettingsPath_get() << "commandlist.txt";
        globalOutputStream() << "Writing the command list to " << path.c_str() << "\n";
        class BuildCommandList : public CommandVisitor
        {
          TextFileOutputStream m_commandList;
          GtkListStore* m_store;
        public:
          BuildCommandList(const char* filename, GtkListStore* store) : m_commandList(filename), m_store(store)
          {
          }
          void visit(const char* name, Accelerator& accelerator)
          {
            StringOutputStream modifiers;
            modifiers << accelerator;

            {
              GtkTreeIter iter;
              gtk_list_store_append(m_store, &iter);
              gtk_list_store_set(m_store, &iter, 0, name, 1, modifiers.c_str(), -1);
            }
 
            if(!m_commandList.failed())
            {
              m_commandList << makeLeftJustified(name, 25) << " " << modifiers.c_str() << '\n';
            }
          }
        } visitor(path.c_str(), store);

        GlobalShortcuts_foreach(visitor);
      }
    
      g_object_unref(G_OBJECT(store));
    }
  }

  GtkVBox* vbox = create_dialog_vbox(4);
  gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(vbox), FALSE, FALSE, 0);
  {
    GtkButton* button = create_modal_dialog_button("Close", dialog.m_close_button);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(button), FALSE, FALSE, 0);
    widget_make_default(GTK_WIDGET(button));
    gtk_widget_grab_focus(GTK_WIDGET(button));
    gtk_widget_add_accelerator(GTK_WIDGET(button), "clicked", accel, GDK_Return, (GdkModifierType)0, (GtkAccelFlags)0);
    gtk_widget_add_accelerator(GTK_WIDGET(button), "clicked", accel, GDK_Escape, (GdkModifierType)0, (GtkAccelFlags)0);
  }

  modal_dialog_show(window, dialog);
  gtk_widget_destroy(GTK_WIDGET(window));
}

#include "profile/profile.h"

const char* const COMMANDS_VERSION = "1.0";

void SaveCommandMap(const char* path)
{
  StringOutputStream strINI(256);
  strINI << path << "shortcuts.ini";

  TextFileOutputStream file(strINI.c_str());
  if(!file.failed())
  {
    file << "[Version]\n";
    file << "number=" << COMMANDS_VERSION << "\n";
    file << "\n";
    file << "[Commands]\n";
    class WriteCommandMap : public CommandVisitor
    {
      TextFileOutputStream& m_file;
    public:
      WriteCommandMap(TextFileOutputStream& file) : m_file(file)
      {
      }
      void visit(const char* name, Accelerator& accelerator)
      {
        m_file << name << "=";

        const char* key = global_keys_find(accelerator.key);
        if(!string_empty(key))
        {
          m_file << key;
        }
        else if(accelerator.key != 0)
        {
          m_file << gdk_keyval_name(accelerator.key);
        }

        if(accelerator.modifiers & GDK_MOD1_MASK)
        {
          m_file << "+Alt";
        }
        if(accelerator.modifiers & GDK_CONTROL_MASK)
        {
          m_file << "+Ctrl";
        }
        if(accelerator.modifiers & GDK_SHIFT_MASK)
        {
          m_file << "+Shift";
        }

        m_file << "\n";
      }
    } visitor(file);
    GlobalShortcuts_foreach(visitor);
  }
}

const char* stringrange_find(const char* first, const char* last, char c)
{
  const char* p = strchr(first, '+');
  if(p == 0)
  {
    return last;
  }
  return p;
}

class ReadCommandMap : public CommandVisitor
{
  const char* m_filename;
  std::size_t m_count;
public:
  ReadCommandMap(const char* filename) : m_filename(filename), m_count(0)
  {
  }
  void visit(const char* name, Accelerator& accelerator)
  {
    char value[1024];
    if (read_var(m_filename, "Commands", name, value ))
    {
      if(string_empty(value))
      {
        accelerator.key = 0;
        accelerator.modifiers = (GdkModifierType)0;
        return;
      }
      int modifiers = 0;
      const char* last = value + string_length(value);
      const char* keyEnd = stringrange_find(value, last, '+');
      for(const char* modifier = keyEnd; modifier != last;)
      {
        const char* next = stringrange_find(modifier + 1, last, '+');
        if(next - modifier == 4
          && string_equal_nocase_n(modifier, "+alt", 4))
        {
          modifiers |= GDK_MOD1_MASK;
        }
        else if(next - modifier == 5
          && string_equal_nocase_n(modifier, "+ctrl", 5) != 0)
        {
          modifiers |= GDK_CONTROL_MASK;
        }
        else if(next - modifier == 6
          && string_equal_nocase_n(modifier, "+shift", 6) != 0)
        {
          modifiers |= GDK_SHIFT_MASK;
        }
        else
        {
          globalOutputStream() << "WARNING: failed to parse user command " << makeQuoted(value) << ": unknown modifier " << makeQuoted(StringRange(modifier, next)) << "\n";
        }
        modifier = next;
      }
      accelerator.modifiers = (GdkModifierType)modifiers;


      // strBuff has been cleaned of it's modifiers .. switch between a regular key and a virtual one
      // based on length
      if(keyEnd - value == 1) // most often case.. deal with first
      {
        accelerator.key = std::toupper(value[0]);
        ++m_count;
      }
      else // special key
      {
        CopiedString keyName(StringRange(value, keyEnd));
        accelerator.key = global_keys_find(keyName.c_str());
        if(accelerator.key != 0)
        {
          ++m_count;
        }
        else
        {
          globalOutputStream() << "WARNING: failed to parse user command " << makeQuoted(value) << ": unknown key " << makeQuoted(keyName.c_str()) << "\n";
        }
      }
    }
  }
  std::size_t count() const
  {
    return m_count;
  }
};
    
void LoadCommandMap(const char* path)
{
  StringOutputStream strINI(256);
  strINI << path << "shortcuts.ini";

  FILE* f = fopen (strINI.c_str(), "r");
  if (f != 0)
  {
    fclose(f);
    globalOutputStream() << "loading custom shortcuts list from " << makeQuoted(strINI.c_str()) << "\n";

    Version version = version_parse(COMMANDS_VERSION);
    Version dataVersion = { 0, 0 };

    {
      char value[1024];
      if(read_var(strINI.c_str(), "Version", "number", value))
      {
        dataVersion = version_parse(value);
      }
    }

    if(version_compatible(version, dataVersion))
    {
      globalOutputStream() << "commands import: data version " << dataVersion << " is compatible with code version " << version << "\n";
      ReadCommandMap visitor(strINI.c_str());
      GlobalShortcuts_foreach(visitor);
      globalOutputStream() << "parsed " << Unsigned(visitor.count()) << " custom shortcuts\n";
    }
    else
    {
      globalOutputStream() << "commands import: data version " << dataVersion << " is not compatible with code version " << version << "\n";
    }
  }
  else
  {
    globalOutputStream() << "failed to load custom shortcuts from " << makeQuoted(strINI.c_str()) << "\n";
  }
}
