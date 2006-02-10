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

#if !defined(INCLUDED_MATH_AABB_H)
#define INCLUDED_MATH_AABB_H

/// \file
/// \brief Axis-aligned bounding-box data types and related operations.

#include "math/matrix.h"
#include "math/plane.h"

class AABB
{
public:
  Vector3 origin, extents;

  AABB() : origin(0, 0, 0), extents(-1,-1,-1)
  {
  }
  AABB(const Vector3& origin_, const Vector3& extents_) :
    origin(origin_), extents(extents_)
  {
  }
};

const float c_aabb_max = FLT_MAX;

inline bool extents_valid(float f)
{
  return f >= 0.0f && f <= c_aabb_max;
}

inline bool origin_valid(float f)
{
  return f >= -c_aabb_max && f <= c_aabb_max;
}

inline bool aabb_valid(const AABB& aabb)
{
  return origin_valid(aabb.origin[0])
    && origin_valid(aabb.origin[1])
    && origin_valid(aabb.origin[2])
    && extents_valid(aabb.extents[0])
    && extents_valid(aabb.extents[1])
    && extents_valid(aabb.extents[2]);
}

inline AABB aabb_for_minmax(const Vector3& min, const Vector3& max)
{
  AABB aabb;
  aabb.origin = vector3_mid(min, max);
  aabb.extents = vector3_subtracted(max, aabb.origin);
  return aabb;
}

template<typename Index>
class AABBExtend
{
public:
  static void apply(AABB& aabb, const Vector3& point)
  {
    float displacement = point[Index::VALUE] - aabb.origin[Index::VALUE];
    float half_difference = static_cast<float>(0.5 * (fabs(displacement) - aabb.extents[Index::VALUE]));
    if(half_difference > 0.0f)
    {
      aabb.origin[Index::VALUE] += (displacement >= 0.0f) ? half_difference : -half_difference;
      aabb.extents[Index::VALUE] += half_difference;
    }
  }
  static void apply(AABB& aabb, const AABB& other)
  {
    float displacement = other.origin[Index::VALUE] - aabb.origin[Index::VALUE];
    float difference = other.extents[Index::VALUE] - aabb.extents[Index::VALUE];
    if(fabs(displacement) > fabs(difference))
    {
      float half_difference = static_cast<float>(0.5 * (fabs(displacement) + difference));
      if(half_difference > 0.0f)
      {
        aabb.origin[Index::VALUE] += (displacement >= 0.0f) ? half_difference : -half_difference;
        aabb.extents[Index::VALUE] += half_difference;
      }
    }
    else if(difference > 0.0f)
    {
      aabb.origin[Index::VALUE] = other.origin[Index::VALUE];
      aabb.extents[Index::VALUE] = other.extents[Index::VALUE];
    }
  }
};

inline void aabb_extend_by_point(AABB& aabb, const Vector3& point)
{
  AABBExtend< IntegralConstant<0> >::apply(aabb, point);
  AABBExtend< IntegralConstant<1> >::apply(aabb, point);
  AABBExtend< IntegralConstant<2> >::apply(aabb, point);
}

inline void aabb_extend_by_point_safe(AABB& aabb, const Vector3& point)
{
  if(aabb_valid(aabb))
  {
    aabb_extend_by_point(aabb, point);
  }
  else
  {
    aabb.origin = point;
    aabb.extents = Vector3(0, 0, 0);
  }
}

class AABBExtendByPoint
{
  AABB& m_aabb;
public:
  AABBExtendByPoint(AABB& aabb) : m_aabb(aabb)
  {
  }
  void operator()(const Vector3& point) const
  {
    aabb_extend_by_point_safe(m_aabb, point);
  }
};
  
inline void aabb_extend_by_aabb(AABB& aabb, const AABB& other)
{
  AABBExtend< IntegralConstant<0> >::apply(aabb, other);
  AABBExtend< IntegralConstant<1> >::apply(aabb, other);
  AABBExtend< IntegralConstant<2> >::apply(aabb, other);
}

inline void aabb_extend_by_aabb_safe(AABB& aabb, const AABB& other)
{
  if(aabb_valid(aabb) && aabb_valid(other))
  {
    aabb_extend_by_aabb(aabb, other);
  }
  else if(aabb_valid(other))
  {
    aabb = other;
  }
}

inline void aabb_extend_by_vec3(AABB& aabb, const Vector3& extension)
{
  vector3_add(aabb.extents, extension);
}




template<typename Index>
inline bool aabb_intersects_point_dimension(const AABB& aabb, const Vector3& point)
{
  return fabs(point[Index::VALUE] - aabb.origin[Index::VALUE]) < aabb.extents[Index::VALUE];
}

inline bool aabb_intersects_point(const AABB& aabb, const Vector3& point)
{
  return aabb_intersects_point_dimension< IntegralConstant<0> >(aabb, point)
    && aabb_intersects_point_dimension< IntegralConstant<1> >(aabb, point)
    && aabb_intersects_point_dimension< IntegralConstant<2> >(aabb, point);
}

template<typename Index>
inline bool aabb_intersects_aabb_dimension(const AABB& aabb, const AABB& other)
{
  return fabs(other.origin[Index::VALUE] - aabb.origin[Index::VALUE]) < (aabb.extents[Index::VALUE] + other.extents[Index::VALUE]);
}

