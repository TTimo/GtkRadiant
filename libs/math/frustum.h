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

#if !defined(INCLUDED_MATH_FRUSTUM_H)
#define INCLUDED_MATH_FRUSTUM_H

/// \file
/// \brief View-frustum data types and related operations.

#include "generic/enumeration.h"
#include "math/matrix.h"
#include "math/plane.h"
#include "math/aabb.h"
#include "math/line.h"

inline Matrix4 matrix4_frustum(float left, float right, float bottom, float top, float nearval, float farval)
{
  return Matrix4(
    static_cast<float>( (2*nearval) / (right-left) ),
    0,
    0,
    0,
    0,
    static_cast<float>( (2*nearval) / (top-bottom) ),
    0,
    0,
    static_cast<float>( (right+left) / (right-left) ),
    static_cast<float>( (top+bottom) / (top-bottom) ),
    static_cast<float>( -(farval+nearval) / (farval-nearval) ),
    -1,
    0,
    0,
    static_cast<float>( -(2*farval*nearval) / (farval-nearval) ),
    0
  );
}



typedef unsigned char ClipResult;
const ClipResult c_CLIP_PASS = 0x00; // 000000
const ClipResult c_CLIP_LT_X = 0x01; // 000001
const ClipResult c_CLIP_GT_X = 0x02; // 000010
const ClipResult c_CLIP_LT_Y = 0x04; // 000100
const ClipResult c_CLIP_GT_Y = 0x08; // 001000
const ClipResult c_CLIP_LT_Z = 0x10; // 010000
const ClipResult c_CLIP_GT_Z = 0x20; // 100000
const ClipResult c_CLIP_FAIL = 0x3F; // 111111

template<typename Index>
class Vector4ClipLT
{
public:
  static bool compare(const Vector4& self)
  {
    return self[Index::VALUE] < self[3];
  }
  static double scale(const Vector4& self, const Vector4& other)
  {
    return (self[Index::VALUE] - self[3]) / (other[3] - other[Index::VALUE]);
  }
};

template<typename Index>
class Vector4ClipGT
{
public:
  static bool compare(const Vector4& self)
  {
    return self[Index::VALUE] > -self[3];
  }
  static double scale(const Vector4& self, const Vector4& other)
  {
    return (self[Index::VALUE] + self[3]) / (-other[3] - other[Index::VALUE]);
  }
};

template<typename ClipPlane>
class Vector4ClipPolygon
{
public:
  typedef Vector4* iterator;
  typedef const Vector4* const_iterator;

  static std::size_t apply(const_iterator first, const_iterator last, iterator out)
  {
    const_iterator next = first, i = last - 1;
    iterator tmp(out);
    bool b0 = ClipPlane::compare(*i);
    while(next != last)
    {
      bool b1 = ClipPlane::compare(*next);
      if(b0 ^ b1)
      {
        *out = vector4_subtracted(*next, *i);

        double scale = ClipPlane::scale(*i, *out);

        (*out)[0] = static_cast<float>((*i)[0] + scale*((*out)[0]));
        (*out)[1] = static_cast<float>((*i)[1] + scale*((*out)[1]));
        (*out)[2] = static_cast<float>((*i)[2] + scale*((*out)[2]));
        (*out)[3] = static_cast<float>((*i)[3] + scale*((*out)[3]));

        ++out;
      }

      if(b1)
      {
        *out = *next;
        ++out;
      }

      i = next;
      ++next;
      b0 = b1;
    }

    return out - tmp;
  }
};

#define CLIP_X_LT_W(p) (Vector4ClipLT< IntegralConstant<0> >::compare(p))
#define CLIP_X_GT_W(p) (Vector4ClipGT< IntegralConstant<0> >::compare(p))
#define CLIP_Y_LT_W(p) (Vector4ClipLT< IntegralConstant<1> >::compare(p))
#define CLIP_Y_GT_W(p) (Vector4ClipGT< IntegralConstant<1> >::compare(p))
#define CLIP_Z_LT_W(p) (Vector4ClipLT< IntegralConstant<2> >::compare(p))
#define CLIP_Z_GT_W(p) (Vector4ClipGT< IntegralConstant<2> >::compare(p))

