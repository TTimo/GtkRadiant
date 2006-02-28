
#include "pooledstring.h"
#include "generic/static.h"

#if defined(_DEBUG)

namespace ExamplePooledString
{
  void testStuff()
  {
    PooledString< LazyStatic<StringPool> > a, b;
    a = "monkey";
    b = "monkey";
    a = "";
  }

  struct Always
  {
    Always()
    {
      testStuff();
    }
  } always;
}

#endif