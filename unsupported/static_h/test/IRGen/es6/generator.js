/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck %s --match-full-lines

function* simple() {
  yield 1;
}
//CHECK-LABEL:function simple()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_simple()
//CHECK-NEXT:  %1 = ReturnInst %0 : object
//CHECK-NEXT:function_end

//CHECK-LABEL:function ?anon_0_simple()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StartGeneratorInst
//CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
//CHECK-NEXT:  %2 = ResumeGeneratorInst %1
//CHECK-NEXT:  %3 = LoadStackInst %1
//CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %5 = AllocStackInst $?anon_1_isReturn
//CHECK-NEXT:  %6 = SaveAndYieldInst 1 : number, %BB3
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %7 = ReturnInst %2
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %8 = ResumeGeneratorInst %5
//CHECK-NEXT:  %9 = LoadStackInst %5
//CHECK-NEXT:  %10 = CondBranchInst %9, %BB4, %BB5
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %11 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %12 = ReturnInst %8
//CHECK-NEXT:function_end

function *useResult() {
  var x = yield 1;
}
//CHECK-LABEL:function useResult()
//CHECK-NEXT:frame = [x]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:  %1 = CreateGeneratorInst %?anon_0_useResult()
//CHECK-NEXT:  %2 = ReturnInst %1 : object
//CHECK-NEXT:function_end

//CHECK-LABEL:function ?anon_0_useResult()
//CHECK-NEXT:frame = [x]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StartGeneratorInst
//CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
//CHECK-NEXT:  %2 = ResumeGeneratorInst %1
//CHECK-NEXT:  %3 = LoadStackInst %1
//CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %5 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:  %6 = AllocStackInst $?anon_1_isReturn
//CHECK-NEXT:  %7 = SaveAndYieldInst 1 : number, %BB3
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %8 = ReturnInst %2
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %9 = ResumeGeneratorInst %6
//CHECK-NEXT:  %10 = LoadStackInst %6
//CHECK-NEXT:  %11 = CondBranchInst %10, %BB4, %BB5
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %12 = StoreFrameInst %9, [x]
//CHECK-NEXT:  %13 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %14 = ReturnInst %9
//CHECK-NEXT:function_end

function *loop(x) {
  var i = 0;
  while (true) {
    yield x[i++];
  }
}
//CHECK-LABEL:function loop()
//CHECK-NEXT:frame = [i]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:  %1 = CreateGeneratorInst %?anon_0_loop()
//CHECK-NEXT:  %2 = ReturnInst %1 : object
//CHECK-NEXT:function_end

//CHECK-LABEL:function ?anon_0_loop(x)
//CHECK-NEXT:frame = [i, x]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StartGeneratorInst
//CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
//CHECK-NEXT:  %2 = ResumeGeneratorInst %1
//CHECK-NEXT:  %3 = LoadStackInst %1
//CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %5 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:  %6 = StoreFrameInst %x, [x]
//CHECK-NEXT:  %7 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:  %8 = BranchInst %BB3
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %9 = ReturnInst %2
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %10 = LoadFrameInst [x]
//CHECK-NEXT:  %11 = LoadFrameInst [i]
//CHECK-NEXT:  %12 = AsNumericInst %11
//CHECK-NEXT:  %13 = UnaryOperatorInst '++', %12 : number|bigint
//CHECK-NEXT:  %14 = StoreFrameInst %13, [i]
//CHECK-NEXT:  %15 = LoadPropertyInst %10, %12 : number|bigint
//CHECK-NEXT:  %16 = AllocStackInst $?anon_1_isReturn
//CHECK-NEXT:  %17 = SaveAndYieldInst %15, %BB5
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %18 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %19 = CondBranchInst true : boolean, %BB4, %BB6
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %20 = CondBranchInst true : boolean, %BB4, %BB6
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %21 = BranchInst %BB7
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %22 = ResumeGeneratorInst %16
//CHECK-NEXT:  %23 = LoadStackInst %16
//CHECK-NEXT:  %24 = CondBranchInst %23, %BB9, %BB10
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %25 = BranchInst %BB8
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %26 = ReturnInst %22
//CHECK-NEXT:function_end