inline ClipResult homogenous_clip_point(const Vector4& clipped)
{
  ClipResult result = c_CLIP_FAIL;
  if(CLIP_X_LT_W(clipped)) result &= ~c_CLIP_LT_X; // X < W
  if(CLIP_X_GT_W(clipped)) result &= ~c_CLIP_GT_X; // X > -W
  if(CLIP_Y_LT_W(clipped)) result &= ~c_CLIP_LT_Y; // Y < W
  if(CLIP_Y_GT_W(clipped)) result &= ~c_CLIP_GT_Y; // Y > -W
  if(CLIP_Z_LT_W(clipped)) result &= ~c_CLIP_LT_Z; // Z < W
  if(CLIP_Z_GT_W(clipped)) result &= ~c_CLIP_GT_Z; // Z > -W
  return result;
}

/// \brief Clips \p point by canonical matrix \p self.
/// Stores the result in \p clipped.
/// Returns a bitmask indicating which clip-planes the point was outside.
inline ClipResult matrix4_clip_point(const Matrix4& self, const Vector3& point, Vector4& clipped)
{
  clipped[0] = point[0];
  clipped[1] = point[1];
  clipped[2] = point[2];
  clipped[3] = 1;
  matrix4_transform_vector4(self, clipped);
  return homogenous_clip_point(clipped);
}


inline std::size_t homogenous_clip_triangle(Vector4 clipped[9])
{
  Vector4 buffer[9];
  std::size_t count = 3;
  count = Vector4ClipPolygon< Vector4ClipLT< IntegralConstant<0> > >::apply(clipped, clipped + count, buffer);
  count = Vector4ClipPolygon< Vector4ClipGT< IntegralConstant<0> > >::apply(buffer, buffer + count, clipped);
  count = Vector4ClipPolygon< Vector4ClipLT< IntegralConstant<1> > >::apply(clipped, clipped + count, buffer);
  count = Vector4ClipPolygon< Vector4ClipGT< IntegralConstant<1> > >::apply(buffer, buffer + count, clipped);
  count = Vector4ClipPolygon< Vector4ClipLT< IntegralConstant<2> > >::apply(clipped, clipped + count, buffer);
  return Vector4ClipPolygon< Vector4ClipGT< IntegralConstant<2> > >::apply(buffer, buffer + count, clipped);
}

/// \brief Transforms and clips the triangle formed by \p p0, \p p1, \p p2 by the canonical matrix \p self.
/// Stores the resulting polygon in \p clipped.
/// Returns the number of points in the resulting polygon.
inline std::size_t matrix4_clip_triangle(const Matrix4& self, const Vector3& p0, const Vector3& p1, const Vector3& p2, Vector4 clipped[9])
{
  clipped[0][0] = p0[0];
  clipped[0][1] = p0[1];
  clipped[0][2] = p0[2];
  clipped[0][3] = 1;
  clipped[1][0] = p1[0];
  clipped[1][1] = p1[1];
  clipped[1][2] = p1[2];
  clipped[1][3] = 1;
  clipped[2][0] = p2[0];
  clipped[2][1] = p2[1];
  clipped[2][2] = p2[2];
  clipped[2][3] = 1;

  matrix4_transform_vector4(self, clipped[0]);
  matrix4_transform_vector4(self, clipped[1]);
  matrix4_transform_vector4(self, clipped[2]);

  return homogenous_clip_triangle(clipped);
}

