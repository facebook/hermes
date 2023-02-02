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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "testAnd" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "testOr" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "testNullish" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "testComplex" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %testAnd()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "testAnd" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %testOr()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "testOr" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %testNullish()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "testNullish" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %testComplex()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "testComplex" : string
// CHECK-NEXT:  %12 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %13 = StoreStackInst undefined : undefined, %12
// CHECK-NEXT:  %14 = LoadStackInst %12
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:function_end

// CHECK:function testAnd(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadParamInst %y
// CHECK-NEXT:  %3 = StoreFrameInst %2, [y]
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = LoadPropertyInst %4, "foo" : string
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = LoadFrameInst [y]
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7, %4, "foo" : string
// CHECK-NEXT:  %9 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = PhiInst %5, %BB0, %7, %BB1
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function testOr(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadParamInst %y
// CHECK-NEXT:  %3 = StoreFrameInst %2, [y]
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = LoadPropertyInst %4, "foo" : string
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadFrameInst [y]
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7, %4, "foo" : string
// CHECK-NEXT:  %9 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = PhiInst %5, %BB0, %7, %BB2
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function testNullish(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadParamInst %y
// CHECK-NEXT:  %3 = StoreFrameInst %2, [y]
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = LoadPropertyInst %4, "foo" : string
// CHECK-NEXT:  %6 = BinaryOperatorInst '==', %5, null : null
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = LoadFrameInst [y]
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8, %4, "foo" : string
// CHECK-NEXT:  %10 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = PhiInst %5, %BB0, %8, %BB1
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function testComplex(x, y, z)
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
// CHECK-NEXT:  %8 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %9 = LoadFrameInst [y]
// CHECK-NEXT:  %10 = StoreStackInst %9, %8
// CHECK-NEXT:  %11 = CondBranchInst %9, %BB3, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = PhiInst %6, %BB0, %17, %BB3
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = LoadFrameInst [z]
// CHECK-NEXT:  %15 = StoreStackInst %14, %8
// CHECK-NEXT:  %16 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %17 = LoadStackInst %8
// CHECK-NEXT:  %18 = StoreFrameInst %17, [x]
// CHECK-NEXT:  %19 = BranchInst %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
