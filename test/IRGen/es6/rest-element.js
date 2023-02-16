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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "f1": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "f2": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "f3": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "f4": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %f1(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: closure, globalObject: object, "f1": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:closure) %f2(): any
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: closure, globalObject: object, "f2": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:closure) %f3(): any
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8: closure, globalObject: object, "f3": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:closure) %f4(): any
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: closure, globalObject: object, "f4": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %13 = StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:  %15 = ReturnInst (:any) %14: any
// CHECK-NEXT:function_end

// CHECK:function f1(t: any): any
// CHECK-NEXT:frame = [t: any, a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %t: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [t]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [t]: any
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:  %6 = StoreStackInst %3: any, %5: any
// CHECK-NEXT:  %7 = IteratorBeginInst (:any) %5: any
// CHECK-NEXT:  %8 = StoreStackInst %7: any, %4: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_2_iterDone: any
// CHECK-NEXT:  %10 = StoreStackInst undefined: undefined, %9: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_3_iterValue: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_4_exc: any
// CHECK-NEXT:  %13 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_5_n: any
// CHECK-NEXT:  %15 = StoreStackInst 0: number, %14: any
// CHECK-NEXT:  %16 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %17 = CondBranchInst %16: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %19 = CondBranchInst %18: any, %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %20 = IteratorNextInst (:any) %4: any, %5: any
// CHECK-NEXT:  %21 = LoadStackInst (:any) %4: any
// CHECK-NEXT:  %22 = BinaryStrictlyEqualInst (:any) %21: any, undefined: undefined
// CHECK-NEXT:  %23 = StoreStackInst %22: any, %9: any
// CHECK-NEXT:  %24 = CondBranchInst %22: any, %BB1, %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %25 = LoadStackInst (:number) %14: any
// CHECK-NEXT:  %26 = TryStartInst %BB7, %BB8
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %27 = StoreFrameInst %13: object, [a]: any
// CHECK-NEXT:  %28 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %29 = CatchInst (:any)
// CHECK-NEXT:  %30 = StoreStackInst %29: any, %12: any
// CHECK-NEXT:  %31 = BranchInst %BB3
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %32 = BinaryAddInst (:number) %25: number, 1: number
// CHECK-NEXT:  %33 = StoreStackInst %32: number, %14: any
// CHECK-NEXT:  %34 = BranchInst %BB2
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %35 = StorePropertyLooseInst %20: any, %13: object, %25: number
// CHECK-NEXT:  %36 = BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %37 = TryEndInst
// CHECK-NEXT:  %38 = BranchInst %BB9
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %39 = IteratorCloseInst (:any) %4: any, true: boolean
// CHECK-NEXT:  %40 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %41 = LoadStackInst (:any) %12: any
// CHECK-NEXT:  %42 = ThrowInst %41: any
// CHECK-NEXT:function_end

