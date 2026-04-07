/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -O0 -typed -dump-ir -Xdump-functions=main %s | %FileCheckOrRegen %s --match-full-lines

return function main() {
  let t: {x: number} = {x: 3};
  let tx: number = t.x;
  t.x = 5;

  let t2: {x: number, y: number} = {x: 3, x: 4, y: t.x};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 [exports: any, main: any]

// CHECK:scope %VS1 [t: any, tx: any, t2: any]

// CHECK:function main(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.t]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.tx]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.t2]: any
// CHECK-NEXT:  %5 = AllocObjectLiteralInst (:object) empty: any, "x": string, 3: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: object, [%VS1.t]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [%VS1.t]: any
// CHECK-NEXT:  %8 = CheckedTypeCastInst (:object) %7: any, type(object)
// CHECK-NEXT:  %9 = PrLoadInst (:number) %8: object, 0: number, "x": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: number, [%VS1.tx]: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [%VS1.t]: any
// CHECK-NEXT:  %12 = CheckedTypeCastInst (:object) %11: any, type(object)
// CHECK-NEXT:        PrStoreInst 5: number, %12: object, 0: number, "x": string, true: boolean
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [%VS1.t]: any
// CHECK-NEXT:  %15 = CheckedTypeCastInst (:object) %14: any, type(object)
// CHECK-NEXT:  %16 = PrLoadInst (:number) %15: object, 0: number, "x": string
// CHECK-NEXT:  %17 = AllocObjectLiteralInst (:object) empty: any, "x": string, 4: number, "y": string, 0: number
// CHECK-NEXT:        PrStoreInst %16: number, %17: object, 1: number, "y": string, true: boolean
// CHECK-NEXT:        StoreFrameInst %1: environment, %17: object, [%VS1.t2]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
