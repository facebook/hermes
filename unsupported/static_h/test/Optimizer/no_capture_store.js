/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s -O | %FileCheck %s

function bar() { }


//CHECK-LABEL:function main(p)
//CHECK-NEXT:frame = [k1, k2, k3]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = LoadPropertyInst globalObject : object, "bar" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined
//CHECK-NEXT:  %2 = StoreFrameInst %1, [k1]
//CHECK-NEXT:  %3 = LoadPropertyInst globalObject : object, "bar" : string
//CHECK-NEXT:  %4 = CallInst %3, undefined : undefined
//CHECK-NEXT:  %5 = StoreFrameInst %4, [k2]
//CHECK-NEXT:  %6 = LoadPropertyInst globalObject : object, "bar" : string
//CHECK-NEXT:  %7 = CallInst %6, undefined : undefined
//CHECK-NEXT:  %8 = StoreFrameInst %7, [k3]
//CHECK-NEXT:  %9 = CreateFunctionInst %""()
//CHECK-NEXT:  %10 = ReturnInst %9 : closure
//CHECK-NEXT:function_end

// The first stores to k1, k2, k3 can be eliminated since the variables haven't
// been captured yet.
function main(p) {
  var k1 = bar();
  var k2 = bar();
  var k3 = bar();

  return function () { return k1 + k2 + k3 }
}


//CHECK-LABEL:function test1() : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst 87 : number, [envVar@outer]
//CHECK-NEXT:  %1 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

//CHECK-LABEL:function test2(o) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst 42 : number, [envVar@outer]
//CHECK-NEXT:  %1 = CallInst %o, undefined : undefined
//CHECK-NEXT:  %2 = StoreFrameInst 87 : number, [envVar@outer]
//CHECK-NEXT:  %3 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

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
