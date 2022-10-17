/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines

a.x = a = 42;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "a" : string
// CHECK-NEXT:  %4 = StorePropertyInst 42 : number, globalObject : object, "a" : string
// CHECK-NEXT:  %5 = StorePropertyInst 42 : number, %3, "x" : string
// CHECK-NEXT:  %6 = StoreStackInst 42 : number, %1
// CHECK-NEXT:  %7 = LoadStackInst %1
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:function_end
