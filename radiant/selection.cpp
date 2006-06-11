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

#include "selection.h"

#include "debugging/debugging.h"

#include <map>
#include <list>
#include <set>

#include "windowobserver.h"
#include "iundo.h"
#include "ientity.h"
#include "cullable.h"
#include "renderable.h"
#include "selectable.h"
#include "editable.h"

#include "math/frustum.h"
#include "signal/signal.h"
#include "generic/object.h"
#include "selectionlib.h"
#include "render.h"
#include "view.h"
#include "renderer.h"
#include "stream/stringstream.h"
#include "eclasslib.h"
#include "generic/bitfield.h"
#include "generic/static.h"
#include "pivot.h"
#include "stringio.h"
#include "container/container.h"

#include "grid.h"

struct Pivot2World
{
  Matrix4 m_worldSpace;
  Matrix4 m_viewpointSpace;
  Matrix4 m_viewplaneSpace;
  Vector3 m_axis_screen;

  void update(const Matrix4& pivot2world, const Matrix4& modelview, const Matrix4& projection, const Matrix4& viewport)
  {
    Pivot2World_worldSpace(m_worldSpace, pivot2world, modelview, projection, viewport);
    Pivot2World_viewpointSpace(m_viewpointSpace, m_axis_screen, pivot2world, modelview, projection, viewport);
    Pivot2World_viewplaneSpace(m_viewplaneSpace, pivot2world, modelview, projection, viewport);
  }
};


void point_for_device_point(Vector3& point, const Matrix4& device2object, const float x, const float y, const float z)
{
  // transform from normalised device coords to object coords
  point = vector4_projected(matrix4_transformed_vector4(device2object, Vector4(x, y, z, 1)));
}

void ray_for_device_point(Ray& ray, const Matrix4& device2object, const float x, const float y)
{
    // point at x, y, zNear
  point_for_device_point(ray.origin, device2object, x, y, -1);

  // point at x, y, zFar
  point_for_device_point(ray.direction, device2object, x, y, 1);

  // construct ray
  vector3_subtract(ray.direction, ray.origin);
  vector3_normalise(ray.direction);
}

bool sphere_intersect_ray(const Vector3& origin, float radius, const Ray& ray, Vector3& intersection)
{
  intersection = vector3_subtracted(origin, ray.origin);
  const double a = vector3_dot(intersection, ray.direction);
  const double d = radius * radius - (vector3_dot(intersection, intersection) - a * a);

  if(d > 0)
  {
    intersection = vector3_added(ray.origin, vector3_scaled(ray.direction, a - sqrt(d)));
    return true;
  }
  else
  {
    intersection = vector3_added( ray.origin, vector3_scaled(ray.direction, a));
    return false;
  }
}

void ray_intersect_ray(const Ray& ray, const Ray& other, Vector3& intersection)
{
  intersection = vector3_subtracted(ray.origin, other.origin);
  //float a = 1;//vector3_dot(ray.direction, ray.direction);        // always >= 0
  double dot = vector3_dot(ray.direction, other.direction);
  //float c = 1;//vector3_dot(other.direction, other.direction);        // always >= 0
  double d = vector3_dot(ray.direction, intersection);
  double e = vector3_dot(other.direction, intersection);
  double D = 1 - dot*dot;//a*c - dot*dot;       // always >= 0

  if (D < 0.000001)
  {
    // the lines are almost parallel
    intersection = vector3_added(other.origin, vector3_scaled(other.direction, e));
  }
  else
  {
    intersection = vector3_added(other.origin, vector3_scaled(other.direction, (e - dot*d) / D));
  }    
}

const Vector3 g_origin(0, 0, 0);
const float g_radius = 64;

void point_on_sphere(Vector3& point, const Matrix4& device2object, const float x, const float y)
{
  Ray ray;
  ray_for_device_point(ray, device2object, x, y);
  sphere_intersect_ray(g_origin, g_radius, ray, point);
}

void point_on_axis(Vector3& point, const Vector3& axis, const Matrix4& device2object, const float x, const float y)
{
  Ray ray;
  ray_for_device_point(ray, device2object, x, y);
  ray_intersect_ray(ray, Ray(Vector3(0, 0, 0), axis), point);
}

void point_on_plane(Vector3& point, const Matrix4& device2object, const float x, const float y)
{
  Matrix4 object2device(matrix4_full_inverse(device2object));
  point = vector4_projected(matrix4_transformed_vector4(device2object, Vector4(x, y, object2device[14] / object2device[15], 1)));
}

//! a and b are unit vectors .. returns angle in radians
inline float angle_between(const Vector3& a, const Vector3& b)
{
  return static_cast<float>(2.0 * atan2(
    vector3_length(vector3_subtracted(a, b)),
    vector3_length(vector3_added(a, b))
  ));
}


#if defined(_DEBUG)
class test_quat
{
public:
  test_quat(const Vector3& from, const Vector3& to)
  {
    Vector4 quaternion(quaternion_for_unit_vectors(from, to));
    Matrix4 matrix(matrix4_rotation_for_quaternion(quaternion_multiplied_by_quaternion(quaternion, c_quaternion_identity)));
  }
private:
};

static test_quat bleh(g_vector3_axis_x, g_vector3_axis_y);
#endif

//! axis is a unit vector
inline void constrain_to_axis(Vector3& vec, const Vector3& axis)
{
  vec = vector3_normalised(vector3_added(vec, vector3_scaled(axis, -vector3_dot(vec, axis))));
}

//! a and b are unit vectors .. a and b must be orthogonal to axis .. returns angle in radians
float angle_for_axis(const Vector3& a, const Vector3& b, const Vector3& axis)
{
  if(vector3_dot(axis, vector3_cross(a, b)) > 0.0)
    return angle_between(a, b);
  else
    return -angle_between(a, b);
}

float distance_for_axis(const Vector3& a, const Vector3& b, const Vector3& axis)
{
  return static_cast<float>(vector3_dot(b, axis) - vector3_dot(a, axis));
}

class Manipulatable
{
public:
  virtual void Construct(const Matrix4& device2manip, const float x, const float y) = 0;
  virtual void Transform(const Matrix4& manip2object, const Matrix4& device2manip, const float x, const float y) = 0;
};

void transform_local2object(Matrix4& object, const Matrix4& local, const Matrix4& local2object)
{
  object = matrix4_multiplied_by_matrix4(
    matrix4_multiplied_by_matrix4(local2object, local),
    matrix4_full_inverse(local2object)
  );
}

class Rotatable
{
public:
  virtual void rotate(const Quaternion& rotation) = 0;
};

class RotateFree : public Manipulatable
{
  Vector3 m_start;
  Rotatable& m_rotatable;
public:
  RotateFree(Rotatable& rotatable)
    : m_rotatable(rotatable)
  {
  }
  void Construct(const Matrix4& device2manip, const float x, const float y)
  {
    point_on_sphere(m_start, device2manip, x, y);
    vector3_normalise(m_start);
  }
  void Transform(const Matrix4& manip2object, const Matrix4& device2manip, const float x, const float y)
  {
    Vector3 current;

    point_on_sphere(current, device2manip, x, y);
    vector3_normalise(current);

    m_rotatable.rotate(quaternion_for_unit_vectors(m_start, current));
  }
};

class RotateAxis : public Manipulatable
{
  Vector3 m_axis;
  Vector3 m_start;
  Rotatable& m_rotatable;
public:
  RotateAxis(Rotatable& rotatable)
    : m_rotatable(rotatable)
  {
  }
  void Construct(const Matrix4& device2manip, const float x, const float y)
  {
    point_on_sphere(m_start, device2manip, x, y);
    constrain_to_axis(m_start, m_axis);
  }
  /// \brief Converts current position to a normalised vector orthogonal to axis.
  void Transform(const Matrix4& manip2object, const Matrix4& device2manip, const float x, const float y)
  {
    Vector3 current;
    point_on_sphere(current, device2manip, x, y);
    constrain_to_axis(current, m_axis);

    m_rotatable.rotate(quaternion_for_axisangle(m_axis, angle_for_axis(m_start, current, m_axis)));
  }

  void SetAxis(const Vector3& axis)
  {
    m_axis = axis;
  }
};

void translation_local2object(Vector3& object, const Vector3& local, const Matrix4& local2object)
{
  object = matrix4_get_translation_vec3(
    matrix4_multiplied_by_matrix4(
      matrix4_translated_by_vec3(local2object, local),
      matrix4_full_inverse(local2object)
    )
  );
}

class Translatable
{
public:
  virtual void translate(const Vector3& translation) = 0;
};

class TranslateAxis : public Manipulatable
{
  Vector3 m_start;
  Vector3 m_axis;
  Translatable& m_translatable;
public:
  TranslateAxis(Translatable& translatable)
    : m_translatable(translatable)
  {
  }
  void Construct(const Matrix4& device2manip, const float x, const float y)
  {
    point_on_axis(m_start, m_axis, device2manip, x, y);
  }
  void Transform(const Matrix4& manip2object, const Matrix4& device2manip, const float x, const float y)
  {
    Vector3 current;
    point_on_axis(current, m_axis, device2manip, x, y);
    current = vector3_scaled(m_axis, distance_for_axis(m_start, current, m_axis));

    translation_local2object(current, current, manip2object);
    vector3_snap(current, GetGridSize());

    m_translatable.translate(current);
  }

  void SetAxis(const Vector3& axis)
  {
    m_axis = axis;
  }
};

class TranslateFree : public Manipulatable
{
private:
  Vector3 m_start;
  Translatable& m_translatable;
public:
  TranslateFree(Translatable& translatable)
    : m_translatable(translatable)
  {
  }
  void Construct(const Matrix4& device2manip, const float x, const float y)
  {
    point_on_plane(m_start, device2manip, x, y);
  }
  void Transform(const Matrix4& manip2object, const Matrix4& device2manip, const float x, const float y)
  {
    Vector3 current;
    point_on_plane(current, device2manip, x, y);
    current = vector3_subtracted(current, m_start);

    translation_local2object(current, current, manip2object);
    vector3_snap(current, GetGridSize());
    
    m_translatable.translate(current);
  }
};


class Scalable
{
public:
  virtual void scale(const Vector3& scaling) = 0;
};


class ScaleAxis : public Manipulatable
{
private:
  Vector3 m_start;
  Vector3 m_axis;
  Scalable& m_scalable;
public:
  ScaleAxis(Scalable& scalable)
    : m_scalable(scalable)
  {
  }
  void Construct(const Matrix4& device2manip, const float x, const float y)
  {
    point_on_axis(m_start, m_axis, device2manip, x, y);
  }
  void Transform(const Matrix4& manip2object, const Matrix4& device2manip, const float x, const float y)
  {
    Vector3 current;
    point_on_axis(current, m_axis, device2manip, x, y);
    Vector3 delta = vector3_subtracted(current, m_start);

    translation_local2object(delta, delta, manip2object);
    vector3_snap(delta, GetGridSize());
    
    Vector3 start(vector3_snapped(m_start, GetGridSize()));
    Vector3 scale(
      start[0] == 0 ? 1 : 1 + delta[0] / start[0],
      start[1] == 0 ? 1 : 1 + delta[1] / start[1],
      start[2] == 0 ? 1 : 1 + delta[2] / start[2]
    );
    m_scalable.scale(scale);
  }

  void SetAxis(const Vector3& axis)
  {
    m_axis = axis;
  }
};

class ScaleFree : public Manipulatable
{
private:
  Vector3 m_start;
  Scalable& m_scalable;
public:
  ScaleFree(Scalable& scalable)
    : m_scalable(scalable)
  {
  }
  void Construct(const Matrix4& device2manip, const float x, const float y)
  {
    point_on_plane(m_start, device2manip, x, y);
  }
  void Transform(const Matrix4& manip2object, const Matrix4& device2manip, const float x, const float y)
  {
    Vector3 current;
    point_on_plane(current, device2manip, x, y);
    Vector3 delta = vector3_subtracted(current, m_start);

    translation_local2object(delta, delta, manip2object);
    vector3_snap(delta, GetGridSize());
    
    Vector3 start(vector3_snapped(m_start, GetGridSize()));
    Vector3 scale(
      start[0] == 0 ? 1 : 1 + delta[0] / start[0],
      start[1] == 0 ? 1 : 1 + delta[1] / start[1],
      start[2] == 0 ? 1 : 1 + delta[2] / start[2]
    );
    m_scalable.scale(scale);
  }
};










class RenderableClippedPrimitive : public OpenGLRenderable
{
  struct primitive_t
  {
    PointVertex m_points[9];
    std::size_t m_count;
  };
  Matrix4 m_inverse;
  std::vector<primitive_t> m_primitives;
public:
  Matrix4 m_world;

  void render(RenderStateFlags state) const
  {
    for(std::size_t i=0; i<m_primitives.size(); ++i)
    {
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(PointVertex), &m_primitives[i].m_points[0].colour);
      glVertexPointer(3, GL_FLOAT, sizeof(PointVertex), &m_primitives[i].m_points[0].vertex);
      switch(m_primitives[i].m_count)
      {
      case 1: break;
      case 2: glDrawArrays(GL_LINES, 0, GLsizei(m_primitives[i].m_count)); break;
      default: glDrawArrays(GL_POLYGON, 0, GLsizei(m_primitives[i].m_count)); break;
      }
    }
  }

  void construct(const Matrix4& world2device)
  {
    m_inverse = matrix4_full_inverse(world2device);
    m_world = g_matrix4_identity;
  }

  void insert(const Vector4 clipped[9], std::size_t count)
  {
    add_one();

    m_primitives.back().m_count = count;
    for(std::size_t i=0; i<count; ++i)
    {
      Vector3 world_point(vector4_projected(matrix4_transformed_vector4(m_inverse, clipped[i])));
      m_primitives.back().m_points[i].vertex = vertex3f_for_vector3(world_point);
    }
  }

  void destroy()
  {
    m_primitives.clear();
  }
private:
  void add_one()
  {
    m_primitives.push_back(primitive_t());

    const Colour4b colour_clipped(255, 127, 0, 255);

    for(std::size_t i=0; i<9; ++i)
      m_primitives.back().m_points[i].colour = colour_clipped;
  }
};

#if defined(_DEBUG)
#define DEBUG_SELECTION
#endif

#if defined(DEBUG_SELECTION)
Shader* g_state_clipped;
RenderableClippedPrimitive g_render_clipped;
#endif


#if 0
// dist_Point_to_Line(): get the distance of a point to a line.
//    Input:  a Point P and a Line L (in any dimension)
//    Return: the shortest distance from P to L
float
dist_Point_to_Line( Point P, Line L)
{
    Vector v = L.P1 - L.P0;
    Vector w = P - L.P0;

    double c1 = dot(w,v);
    double c2 = dot(v,v);
    double b = c1 / c2;

    Point Pb = L.P0 + b * v;
    return d(P, Pb);
}
#endif

class Segment3D
{
  typedef Vector3 point_type;
public:
  Segment3D(const point_type& _p0, const point_type& _p1)
    : p0(_p0), p1(_p1)
  {
  }

  point_type p0, p1;
};

typedef Vector3 Point3D;

inline double vector3_distance_squared(const Point3D& a, const Point3D& b)
{
  return vector3_length_squared(b - a);
}

// get the distance of a point to a segment.
Point3D segment_closest_point_to_point(const Segment3D& segment, const Point3D& point)
{
  Vector3 v = segment.p1 - segment.p0;
  Vector3 w = point - segment.p0;

  double c1 = vector3_dot(w,v);
  if ( c1 <= 0 )
    return segment.p0;

  double c2 = vector3_dot(v,v);
  if ( c2 <= c1 )
    return segment.p1;

  return Point3D(segment.p0 + v * (c1 / c2));
}

