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
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %""(): any
// CHECK-NEXT:  %3 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %4 = CallInst [njsf] (:any) %2: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: object
// CHECK-NEXT:       StoreStackInst %4: any, %0: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %0: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function ""(exports: any): any
// CHECK-NEXT:frame = [exports: any, x: any, y: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [y]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [z]: any
// CHECK-NEXT:  %5 = AllocObjectLiteralInst (:object) "0": string, 1: number, "1": string, true: boolean
// CHECK-NEXT:       StoreFrameInst %5: object, [x]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %8 = PrLoadInst (:number) %7: any, 0: number, "0": string
// CHECK-NEXT:       StoreFrameInst %8: number, [y]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %11 = PrLoadInst (:boolean) %10: any, 1: number, "1": string
// CHECK-NEXT:        StoreFrameInst %11: boolean, [z]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:        PrStoreInst 2: number, %13: any, 0: number, "0": string, true: boolean
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:        PrStoreInst false: boolean, %15: any, 1: number, "1": string, true: boolean
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
