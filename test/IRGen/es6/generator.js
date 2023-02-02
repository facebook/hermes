/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

function* simple() {
  yield 1;
}

function *useResult() {
  var x = yield 1;
}

function *loop(x) {
  var i = 0;
  while (true) {
    yield x[i++];
  }
}

// Test generation of function expressions.
var simple2 = function*() {
  yield 1;
}

var yieldStar = function*() {
  yield* foo();
}

var destr = function*([x]) {
  yield x;
}

var initializer = function*(x = foo()) {
  yield 1;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "simple" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "useResult" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "loop" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "simple2" : string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "yieldStar" : string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "destr" : string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "initializer" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %simple()
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7 : closure, globalObject : object, "simple" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %useResult()
// CHECK-NEXT:  %10 = StorePropertyLooseInst %9 : closure, globalObject : object, "useResult" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %loop()
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11 : closure, globalObject : object, "loop" : string
// CHECK-NEXT:  %13 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %14 = StoreStackInst undefined : undefined, %13
// CHECK-NEXT:  %15 = CreateFunctionInst %simple2()
// CHECK-NEXT:  %16 = StorePropertyLooseInst %15 : closure, globalObject : object, "simple2" : string
// CHECK-NEXT:  %17 = CreateFunctionInst %yieldStar()
// CHECK-NEXT:  %18 = StorePropertyLooseInst %17 : closure, globalObject : object, "yieldStar" : string
// CHECK-NEXT:  %19 = CreateFunctionInst %destr()
// CHECK-NEXT:  %20 = StorePropertyLooseInst %19 : closure, globalObject : object, "destr" : string
// CHECK-NEXT:  %21 = CreateFunctionInst %initializer()
// CHECK-NEXT:  %22 = StorePropertyLooseInst %21 : closure, globalObject : object, "initializer" : string
// CHECK-NEXT:  %23 = LoadStackInst %13
// CHECK-NEXT:  %24 = ReturnInst %23
// CHECK-NEXT:function_end

// CHECK:function simple()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_simple()
// CHECK-NEXT:  %1 = ReturnInst %0 : object
// CHECK-NEXT:function_end

// CHECK:function useResult()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_useResult()
// CHECK-NEXT:  %1 = ReturnInst %0 : object
// CHECK-NEXT:function_end

// CHECK:function loop()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_loop()
// CHECK-NEXT:  %1 = ReturnInst %0 : object
// CHECK-NEXT:function_end

// CHECK:function simple2()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_simple2()
// CHECK-NEXT:  %1 = ReturnInst %0 : object
// CHECK-NEXT:function_end

// CHECK:function yieldStar()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_yieldStar()
// CHECK-NEXT:  %1 = ReturnInst %0 : object
// CHECK-NEXT:function_end

// CHECK:function destr()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_destr()
// CHECK-NEXT:  %1 = LoadPropertyInst %0 : object, "next" : string
// CHECK-NEXT:  %2 = CallInst %1, %0 : object
// CHECK-NEXT:  %3 = ReturnInst %0 : object
// CHECK-NEXT:function_end

// CHECK:function initializer()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_initializer()
// CHECK-NEXT:  %1 = LoadPropertyInst %0 : object, "next" : string
// CHECK-NEXT:  %2 = CallInst %1, %0 : object
// CHECK-NEXT:  %3 = ReturnInst %0 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simple()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %2 = ResumeGeneratorInst %1
// CHECK-NEXT:  %3 = LoadStackInst %1
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = AllocStackInst $?anon_1_isReturn
// CHECK-NEXT:  %6 = SaveAndYieldInst 1 : number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst %2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = ResumeGeneratorInst %5
// CHECK-NEXT:  %9 = LoadStackInst %5
// CHECK-NEXT:  %10 = CondBranchInst %9, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = ReturnInst %8
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_useResult()
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %2 = ResumeGeneratorInst %1
// CHECK-NEXT:  %3 = LoadStackInst %1
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_isReturn
// CHECK-NEXT:  %7 = SaveAndYieldInst 1 : number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst %2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ResumeGeneratorInst %6
// CHECK-NEXT:  %10 = LoadStackInst %6
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = StoreFrameInst %9, [x]
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = ReturnInst %9
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_loop(x)
// CHECK-NEXT:frame = [x, i]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %2 = ResumeGeneratorInst %1
// CHECK-NEXT:  %3 = LoadStackInst %1
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = LoadParamInst %x
// CHECK-NEXT:  %6 = StoreFrameInst %5, [x]
// CHECK-NEXT:  %7 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %8 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %9 = CondBranchInst true : boolean, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst %2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadFrameInst [x]
// CHECK-NEXT:  %12 = LoadFrameInst [i]
// CHECK-NEXT:  %13 = AsNumericInst %12
// CHECK-NEXT:  %14 = UnaryOperatorInst '++', %13 : number|bigint
// CHECK-NEXT:  %15 = StoreFrameInst %14, [i]
// CHECK-NEXT:  %16 = LoadPropertyInst %11, %13 : number|bigint
// CHECK-NEXT:  %17 = AllocStackInst $?anon_1_isReturn
// CHECK-NEXT:  %18 = SaveAndYieldInst %16, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %20 = CondBranchInst true : boolean, %BB3, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %21 = ResumeGeneratorInst %17
// CHECK-NEXT:  %22 = LoadStackInst %17
// CHECK-NEXT:  %23 = CondBranchInst %22, %BB7, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %24 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %25 = ReturnInst %21
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simple2()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %2 = ResumeGeneratorInst %1
// CHECK-NEXT:  %3 = LoadStackInst %1
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = AllocStackInst $?anon_1_isReturn
// CHECK-NEXT:  %6 = SaveAndYieldInst 1 : number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst %2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = ResumeGeneratorInst %5
// CHECK-NEXT:  %9 = LoadStackInst %5
// CHECK-NEXT:  %10 = CondBranchInst %9, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = ReturnInst %8
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_yieldStar()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %2 = ResumeGeneratorInst %1
// CHECK-NEXT:  %3 = LoadStackInst %1
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %6 = CallInst %5, undefined : undefined
// CHECK-NEXT:  %7 = GetBuiltinClosureInst [globalThis.Symbol] : number
// CHECK-NEXT:  %8 = LoadPropertyInst %7 : closure, "iterator" : string
// CHECK-NEXT:  %9 = LoadPropertyInst %6, %8
// CHECK-NEXT:  %10 = CallInst %9, %6
// CHECK-NEXT:  %11 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %10, "iterator is not an object" : string
// CHECK-NEXT:  %12 = LoadPropertyInst %10, "next" : string
// CHECK-NEXT:  %13 = AllocStackInst $?anon_1_received
// CHECK-NEXT:  %14 = StoreStackInst undefined : undefined, %13
// CHECK-NEXT:  %15 = AllocStackInst $?anon_2_isReturn
// CHECK-NEXT:  %16 = AllocStackInst $?anon_3_result
// CHECK-NEXT:  %17 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %18 = ReturnInst %2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %19 = LoadStackInst %13
// CHECK-NEXT:  %20 = CallInst %12, %10, %19
// CHECK-NEXT:  %21 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %20, "iterator.next() did not return an object" : string
// CHECK-NEXT:  %22 = StoreStackInst %20, %16
// CHECK-NEXT:  %23 = LoadPropertyInst %20, "done" : string
// CHECK-NEXT:  %24 = CondBranchInst %23, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %25 = TryStartInst %BB6, %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %26 = LoadStackInst %16
// CHECK-NEXT:  %27 = LoadPropertyInst %26, "value" : string
// CHECK-NEXT:  %28 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %29 = ResumeGeneratorInst %15
// CHECK-NEXT:  %30 = StoreStackInst %29, %13
// CHECK-NEXT:  %31 = LoadStackInst %15
// CHECK-NEXT:  %32 = CondBranchInst %31, %BB9, %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %33 = CatchInst
// CHECK-NEXT:  %34 = CallBuiltinInst [HermesBuiltin.getMethod] : number, undefined : undefined, %10, "throw" : string
// CHECK-NEXT:  %35 = CompareBranchInst '===', %34, undefined : undefined, %BB10, %BB11
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %36 = CallBuiltinInst [HermesBuiltin.generatorSetDelegated] : number, undefined : undefined
// CHECK-NEXT:  %37 = SaveAndYieldInst %20, %BB8
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %38 = StoreStackInst %29, %13
// CHECK-NEXT:  %39 = BranchInst %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %40 = TryEndInst
// CHECK-NEXT:  %41 = CallBuiltinInst [HermesBuiltin.getMethod] : number, undefined : undefined, %10, "return" : string
// CHECK-NEXT:  %42 = CompareBranchInst '===', %41, undefined : undefined, %BB13, %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %43 = LoadStackInst %13
// CHECK-NEXT:  %44 = CallInst %41, %10, %43
// CHECK-NEXT:  %45 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %44, "iterator.return() did not return an object" : string
// CHECK-NEXT:  %46 = LoadPropertyInst %44, "done" : string
// CHECK-NEXT:  %47 = CondBranchInst %46, %BB15, %BB16
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %48 = ReturnInst %29
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %49 = LoadPropertyInst %44, "value" : string
// CHECK-NEXT:  %50 = ReturnInst %49
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %51 = CallBuiltinInst [HermesBuiltin.generatorSetDelegated] : number, undefined : undefined
// CHECK-NEXT:  %52 = SaveAndYieldInst %44, %BB8
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %53 = BranchInst %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %54 = TryEndInst
// CHECK-NEXT:  %55 = BranchInst %BB3
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %56 = CallInst %34, %10, %33
// CHECK-NEXT:  %57 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %56, "iterator.throw() did not return an object" : string
// CHECK-NEXT:  %58 = LoadPropertyInst %56, "done" : string
// CHECK-NEXT:  %59 = CondBranchInst %58, %BB19, %BB20
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %60 = CallBuiltinInst [HermesBuiltin.getMethod] : number, undefined : undefined, %10, "return" : string
// CHECK-NEXT:  %61 = CompareBranchInst '===', %60, undefined : undefined, %BB21, %BB22
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %62 = StoreStackInst %56, %16
// CHECK-NEXT:  %63 = BranchInst %BB4
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %64 = CallBuiltinInst [HermesBuiltin.generatorSetDelegated] : number, undefined : undefined
// CHECK-NEXT:  %65 = SaveAndYieldInst %56, %BB8
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %66 = CallInst %60, %10
// CHECK-NEXT:  %67 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %66, "iterator.return() did not return an object" : string
// CHECK-NEXT:  %68 = BranchInst %BB21
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %69 = CallBuiltinInst [HermesBuiltin.throwTypeError] : number, undefined : undefined, "yield* delegate must have a .throw() method" : string
// CHECK-NEXT:  %70 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_destr(?anon_2_param)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %2 = ResumeGeneratorInst %1
// CHECK-NEXT:  %3 = LoadStackInst %1
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = AllocStackInst $?anon_1_isReturn_entry
// CHECK-NEXT:  %6 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %7 = LoadParamInst %?anon_2_param
// CHECK-NEXT:  %8 = AllocStackInst $?anon_3_iter
// CHECK-NEXT:  %9 = AllocStackInst $?anon_4_sourceOrNext
// CHECK-NEXT:  %10 = StoreStackInst %7, %9
// CHECK-NEXT:  %11 = IteratorBeginInst %9
// CHECK-NEXT:  %12 = StoreStackInst %11, %8
// CHECK-NEXT:  %13 = AllocStackInst $?anon_5_iterDone
// CHECK-NEXT:  %14 = StoreStackInst undefined : undefined, %13
// CHECK-NEXT:  %15 = AllocStackInst $?anon_6_iterValue
// CHECK-NEXT:  %16 = StoreStackInst undefined : undefined, %15
// CHECK-NEXT:  %17 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %18 = ReturnInst %2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = ResumeGeneratorInst %5
// CHECK-NEXT:  %20 = LoadStackInst %5
// CHECK-NEXT:  %21 = CondBranchInst %20, %BB5, %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %22 = IteratorNextInst %8, %9
// CHECK-NEXT:  %23 = LoadStackInst %8
// CHECK-NEXT:  %24 = BinaryOperatorInst '===', %23, undefined : undefined
// CHECK-NEXT:  %25 = StoreStackInst %24, %13
// CHECK-NEXT:  %26 = CondBranchInst %24, %BB7, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %27 = StoreStackInst %22, %15
// CHECK-NEXT:  %28 = BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %29 = LoadStackInst %15
// CHECK-NEXT:  %30 = StoreFrameInst %29, [x]
// CHECK-NEXT:  %31 = LoadStackInst %13
// CHECK-NEXT:  %32 = CondBranchInst %31, %BB9, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %33 = IteratorCloseInst %8, false : boolean
// CHECK-NEXT:  %34 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %35 = SaveAndYieldInst undefined : undefined, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %36 = LoadFrameInst [x]
// CHECK-NEXT:  %37 = AllocStackInst $?anon_8_isReturn
// CHECK-NEXT:  %38 = SaveAndYieldInst %36, %BB11
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %39 = ReturnInst %19
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %40 = ResumeGeneratorInst %37
// CHECK-NEXT:  %41 = LoadStackInst %37
// CHECK-NEXT:  %42 = CondBranchInst %41, %BB12, %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %43 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %44 = ReturnInst %40
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_initializer(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %2 = ResumeGeneratorInst %1
// CHECK-NEXT:  %3 = LoadStackInst %1
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = AllocStackInst $?anon_1_isReturn_entry
// CHECK-NEXT:  %6 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %7 = LoadParamInst %x
// CHECK-NEXT:  %8 = BinaryOperatorInst '!==', %7, undefined : undefined
// CHECK-NEXT:  %9 = CondBranchInst %8, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst %2
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = ResumeGeneratorInst %5
// CHECK-NEXT:  %12 = LoadStackInst %5
// CHECK-NEXT:  %13 = CondBranchInst %12, %BB6, %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %15 = CallInst %14, undefined : undefined
// CHECK-NEXT:  %16 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %17 = PhiInst %7, %BB2, %15, %BB4
// CHECK-NEXT:  %18 = StoreFrameInst %17, [x]
// CHECK-NEXT:  %19 = SaveAndYieldInst undefined : undefined, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %20 = AllocStackInst $?anon_2_isReturn
// CHECK-NEXT:  %21 = SaveAndYieldInst 1 : number, %BB8
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %22 = ReturnInst %11
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %23 = ResumeGeneratorInst %20
// CHECK-NEXT:  %24 = LoadStackInst %20
// CHECK-NEXT:  %25 = CondBranchInst %24, %BB9, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %26 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %27 = ReturnInst %23
// CHECK-NEXT:function_end
