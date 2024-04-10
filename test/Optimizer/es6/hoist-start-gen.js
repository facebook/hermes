/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function *foo() {
  var count = 0;
  try {
    yield count;
  } finally {
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [generator_state: number, idx: number, catchVal: any, exception_handler_idx: number]

// CHECK:function foo(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS1.idx]: number
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS1.generator_state]: number
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS1.exception_handler_idx]: number
// CHECK-NEXT:  %5 = CreateGeneratorInst (:object) %1: environment, %?anon_0_foo(): functionCode
// CHECK-NEXT:       ReturnInst %5: object
// CHECK-NEXT:function_end

// CHECK:generator inner ?anon_0_foo(action: number, value: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %value: any
// CHECK-NEXT:  %1 = LoadParamInst (:number) %action: number
// CHECK-NEXT:  %2 = AllocStackInst (:number) $generator_state: any
// CHECK-NEXT:  %3 = AllocStackInst (:any) $catchVal: any
// CHECK-NEXT:  %4 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %5 = LoadFrameInst (:number) %4: environment, [%VS1.generator_state]: number
// CHECK-NEXT:       StoreStackInst %5: number, %2: number
// CHECK-NEXT:       CmpBrStrictlyEqualInst %5: number, 2: number, %BB14, %BB15
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       CmpBrStrictlyEqualInst %1: number, 1: number, %BB8, %BB9
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       StoreFrameInst %4: environment, 1: number, [%VS1.exception_handler_idx]: number
// CHECK-NEXT:        StoreFrameInst %4: environment, 2: number, [%VS1.idx]: number
// CHECK-NEXT:        StoreStackInst 1: number, %2: number
// CHECK-NEXT:        StoreFrameInst %4: environment, 1: number, [%VS1.generator_state]: number
// CHECK-NEXT:        BranchInst %BB26
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        StoreStackInst 3: number, %2: number
// CHECK-NEXT:        StoreFrameInst %4: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:        BranchInst %BB24
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %4: environment, [%VS1.catchVal]: any
// CHECK-NEXT:        StoreStackInst %17: any, %3: any
// CHECK-NEXT:        StoreFrameInst %4: environment, 0: number, [%VS1.exception_handler_idx]: number
// CHECK-NEXT:        ThrowInst %17: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        CmpBrStrictlyEqualInst %1: number, 1: number, %BB10, %BB11
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        StoreFrameInst %4: environment, 0: number, [%VS1.exception_handler_idx]: number
// CHECK-NEXT:        StoreStackInst 3: number, %2: number
// CHECK-NEXT:        StoreFrameInst %4: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:        BranchInst %BB27
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        StoreFrameInst %4: environment, 0: number, [%VS1.exception_handler_idx]: number
// CHECK-NEXT:        StoreStackInst 3: number, %2: number
// CHECK-NEXT:        StoreFrameInst %4: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:        BranchInst %BB25
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        StoreStackInst 3: number, %2: number
// CHECK-NEXT:        StoreFrameInst %4: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:        ThrowInst %0: any
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %33 = FEqualInst (:boolean) %1: number, 2: number
// CHECK-NEXT:        CondBranchInst %33: boolean, %BB3, %BB2
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        StoreStackInst 3: number, %2: number
// CHECK-NEXT:        StoreFrameInst %4: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:        ThrowInst %0: any
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %38 = FEqualInst (:boolean) %1: number, 2: number
// CHECK-NEXT:        CondBranchInst %38: boolean, %BB6, %BB7
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        TryStartInst %BB22, %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        StoreStackInst 2: number, %2: number
// CHECK-NEXT:        StoreFrameInst %4: environment, 2: number, [%VS1.generator_state]: number
// CHECK-NEXT:  %43 = LoadFrameInst (:number) %4: environment, [%VS1.idx]: number
// CHECK-NEXT:        SwitchInst %43: number, %BB28, 0: number, %BB1, 1: number, %BB4, 2: number, %BB5
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        StoreStackInst 3: number, %2: number
// CHECK-NEXT:        StoreFrameInst %4: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:        ThrowTypeErrorInst "Generator functions may not be called on executing generators": string
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %48 = LoadStackInst (:number) %2: number
// CHECK-NEXT:        CmpBrStrictlyEqualInst %48: number, 3: number, %BB16, %BB12
// CHECK-NEXT:%BB16:
// CHECK-NEXT:        CmpBrStrictlyEqualInst %1: number, 1: number, %BB17, %BB18
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        ThrowInst %0: any
// CHECK-NEXT:%BB18:
// CHECK-NEXT:        CmpBrStrictlyEqualInst %1: number, 2: number, %BB19, %BB20
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %53 = AllocObjectLiteralInst (:object) "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %53: object
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %55 = AllocObjectLiteralInst (:object) "value": string, undefined: undefined, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %55: object
// CHECK-NEXT:%BB21:
// CHECK-NEXT:        StoreStackInst 3: number, %2: number
// CHECK-NEXT:        StoreFrameInst %4: environment, 3: number, [%VS1.generator_state]: number
// CHECK-NEXT:  %59 = LoadStackInst (:any) %3: any
// CHECK-NEXT:        ThrowInst %59: any
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %61 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %61: any, %3: any
// CHECK-NEXT:        StoreFrameInst %4: environment, %61: any, [%VS1.catchVal]: any
// CHECK-NEXT:  %64 = LoadFrameInst (:number) %4: environment, [%VS1.exception_handler_idx]: number
// CHECK-NEXT:        SwitchInst %64: number, %BB29, 0: number, %BB21, 1: number, %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:        StoreFrameInst %4: environment, 1: number, [%VS1.idx]: number
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB24:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %69 = AllocObjectLiteralInst (:object) "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %69: object
// CHECK-NEXT:%BB25:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %72 = AllocObjectLiteralInst (:object) "value": string, undefined: undefined, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %72: object
// CHECK-NEXT:%BB26:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %75 = AllocObjectLiteralInst (:object) "value": string, 0: number, "done": string, false: boolean
// CHECK-NEXT:        ReturnInst %75: object
// CHECK-NEXT:%BB27:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %78 = AllocObjectLiteralInst (:object) "value": string, %0: any, "done": string, true: boolean
// CHECK-NEXT:        ReturnInst %78: object
// CHECK-NEXT:%BB28:
// CHECK-NEXT:        UnreachableInst
// CHECK-NEXT:%BB29:
// CHECK-NEXT:        UnreachableInst
// CHECK-NEXT:function_end
