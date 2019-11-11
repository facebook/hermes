/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck %s --match-full-lines

//CHECK-LABEL:function global()
//CHECK-NEXT:frame = [], globals = [a, b, d, e]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:  %1 = StoreStackInst undefined : undefined, %0

var {a} = x;
//CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:  %3 = LoadPropertyInst %2, "a" : string
//CHECK-NEXT:  %4 = StorePropertyInst %3, globalObject : object, "a" : string

var {a, b} = x;
//CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:  %6 = LoadPropertyInst %5, "a" : string
//CHECK-NEXT:  %7 = StorePropertyInst %6, globalObject : object, "a" : string
//CHECK-NEXT:  %8 = LoadPropertyInst %5, "b" : string
//CHECK-NEXT:  %9 = StorePropertyInst %8, globalObject : object, "b" : string

var {a: b, c: d, } = x;
//CHECK-NEXT:  %10 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:  %11 = LoadPropertyInst %10, "a" : string
//CHECK-NEXT:  %12 = StorePropertyInst %11, globalObject : object, "b" : string
//CHECK-NEXT:  %13 = LoadPropertyInst %10, "c" : string
//CHECK-NEXT:  %14 = StorePropertyInst %13, globalObject : object, "d" : string

var {a: b = g} = x;
//CHECK-NEXT:  %15 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:  %16 = LoadPropertyInst %15, "a" : string
//CHECK-NEXT:  %17 = BinaryOperatorInst '!==', %16, undefined : undefined
//CHECK-NEXT:  %18 = CondBranchInst %17, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %19 = TryLoadGlobalPropertyInst globalObject : object, "g" : string
//CHECK-NEXT:  %20 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %21 = PhiInst %16, %BB0, %19, %BB2
//CHECK-NEXT:  %22 = StorePropertyInst %21, globalObject : object, "b" : string

