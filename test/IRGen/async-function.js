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
// CHECK-NEXT:       DeclareGlobalVarInst "simpleReturn": string
// CHECK-NEXT:       DeclareGlobalVarInst "simpleAwait": string
// CHECK-NEXT:       DeclareGlobalVarInst "nonSimpleArrayDestructuring": string
// CHECK-NEXT:       DeclareGlobalVarInst "simpleAsyncFE": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %simpleReturn(): any
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "simpleReturn": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %simpleAwait(): any
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "simpleAwait": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %nonSimpleArrayDestructuring(): any
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "nonSimpleArrayDestructuring": string
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %simpleAsyncFE(): any
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "simpleAsyncFE": string
// CHECK-NEXT:  %14 = LoadStackInst (:any) %10: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function simpleReturn(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %?anon_0_simpleReturn(): any
// CHECK-NEXT:  %4 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %5 = CallInst (:any) %4: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: object, %2: object, %0: object
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function simpleAwait(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %?anon_0_simpleAwait(): any
// CHECK-NEXT:  %4 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %5 = CallInst (:any) %4: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: object, %2: object, %0: object
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function nonSimpleArrayDestructuring(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %?anon_0_nonSimpleArrayDestructuring(): any
// CHECK-NEXT:  %4 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %5 = CallInst (:any) %4: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: object, %2: object, %0: object
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function simpleAsyncFE(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %?anon_0_simpleAsyncFE(): any
// CHECK-NEXT:  %4 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %5 = CallInst (:any) %4: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: object, %2: object, %0: object
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleReturn(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_?anon_0_simpleReturn(): any
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleAwait(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_?anon_0_simpleAwait(): any
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_nonSimpleArrayDestructuring(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_?anon_0_nonSimpleArrayDestructuring(): any
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "next": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, %0: object
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleAsyncFE(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst (:object) %?anon_0_?anon_0_simpleAsyncFE(): any
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_simpleReturn(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_simpleAwait(): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %6 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 2: number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ResumeGeneratorInst (:any) %6: boolean
// CHECK-NEXT:  %10 = LoadStackInst (:boolean) %6: boolean
// CHECK-NEXT:        CondBranchInst %10: boolean, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreFrameInst %9: any, [x]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:        ReturnInst %13: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_nonSimpleArrayDestructuring(?anon_2_param: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = AllocStackInst (:boolean) $?anon_1_isReturn_entry: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %7 = LoadParamInst (:any) %?anon_2_param: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_3_iter: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_4_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %7: any, %9: any
// CHECK-NEXT:  %11 = IteratorBeginInst (:any) %9: any
// CHECK-NEXT:        StoreStackInst %11: any, %8: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_5_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_6_iterValue: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %15: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ReturnInst %2: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = ResumeGeneratorInst (:any) %5: boolean
// CHECK-NEXT:  %20 = LoadStackInst (:boolean) %5: boolean
// CHECK-NEXT:        CondBranchInst %20: boolean, %BB5, %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %22 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %23 = IteratorNextInst (:any) %8: any, %22: any
// CHECK-NEXT:  %24 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %25 = BinaryStrictlyEqualInst (:any) %24: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %25: any, %13: any
// CHECK-NEXT:        CondBranchInst %25: any, %BB7, %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        StoreStackInst %23: any, %15: any
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %30 = LoadStackInst (:any) %15: any
// CHECK-NEXT:        StoreFrameInst %30: any, [x]: any
// CHECK-NEXT:  %32 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        CondBranchInst %32: any, %BB9, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %34 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %35 = IteratorCloseInst (:any) %34: any, false: boolean
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        SaveAndYieldInst undefined: undefined, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %38 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %39 = AllocStackInst (:boolean) $?anon_8_isReturn: any
// CHECK-NEXT:        SaveAndYieldInst %38: any, %BB11
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst %19: any
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %42 = ResumeGeneratorInst (:any) %39: boolean
// CHECK-NEXT:  %43 = LoadStackInst (:boolean) %39: boolean
// CHECK-NEXT:        CondBranchInst %43: boolean, %BB12, %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        StoreFrameInst %42: any, [x]: any
// CHECK-NEXT:  %46 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:        ReturnInst %46: any
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        ReturnInst %42: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_simpleAsyncFE(): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %6 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 2: number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ResumeGeneratorInst (:any) %6: boolean
// CHECK-NEXT:  %10 = LoadStackInst (:boolean) %6: boolean
// CHECK-NEXT:        CondBranchInst %10: boolean, %BB4, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreFrameInst %9: any, [x]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:        ReturnInst %13: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end
