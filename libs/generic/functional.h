
#if !defined(INCLUDED_FUNCTIONAL_H)
#define INCLUDED_FUNCTIONAL_H

template<typename Object, typename R, R (Object::*member)()>
class Member
{
public:
  typedef Object& first_argument_type;
  typedef R result_type;
  static result_type call(first_argument_type object)
  {
    return (object.*member)();
  }
};

template<typename Object, typename R, R (Object::*member)() const>
class ConstMember
{
public:
  typedef const Object& first_argument_type;
  typedef R result_type;
  static result_type call(first_argument_type object)
  {
    return (object.*member)();
  }
};

template<typename Object, typename A1, typename R, R (Object::*member)(A1)>
class Member1
{
public:
  typedef Object& first_argument_type;
  typedef A1 second_argument_type;
  typedef R result_type;
  static result_type call(first_argument_type object, second_argument_type a1)
  {
    return (object.*member)(a1);
  }
};

template<typename Object, typename A1, typename R, R (Object::*member)(A1) const>
class ConstMember1
{
public:
  typedef const Object& first_argument_type;
  typedef A1 second_argument_type;
  typedef R result_type;
  static result_type call(first_argument_type object, second_argument_type a1)
  {
    return (object.*member)(a1);
  }
};

template<typename Object, typename A2, typename A3, typename R, R (Object::*member)(A2, A3)>
class Member2
{
public:
  typedef Object& first_argument_type;
  typedef A2 second_argument_type;
  typedef A3 third_argument_type;
  typedef R result_type;
  static result_type call(first_argument_type object, second_argument_type a2, third_argument_type a3)
  {
    return (object.*member)(a2, a3);
  }
};

template<typename Object, typename A2, typename A3, typename R, R (Object::*member)(A2, A3) const>
class ConstMember2
{
public:
  typedef const Object& first_argument_type;
  typedef A2 second_argument_type;
  typedef A3 third_argument_type;
  typedef R result_type;
  static result_type call(first_argument_type object, second_argument_type a2, third_argument_type a3)
  {
    return (object.*member)(a2, a3);
  }
};

template<typename Object, typename A2, typename A3, typename A4, typename R, R (Object::*member)(A2, A3, A4)>
class Member3
{
public:
  typedef Object& first_argument_type;
  typedef A2 second_argument_type;
  typedef A3 third_argument_type;
  typedef A4 fourth_argument_type;
  typedef R result_type;
  static result_type call(first_argument_type object, second_argument_type a2, third_argument_type a3, fourth_argument_type a4)
  {
    return (object.*member)(a2, a3, a4);
  }
};

template<typename Object, typename A2, typename A3, typename A4, typename R, R (Object::*member)(A2, A3, A4) const>
class ConstMember3
{
public:
  typedef const Object& first_argument_type;
  typedef A2 second_argument_type;
  typedef A3 third_argument_type;
  typedef A4 fourth_argument_type;
  typedef R result_type;
  static result_type call(first_argument_type object, second_argument_type a2, third_argument_type a3, fourth_argument_type a4)
  {
    return (object.*member)(a2, a3, a4);
  }
};

template<typename R, R (*func)()> 
class Function0
{
public:
  typedef R result_type;
  static result_type call()
  {
    return (func)();
  }
};

template<typename A1, typename R, R (*func)(A1)> 
class Function1
{
public:
  typedef A1 first_argument_type;
  typedef R result_type;
  static result_type call(first_argument_type a1)
  {
    return (func)(a1);
  }
};

template<typename A1, typename A2, typename R, R (*func)(A1, A2)> 
class Function2
{
public:
  typedef A1 first_argument_type;
  typedef A2 second_argument_type;
  typedef R result_type;
  static result_type call(first_argument_type a1, second_argument_type a2)
  {
    return (func)(a1, a2);
  }
};

template<typename A1, typename A2, typename A3, typename R, R (*func)(A1, A2, A3)> 
class Function3
{
public:
  typedef A1 first_argument_type;
  typedef A2 second_argument_type;
  typedef A3 third_argument_type;
  typedef R result_type;
  static result_type call(first_argument_type a1, second_argument_type a2, third_argument_type a3)
  {
    return (func)(a1, a2, a3);
  }
};

