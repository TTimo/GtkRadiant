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
/// \brief Type-safe techniques for binding the first argument of an opaque callback. 

#include <cstddef>
#include "functional.h"
#include "callbackfwd.h"

template<typename Type>
inline void* convertToOpaque(Type* t)
{
  return t;
}
template<typename Type>
inline void* convertToOpaque(const Type* t)
{
  return const_cast<Type*>(t);
}
template<typename Type>
inline void* convertToOpaque(Type& t)
{
  return &t;
}
template<typename Type>
inline void* convertToOpaque(const Type& t)
{
  return const_cast<Type*>(&t);
}


template<typename Type>
class ConvertFromOpaque
{
};

template<typename Type>
class ConvertFromOpaque<Type&>
{
public:
  static Type& apply(void* p)
  {
    return *static_cast<Type*>(p);
  }
};

template<typename Type>
class ConvertFromOpaque<const Type&>
{
public:
  static const Type& apply(void* p)
  {
    return *static_cast<Type*>(p);
  }
};


template<typename Type>
class ConvertFromOpaque<Type*>
{
public:
  static Type* apply(void* p)
  {
    return static_cast<Type*>(p);
  }
};

template<typename Type>
class ConvertFromOpaque<const Type*>
{
public:
  static const Type* apply(void* p)
  {
    return static_cast<Type*>(p);
  }
};

template<typename Caller>
class BindFirstOpaque
{
  typedef typename Caller::first_argument_type FirstBound;
  FirstBound firstBound;
public:
  typedef typename Caller::result_type result_type;
  explicit BindFirstOpaque(FirstBound firstBound) : firstBound(firstBound)
  {
  }
  result_type operator()() const
  {
    return Caller::call(firstBound);
  }
  FirstBound getBound() const
  {
    return firstBound;
  }
  static result_type thunk(void* environment)
  {
    return Caller::call(ConvertFromOpaque<FirstBound>::apply(environment));
  }
  void* getEnvironment() const
  {
    return convertToOpaque(firstBound);
  }
};

template<typename Caller>
class BindFirstOpaque1
{
  typedef typename Caller::first_argument_type FirstBound;
  FirstBound firstBound;
public:
  typedef typename Caller::second_argument_type first_argument_type;
  typedef typename Caller::result_type result_type;
  explicit BindFirstOpaque1(FirstBound firstBound) : firstBound(firstBound)
  {
  }
  result_type operator()(first_argument_type a1) const
  {
    return Caller::call(firstBound, a1);
  }
  FirstBound getBound() const
  {
    return firstBound;
  }
  static result_type thunk(void* environment, first_argument_type a1)
  {
    return Caller::call(ConvertFromOpaque<FirstBound>::apply(environment), a1);
  }
  void* getEnvironment() const
  {
    return convertToOpaque(firstBound);
  }
};

template<typename Caller>
class BindFirstOpaque2
{
  typedef typename Caller::first_argument_type FirstBound;
  FirstBound firstBound;
public:
  typedef typename Caller::second_argument_type first_argument_type;
  typedef typename Caller::third_argument_type second_argument_type;
  typedef typename Caller::result_type result_type;
  explicit BindFirstOpaque2(FirstBound firstBound) : firstBound(firstBound)
  {
  }
  result_type operator()(first_argument_type a1, second_argument_type a2) const
  {
    return Caller::call(firstBound, a1, a2);
  }
  FirstBound getBound() const
  {
    return firstBound;
  }
  static result_type thunk(void* environment, first_argument_type a1, second_argument_type a2)
  {
    return Caller::call(ConvertFromOpaque<FirstBound>::apply(environment), a1, a2);
  }
  void* getEnvironment() const
  {
    return convertToOpaque(firstBound);
  }
};

template<typename Caller>
class BindFirstOpaque3
{
  typedef typename Caller::first_argument_type FirstBound;
  FirstBound firstBound;
public:
  typedef typename Caller::second_argument_type first_argument_type;
  typedef typename Caller::third_argument_type second_argument_type;
  typedef typename Caller::fourth_argument_type third_argument_type;
  typedef typename Caller::result_type result_type;
  explicit BindFirstOpaque3(FirstBound firstBound) : firstBound(firstBound)
  {
  }
  result_type operator()(first_argument_type a1, second_argument_type a2, third_argument_type a3) const
  {
    return Caller::call(firstBound, a1, a2, a3);
  }
  FirstBound getBound() const
  {
    return firstBound;
  }
  static result_type thunk(void* environment, first_argument_type a1, second_argument_type a2, third_argument_type a3)
  {
    return Caller::call(ConvertFromOpaque<FirstBound>::apply(environment), a1, a2, a3);
  }
  void* getEnvironment() const
  {
    return convertToOpaque(firstBound);
  }
};

