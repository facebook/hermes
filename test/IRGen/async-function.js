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

// CHECK:scope %VS1 []

// CHECK:function simpleReturn(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %4 = CreateScopeInst (:environment) %VS1: any, %3: environment
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %4: environment, %VS1: any, %?anon_0_simpleReturn(): functionCode
// CHECK-NEXT:  %6 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %7 = CallInst (:any) %6: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %5: object, %2: object, %0: object
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:function simpleAwait(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %4 = CreateScopeInst (:environment) %VS2: any, %3: environment
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %4: environment, %VS2: any, %?anon_0_simpleAwait(): functionCode
// CHECK-NEXT:  %6 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %7 = CallInst (:any) %6: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %5: object, %2: object, %0: object
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:function nonSimpleArrayDestructuring(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %4 = CreateScopeInst (:environment) %VS3: any, %3: environment
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %4: environment, %VS3: any, %?anon_0_nonSimpleArrayDestructuring(): functionCode
// CHECK-NEXT:  %6 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %7 = CallInst (:any) %6: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %5: object, %2: object, %0: object
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:scope %VS4 []

// CHECK:function simpleAsyncFE(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %4 = CreateScopeInst (:environment) %VS4: any, %3: environment
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %4: environment, %VS4: any, %?anon_0_simpleAsyncFE(): functionCode
// CHECK-NEXT:  %6 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %7 = CallInst (:any) %6: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %5: object, %2: object, %0: object
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleReturn(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS1: any, %?anon_0_simpleReturn?inner(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleAwait(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS2: any, %?anon_0_simpleAwait?inner(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_nonSimpleArrayDestructuring(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS3: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS3: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS3: any, %?anon_0_nonSimpleArrayDestructuring?inner(): functionCode
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: object, "next": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, %2: object
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_simpleAsyncFE(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS4: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS4: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS4: any, %?anon_0_simpleAsyncFE?inner(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:scope %VS5 []

// CHECK:generator inner ?anon_0_simpleReturn?inner(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %5 = CreateScopeInst (:environment) %VS5: any, %4: environment
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end

// CHECK:scope %VS6 [x: any]

// CHECK:generator inner ?anon_0_simpleAwait?inner(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %5 = CreateScopeInst (:environment) %VS6: any, %4: environment
// CHECK-NEXT:       StoreFrameInst %5: environment, undefined: undefined, [%VS6.x]: any
// CHECK-NEXT:  %7 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 2: number, false: boolean, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = ResumeGeneratorInst (:any) %7: boolean
// CHECK-NEXT:  %11 = LoadStackInst (:boolean) %7: boolean
// CHECK-NEXT:        CondBranchInst %11: boolean, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        StoreFrameInst %5: environment, %10: any, [%VS6.x]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %5: environment, [%VS6.x]: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end

// CHECK:scope %VS7 [x: any]

// CHECK:generator inner ?anon_0_nonSimpleArrayDestructuring?inner(?anon_2_param: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = AllocStackInst (:boolean) $?anon_1_isReturn_entry: any
// CHECK-NEXT:  %5 = GetParentScopeInst (:environment) %VS3: any, %parentScope: environment
// CHECK-NEXT:  %6 = CreateScopeInst (:environment) %VS7: any, %5: environment
// CHECK-NEXT:       StoreFrameInst %6: environment, undefined: undefined, [%VS7.x]: any
// CHECK-NEXT:  %8 = LoadParamInst (:any) %?anon_2_param: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_3_iter: any
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_4_sourceOrNext: any
// CHECK-NEXT:        StoreStackInst %8: any, %10: any
// CHECK-NEXT:  %12 = IteratorBeginInst (:any) %10: any
// CHECK-NEXT:        StoreStackInst %12: any, %9: any
// CHECK-NEXT:  %14 = AllocStackInst (:any) $?anon_5_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %14: any
// CHECK-NEXT:  %16 = AllocStackInst (:any) $?anon_6_iterValue: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %16: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %20 = ResumeGeneratorInst (:any) %4: boolean
// CHECK-NEXT:  %21 = LoadStackInst (:boolean) %4: boolean
// CHECK-NEXT:        CondBranchInst %21: boolean, %BB10, %BB9
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %23 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %24 = IteratorNextInst (:any) %9: any, %23: any
// CHECK-NEXT:  %25 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %26 = BinaryStrictlyEqualInst (:any) %25: any, undefined: undefined
// CHECK-NEXT:        StoreStackInst %26: any, %14: any
// CHECK-NEXT:        CondBranchInst %26: any, %BB6, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        StoreStackInst %24: any, %16: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %31 = LoadStackInst (:any) %16: any
// CHECK-NEXT:        StoreFrameInst %6: environment, %31: any, [%VS7.x]: any
// CHECK-NEXT:  %33 = LoadStackInst (:any) %14: any
// CHECK-NEXT:        CondBranchInst %33: any, %BB8, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %35 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %36 = IteratorCloseInst (:any) %35: any, false: boolean
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        SaveAndYieldInst undefined: undefined, false: boolean, %BB3
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %39 = LoadFrameInst (:any) %6: environment, [%VS7.x]: any
// CHECK-NEXT:  %40 = AllocStackInst (:boolean) $?anon_8_isReturn: any
// CHECK-NEXT:        SaveAndYieldInst %39: any, false: boolean, %BB11
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        ReturnInst %20: any
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %43 = ResumeGeneratorInst (:any) %40: boolean
// CHECK-NEXT:  %44 = LoadStackInst (:boolean) %40: boolean
// CHECK-NEXT:        CondBranchInst %44: boolean, %BB13, %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        StoreFrameInst %6: environment, %43: any, [%VS7.x]: any
// CHECK-NEXT:  %47 = LoadFrameInst (:any) %6: environment, [%VS7.x]: any
// CHECK-NEXT:        ReturnInst %47: any
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        ReturnInst %43: any
// CHECK-NEXT:function_end

// CHECK:scope %VS8 [x: any]

// CHECK:generator inner ?anon_0_simpleAsyncFE?inner(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetParentScopeInst (:environment) %VS4: any, %parentScope: environment
// CHECK-NEXT:  %5 = CreateScopeInst (:environment) %VS8: any, %4: environment
// CHECK-NEXT:       StoreFrameInst %5: environment, undefined: undefined, [%VS8.x]: any
// CHECK-NEXT:  %7 = AllocStackInst (:boolean) $?anon_1_isReturn: any
// CHECK-NEXT:       SaveAndYieldInst 2: number, false: boolean, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = ResumeGeneratorInst (:any) %7: boolean
// CHECK-NEXT:  %11 = LoadStackInst (:boolean) %7: boolean
// CHECK-NEXT:        CondBranchInst %11: boolean, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        StoreFrameInst %5: environment, %10: any, [%VS8.x]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %5: environment, [%VS8.x]: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end
