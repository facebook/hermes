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
//CHECK-LABEL: function forof_normal(seq, cb)
//CHECK-NEXT: frame = [i, seq, cb]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:   %1 = StoreFrameInst %seq, [seq]
//CHECK-NEXT:   %2 = StoreFrameInst %cb, [cb]
//CHECK-NEXT:   %3 = LoadFrameInst [seq]
//CHECK-NEXT:   %4 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:   %5 = LoadPropertyInst %4, "iterator" : string
//CHECK-NEXT:   %6 = LoadPropertyInst %3, %5
//CHECK-NEXT:   %7 = CallInst %6, %3
//CHECK-NEXT:   %8 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:   %9 = LoadPropertyInst %8, "ensureObject" : string
//CHECK-NEXT:   %10 = CallInst %9, undefined : undefined, %7, "iterator is not an object" : string
//CHECK-NEXT:   %11 = LoadPropertyInst %7, "next" : string
//CHECK-NEXT:   %12 = BranchInst %BB1
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %13 = CallInst %11, %7
//CHECK-NEXT:   %14 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:   %15 = LoadPropertyInst %14, "ensureObject" : string
//CHECK-NEXT:   %16 = CallInst %15, undefined : undefined, %13, "iterator.next() did not return an object" : string
//CHECK-NEXT:   %17 = LoadPropertyInst %13, "done" : string
//CHECK-NEXT:   %18 = CondBranchInst %17, %BB2, %BB3
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   %19 = LoadPropertyInst %13, "value" : string
//CHECK-NEXT:   %20 = TryStartInst %BB4, %BB5
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %21 = ReturnInst undefined : undefined
//CHECK-NEXT: %BB4:
//CHECK-NEXT:   %22 = CatchInst
//CHECK-NEXT:   %23 = LoadPropertyInst %7, "return" : string
//CHECK-NEXT:   %24 = CompareBranchInst '===', %23, undefined : undefined, %BB6, %BB7
//CHECK-NEXT: %BB5:
//CHECK-NEXT:   %25 = StoreFrameInst %19, [i]
//CHECK-NEXT:   %26 = LoadFrameInst [cb]
//CHECK-NEXT:   %27 = LoadFrameInst [i]
//CHECK-NEXT:   %28 = CallInst %26, undefined : undefined, %27
//CHECK-NEXT:   %29 = BranchInst %BB8
//CHECK-NEXT: %BB8:
//CHECK-NEXT:   %30 = TryEndInst
//CHECK-NEXT:   %31 = BranchInst %BB1
//CHECK-NEXT: %BB7:
//CHECK-NEXT:   %32 = TryStartInst %BB9, %BB10
//CHECK-NEXT: %BB6:
//CHECK-NEXT:   %33 = ThrowInst %22
//CHECK-NEXT: %BB9:
//CHECK-NEXT:   %34 = CatchInst
//CHECK-NEXT:   %35 = BranchInst %BB6
//CHECK-NEXT: %BB10:
//CHECK-NEXT:   %36 = CallInst %23, %7
//CHECK-NEXT:   %37 = BranchInst %BB11
//CHECK-NEXT: %BB11:
//CHECK-NEXT:   %38 = TryEndInst
//CHECK-NEXT:   %39 = BranchInst %BB6
//CHECK-NEXT: function_end