template<typename Thunk_>
class CallbackBase
{
  void* m_environment;
  Thunk_ m_thunk;
public:
  typedef Thunk_ Thunk;
  CallbackBase(void* environment, Thunk function) : m_environment(environment), m_thunk(function)
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
};

template<typename Thunk>
inline bool operator==(const CallbackBase<Thunk>& self, const CallbackBase<Thunk>& other)
{
  return self.getEnvironment() == other.getEnvironment() && self.getThunk() == other.getThunk();
}
template<typename Thunk>
inline bool operator!=(const CallbackBase<Thunk>& self, const CallbackBase<Thunk>& other)
{
  return !(self == other);
}
template<typename Thunk>
inline bool operator<(const CallbackBase<Thunk>& self, const CallbackBase<Thunk>& other)
{
  return self.getEnvironment() < other.getEnvironment() || 
        (!(other.getEnvironment() < self.getEnvironment()) && self.getThunk() < other.getThunk()); 
}


/// \brief Combines a void pointer with a pointer to a function which operates on a void pointer.
///
/// Use with the callback constructors MemberCaller, ConstMemberCaller, ReferenceCaller, ConstReferenceCaller, PointerCaller, ConstPointerCaller and FreeCaller.
template<typename Result>
class Callback0 : public CallbackBase<Result (*)(void*)>
{
  typedef CallbackBase<Result (*)(void*)> Base;
  static Result nullThunk(void*)
  {
  }

public:
  typedef Result result_type;

  Callback0() : Base(0, nullThunk)
  {
  }
  template<typename Caller>
  Callback0(const BindFirstOpaque<Caller>& caller) : Base(caller.getEnvironment(), BindFirstOpaque<Caller>::thunk)
  {
  }
  Callback0(void* environment, typename Base::Thunk function) : Base(environment, function)
  {
  }
  result_type operator()() const
  {
    return Base::getThunk()(Base::getEnvironment());
  }
};

template<typename Caller>
inline Callback0<typename Caller::result_type> makeCallback0(const Caller& caller, typename Caller::first_argument_type callee)
{
  return Callback0<typename Caller::result_type>(BindFirstOpaque<Caller>(callee));
}
template<typename Caller>
inline Callback0<typename Caller::result_type> makeStatelessCallback0(const Caller& caller)
{
  return makeCallback0(Caller0To1<Caller>(), 0);
}

typedef Callback0<void> Callback;



/// \brief Combines a void pointer with a pointer to a function which operates on a void pointer and one other argument. 
///
/// Use with the callback constructors MemberCaller1, ConstMemberCaller1, ReferenceCaller1, ConstReferenceCaller1, PointerCaller1, ConstPointerCaller1 and FreeCaller1.
template<typename FirstArgument, typename Result>
class Callback1 : public CallbackBase<Result (*)(void*, FirstArgument)>
{
  typedef CallbackBase<Result (*)(void*, FirstArgument)> Base;
  static Result nullThunk(void*, FirstArgument)
  {
  }

public:
  typedef FirstArgument first_argument_type;
  typedef Result result_type;

  Callback1() : Base(0, nullThunk)
  {
  }
  template<typename Caller>
  Callback1(const BindFirstOpaque1<Caller>& caller) : Base(caller.getEnvironment(), BindFirstOpaque1<Caller>::thunk)
  {
  }
  Callback1(void* environment, typename Base::Thunk function) : Base(environment, function)
  {
  }
  result_type operator()(FirstArgument firstArgument) const
  {
    return Base::getThunk()(Base::getEnvironment(), firstArgument);
  }
};

template<typename Caller>
inline Callback1<typename Caller::second_argument_type, typename Caller::result_type> makeCallback1(const Caller& caller, typename Caller::first_argument_type callee)
{
  return Callback1<typename Caller::second_argument_type, typename Caller::result_type>(BindFirstOpaque1<Caller>(callee));
}
template<typename Caller>
inline Callback1<typename Caller::second_argument_type, typename Caller::result_type> makeStatelessCallback1(const Caller& caller)
{
  return makeCallback1(Caller1To2<Caller>(), 0);
}


/// \brief Combines a void pointer with a pointer to a function which operates on a void pointer and two other arguments. 
///
template<typename FirstArgument, typename SecondArgument, typename Result>
class Callback2 : public CallbackBase<Result (*)(void*, FirstArgument, SecondArgument)>
{
  typedef CallbackBase<Result (*)(void*, FirstArgument, SecondArgument)> Base;
  static Result nullThunk(void*, FirstArgument, SecondArgument)
  {
  }

public:
  typedef FirstArgument first_argument_type;
  typedef SecondArgument second_argument_type;
  typedef Result result_type;

