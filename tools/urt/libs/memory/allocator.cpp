
#include "memory/allocator.h"

#include <vector>

template<typename Value>
struct Vector
{
	typedef std::vector<Value, DefaultAllocator<Value> > Type;
};

namespace
{
class Bleh
{
int* m_blah;
public:
Bleh( int* blah ) : m_blah( blah ){
}
~Bleh(){
	*m_blah = 15;
}
};

void TestAllocator(){
	Vector<Bleh>::Type test;

	int i = 0;
	test.push_back( Bleh( &i ) );
}

void TestNewDelete(){
	{
		NamedAllocator<int> allocator( "test" );
		int* p = NamedNew<int>::type( allocator ).scalar();
		//new int();
		NamedDelete<int>::type( allocator ).scalar( p );
	}

	{
		int* p = New<int>().scalar( 3 );
		Delete<int>().scalar( p );
	}

	{
		int* p = New<int>().scalar( int(15.9) );
		Delete<int>().scalar( p );
	}

	{
		int* p = New<int>().vector( 15 );
		// new int[15]
		Delete<int>().vector( p, 15 );
	}
}
}