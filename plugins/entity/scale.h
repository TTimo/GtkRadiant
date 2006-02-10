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

#if !defined(INCLUDED_SCALE_H)
#define INCLUDED_SCALE_H

#include "ientity.h"

#include "math/matrix.h"
#include "generic/callback.h"
#include "stringio.h"

const Vector3 SCALEKEY_IDENTITY = Vector3(1, 1, 1);

inline void default_scale(Vector3& scale)
{
  scale = SCALEKEY_IDENTITY;
}
inline void read_scale(Vector3& scalevec, const char* value)
{
  float scale;
  if(!string_parse_float(value, scale)
    || scale == 0)
  {
    default_scale(scalevec);
  }
  else
  {
    scalevec = Vector3(scale, scale, scale);
  }
}
inline void read_scalevec(Vector3& scale, const char* value)
{
  if(!string_parse_vector3(value, scale)
    || scale[0] == 0
    || scale[1] == 0
    || scale[2] == 0)
    default_scale(scale);
}
inline void write_scale(const Vector3& scale, Entity* entity)
{
  if(scale[0] == 1 && scale[1] == 1 && scale[2] == 1)
  {
    entity->setKeyValue("modelscale", "");
    entity->setKeyValue("modelscale_vec", "");
  }
  else
  {
    char value[64];

    if(scale[0] == scale[1] && scale[0] == scale[2])
    {
      sprintf(value, "%g", scale[0]);
      entity->setKeyValue("modelscale_vec", "");
      entity->setKeyValue("modelscale", value);
    }
    else
    {
      sprintf(value, "%g %g %g", scale[0], scale[1], scale[2]);
      entity->setKeyValue("modelscale", "");
      entity->setKeyValue("modelscale_vec", value);
    }
  }
}

inline Vector3 scale_scaled(const Vector3& scale, const Vector3& scaling)
{
  return matrix4_get_scale_vec3(
    matrix4_multiplied_by_matrix4(
      matrix4_scale_for_vec3(scale),
      matrix4_scale_for_vec3(scaling)
    )
  );
}


class ScaleKey
{
  Callback m_scaleChanged;
public:
  Vector3 m_scale;


  ScaleKey(const Callback& scaleChanged)
    : m_scaleChanged(scaleChanged), m_scale(SCALEKEY_IDENTITY)
  {
  }

  void uniformScaleChanged(const char* value)
  {
    read_scale(m_scale, value);
    m_scaleChanged();
  }
  typedef MemberCaller1<ScaleKey, const char*, &ScaleKey::uniformScaleChanged> UniformScaleChangedCaller;

  void scaleChanged(const char* value)
  {
    read_scalevec(m_scale, value);
    m_scaleChanged();
  }
  typedef MemberCaller1<ScaleKey, const char*, &ScaleKey::scaleChanged> ScaleChangedCaller;

  void write(Entity* entity) const
  {
    write_scale(m_scale, entity);
  }
};


#endif
