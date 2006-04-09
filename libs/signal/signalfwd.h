
#if !defined(INCLUDED_SIGNALFWD_H)
#define INCLUDED_SIGNALFWD_H

class SignalHandler;
template<typename FirstArgument>
class SignalHandler1;
template<typename FirstArgument, typename SecondArgument>
class SignalHandler2;
template<typename FirstArgument, typename SecondArgument, typename ThirdArgument>
class SignalHandler3;

template<typename Context>
class Opaque;

///\brief A pointer that always has a well-defined value.
/// If no value is specified, the appropriate null value is used.
template<typename Type>
class Handle
{
  Type* p;
public:
  Handle() : p(0)
  {
  }
  explicit Handle(Type* p) : p(p)
  {
  }
  Type* get() const
  {
    return p;
  }
  bool isNull() const
  {
    return p == 0;
  }
};

template<typename Handler>
class SignalFwd
{
public:
  typedef Handle< Opaque<Handler> > handler_id_type;
};

typedef SignalFwd<SignalHandler>::handler_id_type SignalHandlerId;

#endif
