/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

var [a, [b = 1, c] = [,2]] = x;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
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
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %27 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %28 = IteratorNextInst (:any) %7: any, %27: any
// CHECK-NEXT:  %29 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %30 = BinaryStrictlyEqualInst (:any) %29: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %30: any, %12: any
// CHECK-NEXT:        CondBranchInst %30: any, %BB8, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        StoreStackInst %28: any, %14: any
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryStartInst %BB9, %BB11
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %36 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %36: any, %15: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        StoreStackInst undefined: undefined, %14: any
// CHECK-NEXT:  %40 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        CondBranchInst %40: any, %BB15, %BB13
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %42 = LoadStackInst (:any) %14: any
// CHECK-NEXT:        StorePropertyLooseInst %42: any, globalObject: object, "a": string
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %47 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %48 = IteratorNextInst (:any) %7: any, %47: any
// CHECK-NEXT:  %49 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %50 = BinaryStrictlyEqualInst (:any) %49: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %50: any, %12: any
// CHECK-NEXT:        CondBranchInst %50: any, %BB16, %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        StoreStackInst %48: any, %14: any
// CHECK-NEXT:        BranchInst %BB15
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %55 = LoadStackInst (:any) %14: any
// CHECK-NEXT:  %56 = BinaryStrictlyNotEqualInst (:any) %55: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %56: any, %BB17, %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %58 = AllocArrayInst (:object) 2: number
// CHECK-NEXT:        StoreOwnPropertyInst 2: number, %58: object, 1: number, true: boolean
// CHECK-NEXT:        StoreStackInst %58: object, %14: any
// CHECK-NEXT:        BranchInst %BB17
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        TryStartInst %BB18, %BB20
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %63 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %63: any, %15: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %66 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        CondBranchInst %66: any, %BB48, %BB47
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %68 = LoadStackInst (:any) %14: any
// CHECK-NEXT:  %69 = AllocStackInst (:any) $?anon_6_iter: any
// CHECK-NEXT:  %70 = AllocStackInst (:any) $?anon_7_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %68: any, %70: any
// CHECK-NEXT:  %72 = IteratorBeginInst (:any) %70: any
// CHECK-NEXT:        StoreStackInst %72: any, %69: any
// CHECK-NEXT:  %74 = AllocStackInst (:any) $?anon_8_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %74: any
// CHECK-NEXT:  %76 = AllocStackInst (:any) $?anon_9_iterValue: any
// CHECK-NEXT:  %77 = AllocStackInst (:any) $?anon_10_exc: any
// CHECK-NEXT:        TryStartInst %BB22, %BB24
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %79 = LoadStackInst (:any) %74: any
// CHECK-NEXT:        CondBranchInst %79: any, %BB45, %BB44
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %81 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %81: any, %77: any
// CHECK-NEXT:        BranchInst %BB21
// CHECK-NEXT:%BB23:
// CHECK-NEXT:        StoreStackInst undefined: undefined, %76: any
// CHECK-NEXT:        BranchInst %BB26
// CHECK-NEXT:%BB24:
// CHECK-NEXT:        BranchInst %BB25
// CHECK-NEXT:%BB25:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB23
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %89 = LoadStackInst (:any) %70: any
// CHECK-NEXT:  %90 = IteratorNextInst (:any) %69: any, %89: any
// CHECK-NEXT:  %91 = LoadStackInst (:any) %69: any
// CHECK-NEXT:  %92 = BinaryStrictlyEqualInst (:any) %91: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %92: any, %74: any
// CHECK-NEXT:        CondBranchInst %92: any, %BB29, %BB27
// CHECK-NEXT:%BB27:
// CHECK-NEXT:        StoreStackInst %90: any, %76: any
// CHECK-NEXT:        BranchInst %BB28
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %97 = LoadStackInst (:any) %76: any
// CHECK-NEXT:  %98 = BinaryStrictlyNotEqualInst (:any) %97: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %98: any, %BB30, %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:         StoreStackInst 1: number, %76: any
// CHECK-NEXT:         BranchInst %BB30
// CHECK-NEXT:%BB30:
// CHECK-NEXT:         TryStartInst %BB31, %BB33
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %103 = CatchInst (:any)
// CHECK-NEXT:         StoreStackInst %103: any, %77: any
// CHECK-NEXT:         BranchInst %BB21
// CHECK-NEXT:%BB32:
// CHECK-NEXT:         StoreStackInst undefined: undefined, %76: any
// CHECK-NEXT:  %107 = LoadStackInst (:any) %74: any
// CHECK-NEXT:         CondBranchInst %107: any, %BB37, %BB35
// CHECK-NEXT:%BB33:
// CHECK-NEXT:  %109 = LoadStackInst (:any) %76: any
// CHECK-NEXT:         StorePropertyLooseInst %109: any, globalObject: object, "b": string
// CHECK-NEXT:         BranchInst %BB34
// CHECK-NEXT:%BB34:
// CHECK-NEXT:         TryEndInst
// CHECK-NEXT:         BranchInst %BB32
// CHECK-NEXT:%BB35:
// CHECK-NEXT:  %114 = LoadStackInst (:any) %70: any
// CHECK-NEXT:  %115 = IteratorNextInst (:any) %69: any, %114: any
// CHECK-NEXT:  %116 = LoadStackInst (:any) %69: any
// CHECK-NEXT:  %117 = BinaryStrictlyEqualInst (:any) %116: any, undefined: undefined
// CHECK-NEXT:         StoreStackInst %117: any, %74: any
// CHECK-NEXT:         CondBranchInst %117: any, %BB37, %BB36
// CHECK-NEXT:%BB36:
// CHECK-NEXT:         StoreStackInst %115: any, %76: any
// CHECK-NEXT:         BranchInst %BB37
// CHECK-NEXT:%BB37:
// CHECK-NEXT:         TryStartInst %BB38, %BB40
// CHECK-NEXT:%BB38:
// CHECK-NEXT:  %123 = CatchInst (:any)
// CHECK-NEXT:         StoreStackInst %123: any, %77: any
// CHECK-NEXT:         BranchInst %BB21
// CHECK-NEXT:%BB39:
// CHECK-NEXT:  %126 = LoadStackInst (:any) %74: any
// CHECK-NEXT:         CondBranchInst %126: any, %BB43, %BB42
// CHECK-NEXT:%BB40:
// CHECK-NEXT:  %128 = LoadStackInst (:any) %76: any
// CHECK-NEXT:         StorePropertyLooseInst %128: any, globalObject: object, "c": string
// CHECK-NEXT:         BranchInst %BB41
// CHECK-NEXT:%BB41:
// CHECK-NEXT:         TryEndInst
// CHECK-NEXT:         BranchInst %BB39
// CHECK-NEXT:%BB42:
// CHECK-NEXT:  %133 = LoadStackInst (:any) %69: any
// CHECK-NEXT:  %134 = IteratorCloseInst (:any) %133: any, false: boolean
// CHECK-NEXT:         BranchInst %BB43
// CHECK-NEXT:%BB43:
// CHECK-NEXT:         BranchInst %BB46
// CHECK-NEXT:%BB44:
// CHECK-NEXT:  %137 = LoadStackInst (:any) %69: any
// CHECK-NEXT:  %138 = IteratorCloseInst (:any) %137: any, true: boolean
// CHECK-NEXT:         BranchInst %BB45
// CHECK-NEXT:%BB45:
// CHECK-NEXT:  %140 = LoadStackInst (:any) %77: any
// CHECK-NEXT:         ThrowInst %140: any
// CHECK-NEXT:%BB46:
// CHECK-NEXT:         TryEndInst
// CHECK-NEXT:         BranchInst %BB19
// CHECK-NEXT:%BB47:
// CHECK-NEXT:  %144 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %145 = IteratorCloseInst (:any) %144: any, false: boolean
// CHECK-NEXT:         BranchInst %BB48
// CHECK-NEXT:%BB48:
// CHECK-NEXT:  %147 = LoadStackInst (:any) %4: any
// CHECK-NEXT:         ReturnInst %147: any
// CHECK-NEXT:%BB49:
// CHECK-NEXT:  %149 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %150 = IteratorCloseInst (:any) %149: any, true: boolean
// CHECK-NEXT:         BranchInst %BB50
// CHECK-NEXT:%BB50:
// CHECK-NEXT:  %152 = LoadStackInst (:any) %15: any
// CHECK-NEXT:         ThrowInst %152: any
// CHECK-NEXT:function_end
