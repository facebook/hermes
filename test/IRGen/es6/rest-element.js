/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck --match-full-lines %s

function f1(t) {
    var [...a] = t;
}
//CHECK-LABEL:function f1(t)
//CHECK-NEXT:frame = [a, t]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [a]
//CHECK-NEXT:  %1 = StoreFrameInst %t, [t]
//CHECK-NEXT:  %2 = LoadFrameInst [t]
//CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %4 = LoadPropertyInst %3, "iterator" : string
//CHECK-NEXT:  %5 = LoadPropertyInst %2, %4
//CHECK-NEXT:  %6 = CallInst %5, %2
//CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %8 = LoadPropertyInst %7, "ensureObject" : string
//CHECK-NEXT:  %9 = CallInst %8, undefined : undefined, %6, "iterator is not an object" : string
//CHECK-NEXT:  %10 = LoadPropertyInst %6, "next" : string
//CHECK-NEXT:  %11 = AllocStackInst $?anon_0_iterDone
//CHECK-NEXT:  %12 = StoreStackInst undefined : undefined, %11
//CHECK-NEXT:  %13 = AllocStackInst $?anon_1_iterValue
//CHECK-NEXT:  %14 = AllocStackInst $?anon_2_exc
//CHECK-NEXT:  %15 = AllocArrayInst 0 : number
//CHECK-NEXT:  %16 = AllocStackInst $?anon_3_n
//CHECK-NEXT:  %17 = StoreStackInst 0 : number, %16
//CHECK-NEXT:  %18 = LoadStackInst %11
//CHECK-NEXT:  %19 = CondBranchInst %18, %BB1, %BB2
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %20 = LoadStackInst %11
//CHECK-NEXT:  %21 = CondBranchInst %20, %BB4, %BB5
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %22 = CallInst %10, %6
//CHECK-NEXT:  %23 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %24 = LoadPropertyInst %23, "ensureObject" : string
//CHECK-NEXT:  %25 = CallInst %24, undefined : undefined, %22, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %26 = LoadPropertyInst %22, "done" : string
//CHECK-NEXT:  %27 = StoreStackInst %26, %11
//CHECK-NEXT:  %28 = CondBranchInst %26, %BB1, %BB6
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %29 = LoadPropertyInst %22, "value" : string
//CHECK-NEXT:  %30 = LoadStackInst %16
//CHECK-NEXT:  %31 = TryStartInst %BB7, %BB8
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %32 = StoreFrameInst %15 : object, [a]
//CHECK-NEXT:  %33 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %34 = CatchInst
//CHECK-NEXT:  %35 = StoreStackInst %34, %14
//CHECK-NEXT:  %36 = BranchInst %BB3
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %37 = BinaryOperatorInst '+', %30 : number, 1 : number
//CHECK-NEXT:  %38 = StoreStackInst %37 : number, %16
//CHECK-NEXT:  %39 = BranchInst %BB2
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %40 = StorePropertyInst %29, %15 : object, %30 : number
//CHECK-NEXT:  %41 = BranchInst %BB10
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %42 = TryEndInst
//CHECK-NEXT:  %43 = BranchInst %BB9
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %44 = LoadPropertyInst %6, "return" : string
//CHECK-NEXT:  %45 = CompareBranchInst '===', %44, undefined : undefined, %BB11, %BB12
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %46 = LoadStackInst %14
//CHECK-NEXT:  %47 = ThrowInst %46
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %48 = TryStartInst %BB13, %BB14
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %49 = BranchInst %BB4
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %50 = CatchInst
//CHECK-NEXT:  %51 = BranchInst %BB11
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %52 = CallInst %44, %6
//CHECK-NEXT:  %53 = BranchInst %BB15
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %54 = TryEndInst
//CHECK-NEXT:  %55 = BranchInst %BB11
//CHECK-NEXT:function_end

