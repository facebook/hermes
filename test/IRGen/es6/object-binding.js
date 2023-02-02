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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "a" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "b" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "d" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "e" : string
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %5 = StoreStackInst undefined : undefined, %4
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %7 = LoadPropertyInst %6, "a" : string
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7, globalObject : object, "a" : string
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %10 = LoadPropertyInst %9, "a" : string
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10, globalObject : object, "a" : string
// CHECK-NEXT:  %12 = LoadPropertyInst %9, "b" : string
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12, globalObject : object, "b" : string
// CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %15 = LoadPropertyInst %14, "a" : string
// CHECK-NEXT:  %16 = StorePropertyLooseInst %15, globalObject : object, "b" : string
// CHECK-NEXT:  %17 = LoadPropertyInst %14, "c" : string
// CHECK-NEXT:  %18 = StorePropertyLooseInst %17, globalObject : object, "d" : string
// CHECK-NEXT:  %19 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %20 = LoadPropertyInst %19, "a" : string
// CHECK-NEXT:  %21 = BinaryOperatorInst '!==', %20, undefined : undefined
// CHECK-NEXT:  %22 = CondBranchInst %21, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %23 = TryLoadGlobalPropertyInst globalObject : object, "g" : string
// CHECK-NEXT:  %24 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %25 = PhiInst %20, %BB0, %23, %BB2
// CHECK-NEXT:  %26 = StorePropertyLooseInst %25, globalObject : object, "b" : string
// CHECK-NEXT:  %27 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %28 = LoadPropertyInst %27, "a" : string
// CHECK-NEXT:  %29 = BinaryOperatorInst '!==', %28, undefined : undefined
// CHECK-NEXT:  %30 = CondBranchInst %29, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %31 = TryLoadGlobalPropertyInst globalObject : object, "g" : string
// CHECK-NEXT:  %32 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %33 = PhiInst %28, %BB1, %31, %BB4
// CHECK-NEXT:  %34 = AllocStackInst $?anon_1_iter
// CHECK-NEXT:  %35 = AllocStackInst $?anon_2_sourceOrNext
// CHECK-NEXT:  %36 = StoreStackInst %33, %35
// CHECK-NEXT:  %37 = IteratorBeginInst %35
// CHECK-NEXT:  %38 = StoreStackInst %37, %34
// CHECK-NEXT:  %39 = AllocStackInst $?anon_3_iterDone
// CHECK-NEXT:  %40 = StoreStackInst undefined : undefined, %39
// CHECK-NEXT:  %41 = AllocStackInst $?anon_4_iterValue
// CHECK-NEXT:  %42 = AllocStackInst $?anon_5_exc
// CHECK-NEXT:  %43 = TryStartInst %BB5, %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %44 = LoadStackInst %39
// CHECK-NEXT:  %45 = CondBranchInst %44, %BB8, %BB9
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %46 = CatchInst
// CHECK-NEXT:  %47 = StoreStackInst %46, %42
// CHECK-NEXT:  %48 = BranchInst %BB7
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %49 = StoreStackInst undefined : undefined, %41
// CHECK-NEXT:  %50 = BranchInst %BB11
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %51 = BranchInst %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %52 = TryEndInst
// CHECK-NEXT:  %53 = BranchInst %BB10
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %54 = IteratorNextInst %34, %35
// CHECK-NEXT:  %55 = LoadStackInst %34
// CHECK-NEXT:  %56 = BinaryOperatorInst '===', %55, undefined : undefined
// CHECK-NEXT:  %57 = StoreStackInst %56, %39
// CHECK-NEXT:  %58 = CondBranchInst %56, %BB13, %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %59 = StoreStackInst %54, %41
// CHECK-NEXT:  %60 = BranchInst %BB15
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %61 = LoadStackInst %41
// CHECK-NEXT:  %62 = BinaryOperatorInst '!==', %61, undefined : undefined
// CHECK-NEXT:  %63 = CondBranchInst %62, %BB16, %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %64 = StoreStackInst 1 : number, %41
// CHECK-NEXT:  %65 = BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %66 = TryStartInst %BB17, %BB18
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %67 = CatchInst
// CHECK-NEXT:  %68 = StoreStackInst %67, %42
// CHECK-NEXT:  %69 = BranchInst %BB7
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %70 = StoreStackInst undefined : undefined, %41
// CHECK-NEXT:  %71 = LoadStackInst %39
// CHECK-NEXT:  %72 = CondBranchInst %71, %BB20, %BB21
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %73 = LoadStackInst %41
// CHECK-NEXT:  %74 = StorePropertyLooseInst %73, globalObject : object, "b" : string
// CHECK-NEXT:  %75 = BranchInst %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %76 = TryEndInst
// CHECK-NEXT:  %77 = BranchInst %BB19
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %78 = IteratorNextInst %34, %35
// CHECK-NEXT:  %79 = LoadStackInst %34
// CHECK-NEXT:  %80 = BinaryOperatorInst '===', %79, undefined : undefined
// CHECK-NEXT:  %81 = StoreStackInst %80, %39
// CHECK-NEXT:  %82 = CondBranchInst %80, %BB20, %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %83 = StoreStackInst %78, %41
// CHECK-NEXT:  %84 = BranchInst %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %85 = TryStartInst %BB24, %BB25
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %86 = CatchInst
// CHECK-NEXT:  %87 = StoreStackInst %86, %42
// CHECK-NEXT:  %88 = BranchInst %BB7
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %89 = LoadStackInst %39
// CHECK-NEXT:  %90 = CondBranchInst %89, %BB27, %BB28
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %91 = LoadStackInst %41
// CHECK-NEXT:  %92 = StorePropertyLooseInst %91, globalObject : object, "e" : string
// CHECK-NEXT:  %93 = BranchInst %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %94 = TryEndInst
// CHECK-NEXT:  %95 = BranchInst %BB26
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %96 = IteratorCloseInst %34, false : boolean
// CHECK-NEXT:  %97 = BranchInst %BB27
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %98 = LoadStackInst %4
// CHECK-NEXT:  %99 = ReturnInst %98
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %100 = IteratorCloseInst %34, true : boolean
// CHECK-NEXT:  %101 = BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %102 = LoadStackInst %42
// CHECK-NEXT:  %103 = ThrowInst %102
// CHECK-NEXT:function_end
