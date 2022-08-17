/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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
//CHECK-NEXT:  %30 = AllocStackInst $?anon_1_iter
//CHECK-NEXT:  %31 = AllocStackInst $?anon_2_sourceOrNext
//CHECK-NEXT:  %32 = StoreStackInst %29, %31
//CHECK-NEXT:  %33 = IteratorBeginInst %31
//CHECK-NEXT:  %34 = StoreStackInst %33, %30
//CHECK-NEXT:  %35 = AllocStackInst $?anon_3_iterDone
//CHECK-NEXT:  %36 = StoreStackInst undefined : undefined, %35
//CHECK-NEXT:  %37 = AllocStackInst $?anon_4_iterValue
//CHECK-NEXT:  %38 = AllocStackInst $?anon_5_exc
//CHECK-NEXT:  %39 = TryStartInst %BB5, %BB6
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %40 = LoadStackInst %35
//CHECK-NEXT:  %41 = CondBranchInst %40, %BB8, %BB9
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %42 = CatchInst
//CHECK-NEXT:  %43 = StoreStackInst %42, %38
//CHECK-NEXT:  %44 = BranchInst %BB7
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %45 = StoreStackInst undefined : undefined, %37
//CHECK-NEXT:  %46 = BranchInst %BB11
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %47 = BranchInst %BB12
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %48 = TryEndInst
//CHECK-NEXT:  %49 = BranchInst %BB10
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %50 = IteratorNextInst %30, %31
//CHECK-NEXT:  %51 = LoadStackInst %30
//CHECK-NEXT:  %52 = BinaryOperatorInst '===', %51, undefined : undefined
//CHECK-NEXT:  %53 = StoreStackInst %52, %35
//CHECK-NEXT:  %54 = CondBranchInst %52, %BB13, %BB14
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %55 = StoreStackInst %50, %37
//CHECK-NEXT:  %56 = BranchInst %BB15
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %57 = LoadStackInst %37
//CHECK-NEXT:  %58 = BinaryOperatorInst '!==', %57, undefined : undefined
//CHECK-NEXT:  %59 = CondBranchInst %58, %BB16, %BB13
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %60 = StoreStackInst 1 : number, %37
//CHECK-NEXT:  %61 = BranchInst %BB16
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %62 = TryStartInst %BB17, %BB18
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %63 = CatchInst
//CHECK-NEXT:  %64 = StoreStackInst %63, %38
//CHECK-NEXT:  %65 = BranchInst %BB7
//CHECK-NEXT:%BB19:
//CHECK-NEXT:  %66 = StoreStackInst undefined : undefined, %37
//CHECK-NEXT:  %67 = LoadStackInst %35
//CHECK-NEXT:  %68 = CondBranchInst %67, %BB20, %BB21
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %69 = LoadStackInst %37
//CHECK-NEXT:  %70 = StorePropertyInst %69, globalObject : object, "b" : string
//CHECK-NEXT:  %71 = BranchInst %BB22
//CHECK-NEXT:%BB22:
//CHECK-NEXT:  %72 = TryEndInst
//CHECK-NEXT:  %73 = BranchInst %BB19
//CHECK-NEXT:%BB21:
//CHECK-NEXT:  %74 = IteratorNextInst %30, %31
//CHECK-NEXT:  %75 = LoadStackInst %30
//CHECK-NEXT:  %76 = BinaryOperatorInst '===', %75, undefined : undefined
//CHECK-NEXT:  %77 = StoreStackInst %76, %35
//CHECK-NEXT:  %78 = CondBranchInst %76, %BB20, %BB23
//CHECK-NEXT:%BB23:
//CHECK-NEXT:  %79 = StoreStackInst %74, %37
//CHECK-NEXT:  %80 = BranchInst %BB20
//CHECK-NEXT:%BB20:
//CHECK-NEXT:  %81 = TryStartInst %BB24, %BB25
//CHECK-NEXT:%BB24:
//CHECK-NEXT:  %82 = CatchInst
//CHECK-NEXT:  %83 = StoreStackInst %82, %38
//CHECK-NEXT:  %84 = BranchInst %BB7
//CHECK-NEXT:%BB26:
//CHECK-NEXT:  %85 = LoadStackInst %35
//CHECK-NEXT:  %86 = CondBranchInst %85, %BB27, %BB28
//CHECK-NEXT:%BB25:
//CHECK-NEXT:  %87 = LoadStackInst %37
//CHECK-NEXT:  %88 = StorePropertyInst %87, globalObject : object, "e" : string
//CHECK-NEXT:  %89 = BranchInst %BB29
//CHECK-NEXT:%BB29:
//CHECK-NEXT:  %90 = TryEndInst
//CHECK-NEXT:  %91 = BranchInst %BB26
//CHECK-NEXT:%BB28:
//CHECK-NEXT:  %92 = IteratorCloseInst %30, false : boolean
//CHECK-NEXT:  %93 = BranchInst %BB27
//CHECK-NEXT:%BB27:
//CHECK-NEXT:  %94 = LoadStackInst %0
//CHECK-NEXT:  %95 = ReturnInst %94
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %96 = IteratorCloseInst %30, true : boolean
//CHECK-NEXT:  %97 = BranchInst %BB8
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %98 = LoadStackInst %38
//CHECK-NEXT:  %99 = ThrowInst %98
//CHECK-NEXT:function_end
