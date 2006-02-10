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

#if !defined(INCLUDED_MEMORY_ALLOCATOR_H)
#define INCLUDED_MEMORY_ALLOCATOR_H

#include <memory>

#if 0

#define DefaultAllocator std::allocator

#else

/// \brief An allocator that uses c++ new/delete.
/// Compliant with the std::allocator interface.
template<typename Type>
class DefaultAllocator
{
public:

  typedef Type value_type;
  typedef value_type* pointer;
  typedef const Type* const_pointer;
  typedef Type& reference;
  typedef const Type& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  template<typename Other>
  struct rebind
  {
    typedef DefaultAllocator<Other> other;
  };

  DefaultAllocator()
  {
  }
  DefaultAllocator(const DefaultAllocator<Type>&)
  {
  }
  template<typename Other> DefaultAllocator(const DefaultAllocator<Other>&)
  {
  }
  ~DefaultAllocator()
  {
  }

  pointer address(reference instance) const
  {
    return &instance;
  }
  const_pointer address(const_reference instance) const
  {
    return &instance;
  }
  Type* allocate(size_type size, const void* = 0)
  { 
    return static_cast<Type*>(::operator new(size * sizeof(Type)));
  }
  void deallocate(pointer p, size_type)
  {
    ::operator delete(p);
  }
  size_type max_size() const
  {
		return std::size_t(-1) / sizeof (Type);
  }
  void construct(pointer p, const Type& value)
  {
    new(p) Type(value);
  }
  void destroy(pointer p)
  {
    p->~Type();
  }
};

template<typename Type, typename Other>
inline bool operator==(const DefaultAllocator<Type>&, const DefaultAllocator<Other>&)
{ 
    return true;
}
template<typename Type, typename OtherAllocator>
inline bool operator==(const DefaultAllocator<Type>&, const OtherAllocator&)
{ 
    return false; 
}

#endif


template<typename Type>
class NamedAllocator : public DefaultAllocator<Type>
{
  typedef DefaultAllocator<Type> allocator_type;

  const char* m_name;
public:

  typedef Type value_type;
  typedef value_type* pointer;
  typedef const Type* const_pointer;
  typedef Type& reference;
  typedef const Type& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  template<typename Other>
  struct rebind
  {
    typedef NamedAllocator<Other> other;
  };

  explicit NamedAllocator(const char* name) : m_name(name)
  {
  }
  NamedAllocator(const NamedAllocator<Type>& other) : m_name(other.m_name)
  {
  }
  template<typename Other> NamedAllocator(const NamedAllocator<Other>& other) : m_name(other.m_name)
  {
  }
  ~NamedAllocator()
  {
  }

  pointer address(reference instance) const
  {
    return allocator_type::address(instance);
  }
  const_pointer address(const_reference instance) const
  {
    return allocator_type::address(instance);
  }
  Type* allocate(size_type size, const void* = 0)
  { 
    return allocator_type::allocate(size);
  }
  void deallocate(pointer p, size_type size)
  {
    allocator_type::deallocate(p, size);
  }
  size_type max_size() const
  {
    return allocator_type::max_size();
  }
  void construct(pointer p, const Type& value)
  {
    allocator_type::construct(p, value);
  }
  void destroy(pointer p)
  {
    allocator_type::destroy(p);
  }

  template<typename Other>
  bool operator==(const NamedAllocator<Other>& other)
  {
    return true;
  }

  // returns true if the allocators are not interchangeable
  template<typename Other>
  bool operator!=(const NamedAllocator<Other>& other)
  {
    return false;
  }
};



#include <algorithm>
#include "generic/object.h"



template<typename Type>
class DefaultConstruct
{
public:
  void operator()(Type& t)
  {
    constructor(t);
  }
};

template<typename Type, typename T1>
class Construct
{
  const T1& other;
public:
  Construct(const T1& other_) : other(other_)
  {
  }
  void operator()(Type& t)
  {
    constructor(t, other);
  }
};

template<typename Type>
class Destroy
{
public:
  void operator()(Type& t)
  {
    destructor(t);
  }
};

template<typename Type, typename Allocator = DefaultAllocator<Type> >
class New : public Allocator
{
public:
  New()
  {
  }
  explicit New(const Allocator& allocator) : Allocator(allocator)
  {
  }

  Type* scalar()
  {
    return new(Allocator::allocate(1)) Type();
  }
  template<typename T1>
  Type* scalar(const T1& t1)
  {
    return new(Allocator::allocate(1)) Type(t1);
  }
  template<typename T1, typename T2>
  Type* scalar(const T1& t1, const T2& t2)
  {
    return new(Allocator::allocate(1)) Type(t1, t2);
  }
  template<typename T1, typename T2, typename T3>
  Type* scalar(const T1& t1, const T2& t2, const T3& t3)
  {
    return new(Allocator::allocate(1)) Type(t1, t2, t3);
  }
  template<typename T1, typename T2, typename T3, typename T4>
  Type* scalar(const T1& t1, const T2& t2, const T3& t3, const T4& t4)
  {
    return new(Allocator::allocate(1)) Type(t1, t2, t3, t4);
  }
  template<typename T1, typename T2, typename T3, typename T4, typename T5>
  Type* scalar(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5)
  {
    return new(Allocator::allocate(1)) Type(t1, t2, t3, t4, t5);
  }
  Type* vector(std::size_t size)
  {
#if 1
    Type* p = Allocator::allocate(size);
    std::for_each(p, p + size, DefaultConstruct<Type>());
    return p;
#else
    // this does not work with msvc71 runtime
    return new(Allocator::allocate(size)) Type[size];
#endif
  }
  template<typename T1>
  Type* vector(std::size_t size, const T1& t1)
  {
    Type* p = Allocator::allocate(size);
    std::for_each(p, p + size, Construct<Type, T1>(t1));
    return p;
  }
};

template<typename Type, typename Allocator = DefaultAllocator<Type> >
class Delete : public Allocator
{
public:
  Delete()
  {
  }
  explicit Delete(const Allocator& allocator) : Allocator(allocator)
  {
  }

  void scalar(Type* p)
  {
    if(p != 0)
    {
      p->~Type();
      Allocator::deallocate(p, 1);
    }
  }
  void vector(Type* p, std::size_t size)
  {
    // '::operator delete' handles null
    // 'std::allocator::deallocate' requires non-null
    if(p != 0) 
    {
      std::for_each(p, p + size, Destroy<Type>());
      Allocator::deallocate(p, size);
    }
  }
};


template<typename Type>
class NamedNew
{
public:
  typedef New<Type, NamedAllocator<Type> > type;
};

template<typename Type>
class NamedDelete
{
public:
  typedef Delete<Type, NamedAllocator<Type> > type;
};

#endif
