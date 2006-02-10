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

#if !defined(INCLUDED_WINDING_H)
#define INCLUDED_WINDING_H

#include "debugging/debugging.h"

#include <vector>

#include "math/vector.h"
#include "container/array.h"

enum ProjectionAxis
{
  eProjectionAxisX = 0,
  eProjectionAxisY = 1,
  eProjectionAxisZ = 2,
};

const float ProjectionAxisEpsilon = static_cast<float>(0.0001);

inline bool projectionaxis_better(float axis, float other)
{
  return fabs(axis) > fabs(other) + ProjectionAxisEpsilon;
}

/// \brief Texture axis precedence: Z > X > Y
inline ProjectionAxis projectionaxis_for_normal(const Vector3& normal)
{
  return (projectionaxis_better(normal[eProjectionAxisY], normal[eProjectionAxisX]))
    ? (projectionaxis_better(normal[eProjectionAxisY], normal[eProjectionAxisZ]))
      ? eProjectionAxisY
      : eProjectionAxisZ
    : (projectionaxis_better(normal[eProjectionAxisX], normal[eProjectionAxisZ]))
      ? eProjectionAxisX
      : eProjectionAxisZ;
}


struct indexremap_t
{
  indexremap_t(std::size_t _x, std::size_t _y, std::size_t _z)
    : x(_x), y(_y), z(_z)
  {
  }
  std::size_t x, y, z;
};

inline indexremap_t indexremap_for_projectionaxis(const ProjectionAxis axis)
{
  switch(axis)
  {
  case eProjectionAxisX: return indexremap_t(1, 2, 0);
  case eProjectionAxisY: return indexremap_t(2, 0, 1);
  default: return indexremap_t(0, 1, 2);
  }
}

enum PlaneClassification
{
  ePlaneFront = 0,
  ePlaneBack = 1,
  ePlaneOn = 2,
};

#define MAX_POINTS_ON_WINDING 64
const std::size_t c_brush_maxFaces = 1024;


class WindingVertex
{
public:
  Vector3 vertex;
  Vector2 texcoord;
  Vector3 tangent;
  Vector3 bitangent;
  std::size_t adjacent;
};



struct Winding
{
  typedef Array<WindingVertex> container_type;

  std::size_t numpoints;
  container_type points;

  typedef container_type::iterator iterator;
  typedef container_type::const_iterator const_iterator;

  Winding() : numpoints(0)
  {
  }
  Winding(std::size_t size) : numpoints(0), points(size)
  {
  }
  void resize(std::size_t size)
  {
    points.resize(size);
    numpoints = 0;
  }

  iterator begin()
  {
    return points.begin();
  }
  const_iterator begin() const
  {
    return points.begin();
  }
  iterator end()
  {
    return points.begin() + numpoints;
  }
  const_iterator end() const
  {
    return points.begin() + numpoints;
  }

  WindingVertex& operator[](std::size_t index)
  {
    ASSERT_MESSAGE(index < points.size(), "winding: index out of bounds");
    return points[index];
  }
  const WindingVertex& operator[](std::size_t index) const
  {
    ASSERT_MESSAGE(index < points.size(), "winding: index out of bounds");
    return points[index];
  }

  void push_back(const WindingVertex& point)
  {
    points[numpoints] = point;
    ++numpoints;
  }
  void erase(iterator point)
  {
    for(iterator i = point + 1; i != end(); point = i, ++i)
    {
      *point = *i;
    }
    --numpoints;
  }
};

typedef BasicVector3<double> DoubleVector3;

class DoubleLine
{
public:
  DoubleVector3 origin;
  DoubleVector3 direction;
};

class FixedWindingVertex
{
public:
  DoubleVector3 vertex;
  DoubleLine edge;
  std::size_t adjacent;

  FixedWindingVertex(const DoubleVector3& vertex_, const DoubleLine& edge_, std::size_t adjacent_)
    : vertex(vertex_), edge(edge_), adjacent(adjacent_)
  {
  }
};

struct FixedWinding
{
  typedef std::vector<FixedWindingVertex> Points;
  Points points;

