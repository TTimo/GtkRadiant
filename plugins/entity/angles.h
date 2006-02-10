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

#if !defined(INCLUDED_ANGLES_H)
#define INCLUDED_ANGLES_H

#include "ientity.h"

#include "math/quaternion.h"
#include "generic/callback.h"
#include "stringio.h"

#include "angle.h"

const Vector3 ANGLESKEY_IDENTITY = Vector3(0, 0, 0);

inline void default_angles(Vector3& angles)
{
  angles = ANGLESKEY_IDENTITY;
}
inline void normalise_angles(Vector3& angles)
{
  angles[0] = static_cast<float>(float_mod(angles[0], 360));
  angles[1] = static_cast<float>(float_mod(angles[1], 360));
  angles[2] = static_cast<float>(float_mod(angles[2], 360));
}
inline void read_angle(Vector3& angles, const char* value)
{
  if(!string_parse_float(value, angles[2]))
  {
    default_angles(angles);
  }
  else
  {
    angles[0] = 0;
    angles[1] = 0;
    normalise_angles(angles);
  }
}
inline void read_angles(Vector3& angles, const char* value)
{
  if(!string_parse_vector3(value, angles))
  {
    default_angles(angles);
  }
  else
  {
    angles = Vector3(angles[2], angles[0], angles[1]);
    normalise_angles(angles);
  }
}
inline void write_angles(const Vector3& angles, Entity* entity)
{
  if(angles[0] == 0
    && angles[1] == 0
    && angles[2] == 0)
  {
    entity->setKeyValue("angle", "");
    entity->setKeyValue("angles", "");
  }
  else
  {
    char value[64];

    if(angles[0] == 0 && angles[1] == 0)
    {
      entity->setKeyValue("angles", "");
      write_angle(angles[2], entity);
    }
    else
    {
      sprintf(value, "%g %g %g", angles[1], angles[2], angles[0]);
      entity->setKeyValue("angle", "");
      entity->setKeyValue("angles", value);
    }
  }
}

inline Vector3 angles_rotated(const Vector3& angles, const Quaternion& rotation)
{
  return matrix4_get_rotation_euler_xyz_degrees(
    matrix4_multiplied_by_matrix4(
      matrix4_rotation_for_euler_xyz_degrees(angles),
      matrix4_rotation_for_quaternion_quantised(rotation)
    )
  );
}

class AnglesKey
{
  Callback m_anglesChanged;
public:
  Vector3 m_angles;


  AnglesKey(const Callback& anglesChanged)
    : m_anglesChanged(anglesChanged), m_angles(ANGLESKEY_IDENTITY)
  {
  }

  void angleChanged(const char* value)
  {
    read_angle(m_angles, value);
    m_anglesChanged();
  }
  typedef MemberCaller1<AnglesKey, const char*, &AnglesKey::angleChanged> AngleChangedCaller;

  void anglesChanged(const char* value)
  {
    read_angles(m_angles, value);
    m_anglesChanged();
  }
  typedef MemberCaller1<AnglesKey, const char*, &AnglesKey::anglesChanged> AnglesChangedCaller;

  void write(Entity* entity) const
  {
    write_angles(m_angles, entity);
  }
};


#endif
