/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s -O | %FileCheckOrRegen %s

function bar() { }

// The first stores to k1, k2, k3 can be eliminated since the variables haven't
// been captured yet.
function main(p) {
  var k1 = bar();
  var k2 = bar();
  var k3 = bar();

  return function () { return k1 + k2 + k3 }
}

// Make sure that we are not eliminating the "store-42" in test2, because the
// call to o() may clobber it.
function outer() {
    var envVar;

    function setValue(v) {
        envVar = v;
    }

    function test1() {
      envVar = 42;
      envVar = 87;
    }

    function test2(o) {
      envVar = 42;
      o();
      envVar = 87;
    }

    return [setValue, test1, test2]
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [bar, main, outer]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %bar#0#1()#2 : undefined, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "bar" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %main#0#1()#3 : closure, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "main" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %outer#0#1()#5 : object, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "outer" : string
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bar#0#1()#2 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{bar#0#1()#2}
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function main#0#1(p)#3 : closure
// CHECK-NEXT:frame = [k1#3, k2#3, k3#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{main#0#1()#3}
// CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "bar" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined
// CHECK-NEXT:  %3 = StoreFrameInst %2, [k1#3], %0
// CHECK-NEXT:  %4 = LoadPropertyInst globalObject : object, "bar" : string
// CHECK-NEXT:  %5 = CallInst %4, undefined : undefined
// CHECK-NEXT:  %6 = StoreFrameInst %5, [k2#3], %0
// CHECK-NEXT:  %7 = LoadPropertyInst globalObject : object, "bar" : string
// CHECK-NEXT:  %8 = CallInst %7, undefined : undefined
// CHECK-NEXT:  %9 = StoreFrameInst %8, [k3#3], %0
// CHECK-NEXT:  %10 = CreateFunctionInst %""#1#3()#4 : string|number|bigint, %0
// CHECK-NEXT:  %11 = ReturnInst %10 : closure
// CHECK-NEXT:function_end

// CHECK:function ""#1#3()#4 : string|number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{""#1#3()#4}
// CHECK-NEXT:  %1 = LoadFrameInst [k1#3@main], %0
// CHECK-NEXT:  %2 = LoadFrameInst [k2#3@main], %0
// CHECK-NEXT:  %3 = BinaryOperatorInst '+', %1, %2
// CHECK-NEXT:  %4 = LoadFrameInst [k3#3@main], %0
// CHECK-NEXT:  %5 = BinaryOperatorInst '+', %3 : string|number|bigint, %4
// CHECK-NEXT:  %6 = ReturnInst %5 : string|number|bigint
// CHECK-NEXT:function_end

// CHECK:function outer#0#1()#5 : object
// CHECK-NEXT:frame = [envVar#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{outer#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [envVar#5], %0
// CHECK-NEXT:  %2 = CreateFunctionInst %setValue#1#5()#6 : undefined, %0
// CHECK-NEXT:  %3 = CreateFunctionInst %test1#1#5()#7 : undefined, %0
// CHECK-NEXT:  %4 = CreateFunctionInst %test2#1#5()#8 : undefined, %0
// CHECK-NEXT:  %5 = AllocArrayInst 3 : number
// CHECK-NEXT:  %6 = StoreOwnPropertyInst %2 : closure, %5 : object, 0 : number, true : boolean
// CHECK-NEXT:  %7 = StoreOwnPropertyInst %3 : closure, %5 : object, 1 : number, true : boolean
// CHECK-NEXT:  %8 = StoreOwnPropertyInst %4 : closure, %5 : object, 2 : number, true : boolean
// CHECK-NEXT:  %9 = ReturnInst %5 : object
// CHECK-NEXT:function_end

// CHECK:function setValue#1#5(v)#6 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{setValue#1#5()#6}
// CHECK-NEXT:  %1 = StoreFrameInst %v, [envVar#5@outer], %0
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test1#1#5()#7 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test1#1#5()#7}
// CHECK-NEXT:  %1 = StoreFrameInst 87 : number, [envVar#5@outer], %0
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test2#1#5(o)#8 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test2#1#5()#8}
// CHECK-NEXT:  %1 = StoreFrameInst 42 : number, [envVar#5@outer], %0
// CHECK-NEXT:  %2 = CallInst %o, undefined : undefined
// CHECK-NEXT:  %3 = StoreFrameInst 87 : number, [envVar#5@outer], %0
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
