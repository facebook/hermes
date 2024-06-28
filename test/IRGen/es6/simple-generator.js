/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

function* simple() {
  yield 42;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "simple": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %simple(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "simple": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [generator_state: number, idx: number]

// CHECK:function simple(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS1.idx]: number
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS1.generator_state]: number
// CHECK-NEXT:  %4 = CreateGeneratorInst (:object) %1: environment, %simple?inner(): functionCode
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:generator inner simple?inner(action: number, value: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %value: any
// CHECK-NEXT:  %1 = LoadParamInst (:number) %action: number
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:number) %2: environment, [%VS1.generator_state]: number
// CHECK-NEXT:       CmpBrStrictlyEqualInst %3: number, 2: number, %BB12, %BB13
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       CmpBrStrictlyEqualInst %1: number, 1: number, %BB7, %BB8
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       StoreFrameInst %2: environment, 1: number, [%VS1.idx]: number
// CHECK-NEXT:       StoreFrameInst %2: environment, 1: number, [%VS1.generator_state]: number
// CHECK-NEXT:  %8 = AllocObjectLiteralInst (:object) empty: any, "value": string, 42: number, "done": string, false: boolean
// CHECK-NEXT:       ReturnInst %8: object
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:  %11 = AllocObjectLiteralInst (:object) empty: any, "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %11: object
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        CmpBrStrictlyEqualInst %1: number, 1: number, %BB9, %BB10
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:  %15 = AllocObjectLiteralInst (:object) empty: any, "value": string, undefined: undefined, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %15: object
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:  %18 = AllocObjectLiteralInst (:object) empty: any, "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %18: object
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:        ThrowInst %0: any
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %22 = FEqualInst (:boolean) %1: number, 2: number
// CHECK-NEXT:        CondBranchInst %22: boolean, %BB3, %BB2
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:        ThrowInst %0: any
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %26 = FEqualInst (:boolean) %1: number, 2: number
// CHECK-NEXT:        CondBranchInst %26: boolean, %BB6, %BB5
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        StoreFrameInst %2: environment, 2: number, [%VS1.generator_state]: number
// CHECK-NEXT:  %29 = LoadFrameInst (:number) %2: environment, [%VS1.idx]: number
// CHECK-NEXT:        SwitchInst %29: number, %BB4, 0: number, %BB1
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:        ThrowTypeErrorInst "Generator functions may not be called on executing generators": string
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        CmpBrStrictlyEqualInst %3: number, 3: number, %BB14, %BB11
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        CmpBrStrictlyEqualInst %1: number, 1: number, %BB15, %BB16
// CHECK-NEXT:%BB15:
// CHECK-NEXT:        ThrowInst %0: any
// CHECK-NEXT:%BB16:
// CHECK-NEXT:        CmpBrStrictlyEqualInst %1: number, 2: number, %BB17, %BB18
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %37 = AllocObjectLiteralInst (:object) empty: any, "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %37: object
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %39 = AllocObjectLiteralInst (:object) empty: any, "value": string, undefined: undefined, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %39: object
// CHECK-NEXT:function_end