inline std::size_t homogenous_clip_line(Vector4 clipped[2])
{
  const Vector4& p0 = clipped[0];
  const Vector4& p1 = clipped[1];

  // early out
  {
    ClipResult mask0 = homogenous_clip_point(clipped[0]);
    ClipResult mask1 = homogenous_clip_point(clipped[1]);

    if((mask0 | mask1) == c_CLIP_PASS) // both points passed all planes
      return 2;

    if(mask0 & mask1) // both points failed any one plane
      return 0;
  }

  {
    const bool index = CLIP_X_LT_W(p0);
    if(index ^ CLIP_X_LT_W(p1))
    {
      Vector4 clip(vector4_subtracted(p1, p0));

      double scale = (p0[0] - p0[3]) / (clip[3] - clip[0]);

      clip[0] = static_cast<float>(p0[0] + scale * clip[0]);
      clip[1] = static_cast<float>(p0[1] + scale * clip[1]);
      clip[2] = static_cast<float>(p0[2] + scale * clip[2]);
      clip[3] = static_cast<float>(p0[3] + scale * clip[3]);

      clipped[index] = clip;
    }
    else if(index == 0)
      return 0;
  }

  {
    const bool index = CLIP_X_GT_W(p0);
    if(index ^ CLIP_X_GT_W(p1))
    {
      Vector4 clip(vector4_subtracted(p1, p0));

      double scale = (p0[0] + p0[3]) / (-clip[3] - clip[0]);

      clip[0] = static_cast<float>(p0[0] + scale * clip[0]);
      clip[1] = static_cast<float>(p0[1] + scale * clip[1]);
      clip[2] = static_cast<float>(p0[2] + scale * clip[2]);
      clip[3] = static_cast<float>(p0[3] + scale * clip[3]);

      clipped[index] = clip;
    }
    else if(index == 0)
      return 0;
  }

  {
    const bool index = CLIP_Y_LT_W(p0);
    if(index ^ CLIP_Y_LT_W(p1))
    {
      Vector4 clip(vector4_subtracted(p1, p0));

      double scale = (p0[1] - p0[3]) / (clip[3] - clip[1]);

      clip[0] = static_cast<float>(p0[0] + scale * clip[0]);
      clip[1] = static_cast<float>(p0[1] + scale * clip[1]);
      clip[2] = static_cast<float>(p0[2] + scale * clip[2]);
      clip[3] = static_cast<float>(p0[3] + scale * clip[3]);

      clipped[index] = clip;
    }
    else if(index == 0)
      return 0;
  }

  {
    const bool index = CLIP_Y_GT_W(p0);
    if(index ^ CLIP_Y_GT_W(p1))
    {
      Vector4 clip(vector4_subtracted(p1, p0));

      double scale = (p0[1] + p0[3]) / (-clip[3] - clip[1]);

      clip[0] = static_cast<float>(p0[0] + scale * clip[0]);
      clip[1] = static_cast<float>(p0[1] + scale * clip[1]);
      clip[2] = static_cast<float>(p0[2] + scale * clip[2]);
      clip[3] = static_cast<float>(p0[3] + scale * clip[3]);

      clipped[index] = clip;
    }
    else if(index == 0)
      return 0;
  }

  {
    const bool index = CLIP_Z_LT_W(p0);
    if(index ^ CLIP_Z_LT_W(p1))
    {
      Vector4 clip(vector4_subtracted(p1, p0));

      double scale = (p0[2] - p0[3]) / (clip[3] - clip[2]);

      clip[0] = static_cast<float>(p0[0] + scale * clip[0]);
      clip[1] = static_cast<float>(p0[1] + scale * clip[1]);
      clip[2] = static_cast<float>(p0[2] + scale * clip[2]);
      clip[3] = static_cast<float>(p0[3] + scale * clip[3]);

      clipped[index] = clip;
    }
    else if(index == 0)
      return 0;
  }

  {
    const bool index = CLIP_Z_GT_W(p0);
    if(index ^ CLIP_Z_GT_W(p1))
    {
      Vector4 clip(vector4_subtracted(p1, p0));

      double scale = (p0[2] + p0[3]) / (-clip[3] - clip[2]);

      clip[0] = static_cast<float>(p0[0] + scale * clip[0]);
      clip[1] = static_cast<float>(p0[1] + scale * clip[1]);
      clip[2] = static_cast<float>(p0[2] + scale * clip[2]);
      clip[3] = static_cast<float>(p0[3] + scale * clip[3]);

      clipped[index] = clip;
    }
    else if(index == 0)
      return 0;
  }

  return 2;
}

