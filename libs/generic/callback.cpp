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

#include "callback.h"

#if defined(_DEBUG) || defined(DOXYGEN)

namespace ExampleMemberCaller
{
  // MemberCaller example
  class Integer
  {
  public:
    int value;

    void printValue() const
    {
      // print this->value here;
    }

    void setValue()
    {
      value = 3;
    }
    // a typedef to make things more readable
    typedef MemberCaller<Integer, &Integer::setValue> SetValueCaller;
  };

  void example()
  {
    Integer foo = { 0 };

    {
      Callback bar = ConstMemberCaller<Integer, &Integer::printValue>(foo);

      // invoke the callback
      bar(); // foo.printValue()
    }


    {
      // use the typedef to improve readability
      Callback bar = Integer::SetValueCaller(foo);

      // invoke the callback
      bar(); // foo.setValue()
    }
  }
  // end example
}

namespace ExampleReferenceCaller
{
  // ReferenceCaller example
  void Int_printValue(const int& value)
  {
    // print value here;
  }

  void Int_setValue(int& value)
  {
    value = 3;
  }

  // a typedef to make things more readable
  typedef ReferenceCaller<int, Int_setValue> IntSetValueCaller;

  void example()
  {
    int foo = 0;

    {
      Callback bar = ConstReferenceCaller<int, Int_printValue>(foo);

      // invoke the callback
      bar(); // Int_printValue(foo)
    }


    {
      // use the typedef to improve readability
      Callback bar = IntSetValueCaller(foo);

      // invoke the callback
      bar(); // Int_setValue(foo)
    }
  }
  // end example
}

#endif

namespace
{
  class A1
  {
  };
  class A2
  {
  };
  class A3
  {
  };
  class A4
  {
  };

  class Test
  {
  public:
    void test0()
    {
    }
    typedef Member<Test, void, &Test::test0> Test0;
    typedef MemberCaller<Test, &Test::test0> Test0Caller;
    void test0const() const
    {
    }
    typedef ConstMember<Test, void, &Test::test0const> Test0Const;
    typedef ConstMemberCaller<Test, &Test::test0const> Test0ConstCaller;
    void test1(A1)
    {
    }
    typedef Member1<Test, A1, void, &Test::test1> Test1;
    typedef MemberCaller1<Test, A1, &Test::test1> Test1Caller;
    void test1const(A1) const
    {
    }
    typedef ConstMember1<Test, A1, void, &Test::test1const> Test1Const;
    typedef ConstMemberCaller1<Test, A1, &Test::test1const> Test1ConstCaller;
    void test2(A1, A2)
    {
    }
    typedef Member2<Test, A1, A2, void, &Test::test2> Test2;
    void test2const(A1, A2) const
    {
    }
    typedef ConstMember2<Test, A1, A2, void, &Test::test2const> Test2Const;
    void test3(A1, A2, A3)
    {
    }
    typedef Member3<Test, A1, A2, A3, void, &Test::test3> Test3;
    void test3const(A1, A2, A3) const
    {
    }
    typedef ConstMember3<Test, A1, A2, A3, void, &Test::test3const> Test3Const;
  };

  void test0free()
  {
  }
  typedef FreeCaller<&test0free> Test0FreeCaller;
  void test1free(A1)
  {
  }
  typedef FreeCaller1<A1, &test1free> Test1FreeCaller;
  void test2free(A1, A2)
  {
  }
  typedef Function2<A1, A2, void, &test2free> Test2Free;
  void test3free(A1, A2, A3)
  {
  }
  typedef Function3<A1, A2, A3, void, &test3free> Test3Free;


  void test0(Test& test)
  {
  }
  typedef ReferenceCaller<Test, &test0> Test0Caller;

  void test0const(const Test& test)
  {
  }
  typedef ConstReferenceCaller<Test, &test0const> Test0ConstCaller;

  void test0p(Test* test)
  {
  }
  typedef PointerCaller<Test, &test0p> Test0PCaller;

  void test0constp(const Test* test)
  {
  }
  typedef ConstPointerCaller<Test, &test0constp> Test0ConstPCaller;

  void test1(Test& test, A1)
  {
  }
  typedef ReferenceCaller1<Test, A1, &test1> Test1Caller;

  void test1const(const Test& test, A1)
  {
  }
  typedef ConstReferenceCaller1<Test, A1, &test1const> Test1ConstCaller;

  void test1p(Test* test, A1)
  {
  }
  typedef PointerCaller1<Test, A1, &test1p> Test1PCaller;

  void test1constp(const Test* test, A1)
  {
  }
  typedef ConstPointerCaller1<Test, A1, &test1constp> Test1ConstPCaller;

  void test2(Test& test, A1, A2)
  {
  }
  typedef Function3<Test&, A1, A2, void, &test2> Test2;

  void test3(Test& test, A1, A2, A3)
  {
  }
  typedef Function4<Test&, A1, A2, A3, void, &test3> Test3;

  void instantiate()
  {
    Test test;
    const Test& testconst = test;
    {
      Callback a = Test0FreeCaller();
      Callback b = Test::Test0Caller(test);
      b = makeCallback0(Test::Test0(), test);
      Callback c = Test::Test0ConstCaller(testconst);
      c = makeCallback0(Test::Test0Const(), test);
      Callback d = Test0Caller(test);
      Callback e = Test0ConstCaller(testconst);
      Callback f = Test0PCaller(&test);
      Callback g = Test0ConstPCaller(&testconst);
      a();
      bool u = a != b;
    }
    {
      typedef Callback1<A1> TestCallback1;
      TestCallback1 a = Test1FreeCaller();
      TestCallback1 b = Test::Test1Caller(test);
      b = makeCallback1(Test::Test1(), test);
      TestCallback1 c = Test::Test1ConstCaller(testconst);
      c = makeCallback1(Test::Test1Const(), test);
      TestCallback1 d = Test1Caller(test);
      TestCallback1 e = Test1ConstCaller(testconst);
      TestCallback1 f = Test1PCaller(&test);
      TestCallback1 g = Test1ConstPCaller(&testconst);
      a(A1());
      bool u = a != b;
    }
    {
      typedef Callback2<A1, A2> TestCallback2;
      TestCallback2 a = makeStatelessCallback2(Test2Free());
      TestCallback2 b = makeCallback2(Test2(), test);
      TestCallback2 c = makeCallback2(Test::Test2(), test);
      TestCallback2 d = makeCallback2(Test::Test2Const(), test);
      a(A1(), A2());
      bool u = a != b;
    }
    {
      typedef Callback3<A1, A2, A3> TestCallback3;
      TestCallback3 a = makeStatelessCallback3(Test3Free());
      TestCallback3 b = makeCallback3(Test3(), test);
      TestCallback3 c = makeCallback3(Test::Test3(), test);
      TestCallback3 d = makeCallback3(Test::Test3Const(), test);
      a(A1(), A2(), A3());
      bool u = a != b;
    }
  }
}
