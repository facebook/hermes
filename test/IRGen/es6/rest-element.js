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
// CHECK-LABEL: function f1(t)
// CHECK-NEXT: frame = [a, t]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [a]
// CHECK-NEXT:   %1 = StoreFrameInst %t, [t]
// CHECK-NEXT:   %2 = LoadFrameInst [t]
// CHECK-NEXT:   %3 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
// CHECK-NEXT:   %4 = LoadPropertyInst %3, "iterator" : string
// CHECK-NEXT:   %5 = LoadPropertyInst %2, %4
// CHECK-NEXT:   %6 = CallInst %5, %2
// CHECK-NEXT:   %7 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %8 = LoadPropertyInst %7, "ensureObject" : string
// CHECK-NEXT:   %9 = CallInst %8, undefined : undefined, %6, "iterator is not an object" : string
// CHECK-NEXT:   %10 = LoadPropertyInst %6, "next" : string
// CHECK-NEXT:   %11 = AllocStackInst $?anon_0_iterDone
// CHECK-NEXT:   %12 = StoreStackInst undefined : undefined, %11
// CHECK-NEXT:   %13 = AllocStackInst $?anon_1_iterValue
// CHECK-NEXT:   %14 = AllocStackInst $?anon_2_exc
// CHECK-NEXT:   %15 = AllocArrayInst 0 : number
// CHECK-NEXT:   %16 = AllocStackInst $?anon_3_n
// CHECK-NEXT:   %17 = StoreStackInst 0 : number, %16
// CHECK-NEXT:   %18 = LoadStackInst %11
// CHECK-NEXT:   %19 = CondBranchInst %18, %BB1, %BB2
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %20 = LoadStackInst %11
// CHECK-NEXT:   %21 = CondBranchInst %20, %BB4, %BB5
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %22 = CallInst %10, %6
// CHECK-NEXT:   %23 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %24 = LoadPropertyInst %23, "ensureObject" : string
// CHECK-NEXT:   %25 = CallInst %24, undefined : undefined, %22, "iterator.next() did not return an object" : string
// CHECK-NEXT:   %26 = LoadPropertyInst %22, "done" : string
// CHECK-NEXT:   %27 = StoreStackInst %26, %11
// CHECK-NEXT:   %28 = CondBranchInst %26, %BB1, %BB6
// CHECK-NEXT: %BB6:
// CHECK-NEXT:   %29 = LoadPropertyInst %22, "value" : string
// CHECK-NEXT:   %30 = LoadStackInst %16
// CHECK-NEXT:   %31 = TryStartInst %BB7, %BB8
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %32 = StoreFrameInst %15 : object, [a]
// CHECK-NEXT:   %33 = ReturnInst undefined : undefined
// CHECK-NEXT: %BB7:
// CHECK-NEXT:   %34 = CatchInst
// CHECK-NEXT:   %35 = StoreStackInst %34, %14
// CHECK-NEXT:   %36 = BranchInst %BB3
// CHECK-NEXT: %BB9:
// CHECK-NEXT:   %37 = BinaryOperatorInst '+', %30 : number, 1 : number
// CHECK-NEXT:   %38 = StoreStackInst %37 : number, %16
// CHECK-NEXT:   %39 = BranchInst %BB2
// CHECK-NEXT: %BB8:
// CHECK-NEXT:   %40 = StorePropertyInst %29, %15 : object, %30 : number
// CHECK-NEXT:   %41 = BranchInst %BB10
// CHECK-NEXT: %BB10:
// CHECK-NEXT:   %42 = TryEndInst
// CHECK-NEXT:   %43 = BranchInst %BB9
// CHECK-NEXT: %BB5:
// CHECK-NEXT:   %44 = LoadPropertyInst %6, "return" : string
// CHECK-NEXT:   %45 = CompareBranchInst '===', %44, undefined : undefined, %BB11, %BB12
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %46 = LoadStackInst %14
// CHECK-NEXT:   %47 = ThrowInst %46
// CHECK-NEXT: %BB12:
// CHECK-NEXT:   %48 = TryStartInst %BB13, %BB14
// CHECK-NEXT: %BB11:
// CHECK-NEXT:   %49 = BranchInst %BB4
// CHECK-NEXT: %BB13:
// CHECK-NEXT:   %50 = CatchInst
// CHECK-NEXT:   %51 = BranchInst %BB11
// CHECK-NEXT: %BB14:
// CHECK-NEXT:   %52 = CallInst %44, %6
// CHECK-NEXT:   %53 = BranchInst %BB15
// CHECK-NEXT: %BB15:
// CHECK-NEXT:   %54 = TryEndInst
// CHECK-NEXT:   %55 = BranchInst %BB11
// CHECK-NEXT: function_end