/// \brief Transforms and clips the line formed by \p p0, \p p1 by the canonical matrix \p self.
/// Stores the resulting line in \p clipped.
/// Returns the number of points in the resulting line.
inline std::size_t matrix4_clip_line(const Matrix4& self, const Vector3& p0, const Vector3& p1, Vector4 clipped[2])
{
  clipped[0][0] = p0[0];
  clipped[0][1] = p0[1];
  clipped[0][2] = p0[2];
  clipped[0][3] = 1;
  clipped[1][0] = p1[0];
  clipped[1][1] = p1[1];
  clipped[1][2] = p1[2];
  clipped[1][3] = 1;

  matrix4_transform_vector4(self, clipped[0]);
  matrix4_transform_vector4(self, clipped[1]);

  return homogenous_clip_line(clipped);
}




struct Frustum
{
  Plane3 right, left, bottom, top, back, front;

  Frustum()
  {
  }
  Frustum(const Plane3& _right,
    const Plane3& _left,
    const Plane3& _bottom,
    const Plane3& _top,
    const Plane3& _back,
    const Plane3& _front)
    : right(_right), left(_left), bottom(_bottom), top(_top), back(_back), front(_front)
  {
  }
};

inline Frustum frustum_transformed(const Frustum& frustum, const Matrix4& transform)
{
  return Frustum(
    plane3_transformed(frustum.right, transform),
    plane3_transformed(frustum.left, transform),
    plane3_transformed(frustum.bottom, transform),
    plane3_transformed(frustum.top, transform),
    plane3_transformed(frustum.back, transform),
    plane3_transformed(frustum.front, transform)
  );
}

inline Frustum frustum_inverse_transformed(const Frustum& frustum, const Matrix4& transform)
{
  return Frustum(
    plane3_inverse_transformed(frustum.right, transform),
    plane3_inverse_transformed(frustum.left, transform),
    plane3_inverse_transformed(frustum.bottom, transform),
    plane3_inverse_transformed(frustum.top, transform),
    plane3_inverse_transformed(frustum.back, transform),
    plane3_inverse_transformed(frustum.front, transform)
  );
}

inline bool viewproj_test_point(const Matrix4& viewproj, const Vector3& point)
{
  Vector4 hpoint(matrix4_transformed_vector4(viewproj, Vector4(point, 1.0f)));
  if(fabs(hpoint[0]) < fabs(hpoint[3])
    && fabs(hpoint[1]) < fabs(hpoint[3])
    && fabs(hpoint[2]) < fabs(hpoint[3]))
    return true;
  return false;
}

inline bool viewproj_test_transformed_point(const Matrix4& viewproj, const Vector3& point, const Matrix4& localToWorld)
{
  return viewproj_test_point(viewproj, matrix4_transformed_point(localToWorld, point));
}

