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

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "simple": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %simple(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "simple": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS0 [generator_state: number, idx: number]

// CHECK:function simple(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       StoreFrameInst %0: environment, 0: number, [%VS0.idx]: number
// CHECK-NEXT:       StoreFrameInst %0: environment, 0: number, [%VS0.generator_state]: number
// CHECK-NEXT:  %3 = CreateGeneratorInst (:object) %0: environment, %VS0: any, %"simple 1#"(): functionCode
// CHECK-NEXT:       ReturnInst %3: object
// CHECK-NEXT:function_end

// CHECK:generator inner "simple 1#"(action: number, value: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %value: any
// CHECK-NEXT:  %1 = LoadParamInst (:number) %action: number
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:number) %2: environment, [%VS0.generator_state]: number
// CHECK-NEXT:  %4 = FEqualInst (:boolean) %3: number, 2: number
// CHECK-NEXT:       CondBranchInst %4: boolean, %BB12, %BB13
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = FEqualInst (:boolean) %1: number, 1: number
// CHECK-NEXT:       CondBranchInst %6: boolean, %BB7, %BB8
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       StoreFrameInst %2: environment, 1: number, [%VS0.idx]: number
// CHECK-NEXT:       StoreFrameInst %2: environment, 1: number, [%VS0.generator_state]: number
// CHECK-NEXT:  %10 = AllocObjectLiteralInst (:object) empty: any, "value": string, 42: number, "done": string, false: boolean
// CHECK-NEXT:        ReturnInst %10: object
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS0.generator_state]: number
// CHECK-NEXT:  %13 = AllocObjectLiteralInst (:object) empty: any, "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %13: object
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = FEqualInst (:boolean) %1: number, 1: number
// CHECK-NEXT:        CondBranchInst %15: boolean, %BB9, %BB10
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS0.generator_state]: number
// CHECK-NEXT:  %18 = AllocObjectLiteralInst (:object) empty: any, "value": string, undefined: undefined, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %18: object
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS0.generator_state]: number
// CHECK-NEXT:  %21 = AllocObjectLiteralInst (:object) empty: any, "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %21: object
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS0.generator_state]: number
// CHECK-NEXT:        ThrowInst %0: any
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %25 = FEqualInst (:boolean) %1: number, 2: number
// CHECK-NEXT:        CondBranchInst %25: boolean, %BB3, %BB2
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS0.generator_state]: number
// CHECK-NEXT:        ThrowInst %0: any
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %29 = FEqualInst (:boolean) %1: number, 2: number
// CHECK-NEXT:        CondBranchInst %29: boolean, %BB6, %BB5
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        StoreFrameInst %2: environment, 2: number, [%VS0.generator_state]: number
// CHECK-NEXT:  %32 = LoadFrameInst (:number) %2: environment, [%VS0.idx]: number
// CHECK-NEXT:        SwitchInst %32: number, %BB4, 0: number, %BB1
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        StoreFrameInst %2: environment, 3: number, [%VS0.generator_state]: number
// CHECK-NEXT:        ThrowTypeErrorInst "Generator functions may not be called on executing generators": string
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %36 = FEqualInst (:boolean) %3: number, 3: number
// CHECK-NEXT:        CondBranchInst %36: boolean, %BB14, %BB11
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %38 = FEqualInst (:boolean) %1: number, 1: number
// CHECK-NEXT:        CondBranchInst %38: boolean, %BB15, %BB16
// CHECK-NEXT:%BB15:
// CHECK-NEXT:        ThrowInst %0: any
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %41 = FEqualInst (:boolean) %1: number, 2: number
// CHECK-NEXT:        CondBranchInst %41: boolean, %BB17, %BB18
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %43 = AllocObjectLiteralInst (:object) empty: any, "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %43: object
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %45 = AllocObjectLiteralInst (:object) empty: any, "value": string, undefined: undefined, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %45: object
// CHECK-NEXT:function_end
