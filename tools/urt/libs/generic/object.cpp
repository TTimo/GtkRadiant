
#include "object.h"

namespace
{
  class Blah
  {
    int i;
  public:
    Blah()
    {
      i = 3;
    }
  };

  void Test()
  {
    char storage[sizeof(Blah)];
    constructor(*reinterpret_cast<Blah*>(storage));
  }
}