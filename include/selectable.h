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

#if !defined(INCLUDED_SELECTABLE_H)
#define INCLUDED_SELECTABLE_H

#include <cstddef>

#include "generic/vector.h"
#include "scenelib.h"
#include "generic/callbackfwd.h"

class SelectionIntersection
{
  float m_depth;
  float m_distance;
public:
  SelectionIntersection() : m_depth(1), m_distance(2)
  {
  }
  SelectionIntersection(float depth, float distance) : m_depth(depth), m_distance(distance)
  {
  }
  bool operator<(const SelectionIntersection& other) const
  {
    if(m_distance != other.m_distance)
    {
      return m_distance < other.m_distance;
    }
    if(m_depth != other.m_depth)
    {
      return m_depth < other.m_depth;
    }
    return false;
  }
  bool equalEpsilon(const SelectionIntersection& other, float distanceEpsilon, float depthEpsilon) const
  {
    return float_equal_epsilon(m_distance, other.m_distance, distanceEpsilon)
      && float_equal_epsilon(m_depth, other.m_depth, depthEpsilon);
  }
  float depth() const
  {
    return m_depth;
  }
  bool valid() const
  {
    return depth() < 1;
  }
};

// returns true if self is closer than other
inline bool SelectionIntersection_closer(const SelectionIntersection& self, const SelectionIntersection& other)
{
  return self < other;
}

// assigns other to best if other is closer than best
inline void assign_if_closer(SelectionIntersection& best, const SelectionIntersection& other)
{
  if(SelectionIntersection_closer(other, best))
  {
    best = other;
  }
}




class VertexPointer
{
  typedef const unsigned char* byte_pointer;
public:
  typedef float elem_type;
  typedef const elem_type* pointer;
  typedef const elem_type& reference;

  class iterator
  {
  public:
    iterator() {}
    iterator(byte_pointer vertices, std::size_t stride)
      : m_iter(vertices), m_stride(stride) {}

    bool operator==(const iterator& other) const
    {
      return m_iter == other.m_iter;
    }
    bool operator!=(const iterator& other) const
    {
      return !operator==(other);
    }

    iterator operator+(std::size_t i)
    {
      return iterator(m_iter + i * m_stride, m_stride);
    }
    iterator operator+=(std::size_t i)
    {
      m_iter += i * m_stride;
      return *this;
    }
    iterator& operator++()
    {
      m_iter += m_stride;
      return *this;
    }
    iterator operator++(int)
    {
      iterator tmp = *this;
      m_iter += m_stride;
      return tmp;
    }
    reference operator*() const
    {
      return *reinterpret_cast<pointer>(m_iter);
    }
  private:
    byte_pointer m_iter;
    std::size_t m_stride;
  };

  VertexPointer(pointer vertices, std::size_t stride)
    : m_vertices(reinterpret_cast<byte_pointer>(vertices)), m_stride(stride) {}

  iterator begin() const
  {
    return iterator(m_vertices, m_stride);
  }

  reference operator[](std::size_t i) const
  {
    return *reinterpret_cast<pointer>(m_vertices + m_stride*i);
  }

private:
  byte_pointer m_vertices;
  std::size_t m_stride;
};

class IndexPointer
{
public:
  typedef unsigned int index_type;
  typedef const index_type* pointer;

  class iterator
  {
  public:
    iterator(pointer iter) : m_iter(iter) {}

    bool operator==(const iterator& other) const
    {
      return m_iter == other.m_iter;
    }
    bool operator!=(const iterator& other) const
    {
      return !operator==(other);
    }

    iterator operator+(std::size_t i)
    {
      return m_iter + i;
    }
    iterator operator+=(std::size_t i)
    {
      return m_iter += i;
    }
    iterator operator++()
    {
      return ++m_iter;
    }
    iterator operator++(int)
    {
      return m_iter++;
    }
    const index_type& operator*() const
    {
      return *m_iter;
    }
  private:
    void increment()
    {
      ++m_iter;
    }
    pointer m_iter;
  };

  IndexPointer(pointer indices, std::size_t count)
    : m_indices(indices), m_finish(indices + count) {}

  iterator begin() const
  {
    return m_indices;
  }
  iterator end() const
  {
    return m_finish;
  }

private:
  pointer m_indices;
  pointer m_finish;
};

template<typename Element> class BasicVector3;
typedef BasicVector3<float> Vector3;
class Matrix4;
class VolumeTest;

class SelectionTest
{
public:
  virtual void BeginMesh(const Matrix4& localToWorld, bool twoSided = false) = 0;
  virtual const VolumeTest& getVolume() const = 0;
  virtual const Vector3& getNear() const = 0;
  virtual const Vector3& getFar() const = 0;
  virtual void TestPoint(const Vector3& point, SelectionIntersection& best) = 0;
  virtual void TestPolygon(const VertexPointer& vertices, std::size_t count, SelectionIntersection& best) = 0;
  virtual void TestLineLoop(const VertexPointer& vertices, std::size_t count, SelectionIntersection& best) = 0;
  virtual void TestLineStrip(const VertexPointer& vertices, std::size_t count, SelectionIntersection& best) = 0;
  virtual void TestLines(const VertexPointer& vertices, std::size_t count, SelectionIntersection& best) = 0;
  virtual void TestTriangles(const VertexPointer& vertices, const IndexPointer& indices, SelectionIntersection& best) = 0; 
  virtual void TestQuads(const VertexPointer& vertices, const IndexPointer& indices, SelectionIntersection& best) = 0; 
  virtual void TestQuadStrip(const VertexPointer& vertices, const IndexPointer& indices, SelectionIntersection& best) = 0; 
};

class Selectable;

class Selector
{
public:
  virtual void pushSelectable(Selectable& selectable) = 0;
  virtual void popSelectable() = 0;
  virtual void addIntersection(const SelectionIntersection& intersection) = 0;
};

inline void Selector_add(Selector& selector, Selectable& selectable)
{
  selector.pushSelectable(selectable);
  selector.addIntersection(SelectionIntersection(0, 0));
  selector.popSelectable();
}

inline void Selector_add(Selector& selector, Selectable& selectable, const SelectionIntersection& intersection)
{
  selector.pushSelectable(selectable);
  selector.addIntersection(intersection);
  selector.popSelectable();
}


class VolumeTest;
class SelectionTestable
{
public:
  STRING_CONSTANT(Name, "SelectionTestable");

  virtual void testSelect(Selector& selector, SelectionTest& test) = 0;
};

inline SelectionTestable* Instance_getSelectionTestable(scene::Instance& instance)
{
  return InstanceTypeCast<SelectionTestable>::cast(instance);
}


class Plane3;
typedef Callback1<const Plane3&> PlaneCallback;

class SelectedPlanes
{
public:
  virtual bool contains(const Plane3& plane) const = 0;
};

class PlaneSelectable
{
public:
  STRING_CONSTANT(Name, "PlaneSelectable");

  virtual void selectPlanes(Selector& selector, SelectionTest& test, const PlaneCallback& selectedPlaneCallback) = 0;
  virtual void selectReversedPlanes(Selector& selector, const SelectedPlanes& selectedPlanes) = 0;
};



#endif