inline Frustum frustum_from_viewproj(const Matrix4& viewproj)
{
  return Frustum
  (
    plane3_normalised(Plane3(viewproj[ 3] - viewproj[ 0], viewproj[ 7] - viewproj[ 4], viewproj[11] - viewproj[ 8], viewproj[15] - viewproj[12])),
    plane3_normalised(Plane3(viewproj[ 3] + viewproj[ 0], viewproj[ 7] + viewproj[ 4], viewproj[11] + viewproj[ 8], viewproj[15] + viewproj[12])),
    plane3_normalised(Plane3(viewproj[ 3] + viewproj[ 1], viewproj[ 7] + viewproj[ 5], viewproj[11] + viewproj[ 9], viewproj[15] + viewproj[13])),
    plane3_normalised(Plane3(viewproj[ 3] - viewproj[ 1], viewproj[ 7] - viewproj[ 5], viewproj[11] - viewproj[ 9], viewproj[15] - viewproj[13])),
    plane3_normalised(Plane3(viewproj[ 3] - viewproj[ 2], viewproj[ 7] - viewproj[ 6], viewproj[11] - viewproj[10], viewproj[15] - viewproj[14])),
    plane3_normalised(Plane3(viewproj[ 3] + viewproj[ 2], viewproj[ 7] + viewproj[ 6], viewproj[11] + viewproj[10], viewproj[15] + viewproj[14]))
  );
}

struct VolumeIntersection
{
  enum Value
  {
    OUTSIDE,
    INSIDE,
    PARTIAL
  };
};

typedef EnumeratedValue<VolumeIntersection> VolumeIntersectionValue;

const VolumeIntersectionValue c_volumeOutside(VolumeIntersectionValue::OUTSIDE);
const VolumeIntersectionValue c_volumeInside(VolumeIntersectionValue::INSIDE);
const VolumeIntersectionValue c_volumePartial(VolumeIntersectionValue::PARTIAL);

inline VolumeIntersectionValue frustum_test_aabb(const Frustum& frustum, const AABB& aabb)
{
  VolumeIntersectionValue result = c_volumeInside;

  switch(aabb_classify_plane(aabb, frustum.right))
  {
  case 2:
    return c_volumeOutside;
  case 1:
    result = c_volumePartial;
  }

  switch(aabb_classify_plane(aabb, frustum.left))
  {
  case 2:
    return c_volumeOutside;
  case 1:
    result = c_volumePartial;
  }

  switch(aabb_classify_plane(aabb, frustum.bottom))
  {
  case 2:
    return c_volumeOutside;
  case 1:
    result = c_volumePartial;
  }

  switch(aabb_classify_plane(aabb, frustum.top))
  {
  case 2:
    return c_volumeOutside;
  case 1:
    result = c_volumePartial;
  }

  switch(aabb_classify_plane(aabb, frustum.back))
  {
  case 2:
    return c_volumeOutside;
  case 1:
    result = c_volumePartial;
  }

  switch(aabb_classify_plane(aabb, frustum.front))
  {
  case 2:
    return c_volumeOutside;
  case 1:
    result = c_volumePartial;
  }

  return result;
}

inline double plane_distance_to_point(const Plane3& plane, const Vector3& point)
{
  return vector3_dot(plane.normal(), point) + plane.d;
}

inline double plane_distance_to_oriented_extents(const Plane3& plane, const Vector3& extents, const Matrix4& orientation)
{
  return fabs(extents[0] * vector3_dot(plane.normal(), vector4_to_vector3(orientation.x())))
    + fabs(extents[1] * vector3_dot(plane.normal(), vector4_to_vector3(orientation.y())))
    + fabs(extents[2] * vector3_dot(plane.normal(), vector4_to_vector3(orientation.z())));
}

/// \brief Return false if \p aabb with \p orientation is partially or completely outside \p plane.
inline bool plane_contains_oriented_aabb(const Plane3& plane, const AABB& aabb, const Matrix4& orientation)
{
  double dot = plane_distance_to_point(plane, aabb.origin);
  return !(dot > 0 || -dot < plane_distance_to_oriented_extents(plane, aabb.extents, orientation));
}

