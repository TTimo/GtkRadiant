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

#if !defined(INCLUDED_GENERIC_REFERENCECOUNTED_H)
#define INCLUDED_GENERIC_REFERENCECOUNTED_H

/// \file
/// \brief 'smart' pointers and references. 

#include <algorithm>

template<typename Type>
class IncRefDecRefCounter
{
public:
  void increment(Type& value)
  {
    value.IncRef();
  }
  void decrement(Type& value)
  {
    value.DecRef();
  }
};

/// \brief A smart-pointer that uses a counter stored in the object pointed-to.
template<typename Type, typename Counter = IncRefDecRefCounter<Type> >
class SmartPointer : public Counter
{
  Type* m_value;
public:

  SmartPointer(const SmartPointer& other)
    : m_value(other.m_value)
  {
    Counter::increment(*m_value);
  }
  explicit SmartPointer(Type* value)
    : m_value(value)
  {
    Counter::increment(*m_value);
  }
  ~SmartPointer()
  {
    Counter::decrement(*m_value);
  }
  SmartPointer& operator=(const SmartPointer& other)
  {
    SmartPointer temp(other);
    temp.swap(*this);
    return *this;
  }
  SmartPointer& operator=(Type* value)
  {
    SmartPointer temp(value);
    temp.swap(*this);
    return *this;
  }
  void swap(SmartPointer& other)
  {
    std::swap(m_value, other.m_value);
  }

  operator Type*() const
  {
    return m_value;
  }
  Type& operator*() const
  {
    return *m_value;
  }
  Type* operator->() const
  {
    return m_value;
  }
  Type* get() const
  {
    return m_value;
  }
};

template<typename Type>
inline bool operator<(const SmartPointer<Type>& self, const SmartPointer<Type>& other)
{
  return self.get() < other.get();
}
template<typename Type>
inline bool operator==(const SmartPointer<Type>& self, const SmartPointer<Type>& other)
{
  return self.get() == other.get();
}
template<typename Type>
inline bool operator!=(const SmartPointer<Type>& self, const SmartPointer<Type>& other)
{
  return !::operator==(self, other);
}

namespace std
{
  /// \brief Swaps the values of \p self and \p other.
  /// Overloads std::swap().
  template<typename Type>
  inline void swap(SmartPointer<Type>& self, SmartPointer<Type>& other)
  {
    self.swap(other);
  }
}


/// \brief A smart-reference that uses a counter stored in the object pointed-to.
template<typename Type, typename Counter = IncRefDecRefCounter<Type> >
class SmartReference : public Counter
{
  Type* m_value;
public:

  SmartReference(const SmartReference& other)
    : m_value(other.m_value)
  {
    Counter::increment(*m_value);
  }
  explicit SmartReference(Type& value)
    : m_value(&value)
  {
    Counter::increment(*m_value);
  }
  ~SmartReference()
  {
    Counter::decrement(*m_value);
  }
  SmartReference& operator=(const SmartReference& other)
  {
    SmartReference temp(other);
    temp.swap(*this);
    return *this;
  }
  SmartReference& operator=(Type& value)
  {
    SmartReference temp(value);
    temp.swap(*this);
    return *this;
  }
  void swap(SmartReference& other)
  {
    std::swap(m_value, other.m_value);
  }

  operator Type&() const
  {
    return *m_value;
  }
  Type& get() const
  {
    return *m_value;
  }
  Type* get_pointer() const
  {
    return m_value;
  }
};

template<typename Type>
inline bool operator<(const SmartReference<Type>& self, const SmartReference<Type>& other)
{
  return self.get() < other.get();
}
template<typename Type>
inline bool operator==(const SmartReference<Type>& self, const SmartReference<Type>& other)
{
  return self.get() == other.get();
}
template<typename Type>
inline bool operator!=(const SmartReference<Type>& self, const SmartReference<Type>& other)
{
  return !::operator==(self, other);
}

namespace std
{
  /// \brief Swaps the values of \p self and \p other.
  /// Overloads std::swap().
  template<typename Type>
  inline void swap(SmartReference<Type>& self, SmartReference<Type>& other)
  {
    self.swap(other);
  }
}

#endif
