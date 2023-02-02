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
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "simpleReturn" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "simpleAwait" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "nonSimpleArrayDestructuring" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "simpleAsyncFE" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %simpleReturn()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "simpleReturn" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %simpleAwait()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "simpleAwait" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %nonSimpleArrayDestructuring()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "nonSimpleArrayDestructuring" : string
// CHECK-NEXT:  %10 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %11 = StoreStackInst undefined : undefined, %10
// CHECK-NEXT:  %12 = CreateFunctionInst %simpleAsyncFE()
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12 : closure, globalObject : object, "simpleAsyncFE" : string
// CHECK-NEXT:  %14 = LoadStackInst %10
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:function_end

// CHECK:function simpleReturn()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst
// CHECK-NEXT:  %1 = LoadParamInst %this
// CHECK-NEXT:  %2 = CoerceThisNSInst %1
// CHECK-NEXT:  %3 = CreateFunctionInst %?anon_0_simpleReturn()
// CHECK-NEXT:  %4 = GetBuiltinClosureInst [HermesBuiltin.spawnAsync] : number
// CHECK-NEXT:  %5 = CallInst %4 : closure, undefined : undefined, %3 : closure, %2 : object, %0 : object
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function simpleAwait()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst
// CHECK-NEXT:  %1 = LoadParamInst %this
// CHECK-NEXT:  %2 = CoerceThisNSInst %1
// CHECK-NEXT:  %3 = CreateFunctionInst %?anon_0_simpleAwait()
// CHECK-NEXT:  %4 = GetBuiltinClosureInst [HermesBuiltin.spawnAsync] : number
// CHECK-NEXT:  %5 = CallInst %4 : closure, undefined : undefined, %3 : closure, %2 : object, %0 : object
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function nonSimpleArrayDestructuring()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst
// CHECK-NEXT:  %1 = LoadParamInst %this
// CHECK-NEXT:  %2 = CoerceThisNSInst %1
// CHECK-NEXT:  %3 = CreateFunctionInst %?anon_0_nonSimpleArrayDestructuring()
// CHECK-NEXT:  %4 = GetBuiltinClosureInst [HermesBuiltin.spawnAsync] : number
// CHECK-NEXT:  %5 = CallInst %4 : closure, undefined : undefined, %3 : closure, %2 : object, %0 : object
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function simpleAsyncFE()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst
// CHECK-NEXT:  %1 = LoadParamInst %this
// CHECK-NEXT:  %2 = CoerceThisNSInst %1
// CHECK-NEXT:  %3 = CreateFunctionInst %?anon_0_simpleAsyncFE()
// CHECK-NEXT:  %4 = GetBuiltinClosureInst [HermesBuiltin.spawnAsync] : number
// CHECK-NEXT:  %5 = CallInst %4 : closure, undefined : undefined, %3 : closure, %2 : object, %0 : object
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleReturn()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_?anon_0_simpleReturn()
// CHECK-NEXT:  %1 = ReturnInst %0 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleAwait()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_?anon_0_simpleAwait()
// CHECK-NEXT:  %1 = ReturnInst %0 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_nonSimpleArrayDestructuring()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_?anon_0_nonSimpleArrayDestructuring()
// CHECK-NEXT:  %1 = LoadPropertyInst %0 : object, "next" : string
// CHECK-NEXT:  %2 = CallInst %1, %0 : object
// CHECK-NEXT:  %3 = ReturnInst %0 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleAsyncFE()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_?anon_0_simpleAsyncFE()
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
// CHECK-NEXT:  %43 = StoreFrameInst %40, [x]
// CHECK-NEXT:  %44 = LoadFrameInst [x]
// CHECK-NEXT:  %45 = ReturnInst %44
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %46 = ReturnInst %40
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %47 = ReturnInst undefined : undefined
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
