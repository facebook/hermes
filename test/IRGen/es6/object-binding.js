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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "a": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "b": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "d": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "e": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %5 = StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "a": string
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7: any, globalObject: object, "a": string
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "a": string
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: any, globalObject: object, "a": string
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) %9: any, "b": string
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12: any, globalObject: object, "b": string
// CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %15 = LoadPropertyInst (:any) %14: any, "a": string
// CHECK-NEXT:  %16 = StorePropertyLooseInst %15: any, globalObject: object, "b": string
// CHECK-NEXT:  %17 = LoadPropertyInst (:any) %14: any, "c": string
// CHECK-NEXT:  %18 = StorePropertyLooseInst %17: any, globalObject: object, "d": string
// CHECK-NEXT:  %19 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %20 = LoadPropertyInst (:any) %19: any, "a": string
// CHECK-NEXT:  %21 = BinaryStrictlyNotEqualInst (:any) %20: any, undefined: undefined
// CHECK-NEXT:  %22 = CondBranchInst %21: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %23 = TryLoadGlobalPropertyInst (:any) globalObject: object, "g": string
// CHECK-NEXT:  %24 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %25 = PhiInst (:any) %20: any, %BB0, %23: any, %BB2
// CHECK-NEXT:  %26 = StorePropertyLooseInst %25: any, globalObject: object, "b": string
// CHECK-NEXT:  %27 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %28 = LoadPropertyInst (:any) %27: any, "a": string
// CHECK-NEXT:  %29 = BinaryStrictlyNotEqualInst (:any) %28: any, undefined: undefined
// CHECK-NEXT:  %30 = CondBranchInst %29: any, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %31 = TryLoadGlobalPropertyInst (:any) globalObject: object, "g": string
// CHECK-NEXT:  %32 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %33 = PhiInst (:any) %28: any, %BB1, %31: any, %BB4
// CHECK-NEXT:  %34 = AllocStackInst (:any) $?anon_1_iter: any
// CHECK-NEXT:  %35 = AllocStackInst (:any) $?anon_2_sourceOrNext: any
// CHECK-NEXT:  %36 = StoreStackInst %33: any, %35: any
// CHECK-NEXT:  %37 = IteratorBeginInst (:any) %35: any
// CHECK-NEXT:  %38 = StoreStackInst %37: any, %34: any
// CHECK-NEXT:  %39 = AllocStackInst (:any) $?anon_3_iterDone: any
// CHECK-NEXT:  %40 = StoreStackInst undefined: undefined, %39: any
// CHECK-NEXT:  %41 = AllocStackInst (:any) $?anon_4_iterValue: any
// CHECK-NEXT:  %42 = AllocStackInst (:any) $?anon_5_exc: any
// CHECK-NEXT:  %43 = TryStartInst %BB5, %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %44 = LoadStackInst (:any) %39: any
// CHECK-NEXT:  %45 = CondBranchInst %44: any, %BB8, %BB9
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %46 = CatchInst (:any)
// CHECK-NEXT:  %47 = StoreStackInst %46: any, %42: any
// CHECK-NEXT:  %48 = BranchInst %BB7
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %49 = StoreStackInst undefined: undefined, %41: any
// CHECK-NEXT:  %50 = BranchInst %BB11
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %51 = BranchInst %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %52 = TryEndInst
// CHECK-NEXT:  %53 = BranchInst %BB10
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %54 = LoadStackInst (:any) %35: any
// CHECK-NEXT:  %55 = IteratorNextInst (:any) %34: any, %54: any
// CHECK-NEXT:  %56 = LoadStackInst (:any) %34: any
// CHECK-NEXT:  %57 = BinaryStrictlyEqualInst (:any) %56: any, undefined: undefined
// CHECK-NEXT:  %58 = StoreStackInst %57: any, %39: any
// CHECK-NEXT:  %59 = CondBranchInst %57: any, %BB13, %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %60 = StoreStackInst %55: any, %41: any
// CHECK-NEXT:  %61 = BranchInst %BB15
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %62 = LoadStackInst (:any) %41: any
// CHECK-NEXT:  %63 = BinaryStrictlyNotEqualInst (:any) %62: any, undefined: undefined
// CHECK-NEXT:  %64 = CondBranchInst %63: any, %BB16, %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %65 = StoreStackInst 1: number, %41: any
// CHECK-NEXT:  %66 = BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %67 = TryStartInst %BB17, %BB18
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %68 = CatchInst (:any)
// CHECK-NEXT:  %69 = StoreStackInst %68: any, %42: any
// CHECK-NEXT:  %70 = BranchInst %BB7
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %71 = StoreStackInst undefined: undefined, %41: any
// CHECK-NEXT:  %72 = LoadStackInst (:any) %39: any
// CHECK-NEXT:  %73 = CondBranchInst %72: any, %BB20, %BB21
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %74 = LoadStackInst (:any) %41: any
// CHECK-NEXT:  %75 = StorePropertyLooseInst %74: any, globalObject: object, "b": string
// CHECK-NEXT:  %76 = BranchInst %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %77 = TryEndInst
// CHECK-NEXT:  %78 = BranchInst %BB19
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %79 = LoadStackInst (:any) %35: any
// CHECK-NEXT:  %80 = IteratorNextInst (:any) %34: any, %79: any
// CHECK-NEXT:  %81 = LoadStackInst (:any) %34: any
// CHECK-NEXT:  %82 = BinaryStrictlyEqualInst (:any) %81: any, undefined: undefined
// CHECK-NEXT:  %83 = StoreStackInst %82: any, %39: any
// CHECK-NEXT:  %84 = CondBranchInst %82: any, %BB20, %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %85 = StoreStackInst %80: any, %41: any
// CHECK-NEXT:  %86 = BranchInst %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %87 = TryStartInst %BB24, %BB25
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %88 = CatchInst (:any)
// CHECK-NEXT:  %89 = StoreStackInst %88: any, %42: any
// CHECK-NEXT:  %90 = BranchInst %BB7
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %91 = LoadStackInst (:any) %39: any
// CHECK-NEXT:  %92 = CondBranchInst %91: any, %BB27, %BB28
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %93 = LoadStackInst (:any) %41: any
// CHECK-NEXT:  %94 = StorePropertyLooseInst %93: any, globalObject: object, "e": string
// CHECK-NEXT:  %95 = BranchInst %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %96 = TryEndInst
// CHECK-NEXT:  %97 = BranchInst %BB26
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %98 = LoadStackInst (:any) %34: any
// CHECK-NEXT:  %99 = IteratorCloseInst (:any) %98: any, false: boolean
// CHECK-NEXT:  %100 = BranchInst %BB27
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %101 = LoadStackInst (:any) %4: any
// CHECK-NEXT:  %102 = ReturnInst (:any) %101: any
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %103 = LoadStackInst (:any) %34: any
// CHECK-NEXT:  %104 = IteratorCloseInst (:any) %103: any, true: boolean
// CHECK-NEXT:  %105 = BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %106 = LoadStackInst (:any) %42: any
// CHECK-NEXT:  %107 = ThrowInst %106: any
// CHECK-NEXT:function_end
