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

#if !defined(INCLUDED_TYPESYSTEM_H)
#define INCLUDED_TYPESYSTEM_H


#include <list>
#include "generic/callback.h"
#include "generic/static.h"

class InitialiserList
{
  typedef std::list<Callback> Initialisers;
  Initialisers m_initialisers;
  mutable bool m_initialised;
public:
  InitialiserList() : m_initialised(false)
  {
  }
  void addInitialiser(const Callback& callback)
  {
    m_initialisers.push_back(callback); 
  }
  void initialise() const
  {
    if(!m_initialised)
    {
      m_initialised = true;

      for(Initialisers::const_iterator i = m_initialisers.begin(); i != m_initialisers.end(); ++i)
      {
        (*i)();
      }
    }
  }
};

//--Type System-------------------

class TypeSystemInitialiser : public InitialiserList
{
};

typedef SmartStatic<TypeSystemInitialiser> StaticTypeSystemInitialiser;

class TypeSystemRef : public StaticTypeSystemInitialiser
{
public:
  TypeSystemRef()
  {
    StaticTypeSystemInitialiser::instance().initialise();
  }
};



typedef std::size_t TypeId;
typedef void*(*TypeCast)(void*);

template<std::size_t SIZE>
class TypeCastTable
{
  TypeCast m_casts[SIZE];
public:
  TypeCastTable()
  {
    std::uninitialized_fill(m_casts, m_casts + SIZE, TypeCast(0));
  }
  void install(TypeId typeId, TypeCast typeCast)
  {
    m_casts[typeId] = typeCast;
  }
  void* cast(TypeId typeId, void* p)
  {
    TypeCast typeCast = m_casts[typeId];
    if(typeCast != 0)
    {
      return typeCast(p);
    }
    return 0;
  }
};

template<typename Type, typename Cast>
class CastInstaller
{
public:
  static void install(TypeCastTable<Type::SIZE>& table)
  {
    table.install(Type::getTypeId(), Cast::cast);
  }
};

template<typename Type>
class IdentityCast
{
public:
  static void* cast(void* p)
  {
    return p;
  }
};

template<typename Type, typename Base>
class StaticCast
{
public:
  static void* cast(void* p)
  {
    return static_cast<Base*>(reinterpret_cast<Type*>(p));
  }
};

template<typename Type>
class NullType
{
};

template<typename Type, typename Contained>
class ContainedCast
{
public:
  static void* cast(void* p)
  {
    return &reinterpret_cast<Type*>(p)->get(NullType<Contained>());
  }
};


#endif
