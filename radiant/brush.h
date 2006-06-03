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

#if !defined(INCLUDED_BRUSH_H)
#define INCLUDED_BRUSH_H

/// \file
/// \brief The brush primitive.
///
/// A collection of planes that define a convex polyhedron.
/// The Boundary-Representation of this primitive is a manifold polygonal mesh.
/// Each face polygon is represented by a list of vertices in a \c Winding.
/// Each vertex is associated with another face that is adjacent to the edge
/// formed by itself and the next vertex in the winding. This information can
/// be used to find edge-pairs and vertex-rings.


#include "debugging/debugging.h"

#include "itexdef.h"
#include "iundo.h"
#include "iselection.h"
#include "irender.h"
#include "imap.h"
#include "ibrush.h"
#include "igl.h"
#include "ifilter.h"
#include "nameable.h"
#include "moduleobserver.h"

#include <set>

#include "cullable.h"
#include "renderable.h"
#include "selectable.h"
#include "editable.h"
#include "mapfile.h"

#include "math/frustum.h"
#include "selectionlib.h"
#include "render.h"
#include "texturelib.h"
#include "container/container.h"
#include "generic/bitfield.h"
#include "signal/signalfwd.h"

#include "winding.h"
#include "brush_primit.h"

#define CONTENTS_DETAIL 0x8000000


enum EBrushType
{
  eBrushTypeQuake,
  eBrushTypeQuake2,
  eBrushTypeQuake3,
  eBrushTypeQuake3BP,
  eBrushTypeDoom3,
  eBrushTypeQuake4,
  eBrushTypeHalfLife,
};


#define BRUSH_CONNECTIVITY_DEBUG 0
#define BRUSH_DEGENERATE_DEBUG 0

template<typename TextOuputStreamType>
inline TextOuputStreamType& ostream_write(TextOuputStreamType& ostream, const Matrix4& m)
{
  return ostream << "(" << m[0] << " " << m[1] << " " << m[2] << " " << m[3] << ", "
    << m[4] << " " << m[5] << " " << m[6] << " " << m[7] << ", "
    << m[8] << " " << m[9] << " " << m[10] << " " << m[11] << ", "
    << m[12] << " " << m[13] << " " << m[14] << " " << m[15] << ")";
}

inline void print_vector3(const Vector3& v)
{
  globalOutputStream() << "( " << v.x() << " " << v.y() << " " << v.z() << " )\n";
}

inline void print_3x3(const Matrix4& m)
{
  globalOutputStream() << "( " << m.xx() << " " << m.xy() << " " << m.xz() << " ) "
    << "( " << m.yx() << " " << m.yy() << " " << m.yz() << " ) "
    << "( " << m.zx() << " " << m.zy() << " " << m.zz() << " )\n";
}



inline bool texdef_sane(const texdef_t& texdef)
{
  return fabs(texdef.shift[0]) < (1 << 16)
    && fabs(texdef.shift[1]) < (1 << 16);
}

inline void Winding_DrawWireframe(const Winding& winding)
{
  glVertexPointer(3, GL_FLOAT, sizeof(WindingVertex), &winding.points.data()->vertex);
  glDrawArrays(GL_LINE_LOOP, 0, GLsizei(winding.numpoints));
}

inline void Winding_Draw(const Winding& winding, const Vector3& normal, RenderStateFlags state)
{
  glVertexPointer(3, GL_FLOAT, sizeof(WindingVertex), &winding.points.data()->vertex);

  if((state & RENDER_BUMP) != 0)
  {
    Vector3 normals[c_brush_maxFaces];
    typedef Vector3* Vector3Iter;
    for(Vector3Iter i = normals, end = normals + winding.numpoints; i != end; ++i)
    {
      *i = normal;
    }
    if(GlobalShaderCache().useShaderLanguage())
    {
      glNormalPointer(GL_FLOAT, sizeof(Vector3), normals);
      glVertexAttribPointerARB(c_attr_TexCoord0, 2, GL_FLOAT, 0, sizeof(WindingVertex), &winding.points.data()->texcoord);
      glVertexAttribPointerARB(c_attr_Tangent, 3, GL_FLOAT, 0, sizeof(WindingVertex), &winding.points.data()->tangent);
      glVertexAttribPointerARB(c_attr_Binormal, 3, GL_FLOAT, 0, sizeof(WindingVertex), &winding.points.data()->bitangent);
    }
    else
    {
      glVertexAttribPointerARB(11, 3, GL_FLOAT, 0, sizeof(Vector3), normals);
      glVertexAttribPointerARB(8, 2, GL_FLOAT, 0, sizeof(WindingVertex), &winding.points.data()->texcoord);
      glVertexAttribPointerARB(9, 3, GL_FLOAT, 0, sizeof(WindingVertex), &winding.points.data()->tangent);
      glVertexAttribPointerARB(10, 3, GL_FLOAT, 0, sizeof(WindingVertex), &winding.points.data()->bitangent);
    }
  }
  else
  {
    if (state & RENDER_LIGHTING)
    {
      Vector3 normals[c_brush_maxFaces];
      typedef Vector3* Vector3Iter;
      for(Vector3Iter i = normals, last = normals + winding.numpoints; i != last; ++i)
      {
        *i = normal;
      }
      glNormalPointer(GL_FLOAT, sizeof(Vector3), normals);
    }

    if (state & RENDER_TEXTURE)
    {
      glTexCoordPointer(2, GL_FLOAT, sizeof(WindingVertex), &winding.points.data()->texcoord);
    }
  }
#if 0
  if (state & RENDER_FILL)
  {
    glDrawArrays(GL_TRIANGLE_FAN, 0, GLsizei(winding.numpoints));
  }
  else
  {
    glDrawArrays(GL_LINE_LOOP, 0, GLsizei(winding.numpoints));
  }
#else
  glDrawArrays(GL_POLYGON, 0, GLsizei(winding.numpoints));
#endif

#if 0
  const Winding& winding = winding;

  if(state & RENDER_FILL)
  {
    glBegin(GL_POLYGON);
  }
  else
  {
    glBegin(GL_LINE_LOOP);
  }

  if (state & RENDER_LIGHTING)
    glNormal3fv(normal);

  for(int i = 0; i < winding.numpoints; ++i)
  {
    if (state & RENDER_TEXTURE)
      glTexCoord2fv(&winding.points[i][3]);
    glVertex3fv(winding.points[i]);
  }
  glEnd();
#endif
}

const Colour4b colour_vertex(0, 255, 0, 255);


#include "shaderlib.h"

typedef DoubleVector3 PlanePoints[3];

inline bool planepts_equal(const PlanePoints planepts, const PlanePoints other)
{
  return planepts[0] == other[0] && planepts[1] == other[1] && planepts[2] == other[2];
}

inline void planepts_assign(PlanePoints planepts, const PlanePoints other)
{
  planepts[0] = other[0];
  planepts[1] = other[1];
  planepts[2] = other[2];
}

inline void planepts_quantise(PlanePoints planepts, double snap)
{
  vector3_snap(planepts[0], snap);
  vector3_snap(planepts[1], snap);
  vector3_snap(planepts[2], snap);
}

inline float vector3_max_component(const Vector3& vec3)
{
  return std::max(fabsf(vec3[0]), std::max(fabsf(vec3[1]), fabsf(vec3[2])));
}

inline void edge_snap(Vector3& edge, double snap)
{
  float scale = static_cast<float>(ceil(fabs(snap / vector3_max_component(edge))));
  if(scale > 0.0f)
  {
    vector3_scale(edge, scale);
  }
  vector3_snap(edge, snap);
}

inline void planepts_snap(PlanePoints planepts, double snap)
{
  Vector3 edge01(vector3_subtracted(planepts[1], planepts[0]));
  Vector3 edge12(vector3_subtracted(planepts[2], planepts[1]));
  Vector3 edge20(vector3_subtracted(planepts[0], planepts[2]));

  double length_squared_01 = vector3_dot(edge01, edge01);
  double length_squared_12 = vector3_dot(edge12, edge12);
  double length_squared_20 = vector3_dot(edge20, edge20);

  vector3_snap(planepts[0], snap);

  if(length_squared_01 < length_squared_12)
  {
    if(length_squared_12 < length_squared_20)
    {
      edge_snap(edge01, snap);
      edge_snap(edge12, snap);
      planepts[1] = vector3_added(planepts[0], edge01);
      planepts[2] = vector3_added(planepts[1], edge12);
    }
    else
    {
      edge_snap(edge20, snap);
      edge_snap(edge01, snap);
      planepts[1] = vector3_added(planepts[0], edge20);
      planepts[2] = vector3_added(planepts[1], edge01);
    }
  }
  else
  {
    if(length_squared_01 < length_squared_20)
    {
      edge_snap(edge01, snap);
      edge_snap(edge12, snap);
      planepts[1] = vector3_added(planepts[0], edge01);
      planepts[2] = vector3_added(planepts[1], edge12);
    }
    else
    {
      edge_snap(edge12, snap);
      edge_snap(edge20, snap);
      planepts[1] = vector3_added(planepts[0], edge12);
      planepts[2] = vector3_added(planepts[1], edge20);
    }
  }
}

inline PointVertex pointvertex_for_planept(const DoubleVector3& point, const Colour4b& colour)
{
  return PointVertex(
    Vertex3f(
      static_cast<float>(point.x()),
      static_cast<float>(point.y()),
      static_cast<float>(point.z())
    ),
    colour
  );
}

inline PointVertex pointvertex_for_windingpoint(const Vector3& point, const Colour4b& colour)
{
  return PointVertex(
    vertex3f_for_vector3(point),
    colour
  );
}

inline bool check_plane_is_integer(const PlanePoints& planePoints)
{
  return !float_is_integer(planePoints[0][0])
    || !float_is_integer(planePoints[0][1])
    || !float_is_integer(planePoints[0][2])
    || !float_is_integer(planePoints[1][0])
    || !float_is_integer(planePoints[1][1])
    || !float_is_integer(planePoints[1][2])
    || !float_is_integer(planePoints[2][0])
    || !float_is_integer(planePoints[2][1])
    || !float_is_integer(planePoints[2][2]);
}

inline void brush_check_shader(const char* name)
{
  if(!shader_valid(name))
  {
    globalErrorStream() << "brush face has invalid texture name: '" << name << "'\n";
  }
}

class FaceShaderObserver
{
public:
  virtual void realiseShader() = 0;
  virtual void unrealiseShader() = 0;
};

class FaceShaderObserverRealise
{
public:
  void operator()(FaceShaderObserver& observer) const
  {
    observer.realiseShader();
  }
};

class FaceShaderObserverUnrealise
{
public:
  void operator()(FaceShaderObserver& observer) const
  {
    observer.unrealiseShader();
  }
};

typedef ReferencePair<FaceShaderObserver> FaceShaderObserverPair;


class ContentsFlagsValue
{
public:
  ContentsFlagsValue()
  {
  }
  ContentsFlagsValue(int surfaceFlags, int contentFlags, int value, bool specified) :
    m_surfaceFlags(surfaceFlags),
    m_contentFlags(contentFlags),
    m_value(value),
    m_specified(specified)
  {
  }
  int m_surfaceFlags;
  int m_contentFlags;
  int m_value;
  bool m_specified;
};

inline void ContentsFlagsValue_assignMasked(ContentsFlagsValue& flags, const ContentsFlagsValue& other)
{
  bool detail = bitfield_enabled(flags.m_contentFlags, CONTENTS_DETAIL);
  flags = other;
  if(detail)
  {
    flags.m_contentFlags = bitfield_enable(flags.m_contentFlags, CONTENTS_DETAIL);
  }
  else
  {
    flags.m_contentFlags = bitfield_disable(flags.m_contentFlags, CONTENTS_DETAIL);
  }
}


class FaceShader : public ModuleObserver
{
public:
  class SavedState
  {
  public:
    CopiedString m_shader;
    ContentsFlagsValue m_flags;

    SavedState(const FaceShader& faceShader)
    {
      m_shader = faceShader.getShader();
      m_flags = faceShader.m_flags;
    }

    void exportState(FaceShader& faceShader) const
    {
      faceShader.setShader(m_shader.c_str());
      faceShader.setFlags(m_flags);
    }
  };

  CopiedString m_shader;
  Shader* m_state;
  ContentsFlagsValue m_flags;
  FaceShaderObserverPair m_observers;
  bool m_instanced;
  bool m_realised;

  FaceShader(const char* shader, const ContentsFlagsValue& flags = ContentsFlagsValue(0, 0, 0, false)) :
    m_shader(shader),
    m_state(0),
    m_flags(flags),
    m_instanced(false),
    m_realised(false)
  {
    captureShader();
  }
  ~FaceShader()
  {
    releaseShader();
  }
  // copy-construction not supported
  FaceShader(const FaceShader& other);

  void instanceAttach()
  {
    m_instanced = true;
    m_state->incrementUsed();
  }
  void instanceDetach()
  {
    m_state->decrementUsed();
    m_instanced = false;
  }

  void captureShader()
  {
    ASSERT_MESSAGE(m_state == 0, "shader cannot be captured");
    brush_check_shader(m_shader.c_str());
    m_state = GlobalShaderCache().capture(m_shader.c_str());
    m_state->attach(*this);
  }
  void releaseShader()
  {
    ASSERT_MESSAGE(m_state != 0, "shader cannot be released");
    m_state->detach(*this);
    GlobalShaderCache().release(m_shader.c_str());
    m_state = 0;
  }

  void realise()
  {
    ASSERT_MESSAGE(!m_realised, "FaceTexdef::realise: already realised");
    m_realised = true;
    m_observers.forEach(FaceShaderObserverRealise());
  }
  void unrealise()
  {
    ASSERT_MESSAGE(m_realised, "FaceTexdef::unrealise: already unrealised");
    m_observers.forEach(FaceShaderObserverUnrealise());
    m_realised = false;
  }

  void attach(FaceShaderObserver& observer)
  {
    m_observers.attach(observer);
    if(m_realised)
    {
      observer.realiseShader();
    }
  }

  void detach(FaceShaderObserver& observer)
  {
    if(m_realised)
    {
      observer.unrealiseShader();
    }
    m_observers.detach(observer);
  }

