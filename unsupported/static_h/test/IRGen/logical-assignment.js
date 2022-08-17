/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck --match-full-lines %s

function testAnd(x, y) {
  return x.foo &&= y;
}
// CHECK-LABEL: function testAnd(x, y)
// CHECK-NEXT: frame = [x, y]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:   %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:   %2 = LoadFrameInst [x]
// CHECK-NEXT:   %3 = LoadPropertyInst %2, "foo" : string
// CHECK-NEXT:   %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %5 = LoadFrameInst [y]
// CHECK-NEXT:   %6 = StorePropertyInst %5, %2, "foo" : string
// CHECK-NEXT:   %7 = BranchInst %BB2
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %8 = PhiInst %3, %BB0, %5, %BB1
// CHECK-NEXT:   %9 = ReturnInst %8
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %10 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

function testOr(x, y) {
  return x.foo ||= y;
}
// CHECK-LABEL: function testOr(x, y)
// CHECK-NEXT: frame = [x, y]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:   %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:   %2 = LoadFrameInst [x]
// CHECK-NEXT:   %3 = LoadPropertyInst %2, "foo" : string
// CHECK-NEXT:   %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %5 = LoadFrameInst [y]
// CHECK-NEXT:   %6 = StorePropertyInst %5, %2, "foo" : string
// CHECK-NEXT:   %7 = BranchInst %BB1
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %8 = PhiInst %3, %BB0, %5, %BB2
// CHECK-NEXT:   %9 = ReturnInst %8
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %10 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

function testNullish(x, y) {
  return x.foo ??= y;
}
// CHECK-LABEL: function testNullish(x, y)
// CHECK-NEXT: frame = [x, y]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:   %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:   %2 = LoadFrameInst [x]
// CHECK-NEXT:   %3 = LoadPropertyInst %2, "foo" : string
// CHECK-NEXT:   %4 = BinaryOperatorInst '==', %3, null : null
// CHECK-NEXT:   %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %6 = LoadFrameInst [y]
// CHECK-NEXT:   %7 = StorePropertyInst %6, %2, "foo" : string
// CHECK-NEXT:   %8 = BranchInst %BB2
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %9 = PhiInst %3, %BB0, %6, %BB1
// CHECK-NEXT:   %10 = ReturnInst %9
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %11 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

function testComplex(x, y, z) {
  return x ||= y || z;
}
// CHECK-LABEL: function testComplex(x, y, z)
// CHECK-NEXT: frame = [x, y, z]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:   %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:   %2 = StoreFrameInst %z, [z]
// CHECK-NEXT:   %3 = LoadFrameInst [x]
// CHECK-NEXT:   %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %5 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:   %6 = LoadFrameInst [y]
// CHECK-NEXT:   %7 = StoreStackInst %6, %5
// CHECK-NEXT:   %8 = CondBranchInst %6, %BB3, %BB4
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %9 = PhiInst %3, %BB0, %14, %BB3
// CHECK-NEXT:   %10 = ReturnInst %9
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %11 = LoadFrameInst [z]
// CHECK-NEXT:   %12 = StoreStackInst %11, %5
// CHECK-NEXT:   %13 = BranchInst %BB3
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %14 = LoadStackInst %5
// CHECK-NEXT:   %15 = StoreFrameInst %14, [x]
// CHECK-NEXT:   %16 = BranchInst %BB1
// CHECK-NEXT: %BB5:
// CHECK-NEXT:   %17 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end
