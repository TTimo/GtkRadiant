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

#if !defined(INCLUDED_CONTAINER_ARRAY_H)
#define INCLUDED_CONTAINER_ARRAY_H

#include <cstddef>
#include <algorithm>

#include "memory/allocator.h"

/// \brief An array whose size is variable at run-time.
///
/// - Resizing the array destroys all the existing elements and invalidates all iterators.
/// - Default-Constructible, Copyable, Assignable. 
/// - Compatible with the containers and algorithms in the Standard Template Library (STL) - http://www.sgi.com/tech/stl/
///
/// \param Element The type to be stored in the array. Must provide a default-constructor and a copy-constructor.
/// \param Allocator A custom memory-allocator, conforming to the std::allocator interface.
template<typename Element, typename Allocator = DefaultAllocator<Element> >
class Array : public Allocator
{
  std::size_t m_size;
  Element* m_data;

  Element* construct(std::size_t size)
  {
#if 1
    return New<Element, Allocator>(*this).vector(size);
#else
    return new Element[size];
#endif
  }
  template<typename T1>
  Element* construct(std::size_t size, const T1& value)
  {
    return New<Element, Allocator>(*this).vector(size, value);
  }
  void destroy(Element* data, std::size_t size)
  {
#if 1
    Delete<Element, Allocator>(*this).vector(data, size);
#else
    delete[] data;
#endif
  }

public:
  typedef Element value_type;
  typedef value_type* iterator;
  typedef const value_type* const_iterator;

  Array()
    : m_size(0), m_data(0)
  {
  }
  Array(std::size_t size)
    : m_size(size), m_data(construct(size))
  {
  }
  template<typename T1>
  Array(std::size_t size, const T1& value)
    : m_size(size), m_data(construct(size, value))
  {
  }
  Array(const Array& other)
    : Allocator(other), m_size(other.size()), m_data(construct(m_size))
  {
    std::copy(other.begin(), other.end(), begin());
  }
  template<typename Iterator>
  Array(Iterator start, Iterator finish)
  : m_size(std::distance(start, finish)), m_data(construct(m_size))
  {
    std::copy(start, finish, begin());
  }
  ~Array()
  {
    destroy(m_data, m_size);
  }

  Array& operator=(const Array& other)
  {
    if(other.size() == size())
    {
      std::copy(other.begin(), other.end(), begin());
    }
    else
    {
      Array temp(other);
      temp.swap(*this);
    }
    return *this;
  }

  void swap(Array& other)
  {
    std::swap(m_size, other.m_size);
    std::swap(m_data, other.m_data);
  }

  iterator begin()
  {
    return m_data;
  }
  const_iterator begin() const
  {
    return m_data;
  }
  iterator end()
  {
    return m_data + m_size;
  }
  const_iterator end() const
  {
    return m_data + m_size;
  }

  value_type& operator[](std::size_t index)
  {
#if defined(_DEBUG)
    ASSERT_MESSAGE(index < size(), "array index out of bounds");
#endif
    return m_data[index];
  }
  const value_type& operator[](std::size_t index) const
  {
#if defined(_DEBUG)
    ASSERT_MESSAGE(index < size(), "array index out of bounds");
#endif
    return m_data[index];
  }
  value_type* data()
  {
    return m_data;
  }
  const value_type* data() const
  {
    return m_data;
  }
  std::size_t size() const
  {
    return m_size;
  }
  bool empty() const
  {
    return m_size == 0;
  }

  void resize(std::size_t count)
  {
    if(count != size())
    {
      Array temp(count);
      temp.swap(*this);
    }
  }
  void resize(std::size_t count, const value_type& value)
  {
    if(count != size())
    {
      Array temp(count, value);
      temp.swap(*this);
    }
  }
};

namespace std
{
  /// \brief Swaps the values of \p self and \p other.
  /// Overloads std::swap.
  template<typename Element, typename Allocator>
  inline void swap(Array<Element, Allocator>& self, Array<Element, Allocator>& other)
  {
    self.swap(other);
  }
}

#endif