  const char* getShader() const
  {
    return m_shader.c_str();
  }
  void setShader(const char* name)
  {
    if(m_instanced)
    {
      m_state->decrementUsed();
    }
    releaseShader();
    m_shader = name;
    captureShader();
    if(m_instanced)
    {
      m_state->incrementUsed();
    }
  }
  ContentsFlagsValue getFlags() const
  {
    ASSERT_MESSAGE(m_realised, "FaceShader::getFlags: flags not valid when unrealised");
    if(!m_flags.m_specified)
    {
      return ContentsFlagsValue(
        m_state->getTexture().surfaceFlags,
        m_state->getTexture().contentFlags,
        m_state->getTexture().value,
        true
      );
    }
    return m_flags;
  }
  void setFlags(const ContentsFlagsValue& flags)
  {
    ASSERT_MESSAGE(m_realised, "FaceShader::setFlags: flags not valid when unrealised");
    ContentsFlagsValue_assignMasked(m_flags, flags);
  }

  Shader* state() const
  {
    return m_state;
  }

  std::size_t width() const
  {
    if(m_realised)
    {
      return m_state->getTexture().width;
    }
    return 1;
  }
  std::size_t height() const
  {
    if(m_realised)
    {
      return m_state->getTexture().height;
    }
    return 1;
  }
  unsigned int shaderFlags() const
  {
    if(m_realised)
    {
      return m_state->getFlags();
    }
    return 0;
  }
};




class FaceTexdef : public FaceShaderObserver
{
  // not copyable
  FaceTexdef(const FaceTexdef& other);
  // not assignable
  FaceTexdef& operator=(const FaceTexdef& other);
public:
  class SavedState
  {
  public:
    TextureProjection m_projection;

    SavedState(const FaceTexdef& faceTexdef)
    {
      m_projection = faceTexdef.m_projection;
    }

    void exportState(FaceTexdef& faceTexdef) const
    {
      Texdef_Assign(faceTexdef.m_projection, m_projection);
    }
  };

  FaceShader& m_shader;
  TextureProjection m_projection;
  bool m_projectionInitialised;
  bool m_scaleApplied;

  FaceTexdef(
    FaceShader& shader,
    const TextureProjection& projection,
    bool projectionInitialised = true
  ) :
    m_shader(shader),
    m_projection(projection),
    m_projectionInitialised(projectionInitialised),
    m_scaleApplied(false)
  {
    m_shader.attach(*this);
  }
  ~FaceTexdef()
  {
    m_shader.detach(*this);
  }

  void addScale()
  {
    ASSERT_MESSAGE(!m_scaleApplied, "texture scale aready added");
    m_scaleApplied = true;
    m_projection.m_brushprimit_texdef.addScale(m_shader.width(), m_shader.height());
  }
  void removeScale()
  {
    ASSERT_MESSAGE(m_scaleApplied, "texture scale aready removed");
    m_scaleApplied = false;
    m_projection.m_brushprimit_texdef.removeScale(m_shader.width(), m_shader.height());
  }

  void realiseShader()
  {
    if(m_projectionInitialised && !m_scaleApplied)
    {
      addScale();
    }
  }
  void unrealiseShader()
  {
    if(m_projectionInitialised && m_scaleApplied)
    {
      removeScale();
    }
  }

  void setTexdef(const TextureProjection& projection)
  {
    removeScale();
    Texdef_Assign(m_projection, projection);
    addScale();
  }

  void shift(float s, float t)
  {
    ASSERT_MESSAGE(texdef_sane(m_projection.m_texdef), "FaceTexdef::shift: bad texdef");
    removeScale();
    Texdef_Shift(m_projection, s, t);
    addScale();
  }

  void scale(float s, float t)
  {
    removeScale();
    Texdef_Scale(m_projection, s, t);
    addScale();
  }

  void rotate(float angle)
  {
    removeScale();
    Texdef_Rotate(m_projection, angle);
    addScale();
  }

  void fit(const Vector3& normal, const Winding& winding, float s_repeat, float t_repeat)
  {
    Texdef_FitTexture(m_projection, m_shader.width(), m_shader.height(), normal, winding, s_repeat, t_repeat);
  }

  void emitTextureCoordinates(Winding& winding, const Vector3& normal, const Matrix4& localToWorld)
  {
    Texdef_EmitTextureCoordinates(m_projection, m_shader.width(), m_shader.height(), winding, normal, localToWorld);
  }

  void transform(const Plane3& plane, const Matrix4& matrix)
  {
    removeScale();
    Texdef_transformLocked(m_projection, m_shader.width(), m_shader.height(), plane, matrix);
    addScale();
  }

  TextureProjection normalised() const
  {
    brushprimit_texdef_t tmp(m_projection.m_brushprimit_texdef);
    tmp.removeScale(m_shader.width(), m_shader.height());
    return TextureProjection(m_projection.m_texdef, tmp, m_projection.m_basis_s, m_projection.m_basis_t);
  }
  void setBasis(const Vector3& normal)
  {
    Matrix4 basis;
    Normal_GetTransform(normal, basis);
    m_projection.m_basis_s = Vector3(basis.xx(), basis.yx(), basis.zx());
    m_projection.m_basis_t = Vector3(-basis.xy(), -basis.yy(), -basis.zy());
  }
};

inline void planepts_print(const PlanePoints& planePoints, TextOutputStream& ostream)
{
  ostream << "( " << planePoints[0][0] << " " << planePoints[0][1] << " " << planePoints[0][2] << " ) "
    << "( " << planePoints[1][0] << " " << planePoints[1][1] << " " << planePoints[1][2] << " ) "
    << "( " << planePoints[2][0] << " " << planePoints[2][1] << " " << planePoints[2][2] << " )";
}


inline Plane3 Plane3_applyTranslation(const Plane3& plane, const Vector3& translation)
{
  Plane3 tmp(plane3_translated(Plane3(plane.normal(), -plane.dist()), translation));
  return Plane3(tmp.normal(), -tmp.dist());
}

inline Plane3 Plane3_applyTransform(const Plane3& plane, const Matrix4& matrix)
{
  Plane3 tmp(plane3_transformed(Plane3(plane.normal(), -plane.dist()), matrix));
  return Plane3(tmp.normal(), -tmp.dist());
}

class FacePlane
{
  PlanePoints m_planepts;
  Plane3 m_planeCached;
  Plane3 m_plane;
public:
  Vector3 m_funcStaticOrigin;

  static EBrushType m_type;

  static bool isDoom3Plane()
  {
    return FacePlane::m_type == eBrushTypeDoom3 || FacePlane::m_type == eBrushTypeQuake4;
  }

  class SavedState
  {
  public:
    PlanePoints m_planepts;
    Plane3 m_plane;

    SavedState(const FacePlane& facePlane)
    {
      if(facePlane.isDoom3Plane())
      {
        m_plane = facePlane.m_plane;
      }
      else
      {
        planepts_assign(m_planepts, facePlane.planePoints());
      }
    }

    void exportState(FacePlane& facePlane) const
    {
      if(facePlane.isDoom3Plane())
      {
        facePlane.m_plane = m_plane;
        facePlane.updateTranslated();
      }
      else
      {
        planepts_assign(facePlane.planePoints(), m_planepts);
        facePlane.MakePlane();
      }
    }
  };

  FacePlane() : m_funcStaticOrigin(0, 0, 0)
  {
  }
  FacePlane(const FacePlane& other) : m_funcStaticOrigin(0, 0, 0)
  {
    if(!isDoom3Plane())
    {
      planepts_assign(m_planepts, other.m_planepts);
      MakePlane();
    }
    else
    {
      m_plane = other.m_plane;
      updateTranslated();
    }
  }

  void MakePlane()
  {
    if(!isDoom3Plane())
    {
#if 0
      if(check_plane_is_integer(m_planepts))
      {
        globalErrorStream() << "non-integer planepts: ";
        planepts_print(m_planepts, globalErrorStream());
        globalErrorStream() << "\n";
      }
#endif
      m_planeCached = plane3_for_points(m_planepts);
    }
  }

  void reverse()
  {
    if(!isDoom3Plane())
    {
      vector3_swap(m_planepts[0], m_planepts[2]);
      MakePlane();
    }
    else
    {
      m_planeCached = plane3_flipped(m_plane);
      updateSource();
    }
  }
  void transform(const Matrix4& matrix, bool mirror)
  {
    if(!isDoom3Plane())
    {

#if 0
      bool off = check_plane_is_integer(planePoints());
#endif

      matrix4_transform_point(matrix, m_planepts[0]);
      matrix4_transform_point(matrix, m_planepts[1]);
      matrix4_transform_point(matrix, m_planepts[2]);

      if(mirror)
      {
        reverse();
      }

#if 0
      if(check_plane_is_integer(planePoints()))
      {
        if(!off)
        {
          globalErrorStream() << "caused by transform\n";
        }
      }
#endif
      MakePlane();
    }
    else
    {
      m_planeCached = Plane3_applyTransform(m_planeCached, matrix);
      updateSource();
    }
  }
  void offset(float offset)
  {
    if(!isDoom3Plane())
    {
      Vector3 move(vector3_scaled(m_planeCached.normal(), -offset));

      vector3_subtract(m_planepts[0], move);
      vector3_subtract(m_planepts[1], move);
      vector3_subtract(m_planepts[2], move);

      MakePlane();
    }
    else
    {
      m_planeCached.d += offset;
      updateSource();
    }
  }

  void updateTranslated()
  {
    m_planeCached = Plane3_applyTranslation(m_plane, m_funcStaticOrigin);
  }
  void updateSource()
  {
    m_plane = Plane3_applyTranslation(m_planeCached, vector3_negated(m_funcStaticOrigin));
  }


  PlanePoints& planePoints()
  {
    return m_planepts;
  }
  const PlanePoints& planePoints() const
  {
    return m_planepts;
  }
  const Plane3& plane3() const
  {
    return m_planeCached;
  }
  void setDoom3Plane(const Plane3& plane)
  {
    m_plane = plane;
    updateTranslated();
  }
  const Plane3& getDoom3Plane() const
  {
    return m_plane;
  }

  void copy(const FacePlane& other)
  {
    if(!isDoom3Plane())
    {
      planepts_assign(m_planepts, other.m_planepts);
      MakePlane();
    }
    else
    {
      m_planeCached = other.m_plane;
      updateSource();
    }
  }
  void copy(const Vector3& p0, const Vector3& p1, const Vector3& p2)
  {
    if(!isDoom3Plane())
    {
      m_planepts[0] = p0;
      m_planepts[1] = p1;
      m_planepts[2] = p2;
      MakePlane();
    }
    else
    {
      m_planeCached = plane3_for_points(p2, p1, p0);
      updateSource();
    }
  }
};

inline void Winding_testSelect(Winding& winding, SelectionTest& test, SelectionIntersection& best)
{
  test.TestPolygon(VertexPointer(reinterpret_cast<VertexPointer::pointer>(&winding.points.data()->vertex), sizeof(WindingVertex)), winding.numpoints, best);
}

const double GRID_MIN = 0.125;

inline double quantiseInteger(double f)
{
  return float_to_integer(f);
}

inline double quantiseFloating(double f)
{
  return float_snapped(f, 1.f / (1 << 16));
}

typedef double (*QuantiseFunc)(double f);

class Face;

class FaceFilter
{
public:
  virtual bool filter(const Face& face) const = 0;
};

bool face_filtered(Face& face);

void Brush_addTextureChangedCallback(const SignalHandler& callback);
void Brush_textureChanged();


extern bool g_brush_texturelock_enabled;

class FaceObserver
{
public:
  virtual void planeChanged() = 0;
  virtual void connectivityChanged() = 0;
  virtual void shaderChanged() = 0;
  virtual void evaluateTransform() = 0;
};

