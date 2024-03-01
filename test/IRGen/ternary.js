/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

// Simple test.
function test_one(x,y,z) {
  return x ? y : z;
}

// Assignment expression.

function test_two() {
  var stop = false, age = 16;
  return age > 18 ? age = 2 : stop = true;
}

// Function call expression.
function test_three(x, one, two) {
  return x ? ( one() ) : ( two() );
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "test_one": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_two": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_three": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %test_one(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "test_one": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %test_two(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "test_two": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %test_three(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "test_three": string
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = LoadStackInst (:any) %10: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:function test_one(x: any, y: any, z: any): any
// CHECK-NEXT:frame = [x: any, y: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test_one(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %z: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [z]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:       CondBranchInst %8: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [z]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = PhiInst (:any) %12: any, %BB2, %10: any, %BB1
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function test_two(): any
// CHECK-NEXT:frame = [stop: any, age: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test_two(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [stop]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [age]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, false: boolean, [stop]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 16: number, [age]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [age]: any
// CHECK-NEXT:  %7 = BinaryGreaterThanInst (:boolean) %6: any, 18: number
// CHECK-NEXT:       CondBranchInst %7: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       StoreFrameInst %1: environment, true: boolean, [stop]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        StoreFrameInst %1: environment, 2: number, [age]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = PhiInst (:boolean|number) 2: number, %BB2, true: boolean, %BB1
// CHECK-NEXT:        ReturnInst %13: boolean|number
// CHECK-NEXT:function_end

// CHECK:function test_three(x: any, one: any, two: any): any
// CHECK-NEXT:frame = [x: any, one: any, two: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test_three(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %one: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [one]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %two: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [two]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:       CondBranchInst %8: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [two]: any
// CHECK-NEXT:  %11 = CallInst (:any) %10: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [one]: any
// CHECK-NEXT:  %14 = CallInst (:any) %13: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = PhiInst (:any) %14: any, %BB2, %11: any, %BB1
// CHECK-NEXT:        ReturnInst %16: any
// CHECK-NEXT:function_end
