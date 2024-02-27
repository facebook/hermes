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

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "bug1": string
// CHECK-NEXT:       DeclareGlobalVarInst "bug2": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %bug1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "bug1": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %bug2(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "bug2": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function bug1(): any [noReturn]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = PhiInst (:undefined|number) undefined: undefined, %BB0, %2: number, %BB1
// CHECK-NEXT:  %2 = UnaryIncInst (:number) %1: undefined|number
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function bug2(): any [noReturn]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:function_end
