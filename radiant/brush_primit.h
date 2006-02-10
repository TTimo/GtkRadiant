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

#if !defined(INCLUDED_BRUSH_PRIMIT_H)
#define INCLUDED_BRUSH_PRIMIT_H

#include "math/vector.h"
#include "itexdef.h"
// Timo
// new brush primitive texdef
struct brushprimit_texdef_t
{
  brushprimit_texdef_t()
  {
    coords[0][0] = 2.0f;
    coords[0][1] = 0.f;
    coords[0][2] = 0.f;
    coords[1][0] = 0.f;
    coords[1][1] = 2.0f;
    coords[1][2] = 0.f;
  }
  void removeScale(std::size_t width, std::size_t height)
  {
#if 1
    coords[0][0] *= width;
    coords[0][1] *= width;
    coords[0][2] *= width;
    coords[1][0] *= height;
    coords[1][1] *= height;
    coords[1][2] *= height;
#endif
  }
  void addScale(std::size_t width, std::size_t height)
  {
#if 1
    coords[0][0] /= width;
    coords[0][1] /= width;
    coords[0][2] /= width;
    coords[1][0] /= height;
    coords[1][1] /= height;
    coords[1][2] /= height;
#endif
  }
	float coords[2][3];
};

class TextureProjection
{
public:
	texdef_t m_texdef;
	brushprimit_texdef_t m_brushprimit_texdef;
  Vector3 m_basis_s;
  Vector3 m_basis_t;

  TextureProjection()
  {
  }
  TextureProjection(
    const texdef_t& texdef,
    const brushprimit_texdef_t& brushprimit_texdef,
    const Vector3& basis_s,
    const Vector3& basis_t
  ) :
    m_texdef(texdef),
    m_brushprimit_texdef(brushprimit_texdef),
    m_basis_s(basis_s),
    m_basis_t(basis_t)
  {
  }
};

float Texdef_getDefaultTextureScale();

class texdef_t;
struct Winding;
template<typename Element> class BasicVector3;
typedef BasicVector3<float> Vector3;
template<typename Element> class BasicVector4;
typedef BasicVector4<float> Vector4;
typedef Vector4 Quaternion;
class Matrix4;
class Plane3;

void Normal_GetTransform(const Vector3& normal, Matrix4& transform);

void TexDef_Construct_Default(TextureProjection& projection);

void Texdef_Assign(TextureProjection& projection, const TextureProjection& other);
void Texdef_Shift(TextureProjection& projection, float s, float t);
void Texdef_Scale(TextureProjection& projection, float s, float t);
void Texdef_Rotate(TextureProjection& projection, float angle);
void Texdef_FitTexture(TextureProjection& projection, std::size_t width, std::size_t height, const Vector3& normal, const Winding& w, float s_repeat, float t_repeat);
void Texdef_EmitTextureCoordinates(const TextureProjection& projection, std::size_t width, std::size_t height, Winding& w, const Vector3& normal, const Matrix4& localToWorld);

void ShiftScaleRotate_fromFace(texdef_t& shiftScaleRotate, const TextureProjection& projection);
void ShiftScaleRotate_toFace(const texdef_t& shiftScaleRotate, TextureProjection& projection);

void Texdef_transformLocked(TextureProjection& projection, std::size_t width, std::size_t height, const Plane3& plane, const Matrix4& transform);
void Texdef_normalise(TextureProjection& projection, float width, float height);

enum TexdefTypeId
{
  TEXDEFTYPEID_QUAKE,
  TEXDEFTYPEID_BRUSHPRIMITIVES,
  TEXDEFTYPEID_HALFLIFE,
};

struct bp_globals_t
{
  // tells if we are internally using brush primitive (texture coordinates and map format)
  // this is a shortcut for IntForKey( g_qeglobals.d_project_entity, "brush_primit" )
  // NOTE: must keep the two ones in sync
  TexdefTypeId m_texdefTypeId;
};

extern bp_globals_t g_bp_globals;
extern float g_texdef_default_scale;

#endif