class Face :
public OpenGLRenderable,
public Filterable,
public Undoable,
public FaceShaderObserver
{
  std::size_t m_refcount;

  class SavedState : public UndoMemento
  {
  public:
    FacePlane::SavedState m_planeState;
    FaceTexdef::SavedState m_texdefState;
    FaceShader::SavedState m_shaderState;

    SavedState(const Face& face) : m_planeState(face.getPlane()), m_texdefState(face.getTexdef()), m_shaderState(face.getShader())
    {
    }

    void exportState(Face& face) const
    {
      m_planeState.exportState(face.getPlane());
      m_shaderState.exportState(face.getShader());
      m_texdefState.exportState(face.getTexdef());
    }

    void release()
    {
      delete this;
    }
  };

public:
  static QuantiseFunc m_quantise;
  static EBrushType m_type;

  PlanePoints m_move_planepts;
  PlanePoints m_move_planeptsTransformed;
private:
  FacePlane m_plane;
  FacePlane m_planeTransformed;
  FaceShader m_shader;
  FaceTexdef m_texdef;
  TextureProjection m_texdefTransformed;

  Winding m_winding;
  Vector3 m_centroid;
  bool m_filtered;

  FaceObserver* m_observer;
  UndoObserver* m_undoable_observer;
  MapFile* m_map;

  // assignment not supported
  Face& operator=(const Face& other);
  // copy-construction not supported
  Face(const Face& other);

public:

  Face(FaceObserver* observer) :
    m_refcount(0),
    m_shader(texdef_name_default()),
    m_texdef(m_shader, TextureProjection(), false),
    m_filtered(false),
    m_observer(observer),
    m_undoable_observer(0),
    m_map(0)
  {
    m_shader.attach(*this);
    m_plane.copy(Vector3(0, 0, 0), Vector3(64, 0, 0), Vector3(0, 64, 0));
    m_texdef.setBasis(m_plane.plane3().normal());
    planeChanged();
  }
  Face(
    const Vector3& p0,
    const Vector3& p1,
    const Vector3& p2,
    const char* shader,
    const TextureProjection& projection,
    FaceObserver* observer
  ) :
    m_refcount(0),
    m_shader(shader),
    m_texdef(m_shader, projection),
    m_observer(observer),
    m_undoable_observer(0),
    m_map(0)
  {
    m_shader.attach(*this);
    m_plane.copy(p0, p1, p2);
    m_texdef.setBasis(m_plane.plane3().normal());
    planeChanged();
    updateFiltered();
  }
  Face(const Face& other, FaceObserver* observer) :
    m_refcount(0),
    m_shader(other.m_shader.getShader(), other.m_shader.m_flags),
    m_texdef(m_shader, other.getTexdef().normalised()),
    m_observer(observer),
    m_undoable_observer(0),
    m_map(0)
  {
    m_shader.attach(*this);
    m_plane.copy(other.m_plane);
    planepts_assign(m_move_planepts, other.m_move_planepts);
    m_texdef.setBasis(m_plane.plane3().normal());
    planeChanged();
    updateFiltered();
  }
  ~Face()
  {
    m_shader.detach(*this);
  }

  void planeChanged()
  {
    revertTransform();
    m_observer->planeChanged();
  }

  void realiseShader()
  {
    m_observer->shaderChanged();
  }
  void unrealiseShader()
  {
  }

  void instanceAttach(MapFile* map)
  {
    m_shader.instanceAttach();
    m_map = map;
    m_undoable_observer = GlobalUndoSystem().observer(this);
    GlobalFilterSystem().registerFilterable(*this);
  }
  void instanceDetach(MapFile* map)
  {
    GlobalFilterSystem().unregisterFilterable(*this);
    m_undoable_observer = 0;
    GlobalUndoSystem().release(this);
    m_map = 0;
    m_shader.instanceDetach();
  }

  void render(RenderStateFlags state) const
  {
    Winding_Draw(m_winding, m_planeTransformed.plane3().normal(), state);
  }

  void updateFiltered()
  {
    m_filtered = face_filtered(*this);
  }
  bool isFiltered() const
  {
    return m_filtered;
  }

  void undoSave()
  {
    if(m_map != 0)
    {
      m_map->changed();
    }
    if(m_undoable_observer != 0)
    {
      m_undoable_observer->save(this);
    }
  }

  // undoable
  UndoMemento* exportState() const
  {
    return new SavedState(*this);
  }
  void importState(const UndoMemento* data)
  {
    undoSave();

    static_cast<const SavedState*>(data)->exportState(*this);

    planeChanged();
    m_observer->connectivityChanged();
    texdefChanged();
    m_observer->shaderChanged();
    updateFiltered();
  }

  void IncRef()
  {
    ++m_refcount;
  }
  void DecRef()
  {
    if(--m_refcount == 0)
      delete this;
  }

  void flipWinding()
  {
    m_plane.reverse();
    planeChanged();
  }

  bool intersectVolume(const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    return volume.TestPlane(Plane3(plane3().normal(), -plane3().dist()), localToWorld);
  }

  void render(Renderer& renderer, const Matrix4& localToWorld) const
  {
    renderer.SetState(m_shader.state(), Renderer::eFullMaterials);
    renderer.addRenderable(*this, localToWorld);
  }

  void transform(const Matrix4& matrix, bool mirror)
  {
    if(g_brush_texturelock_enabled)
    {
      Texdef_transformLocked(m_texdefTransformed, m_shader.width(), m_shader.height(), m_plane.plane3(), matrix);
    }

    m_planeTransformed.transform(matrix, mirror);

#if 0
    ASSERT_MESSAGE(projectionaxis_for_normal(normal) == projectionaxis_for_normal(plane3().normal()), "bleh");
#endif
    m_observer->planeChanged();
  }

  void assign_planepts(const PlanePoints planepts)
  {
    m_planeTransformed.copy(planepts[0], planepts[1], planepts[2]);
    m_observer->planeChanged();
  }

  /// \brief Reverts the transformable state of the brush to identity. 
  void revertTransform()
  {
    m_planeTransformed = m_plane;
    planepts_assign(m_move_planeptsTransformed, m_move_planepts);
    m_texdefTransformed = m_texdef.m_projection;
  }
  void freezeTransform()
  {
    undoSave();
    m_plane = m_planeTransformed;
    planepts_assign(m_move_planepts, m_move_planeptsTransformed);
    m_texdef.m_projection = m_texdefTransformed;
  }

  void update_move_planepts_vertex(std::size_t index, PlanePoints planePoints)
  {
    std::size_t numpoints = getWinding().numpoints;
    ASSERT_MESSAGE(index < numpoints, "update_move_planepts_vertex: invalid index");

    std::size_t opposite = Winding_Opposite(getWinding(), index);
    std::size_t adjacent = Winding_wrap(getWinding(), opposite+numpoints-1);
    planePoints[0] = getWinding()[opposite].vertex;
    planePoints[1] = getWinding()[index].vertex;
    planePoints[2] = getWinding()[adjacent].vertex;
    // winding points are very inaccurate, so they must be quantised before using them to generate the face-plane
    planepts_quantise(planePoints, GRID_MIN);
  }

  void snapto(float snap)
  {
    if(contributes())
    {
#if 0
      ASSERT_MESSAGE(plane3_valid(m_plane.plane3()), "invalid plane before snap to grid");
      planepts_snap(m_plane.planePoints(), snap);
      ASSERT_MESSAGE(plane3_valid(m_plane.plane3()), "invalid plane after snap to grid");
#else
      PlanePoints planePoints;
      update_move_planepts_vertex(0, planePoints);
      vector3_snap(planePoints[0], snap);
      vector3_snap(planePoints[1], snap);
      vector3_snap(planePoints[2], snap);
      assign_planepts(planePoints);
      freezeTransform();
#endif
      SceneChangeNotify();
      if(!plane3_valid(m_plane.plane3()))
      {
        globalErrorStream() << "WARNING: invalid plane after snap to grid\n";
      }
    }
  }

  void testSelect(SelectionTest& test, SelectionIntersection& best)
  {
    Winding_testSelect(m_winding, test, best);
  }

  void testSelect_centroid(SelectionTest& test, SelectionIntersection& best)
  {
    test.TestPoint(m_centroid, best);
  }

  void shaderChanged()
  {
    EmitTextureCoordinates();
    Brush_textureChanged();
    m_observer->shaderChanged();
    updateFiltered();
    SceneChangeNotify();
  }

  const char* GetShader() const
  {
    return m_shader.getShader();
  }
  void SetShader(const char* name)
  {
    undoSave();
    m_shader.setShader(name);
    shaderChanged();
  }

  void revertTexdef()
  {
    m_texdefTransformed = m_texdef.m_projection;
  }
  void texdefChanged()
  {
    revertTexdef();
    EmitTextureCoordinates();
    Brush_textureChanged();
  }

  void GetTexdef(TextureProjection& projection) const
  {
    projection = m_texdef.normalised();
  }
  void SetTexdef(const TextureProjection& projection)
  {
    undoSave();
    m_texdef.setTexdef(projection);
    texdefChanged();
  }

  void GetFlags(ContentsFlagsValue& flags) const
  {
    flags = m_shader.getFlags();
  }
  void SetFlags(const ContentsFlagsValue& flags)
  {
    undoSave();
    m_shader.setFlags(flags);
    m_observer->shaderChanged();
    updateFiltered();
  }

  void ShiftTexdef(float s, float t)
  {
    undoSave();
    m_texdef.shift(s, t);
    texdefChanged();
  }

  void ScaleTexdef(float s, float t)
  {
    undoSave();
    m_texdef.scale(s, t);
    texdefChanged();
  }

  void RotateTexdef(float angle)
  {
    undoSave();
    m_texdef.rotate(angle);
    texdefChanged();
  }

  void FitTexture(float s_repeat, float t_repeat)
  {
    undoSave();
    m_texdef.fit(m_plane.plane3().normal(), m_winding, s_repeat, t_repeat);
    texdefChanged();
  }

  void EmitTextureCoordinates()
  {
    Texdef_EmitTextureCoordinates(m_texdefTransformed, m_shader.width(), m_shader.height(), m_winding, plane3().normal(), g_matrix4_identity);
  }


  const Vector3& centroid() const
  {
    return m_centroid;
  }

  void construct_centroid()
  {
    Winding_Centroid(m_winding, plane3(), m_centroid);
  }

  const Winding& getWinding() const
  {
    return m_winding;
  }
  Winding& getWinding()
  {
    return m_winding;
  }

  const Plane3& plane3() const
  {
    m_observer->evaluateTransform();
    return m_planeTransformed.plane3();
  }
  FacePlane& getPlane()
  {
    return m_plane;
  }
  const FacePlane& getPlane() const
  {
    return m_plane;
  }
  FaceTexdef& getTexdef()
  {
    return m_texdef;
  }
  const FaceTexdef& getTexdef() const
  {
    return m_texdef;
  }
  FaceShader& getShader()
  {
    return m_shader;
  }
  const FaceShader& getShader() const
  {
    return m_shader;
  }

  bool isDetail() const
  {
    return (m_shader.m_flags.m_contentFlags & CONTENTS_DETAIL) != 0;
  }
  void setDetail(bool detail)
  {
    undoSave();
    if(detail && !isDetail())
    {
      m_shader.m_flags.m_contentFlags |= CONTENTS_DETAIL;
    }
    else if(!detail && isDetail())
    {
      m_shader.m_flags.m_contentFlags &= ~CONTENTS_DETAIL;
    }
    m_observer->shaderChanged();
  }

  bool contributes() const
  {
    return m_winding.numpoints > 2;
  }
  bool is_bounded() const
  {
    for(Winding::const_iterator i = m_winding.begin(); i != m_winding.end(); ++i)
    {
      if((*i).adjacent == c_brush_maxFaces)
      {
        return false;
      }
    }
    return true;
  }
};


class FaceVertexId
{
  std::size_t m_face;
  std::size_t m_vertex;

public:
  FaceVertexId(std::size_t face, std::size_t vertex)
    : m_face(face), m_vertex(vertex)
  {
  }

  std::size_t getFace() const
  {
    return m_face;
  }
  std::size_t getVertex() const
  {
    return m_vertex;
  }
};

typedef std::size_t faceIndex_t;

struct EdgeRenderIndices
{
  RenderIndex first;
  RenderIndex second;

  EdgeRenderIndices()
    : first(0), second(0)
  {
  }
  EdgeRenderIndices(const RenderIndex _first, const RenderIndex _second)
    : first(_first), second(_second)
  {
  }
};

struct EdgeFaces
{
  faceIndex_t first;
  faceIndex_t second;

  EdgeFaces()
    : first(c_brush_maxFaces), second(c_brush_maxFaces)
  {
  }
  EdgeFaces(const faceIndex_t _first, const faceIndex_t _second)
    : first(_first), second(_second)
  {
  }
};

class RenderableWireframe : public OpenGLRenderable
{
public:
  void render(RenderStateFlags state) const
  {
#if 1
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(PointVertex), &m_vertices->colour);
    glVertexPointer(3, GL_FLOAT, sizeof(PointVertex), &m_vertices->vertex);
    glDrawElements(GL_LINES, GLsizei(m_size<<1), RenderIndexTypeID, m_faceVertex.data());
#else
    glBegin(GL_LINES);
    for(std::size_t i = 0; i < m_size; ++i)
    {
      glVertex3fv(&m_vertices[m_faceVertex[i].first].vertex.x);
      glVertex3fv(&m_vertices[m_faceVertex[i].second].vertex.x);
    }
    glEnd();
#endif
  }

  Array<EdgeRenderIndices> m_faceVertex;
  std::size_t m_size;
  const PointVertex* m_vertices;
};

class Brush;
typedef std::vector<Brush*> brush_vector_t;

class BrushFilter
{
public:
  virtual bool filter(const Brush& brush) const = 0;
};

bool brush_filtered(Brush& brush);
void add_brush_filter(BrushFilter& filter, int mask, bool invert = false);


/// \brief Returns true if 'self' takes priority when building brush b-rep.
inline bool plane3_inside(const Plane3& self, const Plane3& other)
{
  if(vector3_equal_epsilon(self.normal(), other.normal(), 0.001))
  {
    return self.dist() < other.dist();
  }
  return true;
}

typedef SmartPointer<Face> FaceSmartPointer;
typedef std::vector<FaceSmartPointer> Faces;

/// \brief Returns the unique-id of the edge adjacent to \p faceVertex in the edge-pair for the set of \p faces.
inline FaceVertexId next_edge(const Faces& faces, FaceVertexId faceVertex)
{
  std::size_t adjacent_face = faces[faceVertex.getFace()]->getWinding()[faceVertex.getVertex()].adjacent;
  std::size_t adjacent_vertex = Winding_FindAdjacent(faces[adjacent_face]->getWinding(), faceVertex.getFace());

  ASSERT_MESSAGE(adjacent_vertex != c_brush_maxFaces, "connectivity data invalid");
  if(adjacent_vertex == c_brush_maxFaces)
  {
    return faceVertex;
  }

  return FaceVertexId(adjacent_face, adjacent_vertex);
}

/// \brief Returns the unique-id of the vertex adjacent to \p faceVertex in the vertex-ring for the set of \p faces.
inline FaceVertexId next_vertex(const Faces& faces, FaceVertexId faceVertex)
{
  FaceVertexId nextEdge = next_edge(faces, faceVertex);
  return FaceVertexId(nextEdge.getFace(), Winding_next(faces[nextEdge.getFace()]->getWinding(), nextEdge.getVertex()));
}

class SelectableEdge
{
  Vector3 getEdge() const
  {
    const Winding& winding = getFace().getWinding();
    return vector3_mid(winding[m_faceVertex.getVertex()].vertex, winding[Winding_next(winding, m_faceVertex.getVertex())].vertex);
  }

public:
  Faces& m_faces;
  FaceVertexId m_faceVertex;

  SelectableEdge(Faces& faces, FaceVertexId faceVertex)
    : m_faces(faces), m_faceVertex(faceVertex)
  {
  }
  SelectableEdge& operator=(const SelectableEdge& other)
  {
    m_faceVertex = other.m_faceVertex;
    return *this;
  }

  Face& getFace() const
  {
    return *m_faces[m_faceVertex.getFace()];
  }

  void testSelect(SelectionTest& test, SelectionIntersection& best)
  {
    test.TestPoint(getEdge(), best);
  }
};

class SelectableVertex
{
  Vector3 getVertex() const
  {
    return getFace().getWinding()[m_faceVertex.getVertex()].vertex;
  }

public:
  Faces& m_faces;
  FaceVertexId m_faceVertex;

  SelectableVertex(Faces& faces, FaceVertexId faceVertex)
    : m_faces(faces), m_faceVertex(faceVertex)
  {
  }
  SelectableVertex& operator=(const SelectableVertex& other)
  {
    m_faceVertex = other.m_faceVertex;
    return *this;
  }

