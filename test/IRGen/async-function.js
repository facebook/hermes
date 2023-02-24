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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "simpleReturn": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "simpleAwait": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "nonSimpleArrayDestructuring": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "simpleAsyncFE": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %simpleReturn(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: closure, globalObject: object, "simpleReturn": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:closure) %simpleAwait(): any
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: closure, globalObject: object, "simpleAwait": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:closure) %nonSimpleArrayDestructuring(): any
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8: closure, globalObject: object, "nonSimpleArrayDestructuring": string
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %11 = StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = CreateFunctionInst (:closure) %simpleAsyncFE(): any
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12: closure, globalObject: object, "simpleAsyncFE": string
// CHECK-NEXT:  %14 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %15 = ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function simpleReturn(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %this: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:closure) %?anon_0_simpleReturn(): any
// CHECK-NEXT:  %4 = GetBuiltinClosureInst (:closure) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %5 = CallInst (:any) %4: closure, empty: any, empty: any, undefined: undefined, %3: closure, %2: object, %0: object
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function simpleAwait(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %this: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:closure) %?anon_0_simpleAwait(): any
// CHECK-NEXT:  %4 = GetBuiltinClosureInst (:closure) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %5 = CallInst (:any) %4: closure, empty: any, empty: any, undefined: undefined, %3: closure, %2: object, %0: object
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function nonSimpleArrayDestructuring(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %this: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:closure) %?anon_0_nonSimpleArrayDestructuring(): any
// CHECK-NEXT:  %4 = GetBuiltinClosureInst (:closure) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %5 = CallInst (:any) %4: closure, empty: any, empty: any, undefined: undefined, %3: closure, %2: object, %0: object
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function simpleAsyncFE(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %this: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:closure) %?anon_0_simpleAsyncFE(): any
// CHECK-NEXT:  %4 = GetBuiltinClosureInst (:closure) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %5 = CallInst (:any) %4: closure, empty: any, empty: any, undefined: undefined, %3: closure, %2: object, %0: object
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleReturn(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_?anon_0_simpleReturn(): any
// CHECK-NEXT:  %1 = ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleAwait(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_?anon_0_simpleAwait(): any
// CHECK-NEXT:  %1 = ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_nonSimpleArrayDestructuring(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_?anon_0_nonSimpleArrayDestructuring(): any
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "next": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, %0: object
// CHECK-NEXT:  %3 = ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleAsyncFE(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_?anon_0_simpleAsyncFE(): any
// CHECK-NEXT:  %1 = ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_simpleReturn(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: any
// CHECK-NEXT:  %3 = LoadStackInst (:any) %1: any
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = ReturnInst 1: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_simpleAwait(): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: any
// CHECK-NEXT:  %3 = LoadStackInst (:any) %1: any
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_isReturn: any
// CHECK-NEXT:  %7 = SaveAndYieldInst 2: number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ResumeGeneratorInst (:any) %6: any
// CHECK-NEXT:  %10 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %11 = CondBranchInst %10: any, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = StoreFrameInst %9: any, [x]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %14 = ReturnInst %13: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = ReturnInst %9: any
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %16 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_nonSimpleArrayDestructuring(?anon_2_param: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: any
// CHECK-NEXT:  %3 = LoadStackInst (:any) %1: any
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_1_isReturn_entry: any
// CHECK-NEXT:  %6 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %7 = LoadParamInst (:any) %?anon_2_param: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_3_iter: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_4_sourceOrNext: any
// CHECK-NEXT:  %10 = StoreStackInst %7: any, %9: any
// CHECK-NEXT:  %11 = IteratorBeginInst (:any) %9: any
// CHECK-NEXT:  %12 = StoreStackInst %11: any, %8: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_5_iterDone: any
// CHECK-NEXT:  %14 = StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_6_iterValue: any
// CHECK-NEXT:  %16 = StoreStackInst undefined: undefined, %15: any
// CHECK-NEXT:  %17 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %18 = ReturnInst %2: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = ResumeGeneratorInst (:any) %5: any
// CHECK-NEXT:  %20 = LoadStackInst (:any) %5: any
// CHECK-NEXT:  %21 = CondBranchInst %20: any, %BB5, %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %22 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %23 = IteratorNextInst (:any) %8: any, %22: any
// CHECK-NEXT:  %24 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %25 = BinaryStrictlyEqualInst (:any) %24: any, undefined: undefined
// CHECK-NEXT:  %26 = StoreStackInst %25: any, %13: any
// CHECK-NEXT:  %27 = CondBranchInst %25: any, %BB7, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %28 = StoreStackInst %23: any, %15: any
// CHECK-NEXT:  %29 = BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %30 = LoadStackInst (:any) %15: any
// CHECK-NEXT:  %31 = StoreFrameInst %30: any, [x]: any
// CHECK-NEXT:  %32 = LoadStackInst (:any) %13: any
// CHECK-NEXT:  %33 = CondBranchInst %32: any, %BB9, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %34 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %35 = IteratorCloseInst (:any) %34: any, false: boolean
// CHECK-NEXT:  %36 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %37 = SaveAndYieldInst undefined: undefined, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %38 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %39 = AllocStackInst (:any) $?anon_8_isReturn: any
// CHECK-NEXT:  %40 = SaveAndYieldInst %38: any, %BB11
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %41 = ReturnInst %19: any
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %42 = ResumeGeneratorInst (:any) %39: any
// CHECK-NEXT:  %43 = LoadStackInst (:any) %39: any
// CHECK-NEXT:  %44 = CondBranchInst %43: any, %BB12, %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %45 = StoreFrameInst %42: any, [x]: any
// CHECK-NEXT:  %46 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %47 = ReturnInst %46: any
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %48 = ReturnInst %42: any
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %49 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_simpleAsyncFE(): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: any
// CHECK-NEXT:  %3 = LoadStackInst (:any) %1: any
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_isReturn: any
// CHECK-NEXT:  %7 = SaveAndYieldInst 2: number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ResumeGeneratorInst (:any) %6: any
// CHECK-NEXT:  %10 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %11 = CondBranchInst %10: any, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = StoreFrameInst %9: any, [x]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %14 = ReturnInst %13: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = ReturnInst %9: any
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %16 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
