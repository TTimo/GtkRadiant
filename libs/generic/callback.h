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

#if !defined(INCLUDED_GENERIC_CLOSURE_H)
#define INCLUDED_GENERIC_CLOSURE_H

/// \file
/// \brief Type-safe techniques for binding the first argument of an anonymous callback. 

#include <cstddef>

/// \brief Combines a void pointer with a pointer to a function which operates on a void pointer.
///
/// Use with the callback constructors MemberCaller, ConstMemberCaller, ReferenceCaller, ConstReferenceCaller, PointerCaller, ConstPointerCaller and FreeCaller.
class Callback
{
  typedef void (*Thunk)(void*);
  void* m_environment;
  Thunk m_thunk;

  static void nullThunk(void*)
  {
  }

public:
  Callback() : m_environment(0), m_thunk(nullThunk)
  {
  }
  Callback(void* environment, Thunk function) : m_environment(environment), m_thunk(function)
  {
  }
  void* getEnvironment() const
  {
    return m_environment;
  }
  Thunk getThunk() const
  {
    return m_thunk;
  }
  void operator()() const
  {
    m_thunk(m_environment);
  }
};

inline bool operator==(const Callback& self, const Callback& other)
{
  return self.getEnvironment() == other.getEnvironment() && self.getThunk() == other.getThunk();
}
inline bool operator<(const Callback& self, const Callback& other)
{
  return self.getEnvironment() < other.getEnvironment() || 
        (!(other.getEnvironment() < self.getEnvironment()) && self.getThunk() < other.getThunk()); 
}

/// \brief Combines a void pointer with a pointer to a function which operates on a void pointer and one other argument. 
///
/// Use with the callback constructors MemberCaller1, ConstMemberCaller1, ReferenceCaller1, ConstReferenceCaller1, PointerCaller1, ConstPointerCaller1 and FreeCaller1.
template<typename FirstArgument>
class Callback1
{
  typedef void (*Thunk)(void*, FirstArgument);
  void* m_environment;
  Thunk m_thunk;

  static void nullThunk(void*, FirstArgument)
  {
  }

public:
  typedef FirstArgument first_argument_type;

  Callback1() : m_environment(0), m_thunk(nullThunk)
  {
  }
  Callback1(void* environment, Thunk function) : m_environment(environment), m_thunk(function)
  {
  }
  void* getEnvironment() const
  {
    return m_environment;
  }
  Thunk getThunk() const
  {
    return m_thunk;
  }
  void operator()(FirstArgument firstArgument) const
  {
    m_thunk(m_environment, firstArgument);
  }
};

template<typename FirstArgument>
inline bool operator==(const Callback1<FirstArgument>& self, const Callback1<FirstArgument>& other)
{
  return self.getEnvironment() == other.getEnvironment() && self.getThunk() == other.getThunk();
}
template<typename FirstArgument>
inline bool operator<(const Callback1<FirstArgument>& self, const Callback1<FirstArgument>& other)
{
  return self.getEnvironment() < other.getEnvironment() || 
        (!(other.getEnvironment() < self.getEnvironment()) && self.getThunk() < other.getThunk()); 
}

template<typename Functor>
class FunctorInvoke
{
public:
  inline void operator()(Functor functor)
  {
    functor();
  }
};

typedef FunctorInvoke<Callback> CallbackInvoke;


template<typename Functor, typename FirstArgument>
class Functor1Invoke
{
  FirstArgument m_firstArgument;
public:
  Functor1Invoke(FirstArgument firstArgument) : m_firstArgument(firstArgument)
  {
  }
  inline void operator()(Functor functor)
  {
    functor(m_firstArgument);
  }
};


typedef Callback1<bool> BoolImportCallback;
typedef Callback1<const BoolImportCallback&> BoolExportCallback;

typedef Callback1<int> IntImportCallback;
typedef Callback1<const IntImportCallback&> IntExportCallback;

typedef Callback1<float> FloatImportCallback;
typedef Callback1<const FloatImportCallback&> FloatExportCallback;

typedef Callback1<const char*> StringImportCallback;
typedef Callback1<const StringImportCallback&> StringExportCallback;

typedef Callback1<std::size_t> SizeImportCallback;
typedef Callback1<const SizeImportCallback&> SizeExportCallback;


/// \brief Forms a Callback from a non-const Environment reference and a non-const Environment member-function.
///
/// \dontinclude generic/callback.cpp
/// \skipline MemberCaller example
/// \until end example
template<typename Environment, void (Environment::*member)()>
class MemberCaller
{
  Environment& m_environment;
public:
  MemberCaller(Environment& environment) : m_environment(environment)
  {
  }
  void* getEnvironment() const
  {
    return &m_environment;
  }
  static void thunk(void* environment)
  {
    ((*reinterpret_cast<Environment*>(environment)).*member)();
  }
  operator Callback() const
  {
    return Callback(getEnvironment(), thunk);
  }
};

