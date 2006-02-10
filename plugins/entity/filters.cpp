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

#include "filters.h"

#include "ifilter.h"

#include <list>

class EntityFilterWrapper : public Filter
{
  bool m_active;
  bool m_invert;
  EntityFilter& m_filter;
public:
  EntityFilterWrapper(EntityFilter& filter, bool invert) : m_invert(invert), m_filter(filter)
  {
  }
  void setActive(bool active)
  {
    m_active = active;
  }
  bool active()
  {
    return m_active;
  }
  bool filter(const Entity& entity)
  {
    return m_invert ^ m_filter.filter(entity);
  }
};


typedef std::list<EntityFilterWrapper> EntityFilters;
EntityFilters g_entityFilters;

void add_entity_filter(EntityFilter& filter, int mask, bool invert)
{
  g_entityFilters.push_back(EntityFilterWrapper(filter, invert));
  GlobalFilterSystem().addFilter(g_entityFilters.back(), mask);
}

bool entity_filtered(Entity& entity)
{
  for(EntityFilters::iterator i = g_entityFilters.begin(); i != g_entityFilters.end(); ++i)
  {
    if((*i).active() && (*i).filter(entity))
    {
      return true;
    }
  }
  return false;
}