function forof_update(seq) {
    var i = 0, ar = [];
    for(ar[i++] of seq);
    return ar;
}
//CHECK-LABEL: function forof_update(seq)
//CHECK-NEXT: frame = [i, ar, seq]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:   %1 = StoreFrameInst undefined : undefined, [ar]
//CHECK-NEXT:   %2 = StoreFrameInst %seq, [seq]
//CHECK-NEXT:   %3 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:   %4 = AllocArrayInst 0 : number
//CHECK-NEXT:   %5 = StoreFrameInst %4 : object, [ar]
//CHECK-NEXT:   %6 = LoadFrameInst [seq]
//CHECK-NEXT:   %7 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:   %8 = LoadPropertyInst %7, "iterator" : string
//CHECK-NEXT:   %9 = LoadPropertyInst %6, %8
//CHECK-NEXT:   %10 = CallInst %9, %6
//CHECK-NEXT:   %11 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:   %12 = LoadPropertyInst %11, "ensureObject" : string
//CHECK-NEXT:   %13 = CallInst %12, undefined : undefined, %10, "iterator is not an object" : string
//CHECK-NEXT:   %14 = LoadPropertyInst %10, "next" : string
//CHECK-NEXT:   %15 = BranchInst %BB1
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %16 = CallInst %14, %10
//CHECK-NEXT:   %17 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:   %18 = LoadPropertyInst %17, "ensureObject" : string
//CHECK-NEXT:   %19 = CallInst %18, undefined : undefined, %16, "iterator.next() did not return an object" : string
//CHECK-NEXT:   %20 = LoadPropertyInst %16, "done" : string
//CHECK-NEXT:   %21 = CondBranchInst %20, %BB2, %BB3
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   %22 = LoadPropertyInst %16, "value" : string
//CHECK-NEXT:   %23 = TryStartInst %BB4, %BB5
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %24 = LoadFrameInst [ar]
//CHECK-NEXT:   %25 = ReturnInst %24
//CHECK-NEXT: %BB4:
//CHECK-NEXT:   %26 = CatchInst
//CHECK-NEXT:   %27 = LoadPropertyInst %10, "return" : string
//CHECK-NEXT:   %28 = CompareBranchInst '===', %27, undefined : undefined, %BB6, %BB7
//CHECK-NEXT: %BB5:
//CHECK-NEXT:   %29 = LoadFrameInst [ar]
//CHECK-NEXT:   %30 = LoadFrameInst [i]
//CHECK-NEXT:   %31 = AsNumberInst %30
//CHECK-NEXT:   %32 = BinaryOperatorInst '+', %31 : number, 1 : number
//CHECK-NEXT:   %33 = StoreFrameInst %32, [i]
//CHECK-NEXT:   %34 = StorePropertyInst %22, %29, %31 : number
//CHECK-NEXT:   %35 = BranchInst %BB8
//CHECK-NEXT: %BB8:
//CHECK-NEXT:   %36 = TryEndInst
//CHECK-NEXT:   %37 = BranchInst %BB1
//CHECK-NEXT: %BB7:
//CHECK-NEXT:   %38 = TryStartInst %BB9, %BB10
//CHECK-NEXT: %BB6:
//CHECK-NEXT:   %39 = ThrowInst %26
//CHECK-NEXT: %BB9:
//CHECK-NEXT:   %40 = CatchInst
//CHECK-NEXT:   %41 = BranchInst %BB6
//CHECK-NEXT: %BB10:
//CHECK-NEXT:   %42 = CallInst %27, %10
//CHECK-NEXT:   %43 = BranchInst %BB11
//CHECK-NEXT: %BB11:
//CHECK-NEXT:   %44 = TryEndInst
//CHECK-NEXT:   %45 = BranchInst %BB6
//CHECK-NEXT: %BB12:
//CHECK-NEXT:   %46 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end


