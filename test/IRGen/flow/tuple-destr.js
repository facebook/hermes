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
let j: string;
[i, j] = inner;

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
// CHECK-NEXT:frame = [exports: any, inner: any, outer: any, x: any, y: any, a: any, b: any, i: any, j: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [inner]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [outer]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [y]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [a]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [b]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, undefined: undefined, [i]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, undefined: undefined, [j]: any
// CHECK-NEXT:  %12 = AllocObjectLiteralInst (:object) "0": string, 2: number, "1": string, "asdf": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %12: object, [inner]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [inner]: any
// CHECK-NEXT:  %15 = CheckedTypeCastInst (:object) %14: any, type(object)
// CHECK-NEXT:  %16 = AllocObjectLiteralInst (:object) "0": string, 1: number, "1": string, true: boolean, "2": string, %15: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %16: object, [outer]: any
// CHECK-NEXT:  %18 = LoadFrameInst (:any) %1: environment, [outer]: any
// CHECK-NEXT:  %19 = CheckedTypeCastInst (:object) %18: any, type(object)
// CHECK-NEXT:  %20 = PrLoadInst (:number) %19: object, 0: number, "0": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %20: number, [x]: any
// CHECK-NEXT:  %22 = PrLoadInst (:boolean) %19: object, 1: number, "1": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %22: boolean, [y]: any
// CHECK-NEXT:  %24 = PrLoadInst (:object) %19: object, 2: number, "2": string
// CHECK-NEXT:  %25 = PrLoadInst (:number) %24: object, 0: number, "0": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %25: number, [a]: any
// CHECK-NEXT:  %27 = PrLoadInst (:string) %24: object, 1: number, "1": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %27: string, [b]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, undefined: undefined, [i]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, undefined: undefined, [j]: any
// CHECK-NEXT:  %31 = LoadFrameInst (:any) %1: environment, [inner]: any
// CHECK-NEXT:  %32 = CheckedTypeCastInst (:object) %31: any, type(object)
// CHECK-NEXT:  %33 = PrLoadInst (:number) %32: object, 0: number, "0": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %33: number, [i]: any
// CHECK-NEXT:  %35 = PrLoadInst (:string) %32: object, 1: number, "1": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %35: string, [j]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
