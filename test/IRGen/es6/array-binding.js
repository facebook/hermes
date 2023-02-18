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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "a": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "b": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "c": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_iter: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_2_sourceOrNext: any
// CHECK-NEXT:  %8 = StoreStackInst %5: any, %7: any
// CHECK-NEXT:  %9 = IteratorBeginInst (:any) %7: any
// CHECK-NEXT:  %10 = StoreStackInst %9: any, %6: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_3_iterDone: any
// CHECK-NEXT:  %12 = StoreStackInst undefined: undefined, %11: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_4_iterValue: any
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_5_exc: any
// CHECK-NEXT:  %15 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst (:any) %11: any
// CHECK-NEXT:  %17 = CondBranchInst %16: any, %BB4, %BB5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %18 = CatchInst (:any)
// CHECK-NEXT:  %19 = StoreStackInst %18: any, %14: any
// CHECK-NEXT:  %20 = BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %21 = StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %22 = BranchInst %BB7
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %23 = BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %24 = TryEndInst
// CHECK-NEXT:  %25 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %26 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %27 = IteratorNextInst (:any) %6: any, %26: any
// CHECK-NEXT:  %28 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %29 = BinaryStrictlyEqualInst (:any) %28: any, undefined: undefined
// CHECK-NEXT:  %30 = StoreStackInst %29: any, %11: any
// CHECK-NEXT:  %31 = CondBranchInst %29: any, %BB9, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %32 = StoreStackInst %27: any, %13: any
// CHECK-NEXT:  %33 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %34 = TryStartInst %BB11, %BB12
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %35 = CatchInst (:any)
// CHECK-NEXT:  %36 = StoreStackInst %35: any, %14: any
// CHECK-NEXT:  %37 = BranchInst %BB3
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %38 = StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %39 = LoadStackInst (:any) %11: any
// CHECK-NEXT:  %40 = CondBranchInst %39: any, %BB14, %BB15
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %41 = LoadStackInst (:any) %13: any
// CHECK-NEXT:  %42 = StorePropertyLooseInst %41: any, globalObject: object, "a": string
// CHECK-NEXT:  %43 = BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %44 = TryEndInst
// CHECK-NEXT:  %45 = BranchInst %BB13
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %46 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %47 = IteratorNextInst (:any) %6: any, %46: any
// CHECK-NEXT:  %48 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %49 = BinaryStrictlyEqualInst (:any) %48: any, undefined: undefined
// CHECK-NEXT:  %50 = StoreStackInst %49: any, %11: any
// CHECK-NEXT:  %51 = CondBranchInst %49: any, %BB17, %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %52 = StoreStackInst %47: any, %13: any
// CHECK-NEXT:  %53 = BranchInst %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %54 = LoadStackInst (:any) %13: any
// CHECK-NEXT:  %55 = BinaryStrictlyNotEqualInst (:any) %54: any, undefined: undefined
// CHECK-NEXT:  %56 = CondBranchInst %55: any, %BB19, %BB17
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %57 = AllocArrayInst (:object) 2: number
// CHECK-NEXT:  %58 = StoreOwnPropertyInst 2: number, %57: object, 1: number, true: boolean
// CHECK-NEXT:  %59 = StoreStackInst %57: object, %13: any
// CHECK-NEXT:  %60 = BranchInst %BB19
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %61 = TryStartInst %BB20, %BB21
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %62 = CatchInst (:any)
// CHECK-NEXT:  %63 = StoreStackInst %62: any, %14: any
// CHECK-NEXT:  %64 = BranchInst %BB3
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %65 = LoadStackInst (:any) %11: any
// CHECK-NEXT:  %66 = CondBranchInst %65: any, %BB23, %BB24
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %67 = LoadStackInst (:any) %13: any
// CHECK-NEXT:  %68 = AllocStackInst (:any) $?anon_6_iter: any
// CHECK-NEXT:  %69 = AllocStackInst (:any) $?anon_7_sourceOrNext: any
// CHECK-NEXT:  %70 = StoreStackInst %67: any, %69: any
// CHECK-NEXT:  %71 = IteratorBeginInst (:any) %69: any
// CHECK-NEXT:  %72 = StoreStackInst %71: any, %68: any
// CHECK-NEXT:  %73 = AllocStackInst (:any) $?anon_8_iterDone: any
// CHECK-NEXT:  %74 = StoreStackInst undefined: undefined, %73: any
// CHECK-NEXT:  %75 = AllocStackInst (:any) $?anon_9_iterValue: any
// CHECK-NEXT:  %76 = AllocStackInst (:any) $?anon_10_exc: any
// CHECK-NEXT:  %77 = TryStartInst %BB25, %BB26
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %78 = LoadStackInst (:any) %73: any
// CHECK-NEXT:  %79 = CondBranchInst %78: any, %BB28, %BB29
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %80 = CatchInst (:any)
// CHECK-NEXT:  %81 = StoreStackInst %80: any, %76: any
// CHECK-NEXT:  %82 = BranchInst %BB27
// CHECK-NEXT:%BB30:
// CHECK-NEXT:  %83 = StoreStackInst undefined: undefined, %75: any
// CHECK-NEXT:  %84 = BranchInst %BB31
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %85 = BranchInst %BB32
// CHECK-NEXT:%BB32:
// CHECK-NEXT:  %86 = TryEndInst
// CHECK-NEXT:  %87 = BranchInst %BB30
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %88 = LoadStackInst (:any) %69: any
// CHECK-NEXT:  %89 = IteratorNextInst (:any) %68: any, %88: any
// CHECK-NEXT:  %90 = LoadStackInst (:any) %68: any
// CHECK-NEXT:  %91 = BinaryStrictlyEqualInst (:any) %90: any, undefined: undefined
// CHECK-NEXT:  %92 = StoreStackInst %91: any, %73: any
// CHECK-NEXT:  %93 = CondBranchInst %91: any, %BB33, %BB34
// CHECK-NEXT:%BB34:
// CHECK-NEXT:  %94 = StoreStackInst %89: any, %75: any
// CHECK-NEXT:  %95 = BranchInst %BB35
// CHECK-NEXT:%BB35:
// CHECK-NEXT:  %96 = LoadStackInst (:any) %75: any
// CHECK-NEXT:  %97 = BinaryStrictlyNotEqualInst (:any) %96: any, undefined: undefined
// CHECK-NEXT:  %98 = CondBranchInst %97: any, %BB36, %BB33
// CHECK-NEXT:%BB33:
// CHECK-NEXT:  %99 = StoreStackInst 1: number, %75: any
// CHECK-NEXT:  %100 = BranchInst %BB36
// CHECK-NEXT:%BB36:
// CHECK-NEXT:  %101 = TryStartInst %BB37, %BB38
// CHECK-NEXT:%BB37:
// CHECK-NEXT:  %102 = CatchInst (:any)
// CHECK-NEXT:  %103 = StoreStackInst %102: any, %76: any
// CHECK-NEXT:  %104 = BranchInst %BB27
// CHECK-NEXT:%BB39:
// CHECK-NEXT:  %105 = StoreStackInst undefined: undefined, %75: any
// CHECK-NEXT:  %106 = LoadStackInst (:any) %73: any
// CHECK-NEXT:  %107 = CondBranchInst %106: any, %BB40, %BB41
// CHECK-NEXT:%BB38:
// CHECK-NEXT:  %108 = LoadStackInst (:any) %75: any
// CHECK-NEXT:  %109 = StorePropertyLooseInst %108: any, globalObject: object, "b": string
// CHECK-NEXT:  %110 = BranchInst %BB42
// CHECK-NEXT:%BB42:
// CHECK-NEXT:  %111 = TryEndInst
// CHECK-NEXT:  %112 = BranchInst %BB39
// CHECK-NEXT:%BB41:
// CHECK-NEXT:  %113 = LoadStackInst (:any) %69: any
// CHECK-NEXT:  %114 = IteratorNextInst (:any) %68: any, %113: any
// CHECK-NEXT:  %115 = LoadStackInst (:any) %68: any
// CHECK-NEXT:  %116 = BinaryStrictlyEqualInst (:any) %115: any, undefined: undefined
// CHECK-NEXT:  %117 = StoreStackInst %116: any, %73: any
// CHECK-NEXT:  %118 = CondBranchInst %116: any, %BB40, %BB43
// CHECK-NEXT:%BB43:
// CHECK-NEXT:  %119 = StoreStackInst %114: any, %75: any
// CHECK-NEXT:  %120 = BranchInst %BB40
// CHECK-NEXT:%BB40:
// CHECK-NEXT:  %121 = TryStartInst %BB44, %BB45
// CHECK-NEXT:%BB44:
// CHECK-NEXT:  %122 = CatchInst (:any)
// CHECK-NEXT:  %123 = StoreStackInst %122: any, %76: any
// CHECK-NEXT:  %124 = BranchInst %BB27
// CHECK-NEXT:%BB46:
// CHECK-NEXT:  %125 = LoadStackInst (:any) %73: any
// CHECK-NEXT:  %126 = CondBranchInst %125: any, %BB47, %BB48
// CHECK-NEXT:%BB45:
// CHECK-NEXT:  %127 = LoadStackInst (:any) %75: any
// CHECK-NEXT:  %128 = StorePropertyLooseInst %127: any, globalObject: object, "c": string
// CHECK-NEXT:  %129 = BranchInst %BB49
// CHECK-NEXT:%BB49:
// CHECK-NEXT:  %130 = TryEndInst
// CHECK-NEXT:  %131 = BranchInst %BB46
// CHECK-NEXT:%BB48:
// CHECK-NEXT:  %132 = LoadStackInst (:any) %68: any
// CHECK-NEXT:  %133 = IteratorCloseInst (:any) %132: any, false: boolean
// CHECK-NEXT:  %134 = BranchInst %BB47
// CHECK-NEXT:%BB47:
// CHECK-NEXT:  %135 = BranchInst %BB50
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %136 = LoadStackInst (:any) %68: any
// CHECK-NEXT:  %137 = IteratorCloseInst (:any) %136: any, true: boolean
// CHECK-NEXT:  %138 = BranchInst %BB28
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %139 = LoadStackInst (:any) %76: any
// CHECK-NEXT:  %140 = ThrowInst %139: any
// CHECK-NEXT:%BB50:
// CHECK-NEXT:  %141 = TryEndInst
// CHECK-NEXT:  %142 = BranchInst %BB22
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %143 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %144 = IteratorCloseInst (:any) %143: any, false: boolean
// CHECK-NEXT:  %145 = BranchInst %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %146 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %147 = ReturnInst (:any) %146: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %148 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %149 = IteratorCloseInst (:any) %148: any, true: boolean
// CHECK-NEXT:  %150 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %151 = LoadStackInst (:any) %14: any
// CHECK-NEXT:  %152 = ThrowInst %151: any
// CHECK-NEXT:function_end
