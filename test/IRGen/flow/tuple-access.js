/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -fno-std-globals -typed -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

let x: [number, bool] = [1, true]
let y: number = x[0]
let z: bool = x[1]
x[0] = 2;
x[1] = false;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %3: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: object
// CHECK-NEXT:       StoreStackInst %5: any, %1: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:function ""(exports: any): any
// CHECK-NEXT:frame = [exports: any, x: any, y: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [y]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [z]: any
// CHECK-NEXT:  %7 = AllocObjectLiteralInst (:object) "0": string, 1: number, "1": string, true: boolean
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: object, [x]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %10 = CheckedTypeCastInst (:object) %9: any, type(object)
// CHECK-NEXT:  %11 = PrLoadInst (:number) %10: object, 0: number, "0": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: number, [y]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %14 = CheckedTypeCastInst (:object) %13: any, type(object)
// CHECK-NEXT:  %15 = PrLoadInst (:boolean) %14: object, 1: number, "1": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: boolean, [z]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %18 = CheckedTypeCastInst (:object) %17: any, type(object)
// CHECK-NEXT:        PrStoreInst 2: number, %18: object, 0: number, "0": string, true: boolean
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %21 = CheckedTypeCastInst (:object) %20: any, type(object)
// CHECK-NEXT:        PrStoreInst false: boolean, %21: object, 1: number, "1": string, true: boolean
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
