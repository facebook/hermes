/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

// Make sure we can remove all trampolines from our code.

function sink() {}

function recursive_phi(x) {
  var k = 1;
  var j = "hi";
  var t;

  for (var i = 0; i < 10; i++) {
    if (x > 3) {
      t = k;
      k = j;
      j = t;
    }
  }

  return k + j;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [sink, recursive_phi]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %sink() : undefined
// CHECK-NEXT:  %1 = StorePropertyInst %0 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %recursive_phi() : string|number
// CHECK-NEXT:  %3 = StorePropertyInst %2 : closure, globalObject : object, "recursive_phi" : string
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function sink() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function recursive_phi(x) : string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = PhiInst 1 : number, %BB0, %8 : string|number, %BB2
// CHECK-NEXT:  %2 = PhiInst "hi" : string, %BB0, %9 : string|number, %BB2
// CHECK-NEXT:  %3 = PhiInst 0 : number, %BB0, %10 : number, %BB2
// CHECK-NEXT:  %4 = BinaryOperatorInst '>', %x, 3 : number
// CHECK-NEXT:  %5 = CondBranchInst %4 : boolean, %BB3, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %8 : string|number, %9 : string|number
// CHECK-NEXT:  %7 = ReturnInst %6 : string|number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = PhiInst %2 : string|number, %BB3, %1 : string|number, %BB1
// CHECK-NEXT:  %9 = PhiInst %1 : string|number, %BB3, %2 : string|number, %BB1
// CHECK-NEXT:  %10 = UnaryOperatorInst '++', %3 : number
// CHECK-NEXT:  %11 = BinaryOperatorInst '<', %10 : number, 10 : number
// CHECK-NEXT:  %12 = CondBranchInst %11 : boolean, %BB1, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = BranchInst %BB2
// CHECK-NEXT:function_end
