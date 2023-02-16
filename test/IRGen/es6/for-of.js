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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "forof_normal": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "forof_update": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "forof_break": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "forof_continue": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %forof_normal(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: closure, globalObject: object, "forof_normal": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:closure) %forof_update(): any
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: closure, globalObject: object, "forof_update": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:closure) %forof_break(): any
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8: closure, globalObject: object, "forof_break": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:closure) %forof_continue(): any
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: closure, globalObject: object, "forof_continue": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %13 = StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:  %15 = ReturnInst (:any) %14: any
// CHECK-NEXT:function_end

// CHECK:function forof_normal(seq: any, cb: any): any
// CHECK-NEXT:frame = [seq: any, cb: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %seq: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [seq]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cb: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [cb]: any
// CHECK-NEXT:  %4 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [seq]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:  %8 = StoreStackInst %5: any, %7: any
// CHECK-NEXT:  %9 = IteratorBeginInst (:any) %7: any
// CHECK-NEXT:  %10 = StoreStackInst %9: any, %6: any
// CHECK-NEXT:  %11 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = IteratorNextInst (:any) %6: any, %7: any
// CHECK-NEXT:  %13 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %14 = BinaryStrictlyEqualInst (:any) %13: any, undefined: undefined
// CHECK-NEXT:  %15 = CondBranchInst %14: any, %BB2, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %17 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %18 = CatchInst (:any)
// CHECK-NEXT:  %19 = IteratorCloseInst (:any) %6: any, true: boolean
// CHECK-NEXT:  %20 = ThrowInst %18: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %21 = StoreFrameInst %12: any, [i]: any
// CHECK-NEXT:  %22 = LoadFrameInst (:any) [cb]: any
// CHECK-NEXT:  %23 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %24 = CallInst (:any) %22: any, empty: any, empty: any, undefined: undefined, %23: any
// CHECK-NEXT:  %25 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %26 = TryEndInst
// CHECK-NEXT:  %27 = BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function forof_update(seq: any): any
// CHECK-NEXT:frame = [seq: any, i: any, ar: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %seq: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [seq]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %3 = StoreFrameInst undefined: undefined, [ar]: any
// CHECK-NEXT:  %4 = StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %5 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %6 = StoreFrameInst %5: object, [ar]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [seq]: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:  %10 = StoreStackInst %7: any, %9: any
// CHECK-NEXT:  %11 = IteratorBeginInst (:any) %9: any
// CHECK-NEXT:  %12 = StoreStackInst %11: any, %8: any
// CHECK-NEXT:  %13 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = IteratorNextInst (:any) %8: any, %9: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %16 = BinaryStrictlyEqualInst (:any) %15: any, undefined: undefined
// CHECK-NEXT:  %17 = CondBranchInst %16: any, %BB2, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %19 = LoadFrameInst (:any) [ar]: any
// CHECK-NEXT:  %20 = ReturnInst (:any) %19: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %21 = CatchInst (:any)
// CHECK-NEXT:  %22 = IteratorCloseInst (:any) %8: any, true: boolean
// CHECK-NEXT:  %23 = ThrowInst %21: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %24 = LoadFrameInst (:any) [ar]: any
// CHECK-NEXT:  %25 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %26 = AsNumericInst (:number|bigint) %25: any
// CHECK-NEXT:  %27 = UnaryIncInst (:any) %26: number|bigint
// CHECK-NEXT:  %28 = StoreFrameInst %27: any, [i]: any
// CHECK-NEXT:  %29 = StorePropertyLooseInst %14: any, %24: any, %26: number|bigint
// CHECK-NEXT:  %30 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %31 = TryEndInst
// CHECK-NEXT:  %32 = BranchInst %BB1
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %33 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function forof_break(seq: any): any
// CHECK-NEXT:frame = [seq: any, sum: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %seq: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [seq]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [sum]: any
// CHECK-NEXT:  %3 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %4 = StoreFrameInst 0: number, [sum]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [seq]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:  %8 = StoreStackInst %5: any, %7: any
// CHECK-NEXT:  %9 = IteratorBeginInst (:any) %7: any
// CHECK-NEXT:  %10 = StoreStackInst %9: any, %6: any
// CHECK-NEXT:  %11 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = IteratorNextInst (:any) %6: any, %7: any
// CHECK-NEXT:  %13 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %14 = BinaryStrictlyEqualInst (:any) %13: any, undefined: undefined
// CHECK-NEXT:  %15 = CondBranchInst %14: any, %BB2, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %17 = LoadFrameInst (:any) [sum]: any
// CHECK-NEXT:  %18 = ReturnInst (:any) %17: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = CatchInst (:any)
// CHECK-NEXT:  %20 = IteratorCloseInst (:any) %6: any, true: boolean
// CHECK-NEXT:  %21 = ThrowInst %19: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %22 = StoreFrameInst %12: any, [i]: any
// CHECK-NEXT:  %23 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %24 = BinaryLessThanInst (:any) %23: any, 0: number
// CHECK-NEXT:  %25 = CondBranchInst %24: any, %BB6, %BB7
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %26 = BranchInst %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %27 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %28 = LoadFrameInst (:any) [sum]: any
// CHECK-NEXT:  %29 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %30 = BinaryAddInst (:any) %28: any, %29: any
// CHECK-NEXT:  %31 = StoreFrameInst %30: any, [sum]: any
// CHECK-NEXT:  %32 = BranchInst %BB10
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %33 = TryEndInst
// CHECK-NEXT:  %34 = IteratorCloseInst (:any) %6: any, false: boolean
// CHECK-NEXT:  %35 = BranchInst %BB2
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %36 = BranchInst %BB9
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %37 = TryEndInst
// CHECK-NEXT:  %38 = BranchInst %BB1
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %39 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function forof_continue(seq: any): any
// CHECK-NEXT:frame = [seq: any, sum: any, i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %seq: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [seq]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [sum]: any
// CHECK-NEXT:  %3 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %4 = StoreFrameInst 0: number, [sum]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [seq]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:  %8 = StoreStackInst %5: any, %7: any
// CHECK-NEXT:  %9 = IteratorBeginInst (:any) %7: any
// CHECK-NEXT:  %10 = StoreStackInst %9: any, %6: any
// CHECK-NEXT:  %11 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = IteratorNextInst (:any) %6: any, %7: any
// CHECK-NEXT:  %13 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %14 = BinaryStrictlyEqualInst (:any) %13: any, undefined: undefined
// CHECK-NEXT:  %15 = CondBranchInst %14: any, %BB2, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %17 = LoadFrameInst (:any) [sum]: any
// CHECK-NEXT:  %18 = ReturnInst (:any) %17: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = CatchInst (:any)
// CHECK-NEXT:  %20 = IteratorCloseInst (:any) %6: any, true: boolean
// CHECK-NEXT:  %21 = ThrowInst %19: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %22 = StoreFrameInst %12: any, [i]: any
// CHECK-NEXT:  %23 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %24 = BinaryLessThanInst (:any) %23: any, 0: number
// CHECK-NEXT:  %25 = CondBranchInst %24: any, %BB6, %BB7
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %26 = BranchInst %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %27 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %28 = LoadFrameInst (:any) [sum]: any
// CHECK-NEXT:  %29 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %30 = BinaryAddInst (:any) %28: any, %29: any
// CHECK-NEXT:  %31 = StoreFrameInst %30: any, [sum]: any
// CHECK-NEXT:  %32 = BranchInst %BB10
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %33 = TryEndInst
// CHECK-NEXT:  %34 = BranchInst %BB1
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %35 = BranchInst %BB9
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %36 = TryEndInst
// CHECK-NEXT:  %37 = BranchInst %BB1
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %38 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end