// Test generation of function expressions.
var simple2 = function*() {
  yield 1;
}
//CHECK-LABEL:function simple2()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_simple2()
//CHECK-NEXT:  %1 = ReturnInst %0 : object
//CHECK-NEXT:function_end

//CHECK-LABEL:function ?anon_0_simple2()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StartGeneratorInst
//CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
//CHECK-NEXT:  %2 = ResumeGeneratorInst %1
//CHECK-NEXT:  %3 = LoadStackInst %1
//CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %5 = AllocStackInst $?anon_1_isReturn
//CHECK-NEXT:  %6 = SaveAndYieldInst 1 : number, %BB3
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %7 = ReturnInst %2
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %8 = ResumeGeneratorInst %5
//CHECK-NEXT:  %9 = LoadStackInst %5
//CHECK-NEXT:  %10 = CondBranchInst %9, %BB4, %BB5
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %11 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %12 = ReturnInst %8
//CHECK-NEXT:function_end

var yieldStar = function*() {
  yield* foo();
}
//CHECK-LABEL:function yieldStar()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_yieldStar()
//CHECK-NEXT:  %1 = ReturnInst %0 : object
//CHECK-NEXT:function_end

//CHECK-LABEL:function ?anon_0_yieldStar()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StartGeneratorInst
//CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
//CHECK-NEXT:  %2 = ResumeGeneratorInst %1
//CHECK-NEXT:  %3 = LoadStackInst %1
//CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst globalObject : object, "foo" : string
//CHECK-NEXT:  %6 = CallInst %5, undefined : undefined
//CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %8 = LoadPropertyInst %7, "iterator" : string
//CHECK-NEXT:  %9 = LoadPropertyInst %6, %8
//CHECK-NEXT:  %10 = CallInst %9, %6
//CHECK-NEXT:  %11 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %10, "iterator is not an object" : string
//CHECK-NEXT:  %12 = LoadPropertyInst %10, "next" : string
//CHECK-NEXT:  %13 = AllocStackInst $?anon_1_received
//CHECK-NEXT:  %14 = StoreStackInst undefined : undefined, %13
//CHECK-NEXT:  %15 = AllocStackInst $?anon_2_isReturn
//CHECK-NEXT:  %16 = AllocStackInst $?anon_3_result
//CHECK-NEXT:  %17 = BranchInst %BB3
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %18 = ReturnInst %2
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %19 = LoadStackInst %13
//CHECK-NEXT:  %20 = CallInst %12, %10, %19
//CHECK-NEXT:  %21 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %20, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %22 = StoreStackInst %20, %16
//CHECK-NEXT:  %23 = LoadPropertyInst %20, "done" : string
//CHECK-NEXT:  %24 = CondBranchInst %23, %BB4, %BB5
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %25 = TryStartInst %BB6, %BB7
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %26 = LoadStackInst %16
//CHECK-NEXT:  %27 = LoadPropertyInst %26, "value" : string
//CHECK-NEXT:  %28 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %29 = ResumeGeneratorInst %15
//CHECK-NEXT:  %30 = StoreStackInst %29, %13
//CHECK-NEXT:  %31 = LoadStackInst %15
//CHECK-NEXT:  %32 = CondBranchInst %31, %BB9, %BB3
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %33 = CatchInst
//CHECK-NEXT:  %34 = CallBuiltinInst [HermesBuiltin.getMethod] : number, undefined : undefined, %10, "throw" : string
//CHECK-NEXT:  %35 = CompareBranchInst '===', %34, undefined : undefined, %BB10, %BB11
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %36 = CallBuiltinInst [HermesBuiltin.generatorSetDelegated] : number, undefined : undefined
//CHECK-NEXT:  %37 = SaveAndYieldInst %20, %BB8
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %38 = StoreStackInst %29, %13
//CHECK-NEXT:  %39 = BranchInst %BB12
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %40 = TryEndInst
//CHECK-NEXT:  %41 = CallBuiltinInst [HermesBuiltin.getMethod] : number, undefined : undefined, %10, "return" : string
//CHECK-NEXT:  %42 = CompareBranchInst '===', %41, undefined : undefined, %BB13, %BB14
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %43 = LoadStackInst %13
//CHECK-NEXT:  %44 = CallInst %41, %10, %43
//CHECK-NEXT:  %45 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %44, "iterator.return() did not return an object" : string
//CHECK-NEXT:  %46 = LoadPropertyInst %44, "done" : string
//CHECK-NEXT:  %47 = CondBranchInst %46, %BB15, %BB16
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %48 = ReturnInst %29
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %49 = LoadPropertyInst %44, "value" : string
//CHECK-NEXT:  %50 = ReturnInst %49
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %51 = CallBuiltinInst [HermesBuiltin.generatorSetDelegated] : number, undefined : undefined
//CHECK-NEXT:  %52 = SaveAndYieldInst %44, %BB8
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %53 = BranchInst %BB18
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %54 = TryEndInst
//CHECK-NEXT:  %55 = BranchInst %BB3
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %56 = CallInst %34, %10, %33
//CHECK-NEXT:  %57 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %56, "iterator.throw() did not return an object" : string
//CHECK-NEXT:  %58 = LoadPropertyInst %56, "done" : string
//CHECK-NEXT:  %59 = CondBranchInst %58, %BB19, %BB20
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %60 = CallBuiltinInst [HermesBuiltin.getMethod] : number, undefined : undefined, %10, "return" : string
//CHECK-NEXT:  %61 = CompareBranchInst '===', %60, undefined : undefined, %BB21, %BB22
//CHECK-NEXT:%BB19:
//CHECK-NEXT:  %62 = StoreStackInst %56, %16
//CHECK-NEXT:  %63 = BranchInst %BB4
//CHECK-NEXT:%BB20:
//CHECK-NEXT:  %64 = CallBuiltinInst [HermesBuiltin.generatorSetDelegated] : number, undefined : undefined
//CHECK-NEXT:  %65 = SaveAndYieldInst %56, %BB8
//CHECK-NEXT:%BB22:
//CHECK-NEXT:  %66 = CallInst %60, %10
//CHECK-NEXT:  %67 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %66, "iterator.return() did not return an object" : string
//CHECK-NEXT:  %68 = BranchInst %BB21
//CHECK-NEXT:%BB21:
//CHECK-NEXT:  %69 = CallBuiltinInst [HermesBuiltin.throwTypeError] : number, undefined : undefined, "yield* delegate must have a .throw() method" : string
//CHECK-NEXT:  %70 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