/// \brief Forms a Callback from a const Environment reference and a const Environment member-function.
///
/// \dontinclude generic/callback.cpp
/// \skipline MemberCaller example
/// \until end example
template<typename Environment, void (Environment::*member)() const>
class ConstMemberCaller
{
  const Environment& m_environment;
public:
  ConstMemberCaller(const Environment& environment) : m_environment(environment)
  {
  }
  void* getEnvironment() const
  {
    return const_cast<Environment*>(&m_environment);
  }
  static void thunk(void* environment)
  {
    ((*reinterpret_cast<const Environment*>(environment)).*member)();
  }
  operator Callback() const
  {
    return Callback(getEnvironment(), thunk);
  }
};

/// \brief Forms a Callback from a non-const Environment reference and a const Environment member-function which takes one argument.
template<typename Environment, typename FirstArgument, void (Environment::*member)(FirstArgument)>
class MemberCaller1
{
  Environment& m_environment;
public:
  MemberCaller1(Environment& environment) : m_environment(environment)
  {
  }
  void* getEnvironment() const
  {
    return &m_environment;
  }
  static void thunk(void* environment, FirstArgument firstArgument)
  {
    ((*reinterpret_cast<Environment*>(environment)).*member)(firstArgument);
  }
  operator Callback1<FirstArgument>() const
  {
    return Callback1<FirstArgument>(getEnvironment(), thunk);
  }
};

/// \brief Forms a Callback from a const Environment reference and a const Environment member-function which takes one argument.
template<typename Environment, typename FirstArgument, void (Environment::*member)(FirstArgument) const>
class ConstMemberCaller1
{
  const Environment& m_environment;
public:
  ConstMemberCaller1(const Environment& environment) : m_environment(environment)
  {
  }
  void* getEnvironment() const
  {
    return const_cast<Environment*>(&m_environment);
  }
  static void thunk(void* environment, FirstArgument firstArgument)
  {
    ((*reinterpret_cast<Environment*>(environment)).*member)(firstArgument);
  }
  operator Callback1<FirstArgument>() const
  {
    return Callback1<FirstArgument>(getEnvironment(), thunk);
  }
};

/// \brief Forms a Callback from a non-const Environment reference and a free function which operates on a non-const Environment reference.
///
/// \dontinclude generic/callback.cpp
/// \skipline ReferenceCaller example
/// \until end example
template<typename Environment, void (*func)(Environment&)> 
class ReferenceCaller
{
  Environment& m_environment;
public:
  ReferenceCaller(Environment& environment) : m_environment(environment)
  {
  }
  void* getEnvironment() const
  {
    return &m_environment;
  }
  static void thunk(void* environment)
  {
    (func)(*reinterpret_cast<Environment*>(environment));
  }
  operator Callback() const
  {
    return Callback(getEnvironment(), thunk);
  }
};

/// \brief Forms a Callback from a const Environment reference and a free function which operates on a const Environment reference.
///
/// \dontinclude generic/callback.cpp
/// \skipline ReferenceCaller example
/// \until end example
template<typename Environment, void (*func)(const Environment&)> 
class ConstReferenceCaller
{
  const Environment& m_environment;
public:
  ConstReferenceCaller(const Environment& environment) : m_environment(environment)
  {
  }
  void* getEnvironment() const
  {
    return const_cast<Environment*>(&m_environment);
  }
  static void thunk(void* environment)
  {
    (func)(*reinterpret_cast<const Environment*>(environment));
  }
  operator Callback() const
  {
    return Callback(getEnvironment(), thunk);
  }
};

/// \brief Forms a Callback from a non-const Environment reference and a free function which operates on a non-const Environment reference and one other argument.
template<typename Environment, typename FirstArgument, void (*func)(Environment&, FirstArgument)> 
class ReferenceCaller1
{
  Environment& m_environment;
public:
  ReferenceCaller1(Environment& environment) : m_environment(environment)
  {
  }
  void* getEnvironment() const
  {
    return &m_environment;
  }
  static void thunk(void* environment, FirstArgument firstArgument)
  {
    (func)(*reinterpret_cast<Environment*>(environment), firstArgument);
  }
  operator Callback1<FirstArgument>() const
  {
    return Callback1<FirstArgument>(getEnvironment(), thunk);
  }
};

/// \brief Forms a Callback from a const Environment reference and a free function which operates on a const Environment reference and one other argument.
template<typename Environment, typename FirstArgument, void (*func)(const Environment&, FirstArgument)> 
class ConstReferenceCaller1
{
  const Environment& m_environment;
public:
  ConstReferenceCaller1(const Environment& environment) : m_environment(environment)
  {
  }
  void* getEnvironment() const
  {
    return const_cast<Environment*>(&m_environment);
  }
  static void thunk(void* environment, FirstArgument firstArgument)
  {
    (func)(*reinterpret_cast<const Environment*>(environment), firstArgument);
  }
  operator Callback1<FirstArgument>() const
  {
    return Callback1<FirstArgument>(getEnvironment(), thunk);
  }
};

