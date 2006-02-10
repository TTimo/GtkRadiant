/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
For a list of contributors, see the accompanying CONTRIBUTORS file.

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

#include "winding.h"

#include <algorithm>

#include "math/line.h"


inline double plane3_distance_to_point(const Plane3& plane, const DoubleVector3& point)
{
  return vector3_dot(point, plane.normal()) - plane.dist();
}

inline double plane3_distance_to_point(const Plane3& plane, const Vector3& point)
{
  return vector3_dot(point, plane.normal()) - plane.dist();
}

/// \brief Returns the point at which \p line intersects \p plane, or an undefined value if there is no intersection.
inline DoubleVector3 line_intersect_plane(const DoubleLine& line, const Plane3& plane)
{
  return line.origin + vector3_scaled(
    line.direction,
    -plane3_distance_to_point(plane, line.origin)
    / vector3_dot(line.direction, plane.normal())
  );
}

inline bool float_is_largest_absolute(double axis, double other)
{
  return fabs(axis) > fabs(other);
}

/// \brief Returns the index of the component of \p v that has the largest absolute value.
inline int vector3_largest_absolute_component_index(const DoubleVector3& v)
{
  return (float_is_largest_absolute(v[1], v[0]))
    ? (float_is_largest_absolute(v[1], v[2]))
      ? 1
      : 2
    : (float_is_largest_absolute(v[0], v[2]))
      ? 0
      : 2;
}

/// \brief Returns the infinite line that is the intersection of \p plane and \p other.
inline DoubleLine plane3_intersect_plane3(const Plane3& plane, const Plane3& other)
{
  DoubleLine line;
  line.direction = vector3_cross(plane.normal(), other.normal());
  switch(vector3_largest_absolute_component_index(line.direction))
  {
  case 0:
    line.origin.x() = 0;
    line.origin.y() = (-other.dist() * plane.normal().z() - -plane.dist() * other.normal().z()) / line.direction.x();
    line.origin.z() = (-plane.dist() * other.normal().y() - -other.dist() * plane.normal().y()) / line.direction.x();
    break;
  case 1:
    line.origin.x() = (-plane.dist() * other.normal().z() - -other.dist() * plane.normal().z()) / line.direction.y();
    line.origin.y() = 0;
    line.origin.z() = (-other.dist() * plane.normal().x() - -plane.dist() * other.normal().x()) / line.direction.y();
    break;
  case 2:
    line.origin.x() = (-other.dist() * plane.normal().y() - -plane.dist() * other.normal().y()) / line.direction.z();
    line.origin.y() = (-plane.dist() * other.normal().x() - -other.dist() * plane.normal().x()) / line.direction.z();
    line.origin.z() = 0;
    break;
  default:
    break;
  }

  return line;
}


/// \brief Keep the value of \p infinity as small as possible to improve precision in Winding_Clip.
void Winding_createInfinite(FixedWinding& winding, const Plane3& plane, double infinity)
{
  double max = -infinity;
  int x = -1;
  for (int i=0 ; i<3; i++)
  {
    double d = fabs(plane.normal()[i]);
    if (d > max)
    {
      x = i;
      max = d;
    }
  }
  if(x == -1)
  {
    globalErrorStream() << "invalid plane\n";
    return;
  }
    
  DoubleVector3 vup = g_vector3_identity;  
  switch (x)
  {
  case 0:
  case 1:
    vup[2] = 1;
    break;    
  case 2:
    vup[0] = 1;
    break;    
  }


  vector3_add(vup, vector3_scaled(plane.normal(), -vector3_dot(vup, plane.normal())));
  vector3_normalise(vup);
    
  DoubleVector3 org = vector3_scaled(plane.normal(), plane.dist());
  
  DoubleVector3 vright = vector3_cross(vup, plane.normal());
  
  vector3_scale(vup, infinity);
  vector3_scale(vright, infinity);

  // project a really big  axis aligned box onto the plane
  
  DoubleLine r1, r2, r3, r4;
  r1.origin = vector3_added(vector3_subtracted(org, vright), vup);
  r1.direction = vector3_normalised(vright);
  winding.push_back(FixedWindingVertex(r1.origin, r1, c_brush_maxFaces));
  r2.origin = vector3_added(vector3_added(org, vright), vup);
  r2.direction = vector3_normalised(vector3_negated(vup));
  winding.push_back(FixedWindingVertex(r2.origin, r2, c_brush_maxFaces));
  r3.origin = vector3_subtracted(vector3_added(org, vright), vup);
  r3.direction = vector3_normalised(vector3_negated(vright));
  winding.push_back(FixedWindingVertex(r3.origin, r3, c_brush_maxFaces));
  r4.origin = vector3_subtracted(vector3_subtracted(org, vright), vup);
  r4.direction = vector3_normalised(vup);
  winding.push_back(FixedWindingVertex(r4.origin, r4, c_brush_maxFaces));
}


