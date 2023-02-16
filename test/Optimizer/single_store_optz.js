/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s     -O  | %FileCheckOrRegen %s --match-full-lines

function g12(z) {
    var w = function () { return 1; }
    w();  // site 1
    if (z > 0) {
        w(); // site 2
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "g12": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %g12(): undefined
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "g12": string
// CHECK-NEXT:  %3 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function g12(z: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %w(): number
// CHECK-NEXT:  %2 = CallInst (:number) %1: closure, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %3 = BinaryGreaterThanInst (:boolean) %0: any, 0: number
// CHECK-NEXT:  %4 = CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CallInst (:number) %1: closure, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %6 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function w(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst (:number) 1: number
// CHECK-NEXT:function_end
