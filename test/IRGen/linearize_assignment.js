/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines

a.x = a = 42;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %1 = StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "a": string
// CHECK-NEXT:  %3 = StorePropertyLooseInst 42: number, globalObject: object, "a": string
// CHECK-NEXT:  %4 = StorePropertyLooseInst 42: number, %2: any, "x": string
// CHECK-NEXT:  %5 = StoreStackInst 42: number, %0: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %0: any
// CHECK-NEXT:  %7 = ReturnInst (:any) %6: any
// CHECK-NEXT:function_end
