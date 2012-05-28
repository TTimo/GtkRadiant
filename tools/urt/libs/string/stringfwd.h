
#if !defined( INCLUDED_STRING_STRINGFWD_H )
#define INCLUDED_STRING_STRINGFWD_H

// forward-declaration of CopiedString

template<typename Type>
class DefaultAllocator;
template<typename Allocator>
class CopiedBuffer;
template<typename Buffer>
class String;
typedef String< CopiedBuffer< DefaultAllocator<char> > > CopiedString;

#endif