template<typename A1, typename A2, typename A3, typename A4, typename R, R (*func)(A1, A2, A3, A4)> 
class Function4
{
public:
  typedef A1 first_argument_type;
  typedef A2 second_argument_type;
  typedef A3 third_argument_type;
  typedef A4 fourth_argument_type;
  typedef R result_type;
  static result_type call(first_argument_type a1, second_argument_type a2, third_argument_type a3, fourth_argument_type a4)
  {
    return (func)(a1, a2, a3, a4);
  }
};

template<typename Caller, typename FirstArgument = void*> 
class Caller0To1
{
public:
  typedef FirstArgument first_argument_type;
  typedef typename Caller::result_type result_type;
  static result_type call(first_argument_type)
  {
    return Caller::call();
  }
};

template<typename Caller, typename FirstArgument = void*> 
class Caller1To2
{
public:
  typedef FirstArgument first_argument_type;
  typedef typename Caller::first_argument_type second_argument_type;
  typedef typename Caller::result_type result_type;
  static result_type call(first_argument_type, second_argument_type a2)
  {
    return Caller::call(a2);
  }
};

template<typename Caller, typename FirstArgument = void*> 
class Caller2To3
{
public:
  typedef FirstArgument first_argument_type;
  typedef typename Caller::first_argument_type second_argument_type;
  typedef typename Caller::second_argument_type third_argument_type;
  typedef typename Caller::result_type result_type;
  static result_type call(first_argument_type, second_argument_type a2, third_argument_type a3)
  {
    return Caller::call(a2, a3);
  }
};

template<typename Caller, typename FirstArgument = void*> 
class Caller3To4
{
public:
  typedef FirstArgument first_argument_type;
  typedef typename Caller::first_argument_type second_argument_type;
  typedef typename Caller::second_argument_type third_argument_type;
  typedef typename Caller::third_argument_type fourth_argument_type;
  typedef typename Caller::result_type result_type;
  static result_type call(first_argument_type, second_argument_type a2, third_argument_type a3, fourth_argument_type a4)
  {
    return Caller::call(a2, a3, a4);
  }
};

template<typename Functor>
class FunctorInvoke
{
public:
  typedef typename Functor::result_type result_type;
  inline result_type operator()(Functor functor)
  {
    return functor();
  }
};

template<typename Functor>
class Functor1Invoke
{
  typename Functor::first_argument_type a1;
public:
  typedef typename Functor::first_argument_type first_argument_type;
  typedef typename Functor::result_type result_type;
  Functor1Invoke(first_argument_type a1) : a1(a1)
  {
  }
  inline result_type operator()(Functor functor)
  {
    return functor(a1);
  }
};

template<typename Functor>
class Functor2Invoke
{
  typename Functor::first_argument_type a1;
  typename Functor::second_argument_type a2;
public:
  typedef typename Functor::first_argument_type first_argument_type;
  typedef typename Functor::second_argument_type second_argument_type;
  typedef typename Functor::result_type result_type;
  Functor2Invoke(first_argument_type a1, second_argument_type a2)
    : a1(a1), a2(a2)
  {
  }
  inline result_type operator()(Functor functor)
  {
    return functor(a1, a2);
  }
};

template<typename Functor>
class Functor3Invoke
{
  typename Functor::first_argument_type a1;
  typename Functor::second_argument_type a2;
  typename Functor::third_argument_type a3;
public:
  typedef typename Functor::first_argument_type first_argument_type;
  typedef typename Functor::second_argument_type second_argument_type;
  typedef typename Functor::third_argument_type third_argument_type;
  typedef typename Functor::result_type result_type;
  Functor3Invoke(first_argument_type a1, second_argument_type a2, third_argument_type a3)
    : a1(a1), a2(a2), a3(a3)
  {
  }
  inline result_type operator()(Functor functor)
  {
    return functor(a1, a2, a3);
  }
};

template<typename Other, typename True, typename False, typename Type>
class TypeEqual
{
public:
  typedef False type;
};
template<typename Other, typename True, typename False>
class TypeEqual<Other, True, False, Other>
{
public:
  typedef True type;
};


#endif