double segment_dist_to_point_3d(const Segment3D& segment, const Point3D& point)
{
  return vector3_distance_squared(point, segment_closest_point_to_point(segment, point));
}

typedef Vector3 point_t;
typedef const Vector3* point_iterator_t;

// crossing number test for a point in a polygon
// This code is patterned after [Franklin, 2000]
bool point_test_polygon_2d( const point_t& P, point_iterator_t start, point_iterator_t finish )
{
  std::size_t crossings = 0;

  // loop through all edges of the polygon
  for(point_iterator_t prev = finish-1, cur = start; cur != finish; prev = cur, ++cur)
  {    // edge from (*prev) to (*cur)
    if ((((*prev)[1] <= P[1]) && ((*cur)[1] > P[1]))    // an upward crossing
    || (((*prev)[1] > P[1]) && ((*cur)[1] <= P[1])))
    { // a downward crossing
      // compute the actual edge-ray intersect x-coordinate
      float vt = (float)(P[1] - (*prev)[1]) / ((*cur)[1] - (*prev)[1]);
      if (P[0] < (*prev)[0] + vt * ((*cur)[0] - (*prev)[0])) // P[0] < intersect
      {
        ++crossings;   // a valid crossing of y=P[1] right of P[0]
      }
    }
  }
  return (crossings & 0x1) != 0;    // 0 if even (out), and 1 if odd (in)
}

inline double triangle_signed_area_XY(const Vector3& p0, const Vector3& p1, const Vector3& p2)
{
  return ((p1[0] - p0[0]) * (p2[1] - p0[1])) - ((p2[0] - p0[0]) * (p1[1] - p0[1]));
}

enum clipcull_t
{
  eClipCullNone,
  eClipCullCW,
  eClipCullCCW,
};


inline SelectionIntersection select_point_from_clipped(Vector4& clipped)
{
  return SelectionIntersection(clipped[2] / clipped[3], static_cast<float>(vector3_length_squared(Vector3(clipped[0] / clipped[3], clipped[1] / clipped[3], 0))));
}

void BestPoint(std::size_t count, Vector4 clipped[9], SelectionIntersection& best, clipcull_t cull)
{
  Vector3 normalised[9];

  {
    for(std::size_t i=0; i<count; ++i)
    {
      normalised[i][0] = clipped[i][0] / clipped[i][3];
      normalised[i][1] = clipped[i][1] / clipped[i][3];
      normalised[i][2] = clipped[i][2] / clipped[i][3];
    }
  }

  if(cull != eClipCullNone && count > 2)
  {      
    double signed_area = triangle_signed_area_XY(normalised[0], normalised[1], normalised[2]);

    if((cull == eClipCullCW && signed_area > 0)
      || (cull == eClipCullCCW && signed_area < 0))
      return;
  }

  if(count == 2)
  {
    Segment3D segment(normalised[0], normalised[1]);
    Point3D point = segment_closest_point_to_point(segment, Vector3(0, 0, 0));
    assign_if_closer(best, SelectionIntersection(point.z(), 0));
  }
  else if(count > 2 && !point_test_polygon_2d(Vector3(0, 0, 0), normalised, normalised + count))
  {
    point_iterator_t end = normalised + count;
    for(point_iterator_t previous = end-1, current = normalised; current != end; previous = current, ++current)
    {
      Segment3D segment(*previous, *current);
      Point3D point = segment_closest_point_to_point(segment, Vector3(0, 0, 0));
      float depth = point.z();
      point.z() = 0;
      float distance = static_cast<float>(vector3_length_squared(point));

      assign_if_closer(best, SelectionIntersection(depth, distance));
    }
  }
  else if(count > 2)
  {
    assign_if_closer(
      best,
      SelectionIntersection(
      static_cast<float>(ray_distance_to_plane(
          Ray(Vector3(0, 0, 0), Vector3(0, 0, 1)),
          plane3_for_points(normalised[0], normalised[1], normalised[2])
        )), 
        0
      )
    );
  }

#if defined(DEBUG_SELECTION)
    if(count >= 2)
      g_render_clipped.insert(clipped, count);
#endif
}

void LineStrip_BestPoint(const Matrix4& local2view, const PointVertex* vertices, const std::size_t size, SelectionIntersection& best)
{
  Vector4 clipped[2];
  for(std::size_t i = 0; (i + 1) < size; ++i)
  {
    const std::size_t count = matrix4_clip_line(local2view, vertex3f_to_vector3(vertices[i].vertex), vertex3f_to_vector3(vertices[i + 1].vertex), clipped);
    BestPoint(count, clipped, best, eClipCullNone);
  }
}

void LineLoop_BestPoint(const Matrix4& local2view, const PointVertex* vertices, const std::size_t size, SelectionIntersection& best)
{
  Vector4 clipped[2];
  for(std::size_t i = 0; i < size; ++i)
  {
    const std::size_t count = matrix4_clip_line(local2view, vertex3f_to_vector3(vertices[i].vertex), vertex3f_to_vector3(vertices[(i+1)%size].vertex), clipped);
    BestPoint(count, clipped, best, eClipCullNone);
  }
}

void Line_BestPoint(const Matrix4& local2view, const PointVertex vertices[2], SelectionIntersection& best)
{
  Vector4 clipped[2];
  const std::size_t count = matrix4_clip_line(local2view, vertex3f_to_vector3(vertices[0].vertex), vertex3f_to_vector3(vertices[1].vertex), clipped);
  BestPoint(count, clipped, best, eClipCullNone);
}

void Circle_BestPoint(const Matrix4& local2view, clipcull_t cull, const PointVertex* vertices, const std::size_t size, SelectionIntersection& best)
{
  Vector4 clipped[9];
  for(std::size_t i=0; i<size; ++i)
  {
    const std::size_t count = matrix4_clip_triangle(local2view, g_vector3_identity, vertex3f_to_vector3(vertices[i].vertex), vertex3f_to_vector3(vertices[(i+1)%size].vertex), clipped);
    BestPoint(count, clipped, best, cull);
  }
}

void Quad_BestPoint(const Matrix4& local2view, clipcull_t cull, const PointVertex* vertices, SelectionIntersection& best)
{
  Vector4 clipped[9];
  {
    const std::size_t count = matrix4_clip_triangle(local2view, vertex3f_to_vector3(vertices[0].vertex), vertex3f_to_vector3(vertices[1].vertex), vertex3f_to_vector3(vertices[3].vertex), clipped);
    BestPoint(count, clipped, best, cull);
  }
  {
    const std::size_t count = matrix4_clip_triangle(local2view, vertex3f_to_vector3(vertices[1].vertex), vertex3f_to_vector3(vertices[2].vertex), vertex3f_to_vector3(vertices[3].vertex), clipped);
	  BestPoint(count, clipped, best, cull);
  }
}

struct FlatShadedVertex
{
  Vertex3f vertex;
  Colour4b colour;
  Normal3f normal;

  FlatShadedVertex()
  {
  }
};


typedef FlatShadedVertex* FlatShadedVertexIterator;
void Triangles_BestPoint(const Matrix4& local2view, clipcull_t cull, FlatShadedVertexIterator first, FlatShadedVertexIterator last, SelectionIntersection& best)
{
  for(FlatShadedVertexIterator x(first), y(first+1), z(first+2); x != last; x += 3, y += 3, z +=3)
  {
    Vector4 clipped[9];
    BestPoint(
      matrix4_clip_triangle(
        local2view,
        reinterpret_cast<const Vector3&>((*x).vertex),
        reinterpret_cast<const Vector3&>((*y).vertex),
        reinterpret_cast<const Vector3&>((*z).vertex),
        clipped
      ),
      clipped,
      best,
      cull
    );
  }
}


typedef std::multimap<SelectionIntersection, Selectable*> SelectableSortedSet;

class SelectionPool : public Selector
{
  SelectableSortedSet m_pool;
  SelectionIntersection m_intersection;
  Selectable* m_selectable;

public:
  void pushSelectable(Selectable& selectable)
  {
    m_intersection = SelectionIntersection();
    m_selectable = &selectable;
  }
  void popSelectable()
  {
    addSelectable(m_intersection, m_selectable);
    m_intersection = SelectionIntersection();
  }
  void addIntersection(const SelectionIntersection& intersection)
  {
    assign_if_closer(m_intersection, intersection);
  }
  void addSelectable(const SelectionIntersection& intersection, Selectable* selectable)
  {
    if(intersection.valid())
    {
      m_pool.insert(SelectableSortedSet::value_type(intersection, selectable));
    }
  }

  typedef SelectableSortedSet::iterator iterator;

  iterator begin()
  {
    return m_pool.begin();
  }
  iterator end()
  {
    return m_pool.end();
  }

  bool failed()
  {
    return m_pool.empty();
  }
};


const Colour4b g_colour_sphere(0, 0, 0, 255);
const Colour4b g_colour_screen(0, 255, 255, 255);
const Colour4b g_colour_selected(255, 255, 0, 255);

inline const Colour4b& colourSelected(const Colour4b& colour, bool selected)
{
  return (selected) ? g_colour_selected : colour;
}

template<typename remap_policy>
inline void draw_semicircle(const std::size_t segments, const float radius, PointVertex* vertices, remap_policy remap)
{
  const double increment = c_pi / double(segments << 2);

  std::size_t count = 0;
  float x = radius;
  float y = 0;
  remap_policy::set(vertices[segments << 2].vertex, -radius, 0, 0);
  while(count < segments)
  {
    PointVertex* i = vertices + count;
    PointVertex* j = vertices + ((segments << 1) - (count + 1));

    PointVertex* k = i + (segments << 1);
    PointVertex* l = j + (segments << 1);

#if 0
    PointVertex* m = i + (segments << 2);
    PointVertex* n = j + (segments << 2);
    PointVertex* o = k + (segments << 2);
    PointVertex* p = l + (segments << 2);
#endif

    remap_policy::set(i->vertex, x,-y, 0);
    remap_policy::set(k->vertex,-y,-x, 0);
#if 0
    remap_policy::set(m->vertex,-x, y, 0);
    remap_policy::set(o->vertex, y, x, 0);
#endif

    ++count;

    {
      const double theta = increment * count;
      x = static_cast<float>(radius * cos(theta));
      y = static_cast<float>(radius * sin(theta));
    }

    remap_policy::set(j->vertex, y,-x, 0);
    remap_policy::set(l->vertex,-x,-y, 0);
#if 0
    remap_policy::set(n->vertex,-y, x, 0);
    remap_policy::set(p->vertex, x, y, 0);
#endif
  }
}

class Manipulator
{
public:
  virtual Manipulatable* GetManipulatable() = 0;
  virtual void testSelect(const View& view, const Matrix4& pivot2world)
  {
  }
  virtual void render(Renderer& renderer, const VolumeTest& volume, const Matrix4& pivot2world)
  {
  }
  virtual void setSelected(bool select) = 0;
  virtual bool isSelected() const = 0;
};


inline Vector3 normalised_safe(const Vector3& self)
{
  if(vector3_equal(self, g_vector3_identity))
  {
    return g_vector3_identity;
  }
  return vector3_normalised(self);
}


class RotateManipulator : public Manipulator
{
  struct RenderableCircle : public OpenGLRenderable
  {
    Array<PointVertex> m_vertices;

    RenderableCircle(std::size_t size) : m_vertices(size)
    {
    }
    void render(RenderStateFlags state) const
    {
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(PointVertex), &m_vertices.data()->colour);
      glVertexPointer(3, GL_FLOAT, sizeof(PointVertex), &m_vertices.data()->vertex);
      glDrawArrays(GL_LINE_LOOP, 0, GLsizei(m_vertices.size()));
    }
    void setColour(const Colour4b& colour)
    {
      for(Array<PointVertex>::iterator i = m_vertices.begin(); i != m_vertices.end(); ++i)
      {
        (*i).colour = colour;
      }
    }
  };

  struct RenderableSemiCircle : public OpenGLRenderable
  {
    Array<PointVertex> m_vertices;

    RenderableSemiCircle(std::size_t size) : m_vertices(size)
    {
    }
    void render(RenderStateFlags state) const
    {
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(PointVertex), &m_vertices.data()->colour);
      glVertexPointer(3, GL_FLOAT, sizeof(PointVertex), &m_vertices.data()->vertex);
      glDrawArrays(GL_LINE_STRIP, 0, GLsizei(m_vertices.size()));
    }
    void setColour(const Colour4b& colour)
    {
      for(Array<PointVertex>::iterator i = m_vertices.begin(); i != m_vertices.end(); ++i)
      {
        (*i).colour = colour;
      }
    }
  };

  RotateFree m_free;
  RotateAxis m_axis;
  Vector3 m_axis_screen;
  RenderableSemiCircle m_circle_x;
  RenderableSemiCircle m_circle_y;
  RenderableSemiCircle m_circle_z;
  RenderableCircle m_circle_screen;
  RenderableCircle m_circle_sphere;
  SelectableBool m_selectable_x;
  SelectableBool m_selectable_y;
  SelectableBool m_selectable_z;
  SelectableBool m_selectable_screen;
  SelectableBool m_selectable_sphere;
  Pivot2World m_pivot;
  Matrix4 m_local2world_x;
  Matrix4 m_local2world_y;
  Matrix4 m_local2world_z;
  bool m_circle_x_visible;
  bool m_circle_y_visible;
  bool m_circle_z_visible;