// CHECK:function f2(t: any): any
// CHECK-NEXT:frame = [t: any, b: any, c: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %t: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [t]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [b]: any
// CHECK-NEXT:  %3 = StoreFrameInst undefined: undefined, [c]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [t]: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:  %7 = StoreStackInst %4: any, %6: any
// CHECK-NEXT:  %8 = IteratorBeginInst (:any) %6: any
// CHECK-NEXT:  %9 = StoreStackInst %8: any, %5: any
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_2_iterDone: any
// CHECK-NEXT:  %11 = StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_3_iterValue: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_4_exc: any
// CHECK-NEXT:  %14 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %15 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %16 = CondBranchInst %15: any, %BB4, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %17 = IteratorNextInst (:any) %5: any, %6: any
// CHECK-NEXT:  %18 = LoadStackInst (:any) %5: any
// CHECK-NEXT:  %19 = BinaryStrictlyEqualInst (:any) %18: any, undefined: undefined
// CHECK-NEXT:  %20 = StoreStackInst %19: any, %10: any
// CHECK-NEXT:  %21 = CondBranchInst %19: any, %BB7, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %22 = LoadStackInst (:number) %29: any
// CHECK-NEXT:  %23 = TryStartInst %BB9, %BB10
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %24 = TryStartInst %BB11, %BB12
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %25 = CatchInst (:any)
// CHECK-NEXT:  %26 = StoreStackInst %25: any, %13: any
// CHECK-NEXT:  %27 = BranchInst %BB3
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %28 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %29 = AllocStackInst (:any) $?anon_5_n: any
// CHECK-NEXT:  %30 = StoreStackInst 0: number, %29: any
// CHECK-NEXT:  %31 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %32 = CondBranchInst %31: any, %BB7, %BB6
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %33 = BranchInst %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %34 = TryEndInst
// CHECK-NEXT:  %35 = BranchInst %BB13
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %36 = CatchInst (:any)
// CHECK-NEXT:  %37 = StoreStackInst %36: any, %13: any
// CHECK-NEXT:  %38 = BranchInst %BB3
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %39 = BinaryAddInst (:number) %22: number, 1: number
// CHECK-NEXT:  %40 = StoreStackInst %39: number, %29: any
// CHECK-NEXT:  %41 = BranchInst %BB6
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %42 = StorePropertyLooseInst %17: any, %28: object, %22: number
// CHECK-NEXT:  %43 = BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %44 = TryEndInst
// CHECK-NEXT:  %45 = BranchInst %BB15
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %46 = CatchInst (:any)
// CHECK-NEXT:  %47 = StoreStackInst %46: any, %13: any
// CHECK-NEXT:  %48 = BranchInst %BB3
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %49 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %50 = AllocStackInst (:any) $?anon_6_iter: any
// CHECK-NEXT:  %51 = AllocStackInst (:any) $?anon_7_sourceOrNext: any
// CHECK-NEXT:  %52 = StoreStackInst %28: object, %51: any
// CHECK-NEXT:  %53 = IteratorBeginInst (:any) %51: any
// CHECK-NEXT:  %54 = StoreStackInst %53: any, %50: any
// CHECK-NEXT:  %55 = AllocStackInst (:any) $?anon_8_iterDone: any
// CHECK-NEXT:  %56 = StoreStackInst undefined: undefined, %55: any
// CHECK-NEXT:  %57 = AllocStackInst (:any) $?anon_9_iterValue: any
// CHECK-NEXT:  %58 = StoreStackInst undefined: undefined, %57: any
// CHECK-NEXT:  %59 = BranchInst %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %60 = IteratorNextInst (:any) %50: any, %51: any
// CHECK-NEXT:  %61 = LoadStackInst (:any) %50: any
// CHECK-NEXT:  %62 = BinaryStrictlyEqualInst (:any) %61: any, undefined: undefined
// CHECK-NEXT:  %63 = StoreStackInst %62: any, %55: any
// CHECK-NEXT:  %64 = CondBranchInst %62: any, %BB19, %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %65 = StoreStackInst %60: any, %57: any
// CHECK-NEXT:  %66 = BranchInst %BB19
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %67 = LoadStackInst (:any) %57: any
// CHECK-NEXT:  %68 = StoreFrameInst %67: any, [b]: any
// CHECK-NEXT:  %69 = StoreStackInst undefined: undefined, %57: any
// CHECK-NEXT:  %70 = LoadStackInst (:any) %55: any
// CHECK-NEXT:  %71 = CondBranchInst %70: any, %BB21, %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %72 = IteratorNextInst (:any) %50: any, %51: any
// CHECK-NEXT:  %73 = LoadStackInst (:any) %50: any
// CHECK-NEXT:  %74 = BinaryStrictlyEqualInst (:any) %73: any, undefined: undefined
// CHECK-NEXT:  %75 = StoreStackInst %74: any, %55: any
// CHECK-NEXT:  %76 = CondBranchInst %74: any, %BB21, %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %77 = StoreStackInst %72: any, %57: any
// CHECK-NEXT:  %78 = BranchInst %BB21
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %79 = LoadStackInst (:any) %57: any
// CHECK-NEXT:  %80 = StoreFrameInst %79: any, [c]: any
// CHECK-NEXT:  %81 = LoadStackInst (:any) %55: any
// CHECK-NEXT:  %82 = CondBranchInst %81: any, %BB24, %BB25
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %83 = IteratorCloseInst (:any) %50: any, false: boolean
// CHECK-NEXT:  %84 = BranchInst %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %85 = BranchInst %BB26
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %86 = TryEndInst
// CHECK-NEXT:  %87 = BranchInst %BB17
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %88 = IteratorCloseInst (:any) %5: any, true: boolean
// CHECK-NEXT:  %89 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %90 = LoadStackInst (:any) %13: any
// CHECK-NEXT:  %91 = ThrowInst %90: any
// CHECK-NEXT:function_end

