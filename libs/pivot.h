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

#if !defined(INCLUDED_PIVOT_H)
#define INCLUDED_PIVOT_H

#include "math/matrix.h"


inline void billboard_viewplaneOriented(Matrix4& rotation, const Matrix4& world2screen)
{
#if 1
  rotation = g_matrix4_identity;
  Vector3 x(vector3_normalised(vector4_to_vector3(world2screen.x())));
  Vector3 y(vector3_normalised(vector4_to_vector3(world2screen.y())));
  Vector3 z(vector3_normalised(vector4_to_vector3(world2screen.z())));
  vector4_to_vector3(rotation.y()) = Vector3(x.y(), y.y(), z.y());
  vector4_to_vector3(rotation.z()) = vector3_negated(Vector3(x.z(), y.z(), z.z()));
  vector4_to_vector3(rotation.x()) = vector3_normalised(vector3_cross(vector4_to_vector3(rotation.y()), vector4_to_vector3(rotation.z())));
  vector4_to_vector3(rotation.y()) = vector3_cross(vector4_to_vector3(rotation.z()), vector4_to_vector3(rotation.x()));
#else
  Matrix4 screen2world(matrix4_full_inverse(world2screen));

  Vector3 near_(
    vector4_projected(
      matrix4_transformed_vector4(
        screen2world,
        Vector4(0, 0, -1, 1)
      )
    )
  );

  Vector3 far_(
    vector4_projected(
      matrix4_transformed_vector4(
        screen2world,
        Vector4(0, 0, 1, 1)
      )
    )
  );

  Vector3 up(
    vector4_projected(
      matrix4_transformed_vector4(
        screen2world,
        Vector4(0, 1, -1, 1)
      )
    )
  );

  rotation = g_matrix4_identity;
  vector4_to_vector3(rotation.y()) = vector3_normalised(vector3_subtracted(up, near_));
  vector4_to_vector3(rotation.z()) = vector3_normalised(vector3_subtracted(near_, far_));
  vector4_to_vector3(rotation.x()) = vector3_normalised(vector3_cross(vector4_to_vector3(rotation.y()), vector4_to_vector3(rotation.z())));
  vector4_to_vector3(rotation.y()) = vector3_cross(vector4_to_vector3(rotation.z()), vector4_to_vector3(rotation.x()));
#endif
}

inline void billboard_viewpointOriented(Matrix4& rotation, const Matrix4& world2screen)
{
  Matrix4 screen2world(matrix4_full_inverse(world2screen));

#if 1
  rotation = g_matrix4_identity;
  vector4_to_vector3(rotation.y()) = vector3_normalised(vector4_to_vector3(screen2world.y()));
  vector4_to_vector3(rotation.z()) = vector3_negated(vector3_normalised(vector4_to_vector3(screen2world.z())));
  vector4_to_vector3(rotation.x()) = vector3_normalised(vector3_cross(vector4_to_vector3(rotation.y()), vector4_to_vector3(rotation.z())));
  vector4_to_vector3(rotation.y()) = vector3_cross(vector4_to_vector3(rotation.z()), vector4_to_vector3(rotation.x()));
#else
  Vector3 near_(
    vector4_projected(
      matrix4_transformed_vector4(
        screen2world,
        Vector4(world2screen[12] / world2screen[15], world2screen[13] / world2screen[15], -1, 1)
      )
    )
  );

  Vector3 far_(
    vector4_projected(
      matrix4_transformed_vector4(
        screen2world,
        Vector4(world2screen[12] / world2screen[15], world2screen[13] / world2screen[15], 1, 1)
      )
    )
  );

  Vector3 up(
    vector4_projected(
      matrix4_transformed_vector4(
        screen2world,
        Vector4(world2screen[12] / world2screen[15], world2screen[13] / world2screen[15] + 1, -1, 1)
      )
    )
  );

  rotation = g_matrix4_identity;
  vector4_to_vector3(rotation.y()) = vector3_normalised(vector3_subtracted(up, near_));
  vector4_to_vector3(rotation.z()) = vector3_normalised(vector3_subtracted(near_, far_));
  vector4_to_vector3(rotation.x()) = vector3_normalised(vector3_cross(vector4_to_vector3(rotation.y()), vector4_to_vector3(rotation.z())));
  vector4_to_vector3(rotation.y()) = vector3_cross(vector4_to_vector3(rotation.z()), vector4_to_vector3(rotation.x()));
#endif
}


inline void ConstructObject2Screen(Matrix4& object2screen, const Matrix4& object2world, const Matrix4& world2view, const Matrix4& view2device, const Matrix4& device2screen)
{
  object2screen = device2screen;
  matrix4_multiply_by_matrix4(object2screen, view2device);
  matrix4_multiply_by_matrix4(object2screen, world2view);
  matrix4_multiply_by_matrix4(object2screen, object2world);
}

inline void ConstructObject2Device(Matrix4& object2screen, const Matrix4& object2world, const Matrix4& world2view, const Matrix4& view2device)
{
  object2screen = view2device;
  matrix4_multiply_by_matrix4(object2screen, world2view);
  matrix4_multiply_by_matrix4(object2screen, object2world);
}

inline void ConstructDevice2Object(Matrix4& device2object, const Matrix4& object2world, const Matrix4& world2view, const Matrix4& view2device)
{
  ConstructObject2Device(device2object, object2world, world2view, view2device);
  matrix4_full_invert(device2object);
}

