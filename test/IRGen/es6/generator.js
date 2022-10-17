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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [simple2, yieldStar, destr, initializer, simple, useResult, loop]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %simple#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "simple" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %useResult#0#1()#4, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "useResult" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %loop#0#1()#6, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "loop" : string
// CHECK-NEXT:  %7 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %8 = StoreStackInst undefined : undefined, %7
// CHECK-NEXT:  %9 = CreateFunctionInst %simple2#0#1()#8, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "simple2" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %yieldStar#0#1()#10, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "yieldStar" : string
// CHECK-NEXT:  %13 = CreateFunctionInst %destr#0#1()#12, %0
// CHECK-NEXT:  %14 = StorePropertyInst %13 : closure, globalObject : object, "destr" : string
// CHECK-NEXT:  %15 = CreateFunctionInst %initializer#0#1()#14, %0
// CHECK-NEXT:  %16 = StorePropertyInst %15 : closure, globalObject : object, "initializer" : string
// CHECK-NEXT:  %17 = LoadStackInst %7
// CHECK-NEXT:  %18 = ReturnInst %17
// CHECK-NEXT:function_end

// CHECK:function simple#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple#0#1()#2}
// CHECK-NEXT:  %1 = CreateGeneratorInst %?anon_0_simple#1#2()#3, %0
// CHECK-NEXT:  %2 = ReturnInst %1 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simple#1#2()#3
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{?anon_0_simple#1#2()#3}
// CHECK-NEXT:  %1 = StartGeneratorInst
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %3 = ResumeGeneratorInst %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_isReturn
// CHECK-NEXT:  %7 = SaveAndYieldInst 1 : number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst %3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ResumeGeneratorInst %6
// CHECK-NEXT:  %10 = LoadStackInst %6
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = ReturnInst %9
// CHECK-NEXT:function_end

// CHECK:function useResult#0#1()#4
// CHECK-NEXT:frame = [x#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{useResult#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [x#4], %0
// CHECK-NEXT:  %2 = CreateGeneratorInst %?anon_0_useResult#1#4()#5, %0
// CHECK-NEXT:  %3 = ReturnInst %2 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_useResult#1#4()#5
// CHECK-NEXT:frame = [x#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{?anon_0_useResult#1#4()#5}
// CHECK-NEXT:  %1 = StartGeneratorInst
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %3 = ResumeGeneratorInst %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = StoreFrameInst undefined : undefined, [x#5], %0
// CHECK-NEXT:  %7 = AllocStackInst $?anon_1_isReturn
// CHECK-NEXT:  %8 = SaveAndYieldInst 1 : number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst %3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = ResumeGeneratorInst %7
// CHECK-NEXT:  %11 = LoadStackInst %7
// CHECK-NEXT:  %12 = CondBranchInst %11, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %13 = StoreFrameInst %10, [x#5], %0
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = ReturnInst %10
// CHECK-NEXT:function_end

// CHECK:function loop#0#1()#6
// CHECK-NEXT:frame = [i#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{loop#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [i#6], %0
// CHECK-NEXT:  %2 = CreateGeneratorInst %?anon_0_loop#1#6()#7, %0
// CHECK-NEXT:  %3 = ReturnInst %2 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_loop#1#6(x)#7
// CHECK-NEXT:frame = [x#7, i#7]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{?anon_0_loop#1#6()#7}
// CHECK-NEXT:  %1 = StartGeneratorInst
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %3 = ResumeGeneratorInst %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = StoreFrameInst %x, [x#7], %0
// CHECK-NEXT:  %7 = StoreFrameInst undefined : undefined, [i#7], %0
// CHECK-NEXT:  %8 = StoreFrameInst 0 : number, [i#7], %0
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst %3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = LoadFrameInst [x#7], %0
// CHECK-NEXT:  %12 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %13 = AsNumericInst %12
// CHECK-NEXT:  %14 = UnaryOperatorInst '++', %13 : number|bigint
// CHECK-NEXT:  %15 = StoreFrameInst %14, [i#7], %0
// CHECK-NEXT:  %16 = LoadPropertyInst %11, %13 : number|bigint
// CHECK-NEXT:  %17 = AllocStackInst $?anon_1_isReturn
// CHECK-NEXT:  %18 = SaveAndYieldInst %16, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %19 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %20 = CondBranchInst true : boolean, %BB4, %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %21 = CondBranchInst true : boolean, %BB4, %BB6
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %22 = BranchInst %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %23 = ResumeGeneratorInst %17
// CHECK-NEXT:  %24 = LoadStackInst %17
// CHECK-NEXT:  %25 = CondBranchInst %24, %BB9, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %26 = BranchInst %BB8
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %27 = ReturnInst %23
// CHECK-NEXT:function_end

// CHECK:function simple2#0#1()#8
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple2#0#1()#8}
// CHECK-NEXT:  %1 = CreateGeneratorInst %?anon_0_simple2#1#8()#9, %0
// CHECK-NEXT:  %2 = ReturnInst %1 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simple2#1#8()#9
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{?anon_0_simple2#1#8()#9}
// CHECK-NEXT:  %1 = StartGeneratorInst
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %3 = ResumeGeneratorInst %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_isReturn
// CHECK-NEXT:  %7 = SaveAndYieldInst 1 : number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst %3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ResumeGeneratorInst %6
// CHECK-NEXT:  %10 = LoadStackInst %6
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = ReturnInst %9
// CHECK-NEXT:function_end

// CHECK:function yieldStar#0#1()#10
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{yieldStar#0#1()#10}
// CHECK-NEXT:  %1 = CreateGeneratorInst %?anon_0_yieldStar#1#10()#11, %0
// CHECK-NEXT:  %2 = ReturnInst %1 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_yieldStar#1#10()#11
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{?anon_0_yieldStar#1#10()#11}
// CHECK-NEXT:  %1 = StartGeneratorInst
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %3 = ResumeGeneratorInst %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %7 = CallInst %6, undefined : undefined
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
// CHECK-NEXT:  %9 = LoadPropertyInst %8, "iterator" : string
// CHECK-NEXT:  %10 = LoadPropertyInst %7, %9
// CHECK-NEXT:  %11 = CallInst %10, %7
// CHECK-NEXT:  %12 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %11, "iterator is not an object" : string
// CHECK-NEXT:  %13 = LoadPropertyInst %11, "next" : string
// CHECK-NEXT:  %14 = AllocStackInst $?anon_1_received
// CHECK-NEXT:  %15 = StoreStackInst undefined : undefined, %14
// CHECK-NEXT:  %16 = AllocStackInst $?anon_2_isReturn
// CHECK-NEXT:  %17 = AllocStackInst $?anon_3_result
// CHECK-NEXT:  %18 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %19 = ReturnInst %3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %20 = LoadStackInst %14
// CHECK-NEXT:  %21 = CallInst %13, %11, %20
// CHECK-NEXT:  %22 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %21, "iterator.next() did not return an object" : string
// CHECK-NEXT:  %23 = StoreStackInst %21, %17
// CHECK-NEXT:  %24 = LoadPropertyInst %21, "done" : string
// CHECK-NEXT:  %25 = CondBranchInst %24, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %26 = TryStartInst %BB6, %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %27 = LoadStackInst %17
// CHECK-NEXT:  %28 = LoadPropertyInst %27, "value" : string
// CHECK-NEXT:  %29 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %30 = ResumeGeneratorInst %16
// CHECK-NEXT:  %31 = StoreStackInst %30, %14
// CHECK-NEXT:  %32 = LoadStackInst %16
// CHECK-NEXT:  %33 = CondBranchInst %32, %BB9, %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %34 = CatchInst
// CHECK-NEXT:  %35 = CallBuiltinInst [HermesBuiltin.getMethod] : number, undefined : undefined, %11, "throw" : string
// CHECK-NEXT:  %36 = CompareBranchInst '===', %35, undefined : undefined, %BB10, %BB11
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %37 = CallBuiltinInst [HermesBuiltin.generatorSetDelegated] : number, undefined : undefined
// CHECK-NEXT:  %38 = SaveAndYieldInst %21, %BB8
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %39 = StoreStackInst %30, %14
// CHECK-NEXT:  %40 = BranchInst %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %41 = TryEndInst
// CHECK-NEXT:  %42 = CallBuiltinInst [HermesBuiltin.getMethod] : number, undefined : undefined, %11, "return" : string
// CHECK-NEXT:  %43 = CompareBranchInst '===', %42, undefined : undefined, %BB13, %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %44 = LoadStackInst %14
// CHECK-NEXT:  %45 = CallInst %42, %11, %44
// CHECK-NEXT:  %46 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %45, "iterator.return() did not return an object" : string
// CHECK-NEXT:  %47 = LoadPropertyInst %45, "done" : string
// CHECK-NEXT:  %48 = CondBranchInst %47, %BB15, %BB16
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %49 = ReturnInst %30
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %50 = LoadPropertyInst %45, "value" : string
// CHECK-NEXT:  %51 = ReturnInst %50
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %52 = CallBuiltinInst [HermesBuiltin.generatorSetDelegated] : number, undefined : undefined
// CHECK-NEXT:  %53 = SaveAndYieldInst %45, %BB8
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %54 = BranchInst %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %55 = TryEndInst
// CHECK-NEXT:  %56 = BranchInst %BB3
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %57 = CallInst %35, %11, %34
// CHECK-NEXT:  %58 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %57, "iterator.throw() did not return an object" : string
// CHECK-NEXT:  %59 = LoadPropertyInst %57, "done" : string
// CHECK-NEXT:  %60 = CondBranchInst %59, %BB19, %BB20
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %61 = CallBuiltinInst [HermesBuiltin.getMethod] : number, undefined : undefined, %11, "return" : string
// CHECK-NEXT:  %62 = CompareBranchInst '===', %61, undefined : undefined, %BB21, %BB22
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %63 = StoreStackInst %57, %17
// CHECK-NEXT:  %64 = BranchInst %BB4
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %65 = CallBuiltinInst [HermesBuiltin.generatorSetDelegated] : number, undefined : undefined
// CHECK-NEXT:  %66 = SaveAndYieldInst %57, %BB8
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %67 = CallInst %61, %11
// CHECK-NEXT:  %68 = CallBuiltinInst [HermesBuiltin.ensureObject] : number, undefined : undefined, %67, "iterator.return() did not return an object" : string
// CHECK-NEXT:  %69 = BranchInst %BB21
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %70 = CallBuiltinInst [HermesBuiltin.throwTypeError] : number, undefined : undefined, "yield* delegate must have a .throw() method" : string
// CHECK-NEXT:  %71 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function destr#0#1()#12
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{destr#0#1()#12}
// CHECK-NEXT:  %1 = CreateGeneratorInst %?anon_0_destr#1#12()#13, %0
// CHECK-NEXT:  %2 = LoadPropertyInst %1 : object, "next" : string
// CHECK-NEXT:  %3 = CallInst %2, %1 : object
// CHECK-NEXT:  %4 = ReturnInst %1 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_destr#1#12(?anon_2_param)#13
// CHECK-NEXT:frame = [x#13]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{?anon_0_destr#1#12()#13}
// CHECK-NEXT:  %1 = StartGeneratorInst
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %3 = ResumeGeneratorInst %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_isReturn_entry
// CHECK-NEXT:  %7 = AllocStackInst $?anon_3_iter
// CHECK-NEXT:  %8 = AllocStackInst $?anon_4_sourceOrNext
// CHECK-NEXT:  %9 = StoreStackInst %?anon_2_param, %8
// CHECK-NEXT:  %10 = IteratorBeginInst %8
// CHECK-NEXT:  %11 = StoreStackInst %10, %7
// CHECK-NEXT:  %12 = AllocStackInst $?anon_5_iterDone
// CHECK-NEXT:  %13 = StoreStackInst undefined : undefined, %12
// CHECK-NEXT:  %14 = AllocStackInst $?anon_6_iterValue
// CHECK-NEXT:  %15 = StoreStackInst undefined : undefined, %14
// CHECK-NEXT:  %16 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %17 = ReturnInst %3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %18 = ResumeGeneratorInst %6
// CHECK-NEXT:  %19 = LoadStackInst %6
// CHECK-NEXT:  %20 = CondBranchInst %19, %BB5, %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %21 = IteratorNextInst %7, %8
// CHECK-NEXT:  %22 = LoadStackInst %7
// CHECK-NEXT:  %23 = BinaryOperatorInst '===', %22, undefined : undefined
// CHECK-NEXT:  %24 = StoreStackInst %23, %12
// CHECK-NEXT:  %25 = CondBranchInst %23, %BB7, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %26 = StoreStackInst %21, %14
// CHECK-NEXT:  %27 = BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %28 = LoadStackInst %14
// CHECK-NEXT:  %29 = StoreFrameInst %28, [x#13], %0
// CHECK-NEXT:  %30 = LoadStackInst %12
// CHECK-NEXT:  %31 = CondBranchInst %30, %BB9, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %32 = IteratorCloseInst %7, false : boolean
// CHECK-NEXT:  %33 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %34 = SaveAndYieldInst undefined : undefined, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %35 = LoadFrameInst [x#13], %0
// CHECK-NEXT:  %36 = AllocStackInst $?anon_8_isReturn
// CHECK-NEXT:  %37 = SaveAndYieldInst %35, %BB11
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %38 = ReturnInst %18
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %39 = ResumeGeneratorInst %36
// CHECK-NEXT:  %40 = LoadStackInst %36
// CHECK-NEXT:  %41 = CondBranchInst %40, %BB12, %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %42 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %43 = ReturnInst %39
// CHECK-NEXT:function_end

// CHECK:function initializer#0#1()#14
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{initializer#0#1()#14}
// CHECK-NEXT:  %1 = CreateGeneratorInst %?anon_0_initializer#1#14()#15, %0
// CHECK-NEXT:  %2 = LoadPropertyInst %1 : object, "next" : string
// CHECK-NEXT:  %3 = CallInst %2, %1 : object
// CHECK-NEXT:  %4 = ReturnInst %1 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_initializer#1#14(x)#15
// CHECK-NEXT:frame = [x#15]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{?anon_0_initializer#1#14()#15}
// CHECK-NEXT:  %1 = StartGeneratorInst
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %3 = ResumeGeneratorInst %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = AllocStackInst $?anon_1_isReturn_entry
// CHECK-NEXT:  %7 = BinaryOperatorInst '!==', %x, undefined : undefined
// CHECK-NEXT:  %8 = CondBranchInst %7, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst %3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %10 = ResumeGeneratorInst %6
// CHECK-NEXT:  %11 = LoadStackInst %6
// CHECK-NEXT:  %12 = CondBranchInst %11, %BB6, %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = TryLoadGlobalPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %14 = CallInst %13, undefined : undefined
// CHECK-NEXT:  %15 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = PhiInst %x, %BB2, %14, %BB4
// CHECK-NEXT:  %17 = StoreFrameInst %16, [x#15], %0
// CHECK-NEXT:  %18 = SaveAndYieldInst undefined : undefined, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %19 = AllocStackInst $?anon_2_isReturn
// CHECK-NEXT:  %20 = SaveAndYieldInst 1 : number, %BB8
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %21 = ReturnInst %10
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %22 = ResumeGeneratorInst %19
// CHECK-NEXT:  %23 = LoadStackInst %19
// CHECK-NEXT:  %24 = CondBranchInst %23, %BB9, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %25 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %26 = ReturnInst %22
// CHECK-NEXT:function_end
