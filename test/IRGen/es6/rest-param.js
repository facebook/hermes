/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function f1(a, ...b) {}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "f1": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %f1(): any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "f1": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function f1(a: any): any
// CHECK-NEXT:frame = [a: any, b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:  %1 = StoreFrameInst undefined: undefined, [b]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [a]: any
// CHECK-NEXT:  %4 = CallBuiltinInst (:any) [HermesBuiltin.copyRestArgs]: number, empty: any, empty: any, undefined: undefined, 1: number
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [b]: any
// CHECK-NEXT:  %6 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
