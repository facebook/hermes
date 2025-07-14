/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir -O0 %s | %FileCheckOrRegen %s --match-full-lines
//
// Test for a regression where we were generating invalid IR straight out
// of IRGen in array destructuring when a yield was present.

function* g(arr) {
  [(yield).p] = arr;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "g": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %g(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "g": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function g(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS0: any, %"g 1#"(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [arr: any]

// CHECK:generator inner "g 1#"(arr: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %5 = CreateScopeInst (:environment) %VS1: any, %4: environment
// CHECK-NEXT:  %6 = LoadParamInst (:any) %arr: any
// CHECK-NEXT:       StoreFrameInst %5: environment, %6: any, [%VS1.arr]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %5: environment, [%VS1.arr]: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_1_iter: any
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_2_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %8: any, %10: any
// CHECK-NEXT:  %12 = IteratorBeginInst (:any) %10: any
// CHECK-NEXT:        StoreStackInst %12: any, %9: any
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_3_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %14: any
// CHECK-NEXT:  %16 = AllocStackInst (:any) $?anon_4_iterValue: any
// CHECK-NEXT:  %17 = AllocStackInst (:any) $?anon_5_exc: any
// CHECK-NEXT:        TryStartInst %BB4, %BB6
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %20 = LoadStackInst (:any) %14: any
// CHECK-NEXT:        CondBranchInst %20: any, %BB22, %BB21
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %22 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %22: any, %17: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreStackInst undefined: undefined, %16: any
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %27 = AllocStackInst (:boolean) $?anon_6_isReturn: any
// CHECK-NEXT:        SaveAndYieldInst undefined: undefined, false: boolean, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %29 = ResumeGeneratorInst (:any) %27: boolean
// CHECK-NEXT:  %30 = LoadStackInst (:boolean) %27: boolean
// CHECK-NEXT:        CondBranchInst %30: boolean, %BB9, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryEndInst %BB4, %BB11
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        TryEndInst %BB4, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        ReturnInst %29: any
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %36 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %37 = IteratorNextInst (:any) %9: any, %36: any
// CHECK-NEXT:  %38 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %39 = BinaryStrictlyEqualInst (:any) %38: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %39: any, %14: any
// CHECK-NEXT:        CondBranchInst %39: any, %BB14, %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        StoreStackInst %37: any, %16: any
// CHECK-NEXT:        BranchInst %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        TryStartInst %BB15, %BB17
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %45 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %45: any, %17: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %48 = LoadStackInst (:any) %14: any
// CHECK-NEXT:        CondBranchInst %48: any, %BB20, %BB19
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %50 = LoadStackInst (:any) %16: any
// CHECK-NEXT:        StorePropertyLooseInst %50: any, %29: any, "p": string
// CHECK-NEXT:        TryEndInst %BB15, %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:        BranchInst %BB16
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %54 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %55 = IteratorCloseInst (:any) %54: any, false: boolean
// CHECK-NEXT:        BranchInst %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %58 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %59 = IteratorCloseInst (:any) %58: any, true: boolean
// CHECK-NEXT:        BranchInst %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %61 = LoadStackInst (:any) %17: any
// CHECK-NEXT:        ThrowInst %61: any
// CHECK-NEXT:function_end
