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
// CHECK-NEXT:       DeclareGlobalVarInst "forof_normal": string
// CHECK-NEXT:       DeclareGlobalVarInst "forof_update": string
// CHECK-NEXT:       DeclareGlobalVarInst "forof_break": string
// CHECK-NEXT:       DeclareGlobalVarInst "forof_continue": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %forof_normal(): any
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "forof_normal": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %forof_update(): any
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "forof_update": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %forof_break(): any
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "forof_break": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %forof_continue(): any
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "forof_continue": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function forof_normal(seq: any, cb: any): any
// CHECK-NEXT:frame = [seq: any, cb: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %seq: any
// CHECK-NEXT:       StoreFrameInst %0: any, [seq]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cb: any
// CHECK-NEXT:       StoreFrameInst %2: any, [cb]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [seq]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:       StoreStackInst %5: any, %7: any
// CHECK-NEXT:  %9 = IteratorBeginInst (:any) %7: any
// CHECK-NEXT:        StoreStackInst %9: any, %6: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %13 = IteratorNextInst (:any) %6: any, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %15 = BinaryStrictlyEqualInst (:any) %14: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %15: any, %BB2, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = CatchInst (:any)
// CHECK-NEXT:  %20 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %21 = IteratorCloseInst (:any) %20: any, true: boolean
// CHECK-NEXT:        ThrowInst %19: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreFrameInst %13: any, [i]: any
// CHECK-NEXT:  %24 = LoadFrameInst (:any) [cb]: any
// CHECK-NEXT:  %25 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %26 = CallInst (:any) %24: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %25: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function forof_update(seq: any): any
// CHECK-NEXT:frame = [seq: any, i: any, ar: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %seq: any
// CHECK-NEXT:       StoreFrameInst %0: any, [seq]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [ar]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %5 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:       StoreFrameInst %5: object, [ar]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [seq]: any
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
// CHECK-NEXT:        CondBranchInst %17: any, %BB2, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %20 = LoadFrameInst (:any) [ar]: any
// CHECK-NEXT:        ReturnInst %20: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %22 = CatchInst (:any)
// CHECK-NEXT:  %23 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %24 = IteratorCloseInst (:any) %23: any, true: boolean
// CHECK-NEXT:        ThrowInst %22: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %26 = LoadFrameInst (:any) [ar]: any
// CHECK-NEXT:  %27 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %28 = AsNumericInst (:number|bigint) %27: any
// CHECK-NEXT:  %29 = UnaryIncInst (:any) %28: number|bigint
// CHECK-NEXT:        StoreFrameInst %29: any, [i]: any
// CHECK-NEXT:        StorePropertyLooseInst %15: any, %26: any, %28: number|bigint
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function forof_break(seq: any): any
// CHECK-NEXT:frame = [seq: any, sum: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %seq: any
// CHECK-NEXT:       StoreFrameInst %0: any, [seq]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [sum]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [sum]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [seq]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:       StoreStackInst %5: any, %7: any
// CHECK-NEXT:  %9 = IteratorBeginInst (:any) %7: any
// CHECK-NEXT:        StoreStackInst %9: any, %6: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %13 = IteratorNextInst (:any) %6: any, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %15 = BinaryStrictlyEqualInst (:any) %14: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %15: any, %BB2, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [sum]: any
// CHECK-NEXT:        ReturnInst %18: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %20 = CatchInst (:any)
// CHECK-NEXT:  %21 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %22 = IteratorCloseInst (:any) %21: any, true: boolean
// CHECK-NEXT:        ThrowInst %20: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreFrameInst %13: any, [i]: any
// CHECK-NEXT:  %25 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %26 = BinaryLessThanInst (:any) %25: any, 0: number
// CHECK-NEXT:        CondBranchInst %26: any, %BB6, %BB7
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %30 = LoadFrameInst (:any) [sum]: any
// CHECK-NEXT:  %31 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %32 = BinaryAddInst (:any) %30: any, %31: any
// CHECK-NEXT:        StoreFrameInst %32: any, [sum]: any
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %36 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %37 = IteratorCloseInst (:any) %36: any, false: boolean
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function forof_continue(seq: any): any
// CHECK-NEXT:frame = [seq: any, sum: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %seq: any
// CHECK-NEXT:       StoreFrameInst %0: any, [seq]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [sum]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [sum]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [seq]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:       StoreStackInst %5: any, %7: any
// CHECK-NEXT:  %9 = IteratorBeginInst (:any) %7: any
// CHECK-NEXT:        StoreStackInst %9: any, %6: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %13 = IteratorNextInst (:any) %6: any, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %15 = BinaryStrictlyEqualInst (:any) %14: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %15: any, %BB2, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [sum]: any
// CHECK-NEXT:        ReturnInst %18: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %20 = CatchInst (:any)
// CHECK-NEXT:  %21 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %22 = IteratorCloseInst (:any) %21: any, true: boolean
// CHECK-NEXT:        ThrowInst %20: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreFrameInst %13: any, [i]: any
// CHECK-NEXT:  %25 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %26 = BinaryLessThanInst (:any) %25: any, 0: number
// CHECK-NEXT:        CondBranchInst %26: any, %BB6, %BB7
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %30 = LoadFrameInst (:any) [sum]: any
// CHECK-NEXT:  %31 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %32 = BinaryAddInst (:any) %30: any, %31: any
// CHECK-NEXT:        StoreFrameInst %32: any, [sum]: any
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end
