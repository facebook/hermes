/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir %s | %FileCheck --match-full-lines %s

function forof_normal(seq, cb) {
    for(var i of seq)
        cb(i);
}
//CHECK-LABEL:function forof_normal(seq, cb)
//CHECK-NEXT:frame = [i, seq, cb]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:  %1 = StoreFrameInst %seq, [seq]
//CHECK-NEXT:  %2 = StoreFrameInst %cb, [cb]
//CHECK-NEXT:  %3 = LoadFrameInst [seq]
//CHECK-NEXT:  %4 = AllocStackInst $?anon_0_iter
//CHECK-NEXT:  %5 = AllocStackInst $?anon_1_sourceOrNext
//CHECK-NEXT:  %6 = StoreStackInst %3, %5
//CHECK-NEXT:  %7 = IteratorBeginInst %5
//CHECK-NEXT:  %8 = StoreStackInst %7, %4
//CHECK-NEXT:  %9 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %10 = IteratorNextInst %4, %5
//CHECK-NEXT:  %11 = LoadStackInst %4
//CHECK-NEXT:  %12 = BinaryOperatorInst '===', %11, undefined : undefined
//CHECK-NEXT:  %13 = CondBranchInst %12, %BB2, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %14 = TryStartInst %BB4, %BB5
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %15 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %16 = CatchInst
//CHECK-NEXT:  %17 = IteratorCloseInst %4, true : boolean
//CHECK-NEXT:  %18 = ThrowInst %16
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %19 = StoreFrameInst %10, [i]
//CHECK-NEXT:  %20 = LoadFrameInst [cb]
//CHECK-NEXT:  %21 = LoadFrameInst [i]
//CHECK-NEXT:  %22 = CallInst %20, undefined : undefined, %21
//CHECK-NEXT:  %23 = BranchInst %BB6
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %24 = TryEndInst
//CHECK-NEXT:  %25 = BranchInst %BB1
//CHECK-NEXT:function_end

