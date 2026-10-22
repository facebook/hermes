/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

var [a, [b = 1, c] = [,2]] = x;

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "a": string
// CHECK-NEXT:       DeclareGlobalVarInst "b": string
// CHECK-NEXT:       DeclareGlobalVarInst "c": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_1_iter: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_2_sourceOrNext: any
// CHECK-NEXT:       StoreStackInst %6: any, %8: any
// CHECK-NEXT:  %10 = IteratorBeginInst (:any) %8: any
// CHECK-NEXT:        StoreStackInst %10: any, %7: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_3_iterValue: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_4_exc: any
// CHECK-NEXT:        TryStartInst %BB2, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %15 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %16 = IteratorCloseInst (:any) %15: any, true: boolean
// CHECK-NEXT:  %17 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ThrowInst %17: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %19 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %19: any, %13: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %22 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %23 = IteratorNextInst (:any) %7: any, %22: any
// CHECK-NEXT:        StoreStackInst %23: any, %12: any
// CHECK-NEXT:        TryStartInst %BB6, %BB8
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryEndInst %BB2, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %28 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %28: any, %13: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %31 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %32 = IteratorNextInst (:any) %7: any, %31: any
// CHECK-NEXT:        StoreStackInst %32: any, %12: any
// CHECK-NEXT:  %34 = LoadStackInst (:any) %12: any
// CHECK-NEXT:  %35 = BinaryStrictlyNotEqualInst (:any) %34: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %35: any, %BB11, %BB10
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %37 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        StorePropertyLooseInst %37: any, globalObject: object, "a": string
// CHECK-NEXT:        TryEndInst %BB6, %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        TryStartInst %BB12, %BB14
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        TryStartInst %BB16, %BB18
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %43 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %43: any, %13: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        BranchInst %BB11
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %47 = AllocArrayInst (:object) 2: number
// CHECK-NEXT:        DefineOwnInDenseArrayInst 2: number, %47: object, 1: number
// CHECK-NEXT:        StoreStackInst %47: object, %12: any
// CHECK-NEXT:        TryEndInst %BB12, %BB15
// CHECK-NEXT:%BB15:
// CHECK-NEXT:        BranchInst %BB13
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %52 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %52: any, %13: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %55 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %56 = IteratorCloseInst (:any) %55: any, false: boolean
// CHECK-NEXT:  %57 = LoadStackInst (:any) %4: any
// CHECK-NEXT:        ReturnInst %57: any
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %59 = LoadStackInst (:any) %12: any
// CHECK-NEXT:  %60 = AllocStackInst (:any) $?anon_5_iter: any
// CHECK-NEXT:  %61 = AllocStackInst (:any) $?anon_6_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %59: any, %61: any
// CHECK-NEXT:  %63 = IteratorBeginInst (:any) %61: any
// CHECK-NEXT:        StoreStackInst %63: any, %60: any
// CHECK-NEXT:  %65 = AllocStackInst (:any) $?anon_7_iterValue: any
// CHECK-NEXT:  %66 = AllocStackInst (:any) $?anon_8_exc: any
// CHECK-NEXT:        TryStartInst %BB20, %BB22
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %68 = LoadStackInst (:any) %60: any
// CHECK-NEXT:  %69 = IteratorCloseInst (:any) %68: any, true: boolean
// CHECK-NEXT:  %70 = LoadStackInst (:any) %66: any
// CHECK-NEXT:        ThrowInst %70: any, %BB16
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %72 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %72: any, %66: any
// CHECK-NEXT:        BranchInst %BB19
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %75 = LoadStackInst (:any) %61: any
// CHECK-NEXT:  %76 = IteratorNextInst (:any) %60: any, %75: any
// CHECK-NEXT:        StoreStackInst %76: any, %65: any
// CHECK-NEXT:  %78 = LoadStackInst (:any) %65: any
// CHECK-NEXT:  %79 = BinaryStrictlyNotEqualInst (:any) %78: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %79: any, %BB25, %BB24
// CHECK-NEXT:%BB22:
// CHECK-NEXT:        TryEndInst %BB20, %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:        BranchInst %BB21
// CHECK-NEXT:%BB24:
// CHECK-NEXT:        TryStartInst %BB26, %BB28
// CHECK-NEXT:%BB25:
// CHECK-NEXT:        TryStartInst %BB30, %BB32
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %85 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %85: any, %66: any
// CHECK-NEXT:        BranchInst %BB19
// CHECK-NEXT:%BB27:
// CHECK-NEXT:        BranchInst %BB25
// CHECK-NEXT:%BB28:
// CHECK-NEXT:        StoreStackInst 1: number, %65: any
// CHECK-NEXT:        TryEndInst %BB26, %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:        BranchInst %BB27
// CHECK-NEXT:%BB30:
// CHECK-NEXT:  %92 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %92: any, %66: any
// CHECK-NEXT:        BranchInst %BB19
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %95 = LoadStackInst (:any) %61: any
// CHECK-NEXT:  %96 = IteratorNextInst (:any) %60: any, %95: any
// CHECK-NEXT:        StoreStackInst %96: any, %65: any
// CHECK-NEXT:        TryStartInst %BB34, %BB36
// CHECK-NEXT:%BB32:
// CHECK-NEXT:  %99 = LoadStackInst (:any) %65: any
// CHECK-NEXT:         StorePropertyLooseInst %99: any, globalObject: object, "b": string
// CHECK-NEXT:         TryEndInst %BB30, %BB33
// CHECK-NEXT:%BB33:
// CHECK-NEXT:         BranchInst %BB31
// CHECK-NEXT:%BB34:
// CHECK-NEXT:  %103 = CatchInst (:any)
// CHECK-NEXT:         StoreStackInst %103: any, %66: any
// CHECK-NEXT:         BranchInst %BB19
// CHECK-NEXT:%BB35:
// CHECK-NEXT:  %106 = LoadStackInst (:any) %60: any
// CHECK-NEXT:  %107 = IteratorCloseInst (:any) %106: any, false: boolean
// CHECK-NEXT:         TryEndInst %BB16, %BB38
// CHECK-NEXT:%BB36:
// CHECK-NEXT:  %109 = LoadStackInst (:any) %65: any
// CHECK-NEXT:         StorePropertyLooseInst %109: any, globalObject: object, "c": string
// CHECK-NEXT:         TryEndInst %BB34, %BB37
// CHECK-NEXT:%BB37:
// CHECK-NEXT:         BranchInst %BB35
// CHECK-NEXT:%BB38:
// CHECK-NEXT:         BranchInst %BB17
// CHECK-NEXT:function_end
