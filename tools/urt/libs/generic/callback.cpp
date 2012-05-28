
#include "callback.h"

#if defined( _DEBUG ) || defined( DOXYGEN )

namespace ExampleMemberCaller
{
// MemberCaller example
class Integer
{
public:
int value;

void printValue() const {
	// print this->value here;
}

void setValue(){
	value = 3;
}
// a typedef to make things more readable
typedef MemberCaller<Integer, &Integer::setValue> SetValueCaller;
};

void example(){
	Integer foo = { 0 };

	{
		Callback bar = ConstMemberCaller<Integer, &Integer::printValue>( foo );

		// invoke the callback
		bar(); // foo.printValue()
	}


	{
		// use the typedef to improve readability
		Callback bar = Integer::SetValueCaller( foo );

		// invoke the callback
		bar(); // foo.setValue()
	}
}
// end example
}

namespace ExampleReferenceCaller
{
// ReferenceCaller example
void Int_printValue( const int& value ){
	// print value here;
}

void Int_setValue( int& value ){
	value = 3;
}

// a typedef to make things more readable
typedef ReferenceCaller<int, Int_setValue> IntSetValueCaller;

void example(){
	int foo = 0;

	{
		Callback bar = ConstReferenceCaller<int, Int_printValue>( foo );

		// invoke the callback
		bar(); // Int_printValue(foo)
	}


	{
		// use the typedef to improve readability
		Callback bar = IntSetValueCaller( foo );

		// invoke the callback
		bar(); // Int_setValue(foo)
	}
}
// end example
}

#endif
