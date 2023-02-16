/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir -strict %s 2>&1 | %FileCheckOrRegen %s --match-full-lines

var x = y;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}undeclared_strict.js:10:9: warning: the variable "y" was not declared in function "global"
// CHECK-NEXT:var x = y;
// CHECK-NEXT:        ^
// CHECK-NEXT:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "x": string
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %2 = StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "y": string
// CHECK-NEXT:  %4 = StorePropertyStrictInst %3: any, globalObject: object, "x": string
// CHECK-NEXT:  %5 = LoadStackInst (:any) %1: any
// CHECK-NEXT:  %6 = ReturnInst (:any) %5: any
// CHECK-NEXT:function_end