  Face& getFace() const
  {
    return *m_faces[m_faceVertex.getFace()];
  }

  void testSelect(SelectionTest& test, SelectionIntersection& best)
  {
    test.TestPoint(getVertex(), best);
  }
};

class BrushObserver
{
public:
  virtual void reserve(std::size_t size) = 0;
  virtual void clear() = 0;
  virtual void push_back(Face& face) = 0;
  virtual void pop_back() = 0;
  virtual void erase(std::size_t index) = 0;
  virtual void connectivityChanged() = 0;

  virtual void edge_clear() = 0;
  virtual void edge_push_back(SelectableEdge& edge) = 0;

  virtual void vertex_clear() = 0;
  virtual void vertex_push_back(SelectableVertex& vertex) = 0;

  virtual void DEBUG_verify() const = 0;
};

class BrushVisitor
{
public:
  virtual void visit(Face& face) const = 0;
};

class Brush :
  public TransformNode,
  public Bounded,
  public Cullable,
  public Snappable,
  public Undoable,
  public FaceObserver,
  public Filterable,
  public Nameable,
  public BrushDoom3
{
private:
  scene::Node* m_node;
  typedef UniqueSet<BrushObserver*> Observers;
  Observers m_observers;
  UndoObserver* m_undoable_observer;
  MapFile* m_map;

  // state
  Faces m_faces;
  // ----

  // cached data compiled from state
  Array<PointVertex> m_faceCentroidPoints;
  RenderablePointArray m_render_faces;

  Array<PointVertex> m_uniqueVertexPoints;
  typedef std::vector<SelectableVertex> SelectableVertices;
  SelectableVertices m_select_vertices;
  RenderablePointArray m_render_vertices;

  Array<PointVertex> m_uniqueEdgePoints;
  typedef std::vector<SelectableEdge> SelectableEdges;
  SelectableEdges m_select_edges;
  RenderablePointArray m_render_edges;

  Array<EdgeRenderIndices> m_edge_indices;
  Array<EdgeFaces> m_edge_faces;

  AABB m_aabb_local;
  // ----

  Callback m_evaluateTransform;
  Callback m_boundsChanged;

  mutable bool m_planeChanged; // b-rep evaluation required
  mutable bool m_transformChanged; // transform evaluation required
  // ----

public:  
  STRING_CONSTANT(Name, "Brush");

  Callback m_lightsChanged;

  // static data
  static Shader* m_state_point;
  // ----

  static EBrushType m_type;
  static double m_maxWorldCoord;

  Brush(scene::Node& node, const Callback& evaluateTransform, const Callback& boundsChanged) :
    m_node(&node),
    m_undoable_observer(0),
    m_map(0),
    m_render_faces(m_faceCentroidPoints, GL_POINTS),
    m_render_vertices(m_uniqueVertexPoints, GL_POINTS),
    m_render_edges(m_uniqueEdgePoints, GL_POINTS),
    m_planeChanged(false),
    m_transformChanged(false),
    m_evaluateTransform(evaluateTransform),
    m_boundsChanged(boundsChanged)
  {
    planeChanged();
  }
  Brush(const Brush& other, scene::Node& node, const Callback& evaluateTransform, const Callback& boundsChanged) :
    m_node(&node),
    m_undoable_observer(0),
    m_map(0),
    m_render_faces(m_faceCentroidPoints, GL_POINTS),
    m_render_vertices(m_uniqueVertexPoints, GL_POINTS),
    m_render_edges(m_uniqueEdgePoints, GL_POINTS),
    m_planeChanged(false),
    m_transformChanged(false),
    m_evaluateTransform(evaluateTransform),
    m_boundsChanged(boundsChanged)
  {
    copy(other);
  }
  Brush(const Brush& other) :
    TransformNode(other),
    Bounded(other),
    Cullable(other),
    Undoable(other),
    FaceObserver(other),
    Filterable(other),
    Nameable(other),
    BrushDoom3(other),
    m_node(0),
    m_undoable_observer(0),
    m_map(0),
    m_render_faces(m_faceCentroidPoints, GL_POINTS),
    m_render_vertices(m_uniqueVertexPoints, GL_POINTS),
    m_render_edges(m_uniqueEdgePoints, GL_POINTS),
    m_planeChanged(false),
    m_transformChanged(false)
  {
    copy(other);
  }
  ~Brush()
  {
    ASSERT_MESSAGE(m_observers.empty(), "Brush::~Brush: observers still attached");
  }

  // assignment not supported
  Brush& operator=(const Brush& other);

  void setDoom3GroupOrigin(const Vector3& origin)
  {
    //globalOutputStream() << "func_static origin before: " << m_funcStaticOrigin << " after: " << origin << "\n";
    for(Faces::iterator i = m_faces.begin(); i != m_faces.end(); ++i)
    {
      (*i)->getPlane().m_funcStaticOrigin = origin;
      (*i)->getPlane().updateTranslated();
      (*i)->planeChanged();
    }
    planeChanged();
  }

  void attach(BrushObserver& observer)
  {
    for(Faces::iterator i = m_faces.begin(); i != m_faces.end(); ++i)
    {
      observer.push_back(*(*i));
    }

    for(SelectableEdges::iterator i = m_select_edges.begin(); i !=m_select_edges.end(); ++i)
    {
      observer.edge_push_back(*i);
    }

    for(SelectableVertices::iterator i = m_select_vertices.begin(); i != m_select_vertices.end(); ++i)
    {
      observer.vertex_push_back(*i);
    }

    m_observers.insert(&observer);
  }
  void detach(BrushObserver& observer)
  {
    m_observers.erase(&observer);
  }

  void forEachFace(const BrushVisitor& visitor) const
  {
    for(Faces::const_iterator i = m_faces.begin(); i != m_faces.end(); ++i)
    {
      visitor.visit(*(*i));
    }
  }

  void forEachFace_instanceAttach(MapFile* map) const
  {
    for(Faces::const_iterator i = m_faces.begin(); i != m_faces.end(); ++i)
    {
      (*i)->instanceAttach(map);
    }
  }
  void forEachFace_instanceDetach(MapFile* map) const
  {
    for(Faces::const_iterator i = m_faces.begin(); i != m_faces.end(); ++i)
    {
      (*i)->instanceDetach(map);
    }
  }

  InstanceCounter m_instanceCounter;
  void instanceAttach(const scene::Path& path)
  {
    if(++m_instanceCounter.m_count == 1)
    {
      m_map = path_find_mapfile(path.begin(), path.end());
      m_undoable_observer = GlobalUndoSystem().observer(this);
      GlobalFilterSystem().registerFilterable(*this);
      forEachFace_instanceAttach(m_map);
    }
    else
    {
      ASSERT_MESSAGE(path_find_mapfile(path.begin(), path.end()) == m_map, "node is instanced across more than one file");
    }
  }
  void instanceDetach(const scene::Path& path)
  {
    if(--m_instanceCounter.m_count == 0)
    {
      forEachFace_instanceDetach(m_map);
      GlobalFilterSystem().unregisterFilterable(*this);
      m_map = 0;
      m_undoable_observer = 0;
      GlobalUndoSystem().release(this);
    }
  }

  // nameable
  const char* name() const
  {
    return "brush";
  }
  void attach(const NameCallback& callback)
  {
  }
  void detach(const NameCallback& callback)
  {
  }

  // filterable
  void updateFiltered()
  {
    if(m_node != 0)
    {
      if(brush_filtered(*this))
      {
        m_node->enable(scene::Node::eFiltered);
      }
      else
      {
        m_node->disable(scene::Node::eFiltered);
      }
    }
  }

  // observer
  void planeChanged()
  {
    m_planeChanged = true;
    aabbChanged();
    m_lightsChanged();
  }
  void shaderChanged()
  {
    updateFiltered();
  }

  void evaluateBRep() const
  {
    if(m_planeChanged)
    {
      m_planeChanged = false;
      const_cast<Brush*>(this)->buildBRep();
    }
  }

  void transformChanged()
  {
    m_transformChanged = true;
    planeChanged();
  }
  typedef MemberCaller<Brush, &Brush::transformChanged> TransformChangedCaller;

  void evaluateTransform()
  {
    if(m_transformChanged)
    {
      m_transformChanged = false;
      revertTransform();
      m_evaluateTransform();
    }
  }
  const Matrix4& localToParent() const
  {
    return g_matrix4_identity;
  }
  void aabbChanged()
  {
    m_boundsChanged();
  }
  const AABB& localAABB() const
  {
    evaluateBRep();
    return m_aabb_local;
  }

  VolumeIntersectionValue intersectVolume(const VolumeTest& test, const Matrix4& localToWorld) const
  {
    return test.TestAABB(m_aabb_local, localToWorld);
  }

  void renderComponents(SelectionSystem::EComponentMode mode, Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    switch(mode)
    {
    case SelectionSystem::eVertex:
      renderer.addRenderable(m_render_vertices, localToWorld);
      break;
    case SelectionSystem::eEdge:
      renderer.addRenderable(m_render_edges, localToWorld);
      break;
    case SelectionSystem::eFace:
      renderer.addRenderable(m_render_faces, localToWorld);
      break;
    default:
      break;
    }
  }

  void transform(const Matrix4& matrix)
  {
    bool mirror = matrix4_handedness(matrix) == MATRIX4_LEFTHANDED;

    for(Faces::iterator i = m_faces.begin(); i != m_faces.end(); ++i)
    {
      (*i)->transform(matrix, mirror);
    }
  }
  void snapto(float snap)
  {
    for(Faces::iterator i = m_faces.begin(); i != m_faces.end(); ++i)
    {
      (*i)->snapto(snap);
    }
  }
  void revertTransform()
  {
    for(Faces::iterator i = m_faces.begin(); i != m_faces.end(); ++i)
    {
      (*i)->revertTransform();
    }
  }
  void freezeTransform()
  {
    for(Faces::iterator i = m_faces.begin(); i != m_faces.end(); ++i)
    {
      (*i)->freezeTransform();
    }
  }

  /// \brief Returns the absolute index of the \p faceVertex.
  std::size_t absoluteIndex(FaceVertexId faceVertex)
  {
    std::size_t index = 0;
    for(std::size_t i = 0; i < faceVertex.getFace(); ++i)
    {
      index += m_faces[i]->getWinding().numpoints;
    }
    return index + faceVertex.getVertex();
  }

  void appendFaces(const Faces& other)
  {
    clear();
    for(Faces::const_iterator i = other.begin(); i != other.end(); ++i)
    {
      push_back(*i);
    }
  }

  /// \brief The undo memento for a brush stores only the list of face references - the faces are not copied.
  class BrushUndoMemento : public UndoMemento
  {
  public:
    BrushUndoMemento(const Faces& faces) : m_faces(faces)
    {
    }
    void release()
    {
      delete this;
    }

    Faces m_faces;
  };

  void undoSave()
  {
    if(m_map != 0)
    {
      m_map->changed();
    }
    if(m_undoable_observer != 0)
    {
      m_undoable_observer->save(this);
    }
  }

  UndoMemento* exportState() const
  {
    return new BrushUndoMemento(m_faces);
  }

  void importState(const UndoMemento* state)
  {
    undoSave();
    appendFaces(static_cast<const BrushUndoMemento*>(state)->m_faces);
    planeChanged();

    for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i)
    {
      (*i)->DEBUG_verify();
    }
  }

  bool isDetail()
  {
    return !m_faces.empty() && m_faces.front()->isDetail();
  }

  /// \brief Appends a copy of \p face to the end of the face list.
  Face* addFace(const Face& face)
  {
    if(m_faces.size() == c_brush_maxFaces)
    {
      return 0;
    }
    undoSave();
    push_back(FaceSmartPointer(new Face(face, this)));
    m_faces.back()->setDetail(isDetail());
    planeChanged();
    return m_faces.back();
  }

  /// \brief Appends a new face constructed from the parameters to the end of the face list.
  Face* addPlane(const Vector3& p0, const Vector3& p1, const Vector3& p2, const char* shader, const TextureProjection& projection)
  {
    if(m_faces.size() == c_brush_maxFaces)
    {
      return 0;
    }
    undoSave();
    push_back(FaceSmartPointer(new Face(p0, p1, p2, shader, projection, this)));
    m_faces.back()->setDetail(isDetail());
    planeChanged();
    return m_faces.back();
  }

  static void constructStatic(EBrushType type)
  {
    m_type = type;
    Face::m_type = type;
    FacePlane::m_type = type;

    g_bp_globals.m_texdefTypeId = TEXDEFTYPEID_QUAKE;
    if(m_type == eBrushTypeQuake3BP || m_type == eBrushTypeDoom3 || m_type == eBrushTypeQuake4)
    {
      g_bp_globals.m_texdefTypeId = TEXDEFTYPEID_BRUSHPRIMITIVES;
      g_brush_texturelock_enabled = true;
    }
    else if(m_type == eBrushTypeHalfLife)
    {
      g_bp_globals.m_texdefTypeId = TEXDEFTYPEID_HALFLIFE;
      g_brush_texturelock_enabled = true;
    }

    Face::m_quantise = (m_type == eBrushTypeQuake) ? quantiseInteger : quantiseFloating;

    m_state_point = GlobalShaderCache().capture("$POINT");
  }
  static void destroyStatic()
  {
    GlobalShaderCache().release("$POINT");
  }

  std::size_t DEBUG_size()
  {
    return m_faces.size();
  }

  typedef Faces::const_iterator const_iterator;

  const_iterator begin() const
  {
    return m_faces.begin();
  }
  const_iterator end() const
  {
    return m_faces.end();
  }

  Face* back()
  {
    return m_faces.back();
  }
  const Face* back() const
  {
    return m_faces.back();
  }
  void reserve(std::size_t count)
  {
    m_faces.reserve(count);
    for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i)
    {
      (*i)->reserve(count);
    }
  }
  void push_back(Faces::value_type face)
  {
    m_faces.push_back(face);
    if(m_instanceCounter.m_count != 0)
    {
      m_faces.back()->instanceAttach(m_map);
    }
    for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i)
    {
      (*i)->push_back(*face);
      (*i)->DEBUG_verify();
    }
  }
  void pop_back()
  {
    if(m_instanceCounter.m_count != 0)
    {
      m_faces.back()->instanceDetach(m_map);
    }
    m_faces.pop_back();
    for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i)
    {
      (*i)->pop_back();
      (*i)->DEBUG_verify();
    }
  }
  void erase(std::size_t index)
  {
    if(m_instanceCounter.m_count != 0)
    {
      m_faces[index]->instanceDetach(m_map);
    }
    m_faces.erase(m_faces.begin() + index);
    for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i)
    {
      (*i)->erase(index);
      (*i)->DEBUG_verify();
    }
  }
  void connectivityChanged()
  {
    for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i)
    {
      (*i)->connectivityChanged();
    }
  }


  void clear()
  {
    undoSave();
    if(m_instanceCounter.m_count != 0)
    {
      forEachFace_instanceDetach(m_map);
    }
    m_faces.clear();
    for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i)
    {
      (*i)->clear();
      (*i)->DEBUG_verify();
    }
  }
  std::size_t size() const
  {
    return m_faces.size();
  }
  bool empty() const
  {
    return m_faces.empty();
  }

  /// \brief Returns true if any face of the brush contributes to the final B-Rep.
  bool hasContributingFaces() const
  {
    for(const_iterator i = begin(); i != end(); ++i)
    {
      if((*i)->contributes())
      {
        return true;
      }
    }
    return false;
  }

  /// \brief Removes faces that do not contribute to the brush. This is useful for cleaning up after CSG operations on the brush.
  /// Note: removal of empty faces is not performed during direct brush manipulations, because it would make a manipulation irreversible if it created an empty face.
  void removeEmptyFaces()
  {
    evaluateBRep();

    {
      std::size_t i = 0;
      while(i < m_faces.size())
      {
        if(!m_faces[i]->contributes())
        {
          erase(i);
          planeChanged();
        }
        else
        {
          ++i;
        }
      }
    }
  }

  /// \brief Constructs \p winding from the intersection of \p plane with the other planes of the brush.
  void windingForClipPlane(Winding& winding, const Plane3& plane) const
  {
    FixedWinding buffer[2];
    bool swap = false;

    // get a poly that covers an effectively infinite area
    Winding_createInfinite(buffer[swap], plane, m_maxWorldCoord + 1);

    // chop the poly by all of the other faces
    {
      for (std::size_t i = 0;  i < m_faces.size(); ++i)
      {
        const Face& clip = *m_faces[i];

        if(plane3_equal(clip.plane3(), plane) 
          || !plane3_valid(clip.plane3()) || !plane_unique(i)
          || plane3_opposing(plane, clip.plane3()))
        {
          continue;
        }

        buffer[!swap].clear();

#if BRUSH_CONNECTIVITY_DEBUG
        globalOutputStream() << "clip vs face: " << i << "\n";
#endif

        {
          // flip the plane, because we want to keep the back side
          Plane3 clipPlane(vector3_negated(clip.plane3().normal()), -clip.plane3().dist());
          Winding_Clip(buffer[swap], plane, clipPlane, i, buffer[!swap]);
        }

#if BRUSH_CONNECTIVITY_DEBUG
        for(FixedWinding::Points::iterator k = buffer[!swap].points.begin(), j = buffer[!swap].points.end() - 1; k != buffer[!swap].points.end(); j = k, ++k)
        {
          if(vector3_length_squared(vector3_subtracted((*k).vertex, (*j).vertex)) < 1)
          {
            globalOutputStream() << "v: " << std::distance(buffer[!swap].points.begin(), j) << " tiny edge adjacent to face " << (*j).adjacent << "\n";
          }
        }
#endif

        //ASSERT_MESSAGE(buffer[!swap].numpoints != 1, "created single-point winding");

        swap = !swap;
      }
    }

    Winding_forFixedWinding(winding, buffer[swap]);

#if BRUSH_CONNECTIVITY_DEBUG
    Winding_printConnectivity(winding);

    for(Winding::iterator i = winding.begin(), j = winding.end() - 1; i != winding.end(); j = i, ++i)
    {
      if(vector3_length_squared(vector3_subtracted((*i).vertex, (*j).vertex)) < 1)
      {
        globalOutputStream() << "v: " << std::distance(winding.begin(), j) << " tiny edge adjacent to face " << (*j).adjacent << "\n";
      }
    }
#endif
  }

  void update_wireframe(RenderableWireframe& wire, const bool* faces_visible) const
  {
    wire.m_faceVertex.resize(m_edge_indices.size());
    wire.m_vertices = m_uniqueVertexPoints.data();
    wire.m_size = 0;
    for(std::size_t i = 0; i < m_edge_faces.size(); ++i)
    {
      if(faces_visible[m_edge_faces[i].first]
        || faces_visible[m_edge_faces[i].second])
      {
        wire.m_faceVertex[wire.m_size++] = m_edge_indices[i];
      }
    }
  }


  void update_faces_wireframe(Array<PointVertex>& wire, const bool* faces_visible) const
  {
    std::size_t count = 0;
    for(std::size_t i = 0; i < m_faceCentroidPoints.size(); ++i)
    {
      if(faces_visible[i])
      {
        ++count;
      }
    }

    wire.resize(count);
    Array<PointVertex>::iterator p = wire.begin();
    for(std::size_t i = 0; i < m_faceCentroidPoints.size(); ++i)
    {
      if(faces_visible[i])
      {
        *p++ = m_faceCentroidPoints[i];
      }
    }
  }

  /// \brief Makes this brush a deep-copy of the \p other.
  void copy(const Brush& other)
  {
    for(Faces::const_iterator i = other.m_faces.begin(); i != other.m_faces.end(); ++i)
    {
      addFace(*(*i));
    }
    planeChanged();
  }