/// \brief Forms a Callback from a non-const Environment pointer and a free function which operates on a non-const Environment pointer.
template<typename Environment, void (*func)(Environment*)> 
class PointerCaller
{
  Environment* m_environment;
public:
  PointerCaller(Environment* environment) : m_environment(environment)
  {
  }
  void* getEnvironment() const
  {
    return m_environment;
  }
  static void thunk(void* environment)
  {
    (func)(reinterpret_cast<Environment*>(environment));
  }
  operator Callback() const
  {
    return Callback(getEnvironment(), thunk);
  }
};

/// \brief Forms a Callback from a const Environment pointer and a free function which operates on a const Environment pointer.
template<typename Environment, void (*func)(const Environment*)> 
class ConstPointerCaller
{
  const Environment* m_environment;
public:
  ConstPointerCaller(const Environment* environment) : m_environment(environment)
  {
  }
  void* getEnvironment() const
  {
    return const_cast<Environment*>(m_environment);
  }
  static void thunk(void* environment)
  {
    (func)(reinterpret_cast<const Environment*>(environment));
  }
  operator Callback() const
  {
    return Callback(getEnvironment(), thunk);
  }
};

/// \brief Forms a Callback from a non-const Environment pointer and a free function which operates on a non-const Environment pointer and one other argument.
template<typename Environment, typename FirstArgument, void (*func)(Environment*, FirstArgument)> 
class PointerCaller1
{
  Environment* m_environment;
public:
  PointerCaller1(Environment* environment) : m_environment(environment)
  {
  }
  void* getEnvironment() const
  {
    return m_environment;
  }
  static void thunk(void* environment, FirstArgument firstArgument)
  {
    (func)(reinterpret_cast<Environment*>(environment), firstArgument);
  }
  operator Callback1<FirstArgument>() const
  {
    return Callback1<FirstArgument>(getEnvironment(), thunk);
  }
};

/// \brief Forms a Callback from a const Environment pointer and a free function which operates on a const Environment pointer and one other argument.
template<typename Environment, typename FirstArgument, void (*func)(const Environment*, FirstArgument)> 
class ConstPointerCaller1
{
  const Environment* m_environment;
public:
  ConstPointerCaller1(const Environment* environment) : m_environment(environment)
  {
  }
  void* getEnvironment() const
  {
    return const_cast<Environment*>(m_environment);
  }
  static void thunk(void* environment, FirstArgument firstArgument)
  {
    (func)(reinterpret_cast<const Environment*>(environment), firstArgument);
  }
  operator Callback1<FirstArgument>() const
  {
    return Callback1<FirstArgument>(getEnvironment(), thunk);
  }
};


/// \brief Forms a Callback from a free function which takes no arguments.
template<void (*func)()> 
class FreeCaller
{
public:
  void* getEnvironment() const
  {
    return 0;
  }
  static void thunk(void*)
  {
    (func)();
  }
  operator Callback() const
  {
    return Callback(getEnvironment(), thunk);
  }
};

/// \brief Forms a Callback from a free function which takes a single argument.
template<typename FirstArgument, void (*func)(FirstArgument)> 
class FreeCaller1
{
public:
  void* getEnvironment() const
  {
    return 0;
  }
  static void thunk(void*, FirstArgument firstArgument)
  {
    (func)(firstArgument);
  }
  operator Callback1<FirstArgument>() const
  {
    return Callback1<FirstArgument>(getEnvironment(), thunk);
  }
};


/// \brief Constructs a Callback from a non-const \p functor with zero arguments.
/// 
/// \param Functor Must define \c operator()().
template<typename Functor>
inline Callback makeCallback(Functor& functor)
{
  return Callback(MemberCaller<Functor, &Functor::operator()>(functor));
}

/// \brief  Constructs a Callback from a const \p functor with zero arguments.
/// 
/// \param Functor Must define const \c operator()().
template<typename Functor>
inline Callback makeCallback(const Functor& functor)
{
  return Callback(ConstMemberCaller<Functor, &Functor::operator()>(functor));
}

/// \brief  Constructs a Callback1 from a non-const \p functor with one argument.
/// 
/// \param Functor Must define \c first_argument_type and \c operator()(first_argument_type).
template<typename Functor>
inline Callback1<typename Functor::first_argument_type> makeCallback1(Functor& functor)
{
  typedef typename Functor::first_argument_type FirstArgument;
  return Callback1<FirstArgument>(MemberCaller1<Functor, FirstArgument, &Functor::operator()>(functor));
}

/// \brief  Constructs a Callback1 from a const \p functor with one argument.
/// 
/// \param Functor Must define \c first_argument_type and const \c operator()(first_argument_type).
template<typename Functor>
inline Callback1<typename Functor::first_argument_type> makeCallback1(const Functor& functor)
{
  typedef typename Functor::first_argument_type FirstArgument;
  return Callback1<FirstArgument>(ConstMemberCaller1<Functor, FirstArgument, &Functor::operator()>(functor));
}

#endif
