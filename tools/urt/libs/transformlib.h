
#if !defined (INCLUDED_TRANSFORMLIB_H)
#define INCLUDED_TRANSFORMLIB_H

class Matrix4;

/// \brief A transform node.
class Transformable
{
public:
  static const char* getTypeName()
  {
    return "Transformable";
  }
  /// \brief Returns the transform which maps the node's local-space into the local-space of its parent node.
  virtual const Matrix4& localToParent() const  = 0;
};

#include "math/matrix.h"
#include "math/quaternion.h"


/// \brief A transform node which has no effect.
class IdentityTransform : public Transformable
{
public:
  /// \brief Returns the identity matrix.
  const Matrix4& localToParent() const
  {
    return g_matrix4_identity;
  }
};

/// \brief A transform node which stores a generic transformation matrix.
class MatrixTransform : public Transformable
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


namespace Transform
{
  class Translation
  {
    Vector3 m_value;
  public:
    Translation()
      : m_value(0, 0, 0)
    {
    }
    const Vector3& get() const
    {
      return m_value;
    }
    void set(const Vector3& value)
    {
      m_value = value;
    }
    void translate(const Vector3& value)
    {
      m_value[0] += value[0];
      m_value[1] += value[1];
      m_value[2] += value[2];
    }
  };

  class Rotation
  {
    Quaternion m_value;
  public:
    Rotation()
      : m_value(c_quaternion_identity)
    {
    }
    const Quaternion& get() const
    {
      return m_value;
    }
    void set(const Quaternion& value)
    {
      m_value = value;
    }
    void rotate(const Quaternion& value)
    {
      quaternion_multiply_by_quaternion(m_value, value);
      quaternion_normalise(m_value);
    }
  };

  class Scale
  {
    Vector3 m_value;
  public:
    Scale()
      : m_value(1, 1, 1)
    {
    }
    const Vector3& get() const
    {
      return m_value;
    }
    void set(const Vector3& value)
    {
      m_value = value;
    }
    void scale(const Vector3& value)
    {
      m_value[0] *= value[0];
      m_value[1] *= value[1];
      m_value[2] *= value[2];
    }
  };
}

/// \brief A transform node composed of separate translation, rotation and scale transforms.
///
/// - Each component transform can be individually modified at any time.
/// - The transforms are applied in the order: Scale, Rotate, Translate.
/// - The transformation matrix produced is guaranteed to be affine and orthogonal.
class ComponentTransform : public Transformable
{
  Transform::Translation m_translation;
  Transform::Rotation m_rotation;
  Transform::Scale m_scale;
  mutable Matrix4 m_transform;
  mutable bool m_changed;

  void changed() const
  {
    m_changed = true;
  }
  void evaluate() const
  {
    if(m_changed)
    {
      m_changed = false;

      Matrix4 rotation(matrix4_rotation_for_quaternion_quantised(m_rotation.get()));

      m_transform.xx() = rotation.xx() * m_scale.get().x();
      m_transform.xy() = rotation.xy() * m_scale.get().x();
      m_transform.xz() = rotation.xz() * m_scale.get().x();
      m_transform.xw() = 0;

      m_transform.yx() = rotation.yx() * m_scale.get().y();
      m_transform.yy() = rotation.yy() * m_scale.get().y();
      m_transform.yz() = rotation.yz() * m_scale.get().y();
      m_transform.yw() = 0;

      m_transform.zx() = rotation.zx() * m_scale.get().z();
      m_transform.zy() = rotation.zy() * m_scale.get().z();
      m_transform.zz() = rotation.zz() * m_scale.get().z();
      m_transform.zw() = 0;

      m_transform.tx() = m_translation.get().x();
      m_transform.ty() = m_translation.get().y();
      m_transform.tz() = m_translation.get().z();
      m_transform.tw() = 1;
    }
  }
public:
  ComponentTransform()
    : m_changed(true)
  {
  }
  const Matrix4& localToParent() const
  {
    evaluate();
    return m_transform;
  }

  const Vector3& getTranslation()
  {
    return m_translation.get();
  }
  const Quaternion& getRotation()
  {
    return m_rotation.get();
  }
  const Vector3& getScale()
  {
    return m_scale.get();
  }

  void setTranslation(const Vector3& value)
  {
    m_translation.set(value);
    changed();
  }
  void setRotation(const Quaternion& value)
  {
    m_rotation.set(value);
    changed();
  }
  void setScale(const Vector3& value)
  {
    m_scale.set(value);
    changed();
  }

  void translate(const Vector3& translation)
  {
    m_translation.translate(translation);
    changed();
  }
  void rotate(const Quaternion& rotation)
  {
    m_rotation.rotate(rotation);
    changed();
  }
  void scale(const Vector3& scale)
  {
    m_scale.scale(scale);
    changed();
  }

  void identity_translate()
  {
    m_translation = Transform::Translation();
    changed();
  }
  void identity_rotate()
  {
    m_rotation = Transform::Rotation();
    changed();
  }
  void identity_scale()
  {
    m_scale = Transform::Scale();
    changed();
  }
};


#endif