var destr = function*([x]) {
  yield x;
}

//CHECK-LABEL:function destr()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_destr()
//CHECK-NEXT:  %1 = LoadPropertyInst %0 : object, "next" : string
//CHECK-NEXT:  %2 = CallInst %1, %0 : object
//CHECK-NEXT:  %3 = ReturnInst %0 : object
//CHECK-NEXT:function_end

//CHECK-LABEL:function ?anon_0_destr(?anon_2_param)
//CHECK-NEXT:frame = [x]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StartGeneratorInst
//CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
//CHECK-NEXT:  %2 = ResumeGeneratorInst %1
//CHECK-NEXT:  %3 = LoadStackInst %1
//CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %5 = AllocStackInst $?anon_1_isReturn_entry
//CHECK-NEXT:  %6 = AllocStackInst $?anon_3_iter
//CHECK-NEXT:  %7 = AllocStackInst $?anon_4_sourceOrNext
//CHECK-NEXT:  %8 = StoreStackInst %?anon_2_param, %7
//CHECK-NEXT:  %9 = IteratorBeginInst %7
//CHECK-NEXT:  %10 = StoreStackInst %9, %6
//CHECK-NEXT:  %11 = AllocStackInst $?anon_5_iterDone
//CHECK-NEXT:  %12 = StoreStackInst undefined : undefined, %11
//CHECK-NEXT:  %13 = AllocStackInst $?anon_6_iterValue
//CHECK-NEXT:  %14 = StoreStackInst undefined : undefined, %13
//CHECK-NEXT:  %15 = BranchInst %BB3
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %16 = ReturnInst %2
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %17 = ResumeGeneratorInst %5
//CHECK-NEXT:  %18 = LoadStackInst %5
//CHECK-NEXT:  %19 = CondBranchInst %18, %BB5, %BB6
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %20 = IteratorNextInst %6, %7
//CHECK-NEXT:  %21 = LoadStackInst %6
//CHECK-NEXT:  %22 = BinaryOperatorInst '===', %21, undefined : undefined
//CHECK-NEXT:  %23 = StoreStackInst %22, %11
//CHECK-NEXT:  %24 = CondBranchInst %22, %BB7, %BB8
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %25 = StoreStackInst %20, %13
//CHECK-NEXT:  %26 = BranchInst %BB7
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %27 = LoadStackInst %13
//CHECK-NEXT:  %28 = StoreFrameInst %27, [x]
//CHECK-NEXT:  %29 = LoadStackInst %11
//CHECK-NEXT:  %30 = CondBranchInst %29, %BB9, %BB10
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %31 = IteratorCloseInst %6, false : boolean
//CHECK-NEXT:  %32 = BranchInst %BB9
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %33 = SaveAndYieldInst undefined : undefined, %BB4
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %34 = LoadFrameInst [x]
//CHECK-NEXT:  %35 = AllocStackInst $?anon_8_isReturn
//CHECK-NEXT:  %36 = SaveAndYieldInst %34, %BB11
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %37 = ReturnInst %17
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %38 = ResumeGeneratorInst %35
//CHECK-NEXT:  %39 = LoadStackInst %35
//CHECK-NEXT:  %40 = CondBranchInst %39, %BB12, %BB13
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %41 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %42 = ReturnInst %38
//CHECK-NEXT:function_end

