
#include "array.h"

namespace
{
class Bleh
{
Array<int> m_array;
public:
Bleh() : m_array( 16 ){
}
};

void testAutoArray(){
	Array<Bleh> array( 32 );
}
}