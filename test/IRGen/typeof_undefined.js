/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -hermes-parser -dump-ir -strict %s 2>&1 | %FileCheckOrRegen %s --match-full-lines

var x = typeof foo;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "x": string
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %2 = StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %4 = UnaryTypeofInst (:any) %3: any
// CHECK-NEXT:  %5 = StorePropertyStrictInst %4: any, globalObject: object, "x": string
// CHECK-NEXT:  %6 = LoadStackInst (:any) %1: any
// CHECK-NEXT:  %7 = ReturnInst %6: any
// CHECK-NEXT:function_end