function f2(t) {
    var [...[b, c]] = t;
}
// CHECK-LABEL: function f2(t)
// CHECK-NEXT: frame = [b, c, t]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [b]
// CHECK-NEXT:   %1 = StoreFrameInst undefined : undefined, [c]
// CHECK-NEXT:   %2 = StoreFrameInst %t, [t]
// CHECK-NEXT:   %3 = LoadFrameInst [t]
// CHECK-NEXT:   %4 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
// CHECK-NEXT:   %5 = LoadPropertyInst %4, "iterator" : string
// CHECK-NEXT:   %6 = LoadPropertyInst %3, %5
// CHECK-NEXT:   %7 = CallInst %6, %3
// CHECK-NEXT:   %8 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %9 = LoadPropertyInst %8, "ensureObject" : string
// CHECK-NEXT:   %10 = CallInst %9, undefined : undefined, %7, "iterator is not an object" : string
// CHECK-NEXT:   %11 = LoadPropertyInst %7, "next" : string
// CHECK-NEXT:   %12 = AllocStackInst $?anon_0_iterDone
// CHECK-NEXT:   %13 = StoreStackInst undefined : undefined, %12
// CHECK-NEXT:   %14 = AllocStackInst $?anon_1_iterValue
// CHECK-NEXT:   %15 = AllocStackInst $?anon_2_exc
// CHECK-NEXT:   %16 = TryStartInst %BB1, %BB2
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %17 = LoadStackInst %12
// CHECK-NEXT:   %18 = CondBranchInst %17, %BB4, %BB5
// CHECK-NEXT: %BB6:
// CHECK-NEXT:   %19 = CallInst %11, %7
// CHECK-NEXT:   %20 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %21 = LoadPropertyInst %20, "ensureObject" : string
// CHECK-NEXT:   %22 = CallInst %21, undefined : undefined, %19, "iterator.next() did not return an object" : string
// CHECK-NEXT:   %23 = LoadPropertyInst %19, "done" : string
// CHECK-NEXT:   %24 = StoreStackInst %23, %12
// CHECK-NEXT:   %25 = CondBranchInst %23, %BB7, %BB8
// CHECK-NEXT: %BB8:
// CHECK-NEXT:   %26 = LoadPropertyInst %19, "value" : string
// CHECK-NEXT:   %27 = LoadStackInst %34
// CHECK-NEXT:   %28 = TryStartInst %BB9, %BB10
// CHECK-NEXT: %BB7:
// CHECK-NEXT:   %29 = TryStartInst %BB11, %BB12
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %30 = CatchInst
// CHECK-NEXT:   %31 = StoreStackInst %30, %15
// CHECK-NEXT:   %32 = BranchInst %BB3
// CHECK-NEXT: %BB13:
// CHECK-NEXT:   %33 = AllocArrayInst 0 : number
// CHECK-NEXT:   %34 = AllocStackInst $?anon_3_n
// CHECK-NEXT:   %35 = StoreStackInst 0 : number, %34
// CHECK-NEXT:   %36 = LoadStackInst %12
// CHECK-NEXT:   %37 = CondBranchInst %36, %BB7, %BB6
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %38 = BranchInst %BB14
// CHECK-NEXT: %BB14:
// CHECK-NEXT:   %39 = TryEndInst
// CHECK-NEXT:   %40 = BranchInst %BB13
// CHECK-NEXT: %BB9:
// CHECK-NEXT:   %41 = CatchInst
// CHECK-NEXT:   %42 = StoreStackInst %41, %15
// CHECK-NEXT:   %43 = BranchInst %BB3
// CHECK-NEXT: %BB15:
// CHECK-NEXT:   %44 = BinaryOperatorInst '+', %27 : number, 1 : number
// CHECK-NEXT:   %45 = StoreStackInst %44 : number, %34
// CHECK-NEXT:   %46 = BranchInst %BB6
// CHECK-NEXT: %BB10:
// CHECK-NEXT:   %47 = StorePropertyInst %26, %33 : object, %27 : number
// CHECK-NEXT:   %48 = BranchInst %BB16
// CHECK-NEXT: %BB16:
// CHECK-NEXT:   %49 = TryEndInst
// CHECK-NEXT:   %50 = BranchInst %BB15
// CHECK-NEXT: %BB11:
// CHECK-NEXT:   %51 = CatchInst
// CHECK-NEXT:   %52 = StoreStackInst %51, %15
// CHECK-NEXT:   %53 = BranchInst %BB3
// CHECK-NEXT: %BB17:
// CHECK-NEXT:   %54 = ReturnInst undefined : undefined
// CHECK-NEXT: %BB12:
// CHECK-NEXT:   %55 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
// CHECK-NEXT:   %56 = LoadPropertyInst %55, "iterator" : string
// CHECK-NEXT:   %57 = LoadPropertyInst %33 : object, %56
// CHECK-NEXT:   %58 = CallInst %57, %33 : object
// CHECK-NEXT:   %59 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %60 = LoadPropertyInst %59, "ensureObject" : string
// CHECK-NEXT:   %61 = CallInst %60, undefined : undefined, %58, "iterator is not an object" : string
// CHECK-NEXT:   %62 = LoadPropertyInst %58, "next" : string
// CHECK-NEXT:   %63 = AllocStackInst $?anon_4_iterDone
// CHECK-NEXT:   %64 = StoreStackInst undefined : undefined, %63
// CHECK-NEXT:   %65 = AllocStackInst $?anon_5_iterValue
// CHECK-NEXT:   %66 = StoreStackInst undefined : undefined, %65
// CHECK-NEXT:   %67 = BranchInst %BB18
// CHECK-NEXT: %BB18:
// CHECK-NEXT:   %68 = CallInst %62, %58
// CHECK-NEXT:   %69 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %70 = LoadPropertyInst %69, "ensureObject" : string
// CHECK-NEXT:   %71 = CallInst %70, undefined : undefined, %68, "iterator.next() did not return an object" : string
// CHECK-NEXT:   %72 = LoadPropertyInst %68, "done" : string
// CHECK-NEXT:   %73 = StoreStackInst %72, %63
// CHECK-NEXT:   %74 = CondBranchInst %72, %BB19, %BB20
// CHECK-NEXT: %BB20:
// CHECK-NEXT:   %75 = LoadPropertyInst %68, "value" : string
// CHECK-NEXT:   %76 = StoreStackInst %75, %65
// CHECK-NEXT:   %77 = BranchInst %BB19
// CHECK-NEXT: %BB19:
// CHECK-NEXT:   %78 = LoadStackInst %65
// CHECK-NEXT:   %79 = StoreFrameInst %78, [b]
// CHECK-NEXT:   %80 = StoreStackInst undefined : undefined, %65
// CHECK-NEXT:   %81 = LoadStackInst %63
// CHECK-NEXT:   %82 = CondBranchInst %81, %BB21, %BB22
// CHECK-NEXT: %BB22:
// CHECK-NEXT:   %83 = CallInst %62, %58
// CHECK-NEXT:   %84 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %85 = LoadPropertyInst %84, "ensureObject" : string
// CHECK-NEXT:   %86 = CallInst %85, undefined : undefined, %83, "iterator.next() did not return an object" : string
// CHECK-NEXT:   %87 = LoadPropertyInst %83, "done" : string
// CHECK-NEXT:   %88 = StoreStackInst %87, %63
// CHECK-NEXT:   %89 = CondBranchInst %87, %BB21, %BB23
// CHECK-NEXT: %BB23:
// CHECK-NEXT:   %90 = LoadPropertyInst %83, "value" : string
// CHECK-NEXT:   %91 = StoreStackInst %90, %65
// CHECK-NEXT:   %92 = BranchInst %BB21
// CHECK-NEXT: %BB21:
// CHECK-NEXT:   %93 = LoadStackInst %65
// CHECK-NEXT:   %94 = StoreFrameInst %93, [c]
// CHECK-NEXT:   %95 = LoadStackInst %63
// CHECK-NEXT:   %96 = CondBranchInst %95, %BB24, %BB25
// CHECK-NEXT: %BB25:
// CHECK-NEXT:   %97 = LoadPropertyInst %58, "return" : string
// CHECK-NEXT:   %98 = CompareBranchInst '===', %97, undefined : undefined, %BB26, %BB27
// CHECK-NEXT: %BB24:
// CHECK-NEXT:   %99 = BranchInst %BB28
// CHECK-NEXT: %BB27:
// CHECK-NEXT:   %100 = CallInst %97, %58
// CHECK-NEXT:   %101 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %102 = LoadPropertyInst %101, "ensureObject" : string
// CHECK-NEXT:   %103 = CallInst %102, undefined : undefined, %100, "iterator.close() did not return an object" : string
// CHECK-NEXT:   %104 = BranchInst %BB26
// CHECK-NEXT: %BB26:
// CHECK-NEXT:   %105 = BranchInst %BB24
// CHECK-NEXT: %BB28:
// CHECK-NEXT:   %106 = TryEndInst
// CHECK-NEXT:   %107 = BranchInst %BB17
// CHECK-NEXT: %BB5:
// CHECK-NEXT:   %108 = LoadPropertyInst %7, "return" : string
// CHECK-NEXT:   %109 = CompareBranchInst '===', %108, undefined : undefined, %BB29, %BB30
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %110 = LoadStackInst %15
// CHECK-NEXT:   %111 = ThrowInst %110
// CHECK-NEXT: %BB30:
// CHECK-NEXT:   %112 = TryStartInst %BB31, %BB32
// CHECK-NEXT: %BB29:
// CHECK-NEXT:   %113 = BranchInst %BB4
// CHECK-NEXT: %BB31:
// CHECK-NEXT:   %114 = CatchInst
// CHECK-NEXT:   %115 = BranchInst %BB29
// CHECK-NEXT: %BB32:
// CHECK-NEXT:   %116 = CallInst %108, %7
// CHECK-NEXT:   %117 = BranchInst %BB33
// CHECK-NEXT: %BB33:
// CHECK-NEXT:   %118 = TryEndInst
// CHECK-NEXT:   %119 = BranchInst %BB29
// CHECK-NEXT: function_end


