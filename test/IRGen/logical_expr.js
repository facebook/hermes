/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function logical_and(y, x) { return x && y }

function logical_or(y, x) { return x || y }

function logical_and_and(y, x, z) { return x && y && z }

function logical_and_or(y, x, z) { return x && (y || z) }

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "logical_and": string
// CHECK-NEXT:       DeclareGlobalVarInst "logical_or": string
// CHECK-NEXT:       DeclareGlobalVarInst "logical_and_and": string
// CHECK-NEXT:       DeclareGlobalVarInst "logical_and_or": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %logical_and(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "logical_and": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %logical_or(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "logical_or": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %logical_and_and(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "logical_and_and": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %logical_and_or(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "logical_and_or": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:function logical_and(y: any, x: any): any
// CHECK-NEXT:frame = [y: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %logical_and(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [x]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:       StoreStackInst %7: any, %6: any
// CHECK-NEXT:       CondBranchInst %7: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:        StoreStackInst %10: any, %6: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = LoadStackInst (:any) %6: any
// CHECK-NEXT:        ReturnInst %13: any
// CHECK-NEXT:function_end

// CHECK:function logical_or(y: any, x: any): any
// CHECK-NEXT:frame = [y: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %logical_or(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [x]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:       StoreStackInst %7: any, %6: any
// CHECK-NEXT:       CondBranchInst %7: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:        StoreStackInst %10: any, %6: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = LoadStackInst (:any) %6: any
// CHECK-NEXT:        ReturnInst %13: any
// CHECK-NEXT:function_end

// CHECK:function logical_and_and(y: any, x: any, z: any): any
// CHECK-NEXT:frame = [y: any, x: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %logical_and_and(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [x]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %z: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [z]: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_1_logical: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:        StoreStackInst %10: any, %9: any
// CHECK-NEXT:        CondBranchInst %10: any, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [z]: any
// CHECK-NEXT:        StoreStackInst %13: any, %8: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %16 = LoadStackInst (:any) %8: any
// CHECK-NEXT:        ReturnInst %16: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:        StoreStackInst %18: any, %9: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %21 = LoadStackInst (:any) %9: any
// CHECK-NEXT:        StoreStackInst %21: any, %8: any
// CHECK-NEXT:        CondBranchInst %21: any, %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:function logical_and_or(y: any, x: any, z: any): any
// CHECK-NEXT:frame = [y: any, x: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %logical_and_or(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [x]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %z: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [z]: any
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:        StoreStackInst %9: any, %8: any
// CHECK-NEXT:        CondBranchInst %9: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_1_logical: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:        StoreStackInst %13: any, %12: any
// CHECK-NEXT:        CondBranchInst %13: any, %BB4, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %16 = LoadStackInst (:any) %8: any
// CHECK-NEXT:        ReturnInst %16: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = LoadFrameInst (:any) %1: environment, [z]: any
// CHECK-NEXT:        StoreStackInst %18: any, %12: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %21 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        StoreStackInst %21: any, %8: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end
