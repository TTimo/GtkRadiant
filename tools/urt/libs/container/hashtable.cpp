
#include "hashtable.h"

#if defined( _DEBUG ) || defined( DOXYGEN )

#include "hashfunc.h"

namespace ExampleHashTable
{
void testStuff(){
	// HashTable example
	typedef HashTable<CopiedString, int, HashString> MyHashTable;
	MyHashTable hashtable;
	hashtable["bleh"] = 5;
	hashtable.insert( "blah", 17 );
	hashtable["foo"] = 99;
	hashtable.insert( "bar", 23 );

	int bleh = ( *hashtable.find( "bleh" ) ).value; // 5
	int blah = hashtable["blah"]; // 17
	hashtable.erase( "foo" );
	MyHashTable::iterator barIter = hashtable.find( "bar" );
	hashtable.erase( barIter );

	for ( MyHashTable::iterator i = hashtable.begin(); i != hashtable.end(); ++i )
	{
		if ( ( *i ).key != "bleh" ) {
			++hashtable["count"]; // insertion does not invalidate iterators
		}
	}
	// end example
}

struct Always
{
	Always(){
		testStuff();
	}
} always;
}

#endif
