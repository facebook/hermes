/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

({10: 1, '11': 2, '10': 3});

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:  %3 = AllocObjectInst 2 : number, empty
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst null : null, %3 : object, "10" : string, true : boolean
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst 2 : number, %3 : object, "11" : string, true : boolean
// CHECK-NEXT:  %6 = StoreOwnPropertyInst 3 : number, %3 : object, "10" : string, true : boolean
// CHECK-NEXT:  %7 = StoreStackInst %3 : object, %1
// CHECK-NEXT:  %8 = LoadStackInst %1
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:function_end
