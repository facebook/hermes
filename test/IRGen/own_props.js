/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

({10: 1, '11': 2, '10': 3});

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %1 = StoreStackInst undefined : undefined, %0
// CHECK-NEXT:  %2 = AllocObjectInst 2 : number, empty
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst null : null, %2 : object, "10" : string, true : boolean
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst 2 : number, %2 : object, "11" : string, true : boolean
// CHECK-NEXT:  %5 = StoreOwnPropertyInst 3 : number, %2 : object, "10" : string, true : boolean
// CHECK-NEXT:  %6 = StoreStackInst %2 : object, %0
// CHECK-NEXT:  %7 = LoadStackInst %0
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:function_end
