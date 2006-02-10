/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
For a list of contributors, see the accompanying CONTRIBUTORS file.

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

#include "mru.h"

#include <string.h>
#include <stdio.h>
#include <gtk/gtklabel.h>

#include "os/file.h"
#include "generic/callback.h"
#include "stream/stringstream.h"
#include "convert.h"

#include "gtkutil/menu.h"
#include "map.h"
#include "qe3.h"

#define MRU_MAX 4
namespace {
  GtkMenuItem *MRU_items[MRU_MAX];
  std::size_t MRU_used;
  typedef CopiedString MRU_filename_t;
  MRU_filename_t MRU_filenames[MRU_MAX];
  typedef const char* MRU_key_t;
  MRU_key_t MRU_keys[MRU_MAX] = { "File0", "File1", "File2", "File3" };
}

inline const char* MRU_GetText(std::size_t index)
{
  return MRU_filenames[index].c_str();
}

class EscapedMnemonic
{
  StringBuffer m_buffer;
public:
  EscapedMnemonic(std::size_t capacity) : m_buffer(capacity)
  {
    m_buffer.push_back('_');
  }
  const char* c_str() const
  {
    return m_buffer.c_str();
  }
  void push_back(char c) // not escaped
  {
    m_buffer.push_back(c);
  }
  std::size_t write(const char* buffer, std::size_t length)
  {
    for(const char* end = buffer + length; buffer != end; ++buffer)
    {
      if(*buffer == '_')
      {
        m_buffer.push_back('_');
      }

      m_buffer.push_back(*buffer);
    }
    return length;
  }
};

template<typename T>
inline EscapedMnemonic& operator<<(EscapedMnemonic& ostream, const T& t)
{
  return ostream_write(ostream, t);
}


void MRU_updateWidget(std::size_t index, const char *filename)
{
  EscapedMnemonic mnemonic(64);
  mnemonic.push_back('_');
  mnemonic << Unsigned(index + 1) << "- " << ConvertLocaleToUTF8(filename);
  gtk_label_set_text_with_mnemonic(GTK_LABEL(gtk_bin_get_child(GTK_BIN(MRU_items[index]))), mnemonic.c_str());
}

void MRU_SetText(std::size_t index, const char *filename)
{
  MRU_filenames[index] = filename;
  MRU_updateWidget(index, filename);
}

void MRU_AddFile (const char *str)
{
  std::size_t i;
  const char* text;

  // check if file is already in our list
  for (i = 0; i < MRU_used; i++)
  {
    text = MRU_GetText (i);

    if (strcmp (text, str) == 0)
    {
      // reorder menu
      for(; i > 0; i--)
	      MRU_SetText(i, MRU_GetText (i-1));

      MRU_SetText(0, str);

      return;
    }
  }

  if (MRU_used < MRU_MAX)
    MRU_used++;

  // move items down
  for (i = MRU_used-1; i > 0; i--)
    MRU_SetText (i, MRU_GetText (i-1));

  MRU_SetText (0, str);
  gtk_widget_set_sensitive(GTK_WIDGET(MRU_items[0]), TRUE);
  gtk_widget_show(GTK_WIDGET(MRU_items[MRU_used-1]));
}

void MRU_Init()
{
  if(MRU_used > MRU_MAX)
    MRU_used = MRU_MAX;
}

void MRU_AddWidget(GtkMenuItem *widget, std::size_t pos)
{
  if(pos < MRU_MAX)
  {
    MRU_items[pos] = widget;
    if(pos < MRU_used)
    {
      MRU_updateWidget(pos, MRU_GetText(pos));
      gtk_widget_set_sensitive(GTK_WIDGET(MRU_items[0]), TRUE);
      gtk_widget_show(GTK_WIDGET(MRU_items[pos]));
    }
  }
}

void MRU_Activate (std::size_t index)
{
  char text[1024];
  strcpy(text, MRU_GetText(index));

  if (file_readable(text)) //\todo Test 'map load succeeds' instead of 'file is readable'.
  {
    MRU_AddFile (text);
    Map_RegionOff();
    Map_Free();
    Map_LoadFile (text);
  }
  else
  {
    MRU_used--;

    for (std::size_t i = index; i < MRU_used; i++)
      MRU_SetText (i, MRU_GetText (i+1));

    if (MRU_used == 0)
    {
      gtk_label_set_text(GTK_LABEL(GTK_BIN(MRU_items[0])->child), "Recent Files");
      gtk_widget_set_sensitive(GTK_WIDGET(MRU_items[0]), FALSE);
    }
    else
    {
      gtk_widget_hide(GTK_WIDGET(MRU_items[MRU_used]));
    }
  }
}


class LoadMRU
{
  std::size_t m_number;
public:
  LoadMRU(std::size_t number)
    : m_number(number)
  {
  }
  void load()
  {
    if (ConfirmModified("Open Map"))
    {
      MRU_Activate(m_number - 1);
    }
  }
};

typedef MemberCaller<LoadMRU, &LoadMRU::load> LoadMRUCaller;

LoadMRU g_load_mru1(1);
LoadMRU g_load_mru2(2);
LoadMRU g_load_mru3(3);
LoadMRU g_load_mru4(4);

void MRU_constructMenu(GtkMenu* menu)
{
  {
    GtkMenuItem* item = create_menu_item_with_mnemonic(menu, "Recent Files", LoadMRUCaller(g_load_mru1));
    gtk_widget_set_sensitive(GTK_WIDGET(item), FALSE);
    MRU_AddWidget(item, 0);
  }
  {
    GtkMenuItem* item = create_menu_item_with_mnemonic(menu, "2", LoadMRUCaller(g_load_mru2));
    gtk_widget_hide(GTK_WIDGET(item));
    MRU_AddWidget(item, 1);
  }
  {
    GtkMenuItem* item = create_menu_item_with_mnemonic(menu, "3", LoadMRUCaller(g_load_mru3));
    gtk_widget_hide(GTK_WIDGET(item));
    MRU_AddWidget(item, 2);
  }
  {
    GtkMenuItem* item = create_menu_item_with_mnemonic(menu, "4", LoadMRUCaller(g_load_mru4));
    gtk_widget_hide(GTK_WIDGET(item));
    MRU_AddWidget(item, 3);
  }
}

#include "preferencesystem.h"
#include "stringio.h"

void MRU_Construct()
{
  GlobalPreferenceSystem().registerPreference("Count", SizeImportStringCaller(MRU_used), SizeExportStringCaller(MRU_used));
  
  for(std::size_t i = 0; i != MRU_MAX; ++i)
  {
    GlobalPreferenceSystem().registerPreference(MRU_keys[i], CopiedStringImportStringCaller(MRU_filenames[i]), CopiedStringExportStringCaller(MRU_filenames[i]));
  }

  MRU_Init();
}
void MRU_Destroy()
{
}
