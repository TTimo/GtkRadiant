
#if !defined( INCLUDED_GENERIC_STATIC_H )
#define INCLUDED_GENERIC_STATIC_H

/// \file
/// \brief Template techniques for instantiating singletons.

#include <cstddef>

/// \brief A singleton which is statically initialised.
///
/// \param Type The singleton object type.
///
/// \dontinclude generic/static.cpp
/// \skipline Static example
/// \until end example
template<typename Type>
class Static
{
static Type m_instance;
public:
static Type& instance(){
	return m_instance;
}
};

template<typename Type>
Type Static<Type>::m_instance;


/// \brief A singleton which is lazily initialised.
/// The instance is constructed the first time it is referenced, and is never destroyed.
///
/// \param Type The singleton object type.
///
/// \dontinclude generic/static.cpp
/// \skipline LazyStatic example
/// \until end example
template<typename Type>
class LazyStatic
{
static Type* m_instance;   // this will be initialised to 0 by the CRT, according to the c++ standard
public:
static Type& instance(){
	if ( m_instance == 0 ) {
		m_instance = new Type; // allocate using 'new' to get the correct alignment
	}
	return *m_instance;
}
};

template<typename Type>
Type * LazyStatic<Type>::m_instance;


/// \brief A singleton which keeps a count of the number of times it is referenced.
///
/// The instance is constructed when its reference count changes from 0 to 1 and destroyed when its reference count changes from 1 to 0.
/// Use with SmartStatic.
///
/// \param Type The singleton object type.
template<typename Type>
class CountedStatic
{
static std::size_t m_refcount;   // this will be initialised to 0 by the CRT, according to the c++ standard
static Type* m_instance;
public:
static Type& instance(){
	return *m_instance;
}
static void capture(){
	if ( ++m_refcount == 1 ) {
		m_instance = new Type; // allocate using 'new' to get the correct alignment
	}
}
static void release(){
	if ( --m_refcount == 0 ) {
		delete m_instance;
	}
}
};

template<typename Type>
std::size_t CountedStatic<Type>::m_refcount; // this will be initialised to 0 by the CRT, according to the c++ standard
template<typename Type>
Type * CountedStatic<Type>::m_instance;

/// \brief A reference to a CountedStatic.
/// Guarantees that CountedStatic<Type> will be constructed for the lifetime of this object.
///
/// \param Type The type parameter of the CountedStatic to reference.
///
/// \dontinclude generic/static.cpp
/// \skipline SmartStatic example
/// \until end example
template<typename Type>
class SmartStatic
{
public:
SmartStatic(){
	CountedStatic<Type>::capture();
}
~SmartStatic(){
	CountedStatic<Type>::release();
}
Type& instance(){
	return CountedStatic<Type>::instance();
}
};


#endif
