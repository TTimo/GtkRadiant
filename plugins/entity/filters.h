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

#if !defined(INCLUDED_FILTERS_H)
#define INCLUDED_FILTERS_H

#include "ifilter.h"

#include "generic/callback.h"
#include "scenelib.h"

class Entity;

class EntityFilter
{
public:
  virtual bool filter(const Entity& entity) const = 0;
};

bool entity_filtered(Entity& entity);
void add_entity_filter(EntityFilter& filter, int mask, bool invert = false);

class ClassnameFilter : public Filterable
{
  scene::Node& m_node;
public:
  Entity& m_entity;

  ClassnameFilter(Entity& entity, scene::Node& node) : m_node(node), m_entity(entity)
  {
  }
  ~ClassnameFilter()
  {
  }

  void instanceAttach()
  {
    GlobalFilterSystem().registerFilterable(*this);
  }
  void instanceDetach()
  {
    GlobalFilterSystem().unregisterFilterable(*this);
  }

  void updateFiltered()
  {
    if(entity_filtered(m_entity))
    {
      m_node.enable(scene::Node::eFiltered);
    }
    else
    {
      m_node.disable(scene::Node::eFiltered);
    }
  }

  void classnameChanged(const char* value)
  {
    updateFiltered();
  }
  typedef MemberCaller1<ClassnameFilter, const char*, &ClassnameFilter::classnameChanged> ClassnameChangedCaller;
};

#endif
