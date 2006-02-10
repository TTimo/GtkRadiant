/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

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
