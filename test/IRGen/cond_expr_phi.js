/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -dump-ir %s -O

function condExpr(a,b,c,d) {
    return a ? b || c : d;
}
//CHECK-LABEL: function condExpr(a, b, c, d)
//CHECK-NEXT: frame = [a, b, c, d]
//CHECK-NEXT:   %BB0:
//CHECK-NEXT:     %0 = StoreFrameInst %a, [a]
//CHECK-NEXT:     %1 = StoreFrameInst %b, [b]
//CHECK-NEXT:     %2 = StoreFrameInst %c, [c]
//CHECK-NEXT:     %3 = StoreFrameInst %d, [d]
//CHECK-NEXT:     %4 = LoadFrameInst [a]
//CHECK-NEXT:     %5 = CondBranchInst %4, %BB1, %BB2
//CHECK-NEXT:   %BB2:
//CHECK-NEXT:     %6 = LoadFrameInst [d]
//CHECK-NEXT:     %7 = BranchInst %BB3
//CHECK-NEXT:   %BB1:
//CHECK-NEXT:     %8 = AllocStackInst $?anon_0_logical
//CHECK-NEXT:     %9 = LoadFrameInst [b]
//CHECK-NEXT:     %10 = StoreStackInst %9, %8
//CHECK-NEXT:     %11 = CondBranchInst %9, %BB4, %BB5
//CHECK-NEXT:   %BB3:
//CHECK-NEXT:     %12 = PhiInst %17, %BB4, %6, %BB2
//CHECK-NEXT:     %13 = ReturnInst %12
//CHECK-NEXT:   %BB5:
//CHECK-NEXT:     %14 = LoadFrameInst [c]
//CHECK-NEXT:     %15 = StoreStackInst %14, %8
//CHECK-NEXT:     %16 = BranchInst %BB4
//CHECK-NEXT:   %BB4:
//CHECK-NEXT:     %17 = LoadStackInst %8
//CHECK-NEXT:     %18 = BranchInst %BB3
//CHECK-NEXT:   %BB6:
//CHECK-NEXT:     %19 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
