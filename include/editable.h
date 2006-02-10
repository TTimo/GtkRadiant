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

#if !defined(INCLUDED_EDITABLE_H)
#define INCLUDED_EDITABLE_H

template<typename Element> class BasicVector3;
typedef BasicVector3<float> Vector3;
template<typename Element> class BasicVector4;
typedef BasicVector4<float> Vector4;
class Matrix4;
typedef Vector4 Quaternion;

#include "scenelib.h"

class Editable
{
public:
  STRING_CONSTANT(Name, "Editable");

  virtual const Matrix4& getLocalPivot() const = 0;
};

inline Editable* Node_getEditable(scene::Node& node)
{
  return NodeTypeCast<Editable>::cast(node);
}

class Snappable
{
public:
  STRING_CONSTANT(Name, "Snappable");

  virtual void snapto(float snap) = 0;
};

inline Snappable* Node_getSnappable(scene::Node& node)
{
  return NodeTypeCast<Snappable>::cast(node);
}

#endif
