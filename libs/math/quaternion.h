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

#if !defined(INCLUDED_MATH_QUATERNION_H)
#define INCLUDED_MATH_QUATERNION_H

/// \file
/// \brief Quaternion data types and related operations.

#include "math/matrix.h"

/// \brief A quaternion stored in single-precision floating-point.
typedef Vector4 Quaternion;

const Quaternion c_quaternion_identity(0, 0, 0, 1);

inline Quaternion quaternion_multiplied_by_quaternion(const Quaternion& quaternion, const Quaternion& other)
{
  return Quaternion(
    quaternion[3]*other[0] + quaternion[0]*other[3] + quaternion[1]*other[2] - quaternion[2]*other[1],
    quaternion[3]*other[1] + quaternion[1]*other[3] + quaternion[2]*other[0] - quaternion[0]*other[2],
    quaternion[3]*other[2] + quaternion[2]*other[3] + quaternion[0]*other[1] - quaternion[1]*other[0],
    quaternion[3]*other[3] - quaternion[0]*other[0] - quaternion[1]*other[1] - quaternion[2]*other[2]
  );
}

inline void quaternion_multiply_by_quaternion(Quaternion& quaternion, const Quaternion& other)
{
  quaternion = quaternion_multiplied_by_quaternion(quaternion, other);
}

/// \brief Constructs a quaternion which rotates between two points on the unit-sphere, \p from and \p to.
inline Quaternion quaternion_for_unit_vectors(const Vector3& from, const Vector3& to)
{
  return Quaternion(vector3_cross(from, to), static_cast<float>(vector3_dot(from, to)));
}

inline Quaternion quaternion_for_axisangle(const Vector3& axis, double angle)
{
  angle *= 0.5;
  float sa = static_cast<float>(sin(angle));
  return Quaternion(axis[0] * sa, axis[1] * sa, axis[2] * sa, static_cast<float>(cos(angle)));
}

inline Quaternion quaternion_inverse(const Quaternion& quaternion)
{
  return Quaternion(vector3_negated(vector4_to_vector3(quaternion)), quaternion[3]);
}

inline void quaternion_conjugate(Quaternion& quaternion)
{
  quaternion = quaternion_inverse(quaternion);
}

inline Quaternion quaternion_normalised(const Quaternion& quaternion)
{
  const double n = (1.0 / (quaternion[0] * quaternion[0] + quaternion[1] * quaternion[1] + quaternion[2] * quaternion[2] + quaternion[3] * quaternion[3]));
  return Quaternion(
    static_cast<float>(quaternion[0] * n),
    static_cast<float>(quaternion[1] * n),
    static_cast<float>(quaternion[2] * n),
    static_cast<float>(quaternion[3] * n)
  );
}

inline void quaternion_normalise(Quaternion& quaternion)
{
  quaternion = quaternion_normalised(quaternion);
}

