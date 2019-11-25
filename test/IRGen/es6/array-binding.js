/**
 * Copyright (c) Facebook, Inc. and its affiliates.
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
//CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %4 = LoadPropertyInst %3, "iterator" : string
//CHECK-NEXT:  %5 = LoadPropertyInst %2, %4
//CHECK-NEXT:  %6 = CallInst %5, %2
//CHECK-NEXT:  %7 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %6, "iterator is not an object" : string
//CHECK-NEXT:  %8 = LoadPropertyInst %6, "next" : string
//CHECK-NEXT:  %9 = AllocStackInst $?anon_1_iterDone
//CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
//CHECK-NEXT:  %11 = AllocStackInst $?anon_2_iterValue
//CHECK-NEXT:  %12 = AllocStackInst $?anon_3_exc
//CHECK-NEXT:  %13 = TryStartInst %BB1, %BB2
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %14 = LoadStackInst %9
//CHECK-NEXT:  %15 = CondBranchInst %14, %BB4, %BB5
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %16 = CatchInst
//CHECK-NEXT:  %17 = StoreStackInst %16, %12
//CHECK-NEXT:  %18 = BranchInst %BB3
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %19 = StoreStackInst undefined : undefined, %11
//CHECK-NEXT:  %20 = BranchInst %BB7
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %21 = BranchInst %BB8
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %22 = TryEndInst
//CHECK-NEXT:  %23 = BranchInst %BB6
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %24 = CallInst %8, %6
//CHECK-NEXT:  %25 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %24, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %26 = LoadPropertyInst %24, "done" : string
//CHECK-NEXT:  %27 = StoreStackInst %26, %9
//CHECK-NEXT:  %28 = CondBranchInst %26, %BB9, %BB10
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %29 = LoadPropertyInst %24, "value" : string
//CHECK-NEXT:  %30 = StoreStackInst %29, %11
//CHECK-NEXT:  %31 = BranchInst %BB9
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %32 = TryStartInst %BB11, %BB12
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %33 = CatchInst
//CHECK-NEXT:  %34 = StoreStackInst %33, %12
//CHECK-NEXT:  %35 = BranchInst %BB3
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %36 = StoreStackInst undefined : undefined, %11
//CHECK-NEXT:  %37 = LoadStackInst %9
//CHECK-NEXT:  %38 = CondBranchInst %37, %BB14, %BB15
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %39 = LoadStackInst %11
//CHECK-NEXT:  %40 = StorePropertyInst %39, globalObject : object, "a" : string
//CHECK-NEXT:  %41 = BranchInst %BB16
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %42 = TryEndInst
//CHECK-NEXT:  %43 = BranchInst %BB13
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %44 = CallInst %8, %6
//CHECK-NEXT:  %45 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %44, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %46 = LoadPropertyInst %44, "done" : string
//CHECK-NEXT:  %47 = StoreStackInst %46, %9
//CHECK-NEXT:  %48 = CondBranchInst %46, %BB17, %BB18
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %49 = LoadPropertyInst %44, "value" : string
//CHECK-NEXT:  %50 = StoreStackInst %49, %11
//CHECK-NEXT:  %51 = BranchInst %BB14
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %52 = LoadStackInst %11
//CHECK-NEXT:  %53 = BinaryOperatorInst '!==', %52, undefined : undefined
//CHECK-NEXT:  %54 = CondBranchInst %53, %BB19, %BB17
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %55 = AllocArrayInst 2 : number
//CHECK-NEXT:  %56 = StoreOwnPropertyInst 2 : number, %55 : object, 1 : number, true : boolean
//CHECK-NEXT:  %57 = StoreStackInst %55 : object, %11
//CHECK-NEXT:  %58 = BranchInst %BB19
//CHECK-NEXT:%BB19:
//CHECK-NEXT:  %59 = TryStartInst %BB20, %BB21
//CHECK-NEXT:%BB20:
//CHECK-NEXT:  %60 = CatchInst
//CHECK-NEXT:  %61 = StoreStackInst %60, %12
//CHECK-NEXT:  %62 = BranchInst %BB3
//CHECK-NEXT:%BB22:
//CHECK-NEXT:  %63 = LoadStackInst %9
//CHECK-NEXT:  %64 = CondBranchInst %63, %BB23, %BB24
//CHECK-NEXT:%BB21:
//CHECK-NEXT:  %65 = LoadStackInst %11
//CHECK-NEXT:  %66 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %67 = LoadPropertyInst %66, "iterator" : string
//CHECK-NEXT:  %68 = LoadPropertyInst %65, %67
//CHECK-NEXT:  %69 = CallInst %68, %65
//CHECK-NEXT:  %70 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %69, "iterator is not an object" : string
//CHECK-NEXT:  %71 = LoadPropertyInst %69, "next" : string
//CHECK-NEXT:  %72 = AllocStackInst $?anon_4_iterDone
//CHECK-NEXT:  %73 = StoreStackInst undefined : undefined, %72
//CHECK-NEXT:  %74 = AllocStackInst $?anon_5_iterValue
//CHECK-NEXT:  %75 = AllocStackInst $?anon_6_exc
//CHECK-NEXT:  %76 = TryStartInst %BB25, %BB26
//CHECK-NEXT:%BB27:
//CHECK-NEXT:  %77 = LoadStackInst %72
//CHECK-NEXT:  %78 = CondBranchInst %77, %BB28, %BB29
//CHECK-NEXT:%BB25:
//CHECK-NEXT:  %79 = CatchInst
//CHECK-NEXT:  %80 = StoreStackInst %79, %75
//CHECK-NEXT:  %81 = BranchInst %BB27
//CHECK-NEXT:%BB30:
//CHECK-NEXT:  %82 = StoreStackInst undefined : undefined, %74
//CHECK-NEXT:  %83 = BranchInst %BB31
//CHECK-NEXT:%BB26:
//CHECK-NEXT:  %84 = BranchInst %BB32
//CHECK-NEXT:%BB32:
//CHECK-NEXT:  %85 = TryEndInst
//CHECK-NEXT:  %86 = BranchInst %BB30
//CHECK-NEXT:%BB31:
//CHECK-NEXT:  %87 = CallInst %71, %69
//CHECK-NEXT:  %88 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %87, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %89 = LoadPropertyInst %87, "done" : string
//CHECK-NEXT:  %90 = StoreStackInst %89, %72
//CHECK-NEXT:  %91 = CondBranchInst %89, %BB33, %BB34
//CHECK-NEXT:%BB34:
//CHECK-NEXT:  %92 = LoadPropertyInst %87, "value" : string
//CHECK-NEXT:  %93 = StoreStackInst %92, %74
//CHECK-NEXT:  %94 = BranchInst %BB35
//CHECK-NEXT:%BB35:
//CHECK-NEXT:  %95 = LoadStackInst %74
//CHECK-NEXT:  %96 = BinaryOperatorInst '!==', %95, undefined : undefined
//CHECK-NEXT:  %97 = CondBranchInst %96, %BB36, %BB33
//CHECK-NEXT:%BB33:
//CHECK-NEXT:  %98 = StoreStackInst 1 : number, %74
//CHECK-NEXT:  %99 = BranchInst %BB36
//CHECK-NEXT:%BB36:
//CHECK-NEXT:  %100 = TryStartInst %BB37, %BB38
//CHECK-NEXT:%BB37:
//CHECK-NEXT:  %101 = CatchInst
//CHECK-NEXT:  %102 = StoreStackInst %101, %75
//CHECK-NEXT:  %103 = BranchInst %BB27
//CHECK-NEXT:%BB39:
//CHECK-NEXT:  %104 = StoreStackInst undefined : undefined, %74
//CHECK-NEXT:  %105 = LoadStackInst %72
//CHECK-NEXT:  %106 = CondBranchInst %105, %BB40, %BB41
//CHECK-NEXT:%BB38:
//CHECK-NEXT:  %107 = LoadStackInst %74
//CHECK-NEXT:  %108 = StorePropertyInst %107, globalObject : object, "b" : string
//CHECK-NEXT:  %109 = BranchInst %BB42
//CHECK-NEXT:%BB42:
//CHECK-NEXT:  %110 = TryEndInst
//CHECK-NEXT:  %111 = BranchInst %BB39
//CHECK-NEXT:%BB41:
//CHECK-NEXT:  %112 = CallInst %71, %69
//CHECK-NEXT:  %113 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %112, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %114 = LoadPropertyInst %112, "done" : string
//CHECK-NEXT:  %115 = StoreStackInst %114, %72
//CHECK-NEXT:  %116 = CondBranchInst %114, %BB40, %BB43
//CHECK-NEXT:%BB43:
//CHECK-NEXT:  %117 = LoadPropertyInst %112, "value" : string
//CHECK-NEXT:  %118 = StoreStackInst %117, %74
//CHECK-NEXT:  %119 = BranchInst %BB40
//CHECK-NEXT:%BB40:
//CHECK-NEXT:  %120 = TryStartInst %BB44, %BB45
//CHECK-NEXT:%BB44:
//CHECK-NEXT:  %121 = CatchInst
//CHECK-NEXT:  %122 = StoreStackInst %121, %75
//CHECK-NEXT:  %123 = BranchInst %BB27
//CHECK-NEXT:%BB46:
//CHECK-NEXT:  %124 = LoadStackInst %72
//CHECK-NEXT:  %125 = CondBranchInst %124, %BB47, %BB48
//CHECK-NEXT:%BB45:
//CHECK-NEXT:  %126 = LoadStackInst %74
//CHECK-NEXT:  %127 = StorePropertyInst %126, globalObject : object, "c" : string
//CHECK-NEXT:  %128 = BranchInst %BB49
//CHECK-NEXT:%BB49:
//CHECK-NEXT:  %129 = TryEndInst
//CHECK-NEXT:  %130 = BranchInst %BB46
//CHECK-NEXT:%BB48:
//CHECK-NEXT:  %131 = LoadPropertyInst %69, "return" : string
//CHECK-NEXT:  %132 = CompareBranchInst '===', %131, undefined : undefined, %BB50, %BB51
//CHECK-NEXT:%BB47:
//CHECK-NEXT:  %133 = BranchInst %BB52
//CHECK-NEXT:%BB51:
//CHECK-NEXT:  %134 = CallInst %131, %69
//CHECK-NEXT:  %135 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %134, "iterator.close() did not return an object" : string
//CHECK-NEXT:  %136 = BranchInst %BB50
//CHECK-NEXT:%BB50:
//CHECK-NEXT:  %137 = BranchInst %BB47
//CHECK-NEXT:%BB29:
//CHECK-NEXT:  %138 = LoadPropertyInst %69, "return" : string
//CHECK-NEXT:  %139 = CompareBranchInst '===', %138, undefined : undefined, %BB53, %BB54
//CHECK-NEXT:%BB28:
//CHECK-NEXT:  %140 = LoadStackInst %75
//CHECK-NEXT:  %141 = ThrowInst %140
//CHECK-NEXT:%BB54:
//CHECK-NEXT:  %142 = TryStartInst %BB55, %BB56
//CHECK-NEXT:%BB53:
//CHECK-NEXT:  %143 = BranchInst %BB28
//CHECK-NEXT:%BB55:
//CHECK-NEXT:  %144 = CatchInst
//CHECK-NEXT:  %145 = BranchInst %BB53
//CHECK-NEXT:%BB56:
//CHECK-NEXT:  %146 = CallInst %138, %69
//CHECK-NEXT:  %147 = BranchInst %BB57
//CHECK-NEXT:%BB57:
//CHECK-NEXT:  %148 = TryEndInst
//CHECK-NEXT:  %149 = BranchInst %BB53
//CHECK-NEXT:%BB52:
//CHECK-NEXT:  %150 = TryEndInst
//CHECK-NEXT:  %151 = BranchInst %BB22
//CHECK-NEXT:%BB24:
//CHECK-NEXT:  %152 = LoadPropertyInst %6, "return" : string
//CHECK-NEXT:  %153 = CompareBranchInst '===', %152, undefined : undefined, %BB58, %BB59
//CHECK-NEXT:%BB23:
//CHECK-NEXT:  %154 = LoadStackInst %0
//CHECK-NEXT:  %155 = ReturnInst %154
//CHECK-NEXT:%BB59:
//CHECK-NEXT:  %156 = CallInst %152, %6
//CHECK-NEXT:  %157 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %156, "iterator.close() did not return an object" : string
//CHECK-NEXT:  %158 = BranchInst %BB58
//CHECK-NEXT:%BB58:
//CHECK-NEXT:  %159 = BranchInst %BB23
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %160 = LoadPropertyInst %6, "return" : string
//CHECK-NEXT:  %161 = CompareBranchInst '===', %160, undefined : undefined, %BB60, %BB61
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %162 = LoadStackInst %12
//CHECK-NEXT:  %163 = ThrowInst %162
//CHECK-NEXT:%BB61:
//CHECK-NEXT:  %164 = TryStartInst %BB62, %BB63
//CHECK-NEXT:%BB60:
//CHECK-NEXT:  %165 = BranchInst %BB4
//CHECK-NEXT:%BB62:
//CHECK-NEXT:  %166 = CatchInst
//CHECK-NEXT:  %167 = BranchInst %BB60
//CHECK-NEXT:%BB63:
//CHECK-NEXT:  %168 = CallInst %160, %6
//CHECK-NEXT:  %169 = BranchInst %BB64
//CHECK-NEXT:%BB64:
//CHECK-NEXT:  %170 = TryEndInst
//CHECK-NEXT:  %171 = BranchInst %BB60
//CHECK-NEXT:function_end

