/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

({10: 1, '11': 2, '10': 3});

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst null: null, %2: object, "10": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 2: number, %2: object, "11": string, true: boolean
// CHECK-NEXT:       StoreOwnPropertyInst 3: number, %2: object, "10": string, true: boolean
// CHECK-NEXT:       StoreStackInst %2: object, %0: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %0: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end