function forof_break(seq) {
    var sum = 0;
    for(var i of seq) {
        if (i < 0)
            break;
        sum += i;
    }
    return sum;
}
//CHECK-LABEL: function forof_break(seq)
//CHECK-NEXT: frame = [sum, i, seq]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [sum]
//CHECK-NEXT:   %1 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:   %2 = StoreFrameInst %seq, [seq]
//CHECK-NEXT:   %3 = StoreFrameInst 0 : number, [sum]
//CHECK-NEXT:   %4 = LoadFrameInst [seq]
//CHECK-NEXT:   %5 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:   %6 = LoadPropertyInst %5, "iterator" : string
//CHECK-NEXT:   %7 = LoadPropertyInst %4, %6
//CHECK-NEXT:   %8 = CallInst %7, %4
//CHECK-NEXT:   %9 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:   %10 = LoadPropertyInst %9, "ensureObject" : string
//CHECK-NEXT:   %11 = CallInst %10, undefined : undefined, %8, "iterator is not an object" : string
//CHECK-NEXT:   %12 = LoadPropertyInst %8, "next" : string
//CHECK-NEXT:   %13 = BranchInst %BB1
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %14 = CallInst %12, %8
//CHECK-NEXT:   %15 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:   %16 = LoadPropertyInst %15, "ensureObject" : string
//CHECK-NEXT:   %17 = CallInst %16, undefined : undefined, %14, "iterator.next() did not return an object" : string
//CHECK-NEXT:   %18 = LoadPropertyInst %14, "done" : string
//CHECK-NEXT:   %19 = CondBranchInst %18, %BB2, %BB3
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   %20 = LoadPropertyInst %14, "value" : string
//CHECK-NEXT:   %21 = TryStartInst %BB4, %BB5
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %22 = LoadFrameInst [sum]
//CHECK-NEXT:   %23 = ReturnInst %22
//CHECK-NEXT: %BB4:
//CHECK-NEXT:   %24 = CatchInst
//CHECK-NEXT:   %25 = LoadPropertyInst %8, "return" : string
//CHECK-NEXT:   %26 = CompareBranchInst '===', %25, undefined : undefined, %BB6, %BB7
//CHECK-NEXT: %BB5:
//CHECK-NEXT:   %27 = StoreFrameInst %20, [i]
//CHECK-NEXT:   %28 = LoadFrameInst [i]
//CHECK-NEXT:   %29 = BinaryOperatorInst '<', %28, 0 : number
//CHECK-NEXT:   %30 = CondBranchInst %29, %BB8, %BB9
//CHECK-NEXT: %BB8:
//CHECK-NEXT:   %31 = BranchInst %BB10
//CHECK-NEXT: %BB9:
//CHECK-NEXT:   %32 = BranchInst %BB11
//CHECK-NEXT: %BB11:
//CHECK-NEXT:   %33 = LoadFrameInst [sum]
//CHECK-NEXT:   %34 = LoadFrameInst [i]
//CHECK-NEXT:   %35 = BinaryOperatorInst '+', %33, %34
//CHECK-NEXT:   %36 = StoreFrameInst %35, [sum]
//CHECK-NEXT:   %37 = BranchInst %BB12
//CHECK-NEXT: %BB10:
//CHECK-NEXT:   %38 = TryEndInst
//CHECK-NEXT:   %39 = LoadPropertyInst %8, "return" : string
//CHECK-NEXT:   %40 = CompareBranchInst '===', %39, undefined : undefined, %BB13, %BB14
//CHECK-NEXT: %BB14:
//CHECK-NEXT:   %41 = CallInst %39, %8
//CHECK-NEXT:   %42 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:   %43 = LoadPropertyInst %42, "ensureObject" : string
//CHECK-NEXT:   %44 = CallInst %43, undefined : undefined, %41, "iterator.close() did not return an object" : string
//CHECK-NEXT:   %45 = BranchInst %BB13
//CHECK-NEXT: %BB13:
//CHECK-NEXT:   %46 = BranchInst %BB2
//CHECK-NEXT: %BB15:
//CHECK-NEXT:   %47 = BranchInst %BB11
//CHECK-NEXT: %BB12:
//CHECK-NEXT:   %48 = TryEndInst
//CHECK-NEXT:   %49 = BranchInst %BB1
//CHECK-NEXT: %BB7:
//CHECK-NEXT:   %50 = TryStartInst %BB16, %BB17
//CHECK-NEXT: %BB6:
//CHECK-NEXT:   %51 = ThrowInst %24
//CHECK-NEXT: %BB16:
//CHECK-NEXT:   %52 = CatchInst
//CHECK-NEXT:   %53 = BranchInst %BB6
//CHECK-NEXT: %BB17:
//CHECK-NEXT:   %54 = CallInst %25, %8
//CHECK-NEXT:   %55 = BranchInst %BB18
//CHECK-NEXT: %BB18:
//CHECK-NEXT:   %56 = TryEndInst
//CHECK-NEXT:   %57 = BranchInst %BB6
//CHECK-NEXT: %BB19:
//CHECK-NEXT:   %58 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end