public:
  static Shader* m_state_outer;

  RotateManipulator(Rotatable& rotatable, std::size_t segments, float radius) :
    m_free(rotatable),
    m_axis(rotatable),
    m_circle_x((segments << 2) + 1),
    m_circle_y((segments << 2) + 1),
    m_circle_z((segments << 2) + 1),
    m_circle_screen(segments<<3),
    m_circle_sphere(segments<<3)
  {
    draw_semicircle(segments, radius, m_circle_x.m_vertices.data(), RemapYZX());
    draw_semicircle(segments, radius, m_circle_y.m_vertices.data(), RemapZXY());
    draw_semicircle(segments, radius, m_circle_z.m_vertices.data(), RemapXYZ());

    draw_circle(segments, radius * 1.15f, m_circle_screen.m_vertices.data(), RemapXYZ());
    draw_circle(segments, radius, m_circle_sphere.m_vertices.data(), RemapXYZ());
  }


  void UpdateColours()
  {
    m_circle_x.setColour(colourSelected(g_colour_x, m_selectable_x.isSelected()));
    m_circle_y.setColour(colourSelected(g_colour_y, m_selectable_y.isSelected()));
    m_circle_z.setColour(colourSelected(g_colour_z, m_selectable_z.isSelected()));
    m_circle_screen.setColour(colourSelected(g_colour_screen, m_selectable_screen.isSelected()));
    m_circle_sphere.setColour(colourSelected(g_colour_sphere, false));
  }
  
  void updateCircleTransforms()
  {
    Vector3 localViewpoint(matrix4_transformed_direction(matrix4_transposed(m_pivot.m_worldSpace), vector4_to_vector3(m_pivot.m_viewpointSpace.z())));

    m_circle_x_visible = !vector3_equal_epsilon(g_vector3_axis_x, localViewpoint, 1e-6f);
    if(m_circle_x_visible)
    {
      m_local2world_x = g_matrix4_identity;
      vector4_to_vector3(m_local2world_x.y()) = normalised_safe(
        vector3_cross(g_vector3_axis_x, localViewpoint)
      );
      vector4_to_vector3(m_local2world_x.z()) = normalised_safe(
        vector3_cross(vector4_to_vector3(m_local2world_x.x()), vector4_to_vector3(m_local2world_x.y()))
      );
      matrix4_premultiply_by_matrix4(m_local2world_x, m_pivot.m_worldSpace);
    }

    m_circle_y_visible = !vector3_equal_epsilon(g_vector3_axis_y, localViewpoint, 1e-6f);
    if(m_circle_y_visible)
    {
      m_local2world_y = g_matrix4_identity;
      vector4_to_vector3(m_local2world_y.z()) = normalised_safe(
        vector3_cross(g_vector3_axis_y, localViewpoint)
      );
      vector4_to_vector3(m_local2world_y.x()) = normalised_safe(
        vector3_cross(vector4_to_vector3(m_local2world_y.y()), vector4_to_vector3(m_local2world_y.z()))
      );
      matrix4_premultiply_by_matrix4(m_local2world_y, m_pivot.m_worldSpace);
    }

    m_circle_z_visible = !vector3_equal_epsilon(g_vector3_axis_z, localViewpoint, 1e-6f);
    if(m_circle_z_visible)
    {
      m_local2world_z = g_matrix4_identity;
      vector4_to_vector3(m_local2world_z.x()) = normalised_safe(
        vector3_cross(g_vector3_axis_z, localViewpoint)
      );
      vector4_to_vector3(m_local2world_z.y()) = normalised_safe(
        vector3_cross(vector4_to_vector3(m_local2world_z.z()), vector4_to_vector3(m_local2world_z.x()))
      );
      matrix4_premultiply_by_matrix4(m_local2world_z, m_pivot.m_worldSpace);
    }
  }

  void render(Renderer& renderer, const VolumeTest& volume, const Matrix4& pivot2world)
  {
    m_pivot.update(pivot2world, volume.GetModelview(), volume.GetProjection(), volume.GetViewport());
    updateCircleTransforms();

    // temp hack
    UpdateColours();

    renderer.SetState(m_state_outer, Renderer::eWireframeOnly);
    renderer.SetState(m_state_outer, Renderer::eFullMaterials);

    renderer.addRenderable(m_circle_screen, m_pivot.m_viewpointSpace);
    renderer.addRenderable(m_circle_sphere, m_pivot.m_viewpointSpace);

    if(m_circle_x_visible)
    {
      renderer.addRenderable(m_circle_x, m_local2world_x);
    }
    if(m_circle_y_visible)
    {
      renderer.addRenderable(m_circle_y, m_local2world_y);
    }
    if(m_circle_z_visible)
    {
      renderer.addRenderable(m_circle_z, m_local2world_z);
    }
  }
  void testSelect(const View& view, const Matrix4& pivot2world)
  {
    m_pivot.update(pivot2world, view.GetModelview(), view.GetProjection(), view.GetViewport());
    updateCircleTransforms();

    SelectionPool selector;

    {
      {
        Matrix4 local2view(matrix4_multiplied_by_matrix4(view.GetViewMatrix(), m_local2world_x));

#if defined(DEBUG_SELECTION)
        g_render_clipped.construct(view.GetViewMatrix());
#endif

        SelectionIntersection best;
        LineStrip_BestPoint(local2view, m_circle_x.m_vertices.data(), m_circle_x.m_vertices.size(), best);
        selector.addSelectable(best, &m_selectable_x);
      }

      {
        Matrix4 local2view(matrix4_multiplied_by_matrix4(view.GetViewMatrix(), m_local2world_y));

#if defined(DEBUG_SELECTION)
        g_render_clipped.construct(view.GetViewMatrix());
#endif

        SelectionIntersection best;
        LineStrip_BestPoint(local2view, m_circle_y.m_vertices.data(), m_circle_y.m_vertices.size(), best);
        selector.addSelectable(best, &m_selectable_y);
      }

      {
        Matrix4 local2view(matrix4_multiplied_by_matrix4(view.GetViewMatrix(), m_local2world_z));

#if defined(DEBUG_SELECTION)
        g_render_clipped.construct(view.GetViewMatrix());
#endif

        SelectionIntersection best;
        LineStrip_BestPoint(local2view, m_circle_z.m_vertices.data(), m_circle_z.m_vertices.size(), best);
        selector.addSelectable(best, &m_selectable_z);
      }
    }

    {
      Matrix4 local2view(matrix4_multiplied_by_matrix4(view.GetViewMatrix(), m_pivot.m_viewpointSpace));

      {
        SelectionIntersection best;
        LineLoop_BestPoint(local2view, m_circle_screen.m_vertices.data(), m_circle_screen.m_vertices.size(), best);
        selector.addSelectable(best, &m_selectable_screen); 
      }

      {
        SelectionIntersection best;
        Circle_BestPoint(local2view, eClipCullCW, m_circle_sphere.m_vertices.data(), m_circle_sphere.m_vertices.size(), best);
        selector.addSelectable(best, &m_selectable_sphere); 
      }
    }

    m_axis_screen = m_pivot.m_axis_screen;

    if(!selector.failed())
    {
      (*selector.begin()).second->setSelected(true);
    }
  }

  Manipulatable* GetManipulatable()
  {
    if(m_selectable_x.isSelected())
    {
      m_axis.SetAxis(g_vector3_axis_x);
      return &m_axis;
    }
    else if(m_selectable_y.isSelected())
    {
      m_axis.SetAxis(g_vector3_axis_y);
      return &m_axis;
    }
    else if(m_selectable_z.isSelected())
    {
      m_axis.SetAxis(g_vector3_axis_z);
      return &m_axis;
    }
    else if(m_selectable_screen.isSelected())
    {
      m_axis.SetAxis(m_axis_screen);
      return &m_axis;
    }
    else
      return &m_free;
  }

  void setSelected(bool select)
  {
    m_selectable_x.setSelected(select);
    m_selectable_y.setSelected(select);
    m_selectable_z.setSelected(select);
    m_selectable_screen.setSelected(select);
  }
  bool isSelected() const
  {
    return m_selectable_x.isSelected()
      | m_selectable_y.isSelected()
      | m_selectable_z.isSelected()
      | m_selectable_screen.isSelected()
      | m_selectable_sphere.isSelected();
  }
};

Shader* RotateManipulator::m_state_outer;


const float arrowhead_length = 16;
const float arrowhead_radius = 4;

inline void draw_arrowline(const float length, PointVertex* line, const std::size_t axis)
{
  (*line++).vertex = vertex3f_identity;
  (*line).vertex = vertex3f_identity;
  vertex3f_to_array((*line).vertex)[axis] = length - arrowhead_length;
}

template<typename VertexRemap, typename NormalRemap>
inline void draw_arrowhead(const std::size_t segments, const float length, FlatShadedVertex* vertices, VertexRemap, NormalRemap)
{
  std::size_t head_tris = (segments << 3);
  const double head_segment = c_2pi / head_tris;
  for(std::size_t i = 0; i < head_tris; ++i)
  {
    {
      FlatShadedVertex& point = vertices[i*6+0];
      VertexRemap::x(point.vertex) = length - arrowhead_length;
      VertexRemap::y(point.vertex) = arrowhead_radius * static_cast<float>(cos(i * head_segment));
      VertexRemap::z(point.vertex) = arrowhead_radius * static_cast<float>(sin(i * head_segment));
      NormalRemap::x(point.normal) = arrowhead_radius / arrowhead_length;
      NormalRemap::y(point.normal) = static_cast<float>(cos(i * head_segment));
      NormalRemap::z(point.normal) = static_cast<float>(sin(i * head_segment));
    }
    {
      FlatShadedVertex& point = vertices[i*6+1];
      VertexRemap::x(point.vertex) = length;
      VertexRemap::y(point.vertex) = 0;
      VertexRemap::z(point.vertex) = 0;
      NormalRemap::x(point.normal) = arrowhead_radius / arrowhead_length;
      NormalRemap::y(point.normal) = static_cast<float>(cos((i + 0.5) * head_segment));
      NormalRemap::z(point.normal) = static_cast<float>(sin((i + 0.5) * head_segment));
    }
    {
      FlatShadedVertex& point = vertices[i*6+2];
      VertexRemap::x(point.vertex) = length - arrowhead_length;
      VertexRemap::y(point.vertex) = arrowhead_radius * static_cast<float>(cos((i+1) * head_segment));
      VertexRemap::z(point.vertex) = arrowhead_radius * static_cast<float>(sin((i+1) * head_segment));
      NormalRemap::x(point.normal) = arrowhead_radius / arrowhead_length;
      NormalRemap::y(point.normal) = static_cast<float>(cos((i+1) * head_segment));
      NormalRemap::z(point.normal) = static_cast<float>(sin((i+1) * head_segment));
    }

    {
      FlatShadedVertex& point = vertices[i*6+3];
      VertexRemap::x(point.vertex) = length - arrowhead_length;
      VertexRemap::y(point.vertex) = 0;
      VertexRemap::z(point.vertex) = 0;
      NormalRemap::x(point.normal) = -1;
      NormalRemap::y(point.normal) = 0;
      NormalRemap::z(point.normal) = 0;
    }
    {
      FlatShadedVertex& point = vertices[i*6+4];
      VertexRemap::x(point.vertex) = length - arrowhead_length;
      VertexRemap::y(point.vertex) = arrowhead_radius * static_cast<float>(cos(i * head_segment));
      VertexRemap::z(point.vertex) = arrowhead_radius * static_cast<float>(sin(i * head_segment));
      NormalRemap::x(point.normal) = -1;
      NormalRemap::y(point.normal) = 0;
      NormalRemap::z(point.normal) = 0;
    }
    {
      FlatShadedVertex& point = vertices[i*6+5];
      VertexRemap::x(point.vertex) = length - arrowhead_length;
      VertexRemap::y(point.vertex) = arrowhead_radius * static_cast<float>(cos((i+1) * head_segment));
      VertexRemap::z(point.vertex) = arrowhead_radius * static_cast<float>(sin((i+1) * head_segment));
      NormalRemap::x(point.normal) = -1;
      NormalRemap::y(point.normal) = 0;
      NormalRemap::z(point.normal) = 0;
    }
  }
}

template<typename Triple>
class TripleRemapXYZ
{
public:
  static float& x(Triple& triple)
  {
    return triple.x();
  }
  static float& y(Triple& triple)
  {
    return triple.y();
  }
  static float& z(Triple& triple)
  {
    return triple.z();
  }
};

template<typename Triple>
class TripleRemapYZX
{
public:
  static float& x(Triple& triple)
  {
    return triple.y();
  }
  static float& y(Triple& triple)
  {
    return triple.z();
  }
  static float& z(Triple& triple)
  {
    return triple.x();
  }
};

template<typename Triple>
class TripleRemapZXY
{
public:
  static float& x(Triple& triple)
  {
    return triple.z();
  }
  static float& y(Triple& triple)
  {
    return triple.x();
  }
  static float& z(Triple& triple)
  {
    return triple.y();
  }
};

void vector3_print(const Vector3& v)
{
  globalOutputStream() << "( " << v.x() << " " << v.y() << " " << v.z() << " )";
}

class TranslateManipulator : public Manipulator
{
  struct RenderableArrowLine : public OpenGLRenderable
  {
    PointVertex m_line[2];

    RenderableArrowLine()
    {
    }
    void render(RenderStateFlags state) const
    {
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(PointVertex), &m_line[0].colour);
      glVertexPointer(3, GL_FLOAT, sizeof(PointVertex), &m_line[0].vertex);
      glDrawArrays(GL_LINES, 0, 2);
    }
    void setColour(const Colour4b& colour)
    {
      m_line[0].colour = colour;
      m_line[1].colour = colour;
    }
  };
  struct RenderableArrowHead : public OpenGLRenderable
  {
    Array<FlatShadedVertex> m_vertices;

    RenderableArrowHead(std::size_t size)
      : m_vertices(size)
    {
    }
    void render(RenderStateFlags state) const
    {
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(FlatShadedVertex), &m_vertices.data()->colour);
      glVertexPointer(3, GL_FLOAT, sizeof(FlatShadedVertex), &m_vertices.data()->vertex);
      glNormalPointer(GL_FLOAT, sizeof(FlatShadedVertex), &m_vertices.data()->normal);
      glDrawArrays(GL_TRIANGLES, 0, GLsizei(m_vertices.size()));
    }
    void setColour(const Colour4b& colour)
    {
      for(Array<FlatShadedVertex>::iterator i = m_vertices.begin(); i != m_vertices.end(); ++i)
      {
        (*i).colour = colour;
      }
    }
  };
  struct RenderableQuad : public OpenGLRenderable
  {
    PointVertex m_quad[4];
    void render(RenderStateFlags state) const
    {
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(PointVertex), &m_quad[0].colour);
      glVertexPointer(3, GL_FLOAT, sizeof(PointVertex), &m_quad[0].vertex);
      glDrawArrays(GL_LINE_LOOP, 0, 4);
    }
    void setColour(const Colour4b& colour)
    {
      m_quad[0].colour = colour;
      m_quad[1].colour = colour;
      m_quad[2].colour = colour;
      m_quad[3].colour = colour;
    }
  };

  TranslateFree m_free;
  TranslateAxis m_axis;
  RenderableArrowLine m_arrow_x;
  RenderableArrowLine m_arrow_y;
  RenderableArrowLine m_arrow_z;
  RenderableArrowHead m_arrow_head_x;
  RenderableArrowHead m_arrow_head_y;
  RenderableArrowHead m_arrow_head_z;
  RenderableQuad m_quad_screen;
  SelectableBool m_selectable_x;
  SelectableBool m_selectable_y;
  SelectableBool m_selectable_z;
  SelectableBool m_selectable_screen;
  Pivot2World m_pivot;
