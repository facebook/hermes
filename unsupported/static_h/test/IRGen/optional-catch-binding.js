/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir %s | %FileCheck --match-full-lines %s

function foo(f, g) {
  try {
    return f();
  } catch {
    return g();
  }
}

// CHECK-LABEL: function foo(f, g)
// CHECK-NEXT: frame = [f, g]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %f, [f]
// CHECK-NEXT:   %1 = StoreFrameInst %g, [g]
// CHECK-NEXT:   %2 = TryStartInst %BB1, %BB2
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %3 = CatchInst
// CHECK-NEXT:   %4 = LoadFrameInst [g]
// CHECK-NEXT:   %5 = CallInst %4, undefined : undefined
// CHECK-NEXT:   %6 = ReturnInst %5
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %7 = ReturnInst undefined : undefined
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %8 = LoadFrameInst [f]
// CHECK-NEXT:   %9 = CallInst %8, undefined : undefined
// CHECK-NEXT:   %10 = BranchInst %BB4
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %11 = TryEndInst
// CHECK-NEXT:   %12 = ReturnInst %9
// CHECK-NEXT: %BB5:
// CHECK-NEXT:   %13 = BranchInst %BB6
// CHECK-NEXT: %BB6:
// CHECK-NEXT:   %14 = TryEndInst
// CHECK-NEXT:   %15 = BranchInst %BB3
// CHECK-NEXT: %BB7:
// CHECK-NEXT:   %16 = BranchInst %BB3
// CHECK-NEXT: function_end
