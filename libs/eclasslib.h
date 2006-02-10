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

#if !defined (INCLUDED_ECLASSLIB_H)
#define INCLUDED_ECLASSLIB_H

#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <map>
#include <vector>

#include "ieclass.h"
#include "irender.h"

#include "math/vector.h"
#include "string/string.h"

typedef Vector3 Colour3;

class ListAttributeType
{
  typedef std::pair<CopiedString, CopiedString> ListItem;
  typedef std::vector<ListItem> ListItems;
  ListItems m_items;
public:

  typedef ListItems::const_iterator const_iterator;
  const_iterator begin() const
  {
    return m_items.begin();
  }
  const_iterator end() const
  {
    return m_items.end();
  }

  const ListItem& operator[](std::size_t i) const
  {
    return m_items[i];
  }
  const_iterator findValue(const char* value) const
  {
    for(ListItems::const_iterator i = m_items.begin(); i != m_items.end(); ++i)
    {
      if(string_equal(value, (*i).second.c_str()))
      {
        return i;
      }
    }
    return m_items.end();
  }

  void push_back(const char* name, const char* value)
  {
    m_items.push_back(ListItems::value_type(name, value));
  }
};

class EntityClassAttribute
{
public:
  CopiedString m_type;
  CopiedString m_name;
  CopiedString m_value;
  CopiedString m_description;
  EntityClassAttribute()
  {
  }
  EntityClassAttribute(const char* type, const char* name, const char* value = "", const char* description = "") : m_type(type), m_name(name), m_value(value), m_description(description)
  {
  }
};

typedef std::pair<CopiedString, EntityClassAttribute> EntityClassAttributePair;
typedef std::list<EntityClassAttributePair> EntityClassAttributes;
typedef std::list<CopiedString> StringList;

inline const char* EntityClassAttributePair_getName(const EntityClassAttributePair& attributePair)
{
  if(!string_empty(attributePair.second.m_name.c_str()))
  {
    return attributePair.second.m_name.c_str();
  }
  return attributePair.first.c_str();
}

inline const char* EntityClassAttributePair_getDescription(const EntityClassAttributePair& attributePair)
{
  if(!string_empty(attributePair.second.m_description.c_str()))
  {
    return attributePair.second.m_description.c_str();
  }
  return EntityClassAttributePair_getName(attributePair);
}

class EntityClass
{
public:
	CopiedString m_name;
  StringList m_parent;
	bool	fixedsize;
	bool	unknown;		// wasn't found in source
	Vector3	mins;
  Vector3 maxs;

	Colour3	color;
  Shader* m_state_fill;
  Shader* m_state_wire;
  Shader* m_state_blend;

	CopiedString m_comments;
	char	flagnames[MAX_FLAGS][32];

  CopiedString m_modelpath;
  CopiedString m_skin;

  void (*free)(EntityClass*);

  EntityClassAttributes m_attributes;

  bool inheritanceResolved;
  bool sizeSpecified;
  bool colorSpecified;

  const char* name() const
  {
    return m_name.c_str();
  }
  const char* comments() const
  {
    return m_comments.c_str();
  }
  const char* modelpath() const
  {
    return m_modelpath.c_str();
  }
  const char* skin() const
  {
    return m_skin.c_str();
  }
};

inline const char* EntityClass_valueForKey(const EntityClass& entityClass, const char* key)
{
  for(EntityClassAttributes::const_iterator i = entityClass.m_attributes.begin(); i != entityClass.m_attributes.end(); ++i)
  {
    if(string_equal(key, (*i).first.c_str()))
    {
      return (*i).second.m_value.c_str();
    }
  }
  return "";
}

inline EntityClassAttributePair& EntityClass_insertAttribute(EntityClass& entityClass, const char* key, const EntityClassAttribute& attribute = EntityClassAttribute())
{
  entityClass.m_attributes.push_back(EntityClassAttributePair(key, attribute));
  return entityClass.m_attributes.back();
}


inline void buffer_write_colour_fill(char buffer[128], const Colour3& colour)
{
  sprintf(buffer, "(%g %g %g)", colour[0], colour[1], colour[2]);
}