var {a: [b = 1, e] = g} = x;
//CHECK-NEXT:  %23 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:  %24 = LoadPropertyInst %23, "a" : string
//CHECK-NEXT:  %25 = BinaryOperatorInst '!==', %24, undefined : undefined
//CHECK-NEXT:  %26 = CondBranchInst %25, %BB3, %BB4
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %27 = TryLoadGlobalPropertyInst globalObject : object, "g" : string
//CHECK-NEXT:  %28 = BranchInst %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %29 = PhiInst %24, %BB1, %27, %BB4
//CHECK-NEXT:  %30 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %31 = LoadPropertyInst %30, "iterator" : string
//CHECK-NEXT:  %32 = LoadPropertyInst %29, %31
//CHECK-NEXT:  %33 = CallInst %32, %29
//CHECK-NEXT:  %34 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %35 = LoadPropertyInst %34, "ensureObject" : string
//CHECK-NEXT:  %36 = CallInst %35, undefined : undefined, %33, "iterator is not an object" : string
//CHECK-NEXT:  %37 = LoadPropertyInst %33, "next" : string
//CHECK-NEXT:  %38 = AllocStackInst $?anon_1_iterDone
//CHECK-NEXT:  %39 = StoreStackInst undefined : undefined, %38
//CHECK-NEXT:  %40 = AllocStackInst $?anon_2_iterValue
//CHECK-NEXT:  %41 = AllocStackInst $?anon_3_exc
//CHECK-NEXT:  %42 = TryStartInst %BB5, %BB6
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %43 = LoadStackInst %38
//CHECK-NEXT:  %44 = CondBranchInst %43, %BB8, %BB9
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %45 = CatchInst
//CHECK-NEXT:  %46 = StoreStackInst %45, %41
//CHECK-NEXT:  %47 = BranchInst %BB7
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %48 = StoreStackInst undefined : undefined, %40
//CHECK-NEXT:  %49 = BranchInst %BB11
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %50 = BranchInst %BB12
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %51 = TryEndInst
//CHECK-NEXT:  %52 = BranchInst %BB10
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %53 = CallInst %37, %33
//CHECK-NEXT:  %54 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %55 = LoadPropertyInst %54, "ensureObject" : string
//CHECK-NEXT:  %56 = CallInst %55, undefined : undefined, %53, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %57 = LoadPropertyInst %53, "done" : string
//CHECK-NEXT:  %58 = StoreStackInst %57, %38
//CHECK-NEXT:  %59 = CondBranchInst %57, %BB13, %BB14
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %60 = LoadPropertyInst %53, "value" : string
//CHECK-NEXT:  %61 = StoreStackInst %60, %40
//CHECK-NEXT:  %62 = BranchInst %BB15
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %63 = LoadStackInst %40
//CHECK-NEXT:  %64 = BinaryOperatorInst '!==', %63, undefined : undefined
//CHECK-NEXT:  %65 = CondBranchInst %64, %BB16, %BB13
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %66 = StoreStackInst 1 : number, %40
//CHECK-NEXT:  %67 = BranchInst %BB16
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %68 = TryStartInst %BB17, %BB18
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %69 = CatchInst
//CHECK-NEXT:  %70 = StoreStackInst %69, %41
//CHECK-NEXT:  %71 = BranchInst %BB7
//CHECK-NEXT:%BB19:
//CHECK-NEXT:  %72 = StoreStackInst undefined : undefined, %40
//CHECK-NEXT:  %73 = LoadStackInst %38
//CHECK-NEXT:  %74 = CondBranchInst %73, %BB20, %BB21
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %75 = LoadStackInst %40
//CHECK-NEXT:  %76 = StorePropertyInst %75, globalObject : object, "b" : string
//CHECK-NEXT:  %77 = BranchInst %BB22
//CHECK-NEXT:%BB22:
//CHECK-NEXT:  %78 = TryEndInst
//CHECK-NEXT:  %79 = BranchInst %BB19
//CHECK-NEXT:%BB21:
//CHECK-NEXT:  %80 = CallInst %37, %33
//CHECK-NEXT:  %81 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %82 = LoadPropertyInst %81, "ensureObject" : string
//CHECK-NEXT:  %83 = CallInst %82, undefined : undefined, %80, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %84 = LoadPropertyInst %80, "done" : string
//CHECK-NEXT:  %85 = StoreStackInst %84, %38
//CHECK-NEXT:  %86 = CondBranchInst %84, %BB20, %BB23
//CHECK-NEXT:%BB23:
//CHECK-NEXT:  %87 = LoadPropertyInst %80, "value" : string
//CHECK-NEXT:  %88 = StoreStackInst %87, %40
//CHECK-NEXT:  %89 = BranchInst %BB20
//CHECK-NEXT:%BB20:
//CHECK-NEXT:  %90 = TryStartInst %BB24, %BB25
//CHECK-NEXT:%BB24:
//CHECK-NEXT:  %91 = CatchInst
//CHECK-NEXT:  %92 = StoreStackInst %91, %41
//CHECK-NEXT:  %93 = BranchInst %BB7
//CHECK-NEXT:%BB26:
//CHECK-NEXT:  %94 = LoadStackInst %38
//CHECK-NEXT:  %95 = CondBranchInst %94, %BB27, %BB28
//CHECK-NEXT:%BB25:
//CHECK-NEXT:  %96 = LoadStackInst %40
//CHECK-NEXT:  %97 = StorePropertyInst %96, globalObject : object, "e" : string
//CHECK-NEXT:  %98 = BranchInst %BB29
//CHECK-NEXT:%BB29:
//CHECK-NEXT:  %99 = TryEndInst
//CHECK-NEXT:  %100 = BranchInst %BB26
//CHECK-NEXT:%BB28:
//CHECK-NEXT:  %101 = LoadPropertyInst %33, "return" : string
//CHECK-NEXT:  %102 = CompareBranchInst '===', %101, undefined : undefined, %BB30, %BB31
//CHECK-NEXT:%BB27:
//CHECK-NEXT:  %103 = LoadStackInst %0
//CHECK-NEXT:  %104 = ReturnInst %103
//CHECK-NEXT:%BB31:
//CHECK-NEXT:  %105 = CallInst %101, %33
//CHECK-NEXT:  %106 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %107 = LoadPropertyInst %106, "ensureObject" : string
//CHECK-NEXT:  %108 = CallInst %107, undefined : undefined, %105, "iterator.close() did not return an object" : string
//CHECK-NEXT:  %109 = BranchInst %BB30
//CHECK-NEXT:%BB30:
//CHECK-NEXT:  %110 = BranchInst %BB27
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %111 = LoadPropertyInst %33, "return" : string
//CHECK-NEXT:  %112 = CompareBranchInst '===', %111, undefined : undefined, %BB32, %BB33
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %113 = LoadStackInst %41
//CHECK-NEXT:  %114 = ThrowInst %113
//CHECK-NEXT:%BB33:
//CHECK-NEXT:  %115 = TryStartInst %BB34, %BB35
//CHECK-NEXT:%BB32:
//CHECK-NEXT:  %116 = BranchInst %BB8
//CHECK-NEXT:%BB34:
//CHECK-NEXT:  %117 = CatchInst
//CHECK-NEXT:  %118 = BranchInst %BB32
//CHECK-NEXT:%BB35:
//CHECK-NEXT:  %119 = CallInst %111, %33
//CHECK-NEXT:  %120 = BranchInst %BB36
//CHECK-NEXT:%BB36:
//CHECK-NEXT:  %121 = TryEndInst
//CHECK-NEXT:  %122 = BranchInst %BB32
//CHECK-NEXT:function_end