public:
  static Shader* m_state_wire;
  static Shader* m_state_fill;

  TranslateManipulator(Translatable& translatable, std::size_t segments, float length) :
    m_free(translatable),
    m_axis(translatable),
    m_arrow_head_x(3 * 2 * (segments << 3)),
    m_arrow_head_y(3 * 2 * (segments << 3)),
    m_arrow_head_z(3 * 2 * (segments << 3))
  {
    draw_arrowline(length, m_arrow_x.m_line, 0);
    draw_arrowhead(segments, length, m_arrow_head_x.m_vertices.data(), TripleRemapXYZ<Vertex3f>(), TripleRemapXYZ<Normal3f>());
    draw_arrowline(length, m_arrow_y.m_line, 1);
    draw_arrowhead(segments, length, m_arrow_head_y.m_vertices.data(), TripleRemapYZX<Vertex3f>(), TripleRemapYZX<Normal3f>());
    draw_arrowline(length, m_arrow_z.m_line, 2);
    draw_arrowhead(segments, length, m_arrow_head_z.m_vertices.data(), TripleRemapZXY<Vertex3f>(), TripleRemapZXY<Normal3f>());

    draw_quad(16, m_quad_screen.m_quad);
  }

  void UpdateColours()
  {
    m_arrow_x.setColour(colourSelected(g_colour_x, m_selectable_x.isSelected()));
    m_arrow_head_x.setColour(colourSelected(g_colour_x, m_selectable_x.isSelected()));
    m_arrow_y.setColour(colourSelected(g_colour_y, m_selectable_y.isSelected()));
    m_arrow_head_y.setColour(colourSelected(g_colour_y, m_selectable_y.isSelected()));
    m_arrow_z.setColour(colourSelected(g_colour_z, m_selectable_z.isSelected()));
    m_arrow_head_z.setColour(colourSelected(g_colour_z, m_selectable_z.isSelected()));
    m_quad_screen.setColour(colourSelected(g_colour_screen, m_selectable_screen.isSelected()));
  }

  bool manipulator_show_axis(const Pivot2World& pivot, const Vector3& axis)
  {
    return fabs(vector3_dot(pivot.m_axis_screen, axis)) < 0.95;
  }

  void render(Renderer& renderer, const VolumeTest& volume, const Matrix4& pivot2world)
  {
    m_pivot.update(pivot2world, volume.GetModelview(), volume.GetProjection(), volume.GetViewport());

    // temp hack
    UpdateColours();

    Vector3 x = vector3_normalised(vector4_to_vector3(m_pivot.m_worldSpace.x()));
    bool show_x = manipulator_show_axis(m_pivot, x);

    Vector3 y = vector3_normalised(vector4_to_vector3(m_pivot.m_worldSpace.y()));
    bool show_y = manipulator_show_axis(m_pivot, y);

    Vector3 z = vector3_normalised(vector4_to_vector3(m_pivot.m_worldSpace.z()));
    bool show_z = manipulator_show_axis(m_pivot, z);

    renderer.SetState(m_state_wire, Renderer::eWireframeOnly);
    renderer.SetState(m_state_wire, Renderer::eFullMaterials);

    if(show_x)
    {
      renderer.addRenderable(m_arrow_x, m_pivot.m_worldSpace);
    }
    if(show_y)
    {
      renderer.addRenderable(m_arrow_y, m_pivot.m_worldSpace);
    }
    if(show_z)
    {
      renderer.addRenderable(m_arrow_z, m_pivot.m_worldSpace);
    }

    renderer.addRenderable(m_quad_screen, m_pivot.m_viewplaneSpace);

    renderer.SetState(m_state_fill, Renderer::eWireframeOnly);
    renderer.SetState(m_state_fill, Renderer::eFullMaterials);

    if(show_x)
    {
      renderer.addRenderable(m_arrow_head_x, m_pivot.m_worldSpace);
    }
    if(show_y)
    {
      renderer.addRenderable(m_arrow_head_y, m_pivot.m_worldSpace);
    }
    if(show_z)
    {
      renderer.addRenderable(m_arrow_head_z, m_pivot.m_worldSpace);
    }
  }
  void testSelect(const View& view, const Matrix4& pivot2world)
  {
    m_pivot.update(pivot2world, view.GetModelview(), view.GetProjection(), view.GetViewport());

    SelectionPool selector;

    Vector3 x = vector3_normalised(vector4_to_vector3(m_pivot.m_worldSpace.x()));
    bool show_x = manipulator_show_axis(m_pivot, x);

    Vector3 y = vector3_normalised(vector4_to_vector3(m_pivot.m_worldSpace.y()));
    bool show_y = manipulator_show_axis(m_pivot, y);

    Vector3 z = vector3_normalised(vector4_to_vector3(m_pivot.m_worldSpace.z()));
    bool show_z = manipulator_show_axis(m_pivot, z);

    {
      Matrix4 local2view(matrix4_multiplied_by_matrix4(view.GetViewMatrix(), m_pivot.m_viewpointSpace));

      {
        SelectionIntersection best;
        Quad_BestPoint(local2view, eClipCullCW, m_quad_screen.m_quad, best);
        if(best.valid())
        {
          best = SelectionIntersection(0, 0);
          selector.addSelectable(best, &m_selectable_screen);
        }
      }
    }

    {
      Matrix4 local2view(matrix4_multiplied_by_matrix4(view.GetViewMatrix(), m_pivot.m_worldSpace));

#if defined(DEBUG_SELECTION)
      g_render_clipped.construct(view.GetViewMatrix());
#endif

      if(show_x)
      {
        SelectionIntersection best;
        Line_BestPoint(local2view, m_arrow_x.m_line, best);
        Triangles_BestPoint(local2view, eClipCullCW, m_arrow_head_x.m_vertices.begin(), m_arrow_head_x.m_vertices.end(), best);
        selector.addSelectable(best, &m_selectable_x);
      }

      if(show_y)
      {
        SelectionIntersection best;
        Line_BestPoint(local2view, m_arrow_y.m_line, best);
        Triangles_BestPoint(local2view, eClipCullCW, m_arrow_head_y.m_vertices.begin(), m_arrow_head_y.m_vertices.end(), best);
        selector.addSelectable(best, &m_selectable_y);
      }

      if(show_z)
      {
        SelectionIntersection best;
        Line_BestPoint(local2view, m_arrow_z.m_line, best);
        Triangles_BestPoint(local2view, eClipCullCW, m_arrow_head_z.m_vertices.begin(), m_arrow_head_z.m_vertices.end(), best);
        selector.addSelectable(best, &m_selectable_z);
      }
    }

    if(!selector.failed())
    {
      (*selector.begin()).second->setSelected(true);
    }
  }

  Manipulatable* GetManipulatable()
  {
    if(m_selectable_x.isSelected())
    {
      m_axis.SetAxis(g_vector3_axis_x);
      return &m_axis;
    }
    else if(m_selectable_y.isSelected())
    {
      m_axis.SetAxis(g_vector3_axis_y);
      return &m_axis;
    }
    else if(m_selectable_z.isSelected())
    {
      m_axis.SetAxis(g_vector3_axis_z);
      return &m_axis;
    }
    else
    {
      return &m_free;
    }
  }

  void setSelected(bool select)
  {
    m_selectable_x.setSelected(select);
    m_selectable_y.setSelected(select);
    m_selectable_z.setSelected(select);
    m_selectable_screen.setSelected(select);
  }
  bool isSelected() const
  {
    return m_selectable_x.isSelected()
      | m_selectable_y.isSelected()
      | m_selectable_z.isSelected()
      | m_selectable_screen.isSelected();
  }
};

Shader* TranslateManipulator::m_state_wire;
Shader* TranslateManipulator::m_state_fill;

class ScaleManipulator : public Manipulator
{
  struct RenderableArrow : public OpenGLRenderable
  {
    PointVertex m_line[2];

    void render(RenderStateFlags state) const
    {
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(PointVertex), &m_line[0].colour);
      glVertexPointer(3, GL_FLOAT, sizeof(PointVertex), &m_line[0].vertex);
      glDrawArrays(GL_LINES, 0, 2);
    }
    void setColour(const Colour4b& colour)
    {
      m_line[0].colour = colour;
      m_line[1].colour = colour;
    }
  };
  struct RenderableQuad : public OpenGLRenderable
  {
    PointVertex m_quad[4];
    void render(RenderStateFlags state) const
    {
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(PointVertex), &m_quad[0].colour);
      glVertexPointer(3, GL_FLOAT, sizeof(PointVertex), &m_quad[0].vertex);
      glDrawArrays(GL_QUADS, 0, 4);
    }
    void setColour(const Colour4b& colour)
    {
      m_quad[0].colour = colour;
      m_quad[1].colour = colour;
      m_quad[2].colour = colour;
      m_quad[3].colour = colour;
    }
  };

  ScaleFree m_free;
  ScaleAxis m_axis;
  RenderableArrow m_arrow_x;
  RenderableArrow m_arrow_y;
  RenderableArrow m_arrow_z;
  RenderableQuad m_quad_screen;
  SelectableBool m_selectable_x;
  SelectableBool m_selectable_y;
  SelectableBool m_selectable_z;
  SelectableBool m_selectable_screen;
  Pivot2World m_pivot;
public:
  ScaleManipulator(Scalable& scalable, std::size_t segments, float length) :
    m_free(scalable),
    m_axis(scalable)
  {
    draw_arrowline(length, m_arrow_x.m_line, 0);
    draw_arrowline(length, m_arrow_y.m_line, 1);
    draw_arrowline(length, m_arrow_z.m_line, 2);

    draw_quad(16, m_quad_screen.m_quad);
  }

  Pivot2World& getPivot()
  {
    return m_pivot;
  }

  void UpdateColours()
  {
    m_arrow_x.setColour(colourSelected(g_colour_x, m_selectable_x.isSelected()));
    m_arrow_y.setColour(colourSelected(g_colour_y, m_selectable_y.isSelected()));
    m_arrow_z.setColour(colourSelected(g_colour_z, m_selectable_z.isSelected()));
    m_quad_screen.setColour(colourSelected(g_colour_screen, m_selectable_screen.isSelected()));
  }

  void render(Renderer& renderer, const VolumeTest& volume, const Matrix4& pivot2world)
  {
    m_pivot.update(pivot2world, volume.GetModelview(), volume.GetProjection(), volume.GetViewport());

    // temp hack
    UpdateColours();

    renderer.addRenderable(m_arrow_x, m_pivot.m_worldSpace);
    renderer.addRenderable(m_arrow_y, m_pivot.m_worldSpace);
    renderer.addRenderable(m_arrow_z, m_pivot.m_worldSpace);

    renderer.addRenderable(m_quad_screen, m_pivot.m_viewpointSpace);
  }
  void testSelect(const View& view, const Matrix4& pivot2world)
  {
    m_pivot.update(pivot2world, view.GetModelview(), view.GetProjection(), view.GetViewport());

    SelectionPool selector;

    {
      Matrix4 local2view(matrix4_multiplied_by_matrix4(view.GetViewMatrix(), m_pivot.m_worldSpace));

#if defined(DEBUG_SELECTION)
      g_render_clipped.construct(view.GetViewMatrix());
#endif

    {
        SelectionIntersection best;
        Line_BestPoint(local2view, m_arrow_x.m_line, best);
        selector.addSelectable(best, &m_selectable_x);
      }

      {
        SelectionIntersection best;
        Line_BestPoint(local2view, m_arrow_y.m_line, best);
        selector.addSelectable(best, &m_selectable_y);
      }

      {
        SelectionIntersection best;
        Line_BestPoint(local2view, m_arrow_z.m_line, best);
        selector.addSelectable(best, &m_selectable_z);
      }
    }

    {
      Matrix4 local2view(matrix4_multiplied_by_matrix4(view.GetViewMatrix(), m_pivot.m_viewpointSpace));

      {
        SelectionIntersection best;
        Quad_BestPoint(local2view, eClipCullCW, m_quad_screen.m_quad, best);
        selector.addSelectable(best, &m_selectable_screen);
      }
    }

    if(!selector.failed())
    {
      (*selector.begin()).second->setSelected(true);
    }
  }

  Manipulatable* GetManipulatable()
  {
    if(m_selectable_x.isSelected())
    {
      m_axis.SetAxis(g_vector3_axis_x);
      return &m_axis;
    }
    else if(m_selectable_y.isSelected())
    {
      m_axis.SetAxis(g_vector3_axis_y);
      return &m_axis;
    }
    else if(m_selectable_z.isSelected())
    {
      m_axis.SetAxis(g_vector3_axis_z);
      return &m_axis;
    }
    else
      return &m_free;
  }

  void setSelected(bool select)
  {
    m_selectable_x.setSelected(select);
    m_selectable_y.setSelected(select);
    m_selectable_z.setSelected(select);
    m_selectable_screen.setSelected(select);
  }
  bool isSelected() const
  {
    return m_selectable_x.isSelected()
      | m_selectable_y.isSelected()
      | m_selectable_z.isSelected()
      | m_selectable_screen.isSelected();
  }
};


inline PlaneSelectable* Instance_getPlaneSelectable(scene::Instance& instance)
{
  return InstanceTypeCast<PlaneSelectable>::cast(instance);
}

class PlaneSelectableSelectPlanes : public scene::Graph::Walker
{
  Selector& m_selector;
  SelectionTest& m_test;
  PlaneCallback m_selectedPlaneCallback;
public:
  PlaneSelectableSelectPlanes(Selector& selector, SelectionTest& test, const PlaneCallback& selectedPlaneCallback)
    : m_selector(selector), m_test(test), m_selectedPlaneCallback(selectedPlaneCallback)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    if(path.top().get().visible())
    {
      Selectable* selectable = Instance_getSelectable(instance);
      if(selectable != 0 && selectable->isSelected())
      {
        PlaneSelectable* planeSelectable = Instance_getPlaneSelectable(instance);
        if(planeSelectable != 0)
        {
          planeSelectable->selectPlanes(m_selector, m_test, m_selectedPlaneCallback);
        }
      }
    }
    return true; 
  }
};

class PlaneSelectableSelectReversedPlanes : public scene::Graph::Walker
{
  Selector& m_selector;
  const SelectedPlanes& m_selectedPlanes;
public:
  PlaneSelectableSelectReversedPlanes(Selector& selector, const SelectedPlanes& selectedPlanes)
    : m_selector(selector), m_selectedPlanes(selectedPlanes)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    if(path.top().get().visible())
    {
      Selectable* selectable = Instance_getSelectable(instance);
      if(selectable != 0 && selectable->isSelected())
      {
        PlaneSelectable* planeSelectable = Instance_getPlaneSelectable(instance);
        if(planeSelectable != 0)
        {
          planeSelectable->selectReversedPlanes(m_selector, m_selectedPlanes);
        }
      }
    }
    return true; 
  }
};

void Scene_forEachPlaneSelectable_selectPlanes(scene::Graph& graph, Selector& selector, SelectionTest& test, const PlaneCallback& selectedPlaneCallback)
{
  graph.traverse(PlaneSelectableSelectPlanes(selector, test, selectedPlaneCallback));
}

void Scene_forEachPlaneSelectable_selectReversedPlanes(scene::Graph& graph, Selector& selector, const SelectedPlanes& selectedPlanes)
{
  graph.traverse(PlaneSelectableSelectReversedPlanes(selector, selectedPlanes));
}


class PlaneLess
{
public:
  bool operator()(const Plane3& plane, const Plane3& other) const
  {
    if(plane.a < other.a)
    {
      return true;
    }
    if(other.a < plane.a)
    {
      return false;
    }

    if(plane.b < other.b)
    {
      return true;
    }
    if(other.b < plane.b)
    {
      return false;
    }

    if(plane.c < other.c)
    {
      return true;
    }
    if(other.c < plane.c)
    {
      return false;
    }

    if(plane.d < other.d)
    {
      return true;
    }
    if(other.d < plane.d)
    {
      return false;
    }

    return false;
  }
};

typedef std::set<Plane3, PlaneLess> PlaneSet;

inline void PlaneSet_insert(PlaneSet& self, const Plane3& plane)
{
  self.insert(plane);
}

inline bool PlaneSet_contains(const PlaneSet& self, const Plane3& plane)
{
  return self.find(plane) != self.end();
}


class SelectedPlaneSet : public SelectedPlanes
{
  PlaneSet m_selectedPlanes;
public:
  bool empty() const
  {
    return m_selectedPlanes.empty();
  }

  void insert(const Plane3& plane)
  {
    PlaneSet_insert(m_selectedPlanes, plane);
  }
  bool contains(const Plane3& plane) const
  {
    return PlaneSet_contains(m_selectedPlanes, plane);
  }
  typedef MemberCaller1<SelectedPlaneSet, const Plane3&, &SelectedPlaneSet::insert> InsertCaller;
};


bool Scene_forEachPlaneSelectable_selectPlanes(scene::Graph& graph, Selector& selector, SelectionTest& test)
{
  SelectedPlaneSet selectedPlanes;

  Scene_forEachPlaneSelectable_selectPlanes(graph, selector, test, SelectedPlaneSet::InsertCaller(selectedPlanes));
  Scene_forEachPlaneSelectable_selectReversedPlanes(graph, selector, selectedPlanes);

  return !selectedPlanes.empty();
}

void Scene_Translate_Component_Selected(scene::Graph& graph, const Vector3& translation);
void Scene_Translate_Selected(scene::Graph& graph, const Vector3& translation);
void Scene_TestSelect_Primitive(Selector& selector, SelectionTest& test, const VolumeTest& volume);
void Scene_TestSelect_Component(Selector& selector, SelectionTest& test, const VolumeTest& volume, SelectionSystem::EComponentMode componentMode);
void Scene_TestSelect_Component_Selected(Selector& selector, SelectionTest& test, const VolumeTest& volume, SelectionSystem::EComponentMode componentMode);
void Scene_SelectAll_Component(bool select, SelectionSystem::EComponentMode componentMode);

class ResizeTranslatable : public Translatable
{
  void translate(const Vector3& translation)
  {
    Scene_Translate_Component_Selected(GlobalSceneGraph(), translation);
  }
};

