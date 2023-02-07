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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "test_one" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "test_two" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "test_three" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %test_one()
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3 : closure, globalObject : object, "test_one" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %test_two()
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5 : closure, globalObject : object, "test_two" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %test_three()
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7 : closure, globalObject : object, "test_three" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function test_one(x, y, z)
// CHECK-NEXT:frame = [x, y, z]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadParamInst %y
// CHECK-NEXT:  %3 = StoreFrameInst %2, [y]
// CHECK-NEXT:  %4 = LoadParamInst %z
// CHECK-NEXT:  %5 = StoreFrameInst %4, [z]
// CHECK-NEXT:  %6 = LoadFrameInst [x]
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadFrameInst [z]
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadFrameInst [y]
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = PhiInst %10, %BB1, %8, %BB2
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_two()
// CHECK-NEXT:frame = [stop, age]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [stop]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [age]
// CHECK-NEXT:  %2 = StoreFrameInst false : boolean, [stop]
// CHECK-NEXT:  %3 = StoreFrameInst 16 : number, [age]
// CHECK-NEXT:  %4 = LoadFrameInst [age]
// CHECK-NEXT:  %5 = BinaryOperatorInst '>', %4, 18 : number
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = StoreFrameInst true : boolean, [stop]
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = StoreFrameInst 2 : number, [age]
// CHECK-NEXT:  %10 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = PhiInst 2 : number, %BB1, true : boolean, %BB2
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_three(x, one, two)
// CHECK-NEXT:frame = [x, one, two]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadParamInst %one
// CHECK-NEXT:  %3 = StoreFrameInst %2, [one]
// CHECK-NEXT:  %4 = LoadParamInst %two
// CHECK-NEXT:  %5 = StoreFrameInst %4, [two]
// CHECK-NEXT:  %6 = LoadFrameInst [x]
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadFrameInst [two]
// CHECK-NEXT:  %9 = CallInst %8, empty, empty, undefined : undefined
// CHECK-NEXT:  %10 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = LoadFrameInst [one]
// CHECK-NEXT:  %12 = CallInst %11, empty, empty, undefined : undefined
// CHECK-NEXT:  %13 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = PhiInst %12, %BB1, %9, %BB2
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %16 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
