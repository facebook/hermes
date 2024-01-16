/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function func() {
  var foo = [,,"a"];
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "func": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %func(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "func": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function func(): any
// CHECK-NEXT:frame = [foo: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [foo]: any
// CHECK-NEXT:  %1 = AllocArrayInst (:object) 3: number
// CHECK-NEXT:       StoreOwnPropertyInst "a": string, %1: object, 2: number, true: boolean
// CHECK-NEXT:       StoreFrameInst %1: object, [foo]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
