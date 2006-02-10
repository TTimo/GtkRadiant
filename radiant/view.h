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

#if !defined(INCLUDED_VIEW_H)
#define INCLUDED_VIEW_H

#include "cullable.h"
#include "math/frustum.h"


#if defined(_DEBUG)
#define DEBUG_CULLING
#endif


#if defined(DEBUG_CULLING)

extern int g_count_dots;
extern int g_count_planes;
extern int g_count_oriented_planes;
extern int g_count_bboxs;
extern int g_count_oriented_bboxs;

#endif

inline void debug_count_dot()
{
#if defined(DEBUG_CULLING)
  ++g_count_dots;
#endif
}

inline void debug_count_plane()
{
#if defined(DEBUG_CULLING)
  ++g_count_planes;
#endif
}

inline void debug_count_oriented_plane()
{
#if defined(DEBUG_CULLING)
  ++g_count_oriented_planes;
#endif
}

inline void debug_count_bbox()
{
#if defined(DEBUG_CULLING)
  ++g_count_bboxs;
#endif
}

inline void debug_count_oriented_bbox()
{
#if defined(DEBUG_CULLING)
  ++g_count_oriented_bboxs;
#endif
}




/// \brief View-volume culling and transformations.
class View : public VolumeTest
{
  /// modelview matrix
  Matrix4 m_modelview;
  /// projection matrix
  Matrix4 m_projection;
  /// device-to-screen transform
  Matrix4 m_viewport;

  Matrix4 m_scissor;

  /// combined modelview and projection matrix
  Matrix4 m_viewproj;
  /// camera position in world space
  Vector4 m_viewer;
  /// view frustum in world space
  Frustum m_frustum;

  bool m_fill;

  void construct()
  {
    m_viewproj = matrix4_multiplied_by_matrix4(matrix4_multiplied_by_matrix4(m_scissor, m_projection), m_modelview);

    m_frustum = frustum_from_viewproj(m_viewproj);
    m_viewer = viewer_from_viewproj(m_viewproj);
  }
public:
  View(bool fill = false) :
    m_modelview(g_matrix4_identity),
    m_projection(g_matrix4_identity),
    m_scissor(g_matrix4_identity),
    m_fill(fill)
  {
  }
  void Construct(const Matrix4& projection, const Matrix4& modelview, std::size_t width, std::size_t height)
  {
    // modelview
    m_modelview = modelview;

    // projection
    m_projection = projection;

    // viewport
    m_viewport = g_matrix4_identity;
    m_viewport[0] = float(width/2);
    m_viewport[5] = float(height/2);
    if(fabs(m_projection[11]) > 0.0000001)
      m_viewport[10] = m_projection[0] * m_viewport[0];
    else
      m_viewport[10] = 1 / m_projection[10];

    construct();
  }
  void EnableScissor(float min_x, float max_x, float min_y, float max_y)
  {
    m_scissor = g_matrix4_identity;
    m_scissor[0] = static_cast<float>((max_x - min_x) * 0.5);
    m_scissor[5] = static_cast<float>((max_y - min_y) * 0.5);
    m_scissor[12] = static_cast<float>((min_x + max_x) * 0.5);
    m_scissor[13] = static_cast<float>((min_y + max_y) * 0.5);
    matrix4_full_invert(m_scissor);

    construct();
  }
  void DisableScissor()
  {
    m_scissor = g_matrix4_identity;

    construct();
  }

  bool TestPoint(const Vector3& point) const
  {
    return viewproj_test_point(m_viewproj, point);
  }
  bool TestLine(const Segment& segment) const
  {
    return frustum_test_line(m_frustum, segment);
  }
  bool TestPlane(const Plane3& plane) const
  {
    debug_count_plane();
    return viewer_test_plane(m_viewer, plane);
  }
  bool TestPlane(const Plane3& plane, const Matrix4& localToWorld) const
  {
    debug_count_oriented_plane();
    return viewer_test_transformed_plane(m_viewer, plane, localToWorld);
  }
  VolumeIntersectionValue TestAABB(const AABB& aabb) const
  {
    debug_count_bbox();
    return frustum_test_aabb(m_frustum, aabb);
  }
  VolumeIntersectionValue TestAABB(const AABB& aabb, const Matrix4& localToWorld) const
  {
    debug_count_oriented_bbox();
    return frustum_intersects_transformed_aabb(m_frustum, aabb, localToWorld);
  }

  const Matrix4& GetViewMatrix() const
  {
    return m_viewproj;
  }
  const Matrix4& GetViewport() const
  {
    return m_viewport;
  };
  const Matrix4& GetModelview() const
  {
    return m_modelview;
  }
  const Matrix4& GetProjection() const
  {
    return m_projection;
  }

  bool fill() const
  {
    return m_fill;
  }
  const Vector3& getViewer() const
  {
    return m_viewer;
  }
};

#endif
