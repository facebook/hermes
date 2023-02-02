/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "forof_normal" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "forof_update" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "forof_break" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "forof_continue" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %forof_normal()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "forof_normal" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %forof_update()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "forof_update" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %forof_break()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "forof_break" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %forof_continue()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "forof_continue" : string
// CHECK-NEXT:  %12 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %13 = StoreStackInst undefined : undefined, %12
// CHECK-NEXT:  %14 = LoadStackInst %12
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:function_end

// CHECK:function forof_normal(seq, cb)
// CHECK-NEXT:frame = [seq, cb, i]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %seq
// CHECK-NEXT:  %1 = StoreFrameInst %0, [seq]
// CHECK-NEXT:  %2 = LoadParamInst %cb
// CHECK-NEXT:  %3 = StoreFrameInst %2, [cb]
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %5 = LoadFrameInst [seq]
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %7 = AllocStackInst $?anon_1_sourceOrNext
// CHECK-NEXT:  %8 = StoreStackInst %5, %7
// CHECK-NEXT:  %9 = IteratorBeginInst %7
// CHECK-NEXT:  %10 = StoreStackInst %9, %6
// CHECK-NEXT:  %11 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = IteratorNextInst %6, %7
// CHECK-NEXT:  %13 = LoadStackInst %6
// CHECK-NEXT:  %14 = BinaryOperatorInst '===', %13, undefined : undefined
// CHECK-NEXT:  %15 = CondBranchInst %14, %BB2, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %17 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %18 = CatchInst
// CHECK-NEXT:  %19 = IteratorCloseInst %6, true : boolean
// CHECK-NEXT:  %20 = ThrowInst %18
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %21 = StoreFrameInst %12, [i]
// CHECK-NEXT:  %22 = LoadFrameInst [cb]
// CHECK-NEXT:  %23 = LoadFrameInst [i]
// CHECK-NEXT:  %24 = CallInst %22, undefined : undefined, %23
// CHECK-NEXT:  %25 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %26 = TryEndInst
// CHECK-NEXT:  %27 = BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function forof_update(seq)
// CHECK-NEXT:frame = [seq, i, ar]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %seq
// CHECK-NEXT:  %1 = StoreFrameInst %0, [seq]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [ar]
// CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %5 = AllocArrayInst 0 : number
// CHECK-NEXT:  %6 = StoreFrameInst %5 : object, [ar]
// CHECK-NEXT:  %7 = LoadFrameInst [seq]
// CHECK-NEXT:  %8 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %9 = AllocStackInst $?anon_1_sourceOrNext
// CHECK-NEXT:  %10 = StoreStackInst %7, %9
// CHECK-NEXT:  %11 = IteratorBeginInst %9
// CHECK-NEXT:  %12 = StoreStackInst %11, %8
// CHECK-NEXT:  %13 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = IteratorNextInst %8, %9
// CHECK-NEXT:  %15 = LoadStackInst %8
// CHECK-NEXT:  %16 = BinaryOperatorInst '===', %15, undefined : undefined
// CHECK-NEXT:  %17 = CondBranchInst %16, %BB2, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %19 = LoadFrameInst [ar]
// CHECK-NEXT:  %20 = ReturnInst %19
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %21 = CatchInst
// CHECK-NEXT:  %22 = IteratorCloseInst %8, true : boolean
// CHECK-NEXT:  %23 = ThrowInst %21
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %24 = LoadFrameInst [ar]
// CHECK-NEXT:  %25 = LoadFrameInst [i]
// CHECK-NEXT:  %26 = AsNumericInst %25
// CHECK-NEXT:  %27 = UnaryOperatorInst '++', %26 : number|bigint
// CHECK-NEXT:  %28 = StoreFrameInst %27, [i]
// CHECK-NEXT:  %29 = StorePropertyLooseInst %14, %24, %26 : number|bigint
// CHECK-NEXT:  %30 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %31 = TryEndInst
// CHECK-NEXT:  %32 = BranchInst %BB1
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %33 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function forof_break(seq)
// CHECK-NEXT:frame = [seq, sum, i]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %seq
// CHECK-NEXT:  %1 = StoreFrameInst %0, [seq]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [sum]
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [sum]
// CHECK-NEXT:  %5 = LoadFrameInst [seq]
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %7 = AllocStackInst $?anon_1_sourceOrNext
// CHECK-NEXT:  %8 = StoreStackInst %5, %7
// CHECK-NEXT:  %9 = IteratorBeginInst %7
// CHECK-NEXT:  %10 = StoreStackInst %9, %6
// CHECK-NEXT:  %11 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = IteratorNextInst %6, %7
// CHECK-NEXT:  %13 = LoadStackInst %6
// CHECK-NEXT:  %14 = BinaryOperatorInst '===', %13, undefined : undefined
// CHECK-NEXT:  %15 = CondBranchInst %14, %BB2, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %17 = LoadFrameInst [sum]
// CHECK-NEXT:  %18 = ReturnInst %17
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = CatchInst
// CHECK-NEXT:  %20 = IteratorCloseInst %6, true : boolean
// CHECK-NEXT:  %21 = ThrowInst %19
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %22 = StoreFrameInst %12, [i]
// CHECK-NEXT:  %23 = LoadFrameInst [i]
// CHECK-NEXT:  %24 = BinaryOperatorInst '<', %23, 0 : number
// CHECK-NEXT:  %25 = CondBranchInst %24, %BB6, %BB7
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %26 = BranchInst %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %27 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %28 = LoadFrameInst [sum]
// CHECK-NEXT:  %29 = LoadFrameInst [i]
// CHECK-NEXT:  %30 = BinaryOperatorInst '+', %28, %29
// CHECK-NEXT:  %31 = StoreFrameInst %30, [sum]
// CHECK-NEXT:  %32 = BranchInst %BB10
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %33 = TryEndInst
// CHECK-NEXT:  %34 = IteratorCloseInst %6, false : boolean
// CHECK-NEXT:  %35 = BranchInst %BB2
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %36 = BranchInst %BB9
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %37 = TryEndInst
// CHECK-NEXT:  %38 = BranchInst %BB1
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %39 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function forof_continue(seq)
// CHECK-NEXT:frame = [seq, sum, i]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %seq
// CHECK-NEXT:  %1 = StoreFrameInst %0, [seq]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [sum]
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [sum]
// CHECK-NEXT:  %5 = LoadFrameInst [seq]
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %7 = AllocStackInst $?anon_1_sourceOrNext
// CHECK-NEXT:  %8 = StoreStackInst %5, %7
// CHECK-NEXT:  %9 = IteratorBeginInst %7
// CHECK-NEXT:  %10 = StoreStackInst %9, %6
// CHECK-NEXT:  %11 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = IteratorNextInst %6, %7
// CHECK-NEXT:  %13 = LoadStackInst %6
// CHECK-NEXT:  %14 = BinaryOperatorInst '===', %13, undefined : undefined
// CHECK-NEXT:  %15 = CondBranchInst %14, %BB2, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %17 = LoadFrameInst [sum]
// CHECK-NEXT:  %18 = ReturnInst %17
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = CatchInst
// CHECK-NEXT:  %20 = IteratorCloseInst %6, true : boolean
// CHECK-NEXT:  %21 = ThrowInst %19
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %22 = StoreFrameInst %12, [i]
// CHECK-NEXT:  %23 = LoadFrameInst [i]
// CHECK-NEXT:  %24 = BinaryOperatorInst '<', %23, 0 : number
// CHECK-NEXT:  %25 = CondBranchInst %24, %BB6, %BB7
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %26 = BranchInst %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %27 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %28 = LoadFrameInst [sum]
// CHECK-NEXT:  %29 = LoadFrameInst [i]
// CHECK-NEXT:  %30 = BinaryOperatorInst '+', %28, %29
// CHECK-NEXT:  %31 = StoreFrameInst %30, [sum]
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
// CHECK-NEXT:  %38 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