inline PlaneClassification Winding_ClassifyDistance(const double distance, const double epsilon)
{
  if(distance > epsilon)
  {
    return ePlaneFront;
  }
  if(distance < -epsilon)
  {
    return ePlaneBack;
  }
  return ePlaneOn;
}

/// \brief Returns true if
/// !flipped && winding is completely BACK or ON
/// or flipped && winding is completely FRONT or ON
bool Winding_TestPlane(const Winding& winding, const Plane3& plane, bool flipped) 
{
  const int test = (flipped) ? ePlaneBack : ePlaneFront;
  for(Winding::const_iterator i = winding.begin(); i != winding.end(); ++i)
  {
    if(test == Winding_ClassifyDistance(plane3_distance_to_point(plane, (*i).vertex), ON_EPSILON))
    {
      return false;
    }
  }
  return true;
}

/// \brief Returns true if any point in \p w1 is in front of plane2, or any point in \p w2 is in front of plane1
bool Winding_PlanesConcave(const Winding& w1, const Winding& w2, const Plane3& plane1, const Plane3& plane2)
{
  return !Winding_TestPlane(w1, plane2, false) || !Winding_TestPlane(w2, plane1, false);
}

brushsplit_t Winding_ClassifyPlane(const Winding& winding, const Plane3& plane) 
{
  brushsplit_t split;
  for(Winding::const_iterator i = winding.begin(); i != winding.end(); ++i)
  {
    ++split.counts[Winding_ClassifyDistance(plane3_distance_to_point(plane, (*i).vertex), ON_EPSILON)];
  }
  return split;
}


#define DEBUG_EPSILON ON_EPSILON
const double DEBUG_EPSILON_SQUARED = DEBUG_EPSILON * DEBUG_EPSILON;

#define WINDING_DEBUG 0

/// \brief Clip \p winding which lies on \p plane by \p clipPlane, resulting in \p clipped.
/// If \p winding is completely in front of the plane, \p clipped will be identical to \p winding.  
/// If \p winding is completely in back of the plane, \p clipped will be empty.  
/// If \p winding intersects the plane, the edge of \p clipped which lies on \p clipPlane will store the value of \p adjacent.
void Winding_Clip(const FixedWinding& winding, const Plane3& plane, const Plane3& clipPlane, std::size_t adjacent, FixedWinding& clipped)
{
  PlaneClassification classification = Winding_ClassifyDistance(plane3_distance_to_point(clipPlane, winding.back().vertex), ON_EPSILON);
  PlaneClassification nextClassification;
  // for each edge
  for(std::size_t next = 0, i = winding.size()-1; next != winding.size(); i = next, ++next, classification = nextClassification)
  {
    nextClassification = Winding_ClassifyDistance(plane3_distance_to_point(clipPlane, winding[next].vertex), ON_EPSILON);
    const FixedWindingVertex& vertex = winding[i];

    // if first vertex of edge is ON
    if(classification == ePlaneOn)
    {
      // append first vertex to output winding
      if(nextClassification == ePlaneBack)
      {
        // this edge lies on the clip plane
        clipped.push_back(FixedWindingVertex(vertex.vertex, plane3_intersect_plane3(plane, clipPlane), adjacent));
      }
      else
      {
        clipped.push_back(vertex);
      }
      continue;
    }
  
    // if first vertex of edge is FRONT
    if(classification == ePlaneFront)
    {
      // add first vertex to output winding
      clipped.push_back(vertex);
    }
    // if second vertex of edge is ON
    if(nextClassification == ePlaneOn)
    {
      continue;
    }
    // else if second vertex of edge is same as first
    else if(nextClassification == classification)
    {
      continue;
    }
    // else if first vertex of edge is FRONT and there are only two edges
    else if(classification == ePlaneFront && winding.size() == 2)
    {
      continue;
    }
    // else first vertex is FRONT and second is BACK or vice versa
    else
    {
      // append intersection point of line and plane to output winding
      DoubleVector3 mid(line_intersect_plane(vertex.edge, clipPlane));

      if(classification == ePlaneFront)
      {
        // this edge lies on the clip plane
        clipped.push_back(FixedWindingVertex(mid, plane3_intersect_plane3(plane, clipPlane), adjacent));
      }
      else
      {
        clipped.push_back(FixedWindingVertex(mid, vertex.edge, vertex.adjacent));
      }
    }
  }
}

