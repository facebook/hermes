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
// CHECK-NEXT:       DeclareGlobalVarInst "testAnd": string
// CHECK-NEXT:       DeclareGlobalVarInst "testOr": string
// CHECK-NEXT:       DeclareGlobalVarInst "testNullish": string
// CHECK-NEXT:       DeclareGlobalVarInst "testComplex": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %testAnd(): any
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "testAnd": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %testOr(): any
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "testOr": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %testNullish(): any
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "testNullish": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %testComplex(): any
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "testComplex": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function testAnd(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "foo": string
// CHECK-NEXT:       CondBranchInst %5: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:       StorePropertyLooseInst %7: any, %4: any, "foo": string
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = PhiInst (:any) %5: any, %BB0, %7: any, %BB1
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end

// CHECK:function testOr(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "foo": string
// CHECK-NEXT:       CondBranchInst %5: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:       StorePropertyLooseInst %7: any, %4: any, "foo": string
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = PhiInst (:any) %5: any, %BB0, %7: any, %BB2
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end

// CHECK:function testNullish(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "foo": string
// CHECK-NEXT:  %6 = BinaryEqualInst (:any) %5: any, null: null
// CHECK-NEXT:       CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:       StorePropertyLooseInst %8: any, %4: any, "foo": string
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = PhiInst (:any) %5: any, %BB0, %8: any, %BB1
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:function testComplex(x: any, y: any, z: any): any
// CHECK-NEXT:frame = [x: any, y: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %z: any
// CHECK-NEXT:       StoreFrameInst %4: any, [z]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:       CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:        StoreStackInst %9: any, %8: any
// CHECK-NEXT:        CondBranchInst %9: any, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = PhiInst (:any) %6: any, %BB0, %17: any, %BB3
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [z]: any
// CHECK-NEXT:        StoreStackInst %14: any, %8: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %17 = LoadStackInst (:any) %8: any
// CHECK-NEXT:        StoreFrameInst %17: any, [x]: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end
