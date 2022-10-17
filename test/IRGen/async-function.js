/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

// Each async function will generate an inner generator (and its "generator
// inner function"). Every `await` experession inside the async function body
// will be rewritten as a `yield` expression of its inner generator.

async function simpleReturn() {
  return 1;
}

async function simpleAwait() {
  var x = await 2;
  return x;
}

// test that parameters are only destructured once.
async function nonSimpleArrayDestructuring([x]) {
  var x = await x;
  return x;
}

var simpleAsyncFE = async function () {
  var x = await 2;
  return x;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [simpleAsyncFE, simpleReturn, simpleAwait, nonSimpleArrayDestructuring]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %simpleReturn#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "simpleReturn" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %simpleAwait#0#1()#5, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "simpleAwait" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %nonSimpleArrayDestructuring#0#1()#8, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "nonSimpleArrayDestructuring" : string
// CHECK-NEXT:  %7 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %8 = StoreStackInst undefined : undefined, %7
// CHECK-NEXT:  %9 = CreateFunctionInst %simpleAsyncFE#0#1()#11, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "simpleAsyncFE" : string
// CHECK-NEXT:  %11 = LoadStackInst %7
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function simpleReturn#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simpleReturn#0#1()#2}
// CHECK-NEXT:  %1 = CreateArgumentsInst
// CHECK-NEXT:  %2 = CreateFunctionInst %?anon_0_simpleReturn#1#2()#3, %0
// CHECK-NEXT:  %3 = GetBuiltinClosureInst [HermesBuiltin.spawnAsync] : number
// CHECK-NEXT:  %4 = CallInst %3 : closure, undefined : undefined, %2 : closure, %this, %1 : object
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleReturn#1#2()#3
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{?anon_0_simpleReturn#1#2()#3}
// CHECK-NEXT:  %1 = CreateGeneratorInst %?anon_0_?anon_0_simpleReturn#2#3()#4, %0
// CHECK-NEXT:  %2 = ReturnInst %1 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_simpleReturn#2#3()#4
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{?anon_0_?anon_0_simpleReturn#2#3()#4}
// CHECK-NEXT:  %1 = StartGeneratorInst
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %3 = ResumeGeneratorInst %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = ReturnInst 1 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst %3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simpleAwait#0#1()#5
// CHECK-NEXT:frame = [x#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simpleAwait#0#1()#5}
// CHECK-NEXT:  %1 = CreateArgumentsInst
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [x#5], %0
// CHECK-NEXT:  %3 = CreateFunctionInst %?anon_0_simpleAwait#1#5()#6, %0
// CHECK-NEXT:  %4 = GetBuiltinClosureInst [HermesBuiltin.spawnAsync] : number
// CHECK-NEXT:  %5 = CallInst %4 : closure, undefined : undefined, %3 : closure, %this, %1 : object
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleAwait#1#5()#6
// CHECK-NEXT:frame = [x#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{?anon_0_simpleAwait#1#5()#6}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [x#6], %0
// CHECK-NEXT:  %2 = CreateGeneratorInst %?anon_0_?anon_0_simpleAwait#5#6()#7, %0
// CHECK-NEXT:  %3 = ReturnInst %2 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_simpleAwait#5#6()#7
// CHECK-NEXT:frame = [x#7]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{?anon_0_?anon_0_simpleAwait#5#6()#7}
// CHECK-NEXT:  %1 = StartGeneratorInst
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %3 = ResumeGeneratorInst %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = StoreFrameInst undefined : undefined, [x#7], %0
// CHECK-NEXT:  %7 = AllocStackInst $?anon_1_isReturn
// CHECK-NEXT:  %8 = SaveAndYieldInst 2 : number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst %3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = ResumeGeneratorInst %7
// CHECK-NEXT:  %11 = LoadStackInst %7
// CHECK-NEXT:  %12 = CondBranchInst %11, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %13 = StoreFrameInst %10, [x#7], %0
// CHECK-NEXT:  %14 = LoadFrameInst [x#7], %0
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %16 = ReturnInst %10
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %17 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function nonSimpleArrayDestructuring#0#1()#8
// CHECK-NEXT:frame = [x#8]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{nonSimpleArrayDestructuring#0#1()#8}
// CHECK-NEXT:  %1 = CreateArgumentsInst
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [x#8], %0
// CHECK-NEXT:  %3 = CreateFunctionInst %?anon_0_nonSimpleArrayDestructuring#1#8()#9, %0
// CHECK-NEXT:  %4 = GetBuiltinClosureInst [HermesBuiltin.spawnAsync] : number
// CHECK-NEXT:  %5 = CallInst %4 : closure, undefined : undefined, %3 : closure, %this, %1 : object
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_nonSimpleArrayDestructuring#1#8()#9
// CHECK-NEXT:frame = [x#9]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{?anon_0_nonSimpleArrayDestructuring#1#8()#9}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [x#9], %0
// CHECK-NEXT:  %2 = CreateGeneratorInst %?anon_0_?anon_0_nonSimpleArrayDestructuring#8#9()#10, %0
// CHECK-NEXT:  %3 = LoadPropertyInst %2 : object, "next" : string
// CHECK-NEXT:  %4 = CallInst %3, %2 : object
// CHECK-NEXT:  %5 = ReturnInst %2 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_nonSimpleArrayDestructuring#8#9(?anon_2_param)#10
// CHECK-NEXT:frame = [x#10]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{?anon_0_?anon_0_nonSimpleArrayDestructuring#8#9()#10}
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
// CHECK-NEXT:  %29 = StoreFrameInst %28, [x#10], %0
// CHECK-NEXT:  %30 = LoadStackInst %12
// CHECK-NEXT:  %31 = CondBranchInst %30, %BB9, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %32 = IteratorCloseInst %7, false : boolean
// CHECK-NEXT:  %33 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %34 = SaveAndYieldInst undefined : undefined, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %35 = LoadFrameInst [x#10], %0
// CHECK-NEXT:  %36 = AllocStackInst $?anon_8_isReturn
// CHECK-NEXT:  %37 = SaveAndYieldInst %35, %BB11
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %38 = ReturnInst %18
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %39 = ResumeGeneratorInst %36
// CHECK-NEXT:  %40 = LoadStackInst %36
// CHECK-NEXT:  %41 = CondBranchInst %40, %BB12, %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %42 = StoreFrameInst %39, [x#10], %0
// CHECK-NEXT:  %43 = LoadFrameInst [x#10], %0
// CHECK-NEXT:  %44 = ReturnInst %43
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %45 = ReturnInst %39
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %46 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simpleAsyncFE#0#1()#11
// CHECK-NEXT:frame = [x#11]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simpleAsyncFE#0#1()#11}
// CHECK-NEXT:  %1 = CreateArgumentsInst
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [x#11], %0
// CHECK-NEXT:  %3 = CreateFunctionInst %?anon_0_simpleAsyncFE#1#11()#12, %0
// CHECK-NEXT:  %4 = GetBuiltinClosureInst [HermesBuiltin.spawnAsync] : number
// CHECK-NEXT:  %5 = CallInst %4 : closure, undefined : undefined, %3 : closure, %this, %1 : object
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleAsyncFE#1#11()#12
// CHECK-NEXT:frame = [x#12]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{?anon_0_simpleAsyncFE#1#11()#12}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [x#12], %0
// CHECK-NEXT:  %2 = CreateGeneratorInst %?anon_0_?anon_0_simpleAsyncFE#11#12()#13, %0
// CHECK-NEXT:  %3 = ReturnInst %2 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_simpleAsyncFE#11#12()#13
// CHECK-NEXT:frame = [x#13]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{?anon_0_?anon_0_simpleAsyncFE#11#12()#13}
// CHECK-NEXT:  %1 = StartGeneratorInst
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %3 = ResumeGeneratorInst %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = StoreFrameInst undefined : undefined, [x#13], %0
// CHECK-NEXT:  %7 = AllocStackInst $?anon_1_isReturn
// CHECK-NEXT:  %8 = SaveAndYieldInst 2 : number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst %3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = ResumeGeneratorInst %7
// CHECK-NEXT:  %11 = LoadStackInst %7
// CHECK-NEXT:  %12 = CondBranchInst %11, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %13 = StoreFrameInst %10, [x#13], %0
// CHECK-NEXT:  %14 = LoadFrameInst [x#13], %0
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %16 = ReturnInst %10
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %17 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