inline void buffer_write_colour_wire(char buffer[128], const Colour3& colour)
{
  sprintf(buffer, "<%g %g %g>", colour[0], colour[1], colour[2]);
}

inline void buffer_write_colour_blend(char buffer[128], const Colour3& colour)
{
  sprintf(buffer, "[%g %g %g]", colour[0], colour[1], colour[2]);
}

inline Shader* colour_capture_state_fill(const Colour3& colour)
{
  char buffer[128];
  buffer_write_colour_fill(buffer, colour);
  return GlobalShaderCache().capture(buffer);
}

inline void colour_release_state_fill(const Colour3& colour)
{
  char buffer[128];
  buffer_write_colour_fill(buffer, colour);
  GlobalShaderCache().release(buffer);
}

inline Shader* colour_capture_state_wire(const Colour3& colour)
{
  char buffer[128];
  buffer_write_colour_wire(buffer, colour);
  return GlobalShaderCache().capture(buffer);
}

inline void colour_release_state_wire(const Colour3& colour)
{
  char buffer[128];
  buffer_write_colour_wire(buffer, colour);
  GlobalShaderCache().release(buffer);
}

inline Shader* colour_capture_state_blend(const Colour3& colour)
{
  char buffer[128];
  buffer_write_colour_blend(buffer, colour);
  return GlobalShaderCache().capture(buffer);
}

inline void colour_release_state_blend(const Colour3& colour)
{
  char buffer[128];
  buffer_write_colour_blend(buffer, colour);
  GlobalShaderCache().release(buffer);
}

inline void eclass_capture_state(EntityClass* eclass)
{
  eclass->m_state_fill = colour_capture_state_fill(eclass->color);
  eclass->m_state_wire = colour_capture_state_wire(eclass->color);
  eclass->m_state_blend = colour_capture_state_blend(eclass->color);
}

inline void eclass_release_state(EntityClass* eclass)
{
  colour_release_state_fill(eclass->color);
  colour_release_state_wire(eclass->color);
  colour_release_state_blend(eclass->color);
}

// eclass constructor
inline EntityClass* Eclass_Alloc()
{
  EntityClass* e = new EntityClass;

  e->fixedsize = false;
  e->unknown = false;
  memset(e->flagnames, 0, MAX_FLAGS*32);

  e->maxs = Vector3(-1,-1,-1);
  e->mins = Vector3(1, 1, 1);

  e->free = 0;

  e->inheritanceResolved = true;
  e->sizeSpecified = false;
  e->colorSpecified = false;

  return e;
}

// eclass destructor
inline void Eclass_Free(EntityClass* e)
{
  eclass_release_state(e);

  delete e;
}

inline bool classname_equal(const char* classname, const char* other)
{
  return string_equal(classname, other);
}

inline EntityClass* EClass_Create(const char* name, const Vector3& colour, const char* comments)
{
  EntityClass *e = Eclass_Alloc();
  e->free = &Eclass_Free;

  e->m_name = name;

	e->color = colour;
  eclass_capture_state(e);

  if (comments)
    e->m_comments = comments;

  return e;
}

inline EntityClass* EClass_Create_FixedSize(const char* name, const Vector3& colour, const Vector3& mins, const Vector3& maxs, const char* comments)
{
  EntityClass *e = Eclass_Alloc();
  e->free = &Eclass_Free;

  e->m_name = name;

	e->color = colour;
  eclass_capture_state(e);

  e->fixedsize = true;

  e->mins = mins;
  e->maxs = maxs;

  if (comments)
    e->m_comments = comments;

  return e;
}

const Vector3 smallbox[2] = {
  Vector3(-8,-8,-8),
  Vector3( 8, 8, 8),
};

inline EntityClass *EntityClass_Create_Default(const char *name, bool has_brushes)
{
	// create a new class for it
	if (has_brushes)
	{
    return EClass_Create(name, Vector3(0.0f, 0.5f, 0.0f), "Not found in source.");
	}
	else
	{
    return EClass_Create_FixedSize(name, Vector3(0.0f, 0.5f, 0.0f), smallbox[0], smallbox[1], "Not found in source.");
	}
}

#endif