  Callback2() : Base(0, nullThunk)
  {
  }
  template<typename Caller>
  Callback2(const BindFirstOpaque2<Caller>& caller) : Base(caller.getEnvironment(), BindFirstOpaque2<Caller>::thunk)
  {
  }
  Callback2(void* environment, typename Base::Thunk function) : Base(environment, function)
  {
  }
  result_type operator()(FirstArgument firstArgument, SecondArgument secondArgument) const
  {
    return Base::getThunk()(Base::getEnvironment(), firstArgument, secondArgument);
  }
};

template<typename Caller>
inline Callback2<
  typename Caller::second_argument_type,
  typename Caller::third_argument_type,
  typename Caller::result_type
> makeCallback2(const Caller& caller, typename Caller::first_argument_type callee)
{
  return Callback2<
    typename Caller::second_argument_type,
    typename Caller::third_argument_type,
    typename Caller::result_type
  >(BindFirstOpaque2<Caller>(callee));
}
template<typename Caller>
inline Callback2<
  typename Caller::first_argument_type,
  typename Caller::second_argument_type,
  typename Caller::result_type
> makeStatelessCallback2(const Caller& caller)
{
  return makeCallback2(Caller2To3<Caller>(), 0);
}


/// \brief Combines a void pointer with a pointer to a function which operates on a void pointer and three other arguments. 
///
template<typename FirstArgument, typename SecondArgument, typename ThirdArgument, typename Result>
class Callback3 : public CallbackBase<Result (*)(void*, FirstArgument, SecondArgument, ThirdArgument)>
{
  typedef CallbackBase<Result (*)(void*, FirstArgument, SecondArgument, ThirdArgument)> Base;
  static Result nullThunk(void*, FirstArgument, SecondArgument, ThirdArgument)
  {
  }

public:
  typedef FirstArgument first_argument_type;
  typedef SecondArgument second_argument_type;
  typedef ThirdArgument third_argument_type;
  typedef Result result_type;

  Callback3() : Base(0, nullThunk)
  {
  }
  template<typename Caller>
  Callback3(const BindFirstOpaque3<Caller>& caller) : Base(caller.getEnvironment(), BindFirstOpaque3<Caller>::thunk)
  {
  }
  Callback3(void* environment, typename Base::Thunk function) : Base(environment, function)
  {
  }
  result_type operator()(FirstArgument firstArgument, SecondArgument secondArgument, ThirdArgument thirdArgument) const
  {
    return Base::getThunk()(Base::getEnvironment(), firstArgument, secondArgument, thirdArgument);
  }
};

template<typename Caller>
inline Callback3<
  typename Caller::second_argument_type,
  typename Caller::third_argument_type,
  typename Caller::fourth_argument_type,
  typename Caller::result_type
> makeCallback3(const Caller& caller, typename Caller::first_argument_type callee)
{
  return Callback3<
    typename Caller::second_argument_type,
    typename Caller::third_argument_type,
    typename Caller::fourth_argument_type,
    typename Caller::result_type
  >(BindFirstOpaque3<Caller>(callee));
}
template<typename Caller>
inline Callback3<
  typename Caller::first_argument_type,
  typename Caller::second_argument_type,
  typename Caller::third_argument_type,
  typename Caller::result_type
> makeStatelessCallback3(const Caller& caller)
{
  return makeCallback3(Caller3To4<Caller>(), 0);
}


/// \brief Forms a Callback from a non-const Environment reference and a non-const Environment member-function.
///
/// \dontinclude generic/callback.cpp
/// \skipline MemberCaller example
/// \until end example
template<typename Environment, void (Environment::*member)()>
class MemberCaller : public BindFirstOpaque< Member<Environment, void, member> >
{
public:
  MemberCaller(Environment& environment) : BindFirstOpaque< Member<Environment, void, member> >(environment)
  {
  }
};

/// \brief Forms a Callback from a const Environment reference and a const Environment member-function.
///
/// \dontinclude generic/callback.cpp
/// \skipline MemberCaller example
/// \until end example
template<typename Environment, void (Environment::*member)() const>
class ConstMemberCaller : public BindFirstOpaque< ConstMember<Environment, void, member> >
{
public:
  ConstMemberCaller(const Environment& environment) : BindFirstOpaque< ConstMember<Environment, void, member> >(environment)
  {
  }
};

/// \brief Forms a Callback from a non-const Environment reference and a const Environment member-function which takes one argument.
template<typename Environment, typename FirstArgument, void (Environment::*member)(FirstArgument)>
class MemberCaller1 : public BindFirstOpaque1< Member1<Environment, FirstArgument, void, member> >
{
public:
  MemberCaller1(Environment& environment) : BindFirstOpaque1< Member1<Environment, FirstArgument, void, member> >(environment)
  {
  }
};

