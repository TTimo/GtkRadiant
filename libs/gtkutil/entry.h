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

#if !defined(INCLUDED_GTKUTIL_ENTRY_H)
#define INCLUDED_GTKUTIL_ENTRY_H

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtkentry.h>

inline void entry_set_string(GtkEntry* entry, const char* string)
{
  gtk_entry_set_text(entry, string);
}

inline void entry_set_int(GtkEntry* entry, int i)
{
  char buf[32];
  sprintf(buf, "%d", i);
  entry_set_string(entry, buf);
}

inline void entry_set_float(GtkEntry* entry, float f)
{
  char buf[32];
  sprintf(buf, "%g", f);
  entry_set_string(entry, buf);
}

inline const char* entry_get_string(GtkEntry* entry)
{
  return gtk_entry_get_text(entry);
}

inline int entry_get_int(GtkEntry* entry)
{
  return atoi(entry_get_string(entry));
}

inline double entry_get_float(GtkEntry* entry)
{
  return atof(entry_get_string(entry));
}

#endif
