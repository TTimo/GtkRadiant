
#if !defined( INCLUDED_GENERIC_OBJECT_H )
#define INCLUDED_GENERIC_OBJECT_H

/// \file
/// \brief Convenience functions (syntactic sugar) to wrap explicit constructor (aka in-place 'new') and destructor calls.
///
/// Use makeReference() to wrap non-const-reference constructor parameters.

#if _MSC_VER > 1000 && defined( WIN32 )
#pragma warning(disable:4345) // behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized
#endif

#include <new>

template<typename Type>
inline void constructor( Type& object ){
	new( &object )Type();
}

template<typename Type, typename T1>
inline void constructor( Type& object, const T1& t1 ){
	new( &object )Type( t1 );
}

template<typename Type, typename T1, typename T2>
inline void constructor( Type& object, const T1& t1, const T2& t2 ){
	new( &object )Type( t1, t2 );
}

template<typename Type, typename T1, typename T2, typename T3>
inline void constructor( Type& object, const T1& t1, const T2& t2, const T3& t3 ){
	new( &object )Type( t1, t2, t3 );
}

template<typename Type, typename T1, typename T2, typename T3, typename T4>
inline void constructor( Type& object, const T1& t1, const T2& t2, const T3& t3, const T4& t4 ){
	new( &object )Type( t1, t2, t3, t4 );
}

template<typename Type, typename T1, typename T2, typename T3, typename T4, typename T5>
inline void constructor( Type& object, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5 ){
	new( &object )Type( t1, t2, t3, t4, t5 );
}

template<typename Type, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline void constructor( Type& object, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6 ){
	new( &object )Type( t1, t2, t3, t4, t5, t6 );
}

template<typename Type, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
inline void constructor( Type& object, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7 ){
	new( &object )Type( t1, t2, t3, t4, t5, t6, t7 );
}

template<typename Type, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
inline void constructor( Type& object, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8 ){
	new( &object )Type( t1, t2, t3, t4, t5, t6, t7, t8 );
}

template<typename Type>
inline void destructor( Type& object ){
	object.~Type();
}



#endif
