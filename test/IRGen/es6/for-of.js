/**
 * Copyright (c) Facebook, Inc. and its affiliates.
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
//CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %5 = LoadPropertyInst %4, "iterator" : string
//CHECK-NEXT:  %6 = LoadPropertyInst %3, %5
//CHECK-NEXT:  %7 = CallInst %6, %3
//CHECK-NEXT:  %8 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %7, "iterator is not an object" : string
//CHECK-NEXT:  %9 = LoadPropertyInst %7, "next" : string
//CHECK-NEXT:  %10 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %11 = CallInst %9, %7
//CHECK-NEXT:  %12 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %11, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %13 = LoadPropertyInst %11, "done" : string
//CHECK-NEXT:  %14 = CondBranchInst %13, %BB2, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %15 = LoadPropertyInst %11, "value" : string
//CHECK-NEXT:  %16 = TryStartInst %BB4, %BB5
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %17 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %18 = CatchInst
//CHECK-NEXT:  %19 = LoadPropertyInst %7, "return" : string
//CHECK-NEXT:  %20 = CompareBranchInst '===', %19, undefined : undefined, %BB6, %BB7
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %21 = StoreFrameInst %15, [i]
//CHECK-NEXT:  %22 = LoadFrameInst [cb]
//CHECK-NEXT:  %23 = LoadFrameInst [i]
//CHECK-NEXT:  %24 = CallInst %22, undefined : undefined, %23
//CHECK-NEXT:  %25 = BranchInst %BB8
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %26 = TryEndInst
//CHECK-NEXT:  %27 = BranchInst %BB1
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %28 = TryStartInst %BB9, %BB10
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %29 = ThrowInst %18
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %30 = CatchInst
//CHECK-NEXT:  %31 = BranchInst %BB6
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %32 = CallInst %19, %7
//CHECK-NEXT:  %33 = BranchInst %BB11
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %34 = TryEndInst
//CHECK-NEXT:  %35 = BranchInst %BB6
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
//CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %8 = LoadPropertyInst %7, "iterator" : string
//CHECK-NEXT:  %9 = LoadPropertyInst %6, %8
//CHECK-NEXT:  %10 = CallInst %9, %6
//CHECK-NEXT:  %11 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %10, "iterator is not an object" : string
//CHECK-NEXT:  %12 = LoadPropertyInst %10, "next" : string
//CHECK-NEXT:  %13 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %14 = CallInst %12, %10
//CHECK-NEXT:  %15 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %14, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %16 = LoadPropertyInst %14, "done" : string
//CHECK-NEXT:  %17 = CondBranchInst %16, %BB2, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %18 = LoadPropertyInst %14, "value" : string
//CHECK-NEXT:  %19 = TryStartInst %BB4, %BB5
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %20 = LoadFrameInst [ar]
//CHECK-NEXT:  %21 = ReturnInst %20
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %22 = CatchInst
//CHECK-NEXT:  %23 = LoadPropertyInst %10, "return" : string
//CHECK-NEXT:  %24 = CompareBranchInst '===', %23, undefined : undefined, %BB6, %BB7
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %25 = LoadFrameInst [ar]
//CHECK-NEXT:  %26 = LoadFrameInst [i]
//CHECK-NEXT:  %27 = AsNumberInst %26
//CHECK-NEXT:  %28 = BinaryOperatorInst '+', %27 : number, 1 : number
//CHECK-NEXT:  %29 = StoreFrameInst %28, [i]
//CHECK-NEXT:  %30 = StorePropertyInst %18, %25, %27 : number
//CHECK-NEXT:  %31 = BranchInst %BB8
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %32 = TryEndInst
//CHECK-NEXT:  %33 = BranchInst %BB1
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %34 = TryStartInst %BB9, %BB10
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %35 = ThrowInst %22
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %36 = CatchInst
//CHECK-NEXT:  %37 = BranchInst %BB6
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %38 = CallInst %23, %10
//CHECK-NEXT:  %39 = BranchInst %BB11
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %40 = TryEndInst
//CHECK-NEXT:  %41 = BranchInst %BB6
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %42 = ReturnInst undefined : undefined
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
//CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %6 = LoadPropertyInst %5, "iterator" : string
//CHECK-NEXT:  %7 = LoadPropertyInst %4, %6
//CHECK-NEXT:  %8 = CallInst %7, %4
//CHECK-NEXT:  %9 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %8, "iterator is not an object" : string
//CHECK-NEXT:  %10 = LoadPropertyInst %8, "next" : string
//CHECK-NEXT:  %11 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %12 = CallInst %10, %8
//CHECK-NEXT:  %13 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %12, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %14 = LoadPropertyInst %12, "done" : string
//CHECK-NEXT:  %15 = CondBranchInst %14, %BB2, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %16 = LoadPropertyInst %12, "value" : string
//CHECK-NEXT:  %17 = TryStartInst %BB4, %BB5
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %18 = LoadFrameInst [sum]
//CHECK-NEXT:  %19 = ReturnInst %18
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %20 = CatchInst
//CHECK-NEXT:  %21 = LoadPropertyInst %8, "return" : string
//CHECK-NEXT:  %22 = CompareBranchInst '===', %21, undefined : undefined, %BB6, %BB7
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %23 = StoreFrameInst %16, [i]
//CHECK-NEXT:  %24 = LoadFrameInst [i]
//CHECK-NEXT:  %25 = BinaryOperatorInst '<', %24, 0 : number
//CHECK-NEXT:  %26 = CondBranchInst %25, %BB8, %BB9
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %27 = BranchInst %BB10
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %28 = BranchInst %BB11
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %29 = LoadFrameInst [sum]
//CHECK-NEXT:  %30 = LoadFrameInst [i]
//CHECK-NEXT:  %31 = BinaryOperatorInst '+', %29, %30
//CHECK-NEXT:  %32 = StoreFrameInst %31, [sum]
//CHECK-NEXT:  %33 = BranchInst %BB12
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %34 = TryEndInst
//CHECK-NEXT:  %35 = LoadPropertyInst %8, "return" : string
//CHECK-NEXT:  %36 = CompareBranchInst '===', %35, undefined : undefined, %BB13, %BB14
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %37 = CallInst %35, %8
//CHECK-NEXT:  %38 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %37, "iterator.return() did not return an object" : string
//CHECK-NEXT:  %39 = BranchInst %BB13
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %40 = BranchInst %BB2
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %41 = BranchInst %BB11
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %42 = TryEndInst
//CHECK-NEXT:  %43 = BranchInst %BB1
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %44 = TryStartInst %BB16, %BB17
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %45 = ThrowInst %20
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %46 = CatchInst
//CHECK-NEXT:  %47 = BranchInst %BB6
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %48 = CallInst %21, %8
//CHECK-NEXT:  %49 = BranchInst %BB18
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %50 = TryEndInst
//CHECK-NEXT:  %51 = BranchInst %BB6
//CHECK-NEXT:%BB19:
//CHECK-NEXT:  %52 = ReturnInst undefined : undefined
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
//CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %6 = LoadPropertyInst %5, "iterator" : string
//CHECK-NEXT:  %7 = LoadPropertyInst %4, %6
//CHECK-NEXT:  %8 = CallInst %7, %4
//CHECK-NEXT:  %9 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %8, "iterator is not an object" : string
//CHECK-NEXT:  %10 = LoadPropertyInst %8, "next" : string
//CHECK-NEXT:  %11 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %12 = CallInst %10, %8
//CHECK-NEXT:  %13 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %12, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %14 = LoadPropertyInst %12, "done" : string
//CHECK-NEXT:  %15 = CondBranchInst %14, %BB2, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %16 = LoadPropertyInst %12, "value" : string
//CHECK-NEXT:  %17 = TryStartInst %BB4, %BB5
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %18 = LoadFrameInst [sum]
//CHECK-NEXT:  %19 = ReturnInst %18
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %20 = CatchInst
//CHECK-NEXT:  %21 = LoadPropertyInst %8, "return" : string
//CHECK-NEXT:  %22 = CompareBranchInst '===', %21, undefined : undefined, %BB6, %BB7
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %23 = StoreFrameInst %16, [i]
//CHECK-NEXT:  %24 = LoadFrameInst [i]
//CHECK-NEXT:  %25 = BinaryOperatorInst '<', %24, 0 : number
//CHECK-NEXT:  %26 = CondBranchInst %25, %BB8, %BB9
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %27 = BranchInst %BB10
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %28 = BranchInst %BB11
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %29 = LoadFrameInst [sum]
//CHECK-NEXT:  %30 = LoadFrameInst [i]
//CHECK-NEXT:  %31 = BinaryOperatorInst '+', %29, %30
//CHECK-NEXT:  %32 = StoreFrameInst %31, [sum]
//CHECK-NEXT:  %33 = BranchInst %BB12
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %34 = TryEndInst
//CHECK-NEXT:  %35 = BranchInst %BB1
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %36 = BranchInst %BB11
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %37 = TryEndInst
//CHECK-NEXT:  %38 = BranchInst %BB1
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %39 = TryStartInst %BB14, %BB15
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %40 = ThrowInst %20
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %41 = CatchInst
//CHECK-NEXT:  %42 = BranchInst %BB6
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %43 = CallInst %21, %8
//CHECK-NEXT:  %44 = BranchInst %BB16
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %45 = TryEndInst
//CHECK-NEXT:  %46 = BranchInst %BB6
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %47 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