private:
  void edge_push_back(FaceVertexId faceVertex)
  {
    m_select_edges.push_back(SelectableEdge(m_faces, faceVertex));
    for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i)
    {
      (*i)->edge_push_back(m_select_edges.back());
    }
  }
  void edge_clear()
  {
    m_select_edges.clear();
    for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i)
    {
      (*i)->edge_clear();
    }
  }
  void vertex_push_back(FaceVertexId faceVertex)
  {
    m_select_vertices.push_back(SelectableVertex(m_faces, faceVertex));
    for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i)
    {
      (*i)->vertex_push_back(m_select_vertices.back());
    }
  }
  void vertex_clear()
  {
    m_select_vertices.clear();
    for(Observers::iterator i = m_observers.begin(); i != m_observers.end(); ++i)
    {
      (*i)->vertex_clear();
    }
  }

  /// \brief Returns true if the face identified by \p index is preceded by another plane that takes priority over it.
  bool plane_unique(std::size_t index) const
  {
    // duplicate plane
    for(std::size_t i = 0; i < m_faces.size(); ++i)
    {
      if(index != i && !plane3_inside(m_faces[index]->plane3(), m_faces[i]->plane3()))
      {
        return false;
      }
    }
    return true;
  }

  /// \brief Removes edges that are smaller than the tolerance used when generating brush windings.
  void removeDegenerateEdges()
  {
    for (std::size_t i = 0;  i < m_faces.size(); ++i)
    {
      Winding& winding = m_faces[i]->getWinding();
      for(Winding::iterator j = winding.begin(); j != winding.end();)
      {
        std::size_t index = std::distance(winding.begin(), j);
        std::size_t next = Winding_next(winding, index);
        if(Edge_isDegenerate(winding[index].vertex, winding[next].vertex))
        {
#if BRUSH_DEGENERATE_DEBUG
          globalOutputStream() << "Brush::buildWindings: face " << i << ": degenerate edge adjacent to " << winding[index].adjacent << "\n";
#endif
          Winding& other = m_faces[winding[index].adjacent]->getWinding();
          std::size_t adjacent = Winding_FindAdjacent(other, i);
          if(adjacent != c_brush_maxFaces)
          {
            other.erase(other.begin() + adjacent);
          }
          winding.erase(j);
        }
        else
        {
          ++j;
        }
      }
    }
  }

  /// \brief Invalidates faces that have only two vertices in their winding, while preserving edge-connectivity information.
  void removeDegenerateFaces()
  {
    // save adjacency info for degenerate faces
    for (std::size_t i = 0;  i < m_faces.size(); ++i)
    {
      Winding& degen = m_faces[i]->getWinding();
      
      if(degen.numpoints == 2)
      {
#if BRUSH_DEGENERATE_DEBUG
        globalOutputStream() << "Brush::buildWindings: face " << i << ": degenerate winding adjacent to " << degen[0].adjacent << ", " << degen[1].adjacent << "\n";
#endif
        // this is an "edge" face, where the plane touches the edge of the brush
        {
          Winding& winding = m_faces[degen[0].adjacent]->getWinding();
          std::size_t index = Winding_FindAdjacent(winding, i);
          if(index != c_brush_maxFaces)
          {
#if BRUSH_DEGENERATE_DEBUG
            globalOutputStream() << "Brush::buildWindings: face " << degen[0].adjacent << ": remapping adjacent " << winding[index].adjacent << " to " << degen[1].adjacent << "\n";
#endif
            winding[index].adjacent = degen[1].adjacent;
          }
        }

        {
          Winding& winding = m_faces[degen[1].adjacent]->getWinding();
          std::size_t index = Winding_FindAdjacent(winding, i);
          if(index != c_brush_maxFaces)
          {
#if BRUSH_DEGENERATE_DEBUG
            globalOutputStream() << "Brush::buildWindings: face " << degen[1].adjacent << ": remapping adjacent " << winding[index].adjacent << " to " << degen[0].adjacent << "\n";
#endif
            winding[index].adjacent = degen[0].adjacent;
          }
        }

        degen.resize(0);
      }
    }
  }

  /// \brief Removes edges that have the same adjacent-face as their immediate neighbour.
  void removeDuplicateEdges()
  {
    // verify face connectivity graph
    for(std::size_t i = 0; i < m_faces.size(); ++i)
    {
      //if(m_faces[i]->contributes())
      {
        Winding& winding = m_faces[i]->getWinding();
        for(std::size_t j = 0; j != winding.numpoints;)
        {
          std::size_t next = Winding_next(winding, j);
          if(winding[j].adjacent == winding[next].adjacent)
          {
#if BRUSH_DEGENERATE_DEBUG
            globalOutputStream() << "Brush::buildWindings: face " << i << ": removed duplicate edge adjacent to face " << winding[j].adjacent << "\n";
#endif
            winding.erase(winding.begin() + next);
          }
          else
          {
            ++j;
          }
        }
      }
    }
  }

  /// \brief Removes edges that do not have a matching pair in their adjacent-face.
  void verifyConnectivityGraph()
  {
    // verify face connectivity graph
    for(std::size_t i = 0; i < m_faces.size(); ++i)
    {
      //if(m_faces[i]->contributes())
      {
        Winding& winding = m_faces[i]->getWinding();
        for(Winding::iterator j = winding.begin(); j != winding.end();)
        {
#if BRUSH_CONNECTIVITY_DEBUG
          globalOutputStream() << "Brush::buildWindings: face " << i << ": adjacent to face " << (*j).adjacent << "\n";
#endif
          // remove unidirectional graph edges
          if((*j).adjacent == c_brush_maxFaces
            || Winding_FindAdjacent(m_faces[(*j).adjacent]->getWinding(), i) == c_brush_maxFaces)
          {
#if BRUSH_CONNECTIVITY_DEBUG
            globalOutputStream() << "Brush::buildWindings: face " << i << ": removing unidirectional connectivity graph edge adjacent to face " << (*j).adjacent << "\n";
#endif
            winding.erase(j);
          }
          else
          {
            ++j;
          }
        }
      }
    }
  }

  /// \brief Returns true if the brush is a finite volume. A brush without a finite volume extends past the maximum world bounds and is not valid.
  bool isBounded()
  {
    for(const_iterator i = begin(); i != end(); ++i)
    {
      if(!(*i)->is_bounded())
      {
        return false;
      }
    }
    return true;
  }

  /// \brief Constructs the polygon windings for each face of the brush. Also updates the brush bounding-box and face texture-coordinates.
  bool buildWindings()
  {

    {
      m_aabb_local = AABB();

      for (std::size_t i = 0;  i < m_faces.size(); ++i)
      {
        Face& f = *m_faces[i];

        if(!plane3_valid(f.plane3()) || !plane_unique(i))
        {
          f.getWinding().resize(0);
        }
        else
        {
#if BRUSH_CONNECTIVITY_DEBUG
          globalOutputStream() << "face: " << i << "\n";
#endif
          windingForClipPlane(f.getWinding(), f.plane3());

          // update brush bounds
          const Winding& winding = f.getWinding();
          for(Winding::const_iterator i = winding.begin(); i != winding.end(); ++i)
          {
            aabb_extend_by_point_safe(m_aabb_local, (*i).vertex);
          }

          // update texture coordinates
          f.EmitTextureCoordinates();
        }
      }
    }

    bool degenerate = !isBounded();

    if(!degenerate)
    {
      // clean up connectivity information.
      // these cleanups must be applied in a specific order.
      removeDegenerateEdges();
      removeDegenerateFaces();
      removeDuplicateEdges();
      verifyConnectivityGraph();
    }

    return degenerate;
  }

  /// \brief Constructs the face windings and updates anything that depends on them.
  void buildBRep();
};



class FaceInstance;

class FaceInstanceSet
{
  typedef SelectionList<FaceInstance> FaceInstances;
  FaceInstances m_faceInstances;
public:
  void insert(FaceInstance& faceInstance)
  {
    m_faceInstances.append(faceInstance);
  }
  void erase(FaceInstance& faceInstance)
  {
    m_faceInstances.erase(faceInstance);
  }

