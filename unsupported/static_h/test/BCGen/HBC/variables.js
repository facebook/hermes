/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -strict -O0 -target=HBC -dump-lir %s | %FileCheck --match-full-lines %s
// RUN: %hermes -strict -O -target=HBC -dump-lir %s | %FileCheck --match-full-lines --check-prefix=CHKOPT %s

var a = 5;
a;
//CHECK-LABEL:function global()
//CHECK-NEXT:frame = [], globals = [a, bar, foo, daa]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = HBCCreateEnvironmentInst
//CHECK-NEXT:  %1 = HBCGetGlobalObjectInst
//CHECK-NEXT:  %2 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  %3 = HBCLoadConstInst 5 : number
//CHECK-NEXT:  %4 = HBCCreateFunctionInst %bar(), %0
//CHECK-NEXT:  %5 = StorePropertyInst %4 : closure, %1 : object, "bar" : string
//CHECK-NEXT:  %6 = HBCCreateFunctionInst %foo(), %0
//CHECK-NEXT:  %7 = StorePropertyInst %6 : closure, %1 : object, "foo" : string
//CHECK-NEXT:  %8 = HBCCreateFunctionInst %daa(), %0
//CHECK-NEXT:  %9 = StorePropertyInst %8 : closure, %1 : object, "daa" : string
//CHECK-NEXT:  %10 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:  %11 = StoreStackInst %2 : undefined, %10
//CHECK-NEXT:  %12 = StorePropertyInst %3 : number, %1 : object, "a" : string
//CHECK-NEXT:  %13 = LoadPropertyInst %1 : object, "a" : string
//CHECK-NEXT:  %14 = StoreStackInst %13, %10
//CHECK-NEXT:  %15 = LoadStackInst %10
//CHECK-NEXT:  %16 = ReturnInst %15
//CHECK-NEXT:function_end

//CHKOPT-LABEL:function global()
//CHKOPT-NEXT:frame = [], globals = [a, bar, foo, daa]
//CHKOPT-NEXT:%BB0:
//CHKOPT-NEXT:  %0 = HBCCreateEnvironmentInst
//CHKOPT-NEXT:  %1 = HBCCreateFunctionInst %bar(), %0
//CHKOPT-NEXT:  %2 = HBCGetGlobalObjectInst
//CHKOPT-NEXT:  %3 = StorePropertyInst %1 : closure, %2 : object, "bar" : string
//CHKOPT-NEXT:  %4 = HBCCreateFunctionInst %foo(), %0
//CHKOPT-NEXT:  %5 = StorePropertyInst %4 : closure, %2 : object, "foo" : string
//CHKOPT-NEXT:  %6 = HBCCreateFunctionInst %daa() : closure, %0
//CHKOPT-NEXT:  %7 = StorePropertyInst %6 : closure, %2 : object, "daa" : string
//CHKOPT-NEXT:  %8 = HBCLoadConstInst 5 : number
//CHKOPT-NEXT:  %9 = StorePropertyInst %8 : number, %2 : object, "a" : string
//CHKOPT-NEXT:  %10 = LoadPropertyInst %2 : object, "a" : string
//CHKOPT-NEXT:  %11 = ReturnInst %10
//CHKOPT-NEXT:function_end



function bar(a) {
  return a;
}
//CHECK-LABEL:function bar(a)
//CHECK-NEXT:frame = [a]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = HBCCreateEnvironmentInst
//CHECK-NEXT:  %1 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  %2 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  %3 = HBCStoreToEnvironmentInst %0, %1, [a]
//CHECK-NEXT:  %4 = HBCLoadFromEnvironmentInst %0, [a]
//CHECK-NEXT:  %5 = ReturnInst %4
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %6 = ReturnInst %2 : undefined
//CHECK-NEXT:function_end

//CHKOPT-LABEL:function bar(a)
//CHKOPT-NEXT:frame = []
//CHKOPT-NEXT:%BB0:
//CHKOPT-NEXT:  %0 = HBCLoadParamInst 1 : number
//CHKOPT-NEXT:  %1 = ReturnInst %0
//CHKOPT-NEXT:function_end


