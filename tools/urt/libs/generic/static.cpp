
#include "static.h"

#if defined(_DEBUG) || defined(DOXYGEN)

namespace ExampleStatic
{
  // Static example
  // ---- myclass.h
  class MyClass
  {
  public:
    int value;
    MyClass() : value(3)
    {
    }
  };

  typedef Static<MyClass> StaticMyClass;

  // ---- main.cpp
  class DynamicInitialisation
  {
  public:
    DynamicInitialisation()
    {
      // StaticMyClass::instance() may be invalid here because construction order is undefined
    }
  };

  DynamicInitialisation g_dynamicInitialisation;

  void duringMain()
  {
    int bar = StaticMyClass::instance().value;
  }
  // end example
}

namespace ExampleLazyStatic
{
  // LazyStatic example
  // ---- myclass.h
  class MyClass
  {
  public:
    int value;
    MyClass() : value(3)
    {
    }
    // destructor will never be called
  };

  typedef LazyStatic<MyClass> StaticMyClass;

  // ---- main.cpp
  class DynamicInitialisation
  {
  public:
    DynamicInitialisation()
    {
      int bar = StaticMyClass::instance().value;
    }
  };

  DynamicInitialisation g_dynamicInitialisation;

  void duringMain()
  {
    int bar = StaticMyClass::instance().value;
  }
  // end example
}

namespace ExampleSmartStatic
{
  // SmartStatic example
  // ---- myclass.h
  class MyClass
  {
  public:
    int value;
    MyClass() : value(3)
    {
    }
  };

  typedef CountedStatic<MyClass> StaticMyClass;

  // ---- main.cpp
  class DynamicInitialisation
  {
  public:
    DynamicInitialisation()
    {
      // StaticMyClass::instance() is invalid before the ref is constructed
      SmartStatic<MyClass> ref;
      int bar = ref.instance().value;

      SmartStatic<MyClass> ref2; // any number of instances are allowed.
    }
  };

  DynamicInitialisation g_dynamicInitialisation;

  void duringMain()
  {
    int bar = SmartStatic<MyClass>().instance().value; // an instance can be a temporary
  }
  // end example
}

#endif
