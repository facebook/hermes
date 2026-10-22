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
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_3_iterValue: any
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_4_exc: any
// CHECK-NEXT:        TryStartInst %BB4, %BB6
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %19 = IteratorCloseInst (:any) %18: any, true: boolean
// CHECK-NEXT:  %20 = LoadStackInst (:any) %15: any
// CHECK-NEXT:        ThrowInst %20: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %22 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %22: any, %15: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %25 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %26 = IteratorNextInst (:any) %9: any, %25: any
// CHECK-NEXT:        StoreStackInst %26: any, %14: any
// CHECK-NEXT:        TryStartInst %BB12, %BB14
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %29 = AllocStackInst (:boolean) $?anon_5_isReturn: any
// CHECK-NEXT:        SaveAndYieldInst undefined: undefined, false: boolean, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %31 = ResumeGeneratorInst (:any) %29: boolean
// CHECK-NEXT:  %32 = LoadStackInst (:boolean) %29: boolean
// CHECK-NEXT:        CondBranchInst %32: boolean, %BB9, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryEndInst %BB4, %BB11
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        TryEndInst %BB4, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %36 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %37 = IteratorCloseInst (:any) %36: any, false: boolean
// CHECK-NEXT:        ReturnInst %31: any
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %40 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %40: any, %15: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %43 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %44 = IteratorCloseInst (:any) %43: any, false: boolean
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %46 = LoadStackInst (:any) %14: any
// CHECK-NEXT:        StorePropertyLooseInst %46: any, %31: any, "p": string
// CHECK-NEXT:        TryEndInst %BB12, %BB15
// CHECK-NEXT:%BB15:
// CHECK-NEXT:        BranchInst %BB13
// CHECK-NEXT:function_end
