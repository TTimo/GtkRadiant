
#include "signal.h"



namespace
{
  class Test
  {
  };
  class A1
  {
  };
  class A2
  {
  };
  class A3
  {
  };

  SignalHandlerResult handler0(Test&)
  {
    return SIGNAL_CONTINUE_EMISSION;
  }
  typedef Function1<Test&, SignalHandlerResult, handler0> TestHandler0;

  int function0(Test&)
  {
    return 7;
  }
  typedef Function1<Test&, int, function0> TestFunction0;

  SignalHandlerResult handler1(Test&, A1)
  {
    return SIGNAL_CONTINUE_EMISSION;
  }
  typedef Function2<Test&, A1, SignalHandlerResult, handler1> TestHandler1;

  void function1(Test&, A1)
  {
  }
  typedef ReferenceCaller1<Test, A1, function1> TestFunction1;

  SignalHandlerResult handler2(Test&, A1, A2)
  {
    return SIGNAL_CONTINUE_EMISSION;
  }
  typedef Function3<Test&, A1, A2, SignalHandlerResult, handler2> TestHandler2;

  void function2(Test&, A1, A2)
  {
  }
  typedef Function3<Test&, A1, A2, void, function2> TestFunction2;

  SignalHandlerResult handler3(Test&, A1, A2, A3)
  {
    return SIGNAL_CONTINUE_EMISSION;
  }
  typedef Function4<Test&, A1, A2, A3, SignalHandlerResult, handler3> TestHandler3;

  void function3(Test&, A1, A2, A3)
  {
  }
  typedef Function4<Test&, A1, A2, A3, void, function3> TestFunction3;

  void testSignals()
  {
    Test test;
    {
      Signal0 e0;
      Signal0::handler_id_type a = e0.connectLast(makeSignalHandler(TestHandler0(), test)); // signal handler from direct caller returning result
      Signal0::handler_id_type b = e0.connectFirst(makeSignalHandler(TestFunction0(), test)); // signal handler from direct caller returning int
      e0();
      e0.disconnect(a);
      e0.disconnect(b);
    }
    {
      typedef Signal1<A1> Signal1Test;
      Signal1Test e1;
      Signal1Test::handler_id_type a = e1.connectLast(makeSignalHandler1(TestHandler1(), test)); // signal handler from direct caller with one argument, returning result
      Signal1Test::handler_id_type b = e1.connectFirst(makeSignalHandler1(TestFunction1(test))); // signal handler from opaque caller with one argument, returning void
      e1(A1());
      e1.disconnect(a);
      e1.disconnect(b);
    }
    {
      typedef Signal2<A1, A2> Signal2Test;
      Signal2Test e2;
      Signal2Test::handler_id_type a = e2.connectLast(makeSignalHandler2(TestHandler2(), test)); // signal handler from direct caller with two arguments, returning result
      Signal2Test::handler_id_type b = e2.connectLast(makeSignalHandler2(TestFunction2(), test)); // signal handler from direct caller with two arguments, returning void
      e2(A1(), A2());
      e2.disconnect(a);
      e2.disconnect(b);
    }
    {
      typedef Signal3<A1, A2, A3> Signal3Test;
      Signal3Test e3;
      Signal3Test::handler_id_type a = e3.connectLast(makeSignalHandler3(TestHandler3(), test)); // signal handler from direct caller with three arguments, returning result
      Signal3Test::handler_id_type b = e3.connectLast(makeSignalHandler3(TestFunction3(), test)); // signal handler from direct caller with three arguments, returning void
      e3(A1(), A2(), A3());
      e3.disconnect(a);
      e3.disconnect(b);
    }
  }
}