class DragTranslatable : public Translatable
{
  void translate(const Vector3& translation)
  {
    if(GlobalSelectionSystem().Mode() == SelectionSystem::eComponent)
    {
      Scene_Translate_Component_Selected(GlobalSceneGraph(), translation);
    }
    else
    {
      Scene_Translate_Selected(GlobalSceneGraph(), translation);
    }
  }
};

class SelectionVolume : public SelectionTest
{
  Matrix4 m_local2view;
  const View& m_view;
  clipcull_t m_cull;
  Vector3 m_near;
  Vector3 m_far;
public:
  SelectionVolume(const View& view)
    : m_view(view)
  {
  }

  const VolumeTest& getVolume() const
  {
    return m_view;
  }

  const Vector3& getNear() const
  {
    return m_near;
  }
  const Vector3& getFar() const
  {
    return m_far;
  }

  void BeginMesh(const Matrix4& localToWorld, bool twoSided)
  {
    m_local2view = matrix4_multiplied_by_matrix4(m_view.GetViewMatrix(), localToWorld);

    // Cull back-facing polygons based on winding being clockwise or counter-clockwise.
    // Don't cull if the view is wireframe and the polygons are two-sided.
    m_cull = twoSided && !m_view.fill() ? eClipCullNone : (matrix4_handedness(localToWorld) == MATRIX4_RIGHTHANDED) ? eClipCullCW : eClipCullCCW;

    {
      Matrix4 screen2world(matrix4_full_inverse(m_local2view));

      m_near = vector4_projected(
        matrix4_transformed_vector4(
          screen2world,
          Vector4(0, 0, -1, 1)
        )
      );

      m_far = vector4_projected(
        matrix4_transformed_vector4(
          screen2world,
          Vector4(0, 0, 1, 1)
        )
      );
    }

#if defined(DEBUG_SELECTION)
    g_render_clipped.construct(m_view.GetViewMatrix());
#endif
  }
  void TestPoint(const Vector3& point, SelectionIntersection& best)
  {
    Vector4 clipped;
    if(matrix4_clip_point(m_local2view, point, clipped) == c_CLIP_PASS)
    {
      best = select_point_from_clipped(clipped);
    }
  }
  void TestPolygon(const VertexPointer& vertices, std::size_t count, SelectionIntersection& best)
  {
    Vector4 clipped[9];
    for(std::size_t i=0; i+2<count; ++i)
    {
      BestPoint(
        matrix4_clip_triangle(
          m_local2view,
          reinterpret_cast<const Vector3&>(vertices[0]),
          reinterpret_cast<const Vector3&>(vertices[i+1]),
          reinterpret_cast<const Vector3&>(vertices[i+2]),
          clipped
        ),
        clipped,
        best,
        m_cull
      );
    }
  }
  void TestLineLoop(const VertexPointer& vertices, std::size_t count, SelectionIntersection& best)
  {
    if(count == 0)
      return;
    Vector4 clipped[9];
    for(VertexPointer::iterator i = vertices.begin(), end = i + count, prev = i + (count-1); i != end; prev = i, ++i)
    {
      BestPoint(
        matrix4_clip_line(
          m_local2view,
          reinterpret_cast<const Vector3&>((*prev)),
          reinterpret_cast<const Vector3&>((*i)),
          clipped
        ),
        clipped,
        best,
        m_cull
      );
    }
  }
  void TestLineStrip(const VertexPointer& vertices, std::size_t count, SelectionIntersection& best)
  {
    if(count == 0)
      return;
    Vector4 clipped[9];
    for(VertexPointer::iterator i = vertices.begin(), end = i + count, next = i + 1; next != end; i = next, ++next)
    {
      BestPoint(
        matrix4_clip_line(
          m_local2view,
          reinterpret_cast<const Vector3&>((*i)),
          reinterpret_cast<const Vector3&>((*next)),
          clipped
        ),
        clipped,
        best,
        m_cull
      );
    }
  }
  void TestLines(const VertexPointer& vertices, std::size_t count, SelectionIntersection& best)
  {
    if(count == 0)
      return;
    Vector4 clipped[9];
    for(VertexPointer::iterator i = vertices.begin(), end = i + count; i != end; i += 2)
    {
      BestPoint(
        matrix4_clip_line(
          m_local2view,
          reinterpret_cast<const Vector3&>((*i)),
          reinterpret_cast<const Vector3&>((*(i+1))),
          clipped
        ),
        clipped,
        best,
        m_cull
      );
    }
  }
  void TestTriangles(const VertexPointer& vertices, const IndexPointer& indices, SelectionIntersection& best)
  {
    Vector4 clipped[9];
    for(IndexPointer::iterator i(indices.begin()); i != indices.end(); i += 3)
    {
      BestPoint(
        matrix4_clip_triangle(
          m_local2view,
          reinterpret_cast<const Vector3&>(vertices[*i]),
          reinterpret_cast<const Vector3&>(vertices[*(i+1)]),
          reinterpret_cast<const Vector3&>(vertices[*(i+2)]),
          clipped
        ),
        clipped,
        best,
        m_cull
      );
    }
  }
  void TestQuads(const VertexPointer& vertices, const IndexPointer& indices, SelectionIntersection& best)
  {
    Vector4 clipped[9];
    for(IndexPointer::iterator i(indices.begin()); i != indices.end(); i += 4)
    {
      BestPoint(
        matrix4_clip_triangle(
          m_local2view,
          reinterpret_cast<const Vector3&>(vertices[*i]),
          reinterpret_cast<const Vector3&>(vertices[*(i+1)]),
          reinterpret_cast<const Vector3&>(vertices[*(i+3)]),
          clipped
        ),
        clipped,
        best,
        m_cull
      );
	    BestPoint(
        matrix4_clip_triangle(
          m_local2view,
          reinterpret_cast<const Vector3&>(vertices[*(i+1)]),
          reinterpret_cast<const Vector3&>(vertices[*(i+2)]),
          reinterpret_cast<const Vector3&>(vertices[*(i+3)]),
          clipped
        ),
        clipped,
        best,
        m_cull
      );
    }
  }
  void TestQuadStrip(const VertexPointer& vertices, const IndexPointer& indices, SelectionIntersection& best)
  {
    Vector4 clipped[9];
    for(IndexPointer::iterator i(indices.begin()); i+2 != indices.end(); i += 2)
    {
      BestPoint(
        matrix4_clip_triangle(
          m_local2view,
          reinterpret_cast<const Vector3&>(vertices[*i]),
          reinterpret_cast<const Vector3&>(vertices[*(i+1)]),
          reinterpret_cast<const Vector3&>(vertices[*(i+2)]),
          clipped
        ),
        clipped,
        best,
        m_cull
      );
      BestPoint(
        matrix4_clip_triangle(
          m_local2view,
          reinterpret_cast<const Vector3&>(vertices[*(i+2)]),
          reinterpret_cast<const Vector3&>(vertices[*(i+1)]),
          reinterpret_cast<const Vector3&>(vertices[*(i+3)]),
          clipped
        ),
        clipped,
        best,
        m_cull
      );
    }
  }
};

class SelectionCounter
{
public:
  typedef const Selectable& first_argument_type;

  SelectionCounter(const SelectionChangeCallback& onchanged)
    : m_count(0), m_onchanged(onchanged)
  {
  }
  void operator()(const Selectable& selectable)
  {
    if(selectable.isSelected())
    {
      ++m_count;
    }
    else
    {
      ASSERT_MESSAGE(m_count != 0, "selection counter underflow");
      --m_count;
    }

    m_onchanged(selectable);
  }
  bool empty() const
  {
    return m_count == 0;
  }
  std::size_t size() const
  {
    return m_count;
  }
private:
  std::size_t m_count;
  SelectionChangeCallback m_onchanged;
};

inline void ConstructSelectionTest(View& view, const rect_t selection_box)
{
  view.EnableScissor(selection_box.min[0], selection_box.max[0], selection_box.min[1], selection_box.max[1]);
}

inline const rect_t SelectionBoxForPoint(const float device_point[2], const float device_epsilon[2])
{
  rect_t selection_box;
  selection_box.min[0] = device_point[0] - device_epsilon[0];
  selection_box.min[1] = device_point[1] - device_epsilon[1];
  selection_box.max[0] = device_point[0] + device_epsilon[0];
  selection_box.max[1] = device_point[1] + device_epsilon[1];
  return selection_box;
}

inline const rect_t SelectionBoxForArea(const float device_point[2], const float device_delta[2])
{
  rect_t selection_box;
  selection_box.min[0] = (device_delta[0] < 0) ? (device_point[0] + device_delta[0]) : (device_point[0]);
  selection_box.min[1] = (device_delta[1] < 0) ? (device_point[1] + device_delta[1]) : (device_point[1]);
  selection_box.max[0] = (device_delta[0] > 0) ? (device_point[0] + device_delta[0]) : (device_point[0]);
  selection_box.max[1] = (device_delta[1] > 0) ? (device_point[1] + device_delta[1]) : (device_point[1]);
  return selection_box;
}

Quaternion construct_local_rotation(const Quaternion& world, const Quaternion& localToWorld)
{
  return quaternion_normalised(quaternion_multiplied_by_quaternion(
    quaternion_normalised(quaternion_multiplied_by_quaternion(
      quaternion_inverse(localToWorld),
      world
    )),
    localToWorld
  ));
}

inline void matrix4_assign_rotation(Matrix4& matrix, const Matrix4& other)
{
  matrix[0] = other[0];
  matrix[1] = other[1];
  matrix[2] = other[2];
  matrix[4] = other[4];
  matrix[5] = other[5];
  matrix[6] = other[6];
  matrix[8] = other[8];
  matrix[9] = other[9];
  matrix[10] = other[10];
}

void matrix4_assign_rotation_for_pivot(Matrix4& matrix, scene::Instance& instance)
{
  Editable* editable = Node_getEditable(instance.path().top());
  if(editable != 0)
  {
    matrix4_assign_rotation(matrix, matrix4_multiplied_by_matrix4(instance.localToWorld(), editable->getLocalPivot()));
  }
  else
  {
    matrix4_assign_rotation(matrix, instance.localToWorld());
  }
}

inline bool Instance_isSelectedComponents(scene::Instance& instance)
{
  ComponentSelectionTestable* componentSelectionTestable = Instance_getComponentSelectionTestable(instance);
  return componentSelectionTestable != 0
    && componentSelectionTestable->isSelectedComponents();
}

class TranslateSelected : public SelectionSystem::Visitor
{
  const Vector3& m_translate;
public:
  TranslateSelected(const Vector3& translate)
    : m_translate(translate)
  {
  }
  void visit(scene::Instance& instance) const
  {
    Transformable* transform = Instance_getTransformable(instance);
    if(transform != 0)
    {
      transform->setType(TRANSFORM_PRIMITIVE);
      transform->setTranslation(m_translate);
    }
  }
};

void Scene_Translate_Selected(scene::Graph& graph, const Vector3& translation)
{
  if(GlobalSelectionSystem().countSelected() != 0)
  {
    GlobalSelectionSystem().foreachSelected(TranslateSelected(translation));
  }
}

Vector3 get_local_pivot(const Vector3& world_pivot, const Matrix4& localToWorld)
{
  return Vector3(
    matrix4_transformed_point(
      matrix4_full_inverse(localToWorld),
      world_pivot
    )
  );
}

void translation_for_pivoted_rotation(Vector3& parent_translation, const Quaternion& local_rotation, const Vector3& world_pivot, const Matrix4& localToWorld, const Matrix4& localToParent)
{
  Vector3 local_pivot(get_local_pivot(world_pivot, localToWorld));

  Vector3 translation(
    vector3_added(
      local_pivot,
      matrix4_transformed_point(
        matrix4_rotation_for_quaternion_quantised(local_rotation),
        vector3_negated(local_pivot)
      )
    )
  );

  //globalOutputStream() << "translation: " << translation << "\n";

  translation_local2object(parent_translation, translation, localToParent);

  //globalOutputStream() << "parent_translation: " << parent_translation << "\n";
}

void translation_for_pivoted_scale(Vector3& parent_translation, const Vector3& local_scale, const Vector3& world_pivot, const Matrix4& localToWorld, const Matrix4& localToParent)
{
  Vector3 local_pivot(get_local_pivot(world_pivot, localToWorld));

  Vector3 translation(
    vector3_added(
      local_pivot,
      vector3_scaled(
        vector3_negated(local_pivot),
        local_scale
      )
    )
  );

  translation_local2object(parent_translation, translation, localToParent);
}

class rotate_selected : public SelectionSystem::Visitor
{
  const Quaternion& m_rotate;
  const Vector3& m_world_pivot;
public:
  rotate_selected(const Quaternion& rotation, const Vector3& world_pivot)
    : m_rotate(rotation), m_world_pivot(world_pivot)
  {
  }
  void visit(scene::Instance& instance) const
  {
    TransformNode* transformNode = Node_getTransformNode(instance.path().top());
    if(transformNode != 0)
    {
      Transformable* transform = Instance_getTransformable(instance);
      if(transform != 0)
      {
        transform->setType(TRANSFORM_PRIMITIVE);
        transform->setScale(c_scale_identity);
        transform->setTranslation(c_translation_identity);

        transform->setType(TRANSFORM_PRIMITIVE);
        transform->setRotation(m_rotate);

        {
          Editable* editable = Node_getEditable(instance.path().top());
          const Matrix4& localPivot = editable != 0 ? editable->getLocalPivot() : g_matrix4_identity;

          Vector3 parent_translation;
          translation_for_pivoted_rotation(
            parent_translation,
            m_rotate,
            m_world_pivot,
            matrix4_multiplied_by_matrix4(instance.localToWorld(), localPivot),
            matrix4_multiplied_by_matrix4(transformNode->localToParent(), localPivot)
          );

          transform->setTranslation(parent_translation);
        }
      } 
    }
  }
};

void Scene_Rotate_Selected(scene::Graph& graph, const Quaternion& rotation, const Vector3& world_pivot)
{
  if(GlobalSelectionSystem().countSelected() != 0)
  {
    GlobalSelectionSystem().foreachSelected(rotate_selected(rotation, world_pivot));
  }
}

class scale_selected : public SelectionSystem::Visitor
{
  const Vector3& m_scale;
  const Vector3& m_world_pivot;
public:
  scale_selected(const Vector3& scaling, const Vector3& world_pivot)
    : m_scale(scaling), m_world_pivot(world_pivot)
  {
  }
  void visit(scene::Instance& instance) const
  {
    TransformNode* transformNode = Node_getTransformNode(instance.path().top());
    if(transformNode != 0)
    {
      Transformable* transform = Instance_getTransformable(instance);
      if(transform != 0)
      {
        transform->setType(TRANSFORM_PRIMITIVE);
        transform->setScale(c_scale_identity);
        transform->setTranslation(c_translation_identity);

        transform->setType(TRANSFORM_PRIMITIVE);
        transform->setScale(m_scale);
        {
          Editable* editable = Node_getEditable(instance.path().top());
          const Matrix4& localPivot = editable != 0 ? editable->getLocalPivot() : g_matrix4_identity;
    
          Vector3 parent_translation;
          translation_for_pivoted_scale(
            parent_translation,
            m_scale,
            m_world_pivot,
            matrix4_multiplied_by_matrix4(instance.localToWorld(), localPivot),
            matrix4_multiplied_by_matrix4(transformNode->localToParent(), localPivot)
          );

          transform->setTranslation(parent_translation);
        }
      }
    }
  }
};

void Scene_Scale_Selected(scene::Graph& graph, const Vector3& scaling, const Vector3& world_pivot)
{
  if(GlobalSelectionSystem().countSelected() != 0)
  {
    GlobalSelectionSystem().foreachSelected(scale_selected(scaling, world_pivot));
  }
}