inline VolumeIntersectionValue frustum_intersects_transformed_aabb(const Frustum& frustum, const AABB& aabb, const Matrix4& localToWorld)
{
  AABB aabb_world(aabb);
  matrix4_transform_point(localToWorld, aabb_world.origin);

  if(plane_contains_oriented_aabb(frustum.right, aabb_world, localToWorld)
    || plane_contains_oriented_aabb(frustum.left, aabb_world, localToWorld)
    || plane_contains_oriented_aabb(frustum.bottom, aabb_world, localToWorld)
    || plane_contains_oriented_aabb(frustum.top, aabb_world, localToWorld)
    || plane_contains_oriented_aabb(frustum.back, aabb_world, localToWorld)
    || plane_contains_oriented_aabb(frustum.front, aabb_world, localToWorld))
    return c_volumeOutside;
  return c_volumeInside;
}

inline bool plane3_test_point(const Plane3& plane, const Vector3& point)
{
  return vector3_dot(point, plane.normal()) + plane.dist() <= 0;
}

inline bool plane3_test_line(const Plane3& plane, const Segment& segment)
{
  return segment_classify_plane(segment, plane) == 2;
}

inline bool frustum_test_point(const Frustum& frustum, const Vector3& point)
{
  return !plane3_test_point(frustum.right, point)
    && !plane3_test_point(frustum.left, point)
    && !plane3_test_point(frustum.bottom, point)
    && !plane3_test_point(frustum.top, point)
    && !plane3_test_point(frustum.back, point)
    && !plane3_test_point(frustum.front, point);
}

inline bool frustum_test_line(const Frustum& frustum, const Segment& segment)
{
  return !plane3_test_line(frustum.right, segment)
    && !plane3_test_line(frustum.left, segment)
    && !plane3_test_line(frustum.bottom, segment)
    && !plane3_test_line(frustum.top, segment)
    && !plane3_test_line(frustum.back, segment)
    && !plane3_test_line(frustum.front, segment);
}

inline bool viewer_test_plane(const Vector4& viewer, const Plane3& plane)
{
  return ((plane.a * viewer[0])
    + (plane.b * viewer[1])
    + (plane.c * viewer[2])
    + (plane.d * viewer[3])) > 0;
}

inline Vector3 triangle_cross(const Vector3& p0, const Vector3& p1, const Vector3& p2)
{
  return vector3_cross(vector3_subtracted(p1, p0), vector3_subtracted(p1, p2));
}

inline bool viewer_test_triangle(const Vector4& viewer, const Vector3& p0, const Vector3& p1, const Vector3& p2)
{
  Vector3 cross(triangle_cross(p0, p1, p2));
  return ((viewer[0] * cross[0])
    + (viewer[1] * cross[1])
    + (viewer[2] * cross[2])
    + (viewer[3] * 0)) > 0;
}

inline Vector4 viewer_from_transformed_viewer(const Vector4& viewer, const Matrix4& transform)
{
  if(viewer[3] == 0)
  {
    return Vector4(matrix4_transformed_direction(transform, vector4_to_vector3(viewer)), 0);
  }
  else
  {
    return Vector4(matrix4_transformed_point(transform, vector4_to_vector3(viewer)), viewer[3]);
  }
}

inline bool viewer_test_transformed_plane(const Vector4& viewer, const Plane3& plane, const Matrix4& localToWorld)
{
#if 0
  return viewer_test_plane(viewer_from_transformed_viewer(viewer, matrix4_affine_inverse(localToWorld)), plane);
#else
  return viewer_test_plane(viewer, plane3_transformed(plane, localToWorld));
#endif
}

inline Vector4 viewer_from_viewproj(const Matrix4& viewproj)
{
  // get viewer pos in object coords
  Vector4 viewer(matrix4_transformed_vector4(matrix4_full_inverse(viewproj), Vector4(0, 0, -1, 0)));
  if(viewer[3] != 0) // non-affine matrix
  {
    viewer[0] /= viewer[3];
    viewer[1] /= viewer[3];
    viewer[2] /= viewer[3];
    viewer[3] /= viewer[3];
  }
  return viewer;
}

#endif
