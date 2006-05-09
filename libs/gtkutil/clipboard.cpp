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

#include "clipboard.h"

#include "stream/memstream.h"
#include "stream/textstream.h"


/// \file
/// \brief Platform-independent GTK clipboard support.
/// \todo Using GDK_SELECTION_CLIPBOARD fails on win32, so we use the win32 API directly for now.
#if defined(WIN32)

const char* c_clipboard_format = "RadiantClippings";

#include <windows.h>

void clipboard_copy(ClipboardCopyFunc copy)
{
  BufferOutputStream ostream;
  copy(ostream);

  bool bClipped = false;
  UINT nClipboard = ::RegisterClipboardFormat(c_clipboard_format);
  if (nClipboard > 0)
  {
    if (::OpenClipboard(0))
    {
      EmptyClipboard();
      std::size_t length = ostream.size();
      HANDLE h = ::GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE, length + sizeof(std::size_t));
      if (h != 0)
      {
        char *buffer = reinterpret_cast<char*>(::GlobalLock(h));
        *reinterpret_cast<std::size_t*>(buffer) = length;
        buffer += sizeof(std::size_t);
        memcpy(buffer, ostream.data(), length);
        ::GlobalUnlock(h);
        ::SetClipboardData(nClipboard, h);
        ::CloseClipboard();
        bClipped = true;
      }
    }
  }

  if (!bClipped)
  {
    globalOutputStream() << "Unable to register Windows clipboard formats, copy/paste between editors will not be possible\n";
  }
}

void clipboard_paste(ClipboardPasteFunc paste)
{
  UINT nClipboard = ::RegisterClipboardFormat(c_clipboard_format);
  if (nClipboard > 0 && ::OpenClipboard(0))
  {
    if(IsClipboardFormatAvailable(nClipboard))
    {
      HANDLE h = ::GetClipboardData(nClipboard);
      if(h)
      {
        const char *buffer = reinterpret_cast<const char*>(::GlobalLock(h));
        std::size_t length = *reinterpret_cast<const std::size_t*>(buffer);
        buffer += sizeof(std::size_t);
        BufferInputStream istream(buffer, length);
        paste(istream);
        ::GlobalUnlock(h);
      }
    }
    ::CloseClipboard();
  }
}

#else

#include <gtk/gtkclipboard.h>

enum
{
  RADIANT_CLIPPINGS = 23,
};

static const GtkTargetEntry clipboard_targets[] = {
  { "RADIANT_CLIPPINGS", 0, RADIANT_CLIPPINGS, },
};

static void clipboard_get (GtkClipboard *clipboard, GtkSelectionData *selection_data, guint info, gpointer data)
{
  std::size_t len = *reinterpret_cast<std::size_t*>(data);
  const char* buffer = (len != 0) ? reinterpret_cast<const char*>(data) + sizeof(std::size_t) : 0;

  GdkAtom type = GDK_NONE;
  if(info == clipboard_targets[0].info)
  {
    type = gdk_atom_intern(clipboard_targets[0].target, FALSE);
  }

  gtk_selection_data_set (selection_data, type, 8, reinterpret_cast<const guchar*>(buffer), static_cast<gint>(len));
}

static void clipboard_clear (GtkClipboard *clipboard, gpointer data)
{
  delete [] reinterpret_cast<const char*>(data);
}

static void clipboard_received (GtkClipboard *clipboard, GtkSelectionData *data, gpointer user_data)
{
  if (data->length < 0)
  {
    globalErrorStream() << "Error retrieving selection\n";
  }
  else if(strcmp(gdk_atom_name(data->type), clipboard_targets[0].target) == 0)
  {
    BufferInputStream istream(reinterpret_cast<const char*>(data->data), data->length);
    (*reinterpret_cast<ClipboardPasteFunc*>(user_data))(istream);
  }
}

void clipboard_copy(ClipboardCopyFunc copy)
{
  GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

  BufferOutputStream ostream;
  copy(ostream);
  std::size_t length = ostream.size();
  char* data = new char[length + sizeof(std::size_t)];
  *reinterpret_cast<std::size_t*>(data) = length;
  memcpy(data + sizeof(std::size_t), ostream.data(), length);

  gtk_clipboard_set_with_data (clipboard, clipboard_targets, 1, clipboard_get, clipboard_clear, data);
}

ClipboardPasteFunc g_clipboardPasteFunc = 0;
void clipboard_paste(ClipboardPasteFunc paste)
{
  GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  
  g_clipboardPasteFunc = paste;
  gtk_clipboard_request_contents (clipboard, gdk_atom_intern(clipboard_targets[0].target, FALSE), clipboard_received, &g_clipboardPasteFunc);
}


#endif