function foo(a) {
  var b = bar(a);
  return b;
}
//CHECK-LABEL:function foo(a)
//CHECK-NEXT:frame = [b, a]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = HBCCreateEnvironmentInst
//CHECK-NEXT:  %1 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  %2 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  %3 = HBCGetGlobalObjectInst
//CHECK-NEXT:  %4 = HBCStoreToEnvironmentInst %0, %2 : undefined, [b]
//CHECK-NEXT:  %5 = HBCStoreToEnvironmentInst %0, %1, [a]
//CHECK-NEXT:  %6 = LoadPropertyInst %3 : object, "bar" : string
//CHECK-NEXT:  %7 = HBCLoadFromEnvironmentInst %0, [a]
//CHECK-NEXT:  %8 = CallInst %6, %2 : undefined, %7
//CHECK-NEXT:  %9 = HBCStoreToEnvironmentInst %0, %8, [b]
//CHECK-NEXT:  %10 = HBCLoadFromEnvironmentInst %0, [b]
//CHECK-NEXT:  %11 = ReturnInst %10
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %12 = ReturnInst %2 : undefined
//CHECK-NEXT:function_end


//CHKOPT-LABEL:function foo(a)
//CHKOPT-NEXT:frame = []
//CHKOPT-NEXT:%BB0:
//CHKOPT-NEXT:  %0 = HBCGetGlobalObjectInst
//CHKOPT-NEXT:  %1 = LoadPropertyInst %0 : object, "bar" : string
//CHKOPT-NEXT:  %2 = HBCLoadConstInst undefined : undefined
//CHKOPT-NEXT:  %3 = HBCLoadParamInst 1 : number
//CHKOPT-NEXT:  %4 = HBCCallNInst %1, %2 : undefined, %3
//CHKOPT-NEXT:  %5 = ReturnInst %4
//CHKOPT-NEXT:function_end


function daa(a) {
  var b = a + 1;
  function daa_capture() {
    return b;
  }
  return daa_capture;
}
//CHECK-LABEL:function daa(a)
//CHECK-NEXT:frame = [b, daa_capture, a]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = HBCCreateEnvironmentInst
//CHECK-NEXT:  %1 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  %2 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  %3 = HBCLoadConstInst 1 : number
//CHECK-NEXT:  %4 = HBCStoreToEnvironmentInst %0, %2 : undefined, [b]
//CHECK-NEXT:  %5 = HBCStoreToEnvironmentInst %0, %1, [a]
//CHECK-NEXT:  %6 = HBCCreateFunctionInst %daa_capture(), %0
//CHECK-NEXT:  %7 = HBCStoreToEnvironmentInst %0, %6 : closure, [daa_capture]
//CHECK-NEXT:  %8 = HBCLoadFromEnvironmentInst %0, [a]
//CHECK-NEXT:  %9 = BinaryOperatorInst '+', %8, %3 : number
//CHECK-NEXT:  %10 = HBCStoreToEnvironmentInst %0, %9, [b]
//CHECK-NEXT:  %11 = HBCLoadFromEnvironmentInst %0, [daa_capture]
//CHECK-NEXT:  %12 = ReturnInst %11
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %13 = ReturnInst %2 : undefined
//CHECK-NEXT:function_end


//CHKOPT-LABEL:function daa(a) : closure
//CHKOPT-NEXT:frame = [b : undefined|string|number]
//CHKOPT-NEXT:%BB0:
//CHKOPT-NEXT:  %0 = HBCCreateEnvironmentInst
//CHKOPT-NEXT:  %1 = HBCLoadParamInst 1 : number
//CHKOPT-NEXT:  %2 = HBCLoadConstInst 1 : number
//CHKOPT-NEXT:  %3 = BinaryOperatorInst '+', %1, %2 : number
//CHKOPT-NEXT:  %4 = HBCStoreToEnvironmentInst %0, %3 : string|number, [b] : undefined|string|number
//CHKOPT-NEXT:  %5 = HBCCreateFunctionInst %daa_capture() : undefined|string|number, %0
//CHKOPT-NEXT:  %6 = ReturnInst %5 : closure
//CHKOPT-NEXT:function_end


//CHECK-LABEL:function daa_capture()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = HBCCreateEnvironmentInst
//CHECK-NEXT:  %1 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  %2 = HBCResolveEnvironment %daa()
//CHECK-NEXT:  %3 = HBCLoadFromEnvironmentInst %2, [b@daa]
//CHECK-NEXT:  %4 = ReturnInst %3
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %5 = ReturnInst %1 : undefined
//CHECK-NEXT:function_end


//CHKOPT-LABEL:function daa_capture() : undefined|string|number
//CHKOPT-NEXT:frame = []
//CHKOPT-NEXT:%BB0:
//CHKOPT-NEXT:  %0 = HBCResolveEnvironment %daa()
//CHKOPT-NEXT:  %1 = HBCLoadFromEnvironmentInst %0, [b@daa] : undefined|string|number
//CHKOPT-NEXT:  %2 = ReturnInst %1
//CHKOPT-NEXT:function_end