function f3(t) {
    for(var[...d] in t);
}
// CHECK-LABEL: function f3(t)
// CHECK-NEXT: frame = [d, t]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [d]
// CHECK-NEXT:   %1 = StoreFrameInst %t, [t]
// CHECK-NEXT:   %2 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:   %3 = AllocStackInst $?anon_1_base
// CHECK-NEXT:   %4 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:   %5 = AllocStackInst $?anon_3_size
// CHECK-NEXT:   %6 = LoadFrameInst [t]
// CHECK-NEXT:   %7 = StoreStackInst %6, %3
// CHECK-NEXT:   %8 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:   %9 = GetPNamesInst %2, %3, %4, %5, %BB1, %BB2
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %10 = ReturnInst undefined : undefined
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %11 = GetNextPNameInst %8, %3, %4, %5, %2, %BB1, %BB3
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %12 = LoadStackInst %8
// CHECK-NEXT:   %13 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
// CHECK-NEXT:   %14 = LoadPropertyInst %13, "iterator" : string
// CHECK-NEXT:   %15 = LoadPropertyInst %12, %14
// CHECK-NEXT:   %16 = CallInst %15, %12
// CHECK-NEXT:   %17 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %18 = LoadPropertyInst %17, "ensureObject" : string
// CHECK-NEXT:   %19 = CallInst %18, undefined : undefined, %16, "iterator is not an object" : string
// CHECK-NEXT:   %20 = LoadPropertyInst %16, "next" : string
// CHECK-NEXT:   %21 = AllocStackInst $?anon_5_iterDone
// CHECK-NEXT:   %22 = StoreStackInst undefined : undefined, %21
// CHECK-NEXT:   %23 = AllocStackInst $?anon_6_iterValue
// CHECK-NEXT:   %24 = AllocStackInst $?anon_7_exc
// CHECK-NEXT:   %25 = AllocArrayInst 0 : number
// CHECK-NEXT:   %26 = AllocStackInst $?anon_8_n
// CHECK-NEXT:   %27 = StoreStackInst 0 : number, %26
// CHECK-NEXT:   %28 = LoadStackInst %21
// CHECK-NEXT:   %29 = CondBranchInst %28, %BB4, %BB5
// CHECK-NEXT: %BB6:
// CHECK-NEXT:   %30 = LoadStackInst %21
// CHECK-NEXT:   %31 = CondBranchInst %30, %BB7, %BB8
// CHECK-NEXT: %BB5:
// CHECK-NEXT:   %32 = CallInst %20, %16
// CHECK-NEXT:   %33 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %34 = LoadPropertyInst %33, "ensureObject" : string
// CHECK-NEXT:   %35 = CallInst %34, undefined : undefined, %32, "iterator.next() did not return an object" : string
// CHECK-NEXT:   %36 = LoadPropertyInst %32, "done" : string
// CHECK-NEXT:   %37 = StoreStackInst %36, %21
// CHECK-NEXT:   %38 = CondBranchInst %36, %BB4, %BB9
// CHECK-NEXT: %BB9:
// CHECK-NEXT:   %39 = LoadPropertyInst %32, "value" : string
// CHECK-NEXT:   %40 = LoadStackInst %26
// CHECK-NEXT:   %41 = TryStartInst %BB10, %BB11
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %42 = StoreFrameInst %25 : object, [d]
// CHECK-NEXT:   %43 = BranchInst %BB2
// CHECK-NEXT: %BB10:
// CHECK-NEXT:   %44 = CatchInst
// CHECK-NEXT:   %45 = StoreStackInst %44, %24
// CHECK-NEXT:   %46 = BranchInst %BB6
// CHECK-NEXT: %BB12:
// CHECK-NEXT:   %47 = BinaryOperatorInst '+', %40 : number, 1 : number
// CHECK-NEXT:   %48 = StoreStackInst %47 : number, %26
// CHECK-NEXT:   %49 = BranchInst %BB5
// CHECK-NEXT: %BB11:
// CHECK-NEXT:   %50 = StorePropertyInst %39, %25 : object, %40 : number
// CHECK-NEXT:   %51 = BranchInst %BB13
// CHECK-NEXT: %BB13:
// CHECK-NEXT:   %52 = TryEndInst
// CHECK-NEXT:   %53 = BranchInst %BB12
// CHECK-NEXT: %BB8:
// CHECK-NEXT:   %54 = LoadPropertyInst %16, "return" : string
// CHECK-NEXT:   %55 = CompareBranchInst '===', %54, undefined : undefined, %BB14, %BB15
// CHECK-NEXT: %BB7:
// CHECK-NEXT:   %56 = LoadStackInst %24
// CHECK-NEXT:   %57 = ThrowInst %56
// CHECK-NEXT: %BB15:
// CHECK-NEXT:   %58 = TryStartInst %BB16, %BB17
// CHECK-NEXT: %BB14:
// CHECK-NEXT:   %59 = BranchInst %BB7
// CHECK-NEXT: %BB16:
// CHECK-NEXT:   %60 = CatchInst
// CHECK-NEXT:   %61 = BranchInst %BB14
// CHECK-NEXT: %BB17:
// CHECK-NEXT:   %62 = CallInst %54, %16
// CHECK-NEXT:   %63 = BranchInst %BB18
// CHECK-NEXT: %BB18:
// CHECK-NEXT:   %64 = TryEndInst
// CHECK-NEXT:   %65 = BranchInst %BB14
// CHECK-NEXT: function_end

