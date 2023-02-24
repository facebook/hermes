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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "logical_and": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "logical_or": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "logical_and_and": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "logical_and_or": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %logical_and(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: closure, globalObject: object, "logical_and": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:closure) %logical_or(): any
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: closure, globalObject: object, "logical_or": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:closure) %logical_and_and(): any
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8: closure, globalObject: object, "logical_and_and": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:closure) %logical_and_or(): any
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: closure, globalObject: object, "logical_and_or": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %13 = StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:  %15 = ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function logical_and(y: any, x: any): any
// CHECK-NEXT:frame = [y: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [y]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [x]: any
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %6 = StoreStackInst %5: any, %4: any
// CHECK-NEXT:  %7 = CondBranchInst %5: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %9 = StoreStackInst %8: any, %4: any
// CHECK-NEXT:  %10 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = LoadStackInst (:any) %4: any
// CHECK-NEXT:  %12 = ReturnInst %11: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function logical_or(y: any, x: any): any
// CHECK-NEXT:frame = [y: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [y]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [x]: any
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %6 = StoreStackInst %5: any, %4: any
// CHECK-NEXT:  %7 = CondBranchInst %5: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %9 = StoreStackInst %8: any, %4: any
// CHECK-NEXT:  %10 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = LoadStackInst (:any) %4: any
// CHECK-NEXT:  %12 = ReturnInst %11: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function logical_and_and(y: any, x: any, z: any): any
// CHECK-NEXT:frame = [y: any, x: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [y]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [z]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_1_logical: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %9 = StoreStackInst %8: any, %7: any
// CHECK-NEXT:  %10 = CondBranchInst %8: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [z]: any
// CHECK-NEXT:  %12 = StoreStackInst %11: any, %6: any
// CHECK-NEXT:  %13 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %15 = ReturnInst %14: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %17 = StoreStackInst %16: any, %7: any
// CHECK-NEXT:  %18 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %19 = LoadStackInst (:any) %7: any
// CHECK-NEXT:  %20 = StoreStackInst %19: any, %6: any
// CHECK-NEXT:  %21 = CondBranchInst %19: any, %BB3, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %22 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function logical_and_or(y: any, x: any, z: any): any
// CHECK-NEXT:frame = [y: any, x: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [y]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [z]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %8 = StoreStackInst %7: any, %6: any
// CHECK-NEXT:  %9 = CondBranchInst %7: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_1_logical: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %12 = StoreStackInst %11: any, %10: any
// CHECK-NEXT:  %13 = CondBranchInst %11: any, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %14 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %15 = ReturnInst %14: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %16 = LoadFrameInst (:any) [z]: any
// CHECK-NEXT:  %17 = StoreStackInst %16: any, %10: any
// CHECK-NEXT:  %18 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %19 = LoadStackInst (:any) %10: any
// CHECK-NEXT:  %20 = StoreStackInst %19: any, %6: any
// CHECK-NEXT:  %21 = BranchInst %BB2
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %22 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