function forof_update(seq) {
    var i = 0, ar = [];
    for(ar[i++] of seq);
    return ar;
}
//CHECK-LABEL:function forof_update(seq)
//CHECK-NEXT:frame = [i, ar, seq]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [ar]
//CHECK-NEXT:  %2 = StoreFrameInst %seq, [seq]
//CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:  %4 = AllocArrayInst 0 : number
//CHECK-NEXT:  %5 = StoreFrameInst %4 : object, [ar]
//CHECK-NEXT:  %6 = LoadFrameInst [seq]
//CHECK-NEXT:  %7 = AllocStackInst $?anon_0_iter
//CHECK-NEXT:  %8 = AllocStackInst $?anon_1_sourceOrNext
//CHECK-NEXT:  %9 = StoreStackInst %6, %8
//CHECK-NEXT:  %10 = IteratorBeginInst %8
//CHECK-NEXT:  %11 = StoreStackInst %10, %7
//CHECK-NEXT:  %12 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %13 = IteratorNextInst %7, %8
//CHECK-NEXT:  %14 = LoadStackInst %7
//CHECK-NEXT:  %15 = BinaryOperatorInst '===', %14, undefined : undefined
//CHECK-NEXT:  %16 = CondBranchInst %15, %BB2, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %17 = TryStartInst %BB4, %BB5
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %18 = LoadFrameInst [ar]
//CHECK-NEXT:  %19 = ReturnInst %18
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %20 = CatchInst
//CHECK-NEXT:  %21 = IteratorCloseInst %7, true : boolean
//CHECK-NEXT:  %22 = ThrowInst %20
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %23 = LoadFrameInst [ar]
//CHECK-NEXT:  %24 = LoadFrameInst [i]
//CHECK-NEXT:  %25 = AsNumericInst %24
//CHECK-NEXT:  %26 = UnaryOperatorInst '++', %25 : number|bigint
//CHECK-NEXT:  %27 = StoreFrameInst %26, [i]
//CHECK-NEXT:  %28 = StorePropertyInst %13, %23, %25 : number|bigint
//CHECK-NEXT:  %29 = BranchInst %BB6
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %30 = TryEndInst
//CHECK-NEXT:  %31 = BranchInst %BB1
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %32 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function forof_break(seq) {
    var sum = 0;
    for(var i of seq) {
        if (i < 0)
            break;
        sum += i;
    }
    return sum;
}
//CHECK-LABEL:function forof_break(seq)
//CHECK-NEXT:frame = [sum, i, seq]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [sum]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:  %2 = StoreFrameInst %seq, [seq]
//CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [sum]
//CHECK-NEXT:  %4 = LoadFrameInst [seq]
//CHECK-NEXT:  %5 = AllocStackInst $?anon_0_iter
//CHECK-NEXT:  %6 = AllocStackInst $?anon_1_sourceOrNext
//CHECK-NEXT:  %7 = StoreStackInst %4, %6
//CHECK-NEXT:  %8 = IteratorBeginInst %6
//CHECK-NEXT:  %9 = StoreStackInst %8, %5
//CHECK-NEXT:  %10 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %11 = IteratorNextInst %5, %6
//CHECK-NEXT:  %12 = LoadStackInst %5
//CHECK-NEXT:  %13 = BinaryOperatorInst '===', %12, undefined : undefined
//CHECK-NEXT:  %14 = CondBranchInst %13, %BB2, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %15 = TryStartInst %BB4, %BB5
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %16 = LoadFrameInst [sum]
//CHECK-NEXT:  %17 = ReturnInst %16
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %18 = CatchInst
//CHECK-NEXT:  %19 = IteratorCloseInst %5, true : boolean
//CHECK-NEXT:  %20 = ThrowInst %18
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %21 = StoreFrameInst %11, [i]
//CHECK-NEXT:  %22 = LoadFrameInst [i]
//CHECK-NEXT:  %23 = BinaryOperatorInst '<', %22, 0 : number
//CHECK-NEXT:  %24 = CondBranchInst %23, %BB6, %BB7
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %25 = BranchInst %BB8
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %26 = BranchInst %BB9
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %27 = LoadFrameInst [sum]
//CHECK-NEXT:  %28 = LoadFrameInst [i]
//CHECK-NEXT:  %29 = BinaryOperatorInst '+', %27, %28
//CHECK-NEXT:  %30 = StoreFrameInst %29, [sum]
//CHECK-NEXT:  %31 = BranchInst %BB10
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %32 = TryEndInst
//CHECK-NEXT:  %33 = IteratorCloseInst %5, false : boolean
//CHECK-NEXT:  %34 = BranchInst %BB2
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %35 = BranchInst %BB9
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %36 = TryEndInst
//CHECK-NEXT:  %37 = BranchInst %BB1
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %38 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function forof_continue(seq) {
    var sum = 0;
    for(var i of seq) {
        if (i < 0)
            continue;
        sum += i;
    }
    return sum;
}
//CHECK-LABEL:function forof_continue(seq)
//CHECK-NEXT:frame = [sum, i, seq]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [sum]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:  %2 = StoreFrameInst %seq, [seq]
//CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [sum]
//CHECK-NEXT:  %4 = LoadFrameInst [seq]
//CHECK-NEXT:  %5 = AllocStackInst $?anon_0_iter
//CHECK-NEXT:  %6 = AllocStackInst $?anon_1_sourceOrNext
//CHECK-NEXT:  %7 = StoreStackInst %4, %6
//CHECK-NEXT:  %8 = IteratorBeginInst %6
//CHECK-NEXT:  %9 = StoreStackInst %8, %5
//CHECK-NEXT:  %10 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %11 = IteratorNextInst %5, %6
//CHECK-NEXT:  %12 = LoadStackInst %5
//CHECK-NEXT:  %13 = BinaryOperatorInst '===', %12, undefined : undefined
//CHECK-NEXT:  %14 = CondBranchInst %13, %BB2, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %15 = TryStartInst %BB4, %BB5
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %16 = LoadFrameInst [sum]
//CHECK-NEXT:  %17 = ReturnInst %16
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %18 = CatchInst
//CHECK-NEXT:  %19 = IteratorCloseInst %5, true : boolean
//CHECK-NEXT:  %20 = ThrowInst %18
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %21 = StoreFrameInst %11, [i]
//CHECK-NEXT:  %22 = LoadFrameInst [i]
//CHECK-NEXT:  %23 = BinaryOperatorInst '<', %22, 0 : number
//CHECK-NEXT:  %24 = CondBranchInst %23, %BB6, %BB7
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %25 = BranchInst %BB8
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %26 = BranchInst %BB9
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %27 = LoadFrameInst [sum]
//CHECK-NEXT:  %28 = LoadFrameInst [i]
//CHECK-NEXT:  %29 = BinaryOperatorInst '+', %27, %28
//CHECK-NEXT:  %30 = StoreFrameInst %29, [sum]
//CHECK-NEXT:  %31 = BranchInst %BB10
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %32 = TryEndInst
//CHECK-NEXT:  %33 = BranchInst %BB1
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %34 = BranchInst %BB9
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %35 = TryEndInst
//CHECK-NEXT:  %36 = BranchInst %BB1
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %37 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
