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
// CHECK-NEXT:       DeclareGlobalVarInst "logical_and": string
// CHECK-NEXT:       DeclareGlobalVarInst "logical_or": string
// CHECK-NEXT:       DeclareGlobalVarInst "logical_and_and": string
// CHECK-NEXT:       DeclareGlobalVarInst "logical_and_or": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %logical_and(): any
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "logical_and": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %logical_or(): any
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "logical_or": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %logical_and_and(): any
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "logical_and_and": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %logical_and_or(): any
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "logical_and_or": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function logical_and(y: any, x: any): any
// CHECK-NEXT:frame = [y: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %0: any, [y]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %2: any, [x]: any
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:       StoreStackInst %5: any, %4: any
// CHECK-NEXT:       CondBranchInst %5: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:       StoreStackInst %8: any, %4: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = LoadStackInst (:any) %4: any
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:function logical_or(y: any, x: any): any
// CHECK-NEXT:frame = [y: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %0: any, [y]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %2: any, [x]: any
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:       StoreStackInst %5: any, %4: any
// CHECK-NEXT:       CondBranchInst %5: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:       StoreStackInst %8: any, %4: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = LoadStackInst (:any) %4: any
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:function logical_and_and(y: any, x: any, z: any): any
// CHECK-NEXT:frame = [y: any, x: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %0: any, [y]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %z: any
// CHECK-NEXT:       StoreFrameInst %4: any, [z]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_1_logical: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:       StoreStackInst %8: any, %7: any
// CHECK-NEXT:        CondBranchInst %8: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [z]: any
// CHECK-NEXT:        StoreStackInst %11: any, %6: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = LoadStackInst (:any) %6: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:        StoreStackInst %16: any, %7: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %19 = LoadStackInst (:any) %7: any
// CHECK-NEXT:        StoreStackInst %19: any, %6: any
// CHECK-NEXT:        CondBranchInst %19: any, %BB3, %BB4
// CHECK-NEXT:function_end

// CHECK:function logical_and_or(y: any, x: any, z: any): any
// CHECK-NEXT:frame = [y: any, x: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %0: any, [y]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %z: any
// CHECK-NEXT:       StoreFrameInst %4: any, [z]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:       StoreStackInst %7: any, %6: any
// CHECK-NEXT:       CondBranchInst %7: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_1_logical: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:        StoreStackInst %11: any, %10: any
// CHECK-NEXT:        CondBranchInst %11: any, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %14 = LoadStackInst (:any) %6: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %16 = LoadFrameInst (:any) [z]: any
// CHECK-NEXT:        StoreStackInst %16: any, %10: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %19 = LoadStackInst (:any) %10: any
// CHECK-NEXT:        StoreStackInst %19: any, %6: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end