inline bool aabb_intersects_aabb(const AABB& aabb, const AABB& other)
{
  return aabb_intersects_aabb_dimension< IntegralConstant<0> >(aabb, other)
    && aabb_intersects_aabb_dimension< IntegralConstant<1> >(aabb, other)
    && aabb_intersects_aabb_dimension< IntegralConstant<2> >(aabb, other);
}

inline unsigned int aabb_classify_plane(const AABB& aabb, const Plane3& plane)
{
  double distance_origin = vector3_dot(plane.normal(), aabb.origin) + plane.dist();
  
  if(fabs(distance_origin) < (fabs(plane.a * aabb.extents[0])
    + fabs(plane.b * aabb.extents[1])
    + fabs(plane.c * aabb.extents[2])))
  {
    return 1; // partially inside
  }
  else if (distance_origin < 0)
  {
    return 2; // totally inside
  }
  return 0; // totally outside
}

inline unsigned int aabb_oriented_classify_plane(const AABB& aabb, const Matrix4& transform, const Plane3& plane)
{
  double distance_origin = vector3_dot(plane.normal(), aabb.origin) + plane.dist();

  if(fabs(distance_origin) < (fabs(aabb.extents[0] * vector3_dot(plane.normal(), vector4_to_vector3(transform.x())))
    + fabs(aabb.extents[1] * vector3_dot(plane.normal(), vector4_to_vector3(transform.y())))
    + fabs(aabb.extents[2] * vector3_dot(plane.normal(), vector4_to_vector3(transform.z())))))
  {
    return 1; // partially inside
  }
  else if (distance_origin < 0)
  {
    return 2; // totally inside
  }
  return 0; // totally outside
}

inline void aabb_corners(const AABB& aabb, Vector3 corners[8])
{
  Vector3 min(vector3_subtracted(aabb.origin, aabb.extents));
  Vector3 max(vector3_added(aabb.origin, aabb.extents));
  corners[0] = Vector3(min[0], max[1], max[2]);
  corners[1] = Vector3(max[0], max[1], max[2]);
  corners[2] = Vector3(max[0], min[1], max[2]);
  corners[3] = Vector3(min[0], min[1], max[2]);
  corners[4] = Vector3(min[0], max[1], min[2]);
  corners[5] = Vector3(max[0], max[1], min[2]);
  corners[6] = Vector3(max[0], min[1], min[2]);
  corners[7] = Vector3(min[0], min[1], min[2]);
}

inline void aabb_planes(const AABB& aabb, Plane3 planes[6])
{
  planes[0] = Plane3(g_vector3_axes[0], aabb.origin[0] + aabb.extents[0]);
  planes[1] = Plane3(vector3_negated(g_vector3_axes[0]), -(aabb.origin[0] - aabb.extents[0]));
  planes[2] = Plane3(g_vector3_axes[1], aabb.origin[1] + aabb.extents[1]);
  planes[3] = Plane3(vector3_negated(g_vector3_axes[1]), -(aabb.origin[1] - aabb.extents[1]));
  planes[4] = Plane3(g_vector3_axes[2], aabb.origin[2] + aabb.extents[2]);
  planes[5] = Plane3(vector3_negated(g_vector3_axes[2]), -(aabb.origin[2] - aabb.extents[2]));
}

const Vector3 aabb_normals[6] = {
  Vector3( 1, 0, 0 ),
  Vector3( 0, 1, 0 ),
  Vector3( 0, 0, 1 ),
  Vector3(-1, 0, 0 ),
  Vector3( 0,-1, 0 ),
  Vector3( 0, 0,-1 ),
};

const float aabb_texcoord_topleft[2] = { 0, 0 };
const float aabb_texcoord_topright[2] = { 1, 0 };
const float aabb_texcoord_botleft[2] = { 0, 1 };
const float aabb_texcoord_botright[2] = { 1, 1 };


inline AABB aabb_for_oriented_aabb(const AABB& aabb, const Matrix4& transform)
{
  return AABB(
    matrix4_transformed_point(transform, aabb.origin),
    Vector3(
      static_cast<float>(fabs(transform[0]  * aabb.extents[0])
                          + fabs(transform[4]  * aabb.extents[1])
                          + fabs(transform[8]  * aabb.extents[2])),
      static_cast<float>(fabs(transform[1]  * aabb.extents[0])
                          + fabs(transform[5]  * aabb.extents[1])
                          + fabs(transform[9]  * aabb.extents[2])),
      static_cast<float>(fabs(transform[2]  * aabb.extents[0])
                          + fabs(transform[6]  * aabb.extents[1])
                          + fabs(transform[10] * aabb.extents[2]))
    )
  );
}

inline AABB aabb_for_oriented_aabb_safe(const AABB& aabb, const Matrix4& transform)
{
  if(aabb_valid(aabb))
  {
    return aabb_for_oriented_aabb(aabb, transform);
  }
  return aabb;
}

inline AABB aabb_infinite()
{
  return AABB(Vector3(0, 0, 0), Vector3(c_aabb_max, c_aabb_max, c_aabb_max));
}

#endif
