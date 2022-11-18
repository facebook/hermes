/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheckOrRegen %s --match-full-lines

function bug1() {
  var x;
  while (true) { ++x; continue; }
}

function bug2() {
  while (true) { continue; }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [bug1, bug2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %bug1()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "bug1" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %bug2()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "bug2" : string
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bug1()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = PhiInst undefined : undefined, %BB0, %2 : number, %BB1
// CHECK-NEXT:  %2 = UnaryOperatorInst '++', %1 : undefined|number
// CHECK-NEXT:  %3 = BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function bug2()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = BranchInst %BB1
// CHECK-NEXT:function_end
