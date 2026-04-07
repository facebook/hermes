/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -fno-std-globals -typed -dump-ir -Xdump-functions=main %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

return function main() {
  let inner: [number, string] = [2, 'asdf'];
  let outer: [number, bool, [number, string]] = [1, true, inner];
  let [x, y, [a, b]] = outer;
  let i: number;
  let j: string;
  [i, j] = inner;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 [exports: any, main: any]

// CHECK:scope %VS1 [inner: any, outer: any, x: any, y: any, a: any, b: any, i: any, j: any]

// CHECK:function main(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.inner]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.outer]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.x]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.y]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.a]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.b]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.j]: any
// CHECK-NEXT:  %10 = AllocObjectLiteralInst (:object) empty: any, "0": string, 2: number, "1": string, "asdf": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: object, [%VS1.inner]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS1.inner]: any
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:object) %12: any, type(object)
// CHECK-NEXT:  %14 = AllocObjectLiteralInst (:object) empty: any, "0": string, 1: number, "1": string, true: boolean, "2": string, %13: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %14: object, [%VS1.outer]: any
// CHECK-NEXT:  %16 = LoadFrameInst (:any) %1: environment, [%VS1.outer]: any
// CHECK-NEXT:  %17 = CheckedTypeCastInst (:object) %16: any, type(object)
// CHECK-NEXT:  %18 = PrLoadInst (:number) %17: object, 0: number, "0": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: number, [%VS1.x]: any
// CHECK-NEXT:  %20 = PrLoadInst (:boolean) %17: object, 1: number, "1": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %20: boolean, [%VS1.y]: any
// CHECK-NEXT:  %22 = PrLoadInst (:object) %17: object, 2: number, "2": string
// CHECK-NEXT:  %23 = PrLoadInst (:number) %22: object, 0: number, "0": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %23: number, [%VS1.a]: any
// CHECK-NEXT:  %25 = PrLoadInst (:string) %22: object, 1: number, "1": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %25: string, [%VS1.b]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, undefined: undefined, [%VS1.i]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, undefined: undefined, [%VS1.j]: any
// CHECK-NEXT:  %29 = LoadFrameInst (:any) %1: environment, [%VS1.inner]: any
// CHECK-NEXT:  %30 = CheckedTypeCastInst (:object) %29: any, type(object)
// CHECK-NEXT:  %31 = PrLoadInst (:number) %30: object, 0: number, "0": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %31: number, [%VS1.i]: any
// CHECK-NEXT:  %33 = PrLoadInst (:string) %30: object, 1: number, "1": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %33: string, [%VS1.j]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
