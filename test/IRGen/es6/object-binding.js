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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "a": string
// CHECK-NEXT:       DeclareGlobalVarInst "b": string
// CHECK-NEXT:       DeclareGlobalVarInst "d": string
// CHECK-NEXT:       DeclareGlobalVarInst "e": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "a": string
// CHECK-NEXT:       StorePropertyLooseInst %7: any, globalObject: object, "a": string
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "a": string
// CHECK-NEXT:        StorePropertyLooseInst %10: any, globalObject: object, "a": string
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) %9: any, "b": string
// CHECK-NEXT:        StorePropertyLooseInst %12: any, globalObject: object, "b": string
// CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %15 = LoadPropertyInst (:any) %14: any, "a": string
// CHECK-NEXT:        StorePropertyLooseInst %15: any, globalObject: object, "b": string
// CHECK-NEXT:  %17 = LoadPropertyInst (:any) %14: any, "c": string
// CHECK-NEXT:        StorePropertyLooseInst %17: any, globalObject: object, "d": string
// CHECK-NEXT:  %19 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %20 = LoadPropertyInst (:any) %19: any, "a": string
// CHECK-NEXT:  %21 = BinaryStrictlyNotEqualInst (:any) %20: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %21: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %23 = TryLoadGlobalPropertyInst (:any) globalObject: object, "g": string
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %25 = PhiInst (:any) %20: any, %BB0, %23: any, %BB2
// CHECK-NEXT:        StorePropertyLooseInst %25: any, globalObject: object, "b": string
// CHECK-NEXT:  %27 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %28 = LoadPropertyInst (:any) %27: any, "a": string
// CHECK-NEXT:  %29 = BinaryStrictlyNotEqualInst (:any) %28: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %29: any, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %31 = TryLoadGlobalPropertyInst (:any) globalObject: object, "g": string
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %33 = PhiInst (:any) %28: any, %BB1, %31: any, %BB4
// CHECK-NEXT:  %34 = AllocStackInst (:any) $?anon_1_iter: any
// CHECK-NEXT:  %35 = AllocStackInst (:any) $?anon_2_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %33: any, %35: any
// CHECK-NEXT:  %37 = IteratorBeginInst (:any) %35: any
// CHECK-NEXT:        StoreStackInst %37: any, %34: any
// CHECK-NEXT:  %39 = AllocStackInst (:any) $?anon_3_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %39: any
// CHECK-NEXT:  %41 = AllocStackInst (:any) $?anon_4_iterValue: any
// CHECK-NEXT:  %42 = AllocStackInst (:any) $?anon_5_exc: any
// CHECK-NEXT:        TryStartInst %BB5, %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %44 = LoadStackInst (:any) %39: any
// CHECK-NEXT:        CondBranchInst %44: any, %BB8, %BB9
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %46 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %46: any, %42: any
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        StoreStackInst undefined: undefined, %41: any
// CHECK-NEXT:        BranchInst %BB11
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %54 = LoadStackInst (:any) %35: any
// CHECK-NEXT:  %55 = IteratorNextInst (:any) %34: any, %54: any
// CHECK-NEXT:  %56 = LoadStackInst (:any) %34: any
// CHECK-NEXT:  %57 = BinaryStrictlyEqualInst (:any) %56: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %57: any, %39: any
// CHECK-NEXT:        CondBranchInst %57: any, %BB13, %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        StoreStackInst %55: any, %41: any
// CHECK-NEXT:        BranchInst %BB15
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %62 = LoadStackInst (:any) %41: any
// CHECK-NEXT:  %63 = BinaryStrictlyNotEqualInst (:any) %62: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %63: any, %BB16, %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        StoreStackInst 1: number, %41: any
// CHECK-NEXT:        BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:        TryStartInst %BB17, %BB18
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %68 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %68: any, %42: any
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB19:
// CHECK-NEXT:        StoreStackInst undefined: undefined, %41: any
// CHECK-NEXT:  %72 = LoadStackInst (:any) %39: any
// CHECK-NEXT:        CondBranchInst %72: any, %BB20, %BB21
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %74 = LoadStackInst (:any) %41: any
// CHECK-NEXT:        StorePropertyLooseInst %74: any, globalObject: object, "b": string
// CHECK-NEXT:        BranchInst %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB19
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %79 = LoadStackInst (:any) %35: any
// CHECK-NEXT:  %80 = IteratorNextInst (:any) %34: any, %79: any
// CHECK-NEXT:  %81 = LoadStackInst (:any) %34: any
// CHECK-NEXT:  %82 = BinaryStrictlyEqualInst (:any) %81: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %82: any, %39: any
// CHECK-NEXT:        CondBranchInst %82: any, %BB20, %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:        StoreStackInst %80: any, %41: any
// CHECK-NEXT:        BranchInst %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:        TryStartInst %BB24, %BB25
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %88 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %88: any, %42: any
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %91 = LoadStackInst (:any) %39: any
// CHECK-NEXT:        CondBranchInst %91: any, %BB27, %BB28
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %93 = LoadStackInst (:any) %41: any
// CHECK-NEXT:        StorePropertyLooseInst %93: any, globalObject: object, "e": string
// CHECK-NEXT:        BranchInst %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB26
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %98 = LoadStackInst (:any) %34: any
// CHECK-NEXT:  %99 = IteratorCloseInst (:any) %98: any, false: boolean
// CHECK-NEXT:         BranchInst %BB27
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %101 = LoadStackInst (:any) %4: any
// CHECK-NEXT:         ReturnInst %101: any
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %103 = LoadStackInst (:any) %34: any
// CHECK-NEXT:  %104 = IteratorCloseInst (:any) %103: any, true: boolean
// CHECK-NEXT:         BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %106 = LoadStackInst (:any) %42: any
// CHECK-NEXT:         ThrowInst %106: any
// CHECK-NEXT:function_end
