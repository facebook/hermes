/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

var [a, [b = 1, c] = [,2]] = x;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "a" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "b" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "c" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_iter
// CHECK-NEXT:  %7 = AllocStackInst $?anon_2_sourceOrNext
// CHECK-NEXT:  %8 = StoreStackInst %5, %7
// CHECK-NEXT:  %9 = IteratorBeginInst %7
// CHECK-NEXT:  %10 = StoreStackInst %9, %6
// CHECK-NEXT:  %11 = AllocStackInst $?anon_3_iterDone
// CHECK-NEXT:  %12 = StoreStackInst undefined : undefined, %11
// CHECK-NEXT:  %13 = AllocStackInst $?anon_4_iterValue
// CHECK-NEXT:  %14 = AllocStackInst $?anon_5_exc
// CHECK-NEXT:  %15 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst %11
// CHECK-NEXT:  %17 = CondBranchInst %16, %BB4, %BB5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %18 = CatchInst
// CHECK-NEXT:  %19 = StoreStackInst %18, %14
// CHECK-NEXT:  %20 = BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %21 = StoreStackInst undefined : undefined, %13
// CHECK-NEXT:  %22 = BranchInst %BB7
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %23 = BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %24 = TryEndInst
// CHECK-NEXT:  %25 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %26 = IteratorNextInst %6, %7
// CHECK-NEXT:  %27 = LoadStackInst %6
// CHECK-NEXT:  %28 = BinaryOperatorInst '===', %27, undefined : undefined
// CHECK-NEXT:  %29 = StoreStackInst %28, %11
// CHECK-NEXT:  %30 = CondBranchInst %28, %BB9, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %31 = StoreStackInst %26, %13
// CHECK-NEXT:  %32 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %33 = TryStartInst %BB11, %BB12
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %34 = CatchInst
// CHECK-NEXT:  %35 = StoreStackInst %34, %14
// CHECK-NEXT:  %36 = BranchInst %BB3
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %37 = StoreStackInst undefined : undefined, %13
// CHECK-NEXT:  %38 = LoadStackInst %11
// CHECK-NEXT:  %39 = CondBranchInst %38, %BB14, %BB15
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %40 = LoadStackInst %13
// CHECK-NEXT:  %41 = StorePropertyLooseInst %40, globalObject : object, "a" : string
// CHECK-NEXT:  %42 = BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %43 = TryEndInst
// CHECK-NEXT:  %44 = BranchInst %BB13
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %45 = IteratorNextInst %6, %7
// CHECK-NEXT:  %46 = LoadStackInst %6
// CHECK-NEXT:  %47 = BinaryOperatorInst '===', %46, undefined : undefined
// CHECK-NEXT:  %48 = StoreStackInst %47, %11
// CHECK-NEXT:  %49 = CondBranchInst %47, %BB17, %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %50 = StoreStackInst %45, %13
// CHECK-NEXT:  %51 = BranchInst %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %52 = LoadStackInst %13
// CHECK-NEXT:  %53 = BinaryOperatorInst '!==', %52, undefined : undefined
// CHECK-NEXT:  %54 = CondBranchInst %53, %BB19, %BB17
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %55 = AllocArrayInst 2 : number
// CHECK-NEXT:  %56 = StoreOwnPropertyInst 2 : number, %55 : object, 1 : number, true : boolean
// CHECK-NEXT:  %57 = StoreStackInst %55 : object, %13
// CHECK-NEXT:  %58 = BranchInst %BB19
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %59 = TryStartInst %BB20, %BB21
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %60 = CatchInst
// CHECK-NEXT:  %61 = StoreStackInst %60, %14
// CHECK-NEXT:  %62 = BranchInst %BB3
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %63 = LoadStackInst %11
// CHECK-NEXT:  %64 = CondBranchInst %63, %BB23, %BB24
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %65 = LoadStackInst %13
// CHECK-NEXT:  %66 = AllocStackInst $?anon_6_iter
// CHECK-NEXT:  %67 = AllocStackInst $?anon_7_sourceOrNext
// CHECK-NEXT:  %68 = StoreStackInst %65, %67
// CHECK-NEXT:  %69 = IteratorBeginInst %67
// CHECK-NEXT:  %70 = StoreStackInst %69, %66
// CHECK-NEXT:  %71 = AllocStackInst $?anon_8_iterDone
// CHECK-NEXT:  %72 = StoreStackInst undefined : undefined, %71
// CHECK-NEXT:  %73 = AllocStackInst $?anon_9_iterValue
// CHECK-NEXT:  %74 = AllocStackInst $?anon_10_exc
// CHECK-NEXT:  %75 = TryStartInst %BB25, %BB26
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %76 = LoadStackInst %71
// CHECK-NEXT:  %77 = CondBranchInst %76, %BB28, %BB29
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %78 = CatchInst
// CHECK-NEXT:  %79 = StoreStackInst %78, %74
// CHECK-NEXT:  %80 = BranchInst %BB27
// CHECK-NEXT:%BB30:
// CHECK-NEXT:  %81 = StoreStackInst undefined : undefined, %73
// CHECK-NEXT:  %82 = BranchInst %BB31
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %83 = BranchInst %BB32
// CHECK-NEXT:%BB32:
// CHECK-NEXT:  %84 = TryEndInst
// CHECK-NEXT:  %85 = BranchInst %BB30
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %86 = IteratorNextInst %66, %67
// CHECK-NEXT:  %87 = LoadStackInst %66
// CHECK-NEXT:  %88 = BinaryOperatorInst '===', %87, undefined : undefined
// CHECK-NEXT:  %89 = StoreStackInst %88, %71
// CHECK-NEXT:  %90 = CondBranchInst %88, %BB33, %BB34
// CHECK-NEXT:%BB34:
// CHECK-NEXT:  %91 = StoreStackInst %86, %73
// CHECK-NEXT:  %92 = BranchInst %BB35
// CHECK-NEXT:%BB35:
// CHECK-NEXT:  %93 = LoadStackInst %73
// CHECK-NEXT:  %94 = BinaryOperatorInst '!==', %93, undefined : undefined
// CHECK-NEXT:  %95 = CondBranchInst %94, %BB36, %BB33
// CHECK-NEXT:%BB33:
// CHECK-NEXT:  %96 = StoreStackInst 1 : number, %73
// CHECK-NEXT:  %97 = BranchInst %BB36
// CHECK-NEXT:%BB36:
// CHECK-NEXT:  %98 = TryStartInst %BB37, %BB38
// CHECK-NEXT:%BB37:
// CHECK-NEXT:  %99 = CatchInst
// CHECK-NEXT:  %100 = StoreStackInst %99, %74
// CHECK-NEXT:  %101 = BranchInst %BB27
// CHECK-NEXT:%BB39:
// CHECK-NEXT:  %102 = StoreStackInst undefined : undefined, %73
// CHECK-NEXT:  %103 = LoadStackInst %71
// CHECK-NEXT:  %104 = CondBranchInst %103, %BB40, %BB41
// CHECK-NEXT:%BB38:
// CHECK-NEXT:  %105 = LoadStackInst %73
// CHECK-NEXT:  %106 = StorePropertyLooseInst %105, globalObject : object, "b" : string
// CHECK-NEXT:  %107 = BranchInst %BB42
// CHECK-NEXT:%BB42:
// CHECK-NEXT:  %108 = TryEndInst
// CHECK-NEXT:  %109 = BranchInst %BB39
// CHECK-NEXT:%BB41:
// CHECK-NEXT:  %110 = IteratorNextInst %66, %67
// CHECK-NEXT:  %111 = LoadStackInst %66
// CHECK-NEXT:  %112 = BinaryOperatorInst '===', %111, undefined : undefined
// CHECK-NEXT:  %113 = StoreStackInst %112, %71
// CHECK-NEXT:  %114 = CondBranchInst %112, %BB40, %BB43
// CHECK-NEXT:%BB43:
// CHECK-NEXT:  %115 = StoreStackInst %110, %73
// CHECK-NEXT:  %116 = BranchInst %BB40
// CHECK-NEXT:%BB40:
// CHECK-NEXT:  %117 = TryStartInst %BB44, %BB45
// CHECK-NEXT:%BB44:
// CHECK-NEXT:  %118 = CatchInst
// CHECK-NEXT:  %119 = StoreStackInst %118, %74
// CHECK-NEXT:  %120 = BranchInst %BB27
// CHECK-NEXT:%BB46:
// CHECK-NEXT:  %121 = LoadStackInst %71
// CHECK-NEXT:  %122 = CondBranchInst %121, %BB47, %BB48
// CHECK-NEXT:%BB45:
// CHECK-NEXT:  %123 = LoadStackInst %73
// CHECK-NEXT:  %124 = StorePropertyLooseInst %123, globalObject : object, "c" : string
// CHECK-NEXT:  %125 = BranchInst %BB49
// CHECK-NEXT:%BB49:
// CHECK-NEXT:  %126 = TryEndInst
// CHECK-NEXT:  %127 = BranchInst %BB46
// CHECK-NEXT:%BB48:
// CHECK-NEXT:  %128 = IteratorCloseInst %66, false : boolean
// CHECK-NEXT:  %129 = BranchInst %BB47
// CHECK-NEXT:%BB47:
// CHECK-NEXT:  %130 = BranchInst %BB50
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %131 = IteratorCloseInst %66, true : boolean
// CHECK-NEXT:  %132 = BranchInst %BB28
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %133 = LoadStackInst %74
// CHECK-NEXT:  %134 = ThrowInst %133
// CHECK-NEXT:%BB50:
// CHECK-NEXT:  %135 = TryEndInst
// CHECK-NEXT:  %136 = BranchInst %BB22
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %137 = IteratorCloseInst %6, false : boolean
// CHECK-NEXT:  %138 = BranchInst %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %139 = LoadStackInst %3
// CHECK-NEXT:  %140 = ReturnInst %139
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %141 = IteratorCloseInst %6, true : boolean
// CHECK-NEXT:  %142 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %143 = LoadStackInst %14
// CHECK-NEXT:  %144 = ThrowInst %143
// CHECK-NEXT:function_end
