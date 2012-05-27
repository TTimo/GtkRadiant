
#include "singletonmodule.h"

class NullType
{
public:
  enum { VERSION = 1 };
  static const char* name()
  {
    return "";
  }
};

class NullModule
{
public:
  typedef NullType Type;
  static const char* getName()
  {
    return "";
  }
  void* getTable()
  {
    return NULL;
  }
};

void TEST_SINGLETONMODULE()
{
  SingletonModule<NullModule> null;
  null.capture();
  null.release();
}