function f2(t) {
    var [...[b, c]] = t;
}
//CHECK-LABEL:function f2(t)
//CHECK-NEXT:frame = [b, c, t]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [b]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [c]
//CHECK-NEXT:  %2 = StoreFrameInst %t, [t]
//CHECK-NEXT:  %3 = LoadFrameInst [t]
//CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %5 = LoadPropertyInst %4, "iterator" : string
//CHECK-NEXT:  %6 = LoadPropertyInst %3, %5
//CHECK-NEXT:  %7 = CallInst %6, %3
//CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %9 = LoadPropertyInst %8, "ensureObject" : string
//CHECK-NEXT:  %10 = CallInst %9, undefined : undefined, %7, "iterator is not an object" : string
//CHECK-NEXT:  %11 = LoadPropertyInst %7, "next" : string
//CHECK-NEXT:  %12 = AllocStackInst $?anon_0_iterDone
//CHECK-NEXT:  %13 = StoreStackInst undefined : undefined, %12
//CHECK-NEXT:  %14 = AllocStackInst $?anon_1_iterValue
//CHECK-NEXT:  %15 = StoreStackInst undefined : undefined, %14
//CHECK-NEXT:  %16 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %17 = CallInst %11, %7
//CHECK-NEXT:  %18 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %19 = LoadPropertyInst %18, "ensureObject" : string
//CHECK-NEXT:  %20 = CallInst %19, undefined : undefined, %17, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %21 = LoadPropertyInst %17, "done" : string
//CHECK-NEXT:  %22 = StoreStackInst %21, %12
//CHECK-NEXT:  %23 = CondBranchInst %21, %BB2, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %24 = LoadPropertyInst %17, "value" : string
//CHECK-NEXT:  %25 = StoreStackInst %24, %14
//CHECK-NEXT:  %26 = BranchInst %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %27 = LoadStackInst %14
//CHECK-NEXT:  %28 = StoreFrameInst %27, [b]
//CHECK-NEXT:  %29 = StoreStackInst undefined : undefined, %14
//CHECK-NEXT:  %30 = LoadStackInst %12
//CHECK-NEXT:  %31 = CondBranchInst %30, %BB4, %BB5
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %32 = CallInst %11, %7
//CHECK-NEXT:  %33 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %34 = LoadPropertyInst %33, "ensureObject" : string
//CHECK-NEXT:  %35 = CallInst %34, undefined : undefined, %32, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %36 = LoadPropertyInst %32, "done" : string
//CHECK-NEXT:  %37 = StoreStackInst %36, %12
//CHECK-NEXT:  %38 = CondBranchInst %36, %BB4, %BB6
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %39 = LoadPropertyInst %32, "value" : string
//CHECK-NEXT:  %40 = StoreStackInst %39, %14
//CHECK-NEXT:  %41 = BranchInst %BB4
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %42 = LoadStackInst %14
//CHECK-NEXT:  %43 = StoreFrameInst %42, [c]
//CHECK-NEXT:  %44 = LoadStackInst %12
//CHECK-NEXT:  %45 = CondBranchInst %44, %BB7, %BB8
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %46 = LoadPropertyInst %7, "return" : string
//CHECK-NEXT:  %47 = CompareBranchInst '===', %46, undefined : undefined, %BB9, %BB10
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %48 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %49 = CallInst %46, %7
//CHECK-NEXT:  %50 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %51 = LoadPropertyInst %50, "ensureObject" : string
//CHECK-NEXT:  %52 = CallInst %51, undefined : undefined, %49, "iterator.close() did not return an object" : string
//CHECK-NEXT:  %53 = BranchInst %BB9
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %54 = BranchInst %BB7
//CHECK-NEXT:function_end

