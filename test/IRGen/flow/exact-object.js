/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -O0 -typed -dump-ir -Xno-dump-functions=global %s | %FileCheckOrRegen %s --match-full-lines

let t: {x: number} = {x: 3};
let tx: number = t.x;
t.x = 5;

let t2: {x: number, y: number} = {x: 3, x: 4, y: t.x};

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:scope %VS1 [exports: any, t: any, tx: any, t2: any]

// CHECK:function ""(exports: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.t]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.tx]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.t2]: any
// CHECK-NEXT:  %7 = AllocObjectLiteralInst (:object) empty: any, "x": string, 3: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: object, [%VS1.t]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [%VS1.t]: any
// CHECK-NEXT:  %10 = CheckedTypeCastInst (:object) %9: any, type(object)
// CHECK-NEXT:  %11 = PrLoadInst (:number) %10: object, 0: number, "x": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: number, [%VS1.tx]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [%VS1.t]: any
// CHECK-NEXT:  %14 = CheckedTypeCastInst (:object) %13: any, type(object)
// CHECK-NEXT:        PrStoreInst 5: number, %14: object, 0: number, "x": string, true: boolean
// CHECK-NEXT:        StorePropertyStrictInst 5: number, %14: object, "x": string
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [%VS1.t]: any
// CHECK-NEXT:  %18 = CheckedTypeCastInst (:object) %17: any, type(object)
// CHECK-NEXT:  %19 = PrLoadInst (:number) %18: object, 0: number, "x": string
// CHECK-NEXT:  %20 = AllocObjectLiteralInst (:object) empty: any, "x": string, 4: number, "y": string, 0: number
// CHECK-NEXT:        PrStoreInst %19: number, %20: object, 1: number, "y": string, true: boolean
// CHECK-NEXT:        StoreFrameInst %1: environment, %20: object, [%VS1.t2]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
