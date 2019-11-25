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
//CHECK-NEXT:  %7 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %6, "iterator is not an object" : string
//CHECK-NEXT:  %8 = LoadPropertyInst %6, "next" : string
//CHECK-NEXT:  %9 = AllocStackInst $?anon_0_iterDone
//CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
//CHECK-NEXT:  %11 = AllocStackInst $?anon_1_iterValue
//CHECK-NEXT:  %12 = AllocStackInst $?anon_2_exc
//CHECK-NEXT:  %13 = AllocArrayInst 0 : number
//CHECK-NEXT:  %14 = AllocStackInst $?anon_3_n
//CHECK-NEXT:  %15 = StoreStackInst 0 : number, %14
//CHECK-NEXT:  %16 = LoadStackInst %9
//CHECK-NEXT:  %17 = CondBranchInst %16, %BB1, %BB2
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %18 = LoadStackInst %9
//CHECK-NEXT:  %19 = CondBranchInst %18, %BB4, %BB5
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %20 = CallInst %8, %6
//CHECK-NEXT:  %21 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %20, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %22 = LoadPropertyInst %20, "done" : string
//CHECK-NEXT:  %23 = StoreStackInst %22, %9
//CHECK-NEXT:  %24 = CondBranchInst %22, %BB1, %BB6
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %25 = LoadPropertyInst %20, "value" : string
//CHECK-NEXT:  %26 = LoadStackInst %14
//CHECK-NEXT:  %27 = TryStartInst %BB7, %BB8
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %28 = StoreFrameInst %13 : object, [a]
//CHECK-NEXT:  %29 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %30 = CatchInst
//CHECK-NEXT:  %31 = StoreStackInst %30, %12
//CHECK-NEXT:  %32 = BranchInst %BB3
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %33 = BinaryOperatorInst '+', %26 : number, 1 : number
//CHECK-NEXT:  %34 = StoreStackInst %33 : number, %14
//CHECK-NEXT:  %35 = BranchInst %BB2
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %36 = StorePropertyInst %25, %13 : object, %26 : number
//CHECK-NEXT:  %37 = BranchInst %BB10
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %38 = TryEndInst
//CHECK-NEXT:  %39 = BranchInst %BB9
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %40 = LoadPropertyInst %6, "return" : string
//CHECK-NEXT:  %41 = CompareBranchInst '===', %40, undefined : undefined, %BB11, %BB12
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %42 = LoadStackInst %12
//CHECK-NEXT:  %43 = ThrowInst %42
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %44 = TryStartInst %BB13, %BB14
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %45 = BranchInst %BB4
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %46 = CatchInst
//CHECK-NEXT:  %47 = BranchInst %BB11
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %48 = CallInst %40, %6
//CHECK-NEXT:  %49 = BranchInst %BB15
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %50 = TryEndInst
//CHECK-NEXT:  %51 = BranchInst %BB11
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
//CHECK-NEXT:  %8 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %7, "iterator is not an object" : string
//CHECK-NEXT:  %9 = LoadPropertyInst %7, "next" : string
//CHECK-NEXT:  %10 = AllocStackInst $?anon_0_iterDone
//CHECK-NEXT:  %11 = StoreStackInst undefined : undefined, %10
//CHECK-NEXT:  %12 = AllocStackInst $?anon_1_iterValue
//CHECK-NEXT:  %13 = AllocStackInst $?anon_2_exc
//CHECK-NEXT:  %14 = TryStartInst %BB1, %BB2
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %15 = LoadStackInst %10
//CHECK-NEXT:  %16 = CondBranchInst %15, %BB4, %BB5
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %17 = CallInst %9, %7
//CHECK-NEXT:  %18 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %17, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %19 = LoadPropertyInst %17, "done" : string
//CHECK-NEXT:  %20 = StoreStackInst %19, %10
//CHECK-NEXT:  %21 = CondBranchInst %19, %BB7, %BB8
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %22 = LoadPropertyInst %17, "value" : string
//CHECK-NEXT:  %23 = LoadStackInst %30
//CHECK-NEXT:  %24 = TryStartInst %BB9, %BB10
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %25 = TryStartInst %BB11, %BB12
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %26 = CatchInst
//CHECK-NEXT:  %27 = StoreStackInst %26, %13
//CHECK-NEXT:  %28 = BranchInst %BB3
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %29 = AllocArrayInst 0 : number
//CHECK-NEXT:  %30 = AllocStackInst $?anon_3_n
//CHECK-NEXT:  %31 = StoreStackInst 0 : number, %30
//CHECK-NEXT:  %32 = LoadStackInst %10
//CHECK-NEXT:  %33 = CondBranchInst %32, %BB7, %BB6
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %34 = BranchInst %BB14
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %35 = TryEndInst
//CHECK-NEXT:  %36 = BranchInst %BB13
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %37 = CatchInst
//CHECK-NEXT:  %38 = StoreStackInst %37, %13
//CHECK-NEXT:  %39 = BranchInst %BB3
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %40 = BinaryOperatorInst '+', %23 : number, 1 : number
//CHECK-NEXT:  %41 = StoreStackInst %40 : number, %30
//CHECK-NEXT:  %42 = BranchInst %BB6
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %43 = StorePropertyInst %22, %29 : object, %23 : number
//CHECK-NEXT:  %44 = BranchInst %BB16
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %45 = TryEndInst
//CHECK-NEXT:  %46 = BranchInst %BB15
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %47 = CatchInst
//CHECK-NEXT:  %48 = StoreStackInst %47, %13
//CHECK-NEXT:  %49 = BranchInst %BB3
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %50 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %51 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %52 = LoadPropertyInst %51, "iterator" : string
//CHECK-NEXT:  %53 = LoadPropertyInst %29 : object, %52
//CHECK-NEXT:  %54 = CallInst %53, %29 : object
//CHECK-NEXT:  %55 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %54, "iterator is not an object" : string
//CHECK-NEXT:  %56 = LoadPropertyInst %54, "next" : string
//CHECK-NEXT:  %57 = AllocStackInst $?anon_4_iterDone
//CHECK-NEXT:  %58 = StoreStackInst undefined : undefined, %57
//CHECK-NEXT:  %59 = AllocStackInst $?anon_5_iterValue
//CHECK-NEXT:  %60 = StoreStackInst undefined : undefined, %59
//CHECK-NEXT:  %61 = BranchInst %BB18
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %62 = CallInst %56, %54
//CHECK-NEXT:  %63 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %62, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %64 = LoadPropertyInst %62, "done" : string
//CHECK-NEXT:  %65 = StoreStackInst %64, %57
//CHECK-NEXT:  %66 = CondBranchInst %64, %BB19, %BB20
//CHECK-NEXT:%BB20:
//CHECK-NEXT:  %67 = LoadPropertyInst %62, "value" : string
//CHECK-NEXT:  %68 = StoreStackInst %67, %59
//CHECK-NEXT:  %69 = BranchInst %BB19
//CHECK-NEXT:%BB19:
//CHECK-NEXT:  %70 = LoadStackInst %59
//CHECK-NEXT:  %71 = StoreFrameInst %70, [b]
//CHECK-NEXT:  %72 = StoreStackInst undefined : undefined, %59
//CHECK-NEXT:  %73 = LoadStackInst %57
//CHECK-NEXT:  %74 = CondBranchInst %73, %BB21, %BB22
//CHECK-NEXT:%BB22:
//CHECK-NEXT:  %75 = CallInst %56, %54
//CHECK-NEXT:  %76 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %75, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %77 = LoadPropertyInst %75, "done" : string
//CHECK-NEXT:  %78 = StoreStackInst %77, %57
//CHECK-NEXT:  %79 = CondBranchInst %77, %BB21, %BB23
//CHECK-NEXT:%BB23:
//CHECK-NEXT:  %80 = LoadPropertyInst %75, "value" : string
//CHECK-NEXT:  %81 = StoreStackInst %80, %59
//CHECK-NEXT:  %82 = BranchInst %BB21
//CHECK-NEXT:%BB21:
//CHECK-NEXT:  %83 = LoadStackInst %59
//CHECK-NEXT:  %84 = StoreFrameInst %83, [c]
//CHECK-NEXT:  %85 = LoadStackInst %57
//CHECK-NEXT:  %86 = CondBranchInst %85, %BB24, %BB25
//CHECK-NEXT:%BB25:
//CHECK-NEXT:  %87 = LoadPropertyInst %54, "return" : string
//CHECK-NEXT:  %88 = CompareBranchInst '===', %87, undefined : undefined, %BB26, %BB27
//CHECK-NEXT:%BB24:
//CHECK-NEXT:  %89 = BranchInst %BB28
//CHECK-NEXT:%BB27:
//CHECK-NEXT:  %90 = CallInst %87, %54
//CHECK-NEXT:  %91 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %90, "iterator.close() did not return an object" : string
//CHECK-NEXT:  %92 = BranchInst %BB26
//CHECK-NEXT:%BB26:
//CHECK-NEXT:  %93 = BranchInst %BB24
//CHECK-NEXT:%BB28:
//CHECK-NEXT:  %94 = TryEndInst
//CHECK-NEXT:  %95 = BranchInst %BB17
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %96 = LoadPropertyInst %7, "return" : string
//CHECK-NEXT:  %97 = CompareBranchInst '===', %96, undefined : undefined, %BB29, %BB30
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %98 = LoadStackInst %13
//CHECK-NEXT:  %99 = ThrowInst %98
//CHECK-NEXT:%BB30:
//CHECK-NEXT:  %100 = TryStartInst %BB31, %BB32
//CHECK-NEXT:%BB29:
//CHECK-NEXT:  %101 = BranchInst %BB4
//CHECK-NEXT:%BB31:
//CHECK-NEXT:  %102 = CatchInst
//CHECK-NEXT:  %103 = BranchInst %BB29
//CHECK-NEXT:%BB32:
//CHECK-NEXT:  %104 = CallInst %96, %7
//CHECK-NEXT:  %105 = BranchInst %BB33
//CHECK-NEXT:%BB33:
//CHECK-NEXT:  %106 = TryEndInst
//CHECK-NEXT:  %107 = BranchInst %BB29
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
//CHECK-NEXT:  %17 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %16, "iterator is not an object" : string
//CHECK-NEXT:  %18 = LoadPropertyInst %16, "next" : string
//CHECK-NEXT:  %19 = AllocStackInst $?anon_5_iterDone
//CHECK-NEXT:  %20 = StoreStackInst undefined : undefined, %19
//CHECK-NEXT:  %21 = AllocStackInst $?anon_6_iterValue
//CHECK-NEXT:  %22 = AllocStackInst $?anon_7_exc
//CHECK-NEXT:  %23 = AllocArrayInst 0 : number
//CHECK-NEXT:  %24 = AllocStackInst $?anon_8_n
//CHECK-NEXT:  %25 = StoreStackInst 0 : number, %24
//CHECK-NEXT:  %26 = LoadStackInst %19
//CHECK-NEXT:  %27 = CondBranchInst %26, %BB4, %BB5
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %28 = LoadStackInst %19
//CHECK-NEXT:  %29 = CondBranchInst %28, %BB7, %BB8
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %30 = CallInst %18, %16
//CHECK-NEXT:  %31 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %30, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %32 = LoadPropertyInst %30, "done" : string
//CHECK-NEXT:  %33 = StoreStackInst %32, %19
//CHECK-NEXT:  %34 = CondBranchInst %32, %BB4, %BB9
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %35 = LoadPropertyInst %30, "value" : string
//CHECK-NEXT:  %36 = LoadStackInst %24
//CHECK-NEXT:  %37 = TryStartInst %BB10, %BB11
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %38 = StoreFrameInst %23 : object, [d]
//CHECK-NEXT:  %39 = BranchInst %BB2
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %40 = CatchInst
//CHECK-NEXT:  %41 = StoreStackInst %40, %22
//CHECK-NEXT:  %42 = BranchInst %BB6
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %43 = BinaryOperatorInst '+', %36 : number, 1 : number
//CHECK-NEXT:  %44 = StoreStackInst %43 : number, %24
//CHECK-NEXT:  %45 = BranchInst %BB5
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %46 = StorePropertyInst %35, %23 : object, %36 : number
//CHECK-NEXT:  %47 = BranchInst %BB13
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %48 = TryEndInst
//CHECK-NEXT:  %49 = BranchInst %BB12
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %50 = LoadPropertyInst %16, "return" : string
//CHECK-NEXT:  %51 = CompareBranchInst '===', %50, undefined : undefined, %BB14, %BB15
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %52 = LoadStackInst %22
//CHECK-NEXT:  %53 = ThrowInst %52
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %54 = TryStartInst %BB16, %BB17
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %55 = BranchInst %BB7
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %56 = CatchInst
//CHECK-NEXT:  %57 = BranchInst %BB14
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %58 = CallInst %50, %16
//CHECK-NEXT:  %59 = BranchInst %BB18
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %60 = TryEndInst
//CHECK-NEXT:  %61 = BranchInst %BB14
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
//CHECK-NEXT:  %8 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %7, "iterator is not an object" : string
//CHECK-NEXT:  %9 = LoadPropertyInst %7, "next" : string
//CHECK-NEXT:  %10 = AllocStackInst $?anon_0_iterDone
//CHECK-NEXT:  %11 = StoreStackInst undefined : undefined, %10
//CHECK-NEXT:  %12 = AllocStackInst $?anon_1_iterValue
//CHECK-NEXT:  %13 = AllocStackInst $?anon_2_exc
//CHECK-NEXT:  %14 = StoreStackInst undefined : undefined, %12
//CHECK-NEXT:  %15 = BranchInst %BB1
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %16 = LoadStackInst %10
//CHECK-NEXT:  %17 = CondBranchInst %16, %BB3, %BB4
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %18 = CallInst %9, %7
//CHECK-NEXT:  %19 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %18, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %20 = LoadPropertyInst %18, "done" : string
//CHECK-NEXT:  %21 = StoreStackInst %20, %10
//CHECK-NEXT:  %22 = CondBranchInst %20, %BB5, %BB6
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %23 = LoadPropertyInst %18, "value" : string
//CHECK-NEXT:  %24 = StoreStackInst %23, %12
//CHECK-NEXT:  %25 = BranchInst %BB5
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %26 = LoadStackInst %12
//CHECK-NEXT:  %27 = StoreFrameInst %26, [a]
//CHECK-NEXT:  %28 = TryStartInst %BB7, %BB8
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %29 = CallInst %9, %7
//CHECK-NEXT:  %30 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %29, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %31 = LoadPropertyInst %29, "done" : string
//CHECK-NEXT:  %32 = StoreStackInst %31, %10
//CHECK-NEXT:  %33 = CondBranchInst %31, %BB10, %BB11
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %34 = LoadPropertyInst %29, "value" : string
//CHECK-NEXT:  %35 = LoadStackInst %42
//CHECK-NEXT:  %36 = TryStartInst %BB12, %BB13
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %37 = TryStartInst %BB14, %BB15
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %38 = CatchInst
//CHECK-NEXT:  %39 = StoreStackInst %38, %13
//CHECK-NEXT:  %40 = BranchInst %BB2
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %41 = AllocArrayInst 0 : number
//CHECK-NEXT:  %42 = AllocStackInst $?anon_3_n
//CHECK-NEXT:  %43 = StoreStackInst 0 : number, %42
//CHECK-NEXT:  %44 = LoadStackInst %10
//CHECK-NEXT:  %45 = CondBranchInst %44, %BB10, %BB9
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %46 = BranchInst %BB17
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %47 = TryEndInst
//CHECK-NEXT:  %48 = BranchInst %BB16
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %49 = CatchInst
//CHECK-NEXT:  %50 = StoreStackInst %49, %13
//CHECK-NEXT:  %51 = BranchInst %BB2
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %52 = BinaryOperatorInst '+', %35 : number, 1 : number
//CHECK-NEXT:  %53 = StoreStackInst %52 : number, %42
//CHECK-NEXT:  %54 = BranchInst %BB9
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %55 = StorePropertyInst %34, %41 : object, %35 : number
//CHECK-NEXT:  %56 = BranchInst %BB19
//CHECK-NEXT:%BB19:
//CHECK-NEXT:  %57 = TryEndInst
//CHECK-NEXT:  %58 = BranchInst %BB18
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %59 = CatchInst
//CHECK-NEXT:  %60 = StoreStackInst %59, %13
//CHECK-NEXT:  %61 = BranchInst %BB2
//CHECK-NEXT:%BB20:
//CHECK-NEXT:  %62 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %63 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %64 = LoadPropertyInst %63, "iterator" : string
//CHECK-NEXT:  %65 = LoadPropertyInst %41 : object, %64
//CHECK-NEXT:  %66 = CallInst %65, %41 : object
//CHECK-NEXT:  %67 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %66, "iterator is not an object" : string
//CHECK-NEXT:  %68 = LoadPropertyInst %66, "next" : string
//CHECK-NEXT:  %69 = AllocStackInst $?anon_4_iterDone
//CHECK-NEXT:  %70 = StoreStackInst undefined : undefined, %69
//CHECK-NEXT:  %71 = AllocStackInst $?anon_5_iterValue
//CHECK-NEXT:  %72 = AllocStackInst $?anon_6_exc
//CHECK-NEXT:  %73 = TryStartInst %BB21, %BB22
//CHECK-NEXT:%BB23:
//CHECK-NEXT:  %74 = LoadStackInst %69
//CHECK-NEXT:  %75 = CondBranchInst %74, %BB24, %BB25
//CHECK-NEXT:%BB21:
//CHECK-NEXT:  %76 = CatchInst
//CHECK-NEXT:  %77 = StoreStackInst %76, %72
//CHECK-NEXT:  %78 = BranchInst %BB23
//CHECK-NEXT:%BB26:
//CHECK-NEXT:  %79 = StoreStackInst undefined : undefined, %71
//CHECK-NEXT:  %80 = BranchInst %BB27
//CHECK-NEXT:%BB22:
//CHECK-NEXT:  %81 = LoadFrameInst [b]
//CHECK-NEXT:  %82 = BranchInst %BB28
//CHECK-NEXT:%BB28:
//CHECK-NEXT:  %83 = TryEndInst
//CHECK-NEXT:  %84 = BranchInst %BB26
//CHECK-NEXT:%BB27:
//CHECK-NEXT:  %85 = CallInst %68, %66
//CHECK-NEXT:  %86 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %85, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %87 = LoadPropertyInst %85, "done" : string
//CHECK-NEXT:  %88 = StoreStackInst %87, %69
//CHECK-NEXT:  %89 = CondBranchInst %87, %BB29, %BB30
//CHECK-NEXT:%BB30:
//CHECK-NEXT:  %90 = LoadPropertyInst %85, "value" : string
//CHECK-NEXT:  %91 = StoreStackInst %90, %71
//CHECK-NEXT:  %92 = BranchInst %BB29
//CHECK-NEXT:%BB29:
//CHECK-NEXT:  %93 = TryStartInst %BB31, %BB32
//CHECK-NEXT:%BB31:
//CHECK-NEXT:  %94 = CatchInst
//CHECK-NEXT:  %95 = StoreStackInst %94, %72
//CHECK-NEXT:  %96 = BranchInst %BB23
//CHECK-NEXT:%BB33:
//CHECK-NEXT:  %97 = LoadStackInst %69
//CHECK-NEXT:  %98 = CondBranchInst %97, %BB34, %BB35
//CHECK-NEXT:%BB32:
//CHECK-NEXT:  %99 = LoadStackInst %71
//CHECK-NEXT:  %100 = StorePropertyInst %99, %81, 0 : number
//CHECK-NEXT:  %101 = BranchInst %BB36
//CHECK-NEXT:%BB36:
//CHECK-NEXT:  %102 = TryEndInst
//CHECK-NEXT:  %103 = BranchInst %BB33
//CHECK-NEXT:%BB35:
//CHECK-NEXT:  %104 = LoadPropertyInst %66, "return" : string
//CHECK-NEXT:  %105 = CompareBranchInst '===', %104, undefined : undefined, %BB37, %BB38
//CHECK-NEXT:%BB34:
//CHECK-NEXT:  %106 = BranchInst %BB39
//CHECK-NEXT:%BB38:
//CHECK-NEXT:  %107 = CallInst %104, %66
//CHECK-NEXT:  %108 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %107, "iterator.close() did not return an object" : string
//CHECK-NEXT:  %109 = BranchInst %BB37
//CHECK-NEXT:%BB37:
//CHECK-NEXT:  %110 = BranchInst %BB34
//CHECK-NEXT:%BB25:
//CHECK-NEXT:  %111 = LoadPropertyInst %66, "return" : string
//CHECK-NEXT:  %112 = CompareBranchInst '===', %111, undefined : undefined, %BB40, %BB41
//CHECK-NEXT:%BB24:
//CHECK-NEXT:  %113 = LoadStackInst %72
//CHECK-NEXT:  %114 = ThrowInst %113
//CHECK-NEXT:%BB41:
//CHECK-NEXT:  %115 = TryStartInst %BB42, %BB43
//CHECK-NEXT:%BB40:
//CHECK-NEXT:  %116 = BranchInst %BB24
//CHECK-NEXT:%BB42:
//CHECK-NEXT:  %117 = CatchInst
//CHECK-NEXT:  %118 = BranchInst %BB40
//CHECK-NEXT:%BB43:
//CHECK-NEXT:  %119 = CallInst %111, %66
//CHECK-NEXT:  %120 = BranchInst %BB44
//CHECK-NEXT:%BB44:
//CHECK-NEXT:  %121 = TryEndInst
//CHECK-NEXT:  %122 = BranchInst %BB40
//CHECK-NEXT:%BB39:
//CHECK-NEXT:  %123 = TryEndInst
//CHECK-NEXT:  %124 = BranchInst %BB20
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %125 = LoadPropertyInst %7, "return" : string
//CHECK-NEXT:  %126 = CompareBranchInst '===', %125, undefined : undefined, %BB45, %BB46
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %127 = LoadStackInst %13
//CHECK-NEXT:  %128 = ThrowInst %127
//CHECK-NEXT:%BB46:
//CHECK-NEXT:  %129 = TryStartInst %BB47, %BB48
//CHECK-NEXT:%BB45:
//CHECK-NEXT:  %130 = BranchInst %BB3
//CHECK-NEXT:%BB47:
//CHECK-NEXT:  %131 = CatchInst
//CHECK-NEXT:  %132 = BranchInst %BB45
//CHECK-NEXT:%BB48:
//CHECK-NEXT:  %133 = CallInst %125, %7
//CHECK-NEXT:  %134 = BranchInst %BB49
//CHECK-NEXT:%BB49:
//CHECK-NEXT:  %135 = TryEndInst
//CHECK-NEXT:  %136 = BranchInst %BB45
//CHECK-NEXT:function_end