  FixedWinding()
  {
    points.reserve(MAX_POINTS_ON_WINDING);
  }

  FixedWindingVertex& front()
  {
    return points.front();
  }
  const FixedWindingVertex& front() const
  {
    return points.front();
  }
  FixedWindingVertex& back()
  {
    return points.back();
  }
  const FixedWindingVertex& back() const
  {
    return points.back();
  }

  void clear()
  {
    points.clear();
  }

  void push_back(const FixedWindingVertex& point)
  {
    points.push_back(point);
  }
  std::size_t size() const
  {
    return points.size();
  }

  FixedWindingVertex& operator[](std::size_t index)
  {
    //ASSERT_MESSAGE(index < MAX_POINTS_ON_WINDING, "winding: index out of bounds");
    return points[index];
  }
  const FixedWindingVertex& operator[](std::size_t index) const
  {
    //ASSERT_MESSAGE(index < MAX_POINTS_ON_WINDING, "winding: index out of bounds");
    return points[index];
  }

};


inline void Winding_forFixedWinding(Winding& winding, const FixedWinding& fixed)
{
  winding.resize(fixed.size());
  winding.numpoints = fixed.size();
  for(std::size_t i = 0; i < fixed.size(); ++i)
  {
    winding[i].vertex[0] = static_cast<float>(fixed[i].vertex[0]);
    winding[i].vertex[1] = static_cast<float>(fixed[i].vertex[1]);
    winding[i].vertex[2] = static_cast<float>(fixed[i].vertex[2]);
    winding[i].adjacent = fixed[i].adjacent;
  }
}

inline std::size_t Winding_wrap(const Winding& winding, std::size_t i)
{
  ASSERT_MESSAGE(winding.numpoints != 0, "Winding_wrap: empty winding");
  return i % winding.numpoints;
}

inline std::size_t Winding_next(const Winding& winding, std::size_t i)
{
  return Winding_wrap(winding, ++i);
}


class Plane3;

void Winding_createInfinite(FixedWinding& w, const Plane3& plane, double infinity);

const double ON_EPSILON	= 1.0 / (1 << 8);

/// \brief Returns true if edge (\p x, \p y) is smaller than the epsilon used to classify winding points against a plane.
inline bool Edge_isDegenerate(const Vector3& x, const Vector3& y)
{
  return vector3_length_squared(y - x) < (ON_EPSILON * ON_EPSILON);
}

void Winding_Clip(const FixedWinding& winding, const Plane3& plane, const Plane3& clipPlane, std::size_t adjacent, FixedWinding& clipped);

struct brushsplit_t
{
  brushsplit_t()
  {
    counts[0] = 0;
    counts[1] = 0;
    counts[2] = 0;
  }
  brushsplit_t& operator+=(const brushsplit_t& other)
  {
    counts[0] += other.counts[0];
    counts[1] += other.counts[1];
    counts[2] += other.counts[2];
    return *this;
  }
  std::size_t counts[3];
};

brushsplit_t Winding_ClassifyPlane(const Winding& w, const Plane3& plane);

bool Winding_PlanesConcave(const Winding& w1, const Winding& w2, const Plane3& plane1, const Plane3& plane2);
bool Winding_TestPlane(const Winding& w, const Plane3& plane, bool flipped);

std::size_t Winding_FindAdjacent(const Winding& w, std::size_t face);

std::size_t Winding_Opposite(const Winding& w, const std::size_t index, const std::size_t other);
std::size_t Winding_Opposite(const Winding& w, std::size_t index);

void Winding_Centroid(const Winding& w, const Plane3& plane, Vector3& centroid);


inline void Winding_printConnectivity(Winding& winding)
{
  for(Winding::iterator i = winding.begin(); i != winding.end(); ++i)
  {
    std::size_t vertexIndex = std::distance(winding.begin(), i);
    globalOutputStream() << "vertex: " << Unsigned(vertexIndex) << " adjacent: " << Unsigned((*i).adjacent) << "\n";
  }
}

#endif
