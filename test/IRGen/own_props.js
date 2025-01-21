/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

({10: 1, '11': 2, '10': 3});

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:       DefineNewOwnPropertyInst null: null, %3: object, "10": string, true: boolean
// CHECK-NEXT:       DefineNewOwnPropertyInst 2: number, %3: object, "11": string, true: boolean
// CHECK-NEXT:       DefineOwnPropertyInst 3: number, %3: object, "10": string, true: boolean
// CHECK-NEXT:       StoreStackInst %3: object, %1: any
// CHECK-NEXT:  %8 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end
