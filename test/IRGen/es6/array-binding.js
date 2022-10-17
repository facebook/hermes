/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

var [a, [b = 1, c] = [,2]] = x;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [a, b, c]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %4 = AllocStackInst $?anon_1_iter
// CHECK-NEXT:  %5 = AllocStackInst $?anon_2_sourceOrNext
// CHECK-NEXT:  %6 = StoreStackInst %3, %5
// CHECK-NEXT:  %7 = IteratorBeginInst %5
// CHECK-NEXT:  %8 = StoreStackInst %7, %4
// CHECK-NEXT:  %9 = AllocStackInst $?anon_3_iterDone
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = AllocStackInst $?anon_4_iterValue
// CHECK-NEXT:  %12 = AllocStackInst $?anon_5_exc
// CHECK-NEXT:  %13 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = LoadStackInst %9
// CHECK-NEXT:  %15 = CondBranchInst %14, %BB4, %BB5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = CatchInst
// CHECK-NEXT:  %17 = StoreStackInst %16, %12
// CHECK-NEXT:  %18 = BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %19 = StoreStackInst undefined : undefined, %11
// CHECK-NEXT:  %20 = BranchInst %BB7
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %21 = BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %22 = TryEndInst
// CHECK-NEXT:  %23 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %24 = IteratorNextInst %4, %5
// CHECK-NEXT:  %25 = LoadStackInst %4
// CHECK-NEXT:  %26 = BinaryOperatorInst '===', %25, undefined : undefined
// CHECK-NEXT:  %27 = StoreStackInst %26, %9
// CHECK-NEXT:  %28 = CondBranchInst %26, %BB9, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %29 = StoreStackInst %24, %11
// CHECK-NEXT:  %30 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %31 = TryStartInst %BB11, %BB12
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %32 = CatchInst
// CHECK-NEXT:  %33 = StoreStackInst %32, %12
// CHECK-NEXT:  %34 = BranchInst %BB3
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %35 = StoreStackInst undefined : undefined, %11
// CHECK-NEXT:  %36 = LoadStackInst %9
// CHECK-NEXT:  %37 = CondBranchInst %36, %BB14, %BB15
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %38 = LoadStackInst %11
// CHECK-NEXT:  %39 = StorePropertyInst %38, globalObject : object, "a" : string
// CHECK-NEXT:  %40 = BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %41 = TryEndInst
// CHECK-NEXT:  %42 = BranchInst %BB13
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %43 = IteratorNextInst %4, %5
// CHECK-NEXT:  %44 = LoadStackInst %4
// CHECK-NEXT:  %45 = BinaryOperatorInst '===', %44, undefined : undefined
// CHECK-NEXT:  %46 = StoreStackInst %45, %9
// CHECK-NEXT:  %47 = CondBranchInst %45, %BB17, %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %48 = StoreStackInst %43, %11
// CHECK-NEXT:  %49 = BranchInst %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %50 = LoadStackInst %11
// CHECK-NEXT:  %51 = BinaryOperatorInst '!==', %50, undefined : undefined
// CHECK-NEXT:  %52 = CondBranchInst %51, %BB19, %BB17
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %53 = AllocArrayInst 2 : number
// CHECK-NEXT:  %54 = StoreOwnPropertyInst 2 : number, %53 : object, 1 : number, true : boolean
// CHECK-NEXT:  %55 = StoreStackInst %53 : object, %11
// CHECK-NEXT:  %56 = BranchInst %BB19
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %57 = TryStartInst %BB20, %BB21
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %58 = CatchInst
// CHECK-NEXT:  %59 = StoreStackInst %58, %12
// CHECK-NEXT:  %60 = BranchInst %BB3
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %61 = LoadStackInst %9
// CHECK-NEXT:  %62 = CondBranchInst %61, %BB23, %BB24
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %63 = LoadStackInst %11
// CHECK-NEXT:  %64 = AllocStackInst $?anon_6_iter
// CHECK-NEXT:  %65 = AllocStackInst $?anon_7_sourceOrNext
// CHECK-NEXT:  %66 = StoreStackInst %63, %65
// CHECK-NEXT:  %67 = IteratorBeginInst %65
// CHECK-NEXT:  %68 = StoreStackInst %67, %64
// CHECK-NEXT:  %69 = AllocStackInst $?anon_8_iterDone
// CHECK-NEXT:  %70 = StoreStackInst undefined : undefined, %69
// CHECK-NEXT:  %71 = AllocStackInst $?anon_9_iterValue
// CHECK-NEXT:  %72 = AllocStackInst $?anon_10_exc
// CHECK-NEXT:  %73 = TryStartInst %BB25, %BB26
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %74 = LoadStackInst %69
// CHECK-NEXT:  %75 = CondBranchInst %74, %BB28, %BB29
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %76 = CatchInst
// CHECK-NEXT:  %77 = StoreStackInst %76, %72
// CHECK-NEXT:  %78 = BranchInst %BB27
// CHECK-NEXT:%BB30:
// CHECK-NEXT:  %79 = StoreStackInst undefined : undefined, %71
// CHECK-NEXT:  %80 = BranchInst %BB31
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %81 = BranchInst %BB32
// CHECK-NEXT:%BB32:
// CHECK-NEXT:  %82 = TryEndInst
// CHECK-NEXT:  %83 = BranchInst %BB30
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %84 = IteratorNextInst %64, %65
// CHECK-NEXT:  %85 = LoadStackInst %64
// CHECK-NEXT:  %86 = BinaryOperatorInst '===', %85, undefined : undefined
// CHECK-NEXT:  %87 = StoreStackInst %86, %69
// CHECK-NEXT:  %88 = CondBranchInst %86, %BB33, %BB34
// CHECK-NEXT:%BB34:
// CHECK-NEXT:  %89 = StoreStackInst %84, %71
// CHECK-NEXT:  %90 = BranchInst %BB35
// CHECK-NEXT:%BB35:
// CHECK-NEXT:  %91 = LoadStackInst %71
// CHECK-NEXT:  %92 = BinaryOperatorInst '!==', %91, undefined : undefined
// CHECK-NEXT:  %93 = CondBranchInst %92, %BB36, %BB33
// CHECK-NEXT:%BB33:
// CHECK-NEXT:  %94 = StoreStackInst 1 : number, %71
// CHECK-NEXT:  %95 = BranchInst %BB36
// CHECK-NEXT:%BB36:
// CHECK-NEXT:  %96 = TryStartInst %BB37, %BB38
// CHECK-NEXT:%BB37:
// CHECK-NEXT:  %97 = CatchInst
// CHECK-NEXT:  %98 = StoreStackInst %97, %72
// CHECK-NEXT:  %99 = BranchInst %BB27
// CHECK-NEXT:%BB39:
// CHECK-NEXT:  %100 = StoreStackInst undefined : undefined, %71
// CHECK-NEXT:  %101 = LoadStackInst %69
// CHECK-NEXT:  %102 = CondBranchInst %101, %BB40, %BB41
// CHECK-NEXT:%BB38:
// CHECK-NEXT:  %103 = LoadStackInst %71
// CHECK-NEXT:  %104 = StorePropertyInst %103, globalObject : object, "b" : string
// CHECK-NEXT:  %105 = BranchInst %BB42
// CHECK-NEXT:%BB42:
// CHECK-NEXT:  %106 = TryEndInst
// CHECK-NEXT:  %107 = BranchInst %BB39
// CHECK-NEXT:%BB41:
// CHECK-NEXT:  %108 = IteratorNextInst %64, %65
// CHECK-NEXT:  %109 = LoadStackInst %64
// CHECK-NEXT:  %110 = BinaryOperatorInst '===', %109, undefined : undefined
// CHECK-NEXT:  %111 = StoreStackInst %110, %69
// CHECK-NEXT:  %112 = CondBranchInst %110, %BB40, %BB43
// CHECK-NEXT:%BB43:
// CHECK-NEXT:  %113 = StoreStackInst %108, %71
// CHECK-NEXT:  %114 = BranchInst %BB40
// CHECK-NEXT:%BB40:
// CHECK-NEXT:  %115 = TryStartInst %BB44, %BB45
// CHECK-NEXT:%BB44:
// CHECK-NEXT:  %116 = CatchInst
// CHECK-NEXT:  %117 = StoreStackInst %116, %72
// CHECK-NEXT:  %118 = BranchInst %BB27
// CHECK-NEXT:%BB46:
// CHECK-NEXT:  %119 = LoadStackInst %69
// CHECK-NEXT:  %120 = CondBranchInst %119, %BB47, %BB48
// CHECK-NEXT:%BB45:
// CHECK-NEXT:  %121 = LoadStackInst %71
// CHECK-NEXT:  %122 = StorePropertyInst %121, globalObject : object, "c" : string
// CHECK-NEXT:  %123 = BranchInst %BB49
// CHECK-NEXT:%BB49:
// CHECK-NEXT:  %124 = TryEndInst
// CHECK-NEXT:  %125 = BranchInst %BB46
// CHECK-NEXT:%BB48:
// CHECK-NEXT:  %126 = IteratorCloseInst %64, false : boolean
// CHECK-NEXT:  %127 = BranchInst %BB47
// CHECK-NEXT:%BB47:
// CHECK-NEXT:  %128 = BranchInst %BB50
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %129 = IteratorCloseInst %64, true : boolean
// CHECK-NEXT:  %130 = BranchInst %BB28
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %131 = LoadStackInst %72
// CHECK-NEXT:  %132 = ThrowInst %131
// CHECK-NEXT:%BB50:
// CHECK-NEXT:  %133 = TryEndInst
// CHECK-NEXT:  %134 = BranchInst %BB22
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %135 = IteratorCloseInst %4, false : boolean
// CHECK-NEXT:  %136 = BranchInst %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %137 = LoadStackInst %1
// CHECK-NEXT:  %138 = ReturnInst %137
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %139 = IteratorCloseInst %4, true : boolean
// CHECK-NEXT:  %140 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %141 = LoadStackInst %12
// CHECK-NEXT:  %142 = ThrowInst %141
// CHECK-NEXT:function_end
