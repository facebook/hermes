/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen --match-full-lines %s

function testAnd(x, y) {
  return x.foo &&= y;
}

function testOr(x, y) {
  return x.foo ||= y;
}

function testNullish(x, y) {
  return x.foo ??= y;
}

function testComplex(x, y, z) {
  return x ||= y || z;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "testAnd": string
// CHECK-NEXT:       DeclareGlobalVarInst "testOr": string
// CHECK-NEXT:       DeclareGlobalVarInst "testNullish": string
// CHECK-NEXT:       DeclareGlobalVarInst "testComplex": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %testAnd(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "testAnd": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %testOr(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "testOr": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %testNullish(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "testNullish": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %testComplex(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "testComplex": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:function testAnd(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %testAnd(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "foo": string
// CHECK-NEXT:       CondBranchInst %7: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:        StorePropertyLooseInst %9: any, %6: any, "foo": string
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = PhiInst (:any) %7: any, %BB0, %9: any, %BB1
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:function testOr(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %testOr(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "foo": string
// CHECK-NEXT:       CondBranchInst %7: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:        StorePropertyLooseInst %9: any, %6: any, "foo": string
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = PhiInst (:any) %7: any, %BB0, %9: any, %BB1
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:function testNullish(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %testNullish(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "foo": string
// CHECK-NEXT:  %8 = BinaryEqualInst (:any) %7: any, null: null
// CHECK-NEXT:       CondBranchInst %8: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:        StorePropertyLooseInst %10: any, %6: any, "foo": string
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = PhiInst (:any) %7: any, %BB0, %10: any, %BB1
// CHECK-NEXT:        ReturnInst %13: any
// CHECK-NEXT:function_end

// CHECK:function testComplex(x: any, y: any, z: any): any
// CHECK-NEXT:frame = [x: any, y: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %testComplex(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %z: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [z]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:       CondBranchInst %8: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:        StoreStackInst %11: any, %10: any
// CHECK-NEXT:        CondBranchInst %11: any, %BB4, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %14 = PhiInst (:any) %8: any, %BB0, %19: any, %BB4
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadFrameInst (:any) %1: environment, [z]: any
// CHECK-NEXT:        StoreStackInst %16: any, %10: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = LoadStackInst (:any) %10: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %19: any, [x]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end
