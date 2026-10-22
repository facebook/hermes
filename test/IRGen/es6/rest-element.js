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

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "f1": string
// CHECK-NEXT:       DeclareGlobalVarInst "f2": string
// CHECK-NEXT:       DeclareGlobalVarInst "f3": string
// CHECK-NEXT:       DeclareGlobalVarInst "f4": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %VS0: any, %f1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "f1": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %VS0: any, %f2(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "f2": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %VS0: any, %f3(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "f3": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %VS0: any, %f4(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "f4": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [t: any, a: any]

// CHECK:function f1(t: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.t]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.a]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %1: environment, [%VS1.t]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:       StoreStackInst %5: any, %7: any
// CHECK-NEXT:  %9 = IteratorBeginInst (:any) %7: any
// CHECK-NEXT:        StoreStackInst %9: any, %6: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_2_iterValue: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_3_exc: any
// CHECK-NEXT:  %13 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %14 = AllocStackInst (:number) $?anon_4_n: any
// CHECK-NEXT:        StoreStackInst 0: number, %14: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %17 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %18 = IteratorCloseInst (:any) %17: any, true: boolean
// CHECK-NEXT:  %19 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        ThrowInst %19: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %21 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %22 = IteratorNextInst (:any) %6: any, %21: any
// CHECK-NEXT:  %23 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %24 = BinaryStrictlyEqualInst (:any) %23: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %24: any, %BB4, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %26 = LoadStackInst (:number) %14: number
// CHECK-NEXT:        TryStartInst %BB5, %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        StoreFrameInst %1: environment, %13: object, [%VS1.a]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %30 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %30: any, %12: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %33 = BinaryAddInst (:number) %26: number, 1: number
// CHECK-NEXT:        StoreStackInst %33: number, %14: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        StorePropertyLooseInst %22: any, %13: object, %26: number
// CHECK-NEXT:        TryEndInst %BB5, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [t: any, b: any, c: any]

// CHECK:function f2(t: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.t]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.b]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.c]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS2.t]: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:       StoreStackInst %6: any, %8: any
// CHECK-NEXT:  %10 = IteratorBeginInst (:any) %8: any
// CHECK-NEXT:        StoreStackInst %10: any, %7: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_2_iterValue: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_3_exc: any
// CHECK-NEXT:        TryStartInst %BB5, %BB7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %15 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %16 = IteratorCloseInst (:any) %15: any, true: boolean
// CHECK-NEXT:  %17 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ThrowInst %17: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %19 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %20 = IteratorNextInst (:any) %7: any, %19: any
// CHECK-NEXT:  %21 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %22 = BinaryStrictlyEqualInst (:any) %21: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %22: any, %BB4, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %24 = LoadStackInst (:number) %31: number
// CHECK-NEXT:        TryStartInst %BB9, %BB11
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryStartInst %BB13, %BB15
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %27 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %27: any, %13: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %30 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %31 = AllocStackInst (:number) $?anon_4_n: any
// CHECK-NEXT:        StoreStackInst 0: number, %31: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        TryEndInst %BB5, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %36 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %36: any, %13: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %39 = BinaryAddInst (:number) %24: number, 1: number
// CHECK-NEXT:        StoreStackInst %39: number, %31: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        StorePropertyLooseInst %20: any, %30: object, %24: number
// CHECK-NEXT:        TryEndInst %BB9, %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %45 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %45: any, %13: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %49 = AllocStackInst (:any) $?anon_5_iter: any
// CHECK-NEXT:  %50 = AllocStackInst (:any) $?anon_6_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %30: object, %50: any
// CHECK-NEXT:  %52 = IteratorBeginInst (:any) %50: any
// CHECK-NEXT:        StoreStackInst %52: any, %49: any
// CHECK-NEXT:  %54 = AllocStackInst (:any) $?anon_7_iterValue: any
// CHECK-NEXT:  %55 = LoadStackInst (:any) %50: any
// CHECK-NEXT:  %56 = IteratorNextInst (:any) %49: any, %55: any
// CHECK-NEXT:        StoreStackInst %56: any, %54: any
// CHECK-NEXT:  %58 = LoadStackInst (:any) %54: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %58: any, [%VS2.b]: any
// CHECK-NEXT:  %60 = LoadStackInst (:any) %50: any
// CHECK-NEXT:  %61 = IteratorNextInst (:any) %49: any, %60: any
// CHECK-NEXT:        StoreStackInst %61: any, %54: any
// CHECK-NEXT:  %63 = LoadStackInst (:any) %54: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %63: any, [%VS2.c]: any
// CHECK-NEXT:  %65 = LoadStackInst (:any) %49: any
// CHECK-NEXT:  %66 = IteratorCloseInst (:any) %65: any, false: boolean
// CHECK-NEXT:        TryEndInst %BB13, %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:        BranchInst %BB14
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [t: any, d: any]

