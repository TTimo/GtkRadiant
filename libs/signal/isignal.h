
#if !defined(INCLUDED_ISIGNAL_H)
#define INCLUDED_ISIGNAL_H

#include "generic/callback.h"
#include "signal/signalfwd.h"

class SignalHandlerResult
{
  bool value;
public:
  explicit SignalHandlerResult(bool value) : value(value)
  {
  }
  bool operator==(SignalHandlerResult other) const
  {
    return value == other.value;
  }
  bool operator!=(SignalHandlerResult other) const
  {
    return !operator==(other);
  }
};

const SignalHandlerResult SIGNAL_CONTINUE_EMISSION = SignalHandlerResult(false);
const SignalHandlerResult SIGNAL_STOP_EMISSION = SignalHandlerResult(true);

template<typename Caller> 
class SignalHandlerCaller1
{
public:
  typedef typename Caller::first_argument_type first_argument_type;
  typedef SignalHandlerResult result_type;
  static result_type call(first_argument_type a1)
  {
    Caller::call(a1);
    return SIGNAL_CONTINUE_EMISSION;
  }
};

template<typename Caller> 
class SignalHandlerCaller2
{
public:
  typedef typename Caller::first_argument_type first_argument_type;
  typedef typename Caller::second_argument_type second_argument_type;
  typedef SignalHandlerResult result_type;
  static result_type call(first_argument_type a1, second_argument_type a2)
  {
    Caller::call(a1, a2);
    return SIGNAL_CONTINUE_EMISSION;
  }
};

template<typename Caller> 
class SignalHandlerCaller3
{
public:
  typedef typename Caller::first_argument_type first_argument_type;
  typedef typename Caller::second_argument_type second_argument_type;
  typedef typename Caller::third_argument_type third_argument_type;
  typedef SignalHandlerResult result_type;
  static result_type call(first_argument_type a1, second_argument_type a2, third_argument_type a3)
  {
    Caller::call(a1, a2, a3);
    return SIGNAL_CONTINUE_EMISSION;
  }
};

template<typename Caller> 
class SignalHandlerCaller4
{
public:
  typedef typename Caller::first_argument_type first_argument_type;
  typedef typename Caller::second_argument_type second_argument_type;
  typedef typename Caller::third_argument_type third_argument_type;
  typedef typename Caller::fourth_argument_type fourth_argument_type;
  typedef SignalHandlerResult result_type;
  static result_type call(first_argument_type a1, second_argument_type a2, third_argument_type a3, fourth_argument_type a4)
  {
    Caller::call(a1, a2, a3, a4);
    return SIGNAL_CONTINUE_EMISSION;
  }
};

class SignalHandler : public Callback0<SignalHandlerResult>
{
public:
  template<typename Caller>
  SignalHandler(const BindFirstOpaque<Caller>& caller)
    : Callback0<SignalHandlerResult>(BindFirstOpaque<typename TypeEqual<
      SignalHandlerResult,
      Caller,
      SignalHandlerCaller1<Caller>,
      typename Caller::result_type
    >::type>(caller.getBound()))
  {
  }
};

template<typename Caller>
inline SignalHandler makeSignalHandler(const BindFirstOpaque<Caller>& caller)
{
  return SignalHandler(caller);
}
template<typename Caller>
inline SignalHandler makeSignalHandler(const Caller& caller, typename Caller::first_argument_type callee)
{
  return SignalHandler(BindFirstOpaque<Caller>(callee));
}


template<typename FirstArgument>
class SignalHandler1 : public Callback1<FirstArgument, SignalHandlerResult>
{
public:
  template<typename Caller>
  SignalHandler1(const BindFirstOpaque1<Caller>& caller)
    : Callback1<FirstArgument, SignalHandlerResult>(BindFirstOpaque1<typename TypeEqual<
      SignalHandlerResult,
      Caller,
      SignalHandlerCaller2<Caller>,
      typename Caller::result_type
    >::type>(caller.getBound()))
  {
  }
};

template<typename Caller>
inline SignalHandler1<typename Caller::second_argument_type> makeSignalHandler1(const BindFirstOpaque1<Caller>& caller)
{
  return SignalHandler1<typename Caller::second_argument_type>(caller);
}
template<typename Caller>
inline SignalHandler1<typename Caller::second_argument_type> makeSignalHandler1(const Caller& caller, typename Caller::first_argument_type callee)
{
  return SignalHandler1<typename Caller::second_argument_type>(BindFirstOpaque1<Caller>(callee));
}


template<typename FirstArgument, typename SecondArgument>
class SignalHandler2 : public Callback2<FirstArgument, SecondArgument, SignalHandlerResult>
{
public:
  template<typename Caller>
  SignalHandler2(const BindFirstOpaque2<Caller>& caller)
    : Callback2<FirstArgument, SecondArgument, SignalHandlerResult>(BindFirstOpaque2<typename TypeEqual<
      SignalHandlerResult,
      Caller,
      SignalHandlerCaller3<Caller>,
      typename Caller::result_type
    >::type>(caller.getBound()))
  {
  }
};

template<typename Caller>
inline SignalHandler2<
  typename Caller::second_argument_type,
  typename Caller::third_argument_type
> makeSignalHandler2(const BindFirstOpaque2<Caller>& caller)
{
  return SignalHandler2<
    typename Caller::second_argument_type,
    typename Caller::third_argument_type
  >(caller);
}
template<typename Caller>
inline SignalHandler2<
  typename Caller::second_argument_type,
  typename Caller::third_argument_type
> makeSignalHandler2(const Caller& caller, typename Caller::first_argument_type callee)
{
  return SignalHandler2<
    typename Caller::second_argument_type,
    typename Caller::third_argument_type
  >(BindFirstOpaque2<Caller>(callee));
}


template<typename FirstArgument, typename SecondArgument, typename ThirdArgument>
class SignalHandler3 : public Callback3<FirstArgument, SecondArgument, ThirdArgument, SignalHandlerResult>
{
public:
  template<typename Caller>
  SignalHandler3(const BindFirstOpaque3<Caller>& caller)
    : Callback3<FirstArgument, SecondArgument, ThirdArgument, SignalHandlerResult>(BindFirstOpaque3<typename TypeEqual<
      SignalHandlerResult,
      Caller,
      SignalHandlerCaller4<Caller>,
      typename Caller::result_type
    >::type>(caller.getBound()))
  {
  }
};

template<typename Caller>
inline SignalHandler3<
  typename Caller::second_argument_type,
  typename Caller::third_argument_type,
  typename Caller::fourth_argument_type
> makeSignalHandler3(const BindFirstOpaque3<Caller>& caller)
{
  return SignalHandler3<
    typename Caller::second_argument_type,
    typename Caller::third_argument_type,
    typename Caller::fourth_argument_type
  >(caller);
}
template<typename Caller>
inline SignalHandler3<
  typename Caller::second_argument_type,
  typename Caller::third_argument_type,
  typename Caller::fourth_argument_type
> makeSignalHandler3(const Caller& caller, typename Caller::first_argument_type callee)
{
  return SignalHandler3<
    typename Caller::second_argument_type,
    typename Caller::third_argument_type,
    typename Caller::fourth_argument_type
  >(BindFirstOpaque3<Caller>(callee));
}

#endif