function f4(t) {
    var a, b;
    [a, ...[b[0]]] = t;
}
// CHECK-LABEL: function f4(t)
// CHECK-NEXT: frame = [a, b, t]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [a]
// CHECK-NEXT:   %1 = StoreFrameInst undefined : undefined, [b]
// CHECK-NEXT:   %2 = StoreFrameInst %t, [t]
// CHECK-NEXT:   %3 = LoadFrameInst [t]
// CHECK-NEXT:   %4 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
// CHECK-NEXT:   %5 = LoadPropertyInst %4, "iterator" : string
// CHECK-NEXT:   %6 = LoadPropertyInst %3, %5
// CHECK-NEXT:   %7 = CallInst %6, %3
// CHECK-NEXT:   %8 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %9 = LoadPropertyInst %8, "ensureObject" : string
// CHECK-NEXT:   %10 = CallInst %9, undefined : undefined, %7, "iterator is not an object" : string
// CHECK-NEXT:   %11 = LoadPropertyInst %7, "next" : string
// CHECK-NEXT:   %12 = AllocStackInst $?anon_0_iterDone
// CHECK-NEXT:   %13 = StoreStackInst undefined : undefined, %12
// CHECK-NEXT:   %14 = AllocStackInst $?anon_1_iterValue
// CHECK-NEXT:   %15 = AllocStackInst $?anon_2_exc
// CHECK-NEXT:   %16 = StoreStackInst undefined : undefined, %14
// CHECK-NEXT:   %17 = BranchInst %BB1
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %18 = LoadStackInst %12
// CHECK-NEXT:   %19 = CondBranchInst %18, %BB3, %BB4
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %20 = CallInst %11, %7
// CHECK-NEXT:   %21 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %22 = LoadPropertyInst %21, "ensureObject" : string
// CHECK-NEXT:   %23 = CallInst %22, undefined : undefined, %20, "iterator.next() did not return an object" : string
// CHECK-NEXT:   %24 = LoadPropertyInst %20, "done" : string
// CHECK-NEXT:   %25 = StoreStackInst %24, %12
// CHECK-NEXT:   %26 = CondBranchInst %24, %BB5, %BB6
// CHECK-NEXT: %BB6:
// CHECK-NEXT:   %27 = LoadPropertyInst %20, "value" : string
// CHECK-NEXT:   %28 = StoreStackInst %27, %14
// CHECK-NEXT:   %29 = BranchInst %BB5
// CHECK-NEXT: %BB5:
// CHECK-NEXT:   %30 = LoadStackInst %14
// CHECK-NEXT:   %31 = StoreFrameInst %30, [a]
// CHECK-NEXT:   %32 = TryStartInst %BB7, %BB8
// CHECK-NEXT: %BB9:
// CHECK-NEXT:   %33 = CallInst %11, %7
// CHECK-NEXT:   %34 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %35 = LoadPropertyInst %34, "ensureObject" : string
// CHECK-NEXT:   %36 = CallInst %35, undefined : undefined, %33, "iterator.next() did not return an object" : string
// CHECK-NEXT:   %37 = LoadPropertyInst %33, "done" : string
// CHECK-NEXT:   %38 = StoreStackInst %37, %12
// CHECK-NEXT:   %39 = CondBranchInst %37, %BB10, %BB11
// CHECK-NEXT: %BB11:
// CHECK-NEXT:   %40 = LoadPropertyInst %33, "value" : string
// CHECK-NEXT:   %41 = LoadStackInst %48
// CHECK-NEXT:   %42 = TryStartInst %BB12, %BB13
// CHECK-NEXT: %BB10:
// CHECK-NEXT:   %43 = TryStartInst %BB14, %BB15
// CHECK-NEXT: %BB7:
// CHECK-NEXT:   %44 = CatchInst
// CHECK-NEXT:   %45 = StoreStackInst %44, %15
// CHECK-NEXT:   %46 = BranchInst %BB2
// CHECK-NEXT: %BB16:
// CHECK-NEXT:   %47 = AllocArrayInst 0 : number
// CHECK-NEXT:   %48 = AllocStackInst $?anon_3_n
// CHECK-NEXT:   %49 = StoreStackInst 0 : number, %48
// CHECK-NEXT:   %50 = LoadStackInst %12
// CHECK-NEXT:   %51 = CondBranchInst %50, %BB10, %BB9
// CHECK-NEXT: %BB8:
// CHECK-NEXT:   %52 = BranchInst %BB17
// CHECK-NEXT: %BB17:
// CHECK-NEXT:   %53 = TryEndInst
// CHECK-NEXT:   %54 = BranchInst %BB16
// CHECK-NEXT: %BB12:
// CHECK-NEXT:   %55 = CatchInst
// CHECK-NEXT:   %56 = StoreStackInst %55, %15
// CHECK-NEXT:   %57 = BranchInst %BB2
// CHECK-NEXT: %BB18:
// CHECK-NEXT:   %58 = BinaryOperatorInst '+', %41 : number, 1 : number
// CHECK-NEXT:   %59 = StoreStackInst %58 : number, %48
// CHECK-NEXT:   %60 = BranchInst %BB9
// CHECK-NEXT: %BB13:
// CHECK-NEXT:   %61 = StorePropertyInst %40, %47 : object, %41 : number
// CHECK-NEXT:   %62 = BranchInst %BB19
// CHECK-NEXT: %BB19:
// CHECK-NEXT:   %63 = TryEndInst
// CHECK-NEXT:   %64 = BranchInst %BB18
// CHECK-NEXT: %BB14:
// CHECK-NEXT:   %65 = CatchInst
// CHECK-NEXT:   %66 = StoreStackInst %65, %15
// CHECK-NEXT:   %67 = BranchInst %BB2
// CHECK-NEXT: %BB20:
// CHECK-NEXT:   %68 = ReturnInst undefined : undefined
// CHECK-NEXT: %BB15:
// CHECK-NEXT:   %69 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
// CHECK-NEXT:   %70 = LoadPropertyInst %69, "iterator" : string
// CHECK-NEXT:   %71 = LoadPropertyInst %47 : object, %70
// CHECK-NEXT:   %72 = CallInst %71, %47 : object
// CHECK-NEXT:   %73 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %74 = LoadPropertyInst %73, "ensureObject" : string
// CHECK-NEXT:   %75 = CallInst %74, undefined : undefined, %72, "iterator is not an object" : string
// CHECK-NEXT:   %76 = LoadPropertyInst %72, "next" : string
// CHECK-NEXT:   %77 = AllocStackInst $?anon_4_iterDone
// CHECK-NEXT:   %78 = StoreStackInst undefined : undefined, %77
// CHECK-NEXT:   %79 = AllocStackInst $?anon_5_iterValue
// CHECK-NEXT:   %80 = AllocStackInst $?anon_6_exc
// CHECK-NEXT:   %81 = TryStartInst %BB21, %BB22
// CHECK-NEXT: %BB23:
// CHECK-NEXT:   %82 = LoadStackInst %77
// CHECK-NEXT:   %83 = CondBranchInst %82, %BB24, %BB25
// CHECK-NEXT: %BB21:
// CHECK-NEXT:   %84 = CatchInst
// CHECK-NEXT:   %85 = StoreStackInst %84, %80
// CHECK-NEXT:   %86 = BranchInst %BB23
// CHECK-NEXT: %BB26:
// CHECK-NEXT:   %87 = StoreStackInst undefined : undefined, %79
// CHECK-NEXT:   %88 = BranchInst %BB27
// CHECK-NEXT: %BB22:
// CHECK-NEXT:   %89 = LoadFrameInst [b]
// CHECK-NEXT:   %90 = BranchInst %BB28
// CHECK-NEXT: %BB28:
// CHECK-NEXT:   %91 = TryEndInst
// CHECK-NEXT:   %92 = BranchInst %BB26
// CHECK-NEXT: %BB27:
// CHECK-NEXT:   %93 = CallInst %76, %72
// CHECK-NEXT:   %94 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %95 = LoadPropertyInst %94, "ensureObject" : string
// CHECK-NEXT:   %96 = CallInst %95, undefined : undefined, %93, "iterator.next() did not return an object" : string
// CHECK-NEXT:   %97 = LoadPropertyInst %93, "done" : string
// CHECK-NEXT:   %98 = StoreStackInst %97, %77
// CHECK-NEXT:   %99 = CondBranchInst %97, %BB29, %BB30
// CHECK-NEXT: %BB30:
// CHECK-NEXT:   %100 = LoadPropertyInst %93, "value" : string
// CHECK-NEXT:   %101 = StoreStackInst %100, %79
// CHECK-NEXT:   %102 = BranchInst %BB29
// CHECK-NEXT: %BB29:
// CHECK-NEXT:   %103 = TryStartInst %BB31, %BB32
// CHECK-NEXT: %BB31:
// CHECK-NEXT:   %104 = CatchInst
// CHECK-NEXT:   %105 = StoreStackInst %104, %80
// CHECK-NEXT:   %106 = BranchInst %BB23
// CHECK-NEXT: %BB33:
// CHECK-NEXT:   %107 = LoadStackInst %77
// CHECK-NEXT:   %108 = CondBranchInst %107, %BB34, %BB35
// CHECK-NEXT: %BB32:
// CHECK-NEXT:   %109 = LoadStackInst %79
// CHECK-NEXT:   %110 = StorePropertyInst %109, %89, 0 : number
// CHECK-NEXT:   %111 = BranchInst %BB36
// CHECK-NEXT: %BB36:
// CHECK-NEXT:   %112 = TryEndInst
// CHECK-NEXT:   %113 = BranchInst %BB33
// CHECK-NEXT: %BB35:
// CHECK-NEXT:   %114 = LoadPropertyInst %72, "return" : string
// CHECK-NEXT:   %115 = CompareBranchInst '===', %114, undefined : undefined, %BB37, %BB38
// CHECK-NEXT: %BB34:
// CHECK-NEXT:   %116 = BranchInst %BB39
// CHECK-NEXT: %BB38:
// CHECK-NEXT:   %117 = CallInst %114, %72
// CHECK-NEXT:   %118 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHECK-NEXT:   %119 = LoadPropertyInst %118, "ensureObject" : string
// CHECK-NEXT:   %120 = CallInst %119, undefined : undefined, %117, "iterator.close() did not return an object" : string
// CHECK-NEXT:   %121 = BranchInst %BB37
// CHECK-NEXT: %BB37:
// CHECK-NEXT:   %122 = BranchInst %BB34
// CHECK-NEXT: %BB25:
// CHECK-NEXT:   %123 = LoadPropertyInst %72, "return" : string
// CHECK-NEXT:   %124 = CompareBranchInst '===', %123, undefined : undefined, %BB40, %BB41
// CHECK-NEXT: %BB24:
// CHECK-NEXT:   %125 = LoadStackInst %80
// CHECK-NEXT:   %126 = ThrowInst %125
// CHECK-NEXT: %BB41:
// CHECK-NEXT:   %127 = TryStartInst %BB42, %BB43
// CHECK-NEXT: %BB40:
// CHECK-NEXT:   %128 = BranchInst %BB24
// CHECK-NEXT: %BB42:
// CHECK-NEXT:   %129 = CatchInst
// CHECK-NEXT:   %130 = BranchInst %BB40
// CHECK-NEXT: %BB43:
// CHECK-NEXT:   %131 = CallInst %123, %72
// CHECK-NEXT:   %132 = BranchInst %BB44
// CHECK-NEXT: %BB44:
// CHECK-NEXT:   %133 = TryEndInst
// CHECK-NEXT:   %134 = BranchInst %BB40
// CHECK-NEXT: %BB39:
// CHECK-NEXT:   %135 = TryEndInst
// CHECK-NEXT:   %136 = BranchInst %BB20
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %137 = LoadPropertyInst %7, "return" : string
// CHECK-NEXT:   %138 = CompareBranchInst '===', %137, undefined : undefined, %BB45, %BB46
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %139 = LoadStackInst %15
// CHECK-NEXT:   %140 = ThrowInst %139
// CHECK-NEXT: %BB46:
// CHECK-NEXT:   %141 = TryStartInst %BB47, %BB48
// CHECK-NEXT: %BB45:
// CHECK-NEXT:   %142 = BranchInst %BB3
// CHECK-NEXT: %BB47:
// CHECK-NEXT:   %143 = CatchInst
// CHECK-NEXT:   %144 = BranchInst %BB45
// CHECK-NEXT: %BB48:
// CHECK-NEXT:   %145 = CallInst %137, %7
// CHECK-NEXT:   %146 = BranchInst %BB49
// CHECK-NEXT: %BB49:
// CHECK-NEXT:   %147 = TryEndInst
// CHECK-NEXT:   %148 = BranchInst %BB45
// CHECK-NEXT: function_end
