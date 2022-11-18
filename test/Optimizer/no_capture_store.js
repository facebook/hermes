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

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [bar, main, outer]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %bar() : undefined
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "bar" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %main() : closure
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "main" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %outer() : object
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "outer" : string
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bar() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function main(p) : closure
// CHECK-NEXT:frame = [k1, k2, k3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadPropertyInst globalObject : object, "bar" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined
// CHECK-NEXT:  %2 = StoreFrameInst %1, [k1]
// CHECK-NEXT:  %3 = LoadPropertyInst globalObject : object, "bar" : string
// CHECK-NEXT:  %4 = CallInst %3, undefined : undefined
// CHECK-NEXT:  %5 = StoreFrameInst %4, [k2]
// CHECK-NEXT:  %6 = LoadPropertyInst globalObject : object, "bar" : string
// CHECK-NEXT:  %7 = CallInst %6, undefined : undefined
// CHECK-NEXT:  %8 = StoreFrameInst %7, [k3]
// CHECK-NEXT:  %9 = CreateFunctionInst %""() : string|number|bigint
// CHECK-NEXT:  %10 = ReturnInst %9 : closure
// CHECK-NEXT:function_end

// CHECK:function ""() : string|number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [k1@main]
// CHECK-NEXT:  %1 = LoadFrameInst [k2@main]
// CHECK-NEXT:  %2 = BinaryOperatorInst '+', %0, %1
// CHECK-NEXT:  %3 = LoadFrameInst [k3@main]
// CHECK-NEXT:  %4 = BinaryOperatorInst '+', %2 : string|number|bigint, %3
// CHECK-NEXT:  %5 = ReturnInst %4 : string|number|bigint
// CHECK-NEXT:function_end

// CHECK:function outer() : object
// CHECK-NEXT:frame = [envVar]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [envVar]
// CHECK-NEXT:  %1 = CreateFunctionInst %setValue() : undefined
// CHECK-NEXT:  %2 = CreateFunctionInst %test1() : undefined
// CHECK-NEXT:  %3 = CreateFunctionInst %test2() : undefined
// CHECK-NEXT:  %4 = AllocArrayInst 3 : number
// CHECK-NEXT:  %5 = StoreOwnPropertyInst %1 : closure, %4 : object, 0 : number, true : boolean
// CHECK-NEXT:  %6 = StoreOwnPropertyInst %2 : closure, %4 : object, 1 : number, true : boolean
// CHECK-NEXT:  %7 = StoreOwnPropertyInst %3 : closure, %4 : object, 2 : number, true : boolean
// CHECK-NEXT:  %8 = ReturnInst %4 : object
// CHECK-NEXT:function_end

// CHECK:function setValue(v) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %v, [envVar@outer]
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test1() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst 87 : number, [envVar@outer]
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test2(o) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst 42 : number, [envVar@outer]
// CHECK-NEXT:  %1 = CallInst %o, undefined : undefined
// CHECK-NEXT:  %2 = StoreFrameInst 87 : number, [envVar@outer]
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
