/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

var {a} = x;

var {a, b} = x;

var {a: b, c: d, } = x;

var {a: b = g} = x;

var {a: [b = 1, e] = g} = x;

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "a": string
// CHECK-NEXT:       DeclareGlobalVarInst "b": string
// CHECK-NEXT:       DeclareGlobalVarInst "d": string
// CHECK-NEXT:       DeclareGlobalVarInst "e": string
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %5: any
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "a": string
// CHECK-NEXT:       StorePropertyLooseInst %8: any, globalObject: object, "a": string
// CHECK-NEXT:  %10 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %11 = LoadPropertyInst (:any) %10: any, "a": string
// CHECK-NEXT:        StorePropertyLooseInst %11: any, globalObject: object, "a": string
// CHECK-NEXT:  %13 = LoadPropertyInst (:any) %10: any, "b": string
// CHECK-NEXT:        StorePropertyLooseInst %13: any, globalObject: object, "b": string
// CHECK-NEXT:  %15 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %16 = LoadPropertyInst (:any) %15: any, "a": string
// CHECK-NEXT:        StorePropertyLooseInst %16: any, globalObject: object, "b": string
// CHECK-NEXT:  %18 = LoadPropertyInst (:any) %15: any, "c": string
// CHECK-NEXT:        StorePropertyLooseInst %18: any, globalObject: object, "d": string
// CHECK-NEXT:  %20 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %21 = LoadPropertyInst (:any) %20: any, "a": string
// CHECK-NEXT:  %22 = BinaryStrictlyNotEqualInst (:any) %21: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %22: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %24 = TryLoadGlobalPropertyInst (:any) globalObject: object, "g": string
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %26 = PhiInst (:any) %21: any, %BB0, %24: any, %BB1
// CHECK-NEXT:        StorePropertyLooseInst %26: any, globalObject: object, "b": string
// CHECK-NEXT:  %28 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %29 = LoadPropertyInst (:any) %28: any, "a": string
// CHECK-NEXT:  %30 = BinaryStrictlyNotEqualInst (:any) %29: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %30: any, %BB4, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %32 = TryLoadGlobalPropertyInst (:any) globalObject: object, "g": string
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %34 = PhiInst (:any) %29: any, %BB2, %32: any, %BB3
// CHECK-NEXT:  %35 = AllocStackInst (:any) $?anon_1_iter: any
// CHECK-NEXT:  %36 = AllocStackInst (:any) $?anon_2_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %34: any, %36: any
// CHECK-NEXT:  %38 = IteratorBeginInst (:any) %36: any
// CHECK-NEXT:        StoreStackInst %38: any, %35: any
// CHECK-NEXT:  %40 = AllocStackInst (:any) $?anon_3_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %40: any
// CHECK-NEXT:  %42 = AllocStackInst (:any) $?anon_4_iterValue: any
// CHECK-NEXT:  %43 = AllocStackInst (:any) $?anon_5_exc: any
// CHECK-NEXT:        TryStartInst %BB6, %BB8
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %45 = LoadStackInst (:any) %40: any
// CHECK-NEXT:        CondBranchInst %45: any, %BB29, %BB28
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %47 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %47: any, %43: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        StoreStackInst undefined: undefined, %42: any
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryEndInst %BB6, %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %54 = LoadStackInst (:any) %36: any
// CHECK-NEXT:  %55 = IteratorNextInst (:any) %35: any, %54: any
// CHECK-NEXT:  %56 = LoadStackInst (:any) %35: any
// CHECK-NEXT:  %57 = BinaryStrictlyEqualInst (:any) %56: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %57: any, %40: any
// CHECK-NEXT:        CondBranchInst %57: any, %BB13, %BB11
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        StoreStackInst %55: any, %42: any
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %62 = LoadStackInst (:any) %42: any
// CHECK-NEXT:  %63 = BinaryStrictlyNotEqualInst (:any) %62: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %63: any, %BB14, %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        StoreStackInst 1: number, %42: any
// CHECK-NEXT:        BranchInst %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        TryStartInst %BB15, %BB17
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %68 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %68: any, %43: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB16:
// CHECK-NEXT:        StoreStackInst undefined: undefined, %42: any
// CHECK-NEXT:  %72 = LoadStackInst (:any) %40: any
// CHECK-NEXT:        CondBranchInst %72: any, %BB21, %BB19
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %74 = LoadStackInst (:any) %42: any
// CHECK-NEXT:        StorePropertyLooseInst %74: any, globalObject: object, "b": string
// CHECK-NEXT:        TryEndInst %BB15, %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:        BranchInst %BB16
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %78 = LoadStackInst (:any) %36: any
// CHECK-NEXT:  %79 = IteratorNextInst (:any) %35: any, %78: any
// CHECK-NEXT:  %80 = LoadStackInst (:any) %35: any
// CHECK-NEXT:  %81 = BinaryStrictlyEqualInst (:any) %80: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %81: any, %40: any
// CHECK-NEXT:        CondBranchInst %81: any, %BB21, %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:        StoreStackInst %79: any, %42: any
// CHECK-NEXT:        BranchInst %BB21
// CHECK-NEXT:%BB21:
// CHECK-NEXT:        TryStartInst %BB22, %BB24
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %87 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %87: any, %43: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %90 = LoadStackInst (:any) %40: any
// CHECK-NEXT:        CondBranchInst %90: any, %BB27, %BB26
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %92 = LoadStackInst (:any) %42: any
// CHECK-NEXT:        StorePropertyLooseInst %92: any, globalObject: object, "e": string
// CHECK-NEXT:        TryEndInst %BB22, %BB25
// CHECK-NEXT:%BB25:
// CHECK-NEXT:        BranchInst %BB23
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %96 = LoadStackInst (:any) %35: any
// CHECK-NEXT:  %97 = IteratorCloseInst (:any) %96: any, false: boolean
// CHECK-NEXT:        BranchInst %BB27
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %99 = LoadStackInst (:any) %5: any
// CHECK-NEXT:         ReturnInst %99: any
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %101 = LoadStackInst (:any) %35: any
// CHECK-NEXT:  %102 = IteratorCloseInst (:any) %101: any, true: boolean
// CHECK-NEXT:         BranchInst %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %104 = LoadStackInst (:any) %43: any
// CHECK-NEXT:         ThrowInst %104: any
// CHECK-NEXT:function_end
