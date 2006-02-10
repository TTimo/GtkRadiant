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

#if !defined (INCLUDED_TRANSFORMLIB_H)
#define INCLUDED_TRANSFORMLIB_H

#include "generic/constant.h"
#include "math/matrix.h"
#include "math/quaternion.h"


/// \brief A transform node.
class TransformNode
{
public:
  STRING_CONSTANT(Name, "TransformNode");
  /// \brief Returns the transform which maps the node's local-space into the local-space of its parent node.
  virtual const Matrix4& localToParent() const  = 0;
};

/// \brief A transform node which has no effect.
class IdentityTransform : public TransformNode
{
public:
  /// \brief Returns the identity matrix.
  const Matrix4& localToParent() const
  {
    return g_matrix4_identity;
  }
};

/// \brief A transform node which stores a generic transformation matrix.
class MatrixTransform : public TransformNode
{
  Matrix4 m_localToParent;
public:
  MatrixTransform() : m_localToParent(g_matrix4_identity)
  {
  }

  Matrix4& localToParent()
  {
    return m_localToParent;
  }
  /// \brief Returns the stored local->parent transform.
  const Matrix4& localToParent() const
  {
    return m_localToParent;
  }
};


#include "generic/callback.h"

typedef Vector3 Translation;
typedef Quaternion Rotation;
typedef Vector3 Scale;

inline Matrix4 matrix4_transform_for_components(const Translation& translation, const Rotation& rotation, const Scale& scale)
{
  Matrix4 result(matrix4_rotation_for_quaternion_quantised(rotation));
  vector4_to_vector3(result.x()) *= scale.x();
  vector4_to_vector3(result.y()) *= scale.y();
  vector4_to_vector3(result.z()) *= scale.z();
  result.tx() = translation.x();
  result.ty() = translation.y();
  result.tz() = translation.z();
  return result;
}

typedef bool TransformModifierType;
const TransformModifierType TRANSFORM_PRIMITIVE = false;
const TransformModifierType TRANSFORM_COMPONENT = true;

/// \brief A transformable scene-graph instance.
///
/// A transformable instance may be translated, rotated or scaled.
/// The state of the instanced node's geometrical representation
/// will be the product of its geometry and the transforms of each
/// of its instances, applied in the order they appear in a graph
/// traversal.
/// Freezing the transform on an instance will cause its transform
/// to be permanently applied to the geometry of the node.
class Transformable
{
public:
  STRING_CONSTANT(Name, "Transformable");

  virtual void setType(TransformModifierType type) = 0;
  virtual void setTranslation(const Translation& value) = 0;
  virtual void setRotation(const Rotation& value) = 0;
  virtual void setScale(const Scale& value) = 0;
  virtual void freezeTransform() = 0;
};

const Translation c_translation_identity = Translation(0, 0, 0);
const Rotation c_rotation_identity = c_quaternion_identity;
const Scale c_scale_identity = Scale(1, 1, 1);


class TransformModifier : public Transformable
{
  Translation m_translation;
  Rotation m_rotation;
  Scale m_scale;
  Callback m_changed;
  Callback m_apply;
  TransformModifierType m_type;
public:

  TransformModifier(const Callback& changed, const Callback& apply) :
    m_translation(c_translation_identity),
    m_rotation(c_quaternion_identity),
    m_scale(c_scale_identity),
    m_changed(changed),
    m_apply(apply),
    m_type(TRANSFORM_PRIMITIVE)
  {
  }
  void setType(TransformModifierType type)
  {
    m_type = type;
  }
  TransformModifierType getType() const
  {
    return m_type;
  }
  void setTranslation(const Translation& value)
  {
    m_translation = value;
    m_changed();
  }
  void setRotation(const Rotation& value)
  {
    m_rotation = value;
    m_changed();
  }
  void setScale(const Scale& value)
  {
    m_scale = value;
    m_changed();
  }
  void freezeTransform()
  {
    if(m_translation != c_translation_identity
      || m_rotation != c_rotation_identity
      || m_scale != c_scale_identity)
    {
      m_apply();
      m_translation = c_translation_identity;
      m_rotation = c_rotation_identity;
      m_scale = c_scale_identity;
      m_changed();
    }
  }
  const Translation& getTranslation() const
  {
    return m_translation;
  }
  const Rotation& getRotation() const
  {
    return m_rotation;
  }
  const Scale& getScale() const
  {
    return m_scale;
  }
  Matrix4 calculateTransform() const
  {
    return matrix4_transform_for_components(getTranslation(), getRotation(), getScale());
  }
};


#endif