/// \brief Forms a Callback from a const Environment reference and a const Environment member-function which takes one argument.
template<typename Environment, typename FirstArgument, void (Environment::*member)(FirstArgument) const>
class ConstMemberCaller1 : public BindFirstOpaque1< ConstMember1<Environment, FirstArgument, void, member> >
{
public:
  ConstMemberCaller1(const Environment& environment) : BindFirstOpaque1< ConstMember1<Environment, FirstArgument, void, member> >(environment)
  {
  }
};

/// \brief Forms a Callback from a non-const Environment reference and a free function which operates on a non-const Environment reference.
///
/// \dontinclude generic/callback.cpp
/// \skipline ReferenceCaller example
/// \until end example
template<typename Environment, void (*func)(Environment&)> 
class ReferenceCaller : public BindFirstOpaque< Function1<Environment&, void, func> >
{
public:
  ReferenceCaller(Environment& environment) : BindFirstOpaque< Function1<Environment&, void, func> >(environment)
  {
  }
};

/// \brief Forms a Callback from a const Environment reference and a free function which operates on a const Environment reference.
///
/// \dontinclude generic/callback.cpp
/// \skipline ReferenceCaller example
/// \until end example
template<typename Environment, void (*func)(const Environment&)> 
class ConstReferenceCaller : public BindFirstOpaque< Function1<const Environment&, void, func> >
{
public:
  ConstReferenceCaller(const Environment& environment) : BindFirstOpaque< Function1<const Environment&, void, func> >(environment)
  {
  }
};

/// \brief Forms a Callback from a non-const Environment reference and a free function which operates on a non-const Environment reference and one other argument.
template<typename Environment, typename FirstArgument, void (*func)(Environment&, FirstArgument)> 
class ReferenceCaller1 : public BindFirstOpaque1< Function2<Environment&, FirstArgument, void, func> >
{
public:
  ReferenceCaller1(Environment& environment) : BindFirstOpaque1< Function2<Environment&, FirstArgument, void, func> >(environment)
  {
  }
};

/// \brief Forms a Callback from a const Environment reference and a free function which operates on a const Environment reference and one other argument.
template<typename Environment, typename FirstArgument, void (*func)(const Environment&, FirstArgument)> 
class ConstReferenceCaller1 : public BindFirstOpaque1< Function2<const Environment&, FirstArgument, void, func> >
{
public:
  ConstReferenceCaller1(const Environment& environment) : BindFirstOpaque1< Function2<const Environment&, FirstArgument, void, func> >(environment)
  {
  }
};

/// \brief Forms a Callback from a non-const Environment pointer and a free function which operates on a non-const Environment pointer.
template<typename Environment, void (*func)(Environment*)> 
class PointerCaller : public BindFirstOpaque< Function1<Environment*, void, func> >
{
public:
  PointerCaller(Environment* environment) : BindFirstOpaque< Function1<Environment*, void, func> >(environment)
  {
  }
};

/// \brief Forms a Callback from a const Environment pointer and a free function which operates on a const Environment pointer.
template<typename Environment, void (*func)(const Environment*)> 
class ConstPointerCaller : public BindFirstOpaque< Function1<const Environment*, void, func> >
{
public:
  ConstPointerCaller(const Environment* environment) : BindFirstOpaque< Function1<const Environment*, void, func> >(environment)
  {
  }
};

/// \brief Forms a Callback from a non-const Environment pointer and a free function which operates on a non-const Environment pointer and one other argument.
template<typename Environment, typename FirstArgument, void (*func)(Environment*, FirstArgument)> 
class PointerCaller1 : public BindFirstOpaque1< Function2<Environment*, FirstArgument, void, func> >
{
public:
  PointerCaller1(Environment* environment) : BindFirstOpaque1< Function2<Environment*, FirstArgument, void, func> >(environment)
  {
  }
};

/// \brief Forms a Callback from a const Environment pointer and a free function which operates on a const Environment pointer and one other argument.
template<typename Environment, typename FirstArgument, void (*func)(const Environment*, FirstArgument)> 
class ConstPointerCaller1 : public BindFirstOpaque1< Function2<const Environment*, FirstArgument, void, func> >
{
public:
  ConstPointerCaller1(const Environment* environment) : BindFirstOpaque1< Function2<const Environment*, FirstArgument, void, func> >(environment)
  {
  }
};

/// \brief Forms a Callback from a free function which takes no arguments.
template<void (*func)()> 
class FreeCaller : public BindFirstOpaque< Caller0To1< Function0<void, func> > >
{
public:
  FreeCaller() : BindFirstOpaque< Caller0To1< Function0<void, func> > >(0)
  {
  }
};

/// \brief Forms a Callback from a free function which takes a single argument.
template<typename FirstArgument, void (*func)(FirstArgument)> 
class FreeCaller1 : public BindFirstOpaque1< Caller1To2< Function1<FirstArgument, void, func> > >
{
public:
  FreeCaller1() : BindFirstOpaque1< Caller1To2< Function1<FirstArgument, void, func> > >(0)
  {
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


#endif
