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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [simpleAsyncFE, simpleReturn, simpleAwait, nonSimpleArrayDestructuring]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %simpleReturn()
// CHECK-NEXT:  %1 = StorePropertyInst %0 : closure, globalObject : object, "simpleReturn" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %simpleAwait()
// CHECK-NEXT:  %3 = StorePropertyInst %2 : closure, globalObject : object, "simpleAwait" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %nonSimpleArrayDestructuring()
// CHECK-NEXT:  %5 = StorePropertyInst %4 : closure, globalObject : object, "nonSimpleArrayDestructuring" : string
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %7 = StoreStackInst undefined : undefined, %6
// CHECK-NEXT:  %8 = CreateFunctionInst %simpleAsyncFE()
// CHECK-NEXT:  %9 = StorePropertyInst %8 : closure, globalObject : object, "simpleAsyncFE" : string
// CHECK-NEXT:  %10 = LoadStackInst %6
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:function_end

// CHECK:function simpleReturn()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst
// CHECK-NEXT:  %1 = CreateFunctionInst %?anon_0_simpleReturn()
// CHECK-NEXT:  %2 = GetBuiltinClosureInst [HermesBuiltin.spawnAsync] : number
// CHECK-NEXT:  %3 = CallInst %2 : closure, undefined : undefined, %1 : closure, %this, %0 : object
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleReturn()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_?anon_0_simpleReturn()
// CHECK-NEXT:  %1 = ReturnInst %0 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_simpleReturn()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %2 = ResumeGeneratorInst %1
// CHECK-NEXT:  %3 = LoadStackInst %1
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = ReturnInst 1 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst %2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simpleAwait()
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %2 = CreateFunctionInst %?anon_0_simpleAwait()
// CHECK-NEXT:  %3 = GetBuiltinClosureInst [HermesBuiltin.spawnAsync] : number
// CHECK-NEXT:  %4 = CallInst %3 : closure, undefined : undefined, %2 : closure, %this, %0 : object
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleAwait()
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %1 = CreateGeneratorInst %?anon_0_?anon_0_simpleAwait()
// CHECK-NEXT:  %2 = ReturnInst %1 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_simpleAwait()
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
// CHECK-NEXT:  %7 = SaveAndYieldInst 2 : number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst %2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ResumeGeneratorInst %6
// CHECK-NEXT:  %10 = LoadStackInst %6
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = StoreFrameInst %9, [x]
// CHECK-NEXT:  %13 = LoadFrameInst [x]
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = ReturnInst %9
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %16 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function nonSimpleArrayDestructuring()
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %2 = CreateFunctionInst %?anon_0_nonSimpleArrayDestructuring()
// CHECK-NEXT:  %3 = GetBuiltinClosureInst [HermesBuiltin.spawnAsync] : number
// CHECK-NEXT:  %4 = CallInst %3 : closure, undefined : undefined, %2 : closure, %this, %0 : object
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_nonSimpleArrayDestructuring()
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %1 = CreateGeneratorInst %?anon_0_?anon_0_nonSimpleArrayDestructuring()
// CHECK-NEXT:  %2 = LoadPropertyInst %1 : object, "next" : string
// CHECK-NEXT:  %3 = CallInst %2, %1 : object
// CHECK-NEXT:  %4 = ReturnInst %1 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_nonSimpleArrayDestructuring(?anon_2_param)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %2 = ResumeGeneratorInst %1
// CHECK-NEXT:  %3 = LoadStackInst %1
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = AllocStackInst $?anon_1_isReturn_entry
// CHECK-NEXT:  %6 = AllocStackInst $?anon_3_iter
// CHECK-NEXT:  %7 = AllocStackInst $?anon_4_sourceOrNext
// CHECK-NEXT:  %8 = StoreStackInst %?anon_2_param, %7
// CHECK-NEXT:  %9 = IteratorBeginInst %7
// CHECK-NEXT:  %10 = StoreStackInst %9, %6
// CHECK-NEXT:  %11 = AllocStackInst $?anon_5_iterDone
// CHECK-NEXT:  %12 = StoreStackInst undefined : undefined, %11
// CHECK-NEXT:  %13 = AllocStackInst $?anon_6_iterValue
// CHECK-NEXT:  %14 = StoreStackInst undefined : undefined, %13
// CHECK-NEXT:  %15 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = ReturnInst %2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %17 = ResumeGeneratorInst %5
// CHECK-NEXT:  %18 = LoadStackInst %5
// CHECK-NEXT:  %19 = CondBranchInst %18, %BB5, %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %20 = IteratorNextInst %6, %7
// CHECK-NEXT:  %21 = LoadStackInst %6
// CHECK-NEXT:  %22 = BinaryOperatorInst '===', %21, undefined : undefined
// CHECK-NEXT:  %23 = StoreStackInst %22, %11
// CHECK-NEXT:  %24 = CondBranchInst %22, %BB7, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %25 = StoreStackInst %20, %13
// CHECK-NEXT:  %26 = BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %27 = LoadStackInst %13
// CHECK-NEXT:  %28 = StoreFrameInst %27, [x]
// CHECK-NEXT:  %29 = LoadStackInst %11
// CHECK-NEXT:  %30 = CondBranchInst %29, %BB9, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %31 = IteratorCloseInst %6, false : boolean
// CHECK-NEXT:  %32 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %33 = SaveAndYieldInst undefined : undefined, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %34 = LoadFrameInst [x]
// CHECK-NEXT:  %35 = AllocStackInst $?anon_8_isReturn
// CHECK-NEXT:  %36 = SaveAndYieldInst %34, %BB11
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %37 = ReturnInst %17
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %38 = ResumeGeneratorInst %35
// CHECK-NEXT:  %39 = LoadStackInst %35
// CHECK-NEXT:  %40 = CondBranchInst %39, %BB12, %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %41 = StoreFrameInst %38, [x]
// CHECK-NEXT:  %42 = LoadFrameInst [x]
// CHECK-NEXT:  %43 = ReturnInst %42
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %44 = ReturnInst %38
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %45 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simpleAsyncFE()
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %2 = CreateFunctionInst %?anon_0_simpleAsyncFE()
// CHECK-NEXT:  %3 = GetBuiltinClosureInst [HermesBuiltin.spawnAsync] : number
// CHECK-NEXT:  %4 = CallInst %3 : closure, undefined : undefined, %2 : closure, %this, %0 : object
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleAsyncFE()
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %1 = CreateGeneratorInst %?anon_0_?anon_0_simpleAsyncFE()
// CHECK-NEXT:  %2 = ReturnInst %1 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_simpleAsyncFE()
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
// CHECK-NEXT:  %7 = SaveAndYieldInst 2 : number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst %2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ResumeGeneratorInst %6
// CHECK-NEXT:  %10 = LoadStackInst %6
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = StoreFrameInst %9, [x]
// CHECK-NEXT:  %13 = LoadFrameInst [x]
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = ReturnInst %9
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %16 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
