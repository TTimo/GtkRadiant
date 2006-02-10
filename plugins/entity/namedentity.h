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

#if !defined(INCLUDED_NAMEDENTITY_H)
#define INCLUDED_NAMEDENTITY_H

#include "entitylib.h"
#include "eclasslib.h"
#include "generic/callback.h"
#include "nameable.h"

#include <set>

class NameCallbackSet
{
  typedef std::set<NameCallback> NameCallbacks;
  NameCallbacks m_callbacks;
public:
  void insert(const NameCallback& callback)
  {
    m_callbacks.insert(callback);
  }
  void erase(const NameCallback& callback)
  {
    m_callbacks.erase(callback);
  }
  void changed(const char* name) const
  {
    for(NameCallbacks::const_iterator i = m_callbacks.begin(); i != m_callbacks.end(); ++i)
    {
      (*i)(name);
    }
  }
};

class NamedEntity : public Nameable
{
  EntityKeyValues& m_entity;
  NameCallbackSet m_changed;
  CopiedString m_name;
public:
  NamedEntity(EntityKeyValues& entity) : m_entity(entity)
  {
  }
  const char* name() const
  {
    if(string_empty(m_name.c_str()))
    {
      return m_entity.getEntityClass().name();
    }
    return m_name.c_str();
  }
  void attach(const NameCallback& callback)
  {
    m_changed.insert(callback);
  }
  void detach(const NameCallback& callback)
  {
    m_changed.erase(callback);
  }

  void identifierChanged(const char* value)
  {
    if(string_empty(value))
    {
      m_changed.changed(m_entity.getEntityClass().name());
    }
    else
    {
      m_changed.changed(value);
    }
    m_name = value;
  }
  typedef MemberCaller1<NamedEntity, const char*, &NamedEntity::identifierChanged> IdentifierChangedCaller;
};

class RenderableNamedEntity : public OpenGLRenderable
{
  const NamedEntity& m_named;
  const Vector3& m_position;
public:
  RenderableNamedEntity(const NamedEntity& named, const Vector3& position)
    : m_named(named), m_position(position)
  {
  }
  void render(RenderStateFlags state) const
  {
    glRasterPos3fv(vector3_to_array(m_position));
    GlobalOpenGL().drawString(m_named.name());
  }
};



#endif