var initializer = function*(x = foo()) {
  yield 1;
}

//CHECK-LABEL:function initializer()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_initializer()
//CHECK-NEXT:  %1 = LoadPropertyInst %0 : object, "next" : string
//CHECK-NEXT:  %2 = CallInst %1, %0 : object
//CHECK-NEXT:  %3 = ReturnInst %0 : object
//CHECK-NEXT:function_end

//CHECK-LABEL:function ?anon_0_initializer(x)
//CHECK-NEXT:frame = [x]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StartGeneratorInst
//CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
//CHECK-NEXT:  %2 = ResumeGeneratorInst %1
//CHECK-NEXT:  %3 = LoadStackInst %1
//CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %5 = AllocStackInst $?anon_1_isReturn_entry
//CHECK-NEXT:  %6 = BinaryOperatorInst '!==', %x, undefined : undefined
//CHECK-NEXT:  %7 = CondBranchInst %6, %BB3, %BB4
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %8 = ReturnInst %2
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %9 = ResumeGeneratorInst %5
//CHECK-NEXT:  %10 = LoadStackInst %5
//CHECK-NEXT:  %11 = CondBranchInst %10, %BB6, %BB7
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %12 = TryLoadGlobalPropertyInst globalObject : object, "foo" : string
//CHECK-NEXT:  %13 = CallInst %12, undefined : undefined
//CHECK-NEXT:  %14 = BranchInst %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %15 = PhiInst %x, %BB2, %13, %BB4
//CHECK-NEXT:  %16 = StoreFrameInst %15, [x]
//CHECK-NEXT:  %17 = SaveAndYieldInst undefined : undefined, %BB5
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %18 = AllocStackInst $?anon_2_isReturn
//CHECK-NEXT:  %19 = SaveAndYieldInst 1 : number, %BB8
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %20 = ReturnInst %9
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %21 = ResumeGeneratorInst %18
//CHECK-NEXT:  %22 = LoadStackInst %18
//CHECK-NEXT:  %23 = CondBranchInst %22, %BB9, %BB10
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %24 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %25 = ReturnInst %21
//CHECK-NEXT:function_end
