/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function f1(t) {
    var [...a] = t;
}

function f2(t) {
    var [...[b, c]] = t;
}

function f3(t) {
    for(var[...d] in t);
}

function f4(t) {
    var a, b;
    [a, ...[b[0]]] = t;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "f1": string
// CHECK-NEXT:       DeclareGlobalVarInst "f2": string
// CHECK-NEXT:       DeclareGlobalVarInst "f3": string
// CHECK-NEXT:       DeclareGlobalVarInst "f4": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %f1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "f1": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %f2(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "f2": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %f3(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "f3": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %f4(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "f4": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function f1(t: any): any
// CHECK-NEXT:frame = [t: any, a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %0: any, [t]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [t]: any
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:       StoreStackInst %3: any, %5: any
// CHECK-NEXT:  %7 = IteratorBeginInst (:any) %5: any
// CHECK-NEXT:       StoreStackInst %7: any, %4: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_2_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %9: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_3_iterValue: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_4_exc: any
// CHECK-NEXT:  %13 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %14 = AllocStackInst (:number) $?anon_5_n: any
// CHECK-NEXT:        StoreStackInst 0: number, %14: number
// CHECK-NEXT:  %16 = LoadStackInst (:any) %9: any
// CHECK-NEXT:        CondBranchInst %16: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = LoadStackInst (:any) %9: any
// CHECK-NEXT:        CondBranchInst %18: any, %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %20 = LoadStackInst (:any) %5: any
// CHECK-NEXT:  %21 = IteratorNextInst (:any) %4: any, %20: any
// CHECK-NEXT:  %22 = LoadStackInst (:any) %4: any
// CHECK-NEXT:  %23 = BinaryStrictlyEqualInst (:any) %22: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %23: any, %9: any
// CHECK-NEXT:        CondBranchInst %23: any, %BB1, %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %26 = LoadStackInst (:number) %14: number
// CHECK-NEXT:        TryStartInst %BB7, %BB8
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        StoreFrameInst %13: object, [a]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %30 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %30: any, %12: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %33 = BinaryAddInst (:number) %26: number, 1: number
// CHECK-NEXT:        StoreStackInst %33: number, %14: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        StorePropertyLooseInst %21: any, %13: object, %26: number
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %40 = LoadStackInst (:any) %4: any
// CHECK-NEXT:  %41 = IteratorCloseInst (:any) %40: any, true: boolean
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %43 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        ThrowInst %43: any
// CHECK-NEXT:function_end

// CHECK:function f2(t: any): any
// CHECK-NEXT:frame = [t: any, b: any, c: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %0: any, [t]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [b]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [c]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [t]: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:       StoreStackInst %4: any, %6: any
// CHECK-NEXT:  %8 = IteratorBeginInst (:any) %6: any
// CHECK-NEXT:       StoreStackInst %8: any, %5: any
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_2_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_3_iterValue: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_4_exc: any
// CHECK-NEXT:        TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %15 = LoadStackInst (:any) %10: any
// CHECK-NEXT:        CondBranchInst %15: any, %BB4, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %17 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %18 = IteratorNextInst (:any) %5: any, %17: any
// CHECK-NEXT:  %19 = LoadStackInst (:any) %5: any
// CHECK-NEXT:  %20 = BinaryStrictlyEqualInst (:any) %19: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %20: any, %10: any
// CHECK-NEXT:        CondBranchInst %20: any, %BB7, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %23 = LoadStackInst (:number) %30: number
// CHECK-NEXT:        TryStartInst %BB9, %BB10
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        TryStartInst %BB11, %BB12
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %26 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %26: any, %13: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %29 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %30 = AllocStackInst (:number) $?anon_5_n: any
// CHECK-NEXT:        StoreStackInst 0: number, %30: number
// CHECK-NEXT:  %32 = LoadStackInst (:any) %10: any
// CHECK-NEXT:        CondBranchInst %32: any, %BB7, %BB6
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        BranchInst %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB13
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %37 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %37: any, %13: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %40 = BinaryAddInst (:number) %23: number, 1: number
// CHECK-NEXT:        StoreStackInst %40: number, %30: number
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        StorePropertyLooseInst %18: any, %29: object, %23: number
// CHECK-NEXT:        BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB15
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %47 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %47: any, %13: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %51 = AllocStackInst (:any) $?anon_6_iter: any
// CHECK-NEXT:  %52 = AllocStackInst (:any) $?anon_7_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %29: object, %52: any
// CHECK-NEXT:  %54 = IteratorBeginInst (:any) %52: any
// CHECK-NEXT:        StoreStackInst %54: any, %51: any
// CHECK-NEXT:  %56 = AllocStackInst (:any) $?anon_8_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %56: any
// CHECK-NEXT:  %58 = AllocStackInst (:any) $?anon_9_iterValue: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %58: any
// CHECK-NEXT:        BranchInst %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %61 = LoadStackInst (:any) %52: any
// CHECK-NEXT:  %62 = IteratorNextInst (:any) %51: any, %61: any
// CHECK-NEXT:  %63 = LoadStackInst (:any) %51: any
// CHECK-NEXT:  %64 = BinaryStrictlyEqualInst (:any) %63: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %64: any, %56: any
// CHECK-NEXT:        CondBranchInst %64: any, %BB19, %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:        StoreStackInst %62: any, %58: any
// CHECK-NEXT:        BranchInst %BB19
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %69 = LoadStackInst (:any) %58: any
// CHECK-NEXT:        StoreFrameInst %69: any, [b]: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %58: any
// CHECK-NEXT:  %72 = LoadStackInst (:any) %56: any
// CHECK-NEXT:        CondBranchInst %72: any, %BB21, %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %74 = LoadStackInst (:any) %52: any
// CHECK-NEXT:  %75 = IteratorNextInst (:any) %51: any, %74: any
// CHECK-NEXT:  %76 = LoadStackInst (:any) %51: any
// CHECK-NEXT:  %77 = BinaryStrictlyEqualInst (:any) %76: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %77: any, %56: any
// CHECK-NEXT:        CondBranchInst %77: any, %BB21, %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:        StoreStackInst %75: any, %58: any
// CHECK-NEXT:        BranchInst %BB21
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %82 = LoadStackInst (:any) %58: any
// CHECK-NEXT:        StoreFrameInst %82: any, [c]: any
// CHECK-NEXT:  %84 = LoadStackInst (:any) %56: any
// CHECK-NEXT:        CondBranchInst %84: any, %BB24, %BB25
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %86 = LoadStackInst (:any) %51: any
// CHECK-NEXT:  %87 = IteratorCloseInst (:any) %86: any, false: boolean
// CHECK-NEXT:        BranchInst %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:        BranchInst %BB26
// CHECK-NEXT:%BB26:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB17
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %92 = LoadStackInst (:any) %5: any
// CHECK-NEXT:  %93 = IteratorCloseInst (:any) %92: any, true: boolean
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %95 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ThrowInst %95: any
// CHECK-NEXT:function_end

// CHECK:function f3(t: any): any
// CHECK-NEXT:frame = [t: any, d: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %0: any, [t]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [d]: any
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %5 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %6 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [t]: any
// CHECK-NEXT:       StoreStackInst %7: any, %4: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %3: any, %4: any, %5: number, %6: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %9: any, %4: any, %5: number, %6: number, %3: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_5_iter: any
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_6_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %13: any, %15: any
// CHECK-NEXT:  %17 = IteratorBeginInst (:any) %15: any
// CHECK-NEXT:        StoreStackInst %17: any, %14: any
// CHECK-NEXT:  %19 = AllocStackInst (:any) $?anon_7_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %19: any
// CHECK-NEXT:  %21 = AllocStackInst (:any) $?anon_8_iterValue: any
// CHECK-NEXT:  %22 = AllocStackInst (:any) $?anon_9_exc: any
// CHECK-NEXT:  %23 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %24 = AllocStackInst (:number) $?anon_10_n: any
// CHECK-NEXT:        StoreStackInst 0: number, %24: number
// CHECK-NEXT:  %26 = LoadStackInst (:any) %19: any
// CHECK-NEXT:        CondBranchInst %26: any, %BB4, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %28 = LoadStackInst (:any) %19: any
// CHECK-NEXT:        CondBranchInst %28: any, %BB7, %BB8
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %30 = LoadStackInst (:any) %15: any
// CHECK-NEXT:  %31 = IteratorNextInst (:any) %14: any, %30: any
// CHECK-NEXT:  %32 = LoadStackInst (:any) %14: any
// CHECK-NEXT:  %33 = BinaryStrictlyEqualInst (:any) %32: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %33: any, %19: any
// CHECK-NEXT:        CondBranchInst %33: any, %BB4, %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %36 = LoadStackInst (:number) %24: number
// CHECK-NEXT:        TryStartInst %BB10, %BB11
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        StoreFrameInst %23: object, [d]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %40 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %40: any, %22: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %43 = BinaryAddInst (:number) %36: number, 1: number
// CHECK-NEXT:        StoreStackInst %43: number, %24: number
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        StorePropertyLooseInst %31: any, %23: object, %36: number
// CHECK-NEXT:        BranchInst %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %50 = LoadStackInst (:any) %14: any
// CHECK-NEXT:  %51 = IteratorCloseInst (:any) %50: any, true: boolean
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %53 = LoadStackInst (:any) %22: any
// CHECK-NEXT:        ThrowInst %53: any
// CHECK-NEXT:function_end

// CHECK:function f4(t: any): any
// CHECK-NEXT:frame = [t: any, a: any, b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %0: any, [t]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [b]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [t]: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:       StoreStackInst %4: any, %6: any
// CHECK-NEXT:  %8 = IteratorBeginInst (:any) %6: any
// CHECK-NEXT:       StoreStackInst %8: any, %5: any
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_2_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_3_iterValue: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_4_exc: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %16 = LoadStackInst (:any) %10: any
// CHECK-NEXT:        CondBranchInst %16: any, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %18 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %19 = IteratorNextInst (:any) %5: any, %18: any
// CHECK-NEXT:  %20 = LoadStackInst (:any) %5: any
// CHECK-NEXT:  %21 = BinaryStrictlyEqualInst (:any) %20: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %21: any, %10: any
// CHECK-NEXT:        CondBranchInst %21: any, %BB5, %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        StoreStackInst %19: any, %12: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %26 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        StoreFrameInst %26: any, [a]: any
// CHECK-NEXT:        TryStartInst %BB7, %BB8
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %29 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %30 = IteratorNextInst (:any) %5: any, %29: any
// CHECK-NEXT:  %31 = LoadStackInst (:any) %5: any
// CHECK-NEXT:  %32 = BinaryStrictlyEqualInst (:any) %31: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %32: any, %10: any
// CHECK-NEXT:        CondBranchInst %32: any, %BB10, %BB11
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %35 = LoadStackInst (:number) %42: number
// CHECK-NEXT:        TryStartInst %BB12, %BB13
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        TryStartInst %BB14, %BB15
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %38 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %38: any, %13: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %41 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %42 = AllocStackInst (:number) $?anon_5_n: any
// CHECK-NEXT:        StoreStackInst 0: number, %42: number
// CHECK-NEXT:  %44 = LoadStackInst (:any) %10: any
// CHECK-NEXT:        CondBranchInst %44: any, %BB10, %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        BranchInst %BB17
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB16
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %49 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %49: any, %13: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %52 = BinaryAddInst (:number) %35: number, 1: number
// CHECK-NEXT:        StoreStackInst %52: number, %42: number
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        StorePropertyLooseInst %30: any, %41: object, %35: number
// CHECK-NEXT:        BranchInst %BB19
// CHECK-NEXT:%BB19:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB18
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %59 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %59: any, %13: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB20:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %63 = AllocStackInst (:any) $?anon_6_iter: any
// CHECK-NEXT:  %64 = AllocStackInst (:any) $?anon_7_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %41: object, %64: any
// CHECK-NEXT:  %66 = IteratorBeginInst (:any) %64: any
// CHECK-NEXT:        StoreStackInst %66: any, %63: any
// CHECK-NEXT:  %68 = AllocStackInst (:any) $?anon_8_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %68: any
// CHECK-NEXT:  %70 = AllocStackInst (:any) $?anon_9_iterValue: any
// CHECK-NEXT:  %71 = AllocStackInst (:any) $?anon_10_exc: any
// CHECK-NEXT:        TryStartInst %BB21, %BB22
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %73 = LoadStackInst (:any) %68: any
// CHECK-NEXT:        CondBranchInst %73: any, %BB24, %BB25
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %75 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %75: any, %71: any
// CHECK-NEXT:        BranchInst %BB23
// CHECK-NEXT:%BB26:
// CHECK-NEXT:        StoreStackInst undefined: undefined, %70: any
// CHECK-NEXT:        BranchInst %BB27
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %80 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:        BranchInst %BB28
// CHECK-NEXT:%BB28:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB26
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %84 = LoadStackInst (:any) %64: any
// CHECK-NEXT:  %85 = IteratorNextInst (:any) %63: any, %84: any
// CHECK-NEXT:  %86 = LoadStackInst (:any) %63: any
// CHECK-NEXT:  %87 = BinaryStrictlyEqualInst (:any) %86: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %87: any, %68: any
// CHECK-NEXT:        CondBranchInst %87: any, %BB29, %BB30
// CHECK-NEXT:%BB30:
// CHECK-NEXT:        StoreStackInst %85: any, %70: any
// CHECK-NEXT:        BranchInst %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:        TryStartInst %BB31, %BB32
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %93 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %93: any, %71: any
// CHECK-NEXT:        BranchInst %BB23
// CHECK-NEXT:%BB33:
// CHECK-NEXT:  %96 = LoadStackInst (:any) %68: any
// CHECK-NEXT:        CondBranchInst %96: any, %BB34, %BB35
// CHECK-NEXT:%BB32:
// CHECK-NEXT:  %98 = LoadStackInst (:any) %70: any
// CHECK-NEXT:        StorePropertyLooseInst %98: any, %80: any, 0: number
// CHECK-NEXT:         BranchInst %BB36
// CHECK-NEXT:%BB36:
// CHECK-NEXT:         TryEndInst
// CHECK-NEXT:         BranchInst %BB33
// CHECK-NEXT:%BB35:
// CHECK-NEXT:  %103 = LoadStackInst (:any) %63: any
// CHECK-NEXT:  %104 = IteratorCloseInst (:any) %103: any, false: boolean
// CHECK-NEXT:         BranchInst %BB34
// CHECK-NEXT:%BB34:
// CHECK-NEXT:         BranchInst %BB37
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %107 = LoadStackInst (:any) %63: any
// CHECK-NEXT:  %108 = IteratorCloseInst (:any) %107: any, true: boolean
// CHECK-NEXT:         BranchInst %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %110 = LoadStackInst (:any) %71: any
// CHECK-NEXT:         ThrowInst %110: any
// CHECK-NEXT:%BB37:
// CHECK-NEXT:         TryEndInst
// CHECK-NEXT:         BranchInst %BB20
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %114 = LoadStackInst (:any) %5: any
// CHECK-NEXT:  %115 = IteratorCloseInst (:any) %114: any, true: boolean
// CHECK-NEXT:         BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %117 = LoadStackInst (:any) %13: any
// CHECK-NEXT:         ThrowInst %117: any
// CHECK-NEXT:function_end
