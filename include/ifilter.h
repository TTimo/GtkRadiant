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

#if !defined(INCLUDED_IFILTER_H)
#define INCLUDED_IFILTER_H

#include "generic/constant.h"

enum
{
   EXCLUDE_WORLD            = 0x00000001,
   EXCLUDE_ENT              = 0x00000002,
   EXCLUDE_CURVES           = 0x00000004,
   EXCLUDE_TRANSLUCENT      = 0x00000008,
   EXCLUDE_LIQUIDS          = 0x00000010,
   EXCLUDE_CAULK            = 0x00000020,
   EXCLUDE_CLIP             = 0x00000040,
   EXCLUDE_PATHS            = 0x00000080,
   EXCLUDE_LIGHTS           = 0x00000100,
   EXCLUDE_DETAILS          = 0x00000200,
   EXCLUDE_HINTSSKIPS       = 0x00000400,
   EXCLUDE_MODELS           = 0x00000800,
   EXCLUDE_AREAPORTALS      = 0x00001000,
   EXCLUDE_TRIGGERS         = 0x00002000,
   EXCLUDE_CLUSTERPORTALS	  = 0x00004000,
   EXCLUDE_TERRAIN          = 0x00008000,
   EXCLUDE_LIGHTGRID        = 0x00010000,
   EXCLUDE_STRUCTURAL       = 0x00020000,
   EXCLUDE_BOTCLIP          = 0x00040000,
   EXCLUDE_VISPORTALS       = 0x00080000,
};

class Filter
{
public:
  virtual void setActive(bool active) = 0;
};

class Filterable
{
public:
  virtual void updateFiltered() = 0;
};

class FilterSystem
{
public:
  INTEGER_CONSTANT(Version, 1);
  STRING_CONSTANT(Name, "filters");
  virtual void addFilter(Filter& filter, int mask) = 0;
  virtual void registerFilterable(Filterable& filterable) = 0;
  virtual void unregisterFilterable(Filterable& filterable) = 0;
};

#include "modulesystem.h"

template<typename Type>
class GlobalModule;
typedef GlobalModule<FilterSystem> GlobalFilterModule;

template<typename Type>
class GlobalModuleRef;
typedef GlobalModuleRef<FilterSystem> GlobalFilterModuleRef;

inline FilterSystem& GlobalFilterSystem()
{
  return GlobalFilterModule::getTable();
}

#endif
