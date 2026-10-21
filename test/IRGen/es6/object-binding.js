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
// CHECK-NEXT:  %40 = AllocStackInst (:any) $?anon_3_iterValue: any
// CHECK-NEXT:  %41 = AllocStackInst (:any) $?anon_4_exc: any
// CHECK-NEXT:        TryStartInst %BB6, %BB8
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %43 = LoadStackInst (:any) %35: any
// CHECK-NEXT:  %44 = IteratorCloseInst (:any) %43: any, true: boolean
// CHECK-NEXT:  %45 = LoadStackInst (:any) %41: any
// CHECK-NEXT:        ThrowInst %45: any
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %47 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %47: any, %41: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %50 = LoadStackInst (:any) %36: any
// CHECK-NEXT:  %51 = IteratorNextInst (:any) %35: any, %50: any
// CHECK-NEXT:        StoreStackInst %51: any, %40: any
// CHECK-NEXT:  %53 = LoadStackInst (:any) %40: any
// CHECK-NEXT:  %54 = BinaryStrictlyNotEqualInst (:any) %53: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %54: any, %BB11, %BB10
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryEndInst %BB6, %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        TryStartInst %BB12, %BB14
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        TryStartInst %BB16, %BB18
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %60 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %60: any, %41: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        BranchInst %BB11
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        StoreStackInst 1: number, %40: any
// CHECK-NEXT:        TryEndInst %BB12, %BB15
// CHECK-NEXT:%BB15:
// CHECK-NEXT:        BranchInst %BB13
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %67 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %67: any, %41: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %70 = LoadStackInst (:any) %36: any
// CHECK-NEXT:  %71 = IteratorNextInst (:any) %35: any, %70: any
// CHECK-NEXT:        StoreStackInst %71: any, %40: any
// CHECK-NEXT:        TryStartInst %BB20, %BB22
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %74 = LoadStackInst (:any) %40: any
// CHECK-NEXT:        StorePropertyLooseInst %74: any, globalObject: object, "b": string
// CHECK-NEXT:        TryEndInst %BB16, %BB19
// CHECK-NEXT:%BB19:
// CHECK-NEXT:        BranchInst %BB17
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %78 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %78: any, %41: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %81 = LoadStackInst (:any) %35: any
// CHECK-NEXT:  %82 = IteratorCloseInst (:any) %81: any, false: boolean
// CHECK-NEXT:  %83 = LoadStackInst (:any) %5: any
// CHECK-NEXT:        ReturnInst %83: any
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %85 = LoadStackInst (:any) %40: any
// CHECK-NEXT:        StorePropertyLooseInst %85: any, globalObject: object, "e": string
// CHECK-NEXT:        TryEndInst %BB20, %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:        BranchInst %BB21
// CHECK-NEXT:function_end
