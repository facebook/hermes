/**
 * Copyright (c) Facebook, Inc. and its affiliates.
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
