/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [test_one, test_two, test_three]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %test_one#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "test_one" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %test_two#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "test_two" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %test_three#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "test_three" : string
// CHECK-NEXT:  %7 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %8 = StoreStackInst undefined : undefined, %7
// CHECK-NEXT:  %9 = LoadStackInst %7
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:function_end

// CHECK:function test_one#0#1(x, y, z)#2
// CHECK-NEXT:frame = [x#2, y#2, z#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_one#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst %z, [z#2], %0
// CHECK-NEXT:  %4 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = LoadFrameInst [z#2], %0
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = PhiInst %8, %BB1, %6, %BB2
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_two#0#1()#3
// CHECK-NEXT:frame = [stop#3, age#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_two#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [stop#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [age#3], %0
// CHECK-NEXT:  %3 = StoreFrameInst false : boolean, [stop#3], %0
// CHECK-NEXT:  %4 = StoreFrameInst 16 : number, [age#3], %0
// CHECK-NEXT:  %5 = LoadFrameInst [age#3], %0
// CHECK-NEXT:  %6 = BinaryOperatorInst '>', %5, 18 : number
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = StoreFrameInst true : boolean, [stop#3], %0
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = StoreFrameInst 2 : number, [age#3], %0
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = PhiInst 2 : number, %BB1, true : boolean, %BB2
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_three#0#1(x, one, two)#4
// CHECK-NEXT:frame = [x#4, one#4, two#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_three#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#4], %0
// CHECK-NEXT:  %2 = StoreFrameInst %one, [one#4], %0
// CHECK-NEXT:  %3 = StoreFrameInst %two, [two#4], %0
// CHECK-NEXT:  %4 = LoadFrameInst [x#4], %0
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = LoadFrameInst [two#4], %0
// CHECK-NEXT:  %7 = CallInst %6, undefined : undefined
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = LoadFrameInst [one#4], %0
// CHECK-NEXT:  %10 = CallInst %9, undefined : undefined
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = PhiInst %10, %BB1, %7, %BB2
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
