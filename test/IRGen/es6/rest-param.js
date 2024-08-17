/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function f1(a, ...b) {}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "f1": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %f1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "f1": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [a: any, b: any]

// CHECK:function f1(a: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.a]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.b]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [%VS1.a]: any
// CHECK-NEXT:  %6 = CallBuiltinInst (:any) [HermesBuiltin.copyRestArgs]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [%VS1.b]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
