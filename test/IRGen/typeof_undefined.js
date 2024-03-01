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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "x": string
// CHECK-NEXT:  %2 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %2: any
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %5 = UnaryTypeofInst (:string) %4: any
// CHECK-NEXT:       StorePropertyStrictInst %5: string, globalObject: object, "x": string
// CHECK-NEXT:  %7 = LoadStackInst (:any) %2: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end
