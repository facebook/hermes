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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "test_one": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "test_two": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "test_three": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:closure) %test_one(): any
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3: closure, globalObject: object, "test_one": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:closure) %test_two(): any
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5: closure, globalObject: object, "test_two": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:closure) %test_three(): any
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7: closure, globalObject: object, "test_three": string
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %10 = StoreStackInst undefined: undefined, %9: any
// CHECK-NEXT:  %11 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %12 = ReturnInst (:any) %11: any
// CHECK-NEXT:function_end

// CHECK:function test_one(x: any, y: any, z: any): any
// CHECK-NEXT:frame = [x: any, y: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [z]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %7 = CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [z]: any
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = PhiInst (:any) %10: any, %BB1, %8: any, %BB2
// CHECK-NEXT:  %13 = ReturnInst (:any) %12: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_two(): any
// CHECK-NEXT:frame = [stop: any, age: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [stop]: any
// CHECK-NEXT:  %1 = StoreFrameInst undefined: undefined, [age]: any
// CHECK-NEXT:  %2 = StoreFrameInst false: boolean, [stop]: any
// CHECK-NEXT:  %3 = StoreFrameInst 16: number, [age]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [age]: any
// CHECK-NEXT:  %5 = BinaryGreaterThanInst (:any) %4: any, 18: number
// CHECK-NEXT:  %6 = CondBranchInst %5: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = StoreFrameInst true: boolean, [stop]: any
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = StoreFrameInst 2: number, [age]: any
// CHECK-NEXT:  %10 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = PhiInst (:any) 2: number, %BB1, true: boolean, %BB2
// CHECK-NEXT:  %12 = ReturnInst (:any) %11: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_three(x: any, one: any, two: any): any
// CHECK-NEXT:frame = [x: any, one: any, two: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %one: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [one]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %two: any
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [two]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %7 = CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [two]: any
// CHECK-NEXT:  %9 = CallInst (:any) %8: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %10 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [one]: any
// CHECK-NEXT:  %12 = CallInst (:any) %11: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %13 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = PhiInst (:any) %12: any, %BB1, %9: any, %BB2
// CHECK-NEXT:  %15 = ReturnInst (:any) %14: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %16 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end
