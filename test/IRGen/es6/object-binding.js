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
// CHECK-NEXT:  %54 = IteratorNextInst (:any) %34: any, %35: any
// CHECK-NEXT:  %55 = LoadStackInst (:any) %34: any
// CHECK-NEXT:  %56 = BinaryStrictlyEqualInst (:any) %55: any, undefined: undefined
// CHECK-NEXT:  %57 = StoreStackInst %56: any, %39: any
// CHECK-NEXT:  %58 = CondBranchInst %56: any, %BB13, %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %59 = StoreStackInst %54: any, %41: any
// CHECK-NEXT:  %60 = BranchInst %BB15
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %61 = LoadStackInst (:any) %41: any
// CHECK-NEXT:  %62 = BinaryStrictlyNotEqualInst (:any) %61: any, undefined: undefined
// CHECK-NEXT:  %63 = CondBranchInst %62: any, %BB16, %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %64 = StoreStackInst 1: number, %41: any
// CHECK-NEXT:  %65 = BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %66 = TryStartInst %BB17, %BB18
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %67 = CatchInst (:any)
// CHECK-NEXT:  %68 = StoreStackInst %67: any, %42: any
// CHECK-NEXT:  %69 = BranchInst %BB7
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %70 = StoreStackInst undefined: undefined, %41: any
// CHECK-NEXT:  %71 = LoadStackInst (:any) %39: any
// CHECK-NEXT:  %72 = CondBranchInst %71: any, %BB20, %BB21
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %73 = LoadStackInst (:any) %41: any
// CHECK-NEXT:  %74 = StorePropertyLooseInst %73: any, globalObject: object, "b": string
// CHECK-NEXT:  %75 = BranchInst %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %76 = TryEndInst
// CHECK-NEXT:  %77 = BranchInst %BB19
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %78 = IteratorNextInst (:any) %34: any, %35: any
// CHECK-NEXT:  %79 = LoadStackInst (:any) %34: any
// CHECK-NEXT:  %80 = BinaryStrictlyEqualInst (:any) %79: any, undefined: undefined
// CHECK-NEXT:  %81 = StoreStackInst %80: any, %39: any
// CHECK-NEXT:  %82 = CondBranchInst %80: any, %BB20, %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %83 = StoreStackInst %78: any, %41: any
// CHECK-NEXT:  %84 = BranchInst %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %85 = TryStartInst %BB24, %BB25
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %86 = CatchInst (:any)
// CHECK-NEXT:  %87 = StoreStackInst %86: any, %42: any
// CHECK-NEXT:  %88 = BranchInst %BB7
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %89 = LoadStackInst (:any) %39: any
// CHECK-NEXT:  %90 = CondBranchInst %89: any, %BB27, %BB28
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %91 = LoadStackInst (:any) %41: any
// CHECK-NEXT:  %92 = StorePropertyLooseInst %91: any, globalObject: object, "e": string
// CHECK-NEXT:  %93 = BranchInst %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %94 = TryEndInst
// CHECK-NEXT:  %95 = BranchInst %BB26
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %96 = IteratorCloseInst (:any) %34: any, false: boolean
// CHECK-NEXT:  %97 = BranchInst %BB27
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %98 = LoadStackInst (:any) %4: any
// CHECK-NEXT:  %99 = ReturnInst (:any) %98: any
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %100 = IteratorCloseInst (:any) %34: any, true: boolean
// CHECK-NEXT:  %101 = BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %102 = LoadStackInst (:any) %42: any
// CHECK-NEXT:  %103 = ThrowInst %102: any
// CHECK-NEXT:function_end
