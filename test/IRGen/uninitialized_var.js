/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -hermes-parser -dump-ir %s -O

var x;
x;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "x": string
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %2 = StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %4 = StoreStackInst %3: any, %1: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %1: any
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end