class translate_component_selected : public SelectionSystem::Visitor
{
  const Vector3& m_translate;
public:
  translate_component_selected(const Vector3& translate)
    : m_translate(translate)
  {
  }
  void visit(scene::Instance& instance) const
  {
    Transformable* transform = Instance_getTransformable(instance);
    if(transform != 0)
    {
      transform->setType(TRANSFORM_COMPONENT);
      transform->setTranslation(m_translate);
    }
  }
};

void Scene_Translate_Component_Selected(scene::Graph& graph, const Vector3& translation)
{
  if(GlobalSelectionSystem().countSelected() != 0)
  {
    GlobalSelectionSystem().foreachSelectedComponent(translate_component_selected(translation));
  }
}

class rotate_component_selected : public SelectionSystem::Visitor
{
  const Quaternion& m_rotate;
  const Vector3& m_world_pivot;
public:
  rotate_component_selected(const Quaternion& rotation, const Vector3& world_pivot)
    : m_rotate(rotation), m_world_pivot(world_pivot)
  {
  }
  void visit(scene::Instance& instance) const
  {
    Transformable* transform = Instance_getTransformable(instance);
    if(transform != 0)
    {
      Vector3 parent_translation;
      translation_for_pivoted_rotation(parent_translation, m_rotate, m_world_pivot, instance.localToWorld(), Node_getTransformNode(instance.path().top())->localToParent());

      transform->setType(TRANSFORM_COMPONENT);
      transform->setRotation(m_rotate);
      transform->setTranslation(parent_translation);
    }
  }
};

void Scene_Rotate_Component_Selected(scene::Graph& graph, const Quaternion& rotation, const Vector3& world_pivot)
{
  if(GlobalSelectionSystem().countSelectedComponents() != 0)
  {
    GlobalSelectionSystem().foreachSelectedComponent(rotate_component_selected(rotation, world_pivot));
  }
}

class scale_component_selected : public SelectionSystem::Visitor
{
  const Vector3& m_scale;
  const Vector3& m_world_pivot;
public:
  scale_component_selected(const Vector3& scaling, const Vector3& world_pivot)
    : m_scale(scaling), m_world_pivot(world_pivot)
  {
  }
  void visit(scene::Instance& instance) const
  {
    Transformable* transform = Instance_getTransformable(instance);
    if(transform != 0)
    {
      Vector3 parent_translation;
      translation_for_pivoted_scale(parent_translation, m_scale, m_world_pivot, instance.localToWorld(), Node_getTransformNode(instance.path().top())->localToParent());

      transform->setType(TRANSFORM_COMPONENT);
      transform->setScale(m_scale);
      transform->setTranslation(parent_translation);
    }
  }
};

void Scene_Scale_Component_Selected(scene::Graph& graph, const Vector3& scaling, const Vector3& world_pivot)
{
  if(GlobalSelectionSystem().countSelectedComponents() != 0)
  {
    GlobalSelectionSystem().foreachSelectedComponent(scale_component_selected(scaling, world_pivot));
  }
}


class BooleanSelector : public Selector
{
  bool m_selected;
  SelectionIntersection m_intersection;
  Selectable* m_selectable;
public:
  BooleanSelector() : m_selected(false)
  {
  }

  void pushSelectable(Selectable& selectable)
  {
    m_intersection = SelectionIntersection();
    m_selectable = &selectable;
  }
  void popSelectable()
  {
    if(m_intersection.valid())
    {
      m_selected = true;
    }
    m_intersection = SelectionIntersection();
  }
  void addIntersection(const SelectionIntersection& intersection)
  {
    if(m_selectable->isSelected())
    {
      assign_if_closer(m_intersection, intersection);
    }
  }

  bool isSelected()
  {
    return m_selected;
  }
};

class BestSelector : public Selector
{
  SelectionIntersection m_intersection;
  Selectable* m_selectable;
  SelectionIntersection m_bestIntersection;
  std::list<Selectable*> m_bestSelectable;
public:
  BestSelector() : m_bestIntersection(SelectionIntersection()), m_bestSelectable(0)
  {
  }

  void pushSelectable(Selectable& selectable)
  {
    m_intersection = SelectionIntersection();
    m_selectable = &selectable;
  }
  void popSelectable()
  {
    if(m_intersection.equalEpsilon(m_bestIntersection, 0.25f, 0.001f))
    {
      m_bestSelectable.push_back(m_selectable);
      m_bestIntersection = m_intersection;
    }
    else if(m_intersection < m_bestIntersection)
    {
      m_bestSelectable.clear();
      m_bestSelectable.push_back(m_selectable);
      m_bestIntersection = m_intersection;
    }
    m_intersection = SelectionIntersection();
  }
  void addIntersection(const SelectionIntersection& intersection)
  {
    assign_if_closer(m_intersection, intersection);
  }

  std::list<Selectable*>& best()
  {
    return m_bestSelectable;
  }
};

class DragManipulator : public Manipulator
{
  TranslateFree m_freeResize;
  TranslateFree m_freeDrag;
  ResizeTranslatable m_resize;
  DragTranslatable m_drag;
  SelectableBool m_dragSelectable;
public:

  bool m_selected;

  DragManipulator() : m_freeResize(m_resize), m_freeDrag(m_drag), m_selected(false)
  {
  }

  Manipulatable* GetManipulatable()
  {
    return m_dragSelectable.isSelected() ? &m_freeDrag : &m_freeResize;
  }

  void testSelect(const View& view, const Matrix4& pivot2world)
  {
    SelectionPool selector;

    SelectionVolume test(view);

    if(GlobalSelectionSystem().Mode() == SelectionSystem::ePrimitive)
    {
      BooleanSelector booleanSelector;

      Scene_TestSelect_Primitive(booleanSelector, test, view);

      if(booleanSelector.isSelected())
      {
        selector.addSelectable(SelectionIntersection(0, 0), &m_dragSelectable);
        m_selected = false;
      }
      else
      {
        m_selected = Scene_forEachPlaneSelectable_selectPlanes(GlobalSceneGraph(), selector, test);
      }
    }
    else
    {
      BestSelector bestSelector;
      Scene_TestSelect_Component_Selected(bestSelector, test, view, GlobalSelectionSystem().ComponentMode());
      for(std::list<Selectable*>::iterator i = bestSelector.best().begin(); i != bestSelector.best().end(); ++i)
      {
        if(!(*i)->isSelected())
        {
          GlobalSelectionSystem().setSelectedAllComponents(false);
        }
        m_selected = false;
        selector.addSelectable(SelectionIntersection(0, 0), (*i));
        m_dragSelectable.setSelected(true);
      }
    }

    for(SelectionPool::iterator i = selector.begin(); i != selector.end(); ++i)
    {
      (*i).second->setSelected(true);
    }
  }
  
  void setSelected(bool select)
  {
    m_selected = select;
    m_dragSelectable.setSelected(select);
  }
  bool isSelected() const
  {
    return m_selected || m_dragSelectable.isSelected();
  }
};

class ClipManipulator : public Manipulator
{
public:

  Manipulatable* GetManipulatable()
  {
    ERROR_MESSAGE("clipper is not manipulatable");
    return 0;
  }

  void setSelected(bool select)
  {
  }
  bool isSelected() const
  {
    return false;
  }
};

class select_all : public scene::Graph::Walker
{
  bool m_select;
public:
  select_all(bool select)
    : m_select(select)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    Selectable* selectable = Instance_getSelectable(instance);
    if(selectable != 0)
    {
      selectable->setSelected(m_select);
    }
    return true;
  }
};

class select_all_component : public scene::Graph::Walker
{
  bool m_select;
  SelectionSystem::EComponentMode m_mode;
public:
  select_all_component(bool select, SelectionSystem::EComponentMode mode)
    : m_select(select), m_mode(mode)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    ComponentSelectionTestable* componentSelectionTestable = Instance_getComponentSelectionTestable(instance);
    if(componentSelectionTestable)
    {
      componentSelectionTestable->setSelectedComponents(m_select, m_mode);
    }
    return true;
  }
};

void Scene_SelectAll_Component(bool select, SelectionSystem::EComponentMode componentMode)
{
  GlobalSceneGraph().traverse(select_all_component(select, componentMode));
}


