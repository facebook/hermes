/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck --match-full-lines %s

var [a, [b = 1, c] = [,2]] = x;

//CHECK-LABEL:function global()
//CHECK-NEXT:frame = [], globals = [a, b, c]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:  %1 = StoreStackInst undefined : undefined, %0
//CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:  %3 = AllocStackInst $?anon_1_iter
//CHECK-NEXT:  %4 = AllocStackInst $?anon_2_sourceOrNext
//CHECK-NEXT:  %5 = StoreStackInst %2, %4
//CHECK-NEXT:  %6 = IteratorBeginInst %4
//CHECK-NEXT:  %7 = StoreStackInst %6, %3
//CHECK-NEXT:  %8 = AllocStackInst $?anon_3_iterDone
//CHECK-NEXT:  %9 = StoreStackInst undefined : undefined, %8
//CHECK-NEXT:  %10 = AllocStackInst $?anon_4_iterValue
//CHECK-NEXT:  %11 = AllocStackInst $?anon_5_exc
//CHECK-NEXT:  %12 = TryStartInst %BB1, %BB2
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %13 = LoadStackInst %8
//CHECK-NEXT:  %14 = CondBranchInst %13, %BB4, %BB5
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %15 = CatchInst
//CHECK-NEXT:  %16 = StoreStackInst %15, %11
//CHECK-NEXT:  %17 = BranchInst %BB3
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %18 = StoreStackInst undefined : undefined, %10
//CHECK-NEXT:  %19 = BranchInst %BB7
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %20 = BranchInst %BB8
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %21 = TryEndInst
//CHECK-NEXT:  %22 = BranchInst %BB6
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %23 = IteratorNextInst %3, %4
//CHECK-NEXT:  %24 = LoadStackInst %3
//CHECK-NEXT:  %25 = BinaryOperatorInst '===', %24, undefined : undefined
//CHECK-NEXT:  %26 = StoreStackInst %25, %8
//CHECK-NEXT:  %27 = CondBranchInst %25, %BB9, %BB10
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %28 = StoreStackInst %23, %10
//CHECK-NEXT:  %29 = BranchInst %BB9
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %30 = TryStartInst %BB11, %BB12
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %31 = CatchInst
//CHECK-NEXT:  %32 = StoreStackInst %31, %11
//CHECK-NEXT:  %33 = BranchInst %BB3
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %34 = StoreStackInst undefined : undefined, %10
//CHECK-NEXT:  %35 = LoadStackInst %8
//CHECK-NEXT:  %36 = CondBranchInst %35, %BB14, %BB15
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %37 = LoadStackInst %10
//CHECK-NEXT:  %38 = StorePropertyInst %37, globalObject : object, "a" : string
//CHECK-NEXT:  %39 = BranchInst %BB16
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %40 = TryEndInst
//CHECK-NEXT:  %41 = BranchInst %BB13
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %42 = IteratorNextInst %3, %4
//CHECK-NEXT:  %43 = LoadStackInst %3
//CHECK-NEXT:  %44 = BinaryOperatorInst '===', %43, undefined : undefined
//CHECK-NEXT:  %45 = StoreStackInst %44, %8
//CHECK-NEXT:  %46 = CondBranchInst %44, %BB17, %BB18
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %47 = StoreStackInst %42, %10
//CHECK-NEXT:  %48 = BranchInst %BB14
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %49 = LoadStackInst %10
//CHECK-NEXT:  %50 = BinaryOperatorInst '!==', %49, undefined : undefined
//CHECK-NEXT:  %51 = CondBranchInst %50, %BB19, %BB17
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %52 = AllocArrayInst 2 : number
//CHECK-NEXT:  %53 = StoreOwnPropertyInst 2 : number, %52 : object, 1 : number, true : boolean
//CHECK-NEXT:  %54 = StoreStackInst %52 : object, %10
//CHECK-NEXT:  %55 = BranchInst %BB19
//CHECK-NEXT:%BB19:
//CHECK-NEXT:  %56 = TryStartInst %BB20, %BB21
//CHECK-NEXT:%BB20:
//CHECK-NEXT:  %57 = CatchInst
//CHECK-NEXT:  %58 = StoreStackInst %57, %11
//CHECK-NEXT:  %59 = BranchInst %BB3
//CHECK-NEXT:%BB22:
//CHECK-NEXT:  %60 = LoadStackInst %8
//CHECK-NEXT:  %61 = CondBranchInst %60, %BB23, %BB24
//CHECK-NEXT:%BB21:
//CHECK-NEXT:  %62 = LoadStackInst %10
//CHECK-NEXT:  %63 = AllocStackInst $?anon_6_iter
//CHECK-NEXT:  %64 = AllocStackInst $?anon_7_sourceOrNext
//CHECK-NEXT:  %65 = StoreStackInst %62, %64
//CHECK-NEXT:  %66 = IteratorBeginInst %64
//CHECK-NEXT:  %67 = StoreStackInst %66, %63
//CHECK-NEXT:  %68 = AllocStackInst $?anon_8_iterDone
//CHECK-NEXT:  %69 = StoreStackInst undefined : undefined, %68
//CHECK-NEXT:  %70 = AllocStackInst $?anon_9_iterValue
//CHECK-NEXT:  %71 = AllocStackInst $?anon_10_exc
//CHECK-NEXT:  %72 = TryStartInst %BB25, %BB26
//CHECK-NEXT:%BB27:
//CHECK-NEXT:  %73 = LoadStackInst %68
//CHECK-NEXT:  %74 = CondBranchInst %73, %BB28, %BB29
//CHECK-NEXT:%BB25:
//CHECK-NEXT:  %75 = CatchInst
//CHECK-NEXT:  %76 = StoreStackInst %75, %71
//CHECK-NEXT:  %77 = BranchInst %BB27
//CHECK-NEXT:%BB30:
//CHECK-NEXT:  %78 = StoreStackInst undefined : undefined, %70
//CHECK-NEXT:  %79 = BranchInst %BB31
//CHECK-NEXT:%BB26:
//CHECK-NEXT:  %80 = BranchInst %BB32
//CHECK-NEXT:%BB32:
//CHECK-NEXT:  %81 = TryEndInst
//CHECK-NEXT:  %82 = BranchInst %BB30
//CHECK-NEXT:%BB31:
//CHECK-NEXT:  %83 = IteratorNextInst %63, %64
//CHECK-NEXT:  %84 = LoadStackInst %63
//CHECK-NEXT:  %85 = BinaryOperatorInst '===', %84, undefined : undefined
//CHECK-NEXT:  %86 = StoreStackInst %85, %68
//CHECK-NEXT:  %87 = CondBranchInst %85, %BB33, %BB34
//CHECK-NEXT:%BB34:
//CHECK-NEXT:  %88 = StoreStackInst %83, %70
//CHECK-NEXT:  %89 = BranchInst %BB35
//CHECK-NEXT:%BB35:
//CHECK-NEXT:  %90 = LoadStackInst %70
//CHECK-NEXT:  %91 = BinaryOperatorInst '!==', %90, undefined : undefined
//CHECK-NEXT:  %92 = CondBranchInst %91, %BB36, %BB33
//CHECK-NEXT:%BB33:
//CHECK-NEXT:  %93 = StoreStackInst 1 : number, %70
//CHECK-NEXT:  %94 = BranchInst %BB36
//CHECK-NEXT:%BB36:
//CHECK-NEXT:  %95 = TryStartInst %BB37, %BB38
//CHECK-NEXT:%BB37:
//CHECK-NEXT:  %96 = CatchInst
//CHECK-NEXT:  %97 = StoreStackInst %96, %71
//CHECK-NEXT:  %98 = BranchInst %BB27
//CHECK-NEXT:%BB39:
//CHECK-NEXT:  %99 = StoreStackInst undefined : undefined, %70
//CHECK-NEXT:  %100 = LoadStackInst %68
//CHECK-NEXT:  %101 = CondBranchInst %100, %BB40, %BB41
//CHECK-NEXT:%BB38:
//CHECK-NEXT:  %102 = LoadStackInst %70
//CHECK-NEXT:  %103 = StorePropertyInst %102, globalObject : object, "b" : string
//CHECK-NEXT:  %104 = BranchInst %BB42
//CHECK-NEXT:%BB42:
//CHECK-NEXT:  %105 = TryEndInst
//CHECK-NEXT:  %106 = BranchInst %BB39
//CHECK-NEXT:%BB41:
//CHECK-NEXT:  %107 = IteratorNextInst %63, %64
//CHECK-NEXT:  %108 = LoadStackInst %63
//CHECK-NEXT:  %109 = BinaryOperatorInst '===', %108, undefined : undefined
//CHECK-NEXT:  %110 = StoreStackInst %109, %68
//CHECK-NEXT:  %111 = CondBranchInst %109, %BB40, %BB43
//CHECK-NEXT:%BB43:
//CHECK-NEXT:  %112 = StoreStackInst %107, %70
//CHECK-NEXT:  %113 = BranchInst %BB40
//CHECK-NEXT:%BB40:
//CHECK-NEXT:  %114 = TryStartInst %BB44, %BB45
//CHECK-NEXT:%BB44:
//CHECK-NEXT:  %115 = CatchInst
//CHECK-NEXT:  %116 = StoreStackInst %115, %71
//CHECK-NEXT:  %117 = BranchInst %BB27
//CHECK-NEXT:%BB46:
//CHECK-NEXT:  %118 = LoadStackInst %68
//CHECK-NEXT:  %119 = CondBranchInst %118, %BB47, %BB48
//CHECK-NEXT:%BB45:
//CHECK-NEXT:  %120 = LoadStackInst %70
//CHECK-NEXT:  %121 = StorePropertyInst %120, globalObject : object, "c" : string
//CHECK-NEXT:  %122 = BranchInst %BB49
//CHECK-NEXT:%BB49:
//CHECK-NEXT:  %123 = TryEndInst
//CHECK-NEXT:  %124 = BranchInst %BB46
//CHECK-NEXT:%BB48:
//CHECK-NEXT:  %125 = IteratorCloseInst %63, false : boolean
//CHECK-NEXT:  %126 = BranchInst %BB47
//CHECK-NEXT:%BB47:
//CHECK-NEXT:  %127 = BranchInst %BB50
//CHECK-NEXT:%BB29:
//CHECK-NEXT:  %128 = IteratorCloseInst %63, true : boolean
//CHECK-NEXT:  %129 = BranchInst %BB28
//CHECK-NEXT:%BB28:
//CHECK-NEXT:  %130 = LoadStackInst %71
//CHECK-NEXT:  %131 = ThrowInst %130
//CHECK-NEXT:%BB50:
//CHECK-NEXT:  %132 = TryEndInst
//CHECK-NEXT:  %133 = BranchInst %BB22
//CHECK-NEXT:%BB24:
//CHECK-NEXT:  %134 = IteratorCloseInst %3, false : boolean
//CHECK-NEXT:  %135 = BranchInst %BB23
//CHECK-NEXT:%BB23:
//CHECK-NEXT:  %136 = LoadStackInst %0
//CHECK-NEXT:  %137 = ReturnInst %136
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %138 = IteratorCloseInst %3, true : boolean
//CHECK-NEXT:  %139 = BranchInst %BB4
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %140 = LoadStackInst %11
//CHECK-NEXT:  %141 = ThrowInst %140
//CHECK-NEXT:function_end

