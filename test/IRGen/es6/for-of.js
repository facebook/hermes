/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function forof_normal(seq, cb) {
    for(var i of seq)
        cb(i);
}

function forof_update(seq) {
    var i = 0, ar = [];
    for(ar[i++] of seq);
    return ar;
}

function forof_break(seq) {
    var sum = 0;
    for(var i of seq) {
        if (i < 0)
            break;
        sum += i;
    }
    return sum;
}

function forof_continue(seq) {
    var sum = 0;
    for(var i of seq) {
        if (i < 0)
            continue;
        sum += i;
    }
    return sum;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "forof_normal": string
// CHECK-NEXT:       DeclareGlobalVarInst "forof_update": string
// CHECK-NEXT:       DeclareGlobalVarInst "forof_break": string
// CHECK-NEXT:       DeclareGlobalVarInst "forof_continue": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %forof_normal(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "forof_normal": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %forof_update(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "forof_update": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %forof_break(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "forof_break": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %forof_continue(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "forof_continue": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:function forof_normal(seq: any, cb: any): any
// CHECK-NEXT:frame = [seq: any, cb: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %forof_normal(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %seq: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [seq]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %cb: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [cb]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [i]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [seq]: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %7: any, %9: any
// CHECK-NEXT:  %11 = IteratorBeginInst (:any) %9: any
// CHECK-NEXT:        StoreStackInst %11: any, %8: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %15 = IteratorNextInst (:any) %8: any, %14: any
// CHECK-NEXT:  %16 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %17 = BinaryStrictlyEqualInst (:any) %16: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %17: any, %BB3, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %21 = CatchInst (:any)
// CHECK-NEXT:  %22 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %23 = IteratorCloseInst (:any) %22: any, true: boolean
// CHECK-NEXT:        ThrowInst %21: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: any, [i]: any
// CHECK-NEXT:  %26 = LoadFrameInst (:any) %1: environment, [cb]: any
// CHECK-NEXT:  %27 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %28 = CallInst (:any) %26: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %27: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function forof_update(seq: any): any
// CHECK-NEXT:frame = [seq: any, i: any, ar: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %forof_update(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %seq: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [seq]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [ar]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [i]: any
// CHECK-NEXT:  %7 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: object, [ar]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [seq]: any
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %9: any, %11: any
// CHECK-NEXT:  %13 = IteratorBeginInst (:any) %11: any
// CHECK-NEXT:        StoreStackInst %13: any, %10: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = LoadStackInst (:any) %11: any
// CHECK-NEXT:  %17 = IteratorNextInst (:any) %10: any, %16: any
// CHECK-NEXT:  %18 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %19 = BinaryStrictlyEqualInst (:any) %18: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %19: any, %BB3, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %22 = LoadFrameInst (:any) %1: environment, [ar]: any
// CHECK-NEXT:        ReturnInst %22: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %24 = CatchInst (:any)
// CHECK-NEXT:  %25 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %26 = IteratorCloseInst (:any) %25: any, true: boolean
// CHECK-NEXT:        ThrowInst %24: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %28 = LoadFrameInst (:any) %1: environment, [ar]: any
// CHECK-NEXT:  %29 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %30 = AsNumericInst (:number|bigint) %29: any
// CHECK-NEXT:  %31 = UnaryIncInst (:number|bigint) %30: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %31: number|bigint, [i]: any
// CHECK-NEXT:        StorePropertyLooseInst %17: any, %28: any, %30: number|bigint
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function forof_break(seq: any): any
// CHECK-NEXT:frame = [seq: any, sum: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %forof_break(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %seq: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [seq]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [sum]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [sum]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [seq]: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %7: any, %9: any
// CHECK-NEXT:  %11 = IteratorBeginInst (:any) %9: any
// CHECK-NEXT:        StoreStackInst %11: any, %8: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %15 = IteratorNextInst (:any) %8: any, %14: any
// CHECK-NEXT:  %16 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %17 = BinaryStrictlyEqualInst (:any) %16: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %17: any, %BB3, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [sum]: any
// CHECK-NEXT:        ReturnInst %20: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %22 = CatchInst (:any)
// CHECK-NEXT:  %23 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %24 = IteratorCloseInst (:any) %23: any, true: boolean
// CHECK-NEXT:        ThrowInst %22: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: any, [i]: any
// CHECK-NEXT:  %27 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %28 = BinaryLessThanInst (:boolean) %27: any, 0: number
// CHECK-NEXT:        CondBranchInst %28: boolean, %BB6, %BB7
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %32 = LoadFrameInst (:any) %1: environment, [sum]: any
// CHECK-NEXT:  %33 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %34 = BinaryAddInst (:any) %32: any, %33: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %34: any, [sum]: any
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %38 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %39 = IteratorCloseInst (:any) %38: any, false: boolean
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function forof_continue(seq: any): any
// CHECK-NEXT:frame = [seq: any, sum: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %forof_continue(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %seq: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [seq]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [sum]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [sum]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [seq]: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %7: any, %9: any
// CHECK-NEXT:  %11 = IteratorBeginInst (:any) %9: any
// CHECK-NEXT:        StoreStackInst %11: any, %8: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %15 = IteratorNextInst (:any) %8: any, %14: any
// CHECK-NEXT:  %16 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %17 = BinaryStrictlyEqualInst (:any) %16: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %17: any, %BB3, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [sum]: any
// CHECK-NEXT:        ReturnInst %20: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %22 = CatchInst (:any)
// CHECK-NEXT:  %23 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %24 = IteratorCloseInst (:any) %23: any, true: boolean
// CHECK-NEXT:        ThrowInst %22: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: any, [i]: any
// CHECK-NEXT:  %27 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %28 = BinaryLessThanInst (:boolean) %27: any, 0: number
// CHECK-NEXT:        CondBranchInst %28: boolean, %BB6, %BB7
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %32 = LoadFrameInst (:any) %1: environment, [sum]: any
// CHECK-NEXT:  %33 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %34 = BinaryAddInst (:any) %32: any, %33: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %34: any, [sum]: any
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end