  template<typename Functor>
  void foreach(Functor functor)
  {
    for(FaceInstances::iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
    {
      functor(*(*i));
    }
  }

  bool empty() const
  {
    return m_faceInstances.empty();
  }
  FaceInstance& last() const
  {
    return m_faceInstances.back();
  }
};

extern FaceInstanceSet g_SelectedFaceInstances;

typedef std::list<std::size_t> VertexSelection;

inline VertexSelection::iterator VertexSelection_find(VertexSelection& self, std::size_t value)
{
  return std::find(self.begin(), self.end(), value);
}

inline VertexSelection::const_iterator VertexSelection_find(const VertexSelection& self, std::size_t value)
{
  return std::find(self.begin(), self.end(), value);
}

inline VertexSelection::iterator VertexSelection_insert(VertexSelection& self, std::size_t value)
{
  VertexSelection::iterator i = VertexSelection_find(self, value);
  if(i == self.end())
  {
    self.push_back(value);
    return --self.end();
  }
  return i;
}
inline void VertexSelection_erase(VertexSelection& self, std::size_t value)
{
  VertexSelection::iterator i = VertexSelection_find(self, value);
  if(i != self.end())
  {
    self.erase(i);
  }
}

inline bool triangle_reversed(std::size_t x, std::size_t y, std::size_t z)
{
  return !((x < y && y < z) || (z < x && x < y) || (y < z && z < x));
}
template<typename Element>
inline Vector3 triangle_cross(const BasicVector3<Element>& x, const BasicVector3<Element> y, const BasicVector3<Element>& z)
{
  return vector3_cross(y - x, z - x);
}
template<typename Element>
inline bool triangles_same_winding(const BasicVector3<Element>& x1, const BasicVector3<Element> y1, const BasicVector3<Element>& z1, const BasicVector3<Element>& x2, const BasicVector3<Element> y2, const BasicVector3<Element>& z2)
{
  return vector3_dot(triangle_cross(x1, y1, z1), triangle_cross(x2, y2, z2)) > 0;
}


typedef const Plane3* PlanePointer;
typedef PlanePointer* PlanesIterator;

class VectorLightList : public LightList
{
  typedef std::vector<const RendererLight*> Lights;
  Lights m_lights;
public:
  void addLight(const RendererLight& light)
  {
    m_lights.push_back(&light);
  }
  void clear()
  {
    m_lights.clear();
  }
  void evaluateLights() const
  {
  }
  void lightsChanged() const
  {
  }
  void forEachLight(const RendererLightCallback& callback) const
  {
    for(Lights::const_iterator i = m_lights.begin(); i != m_lights.end(); ++i)
    {
      callback(*(*i));
    }
  }
};

class FaceInstance
{
  Face* m_face;
  ObservedSelectable m_selectable;
  ObservedSelectable m_selectableVertices;
  ObservedSelectable m_selectableEdges;
  SelectionChangeCallback m_selectionChanged;

  VertexSelection m_vertexSelection;
  VertexSelection m_edgeSelection;

public:
  mutable VectorLightList m_lights;

  FaceInstance(Face& face, const SelectionChangeCallback& observer) :
    m_face(&face),
    m_selectable(SelectedChangedCaller(*this)),
    m_selectableVertices(observer),
    m_selectableEdges(observer),
    m_selectionChanged(observer)
  {
  }
  FaceInstance(const FaceInstance& other) :
    m_face(other.m_face),
    m_selectable(SelectedChangedCaller(*this)),
    m_selectableVertices(other.m_selectableVertices),
    m_selectableEdges(other.m_selectableEdges),
    m_selectionChanged(other.m_selectionChanged)
  {
  }
  FaceInstance& operator=(const FaceInstance& other)
  {
    m_face = other.m_face;
    return *this;
  }

  Face& getFace()
  {
    return *m_face;
  }
  const Face& getFace() const
  {
    return *m_face;
  }

  void selectedChanged(const Selectable& selectable)
  {
    if(selectable.isSelected())
    {
      g_SelectedFaceInstances.insert(*this);
    }
    else
    {
      g_SelectedFaceInstances.erase(*this);
    }
    m_selectionChanged(selectable);
  }
  typedef MemberCaller1<FaceInstance, const Selectable&, &FaceInstance::selectedChanged> SelectedChangedCaller;

  bool selectedVertices() const
  {
    return !m_vertexSelection.empty();
  }
  bool selectedEdges() const
  {
    return !m_edgeSelection.empty();
  }
  bool isSelected() const
  {
    return m_selectable.isSelected();
  }

  bool selectedComponents() const
  {
    return selectedVertices() || selectedEdges() || isSelected();
  }
  bool selectedComponents(SelectionSystem::EComponentMode mode) const
  {
    switch(mode)
    {
    case SelectionSystem::eVertex:
      return selectedVertices();
    case SelectionSystem::eEdge:
      return selectedEdges();
    case SelectionSystem::eFace:
      return isSelected();
    default:
      return false;
    }
  }
  void setSelected(SelectionSystem::EComponentMode mode, bool select)
  {
    switch(mode)
    {
    case SelectionSystem::eFace:
      m_selectable.setSelected(select);
      break;
    case SelectionSystem::eVertex:
      ASSERT_MESSAGE(!select, "select-all not supported");

      m_vertexSelection.clear();
      m_selectableVertices.setSelected(false);
      break;
    case SelectionSystem::eEdge:
      ASSERT_MESSAGE(!select, "select-all not supported");

      m_edgeSelection.clear();
      m_selectableEdges.setSelected(false);
      break;
    default:
      break;
    }
  }

  template<typename Functor>
  void SelectedVertices_foreach(Functor functor) const
  {
    for(VertexSelection::const_iterator i = m_vertexSelection.begin(); i != m_vertexSelection.end(); ++i)
    {
      std::size_t index = Winding_FindAdjacent(getFace().getWinding(), *i);
      if(index != c_brush_maxFaces)
      {
        functor(getFace().getWinding()[index].vertex);
      }
    }
  }
  template<typename Functor>
  void SelectedEdges_foreach(Functor functor) const
  {
    for(VertexSelection::const_iterator i = m_edgeSelection.begin(); i != m_edgeSelection.end(); ++i)
    {
      std::size_t index = Winding_FindAdjacent(getFace().getWinding(), *i);
      if(index != c_brush_maxFaces)
      {
        const Winding& winding = getFace().getWinding();
        std::size_t adjacent = Winding_next(winding, index);
        functor(vector3_mid(winding[index].vertex, winding[adjacent].vertex));
      }
    }
  }
  template<typename Functor>
  void SelectedFaces_foreach(Functor functor) const
  {
    if(isSelected())
    {
      functor(centroid());
    }
  }

  template<typename Functor>
  void SelectedComponents_foreach(Functor functor) const
  {
    SelectedVertices_foreach(functor);
    SelectedEdges_foreach(functor);
    SelectedFaces_foreach(functor);
  }

  void iterate_selected(AABB& aabb) const
  {
    SelectedComponents_foreach(AABBExtendByPoint(aabb));
  }

  class RenderablePointVectorPushBack
  {
    RenderablePointVector& m_points;
  public:
    RenderablePointVectorPushBack(RenderablePointVector& points) : m_points(points)
    {
    }
    void operator()(const Vector3& point) const
    {
      const Colour4b colour_selected(0, 0, 255, 255);
      m_points.push_back(pointvertex_for_windingpoint(point, colour_selected));
    }
  };
  
  void iterate_selected(RenderablePointVector& points) const
  {
    SelectedComponents_foreach(RenderablePointVectorPushBack(points));
  }
  
  bool intersectVolume(const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    return m_face->intersectVolume(volume, localToWorld);
  }

  void render(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    if(!m_face->isFiltered() && m_face->contributes() && intersectVolume(volume, localToWorld))
    {
      renderer.PushState();
      if(selectedComponents())
      {
        renderer.Highlight(Renderer::eFace);
      }
      m_face->render(renderer, localToWorld);
      renderer.PopState();
    }
  }

  void testSelect(SelectionTest& test, SelectionIntersection& best)
  {
    if(!m_face->isFiltered())
    {
      m_face->testSelect(test, best);
    }
  }
  void testSelect(Selector& selector, SelectionTest& test)
  {
    SelectionIntersection best;
    testSelect(test, best);
    if(best.valid())
    {
      Selector_add(selector, m_selectable, best);
    }
  }
  void testSelect_centroid(Selector& selector, SelectionTest& test)
  {
    if(m_face->contributes() && !m_face->isFiltered())
    {
      SelectionIntersection best;
      m_face->testSelect_centroid(test, best);
      if(best.valid())
      {
        Selector_add(selector, m_selectable, best);
      }
    }
  }

  void selectPlane(Selector& selector, const Line& line, PlanesIterator first, PlanesIterator last, const PlaneCallback& selectedPlaneCallback)
  {
    for(Winding::const_iterator i = getFace().getWinding().begin(); i != getFace().getWinding().end(); ++i)
    {
      Vector3 v(vector3_subtracted(line_closest_point(line, (*i).vertex), (*i).vertex));
      double dot = vector3_dot(getFace().plane3().normal(), v);
      if(dot <= 0)
      {
        return;
      }
    }

    Selector_add(selector, m_selectable);

    selectedPlaneCallback(getFace().plane3());
  }
  void selectReversedPlane(Selector& selector, const SelectedPlanes& selectedPlanes)
  {
    if(selectedPlanes.contains(plane3_flipped(getFace().plane3())))
    {
      Selector_add(selector, m_selectable);
    }
  }

  void transformComponents(const Matrix4& matrix)
  {
    if(isSelected())
    {
      m_face->transform(matrix, false);
    }
    if(selectedVertices())
    {
      if(m_vertexSelection.size() == 1)
      {
        matrix4_transform_point(matrix, m_face->m_move_planeptsTransformed[1]);
        m_face->assign_planepts(m_face->m_move_planeptsTransformed);
      }
      else if(m_vertexSelection.size() == 2)
      {
        matrix4_transform_point(matrix, m_face->m_move_planeptsTransformed[1]);
        matrix4_transform_point(matrix, m_face->m_move_planeptsTransformed[2]);
        m_face->assign_planepts(m_face->m_move_planeptsTransformed);
      }
      else if(m_vertexSelection.size() >= 3)
      {
        matrix4_transform_point(matrix, m_face->m_move_planeptsTransformed[0]);
        matrix4_transform_point(matrix, m_face->m_move_planeptsTransformed[1]);
        matrix4_transform_point(matrix, m_face->m_move_planeptsTransformed[2]);
        m_face->assign_planepts(m_face->m_move_planeptsTransformed);
      }
    }
    if(selectedEdges())
    {
      if(m_edgeSelection.size() == 1)
      {
        matrix4_transform_point(matrix, m_face->m_move_planeptsTransformed[0]);
        matrix4_transform_point(matrix, m_face->m_move_planeptsTransformed[1]);
        m_face->assign_planepts(m_face->m_move_planeptsTransformed);
      }
      else if(m_edgeSelection.size() >= 2)
      {
        matrix4_transform_point(matrix, m_face->m_move_planeptsTransformed[0]);
        matrix4_transform_point(matrix, m_face->m_move_planeptsTransformed[1]);
        matrix4_transform_point(matrix, m_face->m_move_planeptsTransformed[2]);
        m_face->assign_planepts(m_face->m_move_planeptsTransformed);
      }
    }
  }

  void snapto(float snap)
  {
    m_face->snapto(snap);
  }

  void snapComponents(float snap)
  {
    if(isSelected())
    {
      snapto(snap);
    }
    if(selectedVertices())
    {
      vector3_snap(m_face->m_move_planepts[0], snap);
      vector3_snap(m_face->m_move_planepts[1], snap);
      vector3_snap(m_face->m_move_planepts[2], snap);
      m_face->assign_planepts(m_face->m_move_planepts);
      planepts_assign(m_face->m_move_planeptsTransformed, m_face->m_move_planepts);
      m_face->freezeTransform();
    }
    if(selectedEdges())
    {
      vector3_snap(m_face->m_move_planepts[0], snap);
      vector3_snap(m_face->m_move_planepts[1], snap);
      vector3_snap(m_face->m_move_planepts[2], snap);
      m_face->assign_planepts(m_face->m_move_planepts);
      planepts_assign(m_face->m_move_planeptsTransformed, m_face->m_move_planepts);
      m_face->freezeTransform();
    }
  }
  void update_move_planepts_vertex(std::size_t index)
  {
    m_face->update_move_planepts_vertex(index, m_face->m_move_planepts);
  }
  void update_move_planepts_vertex2(std::size_t index, std::size_t other)
  {
    const std::size_t numpoints = m_face->getWinding().numpoints;
    ASSERT_MESSAGE(index < numpoints, "select_vertex: invalid index");

    const std::size_t opposite = Winding_Opposite(m_face->getWinding(), index, other);

    if(triangle_reversed(index, other, opposite))
    {
      std::swap(index, other);
    }

    ASSERT_MESSAGE(
      triangles_same_winding(
        m_face->getWinding()[opposite].vertex,
        m_face->getWinding()[index].vertex,
        m_face->getWinding()[other].vertex,
        m_face->getWinding()[0].vertex,
        m_face->getWinding()[1].vertex,
        m_face->getWinding()[2].vertex
      ),
      "update_move_planepts_vertex2: error"
    )

    m_face->m_move_planepts[0] = m_face->getWinding()[opposite].vertex;
    m_face->m_move_planepts[1] = m_face->getWinding()[index].vertex;
    m_face->m_move_planepts[2] = m_face->getWinding()[other].vertex;
    planepts_quantise(m_face->m_move_planepts, GRID_MIN); // winding points are very inaccurate
  }
  void update_selection_vertex()
  {
    if(m_vertexSelection.size() == 0)
    {
      m_selectableVertices.setSelected(false);
    }
    else
    {
      m_selectableVertices.setSelected(true);

      if(m_vertexSelection.size() == 1)
      {
        std::size_t index = Winding_FindAdjacent(getFace().getWinding(), *m_vertexSelection.begin());

        if(index != c_brush_maxFaces)
        {
          update_move_planepts_vertex(index);
        }
      }
      else if(m_vertexSelection.size() == 2)
      {
        std::size_t index = Winding_FindAdjacent(getFace().getWinding(), *m_vertexSelection.begin());
        std::size_t other = Winding_FindAdjacent(getFace().getWinding(), *(++m_vertexSelection.begin()));

        if(index != c_brush_maxFaces
          && other != c_brush_maxFaces)
        {
          update_move_planepts_vertex2(index, other);
        }
      }
    }
  }
  void select_vertex(std::size_t index, bool select)
  {
    if(select)
    {
      VertexSelection_insert(m_vertexSelection, getFace().getWinding()[index].adjacent);
    }
    else
    {
      VertexSelection_erase(m_vertexSelection, getFace().getWinding()[index].adjacent);
    }

    SceneChangeNotify();
    update_selection_vertex();
  }