// RadiantSelectionSystem
class RadiantSelectionSystem :
  public SelectionSystem,
  public Translatable,
  public Rotatable,
  public Scalable,
  public Renderable
{
  mutable Matrix4 m_pivot2world;
  Matrix4 m_pivot2world_start;
  Matrix4 m_manip2pivot_start;
  Translation m_translation;
  Rotation m_rotation;
  Scale m_scale;
public:
  static Shader* m_state;
private:
  EManipulatorMode m_manipulator_mode;
  Manipulator* m_manipulator;

  // state
  bool m_undo_begun;
  EMode m_mode;
  EComponentMode m_componentmode;

  SelectionCounter m_count_primitive;
  SelectionCounter m_count_component;

  TranslateManipulator m_translate_manipulator;
  RotateManipulator m_rotate_manipulator;
  ScaleManipulator m_scale_manipulator;
  DragManipulator m_drag_manipulator;
  ClipManipulator m_clip_manipulator;

  typedef SelectionList<scene::Instance> selection_t;
  selection_t m_selection;
  selection_t m_component_selection;

  Signal1<const Selectable&> m_selectionChanged_callbacks;

  void ConstructPivot() const;
  mutable bool m_pivotChanged;
  bool m_pivot_moving;

  void Scene_TestSelect(Selector& selector, SelectionTest& test, const View& view, SelectionSystem::EMode mode, SelectionSystem::EComponentMode componentMode);

  bool nothingSelected() const
  {
    return (Mode() == eComponent && m_count_component.empty())
      || (Mode() == ePrimitive && m_count_primitive.empty());
  }


public:
  enum EModifier
  {
    eManipulator,
    eToggle,
    eReplace,
    eCycle,
  };

  RadiantSelectionSystem() :
    m_undo_begun(false),
    m_mode(ePrimitive),
    m_componentmode(eDefault),
    m_count_primitive(SelectionChangedCaller(*this)),
    m_count_component(SelectionChangedCaller(*this)),
    m_translate_manipulator(*this, 2, 64),
    m_rotate_manipulator(*this, 8, 64),
    m_scale_manipulator(*this, 0, 64),
    m_pivotChanged(false),
    m_pivot_moving(false)
  {
    SetManipulatorMode(eTranslate);
    pivotChanged();
    addSelectionChangeCallback(PivotChangedSelectionCaller(*this));
    AddGridChangeCallback(PivotChangedCaller(*this));
  }
  void pivotChanged() const
  {
    m_pivotChanged = true;
    SceneChangeNotify();
  }
  typedef ConstMemberCaller<RadiantSelectionSystem, &RadiantSelectionSystem::pivotChanged> PivotChangedCaller;
  void pivotChangedSelection(const Selectable& selectable)
  {
    pivotChanged();
  }
  typedef MemberCaller1<RadiantSelectionSystem, const Selectable&, &RadiantSelectionSystem::pivotChangedSelection> PivotChangedSelectionCaller;

  void SetMode(EMode mode)
  {
    if(m_mode != mode)
    {
      m_mode = mode;
      pivotChanged();
    }
  }
  EMode Mode() const
  {
    return m_mode;
  }
  void SetComponentMode(EComponentMode mode)
  {
    m_componentmode = mode;
  }
  EComponentMode ComponentMode() const
  {
    return m_componentmode;
  }
  void SetManipulatorMode(EManipulatorMode mode)
  {
    m_manipulator_mode = mode;
    switch(m_manipulator_mode)
    {
    case eTranslate: m_manipulator = &m_translate_manipulator; break;
    case eRotate: m_manipulator = &m_rotate_manipulator; break;
    case eScale: m_manipulator = &m_scale_manipulator; break;
    case eDrag: m_manipulator = &m_drag_manipulator; break;
    case eClip: m_manipulator = &m_clip_manipulator; break;
    }
    pivotChanged();
  }
  EManipulatorMode ManipulatorMode() const
  {
    return m_manipulator_mode;
  }

  SelectionChangeCallback getObserver(EMode mode)
  {
    if(mode == ePrimitive)
    {
      return makeCallback1(m_count_primitive);
    }
    else
    {
      return makeCallback1(m_count_component);
    }
  }
  std::size_t countSelected() const
  {
    return m_count_primitive.size();
  }
  std::size_t countSelectedComponents() const
  {
    return m_count_component.size();
  }
  void onSelectedChanged(scene::Instance& instance, const Selectable& selectable)
  {
    if(selectable.isSelected())
    {
      m_selection.append(instance);
    }
    else
    {
      m_selection.erase(instance);
    }

    ASSERT_MESSAGE(m_selection.size() == m_count_primitive.size(), "selection-tracking error");
  }
  void onComponentSelection(scene::Instance& instance, const Selectable& selectable)
  {
    if(selectable.isSelected())
    {
      m_component_selection.append(instance);
    }
    else
    {
      m_component_selection.erase(instance);
    }

    ASSERT_MESSAGE(m_component_selection.size() == m_count_component.size(), "selection-tracking error");
  }
  scene::Instance& ultimateSelected() const
  {
    ASSERT_MESSAGE(m_selection.size() > 0, "no instance selected");
    return m_selection.back();
  }
  scene::Instance& penultimateSelected() const
  {
    ASSERT_MESSAGE(m_selection.size() > 1, "only one instance selected");
    return *(*(--(--m_selection.end())));
  }
  void setSelectedAll(bool selected)
  {
    GlobalSceneGraph().traverse(select_all(selected));

    m_manipulator->setSelected(selected);
  }
  void setSelectedAllComponents(bool selected)
  {
    Scene_SelectAll_Component(selected, SelectionSystem::eVertex);
    Scene_SelectAll_Component(selected, SelectionSystem::eEdge);
    Scene_SelectAll_Component(selected, SelectionSystem::eFace);

    m_manipulator->setSelected(selected);
  }

  void foreachSelected(const Visitor& visitor) const
  {
    selection_t::const_iterator i = m_selection.begin();
    while(i != m_selection.end())
    {
      visitor.visit(*(*(i++)));
    }
  }
  void foreachSelectedComponent(const Visitor& visitor) const
  {
    selection_t::const_iterator i = m_component_selection.begin();
    while(i != m_component_selection.end())
    {
      visitor.visit(*(*(i++)));
    }
  }

  void addSelectionChangeCallback(const SelectionChangeHandler& handler)
  {
    m_selectionChanged_callbacks.connectLast(handler);
  }
  void selectionChanged(const Selectable& selectable)
  {
    m_selectionChanged_callbacks(selectable);
  }
  typedef MemberCaller1<RadiantSelectionSystem, const Selectable&, &RadiantSelectionSystem::selectionChanged> SelectionChangedCaller;


  void startMove()
  {
    m_pivot2world_start = GetPivot2World();
  }

  bool SelectManipulator(const View& view, const float device_point[2], const float device_epsilon[2])
  {
    if(!nothingSelected() || (ManipulatorMode() == eDrag && Mode() == eComponent))
    {
#if defined (DEBUG_SELECTION)
      g_render_clipped.destroy();
#endif

      m_manipulator->setSelected(false);

      if(!nothingSelected() || (ManipulatorMode() == eDrag && Mode() == eComponent))
      {
        View scissored(view);
        ConstructSelectionTest(scissored, SelectionBoxForPoint(device_point, device_epsilon));
        m_manipulator->testSelect(scissored, GetPivot2World());
      }

      startMove();

      m_pivot_moving = m_manipulator->isSelected();

      if(m_pivot_moving)
      {
        Pivot2World pivot;
        pivot.update(GetPivot2World(), view.GetModelview(), view.GetProjection(), view.GetViewport());

        m_manip2pivot_start = matrix4_multiplied_by_matrix4(matrix4_full_inverse(m_pivot2world_start), pivot.m_worldSpace);

        Matrix4 device2manip;
        ConstructDevice2Manip(device2manip, m_pivot2world_start, view.GetModelview(), view.GetProjection(), view.GetViewport());
        m_manipulator->GetManipulatable()->Construct(device2manip, device_point[0], device_point[1]);

        m_undo_begun = false;
      }

      SceneChangeNotify();
    }

    return m_pivot_moving;
  }

  void deselectAll()
  {
    if(Mode() == eComponent)
    {
      setSelectedAllComponents(false);
    }
    else
    {
      setSelectedAll(false);
    }
  }

  void SelectPoint(const View& view, const float device_point[2], const float device_epsilon[2], RadiantSelectionSystem::EModifier modifier, bool face)
  {
    ASSERT_MESSAGE(fabs(device_point[0]) <= 1.0f && fabs(device_point[1]) <= 1.0f, "point-selection error");
    if(modifier == eReplace)
    {
      if(face)
      {
        setSelectedAllComponents(false);
      }
      else
      {
        deselectAll();
      }
    }

  #if defined (DEBUG_SELECTION)
    g_render_clipped.destroy();
  #endif

    {
      View scissored(view);
      ConstructSelectionTest(scissored, SelectionBoxForPoint(device_point, device_epsilon));

      SelectionVolume volume(scissored);
      SelectionPool selector;
      if(face)
      {
        Scene_TestSelect_Component(selector, volume, scissored, eFace);
      }
      else
      {
        Scene_TestSelect(selector, volume, scissored, Mode(), ComponentMode());
      }

      if(!selector.failed())
      {
        switch(modifier)
        {
        case RadiantSelectionSystem::eToggle:
          {
            SelectableSortedSet::iterator best = selector.begin();
            // toggle selection of the object with least depth
            if((*best).second->isSelected())
              (*best).second->setSelected(false);
            else
              (*best).second->setSelected(true);
          }
          break;
          // if cycle mode not enabled, enable it
        case RadiantSelectionSystem::eReplace:
          {
            // select closest
            (*selector.begin()).second->setSelected(true);
          }
          break;
          // select the next object in the list from the one already selected
        case RadiantSelectionSystem::eCycle:
          {
            SelectionPool::iterator i = selector.begin();
            while(i != selector.end())
            {
              if((*i).second->isSelected())
              {
                (*i).second->setSelected(false);
                ++i;
                if(i != selector.end())
                {
                  i->second->setSelected(true);
                }
                else
                {
                  selector.begin()->second->setSelected(true);
                }
                break;
              }
              ++i;
            }
          }
          break;
        default:
          break;
        }
      }
    }
  } 

  void SelectArea(const View& view, const float device_point[2], const float device_delta[2], RadiantSelectionSystem::EModifier modifier, bool face)
  {
    if(modifier == eReplace)
    {
      if(face)
      {
        setSelectedAllComponents(false);
      }
      else
      {
        deselectAll();
      }
    }

  #if defined (DEBUG_SELECTION)
      g_render_clipped.destroy();
  #endif

    {
      View scissored(view);
      ConstructSelectionTest(scissored, SelectionBoxForArea(device_point, device_delta));

      SelectionVolume volume(scissored);
      SelectionPool pool;
      if(face)
      {
        Scene_TestSelect_Component(pool, volume, scissored, eFace);
      }
      else
      {
        Scene_TestSelect(pool, volume, scissored, Mode(), ComponentMode());
      }

      for(SelectionPool::iterator i = pool.begin(); i != pool.end(); ++i)
      {
        (*i).second->setSelected(!(modifier == RadiantSelectionSystem::eToggle && (*i).second->isSelected()));
      }
    }
  }


  void translate(const Vector3& translation)
  {
    if(!nothingSelected())
    {
      //ASSERT_MESSAGE(!m_pivotChanged, "pivot is invalid");

      m_translation = translation;

      m_pivot2world = m_pivot2world_start;
      matrix4_translate_by_vec3(m_pivot2world, translation);

      if(Mode() == eComponent)
      {
        Scene_Translate_Component_Selected(GlobalSceneGraph(), m_translation);
      }
      else
      {
        Scene_Translate_Selected(GlobalSceneGraph(), m_translation);
      }

      SceneChangeNotify();
    }
  }
  void outputTranslation(TextOutputStream& ostream)
  {
    ostream << " -xyz " << m_translation.x() << " " << m_translation.y() << " " << m_translation.z();
  }
  void rotate(const Quaternion& rotation)
  {
    if(!nothingSelected())
    {
      //ASSERT_MESSAGE(!m_pivotChanged, "pivot is invalid");
      
      m_rotation = rotation;

      if(Mode() == eComponent)
      {
        Scene_Rotate_Component_Selected(GlobalSceneGraph(), m_rotation, vector4_to_vector3(m_pivot2world.t()));

        matrix4_assign_rotation_for_pivot(m_pivot2world, m_component_selection.back());
      }
      else
      {
        Scene_Rotate_Selected(GlobalSceneGraph(), m_rotation, vector4_to_vector3(m_pivot2world.t()));

        matrix4_assign_rotation_for_pivot(m_pivot2world, m_selection.back());
      }

      SceneChangeNotify();
    }
  }
  void outputRotation(TextOutputStream& ostream)
  {
    ostream << " -eulerXYZ " << m_rotation.x() << " " << m_rotation.y() << " " << m_rotation.z();
  }
  void scale(const Vector3& scaling)
  {
    if(!nothingSelected())
    {
      m_scale = scaling;

      if(Mode() == eComponent)
      {
        Scene_Scale_Component_Selected(GlobalSceneGraph(), m_scale, vector4_to_vector3(m_pivot2world.t()));
      }
      else
      {
        Scene_Scale_Selected(GlobalSceneGraph(), m_scale, vector4_to_vector3(m_pivot2world.t()));
      }

      SceneChangeNotify();
    }
  }
  void outputScale(TextOutputStream& ostream)
  {
    ostream << " -scale " << m_scale.x() << " " << m_scale.y() << " " << m_scale.z();
  }

  void rotateSelected(const Quaternion& rotation)
  {
    startMove();
    rotate(rotation);
    freezeTransforms();
  }
  void translateSelected(const Vector3& translation)
  {
    startMove();
    translate(translation);
    freezeTransforms();
  }
  void scaleSelected(const Vector3& scaling)
  {
    startMove();
    scale(scaling);
    freezeTransforms();
  }

  void MoveSelected(const View& view, const float device_point[2])
  {
    if(m_manipulator->isSelected())
    {
      if(!m_undo_begun)
      {
        m_undo_begun = true;
        GlobalUndoSystem().start();
      }

      Matrix4 device2manip;
      ConstructDevice2Manip(device2manip, m_pivot2world_start, view.GetModelview(), view.GetProjection(), view.GetViewport());
      m_manipulator->GetManipulatable()->Transform(m_manip2pivot_start, device2manip, device_point[0], device_point[1]);
    }
  }

  /// \todo Support view-dependent nudge.
  void NudgeManipulator(const Vector3& nudge, const Vector3& view)
  {
    if(ManipulatorMode() == eTranslate || ManipulatorMode() == eDrag)
    {
      translateSelected(nudge);
    }
  }

  void endMove();
  void freezeTransforms();

  void renderSolid(Renderer& renderer, const VolumeTest& volume) const;
  void renderWireframe(Renderer& renderer, const VolumeTest& volume) const
  {
    renderSolid(renderer, volume);
  }

  const Matrix4& GetPivot2World() const
  {
    ConstructPivot();
    return m_pivot2world;
  }

  static void constructStatic()
  {
    m_state = GlobalShaderCache().capture("$POINT");
  #if defined(DEBUG_SELECTION)
    g_state_clipped = GlobalShaderCache().capture("$DEBUG_CLIPPED");
  #endif
    TranslateManipulator::m_state_wire = GlobalShaderCache().capture("$WIRE_OVERLAY");
    TranslateManipulator::m_state_fill = GlobalShaderCache().capture("$FLATSHADE_OVERLAY");
    RotateManipulator::m_state_outer = GlobalShaderCache().capture("$WIRE_OVERLAY");
  }

  static void destroyStatic()
  {
  #if defined(DEBUG_SELECTION)
    GlobalShaderCache().release("$DEBUG_CLIPPED");
  #endif
    GlobalShaderCache().release("$WIRE_OVERLAY");
    GlobalShaderCache().release("$FLATSHADE_OVERLAY");
    GlobalShaderCache().release("$WIRE_OVERLAY");
    GlobalShaderCache().release("$POINT");
  }
};

Shader* RadiantSelectionSystem::m_state = 0;


namespace
{
  RadiantSelectionSystem* g_RadiantSelectionSystem;

  inline RadiantSelectionSystem& getSelectionSystem()
  {
    return *g_RadiantSelectionSystem;
  }
}



class testselect_entity_visible : public scene::Graph::Walker
{
  Selector& m_selector;
  SelectionTest& m_test;
public:
  testselect_entity_visible(Selector& selector, SelectionTest& test)
    : m_selector(selector), m_test(test)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    Selectable* selectable = Instance_getSelectable(instance);
    if(selectable != 0
      && Node_isEntity(path.top()))
    {
      m_selector.pushSelectable(*selectable);
    }

    SelectionTestable* selectionTestable = Instance_getSelectionTestable(instance);
    if(selectionTestable)
    {
      selectionTestable->testSelect(m_selector, m_test);
    }

    return true;
  }
  void post(const scene::Path& path, scene::Instance& instance) const
  {
    Selectable* selectable = Instance_getSelectable(instance);
    if(selectable != 0
      && Node_isEntity(path.top()))
    {
      m_selector.popSelectable();
    }
  }
};

class testselect_primitive_visible : public scene::Graph::Walker
{
  Selector& m_selector;
  SelectionTest& m_test;
public:
  testselect_primitive_visible(Selector& selector, SelectionTest& test)
    : m_selector(selector), m_test(test)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    Selectable* selectable = Instance_getSelectable(instance);
    if(selectable != 0)
    {
      m_selector.pushSelectable(*selectable);
    }

    SelectionTestable* selectionTestable = Instance_getSelectionTestable(instance);
    if(selectionTestable)
    {
      selectionTestable->testSelect(m_selector, m_test);
    }

    return true;
  }
  void post(const scene::Path& path, scene::Instance& instance) const
  {
    Selectable* selectable = Instance_getSelectable(instance);
    if(selectable != 0)
    {
      m_selector.popSelectable();
    }
  }
};

class testselect_component_visible : public scene::Graph::Walker
{
  Selector& m_selector;
  SelectionTest& m_test;
  SelectionSystem::EComponentMode m_mode;
public:
  testselect_component_visible(Selector& selector, SelectionTest& test, SelectionSystem::EComponentMode mode)
    : m_selector(selector), m_test(test), m_mode(mode)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    ComponentSelectionTestable* componentSelectionTestable = Instance_getComponentSelectionTestable(instance);
    if(componentSelectionTestable)
    {
      componentSelectionTestable->testSelectComponents(m_selector, m_test, m_mode);
    }

    return true;
  }
};


class testselect_component_visible_selected : public scene::Graph::Walker
{
  Selector& m_selector;
  SelectionTest& m_test;
  SelectionSystem::EComponentMode m_mode;
public:
  testselect_component_visible_selected(Selector& selector, SelectionTest& test, SelectionSystem::EComponentMode mode)
    : m_selector(selector), m_test(test), m_mode(mode)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    Selectable* selectable = Instance_getSelectable(instance);
    if(selectable != 0 && selectable->isSelected())
    {
      ComponentSelectionTestable* componentSelectionTestable = Instance_getComponentSelectionTestable(instance);
      if(componentSelectionTestable)
      {
        componentSelectionTestable->testSelectComponents(m_selector, m_test, m_mode);
      }
    }

    return true;
  }
};

void Scene_TestSelect_Primitive(Selector& selector, SelectionTest& test, const VolumeTest& volume)
{
  Scene_forEachVisible(GlobalSceneGraph(), volume, testselect_primitive_visible(selector, test));
}

void Scene_TestSelect_Component_Selected(Selector& selector, SelectionTest& test, const VolumeTest& volume, SelectionSystem::EComponentMode componentMode)
{
  Scene_forEachVisible(GlobalSceneGraph(), volume, testselect_component_visible_selected(selector, test, componentMode));
}

void Scene_TestSelect_Component(Selector& selector, SelectionTest& test, const VolumeTest& volume, SelectionSystem::EComponentMode componentMode)
{
  Scene_forEachVisible(GlobalSceneGraph(), volume, testselect_component_visible(selector, test, componentMode));
}

void RadiantSelectionSystem::Scene_TestSelect(Selector& selector, SelectionTest& test, const View& view, SelectionSystem::EMode mode, SelectionSystem::EComponentMode componentMode)
{
  switch(mode)
  {
  case eEntity:
    {
      Scene_forEachVisible(GlobalSceneGraph(), view, testselect_entity_visible(selector, test));
    }
    break;
  case ePrimitive:
    Scene_TestSelect_Primitive(selector, test, view);
    break;
  case eComponent:
    Scene_TestSelect_Component_Selected(selector, test, view, componentMode);
    break;
  }
}

class FreezeTransforms : public scene::Graph::Walker
{
public:
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    TransformNode* transformNode = Node_getTransformNode(path.top());
    if(transformNode != 0)
    {
      Transformable* transform = Instance_getTransformable(instance);
      if(transform != 0)
      {
        transform->freezeTransform(); 
      }
    }
    return true;
  }
};

void RadiantSelectionSystem::freezeTransforms()
{
  GlobalSceneGraph().traverse(FreezeTransforms());
}


void RadiantSelectionSystem::endMove()
{
  freezeTransforms();

  if(Mode() == ePrimitive)
  {
    if(ManipulatorMode() == eDrag)
    {
      Scene_SelectAll_Component(false, SelectionSystem::eFace);
    }
  }

  m_pivot_moving = false;
  pivotChanged();

  SceneChangeNotify();

  if(m_undo_begun)
  {
    StringOutputStream command;

    if(ManipulatorMode() == eTranslate)
    {
      command << "translateTool";
      outputTranslation(command);
    }
    else if(ManipulatorMode() == eRotate)
    {
      command << "rotateTool";
      outputRotation(command);
    }
    else if(ManipulatorMode() == eScale)
    {
      command << "scaleTool";
      outputScale(command);
    }
    else if(ManipulatorMode() == eDrag)
    {
      command << "dragTool";
    }

    GlobalUndoSystem().finish(command.c_str());
  }

}

inline AABB Instance_getPivotBounds(scene::Instance& instance)
{
  Entity* entity = Node_getEntity(instance.path().top());
  if(entity != 0
    && (entity->getEntityClass().fixedsize
    || !node_is_group(instance.path().top())))
  {
    Editable* editable = Node_getEditable(instance.path().top());
    if(editable != 0)
    {
      return AABB(vector4_to_vector3(matrix4_multiplied_by_matrix4(instance.localToWorld(), editable->getLocalPivot()).t()), Vector3(0, 0, 0));
    }
    else
    {
      return AABB(vector4_to_vector3(instance.localToWorld().t()), Vector3(0, 0, 0));
    }
  }

  return instance.worldAABB();
}

class bounds_selected : public scene::Graph::Walker
{
  AABB& m_bounds;
public:
  bounds_selected(AABB& bounds)
    : m_bounds(bounds)
  {
    m_bounds = AABB();
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    Selectable* selectable = Instance_getSelectable(instance);
    if(selectable != 0
      && selectable->isSelected())
    {
      aabb_extend_by_aabb_safe(m_bounds, Instance_getPivotBounds(instance));
    }
    return true;
  }
};

