/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function f1(t) {
    var [...a] = t;
}

function f2(t) {
    var [...[b, c]] = t;
}

function f3(t) {
    for(var[...d] in t);
}

function f4(t) {
    var a, b;
    [a, ...[b[0]]] = t;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [f1, f2, f3, f4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %f1#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "f1" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %f2#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "f2" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %f3#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "f3" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %f4#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "f4" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function f1#0#1(t)#2
// CHECK-NEXT:frame = [t#2, a#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f1#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %t, [t#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [a#2], %0
// CHECK-NEXT:  %3 = LoadFrameInst [t#2], %0
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %5 = AllocStackInst $?anon_1_sourceOrNext
// CHECK-NEXT:  %6 = StoreStackInst %3, %5
// CHECK-NEXT:  %7 = IteratorBeginInst %5
// CHECK-NEXT:  %8 = StoreStackInst %7, %4
// CHECK-NEXT:  %9 = AllocStackInst $?anon_2_iterDone
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = AllocStackInst $?anon_3_iterValue
// CHECK-NEXT:  %12 = AllocStackInst $?anon_4_exc
// CHECK-NEXT:  %13 = AllocArrayInst 0 : number
// CHECK-NEXT:  %14 = AllocStackInst $?anon_5_n
// CHECK-NEXT:  %15 = StoreStackInst 0 : number, %14
// CHECK-NEXT:  %16 = LoadStackInst %9
// CHECK-NEXT:  %17 = CondBranchInst %16, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = LoadStackInst %9
// CHECK-NEXT:  %19 = CondBranchInst %18, %BB4, %BB5
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %20 = IteratorNextInst %4, %5
// CHECK-NEXT:  %21 = LoadStackInst %4
// CHECK-NEXT:  %22 = BinaryOperatorInst '===', %21, undefined : undefined
// CHECK-NEXT:  %23 = StoreStackInst %22, %9
// CHECK-NEXT:  %24 = CondBranchInst %22, %BB1, %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %25 = LoadStackInst %14
// CHECK-NEXT:  %26 = TryStartInst %BB7, %BB8
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %27 = StoreFrameInst %13 : object, [a#2], %0
// CHECK-NEXT:  %28 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %29 = CatchInst
// CHECK-NEXT:  %30 = StoreStackInst %29, %12
// CHECK-NEXT:  %31 = BranchInst %BB3
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %32 = BinaryOperatorInst '+', %25 : number, 1 : number
// CHECK-NEXT:  %33 = StoreStackInst %32 : number, %14
// CHECK-NEXT:  %34 = BranchInst %BB2
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %35 = StorePropertyInst %20, %13 : object, %25 : number
// CHECK-NEXT:  %36 = BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %37 = TryEndInst
// CHECK-NEXT:  %38 = BranchInst %BB9
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %39 = IteratorCloseInst %4, true : boolean
// CHECK-NEXT:  %40 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %41 = LoadStackInst %12
// CHECK-NEXT:  %42 = ThrowInst %41
// CHECK-NEXT:function_end

// CHECK:function f2#0#1(t)#3
// CHECK-NEXT:frame = [t#3, b#3, c#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f2#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %t, [t#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [b#3], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [c#3], %0
// CHECK-NEXT:  %4 = LoadFrameInst [t#3], %0
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_sourceOrNext
// CHECK-NEXT:  %7 = StoreStackInst %4, %6
// CHECK-NEXT:  %8 = IteratorBeginInst %6
// CHECK-NEXT:  %9 = StoreStackInst %8, %5
// CHECK-NEXT:  %10 = AllocStackInst $?anon_2_iterDone
// CHECK-NEXT:  %11 = StoreStackInst undefined : undefined, %10
// CHECK-NEXT:  %12 = AllocStackInst $?anon_3_iterValue
// CHECK-NEXT:  %13 = AllocStackInst $?anon_4_exc
// CHECK-NEXT:  %14 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %15 = LoadStackInst %10
// CHECK-NEXT:  %16 = CondBranchInst %15, %BB4, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %17 = IteratorNextInst %5, %6
// CHECK-NEXT:  %18 = LoadStackInst %5
// CHECK-NEXT:  %19 = BinaryOperatorInst '===', %18, undefined : undefined
// CHECK-NEXT:  %20 = StoreStackInst %19, %10
// CHECK-NEXT:  %21 = CondBranchInst %19, %BB7, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %22 = LoadStackInst %29
// CHECK-NEXT:  %23 = TryStartInst %BB9, %BB10
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %24 = TryStartInst %BB11, %BB12
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %25 = CatchInst
// CHECK-NEXT:  %26 = StoreStackInst %25, %13
// CHECK-NEXT:  %27 = BranchInst %BB3
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %28 = AllocArrayInst 0 : number
// CHECK-NEXT:  %29 = AllocStackInst $?anon_5_n
// CHECK-NEXT:  %30 = StoreStackInst 0 : number, %29
// CHECK-NEXT:  %31 = LoadStackInst %10
// CHECK-NEXT:  %32 = CondBranchInst %31, %BB7, %BB6
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %33 = BranchInst %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %34 = TryEndInst
// CHECK-NEXT:  %35 = BranchInst %BB13
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %36 = CatchInst
// CHECK-NEXT:  %37 = StoreStackInst %36, %13
// CHECK-NEXT:  %38 = BranchInst %BB3
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %39 = BinaryOperatorInst '+', %22 : number, 1 : number
// CHECK-NEXT:  %40 = StoreStackInst %39 : number, %29
// CHECK-NEXT:  %41 = BranchInst %BB6
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %42 = StorePropertyInst %17, %28 : object, %22 : number
// CHECK-NEXT:  %43 = BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %44 = TryEndInst
// CHECK-NEXT:  %45 = BranchInst %BB15
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %46 = CatchInst
// CHECK-NEXT:  %47 = StoreStackInst %46, %13
// CHECK-NEXT:  %48 = BranchInst %BB3
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %49 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %50 = AllocStackInst $?anon_6_iter
// CHECK-NEXT:  %51 = AllocStackInst $?anon_7_sourceOrNext
// CHECK-NEXT:  %52 = StoreStackInst %28 : object, %51
// CHECK-NEXT:  %53 = IteratorBeginInst %51
// CHECK-NEXT:  %54 = StoreStackInst %53, %50
// CHECK-NEXT:  %55 = AllocStackInst $?anon_8_iterDone
// CHECK-NEXT:  %56 = StoreStackInst undefined : undefined, %55
// CHECK-NEXT:  %57 = AllocStackInst $?anon_9_iterValue
// CHECK-NEXT:  %58 = StoreStackInst undefined : undefined, %57
// CHECK-NEXT:  %59 = BranchInst %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %60 = IteratorNextInst %50, %51
// CHECK-NEXT:  %61 = LoadStackInst %50
// CHECK-NEXT:  %62 = BinaryOperatorInst '===', %61, undefined : undefined
// CHECK-NEXT:  %63 = StoreStackInst %62, %55
// CHECK-NEXT:  %64 = CondBranchInst %62, %BB19, %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %65 = StoreStackInst %60, %57
// CHECK-NEXT:  %66 = BranchInst %BB19
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %67 = LoadStackInst %57
// CHECK-NEXT:  %68 = StoreFrameInst %67, [b#3], %0
// CHECK-NEXT:  %69 = StoreStackInst undefined : undefined, %57
// CHECK-NEXT:  %70 = LoadStackInst %55
// CHECK-NEXT:  %71 = CondBranchInst %70, %BB21, %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %72 = IteratorNextInst %50, %51
// CHECK-NEXT:  %73 = LoadStackInst %50
// CHECK-NEXT:  %74 = BinaryOperatorInst '===', %73, undefined : undefined
// CHECK-NEXT:  %75 = StoreStackInst %74, %55
// CHECK-NEXT:  %76 = CondBranchInst %74, %BB21, %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %77 = StoreStackInst %72, %57
// CHECK-NEXT:  %78 = BranchInst %BB21
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %79 = LoadStackInst %57
// CHECK-NEXT:  %80 = StoreFrameInst %79, [c#3], %0
// CHECK-NEXT:  %81 = LoadStackInst %55
// CHECK-NEXT:  %82 = CondBranchInst %81, %BB24, %BB25
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %83 = IteratorCloseInst %50, false : boolean
// CHECK-NEXT:  %84 = BranchInst %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %85 = BranchInst %BB26
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %86 = TryEndInst
// CHECK-NEXT:  %87 = BranchInst %BB17
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %88 = IteratorCloseInst %5, true : boolean
// CHECK-NEXT:  %89 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %90 = LoadStackInst %13
// CHECK-NEXT:  %91 = ThrowInst %90
// CHECK-NEXT:function_end

// CHECK:function f3#0#1(t)#4
// CHECK-NEXT:frame = [t#4, d#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f3#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %t, [t#4], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [d#4], %0
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %4 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %5 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %6 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %7 = LoadFrameInst [t#4], %0
// CHECK-NEXT:  %8 = StoreStackInst %7, %4
// CHECK-NEXT:  %9 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %10 = GetPNamesInst %3, %4, %5, %6, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = GetNextPNameInst %9, %4, %5, %6, %3, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadStackInst %9
// CHECK-NEXT:  %14 = AllocStackInst $?anon_5_iter
// CHECK-NEXT:  %15 = AllocStackInst $?anon_6_sourceOrNext
// CHECK-NEXT:  %16 = StoreStackInst %13, %15
// CHECK-NEXT:  %17 = IteratorBeginInst %15
// CHECK-NEXT:  %18 = StoreStackInst %17, %14
// CHECK-NEXT:  %19 = AllocStackInst $?anon_7_iterDone
// CHECK-NEXT:  %20 = StoreStackInst undefined : undefined, %19
// CHECK-NEXT:  %21 = AllocStackInst $?anon_8_iterValue
// CHECK-NEXT:  %22 = AllocStackInst $?anon_9_exc
// CHECK-NEXT:  %23 = AllocArrayInst 0 : number
// CHECK-NEXT:  %24 = AllocStackInst $?anon_10_n
// CHECK-NEXT:  %25 = StoreStackInst 0 : number, %24
// CHECK-NEXT:  %26 = LoadStackInst %19
// CHECK-NEXT:  %27 = CondBranchInst %26, %BB4, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %28 = LoadStackInst %19
// CHECK-NEXT:  %29 = CondBranchInst %28, %BB7, %BB8
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %30 = IteratorNextInst %14, %15
// CHECK-NEXT:  %31 = LoadStackInst %14
// CHECK-NEXT:  %32 = BinaryOperatorInst '===', %31, undefined : undefined
// CHECK-NEXT:  %33 = StoreStackInst %32, %19
// CHECK-NEXT:  %34 = CondBranchInst %32, %BB4, %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %35 = LoadStackInst %24
// CHECK-NEXT:  %36 = TryStartInst %BB10, %BB11
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %37 = StoreFrameInst %23 : object, [d#4], %0
// CHECK-NEXT:  %38 = BranchInst %BB2
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %39 = CatchInst
// CHECK-NEXT:  %40 = StoreStackInst %39, %22
// CHECK-NEXT:  %41 = BranchInst %BB6
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %42 = BinaryOperatorInst '+', %35 : number, 1 : number
// CHECK-NEXT:  %43 = StoreStackInst %42 : number, %24
// CHECK-NEXT:  %44 = BranchInst %BB5
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %45 = StorePropertyInst %30, %23 : object, %35 : number
// CHECK-NEXT:  %46 = BranchInst %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %47 = TryEndInst
// CHECK-NEXT:  %48 = BranchInst %BB12
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %49 = IteratorCloseInst %14, true : boolean
// CHECK-NEXT:  %50 = BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %51 = LoadStackInst %22
// CHECK-NEXT:  %52 = ThrowInst %51
// CHECK-NEXT:function_end

// CHECK:function f4#0#1(t)#5
// CHECK-NEXT:frame = [t#5, a#5, b#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f4#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %t, [t#5], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [a#5], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [b#5], %0
// CHECK-NEXT:  %4 = LoadFrameInst [t#5], %0
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_sourceOrNext
// CHECK-NEXT:  %7 = StoreStackInst %4, %6
// CHECK-NEXT:  %8 = IteratorBeginInst %6
// CHECK-NEXT:  %9 = StoreStackInst %8, %5
// CHECK-NEXT:  %10 = AllocStackInst $?anon_2_iterDone
// CHECK-NEXT:  %11 = StoreStackInst undefined : undefined, %10
// CHECK-NEXT:  %12 = AllocStackInst $?anon_3_iterValue
// CHECK-NEXT:  %13 = AllocStackInst $?anon_4_exc
// CHECK-NEXT:  %14 = StoreStackInst undefined : undefined, %12
// CHECK-NEXT:  %15 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %16 = LoadStackInst %10
// CHECK-NEXT:  %17 = CondBranchInst %16, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %18 = IteratorNextInst %5, %6
// CHECK-NEXT:  %19 = LoadStackInst %5
// CHECK-NEXT:  %20 = BinaryOperatorInst '===', %19, undefined : undefined
// CHECK-NEXT:  %21 = StoreStackInst %20, %10
// CHECK-NEXT:  %22 = CondBranchInst %20, %BB5, %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %23 = StoreStackInst %18, %12
// CHECK-NEXT:  %24 = BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %25 = LoadStackInst %12
// CHECK-NEXT:  %26 = StoreFrameInst %25, [a#5], %0
// CHECK-NEXT:  %27 = TryStartInst %BB7, %BB8
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %28 = IteratorNextInst %5, %6
// CHECK-NEXT:  %29 = LoadStackInst %5
// CHECK-NEXT:  %30 = BinaryOperatorInst '===', %29, undefined : undefined
// CHECK-NEXT:  %31 = StoreStackInst %30, %10
// CHECK-NEXT:  %32 = CondBranchInst %30, %BB10, %BB11
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %33 = LoadStackInst %40
// CHECK-NEXT:  %34 = TryStartInst %BB12, %BB13
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %35 = TryStartInst %BB14, %BB15
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %36 = CatchInst
// CHECK-NEXT:  %37 = StoreStackInst %36, %13
// CHECK-NEXT:  %38 = BranchInst %BB2
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %39 = AllocArrayInst 0 : number
// CHECK-NEXT:  %40 = AllocStackInst $?anon_5_n
// CHECK-NEXT:  %41 = StoreStackInst 0 : number, %40
// CHECK-NEXT:  %42 = LoadStackInst %10
// CHECK-NEXT:  %43 = CondBranchInst %42, %BB10, %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %44 = BranchInst %BB17
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %45 = TryEndInst
// CHECK-NEXT:  %46 = BranchInst %BB16
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %47 = CatchInst
// CHECK-NEXT:  %48 = StoreStackInst %47, %13
// CHECK-NEXT:  %49 = BranchInst %BB2
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %50 = BinaryOperatorInst '+', %33 : number, 1 : number
// CHECK-NEXT:  %51 = StoreStackInst %50 : number, %40
// CHECK-NEXT:  %52 = BranchInst %BB9
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %53 = StorePropertyInst %28, %39 : object, %33 : number
// CHECK-NEXT:  %54 = BranchInst %BB19
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %55 = TryEndInst
// CHECK-NEXT:  %56 = BranchInst %BB18
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %57 = CatchInst
// CHECK-NEXT:  %58 = StoreStackInst %57, %13
// CHECK-NEXT:  %59 = BranchInst %BB2
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %60 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %61 = AllocStackInst $?anon_6_iter
// CHECK-NEXT:  %62 = AllocStackInst $?anon_7_sourceOrNext
// CHECK-NEXT:  %63 = StoreStackInst %39 : object, %62
// CHECK-NEXT:  %64 = IteratorBeginInst %62
// CHECK-NEXT:  %65 = StoreStackInst %64, %61
// CHECK-NEXT:  %66 = AllocStackInst $?anon_8_iterDone
// CHECK-NEXT:  %67 = StoreStackInst undefined : undefined, %66
// CHECK-NEXT:  %68 = AllocStackInst $?anon_9_iterValue
// CHECK-NEXT:  %69 = AllocStackInst $?anon_10_exc
// CHECK-NEXT:  %70 = TryStartInst %BB21, %BB22
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %71 = LoadStackInst %66
// CHECK-NEXT:  %72 = CondBranchInst %71, %BB24, %BB25
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %73 = CatchInst
// CHECK-NEXT:  %74 = StoreStackInst %73, %69
// CHECK-NEXT:  %75 = BranchInst %BB23
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %76 = StoreStackInst undefined : undefined, %68
// CHECK-NEXT:  %77 = BranchInst %BB27
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %78 = LoadFrameInst [b#5], %0
// CHECK-NEXT:  %79 = BranchInst %BB28
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %80 = TryEndInst
// CHECK-NEXT:  %81 = BranchInst %BB26
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %82 = IteratorNextInst %61, %62
// CHECK-NEXT:  %83 = LoadStackInst %61
// CHECK-NEXT:  %84 = BinaryOperatorInst '===', %83, undefined : undefined
// CHECK-NEXT:  %85 = StoreStackInst %84, %66
// CHECK-NEXT:  %86 = CondBranchInst %84, %BB29, %BB30
// CHECK-NEXT:%BB30:
// CHECK-NEXT:  %87 = StoreStackInst %82, %68
// CHECK-NEXT:  %88 = BranchInst %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %89 = TryStartInst %BB31, %BB32
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %90 = CatchInst
// CHECK-NEXT:  %91 = StoreStackInst %90, %69
// CHECK-NEXT:  %92 = BranchInst %BB23
// CHECK-NEXT:%BB33:
// CHECK-NEXT:  %93 = LoadStackInst %66
// CHECK-NEXT:  %94 = CondBranchInst %93, %BB34, %BB35
// CHECK-NEXT:%BB32:
// CHECK-NEXT:  %95 = LoadStackInst %68
// CHECK-NEXT:  %96 = StorePropertyInst %95, %78, 0 : number
// CHECK-NEXT:  %97 = BranchInst %BB36
// CHECK-NEXT:%BB36:
// CHECK-NEXT:  %98 = TryEndInst
// CHECK-NEXT:  %99 = BranchInst %BB33
// CHECK-NEXT:%BB35:
// CHECK-NEXT:  %100 = IteratorCloseInst %61, false : boolean
// CHECK-NEXT:  %101 = BranchInst %BB34
// CHECK-NEXT:%BB34:
// CHECK-NEXT:  %102 = BranchInst %BB37
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %103 = IteratorCloseInst %61, true : boolean
// CHECK-NEXT:  %104 = BranchInst %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %105 = LoadStackInst %69
// CHECK-NEXT:  %106 = ThrowInst %105
// CHECK-NEXT:%BB37:
// CHECK-NEXT:  %107 = TryEndInst
// CHECK-NEXT:  %108 = BranchInst %BB20
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %109 = IteratorCloseInst %5, true : boolean
// CHECK-NEXT:  %110 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %111 = LoadStackInst %13
// CHECK-NEXT:  %112 = ThrowInst %111
// CHECK-NEXT:function_end
