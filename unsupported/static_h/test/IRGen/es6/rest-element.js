/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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
//CHECK-NEXT:  %3 = AllocStackInst $?anon_0_iter
//CHECK-NEXT:  %4 = AllocStackInst $?anon_1_sourceOrNext
//CHECK-NEXT:  %5 = StoreStackInst %2, %4
//CHECK-NEXT:  %6 = IteratorBeginInst %4
//CHECK-NEXT:  %7 = StoreStackInst %6, %3
//CHECK-NEXT:  %8 = AllocStackInst $?anon_2_iterDone
//CHECK-NEXT:  %9 = StoreStackInst undefined : undefined, %8
//CHECK-NEXT:  %10 = AllocStackInst $?anon_3_iterValue
//CHECK-NEXT:  %11 = AllocStackInst $?anon_4_exc
//CHECK-NEXT:  %12 = AllocArrayInst 0 : number
//CHECK-NEXT:  %13 = AllocStackInst $?anon_5_n
//CHECK-NEXT:  %14 = StoreStackInst 0 : number, %13
//CHECK-NEXT:  %15 = LoadStackInst %8
//CHECK-NEXT:  %16 = CondBranchInst %15, %BB1, %BB2
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %17 = LoadStackInst %8
//CHECK-NEXT:  %18 = CondBranchInst %17, %BB4, %BB5
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %19 = IteratorNextInst %3, %4
//CHECK-NEXT:  %20 = LoadStackInst %3
//CHECK-NEXT:  %21 = BinaryOperatorInst '===', %20, undefined : undefined
//CHECK-NEXT:  %22 = StoreStackInst %21, %8
//CHECK-NEXT:  %23 = CondBranchInst %21, %BB1, %BB6
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %24 = LoadStackInst %13
//CHECK-NEXT:  %25 = TryStartInst %BB7, %BB8
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %26 = StoreFrameInst %12 : object, [a]
//CHECK-NEXT:  %27 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %28 = CatchInst
//CHECK-NEXT:  %29 = StoreStackInst %28, %11
//CHECK-NEXT:  %30 = BranchInst %BB3
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %31 = BinaryOperatorInst '+', %24 : number, 1 : number
//CHECK-NEXT:  %32 = StoreStackInst %31 : number, %13
//CHECK-NEXT:  %33 = BranchInst %BB2
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %34 = StorePropertyInst %19, %12 : object, %24 : number
//CHECK-NEXT:  %35 = BranchInst %BB10
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %36 = TryEndInst
//CHECK-NEXT:  %37 = BranchInst %BB9
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %38 = IteratorCloseInst %3, true : boolean
//CHECK-NEXT:  %39 = BranchInst %BB4
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %40 = LoadStackInst %11
//CHECK-NEXT:  %41 = ThrowInst %40
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
//CHECK-NEXT:  %4 = AllocStackInst $?anon_0_iter
//CHECK-NEXT:  %5 = AllocStackInst $?anon_1_sourceOrNext
//CHECK-NEXT:  %6 = StoreStackInst %3, %5
//CHECK-NEXT:  %7 = IteratorBeginInst %5
//CHECK-NEXT:  %8 = StoreStackInst %7, %4
//CHECK-NEXT:  %9 = AllocStackInst $?anon_2_iterDone
//CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
//CHECK-NEXT:  %11 = AllocStackInst $?anon_3_iterValue
//CHECK-NEXT:  %12 = AllocStackInst $?anon_4_exc
//CHECK-NEXT:  %13 = TryStartInst %BB1, %BB2
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %14 = LoadStackInst %9
//CHECK-NEXT:  %15 = CondBranchInst %14, %BB4, %BB5
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %16 = IteratorNextInst %4, %5
//CHECK-NEXT:  %17 = LoadStackInst %4
//CHECK-NEXT:  %18 = BinaryOperatorInst '===', %17, undefined : undefined
//CHECK-NEXT:  %19 = StoreStackInst %18, %9
//CHECK-NEXT:  %20 = CondBranchInst %18, %BB7, %BB8
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %21 = LoadStackInst %28
//CHECK-NEXT:  %22 = TryStartInst %BB9, %BB10
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %23 = TryStartInst %BB11, %BB12
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %24 = CatchInst
//CHECK-NEXT:  %25 = StoreStackInst %24, %12
//CHECK-NEXT:  %26 = BranchInst %BB3
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %27 = AllocArrayInst 0 : number
//CHECK-NEXT:  %28 = AllocStackInst $?anon_5_n
//CHECK-NEXT:  %29 = StoreStackInst 0 : number, %28
//CHECK-NEXT:  %30 = LoadStackInst %9
//CHECK-NEXT:  %31 = CondBranchInst %30, %BB7, %BB6
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %32 = BranchInst %BB14
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %33 = TryEndInst
//CHECK-NEXT:  %34 = BranchInst %BB13
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %35 = CatchInst
//CHECK-NEXT:  %36 = StoreStackInst %35, %12
//CHECK-NEXT:  %37 = BranchInst %BB3
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %38 = BinaryOperatorInst '+', %21 : number, 1 : number
//CHECK-NEXT:  %39 = StoreStackInst %38 : number, %28
//CHECK-NEXT:  %40 = BranchInst %BB6
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %41 = StorePropertyInst %16, %27 : object, %21 : number
//CHECK-NEXT:  %42 = BranchInst %BB16
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %43 = TryEndInst
//CHECK-NEXT:  %44 = BranchInst %BB15
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %45 = CatchInst
//CHECK-NEXT:  %46 = StoreStackInst %45, %12
//CHECK-NEXT:  %47 = BranchInst %BB3
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %48 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %49 = AllocStackInst $?anon_6_iter
//CHECK-NEXT:  %50 = AllocStackInst $?anon_7_sourceOrNext
//CHECK-NEXT:  %51 = StoreStackInst %27 : object, %50
//CHECK-NEXT:  %52 = IteratorBeginInst %50
//CHECK-NEXT:  %53 = StoreStackInst %52, %49
//CHECK-NEXT:  %54 = AllocStackInst $?anon_8_iterDone
//CHECK-NEXT:  %55 = StoreStackInst undefined : undefined, %54
//CHECK-NEXT:  %56 = AllocStackInst $?anon_9_iterValue
//CHECK-NEXT:  %57 = StoreStackInst undefined : undefined, %56
//CHECK-NEXT:  %58 = BranchInst %BB18
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %59 = IteratorNextInst %49, %50
//CHECK-NEXT:  %60 = LoadStackInst %49
//CHECK-NEXT:  %61 = BinaryOperatorInst '===', %60, undefined : undefined
//CHECK-NEXT:  %62 = StoreStackInst %61, %54
//CHECK-NEXT:  %63 = CondBranchInst %61, %BB19, %BB20
//CHECK-NEXT:%BB20:
//CHECK-NEXT:  %64 = StoreStackInst %59, %56
//CHECK-NEXT:  %65 = BranchInst %BB19
//CHECK-NEXT:%BB19:
//CHECK-NEXT:  %66 = LoadStackInst %56
//CHECK-NEXT:  %67 = StoreFrameInst %66, [b]
//CHECK-NEXT:  %68 = StoreStackInst undefined : undefined, %56
//CHECK-NEXT:  %69 = LoadStackInst %54
//CHECK-NEXT:  %70 = CondBranchInst %69, %BB21, %BB22
//CHECK-NEXT:%BB22:
//CHECK-NEXT:  %71 = IteratorNextInst %49, %50
//CHECK-NEXT:  %72 = LoadStackInst %49
//CHECK-NEXT:  %73 = BinaryOperatorInst '===', %72, undefined : undefined
//CHECK-NEXT:  %74 = StoreStackInst %73, %54
//CHECK-NEXT:  %75 = CondBranchInst %73, %BB21, %BB23
//CHECK-NEXT:%BB23:
//CHECK-NEXT:  %76 = StoreStackInst %71, %56
//CHECK-NEXT:  %77 = BranchInst %BB21
//CHECK-NEXT:%BB21:
//CHECK-NEXT:  %78 = LoadStackInst %56
//CHECK-NEXT:  %79 = StoreFrameInst %78, [c]
//CHECK-NEXT:  %80 = LoadStackInst %54
//CHECK-NEXT:  %81 = CondBranchInst %80, %BB24, %BB25
//CHECK-NEXT:%BB25:
//CHECK-NEXT:  %82 = IteratorCloseInst %49, false : boolean
//CHECK-NEXT:  %83 = BranchInst %BB24
//CHECK-NEXT:%BB24:
//CHECK-NEXT:  %84 = BranchInst %BB26
//CHECK-NEXT:%BB26:
//CHECK-NEXT:  %85 = TryEndInst
//CHECK-NEXT:  %86 = BranchInst %BB17
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %87 = IteratorCloseInst %4, true : boolean
//CHECK-NEXT:  %88 = BranchInst %BB4
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %89 = LoadStackInst %12
//CHECK-NEXT:  %90 = ThrowInst %89
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
//CHECK-NEXT:  %13 = AllocStackInst $?anon_5_iter
//CHECK-NEXT:  %14 = AllocStackInst $?anon_6_sourceOrNext
//CHECK-NEXT:  %15 = StoreStackInst %12, %14
//CHECK-NEXT:  %16 = IteratorBeginInst %14
//CHECK-NEXT:  %17 = StoreStackInst %16, %13
//CHECK-NEXT:  %18 = AllocStackInst $?anon_7_iterDone
//CHECK-NEXT:  %19 = StoreStackInst undefined : undefined, %18
//CHECK-NEXT:  %20 = AllocStackInst $?anon_8_iterValue
//CHECK-NEXT:  %21 = AllocStackInst $?anon_9_exc
//CHECK-NEXT:  %22 = AllocArrayInst 0 : number
//CHECK-NEXT:  %23 = AllocStackInst $?anon_10_n
//CHECK-NEXT:  %24 = StoreStackInst 0 : number, %23
//CHECK-NEXT:  %25 = LoadStackInst %18
//CHECK-NEXT:  %26 = CondBranchInst %25, %BB4, %BB5
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %27 = LoadStackInst %18
//CHECK-NEXT:  %28 = CondBranchInst %27, %BB7, %BB8
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %29 = IteratorNextInst %13, %14
//CHECK-NEXT:  %30 = LoadStackInst %13
//CHECK-NEXT:  %31 = BinaryOperatorInst '===', %30, undefined : undefined
//CHECK-NEXT:  %32 = StoreStackInst %31, %18
//CHECK-NEXT:  %33 = CondBranchInst %31, %BB4, %BB9
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %34 = LoadStackInst %23
//CHECK-NEXT:  %35 = TryStartInst %BB10, %BB11
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %36 = StoreFrameInst %22 : object, [d]
//CHECK-NEXT:  %37 = BranchInst %BB2
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %38 = CatchInst
//CHECK-NEXT:  %39 = StoreStackInst %38, %21
//CHECK-NEXT:  %40 = BranchInst %BB6
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %41 = BinaryOperatorInst '+', %34 : number, 1 : number
//CHECK-NEXT:  %42 = StoreStackInst %41 : number, %23
//CHECK-NEXT:  %43 = BranchInst %BB5
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %44 = StorePropertyInst %29, %22 : object, %34 : number
//CHECK-NEXT:  %45 = BranchInst %BB13
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %46 = TryEndInst
//CHECK-NEXT:  %47 = BranchInst %BB12
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %48 = IteratorCloseInst %13, true : boolean
//CHECK-NEXT:  %49 = BranchInst %BB7
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %50 = LoadStackInst %21
//CHECK-NEXT:  %51 = ThrowInst %50
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
//CHECK-NEXT:  %4 = AllocStackInst $?anon_0_iter
//CHECK-NEXT:  %5 = AllocStackInst $?anon_1_sourceOrNext
//CHECK-NEXT:  %6 = StoreStackInst %3, %5
//CHECK-NEXT:  %7 = IteratorBeginInst %5
//CHECK-NEXT:  %8 = StoreStackInst %7, %4
//CHECK-NEXT:  %9 = AllocStackInst $?anon_2_iterDone
//CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
//CHECK-NEXT:  %11 = AllocStackInst $?anon_3_iterValue
//CHECK-NEXT:  %12 = AllocStackInst $?anon_4_exc
//CHECK-NEXT:  %13 = StoreStackInst undefined : undefined, %11
//CHECK-NEXT:  %14 = BranchInst %BB1
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %15 = LoadStackInst %9
//CHECK-NEXT:  %16 = CondBranchInst %15, %BB3, %BB4
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %17 = IteratorNextInst %4, %5
//CHECK-NEXT:  %18 = LoadStackInst %4
//CHECK-NEXT:  %19 = BinaryOperatorInst '===', %18, undefined : undefined
//CHECK-NEXT:  %20 = StoreStackInst %19, %9
//CHECK-NEXT:  %21 = CondBranchInst %19, %BB5, %BB6
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %22 = StoreStackInst %17, %11
//CHECK-NEXT:  %23 = BranchInst %BB5
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %24 = LoadStackInst %11
//CHECK-NEXT:  %25 = StoreFrameInst %24, [a]
//CHECK-NEXT:  %26 = TryStartInst %BB7, %BB8
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %27 = IteratorNextInst %4, %5
//CHECK-NEXT:  %28 = LoadStackInst %4
//CHECK-NEXT:  %29 = BinaryOperatorInst '===', %28, undefined : undefined
//CHECK-NEXT:  %30 = StoreStackInst %29, %9
//CHECK-NEXT:  %31 = CondBranchInst %29, %BB10, %BB11
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %32 = LoadStackInst %39
//CHECK-NEXT:  %33 = TryStartInst %BB12, %BB13
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %34 = TryStartInst %BB14, %BB15
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %35 = CatchInst
//CHECK-NEXT:  %36 = StoreStackInst %35, %12
//CHECK-NEXT:  %37 = BranchInst %BB2
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %38 = AllocArrayInst 0 : number
//CHECK-NEXT:  %39 = AllocStackInst $?anon_5_n
//CHECK-NEXT:  %40 = StoreStackInst 0 : number, %39
//CHECK-NEXT:  %41 = LoadStackInst %9
//CHECK-NEXT:  %42 = CondBranchInst %41, %BB10, %BB9
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %43 = BranchInst %BB17
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %44 = TryEndInst
//CHECK-NEXT:  %45 = BranchInst %BB16
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %46 = CatchInst
//CHECK-NEXT:  %47 = StoreStackInst %46, %12
//CHECK-NEXT:  %48 = BranchInst %BB2
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %49 = BinaryOperatorInst '+', %32 : number, 1 : number
//CHECK-NEXT:  %50 = StoreStackInst %49 : number, %39
//CHECK-NEXT:  %51 = BranchInst %BB9
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %52 = StorePropertyInst %27, %38 : object, %32 : number
//CHECK-NEXT:  %53 = BranchInst %BB19
//CHECK-NEXT:%BB19:
//CHECK-NEXT:  %54 = TryEndInst
//CHECK-NEXT:  %55 = BranchInst %BB18
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %56 = CatchInst
//CHECK-NEXT:  %57 = StoreStackInst %56, %12
//CHECK-NEXT:  %58 = BranchInst %BB2
//CHECK-NEXT:%BB20:
//CHECK-NEXT:  %59 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %60 = AllocStackInst $?anon_6_iter
//CHECK-NEXT:  %61 = AllocStackInst $?anon_7_sourceOrNext
//CHECK-NEXT:  %62 = StoreStackInst %38 : object, %61
//CHECK-NEXT:  %63 = IteratorBeginInst %61
//CHECK-NEXT:  %64 = StoreStackInst %63, %60
//CHECK-NEXT:  %65 = AllocStackInst $?anon_8_iterDone
//CHECK-NEXT:  %66 = StoreStackInst undefined : undefined, %65
//CHECK-NEXT:  %67 = AllocStackInst $?anon_9_iterValue
//CHECK-NEXT:  %68 = AllocStackInst $?anon_10_exc
//CHECK-NEXT:  %69 = TryStartInst %BB21, %BB22
//CHECK-NEXT:%BB23:
//CHECK-NEXT:  %70 = LoadStackInst %65
//CHECK-NEXT:  %71 = CondBranchInst %70, %BB24, %BB25
//CHECK-NEXT:%BB21:
//CHECK-NEXT:  %72 = CatchInst
//CHECK-NEXT:  %73 = StoreStackInst %72, %68
//CHECK-NEXT:  %74 = BranchInst %BB23
//CHECK-NEXT:%BB26:
//CHECK-NEXT:  %75 = StoreStackInst undefined : undefined, %67
//CHECK-NEXT:  %76 = BranchInst %BB27
//CHECK-NEXT:%BB22:
//CHECK-NEXT:  %77 = LoadFrameInst [b]
//CHECK-NEXT:  %78 = BranchInst %BB28
//CHECK-NEXT:%BB28:
//CHECK-NEXT:  %79 = TryEndInst
//CHECK-NEXT:  %80 = BranchInst %BB26
//CHECK-NEXT:%BB27:
//CHECK-NEXT:  %81 = IteratorNextInst %60, %61
//CHECK-NEXT:  %82 = LoadStackInst %60
//CHECK-NEXT:  %83 = BinaryOperatorInst '===', %82, undefined : undefined
//CHECK-NEXT:  %84 = StoreStackInst %83, %65
//CHECK-NEXT:  %85 = CondBranchInst %83, %BB29, %BB30
//CHECK-NEXT:%BB30:
//CHECK-NEXT:  %86 = StoreStackInst %81, %67
//CHECK-NEXT:  %87 = BranchInst %BB29
//CHECK-NEXT:%BB29:
//CHECK-NEXT:  %88 = TryStartInst %BB31, %BB32
//CHECK-NEXT:%BB31:
//CHECK-NEXT:  %89 = CatchInst
//CHECK-NEXT:  %90 = StoreStackInst %89, %68
//CHECK-NEXT:  %91 = BranchInst %BB23
//CHECK-NEXT:%BB33:
//CHECK-NEXT:  %92 = LoadStackInst %65
//CHECK-NEXT:  %93 = CondBranchInst %92, %BB34, %BB35
//CHECK-NEXT:%BB32:
//CHECK-NEXT:  %94 = LoadStackInst %67
//CHECK-NEXT:  %95 = StorePropertyInst %94, %77, 0 : number
//CHECK-NEXT:  %96 = BranchInst %BB36
//CHECK-NEXT:%BB36:
//CHECK-NEXT:  %97 = TryEndInst
//CHECK-NEXT:  %98 = BranchInst %BB33
//CHECK-NEXT:%BB35:
//CHECK-NEXT:  %99 = IteratorCloseInst %60, false : boolean
//CHECK-NEXT:  %100 = BranchInst %BB34
//CHECK-NEXT:%BB34:
//CHECK-NEXT:  %101 = BranchInst %BB37
//CHECK-NEXT:%BB25:
//CHECK-NEXT:  %102 = IteratorCloseInst %60, true : boolean
//CHECK-NEXT:  %103 = BranchInst %BB24
//CHECK-NEXT:%BB24:
//CHECK-NEXT:  %104 = LoadStackInst %68
//CHECK-NEXT:  %105 = ThrowInst %104
//CHECK-NEXT:%BB37:
//CHECK-NEXT:  %106 = TryEndInst
//CHECK-NEXT:  %107 = BranchInst %BB20
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %108 = IteratorCloseInst %4, true : boolean
//CHECK-NEXT:  %109 = BranchInst %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %110 = LoadStackInst %12
//CHECK-NEXT:  %111 = ThrowInst %110
//CHECK-NEXT:function_end