/// \brief Constructs a pure-rotation matrix from \p quaternion.
inline Matrix4 matrix4_rotation_for_quaternion(const Quaternion& quaternion)
{
#if 0
  const double xx = quaternion[0] * quaternion[0];
  const double xy = quaternion[0] * quaternion[1];
  const double xz = quaternion[0] * quaternion[2];
  const double xw = quaternion[0] * quaternion[3];

  const double yy = quaternion[1] * quaternion[1];
  const double yz = quaternion[1] * quaternion[2];
  const double yw = quaternion[1] * quaternion[3];

  const double zz = quaternion[2] * quaternion[2];
  const double zw = quaternion[2] * quaternion[3];

  return Matrix4(
    static_cast<float>( 1 - 2 * ( yy + zz ) ),
    static_cast<float>(     2 * ( xy + zw ) ),
    static_cast<float>(     2 * ( xz - yw ) ),
    0,
    static_cast<float>(     2 * ( xy - zw ) ),
    static_cast<float>( 1 - 2 * ( xx + zz ) ),
    static_cast<float>(     2 * ( yz + xw ) ),
    0,
    static_cast<float>(     2 * ( xz + yw ) ),
    static_cast<float>(     2 * ( yz - xw ) ),
    static_cast<float>( 1 - 2 * ( xx + yy ) ),
    0,
    0,
    0,
    0,
    1
  );

#else
  const double x2 = quaternion[0] + quaternion[0];
  const double y2 = quaternion[1] + quaternion[1]; 
  const double z2 = quaternion[2] + quaternion[2];
  const double xx = quaternion[0] * x2;
  const double xy = quaternion[0] * y2;
  const double xz = quaternion[0] * z2;
  const double yy = quaternion[1] * y2;
  const double yz = quaternion[1] * z2;
  const double zz = quaternion[2] * z2;
  const double wx = quaternion[3] * x2;
  const double wy = quaternion[3] * y2;
  const double wz = quaternion[3] * z2;

  return Matrix4(
    static_cast<float>( 1.0 - (yy + zz) ),
    static_cast<float>(xy + wz),
    static_cast<float>(xz - wy),
    0,
    static_cast<float>(xy - wz),
    static_cast<float>( 1.0 - (xx + zz) ),
    static_cast<float>(yz + wx),
    0,
    static_cast<float>(xz + wy),
    static_cast<float>(yz - wx),
    static_cast<float>( 1.0 - (xx + yy) ),
    0,
    0,
    0,
    0,
    1
  );

#endif
}

const double c_half_sqrt2 = 0.70710678118654752440084436210485;
const float c_half_sqrt2f = static_cast<float>(c_half_sqrt2);

inline bool quaternion_component_is_90(float component)
{
  return (fabs(component) - c_half_sqrt2) < 0.001;
}

inline Matrix4 matrix4_rotation_for_quaternion_quantised(const Quaternion& quaternion)
{
  if(quaternion.y() == 0
    && quaternion.z() == 0
    && quaternion_component_is_90(quaternion.x())
    && quaternion_component_is_90(quaternion.w()))
  {
    return matrix4_rotation_for_sincos_x((quaternion.x() > 0) ? 1.f : -1.f, 0);
  }

  if(quaternion.x() == 0
    && quaternion.z() == 0
    && quaternion_component_is_90(quaternion.y())
    && quaternion_component_is_90(quaternion.w()))
  {
    return matrix4_rotation_for_sincos_y((quaternion.y() > 0) ? 1.f : -1.f, 0);
  }

  if(quaternion.x() == 0
    && quaternion.y() == 0
    && quaternion_component_is_90(quaternion.z())
    && quaternion_component_is_90(quaternion.w()))
  {
    return matrix4_rotation_for_sincos_z((quaternion.z() > 0) ? 1.f : -1.f, 0);
  }

  return matrix4_rotation_for_quaternion(quaternion);
}

inline Quaternion quaternion_for_matrix4_rotation(const Matrix4& matrix4)
{
  Matrix4 transposed = matrix4_transposed(matrix4);

  double trace = transposed[0] + transposed[5] + transposed[10] + 1.0;

  if(trace > 0.0001)
  {
    double S = 0.5 / sqrt(trace);
    return Quaternion(
      static_cast<float>((transposed[9] - transposed[6]) * S),
      static_cast<float>((transposed[2] - transposed[8]) * S),
      static_cast<float>((transposed[4] - transposed[1]) * S),
      static_cast<float>(0.25 / S)
    );
  }

  if(transposed[0] >= transposed[5] && transposed[0] >= transposed[10])
  {
    double S = 2.0 * sqrt(1.0 + transposed[0] - transposed[5] - transposed[10]);
    return Quaternion(
      static_cast<float>(0.25 / S),
      static_cast<float>((transposed[1] + transposed[4]) / S),
      static_cast<float>((transposed[2] + transposed[8]) / S),
      static_cast<float>((transposed[6] + transposed[9]) / S)
    );
  }
  
  if(transposed[5] >= transposed[0] && transposed[5] >= transposed[10])
  {
    double S = 2.0 * sqrt(1.0 + transposed[5] - transposed[0] - transposed[10]);
    return Quaternion(
      static_cast<float>((transposed[1] + transposed[4]) / S),
      static_cast<float>(0.25 / S),
      static_cast<float>((transposed[6] + transposed[9]) / S),
      static_cast<float>((transposed[2] + transposed[8]) / S)
    );
  }

  double S = 2.0 * sqrt(1.0 + transposed[10] - transposed[0] - transposed[5]);
  return Quaternion(
    static_cast<float>((transposed[2] + transposed[8]) / S),
    static_cast<float>((transposed[6] + transposed[9]) / S),
    static_cast<float>(0.25 / S),
    static_cast<float>((transposed[1] + transposed[4]) / S)
  );
}

