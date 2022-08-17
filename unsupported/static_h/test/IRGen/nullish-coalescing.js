/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck %s --match-full-lines

function f1(a, b) {
  return a ?? b;
}

// CHECK-LABEL: function f1(a, b)
// CHECK-NEXT: frame = [a, b]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:   %1 = StoreFrameInst %b, [b]
// CHECK-NEXT:   %2 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:   %3 = LoadFrameInst [a]
// CHECK-NEXT:   %4 = StoreStackInst %3, %2
// CHECK-NEXT:   %5 = BinaryOperatorInst '==', %3, null : null
// CHECK-NEXT:   %6 = CondBranchInst %5, %BB1, %BB2
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %7 = LoadFrameInst [b]
// CHECK-NEXT:   %8 = StoreStackInst %7, %2
// CHECK-NEXT:   %9 = BranchInst %BB2
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %10 = LoadStackInst %2
// CHECK-NEXT:   %11 = ReturnInst %10
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %12 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

function f2(a, b) {
  if (a ?? b) {
    return 1;
  } else {
    return 2;
  }
}

// CHECK-LABEL: function f2(a, b)
// CHECK-NEXT: frame = [a, b]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:   %1 = StoreFrameInst %b, [b]
// CHECK-NEXT:   %2 = LoadFrameInst [a]
// CHECK-NEXT:   %3 = BinaryOperatorInst '==', %2, null : null
// CHECK-NEXT:   %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %5 = ReturnInst 1 : number
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %6 = ReturnInst 2 : number
// CHECK-NEXT: %BB5:
// CHECK-NEXT:   %7 = ReturnInst undefined : undefined
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %8 = LoadFrameInst [b]
// CHECK-NEXT:   %9 = CondBranchInst %8, %BB3, %BB4
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %10 = CondBranchInst %2, %BB3, %BB4
// CHECK-NEXT: %BB6:
// CHECK-NEXT:   %11 = BranchInst %BB5
// CHECK-NEXT: %BB7:
// CHECK-NEXT:   %12 = BranchInst %BB5
// CHECK-NEXT: function_end
