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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "f1": string
// CHECK-NEXT:       DeclareGlobalVarInst "f2": string
// CHECK-NEXT:       DeclareGlobalVarInst "f3": string
// CHECK-NEXT:       DeclareGlobalVarInst "f4": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %f1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "f1": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %f2(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "f2": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %f3(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "f3": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %f4(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "f4": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:function f1(t: any): any
// CHECK-NEXT:frame = [t: any, a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f1(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [t]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [a]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %1: environment, [t]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:       StoreStackInst %5: any, %7: any
// CHECK-NEXT:  %9 = IteratorBeginInst (:any) %7: any
// CHECK-NEXT:        StoreStackInst %9: any, %6: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_2_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %11: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_3_iterValue: any
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_4_exc: any
// CHECK-NEXT:  %15 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %16 = AllocStackInst (:number) $?anon_5_n: any
// CHECK-NEXT:        StoreStackInst 0: number, %16: number
// CHECK-NEXT:  %18 = LoadStackInst (:any) %11: any
// CHECK-NEXT:        CondBranchInst %18: any, %BB4, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %20 = LoadStackInst (:any) %11: any
// CHECK-NEXT:        CondBranchInst %20: any, %BB10, %BB9
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %22 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %23 = IteratorNextInst (:any) %6: any, %22: any
// CHECK-NEXT:  %24 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %25 = BinaryStrictlyEqualInst (:any) %24: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %25: any, %11: any
// CHECK-NEXT:        CondBranchInst %25: any, %BB4, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %28 = LoadStackInst (:number) %16: number
// CHECK-NEXT:        TryStartInst %BB5, %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: object, [a]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %32 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %32: any, %14: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %35 = BinaryAddInst (:number) %28: number, 1: number
// CHECK-NEXT:        StoreStackInst %35: number, %16: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        StorePropertyLooseInst %23: any, %15: object, %28: number
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %42 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %43 = IteratorCloseInst (:any) %42: any, true: boolean
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %45 = LoadStackInst (:any) %14: any
// CHECK-NEXT:        ThrowInst %45: any
// CHECK-NEXT:function_end

// CHECK:function f2(t: any): any
// CHECK-NEXT:frame = [t: any, b: any, c: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [t]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [b]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [c]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [t]: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:       StoreStackInst %6: any, %8: any
// CHECK-NEXT:  %10 = IteratorBeginInst (:any) %8: any
// CHECK-NEXT:        StoreStackInst %10: any, %7: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_2_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_3_iterValue: any
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_4_exc: any
// CHECK-NEXT:        TryStartInst %BB5, %BB7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %17 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        CondBranchInst %17: any, %BB26, %BB25
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %19 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %20 = IteratorNextInst (:any) %7: any, %19: any
// CHECK-NEXT:  %21 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %22 = BinaryStrictlyEqualInst (:any) %21: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %22: any, %12: any
// CHECK-NEXT:        CondBranchInst %22: any, %BB4, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %25 = LoadStackInst (:number) %32: number
// CHECK-NEXT:        TryStartInst %BB9, %BB11
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryStartInst %BB13, %BB15
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %28 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %28: any, %15: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %31 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %32 = AllocStackInst (:number) $?anon_5_n: any
// CHECK-NEXT:        StoreStackInst 0: number, %32: number
// CHECK-NEXT:  %34 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        CondBranchInst %34: any, %BB4, %BB2
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %39 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %39: any, %15: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %42 = BinaryAddInst (:number) %25: number, 1: number
// CHECK-NEXT:        StoreStackInst %42: number, %32: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        StorePropertyLooseInst %20: any, %31: object, %25: number
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %49 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %49: any, %15: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %53 = AllocStackInst (:any) $?anon_6_iter: any
// CHECK-NEXT:  %54 = AllocStackInst (:any) $?anon_7_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %31: object, %54: any
// CHECK-NEXT:  %56 = IteratorBeginInst (:any) %54: any
// CHECK-NEXT:        StoreStackInst %56: any, %53: any
// CHECK-NEXT:  %58 = AllocStackInst (:any) $?anon_8_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %58: any
// CHECK-NEXT:  %60 = AllocStackInst (:any) $?anon_9_iterValue: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %60: any
// CHECK-NEXT:        BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %63 = LoadStackInst (:any) %54: any
// CHECK-NEXT:  %64 = IteratorNextInst (:any) %53: any, %63: any
// CHECK-NEXT:  %65 = LoadStackInst (:any) %53: any
// CHECK-NEXT:  %66 = BinaryStrictlyEqualInst (:any) %65: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %66: any, %58: any
// CHECK-NEXT:        CondBranchInst %66: any, %BB18, %BB17
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        StoreStackInst %64: any, %60: any
// CHECK-NEXT:        BranchInst %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %71 = LoadStackInst (:any) %60: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %71: any, [b]: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %60: any
// CHECK-NEXT:  %74 = LoadStackInst (:any) %58: any
// CHECK-NEXT:        CondBranchInst %74: any, %BB21, %BB19
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %76 = LoadStackInst (:any) %54: any
// CHECK-NEXT:  %77 = IteratorNextInst (:any) %53: any, %76: any
// CHECK-NEXT:  %78 = LoadStackInst (:any) %53: any
// CHECK-NEXT:  %79 = BinaryStrictlyEqualInst (:any) %78: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %79: any, %58: any
// CHECK-NEXT:        CondBranchInst %79: any, %BB21, %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:        StoreStackInst %77: any, %60: any
// CHECK-NEXT:        BranchInst %BB21
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %84 = LoadStackInst (:any) %60: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %84: any, [c]: any
// CHECK-NEXT:  %86 = LoadStackInst (:any) %58: any
// CHECK-NEXT:        CondBranchInst %86: any, %BB23, %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %88 = LoadStackInst (:any) %53: any
// CHECK-NEXT:  %89 = IteratorCloseInst (:any) %88: any, false: boolean
// CHECK-NEXT:        BranchInst %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:        BranchInst %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB14
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %94 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %95 = IteratorCloseInst (:any) %94: any, true: boolean
// CHECK-NEXT:        BranchInst %BB26
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %97 = LoadStackInst (:any) %15: any
// CHECK-NEXT:        ThrowInst %97: any
// CHECK-NEXT:function_end