std::size_t Winding_FindAdjacent(const Winding& winding, std::size_t face)
{
  for(std::size_t i=0; i<winding.numpoints; ++i)
  {
    ASSERT_MESSAGE(winding[i].adjacent != c_brush_maxFaces, "edge connectivity data is invalid");
    if(winding[i].adjacent == face)
    {
      return i;
    }
  }
  return c_brush_maxFaces;
}

std::size_t Winding_Opposite(const Winding& winding, const std::size_t index, const std::size_t other)
{
  ASSERT_MESSAGE(index < winding.numpoints && other < winding.numpoints, "Winding_Opposite: index out of range");

  double dist_best = 0;
  std::size_t index_best = c_brush_maxFaces;

  Ray edge(ray_for_points(winding[index].vertex, winding[other].vertex));

  for(std::size_t i=0; i<winding.numpoints; ++i)
  {
    if(i == index || i == other)
    {
      continue;
    }

    double dist_squared = ray_squared_distance_to_point(edge, winding[i].vertex);

    if(dist_squared > dist_best)
    {
      dist_best = dist_squared;
      index_best = i;
    }
  }
  return index_best;
}

std::size_t Winding_Opposite(const Winding& winding, const std::size_t index)
{
  return Winding_Opposite(winding, index, Winding_next(winding, index));
}

/// \brief Calculate the \p centroid of the polygon defined by \p winding which lies on plane \p plane.
void Winding_Centroid(const Winding& winding, const Plane3& plane, Vector3& centroid)
{
  double area2 = 0, x_sum = 0, y_sum = 0;
  const ProjectionAxis axis = projectionaxis_for_normal(plane.normal());
  const indexremap_t remap = indexremap_for_projectionaxis(axis);
  for(std::size_t i = winding.numpoints-1, j = 0; j < winding.numpoints; i = j, ++j)
  {
    const double ai = winding[i].vertex[remap.x] * winding[j].vertex[remap.y] - winding[j].vertex[remap.x] * winding[i].vertex[remap.y];
    area2 += ai;
    x_sum += (winding[j].vertex[remap.x] + winding[i].vertex[remap.x]) * ai;
    y_sum += (winding[j].vertex[remap.y] + winding[i].vertex[remap.y]) * ai;
  }

  centroid[remap.x] = static_cast<float>(x_sum / (3 * area2));
  centroid[remap.y] = static_cast<float>(y_sum / (3 * area2));
  {
    Ray ray(Vector3(0, 0, 0), Vector3(0, 0, 0));
    ray.origin[remap.x] = centroid[remap.x];
    ray.origin[remap.y] = centroid[remap.y];
    ray.direction[remap.z] = 1;
    centroid[remap.z] = static_cast<float>(ray_distance_to_plane(ray, plane));
  }
}
