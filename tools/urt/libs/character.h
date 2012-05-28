
#if !defined( INCLUDED_CHARACTER_H )
#define INCLUDED_CHARACTER_H

/// \file
/// \brief Character encoding.

/// \brief Returns true if \p c is an ASCII character that can be represented with 7 bits.
inline bool char_is_ascii( char c ){
	return ( c & 0x80 ) == 0;
}

/// \brief Returns true if \p string consists entirely of ASCII characters.
inline bool string_is_ascii( const char* string ){
	while ( *string != '\0' )
	{
		if ( !char_is_ascii( *string++ ) ) {
			return false;
		}
	}
	return true;
}

#endif