/// \brief Returns \p self concatenated with the rotation transform produced by \p rotation.
/// The concatenated rotation occurs before \p self.
inline Matrix4 matrix4_rotated_by_quaternion(const Matrix4& self, const Quaternion& rotation)
{
  return matrix4_multiplied_by_matrix4(self, matrix4_rotation_for_quaternion(rotation));
}

/// \brief Concatenates \p self with the rotation transform produced by \p rotation.
/// The concatenated rotation occurs before \p self.
inline void matrix4_rotate_by_quaternion(Matrix4& self, const Quaternion& rotation)
{
  self = matrix4_rotated_by_quaternion(self, rotation);
}

/// \brief Rotates \p self by \p rotation, using \p pivotpoint.
inline void matrix4_pivoted_rotate_by_quaternion(Matrix4& self, const Quaternion& rotation, const Vector3& pivotpoint)
{
  matrix4_translate_by_vec3(self, pivotpoint);
  matrix4_rotate_by_quaternion(self, rotation);
  matrix4_translate_by_vec3(self, vector3_negated(pivotpoint));
}

inline Vector3 quaternion_transformed_point(const Quaternion& quaternion, const Vector3& point)
{
  double xx = quaternion.x() * quaternion.x();
  double yy = quaternion.y() * quaternion.y();
  double zz = quaternion.z() * quaternion.z();
  double ww = quaternion.w() * quaternion.w();

  double xy2 = quaternion.x() * quaternion.y() * 2;
  double xz2 = quaternion.x() * quaternion.z() * 2;
  double xw2 = quaternion.x() * quaternion.w() * 2;
  double yz2 = quaternion.y() * quaternion.z() * 2;
  double yw2 = quaternion.y() * quaternion.w() * 2;
  double zw2 = quaternion.z() * quaternion.w() * 2;

	return Vector3(
    static_cast<float>(ww * point.x() + yw2 * point.z() - zw2 * point.y() + xx * point.x() + xy2 * point.y() + xz2 * point.z() - zz * point.x() - yy * point.x()),
    static_cast<float>(xy2 * point.x() + yy * point.y() + yz2 * point.z() + zw2 * point.x() - zz * point.y() + ww * point.y() - xw2 * point.z() - xx * point.y()),
    static_cast<float>(xz2 * point.x() + yz2 * point.y() + zz * point.z() - yw2 * point.x() - yy * point.z() + xw2 * point.y() - xx * point.z() + ww * point.z())
  );
}

/// \brief Constructs a pure-rotation transform from \p axis and \p angle (radians).
inline Matrix4 matrix4_rotation_for_axisangle(const Vector3& axis, double angle)
{
  return matrix4_rotation_for_quaternion(quaternion_for_axisangle(axis, angle));
}

/// \brief Rotates \p self about \p axis by \p angle.
inline void matrix4_rotate_by_axisangle(Matrix4& self, const Vector3& axis, double angle)
{
  matrix4_multiply_by_matrix4(self, matrix4_rotation_for_axisangle(axis, angle));
}

/// \brief Rotates \p self about \p axis by \p angle using \p pivotpoint.
inline void matrix4_pivoted_rotate_by_axisangle(Matrix4& self, const Vector3& axis, double angle, const Vector3& pivotpoint)
{
  matrix4_translate_by_vec3(self, pivotpoint);
  matrix4_rotate_by_axisangle(self, axis, angle);
  matrix4_translate_by_vec3(self, vector3_negated(pivotpoint));
}


#endif
