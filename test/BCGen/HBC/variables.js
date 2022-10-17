/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -strict -O0 -target=HBC -dump-lir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermes -strict -O -target=HBC -dump-lir %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKOPT %s

var a = 5;
a;

function bar(a) {
  return a;
}

function foo(a) {
  var b = bar(a);
  return b;
}

function daa(a) {
  var b = a + 1;
  function daa_capture() {
    return b;
  }
  return daa_capture;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [a, bar, foo, daa]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHECK-NEXT:  %1 = HBCGetGlobalObjectInst
// CHECK-NEXT:  %2 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  %3 = HBCLoadConstInst 5 : number
// CHECK-NEXT:  %4 = HBCCreateFunctionInst %bar#0#1()#2, %0
// CHECK-NEXT:  %5 = StorePropertyInst %4 : closure, %1 : object, "bar" : string
// CHECK-NEXT:  %6 = HBCCreateFunctionInst %foo#0#1()#3, %0
// CHECK-NEXT:  %7 = StorePropertyInst %6 : closure, %1 : object, "foo" : string
// CHECK-NEXT:  %8 = HBCCreateFunctionInst %daa#0#1()#4, %0
// CHECK-NEXT:  %9 = StorePropertyInst %8 : closure, %1 : object, "daa" : string
// CHECK-NEXT:  %10 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %11 = StoreStackInst %2 : undefined, %10
// CHECK-NEXT:  %12 = StorePropertyInst %3 : number, %1 : object, "a" : string
// CHECK-NEXT:  %13 = LoadPropertyInst %1 : object, "a" : string
// CHECK-NEXT:  %14 = StoreStackInst %13, %10
// CHECK-NEXT:  %15 = LoadStackInst %10
// CHECK-NEXT:  %16 = ReturnInst %15
// CHECK-NEXT:function_end

// CHECK:function bar#0#1(a)#2
// CHECK-NEXT:frame = [a#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCCreateEnvironmentInst %S{bar#0#1()#2}
// CHECK-NEXT:  %1 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  %2 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  %3 = HBCStoreToEnvironmentInst %0, %1, [a#2]
// CHECK-NEXT:  %4 = HBCLoadFromEnvironmentInst %0, [a#2]
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst %2 : undefined
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(a)#3
// CHECK-NEXT:frame = [a#3, b#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCCreateEnvironmentInst %S{foo#0#1()#3}
// CHECK-NEXT:  %1 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  %2 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  %3 = HBCGetGlobalObjectInst
// CHECK-NEXT:  %4 = HBCStoreToEnvironmentInst %0, %1, [a#3]
// CHECK-NEXT:  %5 = HBCStoreToEnvironmentInst %0, %2 : undefined, [b#3]
// CHECK-NEXT:  %6 = LoadPropertyInst %3 : object, "bar" : string
// CHECK-NEXT:  %7 = HBCLoadFromEnvironmentInst %0, [a#3]
// CHECK-NEXT:  %8 = CallInst %6, %2 : undefined, %7
// CHECK-NEXT:  %9 = HBCStoreToEnvironmentInst %0, %8, [b#3]
// CHECK-NEXT:  %10 = HBCLoadFromEnvironmentInst %0, [b#3]
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = ReturnInst %2 : undefined
// CHECK-NEXT:function_end

// CHECK:function daa#0#1(a)#4
// CHECK-NEXT:frame = [a#4, b#4, daa_capture#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCCreateEnvironmentInst %S{daa#0#1()#4}
// CHECK-NEXT:  %1 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  %2 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  %3 = HBCLoadConstInst 1 : number
// CHECK-NEXT:  %4 = HBCStoreToEnvironmentInst %0, %1, [a#4]
// CHECK-NEXT:  %5 = HBCStoreToEnvironmentInst %0, %2 : undefined, [b#4]
// CHECK-NEXT:  %6 = HBCCreateFunctionInst %daa_capture#1#4()#5, %0
// CHECK-NEXT:  %7 = HBCStoreToEnvironmentInst %0, %6 : closure, [daa_capture#4]
// CHECK-NEXT:  %8 = HBCLoadFromEnvironmentInst %0, [a#4]
// CHECK-NEXT:  %9 = BinaryOperatorInst '+', %8, %3 : number
// CHECK-NEXT:  %10 = HBCStoreToEnvironmentInst %0, %9, [b#4]
// CHECK-NEXT:  %11 = HBCLoadFromEnvironmentInst %0, [daa_capture#4]
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = ReturnInst %2 : undefined
// CHECK-NEXT:function_end

// CHECK:function daa_capture#1#4()#5
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCCreateEnvironmentInst %S{daa_capture#1#4()#5}
// CHECK-NEXT:  %1 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  %2 = HBCResolveEnvironment %S{daa#0#1()#4}, %S{daa_capture#1#4()#5}
// CHECK-NEXT:  %3 = HBCLoadFromEnvironmentInst %2, [b#4@daa]
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst %1 : undefined
// CHECK-NEXT:function_end

// CHKOPT:function global#0()#1
// CHKOPT-NEXT:frame = [], globals = [a, bar, foo, daa]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHKOPT-NEXT:  %1 = HBCCreateFunctionInst %bar#0#1()#2, %0
// CHKOPT-NEXT:  %2 = HBCGetGlobalObjectInst
// CHKOPT-NEXT:  %3 = StorePropertyInst %1 : closure, %2 : object, "bar" : string
// CHKOPT-NEXT:  %4 = HBCCreateFunctionInst %foo#0#1()#3, %0
// CHKOPT-NEXT:  %5 = StorePropertyInst %4 : closure, %2 : object, "foo" : string
// CHKOPT-NEXT:  %6 = HBCCreateFunctionInst %daa#0#1()#4 : closure, %0
// CHKOPT-NEXT:  %7 = StorePropertyInst %6 : closure, %2 : object, "daa" : string
// CHKOPT-NEXT:  %8 = HBCLoadConstInst 5 : number
// CHKOPT-NEXT:  %9 = StorePropertyInst %8 : number, %2 : object, "a" : string
// CHKOPT-NEXT:  %10 = LoadPropertyInst %2 : object, "a" : string
// CHKOPT-NEXT:  %11 = ReturnInst %10
// CHKOPT-NEXT:function_end

// CHKOPT:function bar#0#1(a)#2
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = HBCLoadParamInst 1 : number
// CHKOPT-NEXT:  %1 = ReturnInst %0
// CHKOPT-NEXT:function_end

// CHKOPT:function foo#0#1(a)#3
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = HBCGetGlobalObjectInst
// CHKOPT-NEXT:  %1 = LoadPropertyInst %0 : object, "bar" : string
// CHKOPT-NEXT:  %2 = HBCLoadConstInst undefined : undefined
// CHKOPT-NEXT:  %3 = HBCLoadParamInst 1 : number
// CHKOPT-NEXT:  %4 = HBCCallNInst %1, %2 : undefined, %3
// CHKOPT-NEXT:  %5 = ReturnInst %4
// CHKOPT-NEXT:function_end

// CHKOPT:function daa#0#1(a)#4 : closure
// CHKOPT-NEXT:frame = [b#4 : undefined|string|number]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = HBCCreateEnvironmentInst %S{daa#0#1()#4}
// CHKOPT-NEXT:  %1 = HBCLoadParamInst 1 : number
// CHKOPT-NEXT:  %2 = HBCLoadConstInst 1 : number
// CHKOPT-NEXT:  %3 = BinaryOperatorInst '+', %1, %2 : number
// CHKOPT-NEXT:  %4 = HBCStoreToEnvironmentInst %0, %3 : string|number, [b#4] : undefined|string|number
// CHKOPT-NEXT:  %5 = HBCCreateFunctionInst %daa_capture#1#4()#5 : undefined|string|number, %0
// CHKOPT-NEXT:  %6 = ReturnInst %5 : closure
// CHKOPT-NEXT:function_end

// CHKOPT:function daa_capture#1#4()#5 : undefined|string|number
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = HBCResolveEnvironment %S{daa#0#1()#4}, %S{daa_capture#1#4()#5}
// CHKOPT-NEXT:  %1 = HBCLoadFromEnvironmentInst %0, [b#4@daa] : undefined|string|number
// CHKOPT-NEXT:  %2 = ReturnInst %1
// CHKOPT-NEXT:function_end
