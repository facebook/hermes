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
// CHECK-NEXT:  %2 = AllocStackInst (:number) $generator_state: any
// CHECK-NEXT:  %3 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %4 = LoadFrameInst (:number) %3: environment, [%VS0.generator_state]: number
// CHECK-NEXT:       StoreStackInst %4: number, %2: number
// CHECK-NEXT:  %6 = FEqualInst (:boolean) %4: number, 2: number
// CHECK-NEXT:       CondBranchInst %6: boolean, %BB13, %BB14
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = FEqualInst (:boolean) %1: number, 1: number
// CHECK-NEXT:       CondBranchInst %8: boolean, %BB7, %BB8
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        StoreFrameInst %3: environment, 1: number, [%VS0.idx]: number
// CHECK-NEXT:        StoreStackInst 1: number, %2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, 1: number, [%VS0.generator_state]: number
// CHECK-NEXT:        TryEndInst %BB20, %BB21
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        StoreStackInst 3: number, %2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, 3: number, [%VS0.generator_state]: number
// CHECK-NEXT:        TryEndInst %BB20, %BB22
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %17 = FEqualInst (:boolean) %1: number, 1: number
// CHECK-NEXT:        CondBranchInst %17: boolean, %BB9, %BB10
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreStackInst 3: number, %2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, 3: number, [%VS0.generator_state]: number
// CHECK-NEXT:        TryEndInst %BB20, %BB23
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        StoreStackInst 3: number, %2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, 3: number, [%VS0.generator_state]: number
// CHECK-NEXT:        TryEndInst %BB20, %BB24
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        StoreStackInst 3: number, %2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, 3: number, [%VS0.generator_state]: number
// CHECK-NEXT:        ThrowInst %0: any, %BB20
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %28 = FEqualInst (:boolean) %1: number, 2: number
// CHECK-NEXT:        CondBranchInst %28: boolean, %BB3, %BB2
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        StoreStackInst 3: number, %2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, 3: number, [%VS0.generator_state]: number
// CHECK-NEXT:        ThrowInst %0: any, %BB20
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %33 = FEqualInst (:boolean) %1: number, 2: number
// CHECK-NEXT:        CondBranchInst %33: boolean, %BB6, %BB5
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        TryStartInst %BB20, %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        StoreStackInst 2: number, %2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, 2: number, [%VS0.generator_state]: number
// CHECK-NEXT:  %38 = LoadFrameInst (:number) %3: environment, [%VS0.idx]: number
// CHECK-NEXT:        SwitchInst %38: number, %BB4, 0: number, %BB1
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        StoreStackInst 3: number, %2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, 3: number, [%VS0.generator_state]: number
// CHECK-NEXT:        ThrowTypeErrorInst "Generator functions may not be called on executing generators": string
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %43 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %44 = FEqualInst (:boolean) %43: number, 3: number
// CHECK-NEXT:        CondBranchInst %44: boolean, %BB15, %BB11
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %46 = FEqualInst (:boolean) %1: number, 1: number
// CHECK-NEXT:        CondBranchInst %46: boolean, %BB16, %BB17
// CHECK-NEXT:%BB16:
// CHECK-NEXT:        ThrowInst %0: any
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %49 = FEqualInst (:boolean) %1: number, 2: number
// CHECK-NEXT:        CondBranchInst %49: boolean, %BB18, %BB19
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %51 = AllocObjectLiteralInst (:object) empty: any, "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %51: object
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %53 = AllocObjectLiteralInst (:object) empty: any, "value": string, undefined: undefined, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %53: object
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %55 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst 3: number, %2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, 3: number, [%VS0.generator_state]: number
// CHECK-NEXT:        ThrowInst %55: any
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %59 = AllocObjectLiteralInst (:object) empty: any, "value": string, 42: number, "done": string, false: boolean
// CHECK-NEXT:        ReturnInst %59: object
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %61 = AllocObjectLiteralInst (:object) empty: any, "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %61: object
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %63 = AllocObjectLiteralInst (:object) empty: any, "value": string, undefined: undefined, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %63: object
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %65 = AllocObjectLiteralInst (:object) empty: any, "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %65: object
// CHECK-NEXT:function_end
