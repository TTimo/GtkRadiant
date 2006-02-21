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

#if !defined(INCLUDED_CONTAINER_STACK_H)
#define INCLUDED_CONTAINER_STACK_H

#include "memory/allocator.h"
#include <algorithm>

/// \brief A stack whose storage capacity is variable at run-time. Similar to std::vector.
///
/// - Pushing or popping elements is a constant-time operation (on average).
/// - The storage capacity of the stack will grow when a new element is added beyond the current capacity. Iterators are invalidated when the storage capacity grows.
/// - DefaultConstructible, Copyable, Assignable.
/// - Compatible with the containers and algorithms in the Standard Template Library (STL) - http://www.sgi.com/tech/stl/
///
/// \param Type: The type to be stored in the stack. Must provide a copy-constructor.
template<typename Type>
class Stack : public DefaultAllocator<Type>
{
  typedef DefaultAllocator<Type> Allocator;

  enum
  {
    DEFAULT_CAPACITY = 4,
  };

  typedef Type* pointer;
  typedef const Type* const_pointer;

public:
  typedef const_pointer const_iterator;
private:

  pointer m_data;
  pointer m_end;
  std::size_t m_capacity;


  void insert(const Type& value)
  {
    Allocator::construct(m_end++, value);
  }
  void insert_overflow(const Type& value)
  {
    const std::size_t new_capacity = (m_capacity) ? m_capacity + m_capacity : std::size_t(DEFAULT_CAPACITY);
    const pointer new_data = Allocator::allocate(new_capacity);
    const pointer new_end = std::copy(m_data, m_end, new_data);

    destroy();
    Allocator::deallocate(m_data, m_capacity);

    m_capacity = new_capacity;
    m_data = new_data;
    m_end = new_end;
    insert(value); 
  }
  void destroy()
  {
    for(pointer p = m_data; p != m_end; ++p)
    {
      Allocator::destroy(p);
    }
  }
  void construct(const Stack& other)
  {
    pointer p = m_data;
    for(const_iterator i = other.begin(); i != other.end(); ++i)
    {
      Allocator::construct(p++, *i);
    }
  }

public:

  Stack() :
    m_data(0),
    m_end(0),
    m_capacity(0)
  {
  }
  Stack(const Type& value) :
    m_data(0),
    m_end(0),
    m_capacity(0)
  {
    push(value);
  }
  Stack(const Stack& other) :
    DefaultAllocator<Type>(other)
  {
    m_capacity = other.m_capacity;
    m_data = Allocator::allocate(m_capacity);
    construct(other);
    m_end = m_data + other.size();
  }
  ~Stack()
  {
    destroy();
    Allocator::deallocate(m_data, m_capacity);
  }

  const_iterator begin() const
  {
    return m_data;
  }
  const_iterator end() const
  {
    return m_end;
  }

  bool empty() const
  {
    return end() == begin();
  }
  void clear()
  {
    destroy();
    m_end = m_data;
  }

  std::size_t size() const
  {
    return m_end - m_data;
  }
  Type operator[](const std::size_t i) const
  {
    return m_data[i];
  }
  /// \brief Pushes \p value onto the stack at the top element. If reserved storage is insufficient for the new element, this will invalidate all iterators.
  void push(const Type& value)
  {
    if(size() == m_capacity)
    {
      insert_overflow(value);
    }
    else
    {
      insert(value);
    }
  }
  /// \brief Removes the top element of the stack.
  void pop()
  {
    Allocator::destroy(--m_end);
  }
  /// \brief Returns the top element of the mutable stack.
  Type& top()
  {
    return *(m_end-1);
  }
  /// \brief Returns the top element of the non-mutable stack.
  const Type& top() const
  {
    return *(m_end-1);
  }
  /// \brief Returns the element below the top element of the mutable stack.
  Type& parent()
  {
    return *(m_end-2);
  }
  /// \brief Returns the element below the top element of the non-mutable stack.
  const Type& parent() const
  {
    return *(m_end-2);
  }
  /// \brief Swaps the values of this stack and \p other.
  void swap(Stack& other)
  {
    std::swap(m_data, other.m_data);
    std::swap(m_end, other.m_end);
    std::swap(m_capacity, other.m_capacity);
  }
#if 1 // use copy-swap technique
  Stack& operator=(const Stack& other)
  {
    Stack temp(other);
    temp.swap(*this);
    return *this;
  }
#else // avoids memory allocation if capacity is already sufficient.
  Stack& operator=(const Stack& other)
  {
    if(&other != this)
    {
      destroy();

      if(other.size() > m_capacity)
      {
        Allocator::deallocate(m_data, m_capacity);
        m_capacity = other.m_capacity;
        m_data = Allocator::allocate(m_capacity);
      }
      m_end = m_data + other.size();

      construct(other);
    }
    return *this;
  }
#endif
};

/// \brief Returns true if \p self is lexicographically less than \p other.
template<typename Type>
inline bool operator<(const Stack<Type>& self, const Stack<Type>& other)
{
  return std::lexicographical_compare(self.begin(), self.end(), other.begin(), other.end());
}

namespace std
{
  /// \brief Swaps the values of \p self and \p other.
  /// Overloads std::swap().
  template<typename Type>
  inline void swap(Stack<Type>& self, Stack<Type>& other)
  {
    self.swap(other);
  }
}

#endif