// CHECK:function f3(t: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS3.t]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS3.d]: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %7 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %8 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [%VS3.t]: any
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
// CHECK-NEXT:  %21 = AllocStackInst (:any) $?anon_7_iterValue: any
// CHECK-NEXT:  %22 = AllocStackInst (:any) $?anon_8_exc: any
// CHECK-NEXT:  %23 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %24 = AllocStackInst (:number) $?anon_9_n: any
// CHECK-NEXT:        StoreStackInst 0: number, %24: number
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %27 = LoadStackInst (:any) %16: any
// CHECK-NEXT:  %28 = IteratorCloseInst (:any) %27: any, true: boolean
// CHECK-NEXT:  %29 = LoadStackInst (:any) %22: any
// CHECK-NEXT:        ThrowInst %29: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %31 = LoadStackInst (:any) %17: any
// CHECK-NEXT:  %32 = IteratorNextInst (:any) %16: any, %31: any
// CHECK-NEXT:  %33 = LoadStackInst (:any) %16: any
// CHECK-NEXT:  %34 = BinaryStrictlyEqualInst (:any) %33: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %34: any, %BB7, %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %36 = LoadStackInst (:number) %24: number
// CHECK-NEXT:        TryStartInst %BB8, %BB10
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        StoreFrameInst %1: environment, %23: object, [%VS3.d]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %40 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %40: any, %22: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %43 = BinaryAddInst (:number) %36: number, 1: number
// CHECK-NEXT:        StoreStackInst %43: number, %24: number
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        StorePropertyLooseInst %32: any, %23: object, %36: number
// CHECK-NEXT:        TryEndInst %BB8, %BB11
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [t: any, a: any, b: any]

// CHECK:function f4(t: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS4.t]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS4.a]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS4.b]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS4.t]: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:       StoreStackInst %6: any, %8: any
// CHECK-NEXT:  %10 = IteratorBeginInst (:any) %8: any
// CHECK-NEXT:        StoreStackInst %10: any, %7: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_2_iterValue: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_3_exc: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %15 = IteratorNextInst (:any) %7: any, %14: any
// CHECK-NEXT:        StoreStackInst %15: any, %12: any
// CHECK-NEXT:  %17 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %17: any, [%VS4.a]: any
// CHECK-NEXT:        TryStartInst %BB5, %BB7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %20 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %21 = IteratorCloseInst (:any) %20: any, true: boolean
// CHECK-NEXT:  %22 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ThrowInst %22: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %24 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %25 = IteratorNextInst (:any) %7: any, %24: any
// CHECK-NEXT:  %26 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %27 = BinaryStrictlyEqualInst (:any) %26: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %27: any, %BB4, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %29 = LoadStackInst (:number) %36: number
// CHECK-NEXT:        TryStartInst %BB9, %BB11
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryStartInst %BB13, %BB15
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %32 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %32: any, %13: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %35 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %36 = AllocStackInst (:number) $?anon_4_n: any
// CHECK-NEXT:        StoreStackInst 0: number, %36: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        TryEndInst %BB5, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %41 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %41: any, %13: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %44 = BinaryAddInst (:number) %29: number, 1: number
// CHECK-NEXT:        StoreStackInst %44: number, %36: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        StorePropertyLooseInst %25: any, %35: object, %29: number
// CHECK-NEXT:        TryEndInst %BB9, %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %50 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %50: any, %13: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %54 = AllocStackInst (:any) $?anon_5_iter: any
// CHECK-NEXT:  %55 = AllocStackInst (:any) $?anon_6_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %35: object, %55: any
// CHECK-NEXT:  %57 = IteratorBeginInst (:any) %55: any
// CHECK-NEXT:        StoreStackInst %57: any, %54: any
// CHECK-NEXT:  %59 = AllocStackInst (:any) $?anon_7_iterValue: any
// CHECK-NEXT:  %60 = AllocStackInst (:any) $?anon_8_exc: any
// CHECK-NEXT:        TryStartInst %BB17, %BB19
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %62 = LoadStackInst (:any) %54: any
// CHECK-NEXT:  %63 = IteratorCloseInst (:any) %62: any, true: boolean
// CHECK-NEXT:  %64 = LoadStackInst (:any) %60: any
// CHECK-NEXT:        ThrowInst %64: any, %BB13
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %66 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %66: any, %60: any
// CHECK-NEXT:        BranchInst %BB16
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %69 = LoadStackInst (:any) %55: any
// CHECK-NEXT:  %70 = IteratorNextInst (:any) %54: any, %69: any
// CHECK-NEXT:        StoreStackInst %70: any, %59: any
// CHECK-NEXT:        TryStartInst %BB21, %BB23
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %73 = LoadFrameInst (:any) %1: environment, [%VS4.b]: any
// CHECK-NEXT:        TryEndInst %BB17, %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:        BranchInst %BB18
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %76 = CatchInst (:any)
// CHECK-NEXT:        StoreStackInst %76: any, %60: any
// CHECK-NEXT:        BranchInst %BB16
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %79 = LoadStackInst (:any) %54: any
// CHECK-NEXT:  %80 = IteratorCloseInst (:any) %79: any, false: boolean
// CHECK-NEXT:        TryEndInst %BB13, %BB25
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %82 = LoadStackInst (:any) %59: any
// CHECK-NEXT:        StorePropertyLooseInst %82: any, %73: any, 0: number
// CHECK-NEXT:        TryEndInst %BB21, %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:        BranchInst %BB22
// CHECK-NEXT:%BB25:
// CHECK-NEXT:        BranchInst %BB14
// CHECK-NEXT:function_end
