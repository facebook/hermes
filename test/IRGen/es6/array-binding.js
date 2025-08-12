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
// CHECK-NEXT:        CondBranchInst %17: any, %BB58, %BB57
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
// CHECK-NEXT:        TryStartInst %BB18, %BB20
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        TryStartInst %BB22, %BB24
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %58 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %58: any, %15: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB19:
// CHECK-NEXT:        BranchInst %BB17
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %62 = AllocArrayInst (:object) 2: number
// CHECK-NEXT:        DefineOwnInDenseArrayInst 2: number, %62: object, 1: number
// CHECK-NEXT:        StoreStackInst %62: object, %14: any
// CHECK-NEXT:        TryEndInst %BB18, %BB21
// CHECK-NEXT:%BB21:
// CHECK-NEXT:        BranchInst %BB19
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %67 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %67: any, %15: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %70 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        CondBranchInst %70: any, %BB56, %BB55
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %72 = LoadStackInst (:any) %14: any
// CHECK-NEXT:  %73 = AllocStackInst (:any) $?anon_6_iter: any
// CHECK-NEXT:  %74 = AllocStackInst (:any) $?anon_7_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %72: any, %74: any
// CHECK-NEXT:  %76 = IteratorBeginInst (:any) %74: any
// CHECK-NEXT:        StoreStackInst %76: any, %73: any
// CHECK-NEXT:  %78 = AllocStackInst (:any) $?anon_8_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %78: any
// CHECK-NEXT:  %80 = AllocStackInst (:any) $?anon_9_iterValue: any
// CHECK-NEXT:  %81 = AllocStackInst (:any) $?anon_10_exc: any
// CHECK-NEXT:        TryStartInst %BB26, %BB28
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %83 = LoadStackInst (:any) %78: any
// CHECK-NEXT:        CondBranchInst %83: any, %BB53, %BB52
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %85 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %85: any, %81: any
// CHECK-NEXT:        BranchInst %BB25
// CHECK-NEXT:%BB27:
// CHECK-NEXT:        StoreStackInst undefined: undefined, %80: any
// CHECK-NEXT:        BranchInst %BB30
// CHECK-NEXT:%BB28:
// CHECK-NEXT:        TryEndInst %BB26, %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:        BranchInst %BB27
// CHECK-NEXT:%BB30:
// CHECK-NEXT:  %92 = LoadStackInst (:any) %74: any
// CHECK-NEXT:  %93 = IteratorNextInst (:any) %73: any, %92: any
// CHECK-NEXT:  %94 = LoadStackInst (:any) %73: any
// CHECK-NEXT:  %95 = BinaryStrictlyEqualInst (:any) %94: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %95: any, %78: any
// CHECK-NEXT:        CondBranchInst %95: any, %BB33, %BB31
// CHECK-NEXT:%BB31:
// CHECK-NEXT:        StoreStackInst %93: any, %80: any
// CHECK-NEXT:        BranchInst %BB32
// CHECK-NEXT:%BB32:
// CHECK-NEXT:  %100 = LoadStackInst (:any) %80: any
// CHECK-NEXT:  %101 = BinaryStrictlyNotEqualInst (:any) %100: any, undefined: undefined
// CHECK-NEXT:         CondBranchInst %101: any, %BB34, %BB33
// CHECK-NEXT:%BB33:
// CHECK-NEXT:         TryStartInst %BB35, %BB37
// CHECK-NEXT:%BB34:
// CHECK-NEXT:         TryStartInst %BB39, %BB41
// CHECK-NEXT:%BB35:
// CHECK-NEXT:  %105 = CatchInst (:any)
// CHECK-NEXT:         StoreStackInst %105: any, %81: any
// CHECK-NEXT:         BranchInst %BB25
// CHECK-NEXT:%BB36:
// CHECK-NEXT:         BranchInst %BB34
// CHECK-NEXT:%BB37:
// CHECK-NEXT:         StoreStackInst 1: number, %80: any
// CHECK-NEXT:         TryEndInst %BB35, %BB38
// CHECK-NEXT:%BB38:
// CHECK-NEXT:         BranchInst %BB36
// CHECK-NEXT:%BB39:
// CHECK-NEXT:  %112 = CatchInst (:any)
// CHECK-NEXT:         StoreStackInst %112: any, %81: any
// CHECK-NEXT:         BranchInst %BB25
// CHECK-NEXT:%BB40:
// CHECK-NEXT:         StoreStackInst undefined: undefined, %80: any
// CHECK-NEXT:  %116 = LoadStackInst (:any) %78: any
// CHECK-NEXT:         CondBranchInst %116: any, %BB45, %BB43
// CHECK-NEXT:%BB41:
// CHECK-NEXT:  %118 = LoadStackInst (:any) %80: any
// CHECK-NEXT:         StorePropertyLooseInst %118: any, globalObject: object, "b": string
// CHECK-NEXT:         TryEndInst %BB39, %BB42
// CHECK-NEXT:%BB42:
// CHECK-NEXT:         BranchInst %BB40
// CHECK-NEXT:%BB43:
// CHECK-NEXT:  %122 = LoadStackInst (:any) %74: any
// CHECK-NEXT:  %123 = IteratorNextInst (:any) %73: any, %122: any
// CHECK-NEXT:  %124 = LoadStackInst (:any) %73: any
// CHECK-NEXT:  %125 = BinaryStrictlyEqualInst (:any) %124: any, undefined: undefined
// CHECK-NEXT:         StoreStackInst %125: any, %78: any
// CHECK-NEXT:         CondBranchInst %125: any, %BB45, %BB44
// CHECK-NEXT:%BB44:
// CHECK-NEXT:         StoreStackInst %123: any, %80: any
// CHECK-NEXT:         BranchInst %BB45
// CHECK-NEXT:%BB45:
// CHECK-NEXT:         TryStartInst %BB46, %BB48
// CHECK-NEXT:%BB46:
// CHECK-NEXT:  %131 = CatchInst (:any)
// CHECK-NEXT:         StoreStackInst %131: any, %81: any
// CHECK-NEXT:         BranchInst %BB25
// CHECK-NEXT:%BB47:
// CHECK-NEXT:  %134 = LoadStackInst (:any) %78: any
// CHECK-NEXT:         CondBranchInst %134: any, %BB51, %BB50
// CHECK-NEXT:%BB48:
// CHECK-NEXT:  %136 = LoadStackInst (:any) %80: any
// CHECK-NEXT:         StorePropertyLooseInst %136: any, globalObject: object, "c": string
// CHECK-NEXT:         TryEndInst %BB46, %BB49
// CHECK-NEXT:%BB49:
// CHECK-NEXT:         BranchInst %BB47
// CHECK-NEXT:%BB50:
// CHECK-NEXT:  %140 = LoadStackInst (:any) %73: any
// CHECK-NEXT:  %141 = IteratorCloseInst (:any) %140: any, false: boolean
// CHECK-NEXT:         BranchInst %BB51
// CHECK-NEXT:%BB51:
// CHECK-NEXT:         TryEndInst %BB22, %BB54
// CHECK-NEXT:%BB52:
// CHECK-NEXT:  %144 = LoadStackInst (:any) %73: any
// CHECK-NEXT:  %145 = IteratorCloseInst (:any) %144: any, true: boolean
// CHECK-NEXT:         BranchInst %BB53
// CHECK-NEXT:%BB53:
// CHECK-NEXT:  %147 = LoadStackInst (:any) %81: any
// CHECK-NEXT:         ThrowInst %147: any, %BB22
// CHECK-NEXT:%BB54:
// CHECK-NEXT:         BranchInst %BB23
// CHECK-NEXT:%BB55:
// CHECK-NEXT:  %150 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %151 = IteratorCloseInst (:any) %150: any, false: boolean
// CHECK-NEXT:         BranchInst %BB56
// CHECK-NEXT:%BB56:
// CHECK-NEXT:  %153 = LoadStackInst (:any) %4: any
// CHECK-NEXT:         ReturnInst %153: any
// CHECK-NEXT:%BB57:
// CHECK-NEXT:  %155 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %156 = IteratorCloseInst (:any) %155: any, true: boolean
// CHECK-NEXT:         BranchInst %BB58
// CHECK-NEXT:%BB58:
// CHECK-NEXT:  %158 = LoadStackInst (:any) %15: any
// CHECK-NEXT:         ThrowInst %158: any
// CHECK-NEXT:function_end
