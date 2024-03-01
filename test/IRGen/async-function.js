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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "simpleReturn": string
// CHECK-NEXT:       DeclareGlobalVarInst "simpleAwait": string
// CHECK-NEXT:       DeclareGlobalVarInst "nonSimpleArrayDestructuring": string
// CHECK-NEXT:       DeclareGlobalVarInst "simpleAsyncFE": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %simpleReturn(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "simpleReturn": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %simpleAwait(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "simpleAwait": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %nonSimpleArrayDestructuring(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "nonSimpleArrayDestructuring": string
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %11: any
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %simpleAsyncFE(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "simpleAsyncFE": string
// CHECK-NEXT:  %15 = LoadStackInst (:any) %11: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:function simpleReturn(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %4 = CreateScopeInst (:environment) %simpleReturn(): any, %3: environment
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %4: environment, %?anon_0_simpleReturn(): functionCode
// CHECK-NEXT:  %6 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %7 = CallInst (:any) %6: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %5: object, %2: object, %0: object
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:function simpleAwait(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %4 = CreateScopeInst (:environment) %simpleAwait(): any, %3: environment
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %4: environment, %?anon_0_simpleAwait(): functionCode
// CHECK-NEXT:  %6 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %7 = CallInst (:any) %6: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %5: object, %2: object, %0: object
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:function nonSimpleArrayDestructuring(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %4 = CreateScopeInst (:environment) %nonSimpleArrayDestructuring(): any, %3: environment
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %4: environment, %?anon_0_nonSimpleArrayDestructuring(): functionCode
// CHECK-NEXT:  %6 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %7 = CallInst (:any) %6: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %5: object, %2: object, %0: object
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:function simpleAsyncFE(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %4 = CreateScopeInst (:environment) %simpleAsyncFE(): any, %3: environment
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %4: environment, %?anon_0_simpleAsyncFE(): functionCode
// CHECK-NEXT:  %6 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %7 = CallInst (:any) %6: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %5: object, %2: object, %0: object
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleReturn(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %simpleReturn(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %?anon_0_simpleReturn(): any, %0: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %?anon_0_?anon_0_simpleReturn(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleAwait(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %simpleAwait(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %?anon_0_simpleAwait(): any, %0: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %?anon_0_?anon_0_simpleAwait(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_nonSimpleArrayDestructuring(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %nonSimpleArrayDestructuring(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %?anon_0_nonSimpleArrayDestructuring(): any, %0: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %?anon_0_?anon_0_nonSimpleArrayDestructuring(): functionCode
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: object, "next": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, %2: object
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleAsyncFE(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %simpleAsyncFE(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %?anon_0_simpleAsyncFE(): any, %0: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %?anon_0_?anon_0_simpleAsyncFE(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_simpleReturn(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = GetParentScopeInst (:environment) %?anon_0_simpleReturn(): any, %parentScope: environment
// CHECK-NEXT:  %6 = CreateScopeInst (:environment) %?anon_0_?anon_0_simpleReturn(): any, %5: environment
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_simpleAwait(): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = GetParentScopeInst (:environment) %?anon_0_simpleAwait(): any, %parentScope: environment
// CHECK-NEXT:  %6 = CreateScopeInst (:environment) %?anon_0_?anon_0_simpleAwait(): any, %5: environment
// CHECK-NEXT:       StoreFrameInst %6: environment, undefined: undefined, [x]: any
// CHECK-NEXT:  %8 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 2: number, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = ResumeGeneratorInst (:any) %8: boolean
// CHECK-NEXT:  %12 = LoadStackInst (:boolean) %8: boolean
// CHECK-NEXT:        CondBranchInst %12: boolean, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        StoreFrameInst %6: environment, %11: any, [x]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %6: environment, [x]: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_nonSimpleArrayDestructuring(?anon_2_param: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = AllocStackInst (:boolean) $?anon_1_isReturn_entry: any
// CHECK-NEXT:  %6 = GetParentScopeInst (:environment) %?anon_0_nonSimpleArrayDestructuring(): any, %parentScope: environment
// CHECK-NEXT:  %7 = CreateScopeInst (:environment) %?anon_0_?anon_0_nonSimpleArrayDestructuring(): any, %6: environment
// CHECK-NEXT:       StoreFrameInst %7: environment, undefined: undefined, [x]: any
// CHECK-NEXT:  %9 = LoadParamInst (:any) %?anon_2_param: any
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_3_iter: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_4_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %9: any, %11: any
// CHECK-NEXT:  %13 = IteratorBeginInst (:any) %11: any
// CHECK-NEXT:        StoreStackInst %13: any, %10: any
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_5_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %15: any
// CHECK-NEXT:  %17 = AllocStackInst (:any) $?anon_6_iterValue: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %17: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %21 = ResumeGeneratorInst (:any) %5: boolean
// CHECK-NEXT:  %22 = LoadStackInst (:boolean) %5: boolean
// CHECK-NEXT:        CondBranchInst %22: boolean, %BB10, %BB9
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %24 = LoadStackInst (:any) %11: any
// CHECK-NEXT:  %25 = IteratorNextInst (:any) %10: any, %24: any
// CHECK-NEXT:  %26 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %27 = BinaryStrictlyEqualInst (:any) %26: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %27: any, %15: any
// CHECK-NEXT:        CondBranchInst %27: any, %BB6, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreStackInst %25: any, %17: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %32 = LoadStackInst (:any) %17: any
// CHECK-NEXT:        StoreFrameInst %7: environment, %32: any, [x]: any
// CHECK-NEXT:  %34 = LoadStackInst (:any) %15: any
// CHECK-NEXT:        CondBranchInst %34: any, %BB8, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %36 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %37 = IteratorCloseInst (:any) %36: any, false: boolean
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        SaveAndYieldInst undefined: undefined, %BB3
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %40 = LoadFrameInst (:any) %7: environment, [x]: any
// CHECK-NEXT:  %41 = AllocStackInst (:boolean) $?anon_8_isReturn: any
// CHECK-NEXT:        SaveAndYieldInst %40: any, %BB11
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        ReturnInst %21: any
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %44 = ResumeGeneratorInst (:any) %41: boolean
// CHECK-NEXT:  %45 = LoadStackInst (:boolean) %41: boolean
// CHECK-NEXT:        CondBranchInst %45: boolean, %BB13, %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        StoreFrameInst %7: environment, %44: any, [x]: any
// CHECK-NEXT:  %48 = LoadFrameInst (:any) %7: environment, [x]: any
// CHECK-NEXT:        ReturnInst %48: any
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        ReturnInst %44: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_simpleAsyncFE(): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %2 = ResumeGeneratorInst (:any) %1: boolean
// CHECK-NEXT:  %3 = LoadStackInst (:boolean) %1: boolean
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = GetParentScopeInst (:environment) %?anon_0_simpleAsyncFE(): any, %parentScope: environment
// CHECK-NEXT:  %6 = CreateScopeInst (:environment) %?anon_0_?anon_0_simpleAsyncFE(): any, %5: environment
// CHECK-NEXT:       StoreFrameInst %6: environment, undefined: undefined, [x]: any
// CHECK-NEXT:  %8 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 2: number, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst %2: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = ResumeGeneratorInst (:any) %8: boolean
// CHECK-NEXT:  %12 = LoadStackInst (:boolean) %8: boolean
// CHECK-NEXT:        CondBranchInst %12: boolean, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        StoreFrameInst %6: environment, %11: any, [x]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %6: environment, [x]: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end
