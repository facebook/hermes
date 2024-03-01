/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function test0(x,  y) {
  if (x) { return x; } else { return y; }
  return y;
}

function test1(x,  y) {
  if (x) { } else { return y; }
}

function test2(x,  y) {
  if (x) { return x; } else {  }
}

function test3(x,  y) {
  return x;
  if (x) { return x; } else { return x; }
}

function test4(x,  y) {
  return x;
  if (x) { return x; } else { return x; }
}

function test5() {
  return;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "test0": string
// CHECK-NEXT:       DeclareGlobalVarInst "test1": string
// CHECK-NEXT:       DeclareGlobalVarInst "test2": string
// CHECK-NEXT:       DeclareGlobalVarInst "test3": string
// CHECK-NEXT:       DeclareGlobalVarInst "test4": string
// CHECK-NEXT:       DeclareGlobalVarInst "test5": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %test0(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "test0": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %test1(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "test1": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %test2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "test2": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %test3(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "test3": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %0: environment, %test4(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "test4": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %0: environment, %test5(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "test5": string
// CHECK-NEXT:  %19 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %19: any
// CHECK-NEXT:  %21 = LoadStackInst (:any) %19: any
// CHECK-NEXT:        ReturnInst %21: any
// CHECK-NEXT:function_end

// CHECK:function test0(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test0(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:       CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end

// CHECK:function test1(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test1(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:       CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test2(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:       CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test3(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test3(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function test4(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test4(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function test5(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test5(): any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
