
#if !defined(INCLUDED_CALLBACKFWD_H)
#define INCLUDED_CALLBACKFWD_H

template<typename Return>
class Callback0;
typedef Callback0<void> Callback;

template<typename FirstArgument, typename Result = void>
class Callback1;

template<typename FirstArgument, typename SecondArgument, typename Result = void>
class Callback2;

template<typename FirstArgument, typename SecondArgument, typename ThirdArgument, typename Result = void>
class Callback3;

#endif