function forof_continue(seq) {
    var sum = 0;
    for(var i of seq) {
        if (i < 0)
            continue;
        sum += i;
    }
    return sum;
}
//CHECK-LABEL: function forof_continue(seq)
//CHECK-NEXT: frame = [sum, i, seq]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [sum]
//CHECK-NEXT:   %1 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:   %2 = StoreFrameInst %seq, [seq]
//CHECK-NEXT:   %3 = StoreFrameInst 0 : number, [sum]
//CHECK-NEXT:   %4 = LoadFrameInst [seq]
//CHECK-NEXT:   %5 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:   %6 = LoadPropertyInst %5, "iterator" : string
//CHECK-NEXT:   %7 = LoadPropertyInst %4, %6
//CHECK-NEXT:   %8 = CallInst %7, %4
//CHECK-NEXT:   %9 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:   %10 = LoadPropertyInst %9, "ensureObject" : string
//CHECK-NEXT:   %11 = CallInst %10, undefined : undefined, %8, "iterator is not an object" : string
//CHECK-NEXT:   %12 = LoadPropertyInst %8, "next" : string
//CHECK-NEXT:   %13 = BranchInst %BB1
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %14 = CallInst %12, %8
//CHECK-NEXT:   %15 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:   %16 = LoadPropertyInst %15, "ensureObject" : string
//CHECK-NEXT:   %17 = CallInst %16, undefined : undefined, %14, "iterator.next() did not return an object" : string
//CHECK-NEXT:   %18 = LoadPropertyInst %14, "done" : string
//CHECK-NEXT:   %19 = CondBranchInst %18, %BB2, %BB3
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   %20 = LoadPropertyInst %14, "value" : string
//CHECK-NEXT:   %21 = TryStartInst %BB4, %BB5
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %22 = LoadFrameInst [sum]
//CHECK-NEXT:   %23 = ReturnInst %22
//CHECK-NEXT: %BB4:
//CHECK-NEXT:   %24 = CatchInst
//CHECK-NEXT:   %25 = LoadPropertyInst %8, "return" : string
//CHECK-NEXT:   %26 = CompareBranchInst '===', %25, undefined : undefined, %BB6, %BB7
//CHECK-NEXT: %BB5:
//CHECK-NEXT:   %27 = StoreFrameInst %20, [i]
//CHECK-NEXT:   %28 = LoadFrameInst [i]
//CHECK-NEXT:   %29 = BinaryOperatorInst '<', %28, 0 : number
//CHECK-NEXT:   %30 = CondBranchInst %29, %BB8, %BB9
//CHECK-NEXT: %BB8:
//CHECK-NEXT:   %31 = BranchInst %BB10
//CHECK-NEXT: %BB9:
//CHECK-NEXT:   %32 = BranchInst %BB11
//CHECK-NEXT: %BB11:
//CHECK-NEXT:   %33 = LoadFrameInst [sum]
//CHECK-NEXT:   %34 = LoadFrameInst [i]
//CHECK-NEXT:   %35 = BinaryOperatorInst '+', %33, %34
//CHECK-NEXT:   %36 = StoreFrameInst %35, [sum]
//CHECK-NEXT:   %37 = BranchInst %BB12
//CHECK-NEXT: %BB10:
//CHECK-NEXT:   %38 = TryEndInst
//CHECK-NEXT:   %39 = BranchInst %BB1
//CHECK-NEXT: %BB13:
//CHECK-NEXT:   %40 = BranchInst %BB11
//CHECK-NEXT: %BB12:
//CHECK-NEXT:   %41 = TryEndInst
//CHECK-NEXT:   %42 = BranchInst %BB1
//CHECK-NEXT: %BB7:
//CHECK-NEXT:   %43 = TryStartInst %BB14, %BB15
//CHECK-NEXT: %BB6:
//CHECK-NEXT:   %44 = ThrowInst %24
//CHECK-NEXT: %BB14:
//CHECK-NEXT:   %45 = CatchInst
//CHECK-NEXT:   %46 = BranchInst %BB6
//CHECK-NEXT: %BB15:
//CHECK-NEXT:   %47 = CallInst %25, %8
//CHECK-NEXT:   %48 = BranchInst %BB16
//CHECK-NEXT: %BB16:
//CHECK-NEXT:   %49 = TryEndInst
//CHECK-NEXT:   %50 = BranchInst %BB6
//CHECK-NEXT: %BB17:
//CHECK-NEXT:   %51 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
