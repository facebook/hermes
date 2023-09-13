/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function test_assignment_expr() {
  var y = 0;
  var x = y = 4;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "test_assignment_expr": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %test_assignment_expr(): any
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "test_assignment_expr": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function test_assignment_expr(): any
// CHECK-NEXT:frame = [y: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [y]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [y]: any
// CHECK-NEXT:       StoreFrameInst 4: number, [y]: any
// CHECK-NEXT:       StoreFrameInst 4: number, [x]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
