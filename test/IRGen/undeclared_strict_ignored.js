/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir -strict -Wno-undefined-variable %s 2>&1 | %FileCheckOrRegen %s --match-full-lines

var x = y;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "y" : string
// CHECK-NEXT:  %4 = StorePropertyInst %3, globalObject : object, "x" : string
// CHECK-NEXT:  %5 = LoadStackInst %1
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end
