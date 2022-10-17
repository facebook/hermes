/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

var {a} = x;

var {a, b} = x;

var {a: b, c: d, } = x;

var {a: b = g} = x;

var {a: [b = 1, e] = g} = x;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [a, b, d, e]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %4 = LoadPropertyInst %3, "a" : string
// CHECK-NEXT:  %5 = StorePropertyInst %4, globalObject : object, "a" : string
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %7 = LoadPropertyInst %6, "a" : string
// CHECK-NEXT:  %8 = StorePropertyInst %7, globalObject : object, "a" : string
// CHECK-NEXT:  %9 = LoadPropertyInst %6, "b" : string
// CHECK-NEXT:  %10 = StorePropertyInst %9, globalObject : object, "b" : string
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %12 = LoadPropertyInst %11, "a" : string
// CHECK-NEXT:  %13 = StorePropertyInst %12, globalObject : object, "b" : string
// CHECK-NEXT:  %14 = LoadPropertyInst %11, "c" : string
// CHECK-NEXT:  %15 = StorePropertyInst %14, globalObject : object, "d" : string
// CHECK-NEXT:  %16 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %17 = LoadPropertyInst %16, "a" : string
// CHECK-NEXT:  %18 = BinaryOperatorInst '!==', %17, undefined : undefined
// CHECK-NEXT:  %19 = CondBranchInst %18, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %20 = TryLoadGlobalPropertyInst globalObject : object, "g" : string
// CHECK-NEXT:  %21 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %22 = PhiInst %17, %BB0, %20, %BB2
// CHECK-NEXT:  %23 = StorePropertyInst %22, globalObject : object, "b" : string
// CHECK-NEXT:  %24 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %25 = LoadPropertyInst %24, "a" : string
// CHECK-NEXT:  %26 = BinaryOperatorInst '!==', %25, undefined : undefined
// CHECK-NEXT:  %27 = CondBranchInst %26, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %28 = TryLoadGlobalPropertyInst globalObject : object, "g" : string
// CHECK-NEXT:  %29 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %30 = PhiInst %25, %BB1, %28, %BB4
// CHECK-NEXT:  %31 = AllocStackInst $?anon_1_iter
// CHECK-NEXT:  %32 = AllocStackInst $?anon_2_sourceOrNext
// CHECK-NEXT:  %33 = StoreStackInst %30, %32
// CHECK-NEXT:  %34 = IteratorBeginInst %32
// CHECK-NEXT:  %35 = StoreStackInst %34, %31
// CHECK-NEXT:  %36 = AllocStackInst $?anon_3_iterDone
// CHECK-NEXT:  %37 = StoreStackInst undefined : undefined, %36
// CHECK-NEXT:  %38 = AllocStackInst $?anon_4_iterValue
// CHECK-NEXT:  %39 = AllocStackInst $?anon_5_exc
// CHECK-NEXT:  %40 = TryStartInst %BB5, %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %41 = LoadStackInst %36
// CHECK-NEXT:  %42 = CondBranchInst %41, %BB8, %BB9
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %43 = CatchInst
// CHECK-NEXT:  %44 = StoreStackInst %43, %39
// CHECK-NEXT:  %45 = BranchInst %BB7
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %46 = StoreStackInst undefined : undefined, %38
// CHECK-NEXT:  %47 = BranchInst %BB11
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %48 = BranchInst %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %49 = TryEndInst
// CHECK-NEXT:  %50 = BranchInst %BB10
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %51 = IteratorNextInst %31, %32
// CHECK-NEXT:  %52 = LoadStackInst %31
// CHECK-NEXT:  %53 = BinaryOperatorInst '===', %52, undefined : undefined
// CHECK-NEXT:  %54 = StoreStackInst %53, %36
// CHECK-NEXT:  %55 = CondBranchInst %53, %BB13, %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %56 = StoreStackInst %51, %38
// CHECK-NEXT:  %57 = BranchInst %BB15
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %58 = LoadStackInst %38
// CHECK-NEXT:  %59 = BinaryOperatorInst '!==', %58, undefined : undefined
// CHECK-NEXT:  %60 = CondBranchInst %59, %BB16, %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %61 = StoreStackInst 1 : number, %38
// CHECK-NEXT:  %62 = BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %63 = TryStartInst %BB17, %BB18
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %64 = CatchInst
// CHECK-NEXT:  %65 = StoreStackInst %64, %39
// CHECK-NEXT:  %66 = BranchInst %BB7
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %67 = StoreStackInst undefined : undefined, %38
// CHECK-NEXT:  %68 = LoadStackInst %36
// CHECK-NEXT:  %69 = CondBranchInst %68, %BB20, %BB21
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %70 = LoadStackInst %38
// CHECK-NEXT:  %71 = StorePropertyInst %70, globalObject : object, "b" : string
// CHECK-NEXT:  %72 = BranchInst %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %73 = TryEndInst
// CHECK-NEXT:  %74 = BranchInst %BB19
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %75 = IteratorNextInst %31, %32
// CHECK-NEXT:  %76 = LoadStackInst %31
// CHECK-NEXT:  %77 = BinaryOperatorInst '===', %76, undefined : undefined
// CHECK-NEXT:  %78 = StoreStackInst %77, %36
// CHECK-NEXT:  %79 = CondBranchInst %77, %BB20, %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %80 = StoreStackInst %75, %38
// CHECK-NEXT:  %81 = BranchInst %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %82 = TryStartInst %BB24, %BB25
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %83 = CatchInst
// CHECK-NEXT:  %84 = StoreStackInst %83, %39
// CHECK-NEXT:  %85 = BranchInst %BB7
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %86 = LoadStackInst %36
// CHECK-NEXT:  %87 = CondBranchInst %86, %BB27, %BB28
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %88 = LoadStackInst %38
// CHECK-NEXT:  %89 = StorePropertyInst %88, globalObject : object, "e" : string
// CHECK-NEXT:  %90 = BranchInst %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %91 = TryEndInst
// CHECK-NEXT:  %92 = BranchInst %BB26
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %93 = IteratorCloseInst %31, false : boolean
// CHECK-NEXT:  %94 = BranchInst %BB27
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %95 = LoadStackInst %1
// CHECK-NEXT:  %96 = ReturnInst %95
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %97 = IteratorCloseInst %31, true : boolean
// CHECK-NEXT:  %98 = BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %99 = LoadStackInst %39
// CHECK-NEXT:  %100 = ThrowInst %99
// CHECK-NEXT:function_end
