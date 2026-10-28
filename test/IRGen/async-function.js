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

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "simpleReturn": string
// CHECK-NEXT:       DeclareGlobalVarInst "simpleAwait": string
// CHECK-NEXT:       DeclareGlobalVarInst "nonSimpleArrayDestructuring": string
// CHECK-NEXT:       DeclareGlobalVarInst "simpleAsyncFE": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %VS0: any, %simpleReturn(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "simpleReturn": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %VS0: any, %simpleAwait(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "simpleAwait": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %VS0: any, %nonSimpleArrayDestructuring(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "nonSimpleArrayDestructuring": string
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %11: any
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %VS0: any, %simpleAsyncFE(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "simpleAsyncFE": string
// CHECK-NEXT:  %15 = LoadStackInst (:any) %11: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:function simpleReturn(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS0: any, %?anon_0_simpleReturn(): functionCode
// CHECK-NEXT:  %3 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %4 = CallInst (:any) %3: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %2: object
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:function simpleAwait(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS0: any, %?anon_0_simpleAwait(): functionCode
// CHECK-NEXT:  %3 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %4 = CallInst (:any) %3: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %2: object
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:function nonSimpleArrayDestructuring(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS0: any, %?anon_0_nonSimpleArrayDestructuring(): functionCode
// CHECK-NEXT:  %3 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %4 = CallInst (:any) %3: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %2: object
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:function simpleAsyncFE(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS0: any, %?anon_0_simpleAsyncFE(): functionCode
// CHECK-NEXT:  %3 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %4 = CallInst (:any) %3: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %2: object
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 []

// CHECK:generator inner ?anon_0_simpleReturn(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %5 = CreateScopeInst (:environment) %VS1: any, %4: environment
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [x: any]

// CHECK:generator inner ?anon_0_simpleAwait(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %5 = CreateScopeInst (:environment) %VS2: any, %4: environment
// CHECK-NEXT:       StoreFrameInst %5: environment, undefined: undefined, [%VS2.x]: any
// CHECK-NEXT:  %7 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 2: number, false: boolean, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = ResumeGeneratorInst (:any) %7: boolean
// CHECK-NEXT:  %11 = LoadStackInst (:boolean) %7: boolean
// CHECK-NEXT:        CondBranchInst %11: boolean, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        StoreFrameInst %5: environment, %10: any, [%VS2.x]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %5: environment, [%VS2.x]: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [x: any]

// CHECK:generator inner ?anon_0_nonSimpleArrayDestructuring(?anon_1_param: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %5 = CreateScopeInst (:environment) %VS3: any, %4: environment
// CHECK-NEXT:       StoreFrameInst %5: environment, undefined: undefined, [%VS3.x]: any
// CHECK-NEXT:  %7 = LoadParamInst (:any) %?anon_1_param: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_2_iter: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_3_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %7: any, %9: any
// CHECK-NEXT:  %11 = IteratorBeginInst (:any) %9: any
// CHECK-NEXT:        StoreStackInst %11: any, %8: any
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_4_iterValue: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %15 = IteratorNextInst (:any) %8: any, %14: any
// CHECK-NEXT:        StoreStackInst %15: any, %13: any
// CHECK-NEXT:  %17 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        StoreFrameInst %5: environment, %17: any, [%VS3.x]: any
// CHECK-NEXT:  %19 = LoadStackInst (:any) %8: any
// CHECK-NEXT:  %20 = IteratorCloseInst (:any) %19: any, false: boolean
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %5: environment, [%VS3.x]: any
// CHECK-NEXT:  %22 = AllocStackInst (:boolean) $?anon_6_isReturn: any
// CHECK-NEXT:        SaveAndYieldInst %21: any, false: boolean, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %25 = ResumeGeneratorInst (:any) %22: boolean
// CHECK-NEXT:  %26 = LoadStackInst (:boolean) %22: boolean
// CHECK-NEXT:        CondBranchInst %26: boolean, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        StoreFrameInst %5: environment, %25: any, [%VS3.x]: any
// CHECK-NEXT:  %29 = LoadFrameInst (:any) %5: environment, [%VS3.x]: any
// CHECK-NEXT:        ReturnInst %29: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst %25: any
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [x: any]

// CHECK:generator inner ?anon_0_simpleAsyncFE(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %5 = CreateScopeInst (:environment) %VS4: any, %4: environment
// CHECK-NEXT:       StoreFrameInst %5: environment, undefined: undefined, [%VS4.x]: any
// CHECK-NEXT:  %7 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 2: number, false: boolean, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = ResumeGeneratorInst (:any) %7: boolean
// CHECK-NEXT:  %11 = LoadStackInst (:boolean) %7: boolean
// CHECK-NEXT:        CondBranchInst %11: boolean, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        StoreFrameInst %5: environment, %10: any, [%VS4.x]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %5: environment, [%VS4.x]: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end
