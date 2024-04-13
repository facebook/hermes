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
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_3_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_4_iterValue: any
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_5_exc: any
// CHECK-NEXT:        TryStartInst %BB2, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %17 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        CondBranchInst %17: any, %BB50, %BB49
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %19 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %19: any, %15: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        StoreStackInst undefined: undefined, %14: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryEndInst %BB2, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %26 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %27 = IteratorNextInst (:any) %7: any, %26: any
// CHECK-NEXT:  %28 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %29 = BinaryStrictlyEqualInst (:any) %28: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %29: any, %12: any
// CHECK-NEXT:        CondBranchInst %29: any, %BB8, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        StoreStackInst %27: any, %14: any
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryStartInst %BB9, %BB11
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %35 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %35: any, %15: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        StoreStackInst undefined: undefined, %14: any
// CHECK-NEXT:  %39 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        CondBranchInst %39: any, %BB15, %BB13
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %41 = LoadStackInst (:any) %14: any
// CHECK-NEXT:        StorePropertyLooseInst %41: any, globalObject: object, "a": string
// CHECK-NEXT:        TryEndInst %BB9, %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %45 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %46 = IteratorNextInst (:any) %7: any, %45: any
// CHECK-NEXT:  %47 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %48 = BinaryStrictlyEqualInst (:any) %47: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %48: any, %12: any
// CHECK-NEXT:        CondBranchInst %48: any, %BB16, %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        StoreStackInst %46: any, %14: any
// CHECK-NEXT:        BranchInst %BB15
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %53 = LoadStackInst (:any) %14: any
// CHECK-NEXT:  %54 = BinaryStrictlyNotEqualInst (:any) %53: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %54: any, %BB17, %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %56 = AllocArrayInst (:object) 2: number
// CHECK-NEXT:        StoreOwnPropertyInst 2: number, %56: object, 1: number, true: boolean
// CHECK-NEXT:        StoreStackInst %56: object, %14: any
// CHECK-NEXT:        BranchInst %BB17
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        TryStartInst %BB18, %BB20
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %61 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %61: any, %15: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %64 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        CondBranchInst %64: any, %BB48, %BB47
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %66 = LoadStackInst (:any) %14: any
// CHECK-NEXT:  %67 = AllocStackInst (:any) $?anon_6_iter: any
// CHECK-NEXT:  %68 = AllocStackInst (:any) $?anon_7_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %66: any, %68: any
// CHECK-NEXT:  %70 = IteratorBeginInst (:any) %68: any
// CHECK-NEXT:        StoreStackInst %70: any, %67: any
// CHECK-NEXT:  %72 = AllocStackInst (:any) $?anon_8_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %72: any
// CHECK-NEXT:  %74 = AllocStackInst (:any) $?anon_9_iterValue: any
// CHECK-NEXT:  %75 = AllocStackInst (:any) $?anon_10_exc: any
// CHECK-NEXT:        TryStartInst %BB22, %BB24
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %77 = LoadStackInst (:any) %72: any
// CHECK-NEXT:        CondBranchInst %77: any, %BB45, %BB44
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %79 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %79: any, %75: any
// CHECK-NEXT:        BranchInst %BB21
// CHECK-NEXT:%BB23:
// CHECK-NEXT:        StoreStackInst undefined: undefined, %74: any
// CHECK-NEXT:        BranchInst %BB26
// CHECK-NEXT:%BB24:
// CHECK-NEXT:        TryEndInst %BB22, %BB25
// CHECK-NEXT:%BB25:
// CHECK-NEXT:        BranchInst %BB23
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %86 = LoadStackInst (:any) %68: any
// CHECK-NEXT:  %87 = IteratorNextInst (:any) %67: any, %86: any
// CHECK-NEXT:  %88 = LoadStackInst (:any) %67: any
// CHECK-NEXT:  %89 = BinaryStrictlyEqualInst (:any) %88: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %89: any, %72: any
// CHECK-NEXT:        CondBranchInst %89: any, %BB29, %BB27
// CHECK-NEXT:%BB27:
// CHECK-NEXT:        StoreStackInst %87: any, %74: any
// CHECK-NEXT:        BranchInst %BB28
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %94 = LoadStackInst (:any) %74: any
// CHECK-NEXT:  %95 = BinaryStrictlyNotEqualInst (:any) %94: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %95: any, %BB30, %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:        StoreStackInst 1: number, %74: any
// CHECK-NEXT:        BranchInst %BB30
// CHECK-NEXT:%BB30:
// CHECK-NEXT:        TryStartInst %BB31, %BB33
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %100 = CatchInst (:any)
// CHECK-NEXT:         StoreStackInst %100: any, %75: any
// CHECK-NEXT:         BranchInst %BB21
// CHECK-NEXT:%BB32:
// CHECK-NEXT:         StoreStackInst undefined: undefined, %74: any
// CHECK-NEXT:  %104 = LoadStackInst (:any) %72: any
// CHECK-NEXT:         CondBranchInst %104: any, %BB37, %BB35
// CHECK-NEXT:%BB33:
// CHECK-NEXT:  %106 = LoadStackInst (:any) %74: any
// CHECK-NEXT:         StorePropertyLooseInst %106: any, globalObject: object, "b": string
// CHECK-NEXT:         TryEndInst %BB31, %BB34
// CHECK-NEXT:%BB34:
// CHECK-NEXT:         BranchInst %BB32
// CHECK-NEXT:%BB35:
// CHECK-NEXT:  %110 = LoadStackInst (:any) %68: any
// CHECK-NEXT:  %111 = IteratorNextInst (:any) %67: any, %110: any
// CHECK-NEXT:  %112 = LoadStackInst (:any) %67: any
// CHECK-NEXT:  %113 = BinaryStrictlyEqualInst (:any) %112: any, undefined: undefined
// CHECK-NEXT:         StoreStackInst %113: any, %72: any
// CHECK-NEXT:         CondBranchInst %113: any, %BB37, %BB36
// CHECK-NEXT:%BB36:
// CHECK-NEXT:         StoreStackInst %111: any, %74: any
// CHECK-NEXT:         BranchInst %BB37
// CHECK-NEXT:%BB37:
// CHECK-NEXT:         TryStartInst %BB38, %BB40
// CHECK-NEXT:%BB38:
// CHECK-NEXT:  %119 = CatchInst (:any)
// CHECK-NEXT:         StoreStackInst %119: any, %75: any
// CHECK-NEXT:         BranchInst %BB21
// CHECK-NEXT:%BB39:
// CHECK-NEXT:  %122 = LoadStackInst (:any) %72: any
// CHECK-NEXT:         CondBranchInst %122: any, %BB43, %BB42
// CHECK-NEXT:%BB40:
// CHECK-NEXT:  %124 = LoadStackInst (:any) %74: any
// CHECK-NEXT:         StorePropertyLooseInst %124: any, globalObject: object, "c": string
// CHECK-NEXT:         TryEndInst %BB38, %BB41
// CHECK-NEXT:%BB41:
// CHECK-NEXT:         BranchInst %BB39
// CHECK-NEXT:%BB42:
// CHECK-NEXT:  %128 = LoadStackInst (:any) %67: any
// CHECK-NEXT:  %129 = IteratorCloseInst (:any) %128: any, false: boolean
// CHECK-NEXT:         BranchInst %BB43
// CHECK-NEXT:%BB43:
// CHECK-NEXT:         TryEndInst %BB18, %BB46
// CHECK-NEXT:%BB44:
// CHECK-NEXT:  %132 = LoadStackInst (:any) %67: any
// CHECK-NEXT:  %133 = IteratorCloseInst (:any) %132: any, true: boolean
// CHECK-NEXT:         BranchInst %BB45
// CHECK-NEXT:%BB45:
// CHECK-NEXT:  %135 = LoadStackInst (:any) %75: any
// CHECK-NEXT:         ThrowInst %135: any
// CHECK-NEXT:%BB46:
// CHECK-NEXT:         BranchInst %BB19
// CHECK-NEXT:%BB47:
// CHECK-NEXT:  %138 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %139 = IteratorCloseInst (:any) %138: any, false: boolean
// CHECK-NEXT:         BranchInst %BB48
// CHECK-NEXT:%BB48:
// CHECK-NEXT:  %141 = LoadStackInst (:any) %4: any
// CHECK-NEXT:         ReturnInst %141: any
// CHECK-NEXT:%BB49:
// CHECK-NEXT:  %143 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %144 = IteratorCloseInst (:any) %143: any, true: boolean
// CHECK-NEXT:         BranchInst %BB50
// CHECK-NEXT:%BB50:
// CHECK-NEXT:  %146 = LoadStackInst (:any) %15: any
// CHECK-NEXT:         ThrowInst %146: any
// CHECK-NEXT:function_end
