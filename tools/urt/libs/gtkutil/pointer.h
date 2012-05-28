
#if !defined( INCLUDED_GTKUTIL_POINTER_H )
#define INCLUDED_GTKUTIL_POINTER_H

typedef int gint;
typedef void* gpointer;

#include <cstddef>

inline gint gpointer_to_int( gpointer p ){
	return gint( std::size_t( p ) );
}

inline gpointer gint_to_pointer( gint i ){
	return gpointer( std::size_t( i ) );
}

#endif