class bounds_selected_component : public scene::Graph::Walker
{
  AABB& m_bounds;
public:
  bounds_selected_component(AABB& bounds)
    : m_bounds(bounds)
  {
    m_bounds = AABB();
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    Selectable* selectable = Instance_getSelectable(instance);
    if(selectable != 0
      && selectable->isSelected())
    {
      ComponentEditable* componentEditable = Instance_getComponentEditable(instance);
      if(componentEditable)
      {
        aabb_extend_by_aabb_safe(m_bounds, aabb_for_oriented_aabb_safe(componentEditable->getSelectedComponentsBounds(), instance.localToWorld()));
      }
    }
    return true;
  }
};

void Scene_BoundsSelected(scene::Graph& graph, AABB& bounds)
{
  graph.traverse(bounds_selected(bounds));
}

void Scene_BoundsSelectedComponent(scene::Graph& graph, AABB& bounds)
{
  graph.traverse(bounds_selected_component(bounds));
}

#if 0
inline void pivot_for_node(Matrix4& pivot, scene::Node& node, scene::Instance& instance)
{
  ComponentEditable* componentEditable = Instance_getComponentEditable(instance);
  if(GlobalSelectionSystem().Mode() == SelectionSystem::eComponent
    && componentEditable != 0)
  {
    pivot = matrix4_translation_for_vec3(componentEditable->getSelectedComponentsBounds().origin);
  }
  else
  {
    Bounded* bounded = Instance_getBounded(instance);
    if(bounded != 0)
    {
      pivot = matrix4_translation_for_vec3(bounded->localAABB().origin);
    }
    else
    {
      pivot = g_matrix4_identity;
    }
  }
}
#endif

void RadiantSelectionSystem::ConstructPivot() const
{
  if(!m_pivotChanged || m_pivot_moving)
    return;
  m_pivotChanged = false;

  Vector3 m_object_pivot;

  if(!nothingSelected())
  {
    {
      AABB bounds;
      if(Mode() == eComponent)
      {
        Scene_BoundsSelectedComponent(GlobalSceneGraph(), bounds);
      }
      else
      {
        Scene_BoundsSelected(GlobalSceneGraph(), bounds);
      }
      m_object_pivot = bounds.origin;
    }

    vector3_snap(m_object_pivot, GetGridSize());
    m_pivot2world = matrix4_translation_for_vec3(m_object_pivot);

    switch(m_manipulator_mode)
    {
    case eTranslate:
      break;
    case eRotate:
      if(Mode() == eComponent)
      {
        matrix4_assign_rotation_for_pivot(m_pivot2world, m_component_selection.back());
      }
      else
      {
        matrix4_assign_rotation_for_pivot(m_pivot2world, m_selection.back());
      }
      break;
    case eScale:
      if(Mode() == eComponent)
      {
        matrix4_assign_rotation_for_pivot(m_pivot2world, m_component_selection.back());
      }
      else
      {
        matrix4_assign_rotation_for_pivot(m_pivot2world, m_selection.back());
      }
      break;
    default:
      break;
    }
  }
}

void RadiantSelectionSystem::renderSolid(Renderer& renderer, const VolumeTest& volume) const
{
  //if(view->TestPoint(m_object_pivot))
  if(!nothingSelected())
  {
    renderer.Highlight(Renderer::ePrimitive, false);
    renderer.Highlight(Renderer::eFace, false);

    renderer.SetState(m_state, Renderer::eWireframeOnly);
    renderer.SetState(m_state, Renderer::eFullMaterials);

    m_manipulator->render(renderer, volume, GetPivot2World());
  }

#if defined(DEBUG_SELECTION)
  renderer.SetState(g_state_clipped, Renderer::eWireframeOnly);
  renderer.SetState(g_state_clipped, Renderer::eFullMaterials);
  renderer.addRenderable(g_render_clipped, g_render_clipped.m_world);
#endif
}


void SelectionSystem_OnBoundsChanged()
{
  getSelectionSystem().pivotChanged();
}


SignalHandlerId SelectionSystem_boundsChanged;

void SelectionSystem_Construct()
{
  RadiantSelectionSystem::constructStatic();

  g_RadiantSelectionSystem = new RadiantSelectionSystem;

  SelectionSystem_boundsChanged = GlobalSceneGraph().addBoundsChangedCallback(FreeCaller<SelectionSystem_OnBoundsChanged>());

  GlobalShaderCache().attachRenderable(getSelectionSystem());
}

void SelectionSystem_Destroy()
{
  GlobalShaderCache().detachRenderable(getSelectionSystem());

  GlobalSceneGraph().removeBoundsChangedCallback(SelectionSystem_boundsChanged);

  delete g_RadiantSelectionSystem;

  RadiantSelectionSystem::destroyStatic();
}




inline float screen_normalised(float pos, std::size_t size)
{
  return ((2.0f * pos) / size) - 1.0f;
}

typedef Vector2 DeviceVector;

inline DeviceVector window_to_normalised_device(WindowVector window, std::size_t width, std::size_t height)
{
  return DeviceVector(screen_normalised(window.x(), width), screen_normalised(height - 1 - window.y(), height));
}

inline float device_constrained(float pos)
{
  return std::min(1.0f, std::max(-1.0f, pos));
}

inline DeviceVector device_constrained(DeviceVector device)
{
  return DeviceVector(device_constrained(device.x()), device_constrained(device.y()));
}

inline float window_constrained(float pos, std::size_t origin, std::size_t size)
{
  return std::min(static_cast<float>(origin + size), std::max(static_cast<float>(origin), pos));
}

inline WindowVector window_constrained(WindowVector window, std::size_t x, std::size_t y, std::size_t width, std::size_t height)
{
  return WindowVector(window_constrained(window.x(), x, width), window_constrained(window.y(), y, height));
}

typedef Callback1<DeviceVector> MouseEventCallback;

Single<MouseEventCallback> g_mouseMovedCallback;
Single<MouseEventCallback> g_mouseUpCallback;

#if 1
const ButtonIdentifier c_button_select = c_buttonLeft;
const ModifierFlags c_modifier_manipulator = c_modifierNone;
const ModifierFlags c_modifier_toggle = c_modifierShift;
const ModifierFlags c_modifier_replace = c_modifierShift | c_modifierAlt;
const ModifierFlags c_modifier_face = c_modifierControl;
#else
const ButtonIdentifier c_button_select = c_buttonLeft;
const ModifierFlags c_modifier_manipulator = c_modifierNone;
const ModifierFlags c_modifier_toggle = c_modifierControl;
const ModifierFlags c_modifier_replace = c_modifierNone;
const ModifierFlags c_modifier_face = c_modifierShift;
#endif
const ModifierFlags c_modifier_toggle_face = c_modifier_toggle | c_modifier_face;
const ModifierFlags c_modifier_replace_face = c_modifier_replace | c_modifier_face;

const ButtonIdentifier c_button_texture = c_buttonMiddle;
const ModifierFlags c_modifier_apply_texture = c_modifierControl | c_modifierShift;
const ModifierFlags c_modifier_copy_texture = c_modifierNone;

class Selector_
{
  RadiantSelectionSystem::EModifier modifier_for_state(ModifierFlags state)
  {
    if(state == c_modifier_toggle || state == c_modifier_toggle_face)
    {
      return RadiantSelectionSystem::eToggle;
    }
    if(state == c_modifier_replace || state == c_modifier_replace_face)
    {
      return RadiantSelectionSystem::eReplace;
    }
    return RadiantSelectionSystem::eManipulator;
  }

  rect_t getDeviceArea() const
  {
    DeviceVector delta(m_current - m_start);
    if(selecting() && fabs(delta.x()) > m_epsilon.x() && fabs(delta.y()) > m_epsilon.y())
    {
      return SelectionBoxForArea(&m_start[0], &delta[0]);
    }
    else
    {
      rect_t default_area = { { 0, 0, }, { 0, 0, }, };
      return default_area;
    }
  }

public:
  DeviceVector m_start;
  DeviceVector m_current;
  DeviceVector m_epsilon;
  std::size_t m_unmoved_replaces;
  ModifierFlags m_state;
  const View* m_view;
  RectangleCallback m_window_update;

  Selector_() : m_start(0.0f, 0.0f), m_current(0.0f, 0.0f), m_unmoved_replaces(0), m_state(c_modifierNone)
  {
  }

  void draw_area()
  {
    m_window_update(getDeviceArea());
  }

  void testSelect(DeviceVector position)
  {
    RadiantSelectionSystem::EModifier modifier = modifier_for_state(m_state);
    if(modifier != RadiantSelectionSystem::eManipulator)
    {
      DeviceVector delta(position - m_start);
      if(fabs(delta.x()) > m_epsilon.x() && fabs(delta.y()) > m_epsilon.y())
      {
        DeviceVector delta(position - m_start);
        getSelectionSystem().SelectArea(*m_view, &m_start[0], &delta[0], modifier, (m_state & c_modifier_face) != c_modifierNone);
      }
      else
      {
        if(modifier == RadiantSelectionSystem::eReplace && m_unmoved_replaces++ > 0)
        {
          modifier = RadiantSelectionSystem::eCycle;
        }
        getSelectionSystem().SelectPoint(*m_view, &position[0], &m_epsilon[0], modifier, (m_state & c_modifier_face) != c_modifierNone);
      }
    }

    m_start = m_current = DeviceVector(0.0f, 0.0f);
    draw_area();
  }

  bool selecting() const
  {
    return m_state != c_modifier_manipulator;
  }

  void setState(ModifierFlags state)
  {
    bool was_selecting = selecting();
    m_state = state;
    if(was_selecting ^ selecting())
    {
      draw_area();
    }
  }

  ModifierFlags getState() const
  {
    return m_state;
  }

  void modifierEnable(ModifierFlags type)
  {
    setState(bitfield_enable(getState(), type));
  }
  void modifierDisable(ModifierFlags type)
  {
    setState(bitfield_disable(getState(), type));
  }

  void mouseDown(DeviceVector position)
  {
    m_start = m_current = device_constrained(position);
  }

  void mouseMoved(DeviceVector position)
  {
    m_current = device_constrained(position);
    draw_area();
  }
  typedef MemberCaller1<Selector_, DeviceVector, &Selector_::mouseMoved> MouseMovedCaller;

  void mouseUp(DeviceVector position)
  {
    testSelect(device_constrained(position));

    g_mouseMovedCallback.clear();
    g_mouseUpCallback.clear();
  }
  typedef MemberCaller1<Selector_, DeviceVector, &Selector_::mouseUp> MouseUpCaller;
};


class Manipulator_
{
public:
  DeviceVector m_epsilon;
  const View* m_view;

  bool mouseDown(DeviceVector position)
  {
    return getSelectionSystem().SelectManipulator(*m_view, &position[0], &m_epsilon[0]);
  }

  void mouseMoved(DeviceVector position)
  {
    getSelectionSystem().MoveSelected(*m_view, &position[0]);
  }
  typedef MemberCaller1<Manipulator_, DeviceVector, &Manipulator_::mouseMoved> MouseMovedCaller;

  void mouseUp(DeviceVector position)
  {
    getSelectionSystem().endMove();
    g_mouseMovedCallback.clear();
    g_mouseUpCallback.clear();
  }
  typedef MemberCaller1<Manipulator_, DeviceVector, &Manipulator_::mouseUp> MouseUpCaller;
};

void Scene_copyClosestTexture(SelectionTest& test);
void Scene_applyClosestTexture(SelectionTest& test);

class RadiantWindowObserver : public SelectionSystemWindowObserver
{
  enum
  {
    SELECT_EPSILON = 8,
  };

  int m_width;
  int m_height;

  bool m_mouse_down;

public:
  Selector_ m_selector;
  Manipulator_ m_manipulator;

  RadiantWindowObserver() : m_mouse_down(false)
  {
  }
  void release()
  {
    delete this;
  }
  void setView(const View& view)
  {
    m_selector.m_view = &view;
    m_manipulator.m_view = &view;
  }
  void setRectangleDrawCallback(const RectangleCallback& callback)
  {
    m_selector.m_window_update = callback;
  }
  void onSizeChanged(int width, int height)
  {
    m_width = width;
    m_height = height;
    DeviceVector epsilon(SELECT_EPSILON / static_cast<float>(m_width), SELECT_EPSILON / static_cast<float>(m_height));
    m_selector.m_epsilon = m_manipulator.m_epsilon = epsilon;
  }
  void onMouseDown(const WindowVector& position, ButtonIdentifier button, ModifierFlags modifiers)
  {
    if(button == c_button_select)
    {
      m_mouse_down = true;

      DeviceVector devicePosition(window_to_normalised_device(position, m_width, m_height));
      if(modifiers == c_modifier_manipulator && m_manipulator.mouseDown(devicePosition))
      {
        g_mouseMovedCallback.insert(MouseEventCallback(Manipulator_::MouseMovedCaller(m_manipulator)));
        g_mouseUpCallback.insert(MouseEventCallback(Manipulator_::MouseUpCaller(m_manipulator)));
      }
      else
      {
        m_selector.mouseDown(devicePosition);
        g_mouseMovedCallback.insert(MouseEventCallback(Selector_::MouseMovedCaller(m_selector)));
        g_mouseUpCallback.insert(MouseEventCallback(Selector_::MouseUpCaller(m_selector)));
      }
    }
    else if(button == c_button_texture)
    {
      DeviceVector devicePosition(device_constrained(window_to_normalised_device(position, m_width, m_height)));

      View scissored(*m_selector.m_view);
      ConstructSelectionTest(scissored, SelectionBoxForPoint(&devicePosition[0], &m_selector.m_epsilon[0]));
      SelectionVolume volume(scissored);

      if(modifiers == c_modifier_apply_texture)
      {
        Scene_applyClosestTexture(volume);
      }
      else if(modifiers == c_modifier_copy_texture)
      {
        Scene_copyClosestTexture(volume);
      }
    }
  }
  void onMouseMotion(const WindowVector& position, ModifierFlags modifiers)
  {
    m_selector.m_unmoved_replaces = 0;

    if(m_mouse_down && !g_mouseMovedCallback.empty())
    {
      g_mouseMovedCallback.get()(window_to_normalised_device(position, m_width, m_height));
    }
  }
  void onMouseUp(const WindowVector& position, ButtonIdentifier button, ModifierFlags modifiers)
  {
    if(button == c_button_select && !g_mouseUpCallback.empty())
    {
      m_mouse_down = false;

      g_mouseUpCallback.get()(window_to_normalised_device(position, m_width, m_height));
    }
  }
  void onModifierDown(ModifierFlags type)
  {
    m_selector.modifierEnable(type);
  }
  void onModifierUp(ModifierFlags type)
  {
    m_selector.modifierDisable(type);
  }
};



SelectionSystemWindowObserver* NewWindowObserver()
{
  return new RadiantWindowObserver;
}



#include "modulesystem/singletonmodule.h"
#include "modulesystem/moduleregistry.h"

class SelectionDependencies :
  public GlobalSceneGraphModuleRef,
  public GlobalShaderCacheModuleRef,
  public GlobalOpenGLModuleRef
{
};

class SelectionAPI : public TypeSystemRef
{
  SelectionSystem* m_selection;
public:
  typedef SelectionSystem Type;
  STRING_CONSTANT(Name, "*");

  SelectionAPI()
  {
    SelectionSystem_Construct();

    m_selection = &getSelectionSystem();
  }
  ~SelectionAPI()
  {
    SelectionSystem_Destroy();
  }
  SelectionSystem* getTable()
  {
    return m_selection;
  }
};

typedef SingletonModule<SelectionAPI, SelectionDependencies> SelectionModule;
typedef Static<SelectionModule> StaticSelectionModule;
StaticRegisterModule staticRegisterSelection(StaticSelectionModule::instance());
