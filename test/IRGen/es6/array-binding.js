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
// CHECK-NEXT:       DeclareGlobalVarInst "a": string
// CHECK-NEXT:       DeclareGlobalVarInst "b": string
// CHECK-NEXT:       DeclareGlobalVarInst "c": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_iter: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_2_sourceOrNext: any
// CHECK-NEXT:       StoreStackInst %5: any, %7: any
// CHECK-NEXT:  %9 = IteratorBeginInst (:any) %7: any
// CHECK-NEXT:        StoreStackInst %9: any, %6: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_3_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %11: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_4_iterValue: any
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_5_exc: any
// CHECK-NEXT:        TryStartInst %BB2, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = LoadStackInst (:any) %11: any
// CHECK-NEXT:        CondBranchInst %16: any, %BB50, %BB49
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %18 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %18: any, %14: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %26 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %27 = IteratorNextInst (:any) %6: any, %26: any
// CHECK-NEXT:  %28 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %29 = BinaryStrictlyEqualInst (:any) %28: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %29: any, %11: any
// CHECK-NEXT:        CondBranchInst %29: any, %BB8, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        StoreStackInst %27: any, %13: any
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryStartInst %BB9, %BB11
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %35 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %35: any, %14: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %39 = LoadStackInst (:any) %11: any
// CHECK-NEXT:        CondBranchInst %39: any, %BB15, %BB13
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %41 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        StorePropertyLooseInst %41: any, globalObject: object, "a": string
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %46 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %47 = IteratorNextInst (:any) %6: any, %46: any
// CHECK-NEXT:  %48 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %49 = BinaryStrictlyEqualInst (:any) %48: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %49: any, %11: any
// CHECK-NEXT:        CondBranchInst %49: any, %BB16, %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        StoreStackInst %47: any, %13: any
// CHECK-NEXT:        BranchInst %BB15
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %54 = LoadStackInst (:any) %13: any
// CHECK-NEXT:  %55 = BinaryStrictlyNotEqualInst (:any) %54: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %55: any, %BB17, %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %57 = AllocArrayInst (:object) 2: number
// CHECK-NEXT:        StoreOwnPropertyInst 2: number, %57: object, 1: number, true: boolean
// CHECK-NEXT:        StoreStackInst %57: object, %13: any
// CHECK-NEXT:        BranchInst %BB17
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        TryStartInst %BB18, %BB20
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %62 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %62: any, %14: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %65 = LoadStackInst (:any) %11: any
// CHECK-NEXT:        CondBranchInst %65: any, %BB48, %BB47
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %67 = LoadStackInst (:any) %13: any
// CHECK-NEXT:  %68 = AllocStackInst (:any) $?anon_6_iter: any
// CHECK-NEXT:  %69 = AllocStackInst (:any) $?anon_7_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %67: any, %69: any
// CHECK-NEXT:  %71 = IteratorBeginInst (:any) %69: any
// CHECK-NEXT:        StoreStackInst %71: any, %68: any
// CHECK-NEXT:  %73 = AllocStackInst (:any) $?anon_8_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %73: any
// CHECK-NEXT:  %75 = AllocStackInst (:any) $?anon_9_iterValue: any
// CHECK-NEXT:  %76 = AllocStackInst (:any) $?anon_10_exc: any
// CHECK-NEXT:        TryStartInst %BB22, %BB24
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %78 = LoadStackInst (:any) %73: any
// CHECK-NEXT:        CondBranchInst %78: any, %BB45, %BB44
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %80 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %80: any, %76: any
// CHECK-NEXT:        BranchInst %BB21
// CHECK-NEXT:%BB23:
// CHECK-NEXT:        StoreStackInst undefined: undefined, %75: any
// CHECK-NEXT:        BranchInst %BB26
// CHECK-NEXT:%BB24:
// CHECK-NEXT:        BranchInst %BB25
// CHECK-NEXT:%BB25:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB23
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %88 = LoadStackInst (:any) %69: any
// CHECK-NEXT:  %89 = IteratorNextInst (:any) %68: any, %88: any
// CHECK-NEXT:  %90 = LoadStackInst (:any) %68: any
// CHECK-NEXT:  %91 = BinaryStrictlyEqualInst (:any) %90: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %91: any, %73: any
// CHECK-NEXT:        CondBranchInst %91: any, %BB29, %BB27
// CHECK-NEXT:%BB27:
// CHECK-NEXT:        StoreStackInst %89: any, %75: any
// CHECK-NEXT:        BranchInst %BB28
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %96 = LoadStackInst (:any) %75: any
// CHECK-NEXT:  %97 = BinaryStrictlyNotEqualInst (:any) %96: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %97: any, %BB30, %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:        StoreStackInst 1: number, %75: any
// CHECK-NEXT:         BranchInst %BB30
// CHECK-NEXT:%BB30:
// CHECK-NEXT:         TryStartInst %BB31, %BB33
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %102 = CatchInst (:any)
// CHECK-NEXT:         StoreStackInst %102: any, %76: any
// CHECK-NEXT:         BranchInst %BB21
// CHECK-NEXT:%BB32:
// CHECK-NEXT:         StoreStackInst undefined: undefined, %75: any
// CHECK-NEXT:  %106 = LoadStackInst (:any) %73: any
// CHECK-NEXT:         CondBranchInst %106: any, %BB37, %BB35
// CHECK-NEXT:%BB33:
// CHECK-NEXT:  %108 = LoadStackInst (:any) %75: any
// CHECK-NEXT:         StorePropertyLooseInst %108: any, globalObject: object, "b": string
// CHECK-NEXT:         BranchInst %BB34
// CHECK-NEXT:%BB34:
// CHECK-NEXT:         TryEndInst
// CHECK-NEXT:         BranchInst %BB32
// CHECK-NEXT:%BB35:
// CHECK-NEXT:  %113 = LoadStackInst (:any) %69: any
// CHECK-NEXT:  %114 = IteratorNextInst (:any) %68: any, %113: any
// CHECK-NEXT:  %115 = LoadStackInst (:any) %68: any
// CHECK-NEXT:  %116 = BinaryStrictlyEqualInst (:any) %115: any, undefined: undefined
// CHECK-NEXT:         StoreStackInst %116: any, %73: any
// CHECK-NEXT:         CondBranchInst %116: any, %BB37, %BB36
// CHECK-NEXT:%BB36:
// CHECK-NEXT:         StoreStackInst %114: any, %75: any
// CHECK-NEXT:         BranchInst %BB37
// CHECK-NEXT:%BB37:
// CHECK-NEXT:         TryStartInst %BB38, %BB40
// CHECK-NEXT:%BB38:
// CHECK-NEXT:  %122 = CatchInst (:any)
// CHECK-NEXT:         StoreStackInst %122: any, %76: any
// CHECK-NEXT:         BranchInst %BB21
// CHECK-NEXT:%BB39:
// CHECK-NEXT:  %125 = LoadStackInst (:any) %73: any
// CHECK-NEXT:         CondBranchInst %125: any, %BB43, %BB42
// CHECK-NEXT:%BB40:
// CHECK-NEXT:  %127 = LoadStackInst (:any) %75: any
// CHECK-NEXT:         StorePropertyLooseInst %127: any, globalObject: object, "c": string
// CHECK-NEXT:         BranchInst %BB41
// CHECK-NEXT:%BB41:
// CHECK-NEXT:         TryEndInst
// CHECK-NEXT:         BranchInst %BB39
// CHECK-NEXT:%BB42:
// CHECK-NEXT:  %132 = LoadStackInst (:any) %68: any
// CHECK-NEXT:  %133 = IteratorCloseInst (:any) %132: any, false: boolean
// CHECK-NEXT:         BranchInst %BB43
// CHECK-NEXT:%BB43:
// CHECK-NEXT:         BranchInst %BB46
// CHECK-NEXT:%BB44:
// CHECK-NEXT:  %136 = LoadStackInst (:any) %68: any
// CHECK-NEXT:  %137 = IteratorCloseInst (:any) %136: any, true: boolean
// CHECK-NEXT:         BranchInst %BB45
// CHECK-NEXT:%BB45:
// CHECK-NEXT:  %139 = LoadStackInst (:any) %76: any
// CHECK-NEXT:         ThrowInst %139: any
// CHECK-NEXT:%BB46:
// CHECK-NEXT:         TryEndInst
// CHECK-NEXT:         BranchInst %BB19
// CHECK-NEXT:%BB47:
// CHECK-NEXT:  %143 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %144 = IteratorCloseInst (:any) %143: any, false: boolean
// CHECK-NEXT:         BranchInst %BB48
// CHECK-NEXT:%BB48:
// CHECK-NEXT:  %146 = LoadStackInst (:any) %3: any
// CHECK-NEXT:         ReturnInst %146: any
// CHECK-NEXT:%BB49:
// CHECK-NEXT:  %148 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %149 = IteratorCloseInst (:any) %148: any, true: boolean
// CHECK-NEXT:         BranchInst %BB50
// CHECK-NEXT:%BB50:
// CHECK-NEXT:  %151 = LoadStackInst (:any) %14: any
// CHECK-NEXT:         ThrowInst %151: any
// CHECK-NEXT:function_end
