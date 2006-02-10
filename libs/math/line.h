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

#if !defined(INCLUDED_MATH_LINE_H)
#define INCLUDED_MATH_LINE_H

/// \file
/// \brief Line data types and related operations.

#include "math/vector.h"
#include "math/plane.h"

/// \brief A line segment defined by a start point and and end point.
class Line
{
public:
  Vector3 start, end;

  Line()
  {
  }
  Line(const Vector3& start_, const Vector3& end_) : start(start_), end(end_)
  {
  }
};

inline Vector3 line_closest_point(const Line& line, const Vector3& point)
{
  Vector3 v = line.end - line.start;
  Vector3 w = point - line.start;

  double c1 = vector3_dot(w,v);
  if ( c1 <= 0 )
    return line.start;

  double c2 = vector3_dot(v,v);
  if ( c2 <= c1 )
    return line.end;

  return Vector3(line.start + v * (c1 / c2));
}


class Segment
{
public:
  Vector3 origin, extents;

  Segment()
  {
  }
  Segment(const Vector3& origin_, const Vector3& extents_) :
    origin(origin_), extents(extents_)
  {
  }
};


inline Segment segment_for_startend(const Vector3& start, const Vector3& end)
{
  Segment segment;
  segment.origin = vector3_mid(start, end);
  segment.extents = vector3_subtracted(end, segment.origin);
  return segment;
}

inline unsigned int segment_classify_plane(const Segment& segment, const Plane3& plane)
{
  double distance_origin = vector3_dot(plane.normal(), segment.origin) + plane.dist();
  
  if (fabs(distance_origin) < fabs(vector3_dot(plane.normal(), segment.extents)))
  {
    return 1; // partially inside
  }
  else if (distance_origin < 0)
  {
    return 2; // totally inside
  }
  return 0; // totally outside
}


class Ray
{
public:
  Vector3 origin, direction;

  Ray()
  {
  }
  Ray(const Vector3& origin_, const Vector3& direction_) :
    origin(origin_), direction(direction_)
  {
  }
};

inline Ray ray_for_points(const Vector3& origin, const Vector3& p2)
{
  return Ray(origin, vector3_normalised(vector3_subtracted(p2, origin)));
}

inline void ray_transform(Ray& ray, const Matrix4& matrix)
{
  matrix4_transform_point(matrix, ray.origin);
  matrix4_transform_direction(matrix, ray.direction);
}

// closest-point-on-line
inline double ray_squared_distance_to_point(const Ray& ray, const Vector3& point)
{
  return vector3_length_squared(
    vector3_subtracted(
      point,
      vector3_added(
        ray.origin,
        vector3_scaled(
          ray.direction,
          vector3_dot(
            vector3_subtracted(point, ray.origin),
            ray.direction
          )
        )
      )
    )
  );
}

inline double ray_distance_to_plane(const Ray& ray, const Plane3& plane)
{
  return -(vector3_dot(plane.normal(), ray.origin) - plane.dist()) / vector3_dot(ray.direction, plane.normal());
}

#endif
