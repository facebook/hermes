/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s | %FileCheck --match-full-lines %s

var [a, [b = 1, c] = [,2]] = x;

//CHECK-LABEL:function global()
//CHECK-NEXT:frame = [], globals = [a, b, c]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:  %1 = StoreStackInst undefined : undefined, %0
//CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %4 = LoadPropertyInst %3, "iterator" : string
//CHECK-NEXT:  %5 = LoadPropertyInst %2, %4
//CHECK-NEXT:  %6 = CallInst %5, %2
//CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %8 = LoadPropertyInst %7, "ensureObject" : string
//CHECK-NEXT:  %9 = CallInst %8, undefined : undefined, %6, "iterator is not an object" : string
//CHECK-NEXT:  %10 = LoadPropertyInst %6, "next" : string
//CHECK-NEXT:  %11 = AllocStackInst $?anon_1_iterDone
//CHECK-NEXT:  %12 = StoreStackInst undefined : undefined, %11
//CHECK-NEXT:  %13 = AllocStackInst $?anon_2_iterValue
//CHECK-NEXT:  %14 = AllocStackInst $?anon_3_exc
//CHECK-NEXT:  %15 = TryStartInst %BB1, %BB2
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %16 = LoadStackInst %11
//CHECK-NEXT:  %17 = CondBranchInst %16, %BB4, %BB5
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %18 = CatchInst
//CHECK-NEXT:  %19 = StoreStackInst %18, %14
//CHECK-NEXT:  %20 = BranchInst %BB3
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %21 = StoreStackInst undefined : undefined, %13
//CHECK-NEXT:  %22 = BranchInst %BB7
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %23 = BranchInst %BB8
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %24 = TryEndInst
//CHECK-NEXT:  %25 = BranchInst %BB6
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %26 = CallInst %10, %6
//CHECK-NEXT:  %27 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %28 = LoadPropertyInst %27, "ensureObject" : string
//CHECK-NEXT:  %29 = CallInst %28, undefined : undefined, %26, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %30 = LoadPropertyInst %26, "done" : string
//CHECK-NEXT:  %31 = StoreStackInst %30, %11
//CHECK-NEXT:  %32 = CondBranchInst %30, %BB9, %BB10
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %33 = LoadPropertyInst %26, "value" : string
//CHECK-NEXT:  %34 = StoreStackInst %33, %13
//CHECK-NEXT:  %35 = BranchInst %BB9
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %36 = TryStartInst %BB11, %BB12
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %37 = CatchInst
//CHECK-NEXT:  %38 = StoreStackInst %37, %14
//CHECK-NEXT:  %39 = BranchInst %BB3
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %40 = StoreStackInst undefined : undefined, %13
//CHECK-NEXT:  %41 = LoadStackInst %11
//CHECK-NEXT:  %42 = CondBranchInst %41, %BB14, %BB15
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %43 = LoadStackInst %13
//CHECK-NEXT:  %44 = StorePropertyInst %43, globalObject : object, "a" : string
//CHECK-NEXT:  %45 = BranchInst %BB16
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %46 = TryEndInst
//CHECK-NEXT:  %47 = BranchInst %BB13
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %48 = CallInst %10, %6
//CHECK-NEXT:  %49 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %50 = LoadPropertyInst %49, "ensureObject" : string
//CHECK-NEXT:  %51 = CallInst %50, undefined : undefined, %48, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %52 = LoadPropertyInst %48, "done" : string
//CHECK-NEXT:  %53 = StoreStackInst %52, %11
//CHECK-NEXT:  %54 = CondBranchInst %52, %BB17, %BB18
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %55 = LoadPropertyInst %48, "value" : string
//CHECK-NEXT:  %56 = StoreStackInst %55, %13
//CHECK-NEXT:  %57 = BranchInst %BB14
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %58 = LoadStackInst %13
//CHECK-NEXT:  %59 = BinaryOperatorInst '!==', %58, undefined : undefined
//CHECK-NEXT:  %60 = CondBranchInst %59, %BB19, %BB17
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %61 = AllocArrayInst 2 : number
//CHECK-NEXT:  %62 = StoreOwnPropertyInst 2 : number, %61 : object, 1 : number, true : boolean
//CHECK-NEXT:  %63 = StoreStackInst %61 : object, %13
//CHECK-NEXT:  %64 = BranchInst %BB19
//CHECK-NEXT:%BB19:
//CHECK-NEXT:  %65 = TryStartInst %BB20, %BB21
//CHECK-NEXT:%BB20:
//CHECK-NEXT:  %66 = CatchInst
//CHECK-NEXT:  %67 = StoreStackInst %66, %14
//CHECK-NEXT:  %68 = BranchInst %BB3
//CHECK-NEXT:%BB22:
//CHECK-NEXT:  %69 = LoadStackInst %11
//CHECK-NEXT:  %70 = CondBranchInst %69, %BB23, %BB24
//CHECK-NEXT:%BB21:
//CHECK-NEXT:  %71 = LoadStackInst %13
//CHECK-NEXT:  %72 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %73 = LoadPropertyInst %72, "iterator" : string
//CHECK-NEXT:  %74 = LoadPropertyInst %71, %73
//CHECK-NEXT:  %75 = CallInst %74, %71
//CHECK-NEXT:  %76 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %77 = LoadPropertyInst %76, "ensureObject" : string
//CHECK-NEXT:  %78 = CallInst %77, undefined : undefined, %75, "iterator is not an object" : string
//CHECK-NEXT:  %79 = LoadPropertyInst %75, "next" : string
//CHECK-NEXT:  %80 = AllocStackInst $?anon_4_iterDone
//CHECK-NEXT:  %81 = StoreStackInst undefined : undefined, %80
//CHECK-NEXT:  %82 = AllocStackInst $?anon_5_iterValue
//CHECK-NEXT:  %83 = AllocStackInst $?anon_6_exc
//CHECK-NEXT:  %84 = TryStartInst %BB25, %BB26
//CHECK-NEXT:%BB27:
//CHECK-NEXT:  %85 = LoadStackInst %80
//CHECK-NEXT:  %86 = CondBranchInst %85, %BB28, %BB29
//CHECK-NEXT:%BB25:
//CHECK-NEXT:  %87 = CatchInst
//CHECK-NEXT:  %88 = StoreStackInst %87, %83
//CHECK-NEXT:  %89 = BranchInst %BB27
//CHECK-NEXT:%BB30:
//CHECK-NEXT:  %90 = StoreStackInst undefined : undefined, %82
//CHECK-NEXT:  %91 = BranchInst %BB31
//CHECK-NEXT:%BB26:
//CHECK-NEXT:  %92 = BranchInst %BB32
//CHECK-NEXT:%BB32:
//CHECK-NEXT:  %93 = TryEndInst
//CHECK-NEXT:  %94 = BranchInst %BB30
//CHECK-NEXT:%BB31:
//CHECK-NEXT:  %95 = CallInst %79, %75
//CHECK-NEXT:  %96 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %97 = LoadPropertyInst %96, "ensureObject" : string
//CHECK-NEXT:  %98 = CallInst %97, undefined : undefined, %95, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %99 = LoadPropertyInst %95, "done" : string
//CHECK-NEXT:  %100 = StoreStackInst %99, %80
//CHECK-NEXT:  %101 = CondBranchInst %99, %BB33, %BB34
//CHECK-NEXT:%BB34:
//CHECK-NEXT:  %102 = LoadPropertyInst %95, "value" : string
//CHECK-NEXT:  %103 = StoreStackInst %102, %82
//CHECK-NEXT:  %104 = BranchInst %BB35
//CHECK-NEXT:%BB35:
//CHECK-NEXT:  %105 = LoadStackInst %82
//CHECK-NEXT:  %106 = BinaryOperatorInst '!==', %105, undefined : undefined
//CHECK-NEXT:  %107 = CondBranchInst %106, %BB36, %BB33
//CHECK-NEXT:%BB33:
//CHECK-NEXT:  %108 = StoreStackInst 1 : number, %82
//CHECK-NEXT:  %109 = BranchInst %BB36
//CHECK-NEXT:%BB36:
//CHECK-NEXT:  %110 = TryStartInst %BB37, %BB38
//CHECK-NEXT:%BB37:
//CHECK-NEXT:  %111 = CatchInst
//CHECK-NEXT:  %112 = StoreStackInst %111, %83
//CHECK-NEXT:  %113 = BranchInst %BB27
//CHECK-NEXT:%BB39:
//CHECK-NEXT:  %114 = StoreStackInst undefined : undefined, %82
//CHECK-NEXT:  %115 = LoadStackInst %80
//CHECK-NEXT:  %116 = CondBranchInst %115, %BB40, %BB41
//CHECK-NEXT:%BB38:
//CHECK-NEXT:  %117 = LoadStackInst %82
//CHECK-NEXT:  %118 = StorePropertyInst %117, globalObject : object, "b" : string
//CHECK-NEXT:  %119 = BranchInst %BB42
//CHECK-NEXT:%BB42:
//CHECK-NEXT:  %120 = TryEndInst
//CHECK-NEXT:  %121 = BranchInst %BB39
//CHECK-NEXT:%BB41:
//CHECK-NEXT:  %122 = CallInst %79, %75
//CHECK-NEXT:  %123 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %124 = LoadPropertyInst %123, "ensureObject" : string
//CHECK-NEXT:  %125 = CallInst %124, undefined : undefined, %122, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %126 = LoadPropertyInst %122, "done" : string
//CHECK-NEXT:  %127 = StoreStackInst %126, %80
//CHECK-NEXT:  %128 = CondBranchInst %126, %BB40, %BB43
//CHECK-NEXT:%BB43:
//CHECK-NEXT:  %129 = LoadPropertyInst %122, "value" : string
//CHECK-NEXT:  %130 = StoreStackInst %129, %82
//CHECK-NEXT:  %131 = BranchInst %BB40
//CHECK-NEXT:%BB40:
//CHECK-NEXT:  %132 = TryStartInst %BB44, %BB45
//CHECK-NEXT:%BB44:
//CHECK-NEXT:  %133 = CatchInst
//CHECK-NEXT:  %134 = StoreStackInst %133, %83
//CHECK-NEXT:  %135 = BranchInst %BB27
//CHECK-NEXT:%BB46:
//CHECK-NEXT:  %136 = LoadStackInst %80
//CHECK-NEXT:  %137 = CondBranchInst %136, %BB47, %BB48
//CHECK-NEXT:%BB45:
//CHECK-NEXT:  %138 = LoadStackInst %82
//CHECK-NEXT:  %139 = StorePropertyInst %138, globalObject : object, "c" : string
//CHECK-NEXT:  %140 = BranchInst %BB49
//CHECK-NEXT:%BB49:
//CHECK-NEXT:  %141 = TryEndInst
//CHECK-NEXT:  %142 = BranchInst %BB46
//CHECK-NEXT:%BB48:
//CHECK-NEXT:  %143 = LoadPropertyInst %75, "return" : string
//CHECK-NEXT:  %144 = CompareBranchInst '===', %143, undefined : undefined, %BB50, %BB51
//CHECK-NEXT:%BB47:
//CHECK-NEXT:  %145 = BranchInst %BB52
//CHECK-NEXT:%BB51:
//CHECK-NEXT:  %146 = CallInst %143, %75
//CHECK-NEXT:  %147 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %148 = LoadPropertyInst %147, "ensureObject" : string
//CHECK-NEXT:  %149 = CallInst %148, undefined : undefined, %146, "iterator.close() did not return an object" : string
//CHECK-NEXT:  %150 = BranchInst %BB50
//CHECK-NEXT:%BB50:
//CHECK-NEXT:  %151 = BranchInst %BB47
//CHECK-NEXT:%BB29:
//CHECK-NEXT:  %152 = LoadPropertyInst %75, "return" : string
//CHECK-NEXT:  %153 = CompareBranchInst '===', %152, undefined : undefined, %BB53, %BB54
//CHECK-NEXT:%BB28:
//CHECK-NEXT:  %154 = LoadStackInst %83
//CHECK-NEXT:  %155 = ThrowInst %154
//CHECK-NEXT:%BB54:
//CHECK-NEXT:  %156 = TryStartInst %BB55, %BB56
//CHECK-NEXT:%BB53:
//CHECK-NEXT:  %157 = BranchInst %BB28
//CHECK-NEXT:%BB55:
//CHECK-NEXT:  %158 = CatchInst
//CHECK-NEXT:  %159 = BranchInst %BB53
//CHECK-NEXT:%BB56:
//CHECK-NEXT:  %160 = CallInst %152, %75
//CHECK-NEXT:  %161 = BranchInst %BB57
//CHECK-NEXT:%BB57:
//CHECK-NEXT:  %162 = TryEndInst
//CHECK-NEXT:  %163 = BranchInst %BB53
//CHECK-NEXT:%BB52:
//CHECK-NEXT:  %164 = TryEndInst
//CHECK-NEXT:  %165 = BranchInst %BB22
//CHECK-NEXT:%BB24:
//CHECK-NEXT:  %166 = LoadPropertyInst %6, "return" : string
//CHECK-NEXT:  %167 = CompareBranchInst '===', %166, undefined : undefined, %BB58, %BB59
//CHECK-NEXT:%BB23:
//CHECK-NEXT:  %168 = LoadStackInst %0
//CHECK-NEXT:  %169 = ReturnInst %168
//CHECK-NEXT:%BB59:
//CHECK-NEXT:  %170 = CallInst %166, %6
//CHECK-NEXT:  %171 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %172 = LoadPropertyInst %171, "ensureObject" : string
//CHECK-NEXT:  %173 = CallInst %172, undefined : undefined, %170, "iterator.close() did not return an object" : string
//CHECK-NEXT:  %174 = BranchInst %BB58
//CHECK-NEXT:%BB58:
//CHECK-NEXT:  %175 = BranchInst %BB23
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %176 = LoadPropertyInst %6, "return" : string
//CHECK-NEXT:  %177 = CompareBranchInst '===', %176, undefined : undefined, %BB60, %BB61
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %178 = LoadStackInst %14
//CHECK-NEXT:  %179 = ThrowInst %178
//CHECK-NEXT:%BB61:
//CHECK-NEXT:  %180 = TryStartInst %BB62, %BB63
//CHECK-NEXT:%BB60:
//CHECK-NEXT:  %181 = BranchInst %BB4
//CHECK-NEXT:%BB62:
//CHECK-NEXT:  %182 = CatchInst
//CHECK-NEXT:  %183 = BranchInst %BB60
//CHECK-NEXT:%BB63:
//CHECK-NEXT:  %184 = CallInst %176, %6
//CHECK-NEXT:  %185 = BranchInst %BB64
//CHECK-NEXT:%BB64:
//CHECK-NEXT:  %186 = TryEndInst
//CHECK-NEXT:  %187 = BranchInst %BB60
//CHECK-NEXT:function_end

