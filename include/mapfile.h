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

#if !defined(INCLUDED_MAPFILE_H)
#define INCLUDED_MAPFILE_H

#include <limits>

#include "iscenegraph.h"
#include "generic/callbackfwd.h"

const std::size_t MAPFILE_MAX_CHANGES = std::numeric_limits<std::size_t>::max();

class MapFile
{
public:
  STRING_CONSTANT(Name, "MapFile");

  virtual void save() = 0;
  virtual bool saved() const = 0;
  virtual void changed() = 0;
  virtual void setChangedCallback(const Callback& changed) = 0;
  virtual std::size_t changes() const = 0;
};

#include "scenelib.h"

inline MapFile* Node_getMapFile(scene::Node& node)
{
  return NodeTypeCast<MapFile>::cast(node);
}

template<typename Iterator>
inline MapFile* path_find_mapfile(Iterator first, Iterator last)
{
  Iterator i = last;
  for(;;)
  {
    --i;

    MapFile* map = Node_getMapFile(*i);
    if(map != 0)
    {
      return map;
    }

    if(i == first)
    {
      break;
    }
  }
  ERROR_MESSAGE("failed to find parent mapfile for path");
  return 0;
}



#endif