// CHECK:function f3(t: any): any
// CHECK-NEXT:frame = [t: any, d: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f3(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [t]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [d]: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %7 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %8 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [t]: any
// CHECK-NEXT:        StoreStackInst %9: any, %6: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %5: any, %6: any, %7: number, %8: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %11: any, %6: any, %7: number, %8: number, %5: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %15 = LoadStackInst (:any) %11: any
// CHECK-NEXT:  %16 = AllocStackInst (:any) $?anon_5_iter: any
// CHECK-NEXT:  %17 = AllocStackInst (:any) $?anon_6_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %15: any, %17: any
// CHECK-NEXT:  %19 = IteratorBeginInst (:any) %17: any
// CHECK-NEXT:        StoreStackInst %19: any, %16: any
// CHECK-NEXT:  %21 = AllocStackInst (:any) $?anon_7_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %21: any
// CHECK-NEXT:  %23 = AllocStackInst (:any) $?anon_8_iterValue: any
// CHECK-NEXT:  %24 = AllocStackInst (:any) $?anon_9_exc: any
// CHECK-NEXT:  %25 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %26 = AllocStackInst (:number) $?anon_10_n: any
// CHECK-NEXT:        StoreStackInst 0: number, %26: number
// CHECK-NEXT:  %28 = LoadStackInst (:any) %21: any
// CHECK-NEXT:        CondBranchInst %28: any, %BB7, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %30 = LoadStackInst (:any) %21: any
// CHECK-NEXT:        CondBranchInst %30: any, %BB13, %BB12
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %32 = LoadStackInst (:any) %17: any
// CHECK-NEXT:  %33 = IteratorNextInst (:any) %16: any, %32: any
// CHECK-NEXT:  %34 = LoadStackInst (:any) %16: any
// CHECK-NEXT:  %35 = BinaryStrictlyEqualInst (:any) %34: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %35: any, %21: any
// CHECK-NEXT:        CondBranchInst %35: any, %BB7, %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %38 = LoadStackInst (:number) %26: number
// CHECK-NEXT:        TryStartInst %BB8, %BB10
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        StoreFrameInst %1: environment, %25: object, [d]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %42 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %42: any, %24: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %45 = BinaryAddInst (:number) %38: number, 1: number
// CHECK-NEXT:        StoreStackInst %45: number, %26: number
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        StorePropertyLooseInst %33: any, %25: object, %38: number
// CHECK-NEXT:        BranchInst %BB11
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %52 = LoadStackInst (:any) %16: any
// CHECK-NEXT:  %53 = IteratorCloseInst (:any) %52: any, true: boolean
// CHECK-NEXT:        BranchInst %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %55 = LoadStackInst (:any) %24: any
// CHECK-NEXT:        ThrowInst %55: any
// CHECK-NEXT:function_end

