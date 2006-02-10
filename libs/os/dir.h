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

#if !defined(INCLUDED_OS_DIR_H)
#define INCLUDED_OS_DIR_H

/// \file
/// \brief OS directory-listing object.

#include <glib/gdir.h>

typedef GDir Directory;

inline bool directory_good(Directory* directory)
{
  return directory != 0;
}

inline Directory* directory_open(const char* name)
{
  return g_dir_open(name, 0, 0);
}

inline void directory_close(Directory* directory)
{
  g_dir_close(directory);
}

inline const char* directory_read_and_increment(Directory* directory)
{
  return g_dir_read_name(directory);
}

template<typename Functor>
void Directory_forEach(const char* path, const Functor& functor)
{
  Directory* dir = directory_open(path);

  if(directory_good(dir))
  {
    for(;;)
    {
      const char* name = directory_read_and_increment(dir);
      if(name == 0)
      {
        break;
      }

      functor(name);
    }

    directory_close(dir);
  }
}


#endif