  bool selected_vertex(std::size_t index) const
  {
    return VertexSelection_find(m_vertexSelection, getFace().getWinding()[index].adjacent) != m_vertexSelection.end();
  }

  void update_move_planepts_edge(std::size_t index)
  {
    std::size_t numpoints = m_face->getWinding().numpoints;
    ASSERT_MESSAGE(index < numpoints, "select_edge: invalid index");

    std::size_t adjacent = Winding_next(m_face->getWinding(), index);
    std::size_t opposite = Winding_Opposite(m_face->getWinding(), index);
    m_face->m_move_planepts[0] = m_face->getWinding()[index].vertex;
    m_face->m_move_planepts[1] = m_face->getWinding()[adjacent].vertex;
    m_face->m_move_planepts[2] = m_face->getWinding()[opposite].vertex;
    planepts_quantise(m_face->m_move_planepts, GRID_MIN); // winding points are very inaccurate
  }
  void update_selection_edge()
  {
    if(m_edgeSelection.size() == 0)
    {
      m_selectableEdges.setSelected(false);
    }
    else
    {
      m_selectableEdges.setSelected(true);

      if(m_edgeSelection.size() == 1)
      {
        std::size_t index = Winding_FindAdjacent(getFace().getWinding(), *m_edgeSelection.begin());

        if(index != c_brush_maxFaces)
        {
          update_move_planepts_edge(index);
        }
      }
    }
  }
  void select_edge(std::size_t index, bool select)
  {
    if(select)
    {
      VertexSelection_insert(m_edgeSelection, getFace().getWinding()[index].adjacent);
    }
    else
    {
      VertexSelection_erase(m_edgeSelection, getFace().getWinding()[index].adjacent);
    }

    SceneChangeNotify();
    update_selection_edge();
  }

  bool selected_edge(std::size_t index) const
  {
    return VertexSelection_find(m_edgeSelection, getFace().getWinding()[index].adjacent) != m_edgeSelection.end();
  }

  const Vector3& centroid() const
  {
    return m_face->centroid();
  }

  void connectivityChanged()
  {
    // This occurs when a face is added or removed.
    // The current vertex and edge selections no longer valid and must be cleared.
    m_vertexSelection.clear();
    m_selectableVertices.setSelected(false);
    m_edgeSelection.clear();
    m_selectableEdges.setSelected(false);
  }
};

class BrushClipPlane : public OpenGLRenderable
{
  Plane3 m_plane;
  Winding m_winding;
  static Shader* m_state;
public:
  static void constructStatic()
  {
    m_state = GlobalShaderCache().capture("$CLIPPER_OVERLAY");
  }
  static void destroyStatic()
  {
    GlobalShaderCache().release("$CLIPPER_OVERLAY");
  }

  void setPlane(const Brush& brush, const Plane3& plane)
  {
    m_plane = plane;
    if(plane3_valid(m_plane))
    {
      brush.windingForClipPlane(m_winding, m_plane);
    }
    else
    {
      m_winding.resize(0);
    }
  }

  void render(RenderStateFlags state) const
  {
    if((state & RENDER_FILL) != 0)
    {
      Winding_Draw(m_winding, m_plane.normal(), state);
    }
    else
    {
      Winding_DrawWireframe(m_winding);
    }
  }

  void render(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    renderer.SetState(m_state, Renderer::eWireframeOnly);
    renderer.SetState(m_state, Renderer::eFullMaterials);
    renderer.addRenderable(*this, localToWorld);
  }
};

inline void Face_addLight(const FaceInstance& face, const Matrix4& localToWorld, const RendererLight& light)
{
  const Plane3& facePlane = face.getFace().plane3();
  const Vector3& origin = light.aabb().origin;
  Plane3 tmp(plane3_transformed(Plane3(facePlane.normal(), -facePlane.dist()), localToWorld));
  if(!plane3_test_point(tmp, origin)
    || !plane3_test_point(tmp, vector3_added(origin, light.offset())))
  {
    face.m_lights.addLight(light);
  }
}



typedef std::vector<FaceInstance> FaceInstances;

class EdgeInstance : public Selectable
{
  FaceInstances& m_faceInstances;
  SelectableEdge* m_edge;

  void select_edge(bool select)
  {
    FaceVertexId faceVertex = m_edge->m_faceVertex;
    m_faceInstances[faceVertex.getFace()].select_edge(faceVertex.getVertex(), select);
    faceVertex = next_edge(m_edge->m_faces, faceVertex);
    m_faceInstances[faceVertex.getFace()].select_edge(faceVertex.getVertex(), select);
  }
  bool selected_edge() const
  {
    FaceVertexId faceVertex = m_edge->m_faceVertex;
    if(!m_faceInstances[faceVertex.getFace()].selected_edge(faceVertex.getVertex()))
    {
      return false;
    }
    faceVertex = next_edge(m_edge->m_faces, faceVertex);
    if(!m_faceInstances[faceVertex.getFace()].selected_edge(faceVertex.getVertex()))
    {
      return false;
    }

    return true;
  }

public:
  EdgeInstance(FaceInstances& faceInstances, SelectableEdge& edge)
    : m_faceInstances(faceInstances), m_edge(&edge)
  {
  }
  EdgeInstance& operator=(const EdgeInstance& other)
  {
    m_edge = other.m_edge;
    return *this;
  }

  void setSelected(bool select)
  {
    select_edge(select);
  }
  bool isSelected() const
  {
    return selected_edge();
  }


  void testSelect(Selector& selector, SelectionTest& test)
  {
    SelectionIntersection best;
    m_edge->testSelect(test, best);
    if(best.valid())
    {
      Selector_add(selector, *this, best);
    }
  }
};

class VertexInstance : public Selectable
{
  FaceInstances& m_faceInstances;
  SelectableVertex* m_vertex;

  void select_vertex(bool select)
  {
    FaceVertexId faceVertex = m_vertex->m_faceVertex;
    do
    {
      m_faceInstances[faceVertex.getFace()].select_vertex(faceVertex.getVertex(), select);
      faceVertex = next_vertex(m_vertex->m_faces, faceVertex);
    }
    while(faceVertex.getFace() != m_vertex->m_faceVertex.getFace());
  }
  bool selected_vertex() const
  {
    FaceVertexId faceVertex = m_vertex->m_faceVertex;
    do
    {
      if(!m_faceInstances[faceVertex.getFace()].selected_vertex(faceVertex.getVertex()))
      {
        return false;
      }
      faceVertex = next_vertex(m_vertex->m_faces, faceVertex);
    }
    while(faceVertex.getFace() != m_vertex->m_faceVertex.getFace());
    return true;
  }

public:
  VertexInstance(FaceInstances& faceInstances, SelectableVertex& vertex)
    : m_faceInstances(faceInstances), m_vertex(&vertex)
  {
  }
  VertexInstance& operator=(const VertexInstance& other)
  {
    m_vertex = other.m_vertex;
    return *this;
  }

  void setSelected(bool select)
  {
    select_vertex(select);
  }
  bool isSelected() const
  {
    return selected_vertex();
  }

