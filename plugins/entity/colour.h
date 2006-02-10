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

#if !defined(INCLUDED_COLOUR_H)
#define INCLUDED_COLOUR_H

#include "ientity.h"
#include "irender.h"

#include "math/vector.h"
#include "eclasslib.h"
#include "generic/callback.h"
#include "stringio.h"

inline void default_colour(Vector3& colour)
{
  colour = Vector3(1, 1, 1);
}
inline void read_colour(Vector3& colour, const char* value)
{
  if(!string_parse_vector3(value, colour))
  {
    default_colour(colour);
  }
}
inline void write_colour(const Vector3& colour, Entity* entity)
{
  char value[64];

  sprintf(value, "%f %f %f", colour[0], colour[1], colour[2]);
  entity->setKeyValue("_color", value);
}

class Colour
{
  Callback m_colourChanged;
  Shader* m_state;

  void capture_state()
  {
    m_state = colour_capture_state_fill(m_colour);
  }
  void release_state()
  {
    colour_release_state_fill(m_colour);
  }

public:
  Vector3 m_colour;

  Colour(const Callback& colourChanged)
    : m_colourChanged(colourChanged)
  {
    default_colour(m_colour);
    capture_state();
  }
  ~Colour()
  {
    release_state();
  }

  void colourChanged(const char* value)
  {
    release_state();
    read_colour(m_colour, value);
    capture_state();

    m_colourChanged();
  }
  typedef MemberCaller1<Colour, const char*, &Colour::colourChanged> ColourChangedCaller;


  void write(Entity* entity) const
  {
    write_colour(m_colour, entity);
  }

  Shader* state() const
  {
    return m_state;
  }
};

#endif
