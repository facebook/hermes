/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s     -O | %FileCheckOrRegen %s --match-full-lines

function bug1() {
  var x;
  while (true) { ++x; continue; }
}

function bug2() {
  while (true) { continue; }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "bug1": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "bug2": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:closure) %bug1(): any
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2: closure, globalObject: object, "bug1": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %bug2(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: closure, globalObject: object, "bug2": string
// CHECK-NEXT:  %6 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function bug1(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = PhiInst (:undefined|number) undefined: undefined, %BB0, %2: number, %BB1
// CHECK-NEXT:  %2 = UnaryIncInst (:number) %1: undefined|number
// CHECK-NEXT:  %3 = BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function bug2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = BranchInst %BB1
// CHECK-NEXT:function_end
