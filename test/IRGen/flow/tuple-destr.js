/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -fno-std-globals -typed -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

let inner: [number, string] = [2, 'asdf'];
let outer: [number, bool, [number, string]] = [1, true, inner];
let [x, y, [a, b]] = outer;
let i: number;
let j: bool;
[i, j] = inner;

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
// CHECK-NEXT:frame = [exports: any, inner: any, outer: any, x: any, y: any, a: any, b: any, i: any, j: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [inner]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [outer]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [y]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [b]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [j]: any
// CHECK-NEXT:  %10 = AllocObjectLiteralInst (:object) "0": string, 2: number, "1": string, "asdf": string
// CHECK-NEXT:        StoreFrameInst %10: object, [inner]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [inner]: any
// CHECK-NEXT:  %13 = AllocObjectLiteralInst (:object) "0": string, 1: number, "1": string, true: boolean, "2": string, %12: any
// CHECK-NEXT:        StoreFrameInst %13: object, [outer]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [outer]: any
// CHECK-NEXT:  %16 = PrLoadInst (:number) %15: any, 0: number, "0": string
// CHECK-NEXT:        StoreFrameInst %16: number, [x]: any
// CHECK-NEXT:  %18 = PrLoadInst (:boolean) %15: any, 1: number, "1": string
// CHECK-NEXT:        StoreFrameInst %18: boolean, [y]: any
// CHECK-NEXT:  %20 = PrLoadInst (:object) %15: any, 2: number, "2": string
// CHECK-NEXT:  %21 = PrLoadInst (:number) %20: object, 0: number, "0": string
// CHECK-NEXT:        StoreFrameInst %21: number, [a]: any
// CHECK-NEXT:  %23 = PrLoadInst (:string) %20: object, 1: number, "1": string
// CHECK-NEXT:        StoreFrameInst %23: string, [b]: any
// CHECK-NEXT:        StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:        StoreFrameInst undefined: undefined, [j]: any
// CHECK-NEXT:  %27 = LoadFrameInst (:any) [inner]: any
// CHECK-NEXT:  %28 = PrLoadInst (:number) %27: any, 0: number, "0": string
// CHECK-NEXT:        StoreFrameInst %28: number, [i]: any
// CHECK-NEXT:  %30 = PrLoadInst (:boolean) %27: any, 1: number, "1": string
// CHECK-NEXT:        StoreFrameInst %30: boolean, [j]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
