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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [forof_normal, forof_update, forof_break, forof_continue]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %forof_normal#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "forof_normal" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %forof_update#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "forof_update" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %forof_break#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "forof_break" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %forof_continue#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "forof_continue" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function forof_normal#0#1(seq, cb)#2
// CHECK-NEXT:frame = [seq#2, cb#2, i#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{forof_normal#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %seq, [seq#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %cb, [cb#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [i#2], %0
// CHECK-NEXT:  %4 = LoadFrameInst [seq#2], %0
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_sourceOrNext
// CHECK-NEXT:  %7 = StoreStackInst %4, %6
// CHECK-NEXT:  %8 = IteratorBeginInst %6
// CHECK-NEXT:  %9 = StoreStackInst %8, %5
// CHECK-NEXT:  %10 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = IteratorNextInst %5, %6
// CHECK-NEXT:  %12 = LoadStackInst %5
// CHECK-NEXT:  %13 = BinaryOperatorInst '===', %12, undefined : undefined
// CHECK-NEXT:  %14 = CondBranchInst %13, %BB2, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %15 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %16 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %17 = CatchInst
// CHECK-NEXT:  %18 = IteratorCloseInst %5, true : boolean
// CHECK-NEXT:  %19 = ThrowInst %17
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = StoreFrameInst %11, [i#2], %0
// CHECK-NEXT:  %21 = LoadFrameInst [cb#2], %0
// CHECK-NEXT:  %22 = LoadFrameInst [i#2], %0
// CHECK-NEXT:  %23 = CallInst %21, undefined : undefined, %22
// CHECK-NEXT:  %24 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %25 = TryEndInst
// CHECK-NEXT:  %26 = BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function forof_update#0#1(seq)#3
// CHECK-NEXT:frame = [seq#3, i#3, ar#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{forof_update#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %seq, [seq#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [i#3], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [ar#3], %0
// CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [i#3], %0
// CHECK-NEXT:  %5 = AllocArrayInst 0 : number
// CHECK-NEXT:  %6 = StoreFrameInst %5 : object, [ar#3], %0
// CHECK-NEXT:  %7 = LoadFrameInst [seq#3], %0
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
// CHECK-NEXT:  %19 = LoadFrameInst [ar#3], %0
// CHECK-NEXT:  %20 = ReturnInst %19
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %21 = CatchInst
// CHECK-NEXT:  %22 = IteratorCloseInst %8, true : boolean
// CHECK-NEXT:  %23 = ThrowInst %21
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %24 = LoadFrameInst [ar#3], %0
// CHECK-NEXT:  %25 = LoadFrameInst [i#3], %0
// CHECK-NEXT:  %26 = AsNumericInst %25
// CHECK-NEXT:  %27 = UnaryOperatorInst '++', %26 : number|bigint
// CHECK-NEXT:  %28 = StoreFrameInst %27, [i#3], %0
// CHECK-NEXT:  %29 = StorePropertyInst %14, %24, %26 : number|bigint
// CHECK-NEXT:  %30 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %31 = TryEndInst
// CHECK-NEXT:  %32 = BranchInst %BB1
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %33 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function forof_break#0#1(seq)#4
// CHECK-NEXT:frame = [seq#4, sum#4, i#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{forof_break#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %seq, [seq#4], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [sum#4], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [i#4], %0
// CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [sum#4], %0
// CHECK-NEXT:  %5 = LoadFrameInst [seq#4], %0
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
// CHECK-NEXT:  %17 = LoadFrameInst [sum#4], %0
// CHECK-NEXT:  %18 = ReturnInst %17
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = CatchInst
// CHECK-NEXT:  %20 = IteratorCloseInst %6, true : boolean
// CHECK-NEXT:  %21 = ThrowInst %19
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %22 = StoreFrameInst %12, [i#4], %0
// CHECK-NEXT:  %23 = LoadFrameInst [i#4], %0
// CHECK-NEXT:  %24 = BinaryOperatorInst '<', %23, 0 : number
// CHECK-NEXT:  %25 = CondBranchInst %24, %BB6, %BB7
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %26 = BranchInst %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %27 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %28 = LoadFrameInst [sum#4], %0
// CHECK-NEXT:  %29 = LoadFrameInst [i#4], %0
// CHECK-NEXT:  %30 = BinaryOperatorInst '+', %28, %29
// CHECK-NEXT:  %31 = StoreFrameInst %30, [sum#4], %0
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

// CHECK:function forof_continue#0#1(seq)#5
// CHECK-NEXT:frame = [seq#5, sum#5, i#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{forof_continue#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %seq, [seq#5], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [sum#5], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [i#5], %0
// CHECK-NEXT:  %4 = StoreFrameInst 0 : number, [sum#5], %0
// CHECK-NEXT:  %5 = LoadFrameInst [seq#5], %0
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
// CHECK-NEXT:  %17 = LoadFrameInst [sum#5], %0
// CHECK-NEXT:  %18 = ReturnInst %17
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = CatchInst
// CHECK-NEXT:  %20 = IteratorCloseInst %6, true : boolean
// CHECK-NEXT:  %21 = ThrowInst %19
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %22 = StoreFrameInst %12, [i#5], %0
// CHECK-NEXT:  %23 = LoadFrameInst [i#5], %0
// CHECK-NEXT:  %24 = BinaryOperatorInst '<', %23, 0 : number
// CHECK-NEXT:  %25 = CondBranchInst %24, %BB6, %BB7
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %26 = BranchInst %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %27 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %28 = LoadFrameInst [sum#5], %0
// CHECK-NEXT:  %29 = LoadFrameInst [i#5], %0
// CHECK-NEXT:  %30 = BinaryOperatorInst '+', %28, %29
// CHECK-NEXT:  %31 = StoreFrameInst %30, [sum#5], %0
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