// CHECK:function f3(t: any): any
// CHECK-NEXT:frame = [t: any, d: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %t: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [t]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [d]: any
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_2_idx: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_3_size: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [t]: any
// CHECK-NEXT:  %8 = StoreStackInst %7: any, %4: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:  %10 = GetPNamesInst %3: any, %4: any, %5: any, %6: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = GetNextPNameInst %9: any, %4: any, %5: any, %6: any, %3: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_5_iter: any
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_6_sourceOrNext: any
// CHECK-NEXT:  %16 = StoreStackInst %13: any, %15: any
// CHECK-NEXT:  %17 = IteratorBeginInst (:any) %15: any
// CHECK-NEXT:  %18 = StoreStackInst %17: any, %14: any
// CHECK-NEXT:  %19 = AllocStackInst (:any) $?anon_7_iterDone: any
// CHECK-NEXT:  %20 = StoreStackInst undefined: undefined, %19: any
// CHECK-NEXT:  %21 = AllocStackInst (:any) $?anon_8_iterValue: any
// CHECK-NEXT:  %22 = AllocStackInst (:any) $?anon_9_exc: any
// CHECK-NEXT:  %23 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %24 = AllocStackInst (:any) $?anon_10_n: any
// CHECK-NEXT:  %25 = StoreStackInst 0: number, %24: any
// CHECK-NEXT:  %26 = LoadStackInst (:any) %19: any
// CHECK-NEXT:  %27 = CondBranchInst %26: any, %BB4, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %28 = LoadStackInst (:any) %19: any
// CHECK-NEXT:  %29 = CondBranchInst %28: any, %BB7, %BB8
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %30 = IteratorNextInst (:any) %14: any, %15: any
// CHECK-NEXT:  %31 = LoadStackInst (:any) %14: any
// CHECK-NEXT:  %32 = BinaryStrictlyEqualInst (:any) %31: any, undefined: undefined
// CHECK-NEXT:  %33 = StoreStackInst %32: any, %19: any
// CHECK-NEXT:  %34 = CondBranchInst %32: any, %BB4, %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %35 = LoadStackInst (:number) %24: any
// CHECK-NEXT:  %36 = TryStartInst %BB10, %BB11
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %37 = StoreFrameInst %23: object, [d]: any
// CHECK-NEXT:  %38 = BranchInst %BB2
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %39 = CatchInst (:any)
// CHECK-NEXT:  %40 = StoreStackInst %39: any, %22: any
// CHECK-NEXT:  %41 = BranchInst %BB6
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %42 = BinaryAddInst (:number) %35: number, 1: number
// CHECK-NEXT:  %43 = StoreStackInst %42: number, %24: any
// CHECK-NEXT:  %44 = BranchInst %BB5
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %45 = StorePropertyLooseInst %30: any, %23: object, %35: number
// CHECK-NEXT:  %46 = BranchInst %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %47 = TryEndInst
// CHECK-NEXT:  %48 = BranchInst %BB12
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %49 = IteratorCloseInst (:any) %14: any, true: boolean
// CHECK-NEXT:  %50 = BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %51 = LoadStackInst (:any) %22: any
// CHECK-NEXT:  %52 = ThrowInst %51: any
// CHECK-NEXT:function_end