// CHECK:function f4(t: any): any
// CHECK-NEXT:frame = [t: any, a: any, b: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f4(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [t]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [a]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [b]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [t]: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:       StoreStackInst %6: any, %8: any
// CHECK-NEXT:  %10 = IteratorBeginInst (:any) %8: any
// CHECK-NEXT:        StoreStackInst %10: any, %7: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_2_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_3_iterValue: any
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_4_exc: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %14: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %18 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        CondBranchInst %18: any, %BB37, %BB36
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %20 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %21 = IteratorNextInst (:any) %7: any, %20: any
// CHECK-NEXT:  %22 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %23 = BinaryStrictlyEqualInst (:any) %22: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %23: any, %12: any
// CHECK-NEXT:        CondBranchInst %23: any, %BB4, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        StoreStackInst %21: any, %14: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %28 = LoadStackInst (:any) %14: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %28: any, [a]: any
// CHECK-NEXT:        TryStartInst %BB8, %BB10
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %31 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %32 = IteratorNextInst (:any) %7: any, %31: any
// CHECK-NEXT:  %33 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %34 = BinaryStrictlyEqualInst (:any) %33: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %34: any, %12: any
// CHECK-NEXT:        CondBranchInst %34: any, %BB7, %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %37 = LoadStackInst (:number) %44: number
// CHECK-NEXT:        TryStartInst %BB12, %BB14
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        TryStartInst %BB16, %BB18
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %40 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %40: any, %15: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %43 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %44 = AllocStackInst (:number) $?anon_5_n: any
// CHECK-NEXT:        StoreStackInst 0: number, %44: number
// CHECK-NEXT:  %46 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        CondBranchInst %46: any, %BB7, %BB5
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        BranchInst %BB11
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %51 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %51: any, %15: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %54 = BinaryAddInst (:number) %37: number, 1: number
// CHECK-NEXT:        StoreStackInst %54: number, %44: number
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        StorePropertyLooseInst %32: any, %43: object, %37: number
// CHECK-NEXT:        BranchInst %BB15
// CHECK-NEXT:%BB15:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB13
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %61 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %61: any, %15: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %65 = AllocStackInst (:any) $?anon_6_iter: any
// CHECK-NEXT:  %66 = AllocStackInst (:any) $?anon_7_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %43: object, %66: any
// CHECK-NEXT:  %68 = IteratorBeginInst (:any) %66: any
// CHECK-NEXT:        StoreStackInst %68: any, %65: any
// CHECK-NEXT:  %70 = AllocStackInst (:any) $?anon_8_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %70: any
// CHECK-NEXT:  %72 = AllocStackInst (:any) $?anon_9_iterValue: any
// CHECK-NEXT:  %73 = AllocStackInst (:any) $?anon_10_exc: any
// CHECK-NEXT:        TryStartInst %BB20, %BB22
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %75 = LoadStackInst (:any) %70: any
// CHECK-NEXT:        CondBranchInst %75: any, %BB34, %BB33
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %77 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %77: any, %73: any
// CHECK-NEXT:        BranchInst %BB19
// CHECK-NEXT:%BB21:
// CHECK-NEXT:        StoreStackInst undefined: undefined, %72: any
// CHECK-NEXT:        BranchInst %BB24
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %82 = LoadFrameInst (:any) %1: environment, [b]: any
// CHECK-NEXT:        BranchInst %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB21
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %86 = LoadStackInst (:any) %66: any
// CHECK-NEXT:  %87 = IteratorNextInst (:any) %65: any, %86: any
// CHECK-NEXT:  %88 = LoadStackInst (:any) %65: any
// CHECK-NEXT:  %89 = BinaryStrictlyEqualInst (:any) %88: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %89: any, %70: any
// CHECK-NEXT:        CondBranchInst %89: any, %BB26, %BB25
// CHECK-NEXT:%BB25:
// CHECK-NEXT:        StoreStackInst %87: any, %72: any
// CHECK-NEXT:        BranchInst %BB26
// CHECK-NEXT:%BB26:
// CHECK-NEXT:        TryStartInst %BB27, %BB29
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %95 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %95: any, %73: any
// CHECK-NEXT:        BranchInst %BB19
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %98 = LoadStackInst (:any) %70: any
// CHECK-NEXT:        CondBranchInst %98: any, %BB32, %BB31
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %100 = LoadStackInst (:any) %72: any
// CHECK-NEXT:         StorePropertyLooseInst %100: any, %82: any, 0: number
// CHECK-NEXT:         BranchInst %BB30
// CHECK-NEXT:%BB30:
// CHECK-NEXT:         TryEndInst
// CHECK-NEXT:         BranchInst %BB28
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %105 = LoadStackInst (:any) %65: any
// CHECK-NEXT:  %106 = IteratorCloseInst (:any) %105: any, false: boolean
// CHECK-NEXT:         BranchInst %BB32
// CHECK-NEXT:%BB32:
// CHECK-NEXT:         BranchInst %BB35
// CHECK-NEXT:%BB33:
// CHECK-NEXT:  %109 = LoadStackInst (:any) %65: any
// CHECK-NEXT:  %110 = IteratorCloseInst (:any) %109: any, true: boolean
// CHECK-NEXT:         BranchInst %BB34
// CHECK-NEXT:%BB34:
// CHECK-NEXT:  %112 = LoadStackInst (:any) %73: any
// CHECK-NEXT:         ThrowInst %112: any
// CHECK-NEXT:%BB35:
// CHECK-NEXT:         TryEndInst
// CHECK-NEXT:         BranchInst %BB17
// CHECK-NEXT:%BB36:
// CHECK-NEXT:  %116 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %117 = IteratorCloseInst (:any) %116: any, true: boolean
// CHECK-NEXT:         BranchInst %BB37
// CHECK-NEXT:%BB37:
// CHECK-NEXT:  %119 = LoadStackInst (:any) %15: any
// CHECK-NEXT:         ThrowInst %119: any
// CHECK-NEXT:function_end
