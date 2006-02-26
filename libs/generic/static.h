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

#if !defined(INCLUDED_GENERIC_STATIC_H)
#define INCLUDED_GENERIC_STATIC_H

/// \file
/// \brief Template techniques for instantiating singletons.

#include <cstddef>

class Null
{
};

/// \brief A singleton which is statically initialised.
///
/// \param Type The singleton object type.
/// \param Type The type distinguishing this instance from others of the same type.
///
/// \dontinclude generic/static.cpp
/// \skipline Static example
/// \until end example
template<typename Type, typename Context = Null>
class Static
{
  static Type m_instance;
public:
  static Type& instance()
  {
    return m_instance;
  }
};

template<typename Type, typename Context>
Type Static<Type, Context>::m_instance;


/// \brief A singleton which is lazily initialised.
/// The instance is constructed the first time it is referenced, and is never destroyed.
///
/// \param Type The singleton object type.
/// \param Type The type distinguishing this instance from others of the same type.
///
/// \dontinclude generic/static.cpp
/// \skipline LazyStatic example
/// \until end example
template<typename Type, typename Context = Null>
class LazyStatic
{
  static Type* m_instance; // this will be initialised to 0 by the CRT, according to the c++ standard
public:
  static Type& instance()
  {
    if(m_instance == 0)
    {
      m_instance = new Type; // allocate using 'new' to get the correct alignment
    }
    return *m_instance;
  }
};

template<typename Type, typename Context>
Type* LazyStatic<Type, Context>::m_instance;


/// \brief A singleton which keeps a count of the number of times it is referenced.
///
/// The instance is constructed when its reference count changes from 0 to 1 and destroyed when its reference count changes from 1 to 0.
/// Use with SmartStatic.
///
/// \param Type The singleton object type.
/// \param Type The type distinguishing this instance from others of the same type.
template<typename Type, typename Context = Null>
class CountedStatic
{
  static std::size_t m_refcount; // this will be initialised to 0 by the CRT, according to the c++ standard
  static Type* m_instance;
public:
  static Type& instance()
  {
    return *m_instance;
  }
  static void capture()
  {
    if(++m_refcount == 1)
    {
      m_instance = new Type; // allocate using 'new' to get the correct alignment
    }
  }
  static void release()
  {
    if(--m_refcount == 0)
    {
      delete m_instance;
    }
  }
};

template<typename Type, typename Context>
std::size_t CountedStatic<Type, Context>::m_refcount; // this will be initialised to 0 by the CRT, according to the c++ standard
template<typename Type, typename Context>
Type* CountedStatic<Type, Context>::m_instance;

/// \brief A reference to a CountedStatic.
/// Guarantees that CountedStatic<Type> will be constructed for the lifetime of this object.
///
/// \param Type The type parameter of the CountedStatic to reference.
/// \param Type The type distinguishing this instance from others of the same type.
///
/// \dontinclude generic/static.cpp
/// \skipline SmartStatic example
/// \until end example
template<typename Type, typename Context = Null>
class SmartStatic
{
public:
  SmartStatic()
  {
    CountedStatic<Type, Context>::capture();
  }
  ~SmartStatic()
  {
    CountedStatic<Type, Context>::release();
  }
  Type& instance()
  {
    return CountedStatic<Type, Context>::instance();
  }
};


#endif