// CHECK:function f4(t: any): any
// CHECK-NEXT:frame = [t: any, a: any, b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %t: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [t]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:  %3 = StoreFrameInst undefined: undefined, [b]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [t]: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:  %7 = StoreStackInst %4: any, %6: any
// CHECK-NEXT:  %8 = IteratorBeginInst (:any) %6: any
// CHECK-NEXT:  %9 = StoreStackInst %8: any, %5: any
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_2_iterDone: any
// CHECK-NEXT:  %11 = StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_3_iterValue: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_4_exc: any
// CHECK-NEXT:  %14 = StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %15 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %16 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %17 = CondBranchInst %16: any, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %18 = IteratorNextInst (:any) %5: any, %6: any
// CHECK-NEXT:  %19 = LoadStackInst (:any) %5: any
// CHECK-NEXT:  %20 = BinaryStrictlyEqualInst (:any) %19: any, undefined: undefined
// CHECK-NEXT:  %21 = StoreStackInst %20: any, %10: any
// CHECK-NEXT:  %22 = CondBranchInst %20: any, %BB5, %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %23 = StoreStackInst %18: any, %12: any
// CHECK-NEXT:  %24 = BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %25 = LoadStackInst (:any) %12: any
// CHECK-NEXT:  %26 = StoreFrameInst %25: any, [a]: any
// CHECK-NEXT:  %27 = TryStartInst %BB7, %BB8
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %28 = IteratorNextInst (:any) %5: any, %6: any
// CHECK-NEXT:  %29 = LoadStackInst (:any) %5: any
// CHECK-NEXT:  %30 = BinaryStrictlyEqualInst (:any) %29: any, undefined: undefined
// CHECK-NEXT:  %31 = StoreStackInst %30: any, %10: any
// CHECK-NEXT:  %32 = CondBranchInst %30: any, %BB10, %BB11
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %33 = LoadStackInst (:number) %40: any
// CHECK-NEXT:  %34 = TryStartInst %BB12, %BB13
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %35 = TryStartInst %BB14, %BB15
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %36 = CatchInst (:any)
// CHECK-NEXT:  %37 = StoreStackInst %36: any, %13: any
// CHECK-NEXT:  %38 = BranchInst %BB2
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %39 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %40 = AllocStackInst (:any) $?anon_5_n: any
// CHECK-NEXT:  %41 = StoreStackInst 0: number, %40: any
// CHECK-NEXT:  %42 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %43 = CondBranchInst %42: any, %BB10, %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %44 = BranchInst %BB17
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %45 = TryEndInst
// CHECK-NEXT:  %46 = BranchInst %BB16
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %47 = CatchInst (:any)
// CHECK-NEXT:  %48 = StoreStackInst %47: any, %13: any
// CHECK-NEXT:  %49 = BranchInst %BB2
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %50 = BinaryAddInst (:number) %33: number, 1: number
// CHECK-NEXT:  %51 = StoreStackInst %50: number, %40: any
// CHECK-NEXT:  %52 = BranchInst %BB9
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %53 = StorePropertyLooseInst %28: any, %39: object, %33: number
// CHECK-NEXT:  %54 = BranchInst %BB19
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %55 = TryEndInst
// CHECK-NEXT:  %56 = BranchInst %BB18
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %57 = CatchInst (:any)
// CHECK-NEXT:  %58 = StoreStackInst %57: any, %13: any
// CHECK-NEXT:  %59 = BranchInst %BB2
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %60 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %61 = AllocStackInst (:any) $?anon_6_iter: any
// CHECK-NEXT:  %62 = AllocStackInst (:any) $?anon_7_sourceOrNext: any
// CHECK-NEXT:  %63 = StoreStackInst %39: object, %62: any
// CHECK-NEXT:  %64 = IteratorBeginInst (:any) %62: any
// CHECK-NEXT:  %65 = StoreStackInst %64: any, %61: any
// CHECK-NEXT:  %66 = AllocStackInst (:any) $?anon_8_iterDone: any
// CHECK-NEXT:  %67 = StoreStackInst undefined: undefined, %66: any
// CHECK-NEXT:  %68 = AllocStackInst (:any) $?anon_9_iterValue: any
// CHECK-NEXT:  %69 = AllocStackInst (:any) $?anon_10_exc: any
// CHECK-NEXT:  %70 = TryStartInst %BB21, %BB22
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %71 = LoadStackInst (:any) %66: any
// CHECK-NEXT:  %72 = CondBranchInst %71: any, %BB24, %BB25
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %73 = CatchInst (:any)
// CHECK-NEXT:  %74 = StoreStackInst %73: any, %69: any
// CHECK-NEXT:  %75 = BranchInst %BB23
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %76 = StoreStackInst undefined: undefined, %68: any
// CHECK-NEXT:  %77 = BranchInst %BB27
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %78 = LoadFrameInst (:any) [b]: any
// CHECK-NEXT:  %79 = BranchInst %BB28
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %80 = TryEndInst
// CHECK-NEXT:  %81 = BranchInst %BB26
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %82 = IteratorNextInst (:any) %61: any, %62: any
// CHECK-NEXT:  %83 = LoadStackInst (:any) %61: any
// CHECK-NEXT:  %84 = BinaryStrictlyEqualInst (:any) %83: any, undefined: undefined
// CHECK-NEXT:  %85 = StoreStackInst %84: any, %66: any
// CHECK-NEXT:  %86 = CondBranchInst %84: any, %BB29, %BB30
// CHECK-NEXT:%BB30:
// CHECK-NEXT:  %87 = StoreStackInst %82: any, %68: any
// CHECK-NEXT:  %88 = BranchInst %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %89 = TryStartInst %BB31, %BB32
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %90 = CatchInst (:any)
// CHECK-NEXT:  %91 = StoreStackInst %90: any, %69: any
// CHECK-NEXT:  %92 = BranchInst %BB23
// CHECK-NEXT:%BB33:
// CHECK-NEXT:  %93 = LoadStackInst (:any) %66: any
// CHECK-NEXT:  %94 = CondBranchInst %93: any, %BB34, %BB35
// CHECK-NEXT:%BB32:
// CHECK-NEXT:  %95 = LoadStackInst (:any) %68: any
// CHECK-NEXT:  %96 = StorePropertyLooseInst %95: any, %78: any, 0: number
// CHECK-NEXT:  %97 = BranchInst %BB36
// CHECK-NEXT:%BB36:
// CHECK-NEXT:  %98 = TryEndInst
// CHECK-NEXT:  %99 = BranchInst %BB33
// CHECK-NEXT:%BB35:
// CHECK-NEXT:  %100 = IteratorCloseInst (:any) %61: any, false: boolean
// CHECK-NEXT:  %101 = BranchInst %BB34
// CHECK-NEXT:%BB34:
// CHECK-NEXT:  %102 = BranchInst %BB37
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %103 = IteratorCloseInst (:any) %61: any, true: boolean
// CHECK-NEXT:  %104 = BranchInst %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %105 = LoadStackInst (:any) %69: any
// CHECK-NEXT:  %106 = ThrowInst %105: any
// CHECK-NEXT:%BB37:
// CHECK-NEXT:  %107 = TryEndInst
// CHECK-NEXT:  %108 = BranchInst %BB20
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %109 = IteratorCloseInst (:any) %5: any, true: boolean
// CHECK-NEXT:  %110 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %111 = LoadStackInst (:any) %13: any
// CHECK-NEXT:  %112 = ThrowInst %111: any
// CHECK-NEXT:function_end