  void testSelect(Selector& selector, SelectionTest& test)
  {
    SelectionIntersection best;
    m_vertex->testSelect(test, best);
    if(best.valid())
    {
      Selector_add(selector, *this, best);
    }
  }
};

class BrushInstanceVisitor
{
public:
  virtual void visit(FaceInstance& face) const = 0;
};

class BrushInstance :
public BrushObserver,
public scene::Instance,
public Selectable,
public Renderable,
public SelectionTestable,
public ComponentSelectionTestable,
public ComponentEditable,
public ComponentSnappable,
public PlaneSelectable,
public LightCullable
{
  class TypeCasts
  {
    InstanceTypeCastTable m_casts;
  public:
    TypeCasts()
    {
      InstanceStaticCast<BrushInstance, Selectable>::install(m_casts);
      InstanceContainedCast<BrushInstance, Bounded>::install(m_casts);
      InstanceContainedCast<BrushInstance, Cullable>::install(m_casts);
      InstanceStaticCast<BrushInstance, Renderable>::install(m_casts);
      InstanceStaticCast<BrushInstance, SelectionTestable>::install(m_casts);
      InstanceStaticCast<BrushInstance, ComponentSelectionTestable>::install(m_casts);
      InstanceStaticCast<BrushInstance, ComponentEditable>::install(m_casts);
      InstanceStaticCast<BrushInstance, ComponentSnappable>::install(m_casts);
      InstanceStaticCast<BrushInstance, PlaneSelectable>::install(m_casts);
      InstanceIdentityCast<BrushInstance>::install(m_casts);
      InstanceContainedCast<BrushInstance, Transformable>::install(m_casts);
    }
    InstanceTypeCastTable& get()
    {
      return m_casts;
    }
  };


  Brush& m_brush;

  FaceInstances m_faceInstances;

  typedef std::vector<EdgeInstance> EdgeInstances;
  EdgeInstances m_edgeInstances;
  typedef std::vector<VertexInstance> VertexInstances;
  VertexInstances m_vertexInstances;

  ObservedSelectable m_selectable;

  mutable RenderableWireframe m_render_wireframe;
  mutable RenderablePointVector m_render_selected;
  mutable AABB m_aabb_component;
  mutable Array<PointVertex> m_faceCentroidPointsCulled;
  RenderablePointArray m_render_faces_wireframe;
  mutable bool m_viewChanged; // requires re-evaluation of view-dependent cached data

  BrushClipPlane m_clipPlane;

  static Shader* m_state_selpoint;

  const LightList* m_lightList;

  TransformModifier m_transform;

  BrushInstance(const BrushInstance& other); // NOT COPYABLE
  BrushInstance& operator=(const BrushInstance& other); // NOT ASSIGNABLE
public:
  static Counter* m_counter;

  typedef LazyStatic<TypeCasts> StaticTypeCasts;

  void lightsChanged()
  {
    m_lightList->lightsChanged();
  }
  typedef MemberCaller<BrushInstance, &BrushInstance::lightsChanged> LightsChangedCaller;

  STRING_CONSTANT(Name, "BrushInstance");

  BrushInstance(const scene::Path& path, scene::Instance* parent, Brush& brush) :
    Instance(path, parent, this, StaticTypeCasts::instance().get()),
    m_brush(brush),
    m_selectable(SelectedChangedCaller(*this)),
    m_render_selected(GL_POINTS),
    m_render_faces_wireframe(m_faceCentroidPointsCulled, GL_POINTS),
    m_viewChanged(false),
    m_transform(Brush::TransformChangedCaller(m_brush), ApplyTransformCaller(*this))
  {
    m_brush.instanceAttach(Instance::path());
    m_brush.attach(*this);
    m_counter->increment();

    m_lightList = &GlobalShaderCache().attach(*this);
    m_brush.m_lightsChanged = LightsChangedCaller(*this); ///\todo Make this work with instancing.

    Instance::setTransformChangedCallback(LightsChangedCaller(*this));
  }
  ~BrushInstance()
  {
    Instance::setTransformChangedCallback(Callback());

    m_brush.m_lightsChanged = Callback();
    GlobalShaderCache().detach(*this);

    m_counter->decrement();
    m_brush.detach(*this);
    m_brush.instanceDetach(Instance::path());
  }

  Brush& getBrush()
  {
    return m_brush;
  }
  const Brush& getBrush() const
  {
    return m_brush;
  }

  Bounded& get(NullType<Bounded>)
  {
    return m_brush;
  }
  Cullable& get(NullType<Cullable>)
  {
    return m_brush;
  }
  Transformable& get(NullType<Transformable>)
  {
    return m_transform;
  }

  void selectedChanged(const Selectable& selectable)
  {
    GlobalSelectionSystem().getObserver(SelectionSystem::ePrimitive)(selectable);
    GlobalSelectionSystem().onSelectedChanged(*this, selectable);

    Instance::selectedChanged();
  }
  typedef MemberCaller1<BrushInstance, const Selectable&, &BrushInstance::selectedChanged> SelectedChangedCaller;

  void selectedChangedComponent(const Selectable& selectable)
  {
    GlobalSelectionSystem().getObserver(SelectionSystem::eComponent)(selectable);
    GlobalSelectionSystem().onComponentSelection(*this, selectable);
  }
  typedef MemberCaller1<BrushInstance, const Selectable&, &BrushInstance::selectedChangedComponent> SelectedChangedComponentCaller;

  const BrushInstanceVisitor& forEachFaceInstance(const BrushInstanceVisitor& visitor)
  {
    for(FaceInstances::iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
    {
      visitor.visit(*i);
    }
    return visitor;
  }

  static void constructStatic()
  {
    m_state_selpoint = GlobalShaderCache().capture("$SELPOINT");
  }
  static void destroyStatic()
  {
    GlobalShaderCache().release("$SELPOINT");
  }

  void clear()
  {
    m_faceInstances.clear();
  }
  void reserve(std::size_t size)
  {
    m_faceInstances.reserve(size);
  }

  void push_back(Face& face)
  {
    m_faceInstances.push_back(FaceInstance(face, SelectedChangedComponentCaller(*this)));
  }
  void pop_back()
  {
    ASSERT_MESSAGE(!m_faceInstances.empty(), "erasing invalid element");
    m_faceInstances.pop_back();
  }
  void erase(std::size_t index)
  {
    ASSERT_MESSAGE(index < m_faceInstances.size(), "erasing invalid element");
    m_faceInstances.erase(m_faceInstances.begin() + index);
  }
  void connectivityChanged()
  {
    for(FaceInstances::iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
    {
      (*i).connectivityChanged();
    }
  }

  void edge_clear()
  {
    m_edgeInstances.clear();
  }
  void edge_push_back(SelectableEdge& edge)
  {
    m_edgeInstances.push_back(EdgeInstance(m_faceInstances, edge));
  }

  void vertex_clear()
  {
    m_vertexInstances.clear();
  }
  void vertex_push_back(SelectableVertex& vertex)
  {
    m_vertexInstances.push_back(VertexInstance(m_faceInstances, vertex));
  }

  void DEBUG_verify() const
  {
    ASSERT_MESSAGE(m_faceInstances.size() == m_brush.DEBUG_size(), "FATAL: mismatch");
  }

  bool isSelected() const
  {
    return m_selectable.isSelected();
  }
  void setSelected(bool select)
  {
    m_selectable.setSelected(select);
  }

  void update_selected() const
  {
    m_render_selected.clear();
    for(FaceInstances::const_iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
    {
      if((*i).getFace().contributes())
      {
        (*i).iterate_selected(m_render_selected);
      }
    }
  }

  void evaluateViewDependent(const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    if(m_viewChanged)
    {
      m_viewChanged = false;

      bool faces_visible[c_brush_maxFaces];
      {
        bool* j = faces_visible;
        for(FaceInstances::const_iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i, ++j)
        {
          *j = (*i).intersectVolume(volume, localToWorld);
        }
      }

      m_brush.update_wireframe(m_render_wireframe, faces_visible);
      m_brush.update_faces_wireframe(m_faceCentroidPointsCulled, faces_visible);
    }
  }

  void renderComponentsSelected(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    m_brush.evaluateBRep();

    update_selected();
    if(!m_render_selected.empty())
    {
      renderer.Highlight(Renderer::ePrimitive, false);
      renderer.SetState(m_state_selpoint, Renderer::eWireframeOnly);
      renderer.SetState(m_state_selpoint, Renderer::eFullMaterials);
      renderer.addRenderable(m_render_selected, localToWorld);
    }
  }

  void renderComponents(Renderer& renderer, const VolumeTest& volume) const
  {
    m_brush.evaluateBRep();

    const Matrix4& localToWorld = Instance::localToWorld();

    renderer.SetState(m_brush.m_state_point, Renderer::eWireframeOnly);
    renderer.SetState(m_brush.m_state_point, Renderer::eFullMaterials);

    if(volume.fill() && GlobalSelectionSystem().ComponentMode() == SelectionSystem::eFace)
    {
      evaluateViewDependent(volume, localToWorld);
      renderer.addRenderable(m_render_faces_wireframe, localToWorld);
    }
    else
    {
      m_brush.renderComponents(GlobalSelectionSystem().ComponentMode(), renderer, volume, localToWorld);
    }
  }

  void renderClipPlane(Renderer& renderer, const VolumeTest& volume) const
  {
    if(GlobalSelectionSystem().ManipulatorMode() == SelectionSystem::eClip && isSelected())
    {
      m_clipPlane.render(renderer, volume, localToWorld());
    }
  }

  void renderCommon(Renderer& renderer, const VolumeTest& volume) const
  {
    bool componentMode = GlobalSelectionSystem().Mode() == SelectionSystem::eComponent;
    
    if(componentMode && isSelected())
    {
      renderComponents(renderer, volume);
    }
    
    if(parentSelected())
    {
      if(!componentMode)
      {
        renderer.Highlight(Renderer::eFace);
      }
      renderer.Highlight(Renderer::ePrimitive);
    }
  }

  void renderSolid(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    //renderCommon(renderer, volume);

    m_lightList->evaluateLights();

    for(FaceInstances::const_iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
    {
      renderer.setLights((*i).m_lights);
      (*i).render(renderer, volume, localToWorld);
    }

    renderComponentsSelected(renderer, volume, localToWorld);
  }

  void renderWireframe(Renderer& renderer, const VolumeTest& volume, const Matrix4& localToWorld) const
  {
    //renderCommon(renderer, volume);

    evaluateViewDependent(volume, localToWorld);

    if(m_render_wireframe.m_size != 0)
    {
      renderer.addRenderable(m_render_wireframe, localToWorld);
    }

    renderComponentsSelected(renderer, volume, localToWorld);
  }

  void renderSolid(Renderer& renderer, const VolumeTest& volume) const
  {
    m_brush.evaluateBRep();

    renderClipPlane(renderer, volume);

    renderSolid(renderer, volume, localToWorld());
  }

  void renderWireframe(Renderer& renderer, const VolumeTest& volume) const
  {
    m_brush.evaluateBRep();

    renderClipPlane(renderer, volume);

    renderWireframe(renderer, volume, localToWorld());
  }

  void viewChanged() const
  {
    m_viewChanged = true;
  }

  void testSelect(Selector& selector, SelectionTest& test)
  {
    test.BeginMesh(localToWorld());

    SelectionIntersection best;
    for(FaceInstances::iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
    {
      (*i).testSelect(test, best);
    }
    if(best.valid())
    {
      selector.addIntersection(best);
    }
  }

  bool isSelectedComponents() const
  {
    for(FaceInstances::const_iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
    {
      if((*i).selectedComponents())
      {
        return true;
      }
    }
    return false;
  }
  void setSelectedComponents(bool select, SelectionSystem::EComponentMode mode)
  {
    for(FaceInstances::iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
    {
      (*i).setSelected(mode, select);
    }
  }
  void testSelectComponents(Selector& selector, SelectionTest& test, SelectionSystem::EComponentMode mode)
  {
    test.BeginMesh(localToWorld());

    switch(mode)
    {
    case SelectionSystem::eVertex:
      {
        for(VertexInstances::iterator i = m_vertexInstances.begin(); i != m_vertexInstances.end(); ++i)
        {
          (*i).testSelect(selector, test);
        }
      }
      break;
    case SelectionSystem::eEdge:
      {
        for(EdgeInstances::iterator i = m_edgeInstances.begin(); i != m_edgeInstances.end(); ++i)
        {
          (*i).testSelect(selector, test);
        }
      }
      break;
    case SelectionSystem::eFace:
      {
        if(test.getVolume().fill())
        {
          for(FaceInstances::iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
          {
            (*i).testSelect(selector, test);
          }
        }
        else
        {
          for(FaceInstances::iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
          {
            (*i).testSelect_centroid(selector, test);
          }
        }
      }
      break;
    default:
      break;
    }
  }

  void selectPlanes(Selector& selector, SelectionTest& test, const PlaneCallback& selectedPlaneCallback)
  {
    test.BeginMesh(localToWorld());

    PlanePointer brushPlanes[c_brush_maxFaces];
    PlanesIterator j = brushPlanes;

    for(Brush::const_iterator i = m_brush.begin(); i != m_brush.end(); ++i)
    {
      *j++ = &(*i)->plane3();
    }

    for(FaceInstances::iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
    {
      (*i).selectPlane(selector, Line(test.getNear(), test.getFar()), brushPlanes, j, selectedPlaneCallback);
    }
  }
  void selectReversedPlanes(Selector& selector, const SelectedPlanes& selectedPlanes)
  {
    for(FaceInstances::iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
    {
      (*i).selectReversedPlane(selector, selectedPlanes);
    }
  }


  void transformComponents(const Matrix4& matrix)
  {
    for(FaceInstances::iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
    {
      (*i).transformComponents(matrix);
    }
  }
  const AABB& getSelectedComponentsBounds() const
  {
    m_aabb_component = AABB();

    for(FaceInstances::const_iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
    {
      (*i).iterate_selected(m_aabb_component);
    }

    return m_aabb_component;
  }

  void snapComponents(float snap)
  {
    for(FaceInstances::iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
    {
      (*i).snapComponents(snap);
    }
  }
  void evaluateTransform()
  {
    Matrix4 matrix(m_transform.calculateTransform());
    //globalOutputStream() << "matrix: " << matrix << "\n";

    if(m_transform.getType() == TRANSFORM_PRIMITIVE)
    {
      m_brush.transform(matrix);
    }
    else
    {
      transformComponents(matrix);
    }
  }
  void applyTransform()
  {
    m_brush.revertTransform();
    evaluateTransform();
    m_brush.freezeTransform();
  }
  typedef MemberCaller<BrushInstance, &BrushInstance::applyTransform> ApplyTransformCaller;

  void setClipPlane(const Plane3& plane)
  {
    m_clipPlane.setPlane(m_brush, plane);
  }

  bool testLight(const RendererLight& light) const
  {
    return light.testAABB(worldAABB());
  }
  void insertLight(const RendererLight& light)
  {
    const Matrix4& localToWorld = Instance::localToWorld();
    for(FaceInstances::const_iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
    {
      Face_addLight(*i, localToWorld, light);
    }
  }
  void clearLights()
  {
    for(FaceInstances::const_iterator i = m_faceInstances.begin(); i != m_faceInstances.end(); ++i)
    {
      (*i).m_lights.clear();
    }
  }
};

inline BrushInstance* Instance_getBrush(scene::Instance& instance)
{
  return InstanceTypeCast<BrushInstance>::cast(instance);
}


template<typename Functor>
class BrushSelectedVisitor : public SelectionSystem::Visitor
{
  const Functor& m_functor;
public:
  BrushSelectedVisitor(const Functor& functor) : m_functor(functor)
  {
  }
  void visit(scene::Instance& instance) const
  {
    BrushInstance* brush = Instance_getBrush(instance);
    if(brush != 0)
    {
      m_functor(*brush);
    }
  }
};

template<typename Functor>
inline const Functor& Scene_forEachSelectedBrush(const Functor& functor)
{
  GlobalSelectionSystem().foreachSelected(BrushSelectedVisitor<Functor>(functor));
  return functor;
}

template<typename Functor>
class BrushVisibleSelectedVisitor : public SelectionSystem::Visitor
{
  const Functor& m_functor;
public:
  BrushVisibleSelectedVisitor(const Functor& functor) : m_functor(functor)
  {
  }
  void visit(scene::Instance& instance) const
  {
    BrushInstance* brush = Instance_getBrush(instance);
    if(brush != 0
      && instance.path().top().get().visible())
    {
      m_functor(*brush);
    }
  }
};

template<typename Functor>
inline const Functor& Scene_forEachVisibleSelectedBrush(const Functor& functor)
{
  GlobalSelectionSystem().foreachSelected(BrushVisibleSelectedVisitor<Functor>(functor));
  return functor;
}

class BrushForEachFace
{
  const BrushInstanceVisitor& m_visitor;
public:
  BrushForEachFace(const BrushInstanceVisitor& visitor) : m_visitor(visitor)
  {
  }
  void operator()(BrushInstance& brush) const
  {
    brush.forEachFaceInstance(m_visitor);
  }
};

template<class Functor>
class FaceInstanceVisitFace : public BrushInstanceVisitor
{
  const Functor& functor;
public:
  FaceInstanceVisitFace(const Functor& functor)
    : functor(functor)
  {
  }
  void visit(FaceInstance& face) const
  {
    functor(face.getFace());
  }
};

template<typename Functor>
inline const Functor& Brush_forEachFace(BrushInstance& brush, const Functor& functor)
{
  brush.forEachFaceInstance(FaceInstanceVisitFace<Functor>(functor));
  return functor;
}

template<class Functor>
class FaceVisitAll : public BrushVisitor
{
  const Functor& functor;
public:
  FaceVisitAll(const Functor& functor)
    : functor(functor)
  {
  }
  void visit(Face& face) const
  {
    functor(face);
  }
};

template<typename Functor>
inline const Functor& Brush_forEachFace(const Brush& brush, const Functor& functor)
{
  brush.forEachFace(FaceVisitAll<Functor>(functor));
  return functor;
}

template<typename Functor>
inline const Functor& Brush_forEachFace(Brush& brush, const Functor& functor)
{
  brush.forEachFace(FaceVisitAll<Functor>(functor));
  return functor;
}

template<class Functor>
class FaceInstanceVisitAll : public BrushInstanceVisitor
{
  const Functor& functor;
public:
  FaceInstanceVisitAll(const Functor& functor)
    : functor(functor)
  {
  }
  void visit(FaceInstance& face) const
  {
    functor(face);
  }
};

template<typename Functor>
inline const Functor& Brush_ForEachFaceInstance(BrushInstance& brush, const Functor& functor)
{
  brush.forEachFaceInstance(FaceInstanceVisitAll<Functor>(functor));
  return functor;
}

template<typename Functor>
inline const Functor& Scene_forEachBrush(scene::Graph& graph, const Functor& functor)
{
  graph.traverse(InstanceWalker< InstanceApply<BrushInstance, Functor> >(functor));
  return functor;
}

template<typename Type, typename Functor>
class InstanceIfVisible : public Functor
{
public:
  InstanceIfVisible(const Functor& functor) : Functor(functor)
  {
  }
  void operator()(scene::Instance& instance)
  {
    if(instance.path().top().get().visible())
    {
      Functor::operator()(instance);
    }
  }
};

template<typename Functor>
class BrushVisibleWalker : public scene::Graph::Walker
{
  const Functor& m_functor;
public:
  BrushVisibleWalker(const Functor& functor) : m_functor(functor)
  {
  }
  bool pre(const scene::Path& path, scene::Instance& instance) const
  {
    if(path.top().get().visible())
    {
      BrushInstance* brush = Instance_getBrush(instance);
      if(brush != 0)
      {
        m_functor(*brush);
      }
    }
    return true;
  }
};

template<typename Functor>
inline const Functor& Scene_forEachVisibleBrush(scene::Graph& graph, const Functor& functor)
{
  graph.traverse(BrushVisibleWalker<Functor>(functor));
  return functor;
}

template<typename Functor>
inline const Functor& Scene_ForEachBrush_ForEachFace(scene::Graph& graph, const Functor& functor)
{
  Scene_forEachBrush(graph, BrushForEachFace(FaceInstanceVisitFace<Functor>(functor)));
  return functor;
}

template<typename Functor>
inline const Functor& Scene_ForEachSelectedBrush_ForEachFace(scene::Graph& graph, const Functor& functor)
{
  Scene_forEachSelectedBrush(BrushForEachFace(FaceInstanceVisitFace<Functor>(functor)));
  return functor;
}

template<typename Functor>
inline const Functor& Scene_ForEachSelectedBrush_ForEachFaceInstance(scene::Graph& graph, const Functor& functor)
{
  Scene_forEachSelectedBrush(BrushForEachFace(FaceInstanceVisitAll<Functor>(functor)));
  return functor;
}

template<typename Functor>
class FaceVisitorWrapper
{
  const Functor& functor;
public:
  FaceVisitorWrapper(const Functor& functor) : functor(functor)
  {
  }

  void operator()(FaceInstance& faceInstance) const
  {
    functor(faceInstance.getFace());
  }
};

template<typename Functor>
inline const Functor& Scene_ForEachSelectedBrushFace(scene::Graph& graph, const Functor& functor)
{
  g_SelectedFaceInstances.foreach(FaceVisitorWrapper<Functor>(functor));
  return functor;
}


#endif