//! S =  ( Inverse(Object2Screen *post ScaleOf(Object2Screen) ) *post Object2Screen
inline void pivot_scale(Matrix4& scale, const Matrix4& pivot2screen)
{
  Matrix4 pre_scale(g_matrix4_identity);
  pre_scale[0] = static_cast<float>(vector3_length(vector4_to_vector3(pivot2screen.x())));
  pre_scale[5] = static_cast<float>(vector3_length(vector4_to_vector3(pivot2screen.y())));
  pre_scale[10] = static_cast<float>(vector3_length(vector4_to_vector3(pivot2screen.z())));

  scale = pivot2screen;
  matrix4_multiply_by_matrix4(scale, pre_scale);
  matrix4_full_invert(scale);
  matrix4_multiply_by_matrix4(scale, pivot2screen);
}

// scale by (inverse) W
inline void pivot_perspective(Matrix4& scale, const Matrix4& pivot2screen)
{
  scale = g_matrix4_identity;
  scale[0] = scale[5] = scale[10] = pivot2screen[15];
}

inline void ConstructDevice2Manip(Matrix4& device2manip, const Matrix4& object2world, const Matrix4& world2view, const Matrix4& view2device, const Matrix4& device2screen)
{
  Matrix4 pivot2screen;
  ConstructObject2Screen(pivot2screen, object2world, world2view, view2device, device2screen);

  ConstructObject2Device(device2manip, object2world, world2view, view2device);

  Matrix4 scale;
  pivot_scale(scale, pivot2screen);
  matrix4_multiply_by_matrix4(device2manip, scale);
  pivot_perspective(scale, pivot2screen);
  matrix4_multiply_by_matrix4(device2manip, scale);

  matrix4_full_invert(device2manip);
}

inline void Pivot2World_worldSpace(Matrix4& manip2world, const Matrix4& pivot2world, const Matrix4& modelview, const Matrix4& projection, const Matrix4& viewport)
{
  manip2world = pivot2world;
  
  Matrix4 pivot2screen;
  ConstructObject2Screen(pivot2screen, pivot2world, modelview, projection, viewport);

  Matrix4 scale;
  pivot_scale(scale, pivot2screen);
  matrix4_multiply_by_matrix4(manip2world, scale);
  pivot_perspective(scale, pivot2screen);
  matrix4_multiply_by_matrix4(manip2world, scale);
}

inline void Pivot2World_viewpointSpace(Matrix4& manip2world, Vector3& axis, const Matrix4& pivot2world, const Matrix4& modelview, const Matrix4& projection, const Matrix4& viewport)
{
  manip2world = pivot2world;

  Matrix4 pivot2screen;
  ConstructObject2Screen(pivot2screen, pivot2world, modelview, projection, viewport);

  Matrix4 scale;
  pivot_scale(scale, pivot2screen);
  matrix4_multiply_by_matrix4(manip2world, scale);

  billboard_viewpointOriented(scale, pivot2screen);
  axis = vector4_to_vector3(scale.z());
  matrix4_multiply_by_matrix4(manip2world, scale);

  pivot_perspective(scale, pivot2screen);
  matrix4_multiply_by_matrix4(manip2world, scale);
}

inline void Pivot2World_viewplaneSpace(Matrix4& manip2world, const Matrix4& pivot2world, const Matrix4& modelview, const Matrix4& projection, const Matrix4& viewport)
{
  manip2world = pivot2world;

  Matrix4 pivot2screen;
  ConstructObject2Screen(pivot2screen, pivot2world, modelview, projection, viewport);

  Matrix4 scale;
  pivot_scale(scale, pivot2screen);
  matrix4_multiply_by_matrix4(manip2world, scale);

  billboard_viewplaneOriented(scale, pivot2screen);
  matrix4_multiply_by_matrix4(manip2world, scale);

  pivot_perspective(scale, pivot2screen);
  matrix4_multiply_by_matrix4(manip2world, scale);
}


#include "renderable.h"
#include "cullable.h"
#include "render.h"

const Colour4b g_colour_x(255, 0, 0, 255);
const Colour4b g_colour_y(0, 255, 0, 255);
const Colour4b g_colour_z(0, 0, 255, 255);

class Shader;

class RenderablePivot : public OpenGLRenderable
{
  VertexBuffer<PointVertex> m_vertices;
public:
  mutable Matrix4 m_localToWorld;
  typedef Static<Shader*> StaticShader;
  static Shader* getShader()
  {
    return StaticShader::instance();
  }

  RenderablePivot()
  {
    m_vertices.reserve(6);

    m_vertices.push_back(PointVertex(Vertex3f(0, 0, 0), g_colour_x));
    m_vertices.push_back(PointVertex(Vertex3f(16, 0, 0), g_colour_x));

    m_vertices.push_back(PointVertex(Vertex3f(0, 0, 0), g_colour_y));
    m_vertices.push_back(PointVertex(Vertex3f(0, 16, 0), g_colour_y));

    m_vertices.push_back(PointVertex(Vertex3f(0, 0, 0), g_colour_z));
    m_vertices.push_back(PointVertex(Vertex3f(0, 0, 16), g_colour_z));
  }

  void render(RenderStateFlags state) const
  {
    if(m_vertices.size() == 0) return;
    if(m_vertices.data() == 0) return;
    glVertexPointer(3, GL_FLOAT, sizeof(PointVertex), &m_vertices.data()->vertex);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(PointVertex), &m_vertices.data()->colour);
    glDrawArrays(GL_LINES, 0, m_vertices.size());
  }

  void render(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    renderer.PushState();

    Pivot2World_worldSpace(m_localToWorld, localToWorld, volume.GetModelview(), volume.GetProjection(), volume.GetViewport());

    renderer.Highlight(Renderer::ePrimitive, false);
    renderer.SetState(getShader(), Renderer::eWireframeOnly);
    renderer.SetState(getShader(), Renderer::eFullMaterials);
    renderer.addRenderable(*this, m_localToWorld);

    renderer.PopState();
  }
};



#endif
