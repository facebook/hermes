/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen --match-full-lines %s

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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [testAnd, testOr, testNullish, testComplex]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %testAnd#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "testAnd" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %testOr#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "testOr" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %testNullish#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "testNullish" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %testComplex#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "testComplex" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function testAnd#0#1(x, y)#2
// CHECK-NEXT:frame = [x#2, y#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{testAnd#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#2], %0
// CHECK-NEXT:  %3 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %4 = LoadPropertyInst %3, "foo" : string
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %7 = StorePropertyInst %6, %3, "foo" : string
// CHECK-NEXT:  %8 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = PhiInst %4, %BB0, %6, %BB1
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function testOr#0#1(x, y)#3
// CHECK-NEXT:frame = [x#3, y#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{testOr#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#3], %0
// CHECK-NEXT:  %3 = LoadFrameInst [x#3], %0
// CHECK-NEXT:  %4 = LoadPropertyInst %3, "foo" : string
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = LoadFrameInst [y#3], %0
// CHECK-NEXT:  %7 = StorePropertyInst %6, %3, "foo" : string
// CHECK-NEXT:  %8 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = PhiInst %4, %BB0, %6, %BB2
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function testNullish#0#1(x, y)#4
// CHECK-NEXT:frame = [x#4, y#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{testNullish#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#4], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#4], %0
// CHECK-NEXT:  %3 = LoadFrameInst [x#4], %0
// CHECK-NEXT:  %4 = LoadPropertyInst %3, "foo" : string
// CHECK-NEXT:  %5 = BinaryOperatorInst '==', %4, null : null
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = LoadFrameInst [y#4], %0
// CHECK-NEXT:  %8 = StorePropertyInst %7, %3, "foo" : string
// CHECK-NEXT:  %9 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = PhiInst %4, %BB0, %7, %BB1
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function testComplex#0#1(x, y, z)#5
// CHECK-NEXT:frame = [x#5, y#5, z#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{testComplex#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#5], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#5], %0
// CHECK-NEXT:  %3 = StoreFrameInst %z, [z#5], %0
// CHECK-NEXT:  %4 = LoadFrameInst [x#5], %0
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %7 = LoadFrameInst [y#5], %0
// CHECK-NEXT:  %8 = StoreStackInst %7, %6
// CHECK-NEXT:  %9 = CondBranchInst %7, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = PhiInst %4, %BB0, %15, %BB3
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = LoadFrameInst [z#5], %0
// CHECK-NEXT:  %13 = StoreStackInst %12, %6
// CHECK-NEXT:  %14 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %15 = LoadStackInst %6
// CHECK-NEXT:  %16 = StoreFrameInst %15, [x#5], %0
// CHECK-NEXT:  %17 = BranchInst %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %18 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
