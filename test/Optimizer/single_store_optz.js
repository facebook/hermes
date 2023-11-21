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
// CHECK-NEXT:       DeclareGlobalVarInst "g12": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %g12(): undefined
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "g12": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function g12(z: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  %1 = BinaryGreaterThanInst (:boolean) %0: any, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