function f3(t) {
    for(var[...d] in t);
}
//CHECK-LABEL:function f3(t)
//CHECK-NEXT:frame = [d, t]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [d]
//CHECK-NEXT:  %1 = StoreFrameInst %t, [t]
//CHECK-NEXT:  %2 = AllocStackInst $?anon_0_iter
//CHECK-NEXT:  %3 = AllocStackInst $?anon_1_base
//CHECK-NEXT:  %4 = AllocStackInst $?anon_2_idx
//CHECK-NEXT:  %5 = AllocStackInst $?anon_3_size
//CHECK-NEXT:  %6 = LoadFrameInst [t]
//CHECK-NEXT:  %7 = StoreStackInst %6, %3
//CHECK-NEXT:  %8 = AllocStackInst $?anon_4_prop
//CHECK-NEXT:  %9 = GetPNamesInst %2, %3, %4, %5, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %10 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %11 = GetNextPNameInst %8, %3, %4, %5, %2, %BB1, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %12 = LoadStackInst %8
//CHECK-NEXT:  %13 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %14 = LoadPropertyInst %13, "iterator" : string
//CHECK-NEXT:  %15 = LoadPropertyInst %12, %14
//CHECK-NEXT:  %16 = CallInst %15, %12
//CHECK-NEXT:  %17 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %18 = LoadPropertyInst %17, "ensureObject" : string
//CHECK-NEXT:  %19 = CallInst %18, undefined : undefined, %16, "iterator is not an object" : string
//CHECK-NEXT:  %20 = LoadPropertyInst %16, "next" : string
//CHECK-NEXT:  %21 = AllocStackInst $?anon_5_iterDone
//CHECK-NEXT:  %22 = StoreStackInst undefined : undefined, %21
//CHECK-NEXT:  %23 = AllocStackInst $?anon_6_iterValue
//CHECK-NEXT:  %24 = AllocStackInst $?anon_7_exc
//CHECK-NEXT:  %25 = AllocArrayInst 0 : number
//CHECK-NEXT:  %26 = AllocStackInst $?anon_8_n
//CHECK-NEXT:  %27 = StoreStackInst 0 : number, %26
//CHECK-NEXT:  %28 = LoadStackInst %21
//CHECK-NEXT:  %29 = CondBranchInst %28, %BB4, %BB5
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %30 = LoadStackInst %21
//CHECK-NEXT:  %31 = CondBranchInst %30, %BB7, %BB8
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %32 = CallInst %20, %16
//CHECK-NEXT:  %33 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %34 = LoadPropertyInst %33, "ensureObject" : string
//CHECK-NEXT:  %35 = CallInst %34, undefined : undefined, %32, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %36 = LoadPropertyInst %32, "done" : string
//CHECK-NEXT:  %37 = StoreStackInst %36, %21
//CHECK-NEXT:  %38 = CondBranchInst %36, %BB4, %BB9
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %39 = LoadPropertyInst %32, "value" : string
//CHECK-NEXT:  %40 = LoadStackInst %26
//CHECK-NEXT:  %41 = TryStartInst %BB10, %BB11
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %42 = StoreFrameInst %25 : object, [d]
//CHECK-NEXT:  %43 = BranchInst %BB2
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %44 = CatchInst
//CHECK-NEXT:  %45 = StoreStackInst %44, %24
//CHECK-NEXT:  %46 = BranchInst %BB6
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %47 = BinaryOperatorInst '+', %40 : number, 1 : number
//CHECK-NEXT:  %48 = StoreStackInst %47 : number, %26
//CHECK-NEXT:  %49 = BranchInst %BB5
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %50 = StorePropertyInst %39, %25 : object, %40 : number
//CHECK-NEXT:  %51 = BranchInst %BB13
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %52 = TryEndInst
//CHECK-NEXT:  %53 = BranchInst %BB12
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %54 = LoadPropertyInst %16, "return" : string
//CHECK-NEXT:  %55 = CompareBranchInst '===', %54, undefined : undefined, %BB14, %BB15
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %56 = LoadStackInst %24
//CHECK-NEXT:  %57 = ThrowInst %56
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %58 = TryStartInst %BB16, %BB17
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %59 = BranchInst %BB7
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %60 = CatchInst
//CHECK-NEXT:  %61 = BranchInst %BB14
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %62 = CallInst %54, %16
//CHECK-NEXT:  %63 = BranchInst %BB18
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %64 = TryEndInst
//CHECK-NEXT:  %65 = BranchInst %BB14
//CHECK-NEXT:function_end

