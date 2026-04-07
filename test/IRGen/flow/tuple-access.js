/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -fno-std-globals -typed -dump-ir -Xdump-functions=main %s | %FileCheckOrRegen %s --match-full-lines

'use strict';

return function main() {
  let x: [number, bool] = [1, true]
  let y: number = x[0]
  let z: bool = x[1]
  x[0] = 2;
  x[1] = false;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 [exports: any, main: any]

// CHECK:scope %VS1 [x: any, y: any, z: any]

// CHECK:function main(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.x]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.y]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.z]: any
// CHECK-NEXT:  %5 = AllocObjectLiteralInst (:object) empty: any, "0": string, 1: number, "1": string, true: boolean
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: object, [%VS1.x]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [%VS1.x]: any
// CHECK-NEXT:  %8 = CheckedTypeCastInst (:object) %7: any, type(object)
// CHECK-NEXT:  %9 = PrLoadInst (:number) %8: object, 0: number, "0": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: number, [%VS1.y]: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [%VS1.x]: any
// CHECK-NEXT:  %12 = CheckedTypeCastInst (:object) %11: any, type(object)
// CHECK-NEXT:  %13 = PrLoadInst (:boolean) %12: object, 1: number, "1": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %13: boolean, [%VS1.z]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [%VS1.x]: any
// CHECK-NEXT:  %16 = CheckedTypeCastInst (:object) %15: any, type(object)
// CHECK-NEXT:        PrStoreInst 2: number, %16: object, 0: number, "0": string, true: boolean
// CHECK-NEXT:  %18 = LoadFrameInst (:any) %1: environment, [%VS1.x]: any
// CHECK-NEXT:  %19 = CheckedTypeCastInst (:object) %18: any, type(object)
// CHECK-NEXT:        PrStoreInst false: boolean, %19: object, 1: number, "1": string, true: boolean
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