function f4(t) {
    var a, b;
    [a, ...[b[0]]] = t;
}
//CHECK-LABEL:function f4(t)
//CHECK-NEXT:frame = [a, b, t]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [a]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [b]
//CHECK-NEXT:  %2 = StoreFrameInst %t, [t]
//CHECK-NEXT:  %3 = LoadFrameInst [t]
//CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %5 = LoadPropertyInst %4, "iterator" : string
//CHECK-NEXT:  %6 = LoadPropertyInst %3, %5
//CHECK-NEXT:  %7 = CallInst %6, %3
//CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %9 = LoadPropertyInst %8, "ensureObject" : string
//CHECK-NEXT:  %10 = CallInst %9, undefined : undefined, %7, "iterator is not an object" : string
//CHECK-NEXT:  %11 = LoadPropertyInst %7, "next" : string
//CHECK-NEXT:  %12 = AllocStackInst $?anon_0_iterDone
//CHECK-NEXT:  %13 = StoreStackInst undefined : undefined, %12
//CHECK-NEXT:  %14 = AllocStackInst $?anon_1_iterValue
//CHECK-NEXT:  %15 = AllocStackInst $?anon_2_exc
//CHECK-NEXT:  %16 = StoreStackInst undefined : undefined, %14
//CHECK-NEXT:  %17 = BranchInst %BB1
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %18 = LoadStackInst %12
//CHECK-NEXT:  %19 = CondBranchInst %18, %BB3, %BB4
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %20 = CallInst %11, %7
//CHECK-NEXT:  %21 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %22 = LoadPropertyInst %21, "ensureObject" : string
//CHECK-NEXT:  %23 = CallInst %22, undefined : undefined, %20, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %24 = LoadPropertyInst %20, "done" : string
//CHECK-NEXT:  %25 = StoreStackInst %24, %12
//CHECK-NEXT:  %26 = CondBranchInst %24, %BB5, %BB6
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %27 = LoadPropertyInst %20, "value" : string
//CHECK-NEXT:  %28 = StoreStackInst %27, %14
//CHECK-NEXT:  %29 = BranchInst %BB5
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %30 = LoadStackInst %14
//CHECK-NEXT:  %31 = StoreFrameInst %30, [a]
//CHECK-NEXT:  %32 = TryStartInst %BB7, %BB8
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %33 = CatchInst
//CHECK-NEXT:  %34 = StoreStackInst %33, %15
//CHECK-NEXT:  %35 = BranchInst %BB2
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %36 = StoreStackInst undefined : undefined, %14
//CHECK-NEXT:  %37 = LoadStackInst %12
//CHECK-NEXT:  %38 = CondBranchInst %37, %BB10, %BB11
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %39 = LoadFrameInst [b]
//CHECK-NEXT:  %40 = BranchInst %BB12
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %41 = TryEndInst
//CHECK-NEXT:  %42 = BranchInst %BB9
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %43 = CallInst %11, %7
//CHECK-NEXT:  %44 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %45 = LoadPropertyInst %44, "ensureObject" : string
//CHECK-NEXT:  %46 = CallInst %45, undefined : undefined, %43, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %47 = LoadPropertyInst %43, "done" : string
//CHECK-NEXT:  %48 = StoreStackInst %47, %12
//CHECK-NEXT:  %49 = CondBranchInst %47, %BB10, %BB13
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %50 = LoadPropertyInst %43, "value" : string
//CHECK-NEXT:  %51 = StoreStackInst %50, %14
//CHECK-NEXT:  %52 = BranchInst %BB10
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %53 = TryStartInst %BB14, %BB15
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %54 = CatchInst
//CHECK-NEXT:  %55 = StoreStackInst %54, %15
//CHECK-NEXT:  %56 = BranchInst %BB2
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %57 = LoadStackInst %12
//CHECK-NEXT:  %58 = CondBranchInst %57, %BB17, %BB18
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %59 = LoadStackInst %14
//CHECK-NEXT:  %60 = StorePropertyInst %59, %39, 0 : number
//CHECK-NEXT:  %61 = BranchInst %BB19
//CHECK-NEXT:%BB19:
//CHECK-NEXT:  %62 = TryEndInst
//CHECK-NEXT:  %63 = BranchInst %BB16
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %64 = LoadPropertyInst %7, "return" : string
//CHECK-NEXT:  %65 = CompareBranchInst '===', %64, undefined : undefined, %BB20, %BB21
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %66 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB21:
//CHECK-NEXT:  %67 = CallInst %64, %7
//CHECK-NEXT:  %68 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %69 = LoadPropertyInst %68, "ensureObject" : string
//CHECK-NEXT:  %70 = CallInst %69, undefined : undefined, %67, "iterator.close() did not return an object" : string
//CHECK-NEXT:  %71 = BranchInst %BB20
//CHECK-NEXT:%BB20:
//CHECK-NEXT:  %72 = BranchInst %BB17
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %73 = LoadPropertyInst %7, "return" : string
//CHECK-NEXT:  %74 = CompareBranchInst '===', %73, undefined : undefined, %BB22, %BB23
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %75 = LoadStackInst %15
//CHECK-NEXT:  %76 = ThrowInst %75
//CHECK-NEXT:%BB23:
//CHECK-NEXT:  %77 = TryStartInst %BB24, %BB25
//CHECK-NEXT:%BB22:
//CHECK-NEXT:  %78 = BranchInst %BB3
//CHECK-NEXT:%BB24:
//CHECK-NEXT:  %79 = CatchInst
//CHECK-NEXT:  %80 = BranchInst %BB22
//CHECK-NEXT:%BB25:
//CHECK-NEXT:  %81 = CallInst %73, %7
//CHECK-NEXT:  %82 = BranchInst %BB26
//CHECK-NEXT:%BB26:
//CHECK-NEXT:  %83 = TryEndInst
//CHECK-NEXT:  %84 = BranchInst %BB22
//CHECK-NEXT:function_end
