/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function bar() { return 1 }

//CHECK: function continue_test(cond)
//CHECK: frame = [cond]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %cond, [cond]
//CHECK:     %1 = BranchInst %BB1
//CHECK:   %BB2:
//CHECK:     %2 = BranchInst %BB3
//CHECK:   %BB4:
//CHECK:     %3 = ReturnInst undefined : undefined
//CHECK:   %BB1:
//CHECK:     %4 = LoadFrameInst [cond]
//CHECK:     %5 = CondBranchInst %4, %BB2, %BB4
//CHECK:   %BB5:
//CHECK:     %6 = LoadFrameInst [cond]
//CHECK:     %7 = CondBranchInst %6, %BB2, %BB4
//CHECK:   %BB3:
//CHECK:     %8 = BranchInst %BB5
//CHECK:   %BB6:
//CHECK:     %9 = BranchInst %BB3
function continue_test(cond) {
  while (cond) { continue; }
}

//CHECK: function break_test(cond)
//CHECK: frame = [cond]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %cond, [cond]
//CHECK:     %1 = BranchInst %BB1
//CHECK:   %BB2:
//CHECK:     %2 = BranchInst %BB3
//CHECK:   %BB3:
//CHECK:     %3 = ReturnInst undefined : undefined
//CHECK:   %BB1:
//CHECK:     %4 = LoadFrameInst [cond]
//CHECK:     %5 = CondBranchInst %4, %BB2, %BB3
//CHECK:   %BB4:
//CHECK:     %6 = LoadFrameInst [cond]
//CHECK:     %7 = CondBranchInst %6, %BB2, %BB3
//CHECK:   %BB5:
//CHECK:     %8 = BranchInst %BB4
//CHECK:   %BB6:
//CHECK:     %9 = BranchInst %BB5
function break_test(cond) {
  while (cond) { break; }
}

//CHECK: function break_label(cond)
//CHECK: frame = [cond]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %cond, [cond]
//CHECK:     %1 = BranchInst %BB1
//CHECK:   %BB2:
//CHECK:     %2 = ReturnInst undefined : undefined
//CHECK:   %BB3:
//CHECK:     %3 = BranchInst %BB4
//CHECK:   %BB4:
//CHECK:     %4 = BranchInst %BB2
//CHECK:   %BB1:
//CHECK:     %5 = LoadFrameInst [cond]
//CHECK:     %6 = CondBranchInst %5, %BB3, %BB4
//CHECK:   %BB5:
//CHECK:     %7 = LoadFrameInst [cond]
//CHECK:     %8 = CondBranchInst %7, %BB3, %BB4
//CHECK:   %BB6:
//CHECK:     %9 = BranchInst %BB5
//CHECK:   %BB7:
//CHECK:     %10 = BranchInst %BB6
function break_label(cond) {
  fail:
  while (cond) { break fail; }
}

//CHECK: function continue_label(cond)
//CHECK: frame = [cond]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %cond, [cond]
//CHECK:     %1 = BranchInst %BB1
//CHECK:   %BB2:
//CHECK:     %2 = ReturnInst undefined : undefined
//CHECK:   %BB3:
//CHECK:     %3 = BranchInst %BB4
//CHECK:   %BB5:
//CHECK:     %4 = BranchInst %BB2
//CHECK:   %BB1:
//CHECK:     %5 = LoadFrameInst [cond]
//CHECK:     %6 = CondBranchInst %5, %BB3, %BB5
//CHECK:   %BB6:
//CHECK:     %7 = LoadFrameInst [cond]
//CHECK:     %8 = CondBranchInst %7, %BB3, %BB5
//CHECK:   %BB4:
//CHECK:     %9 = BranchInst %BB6
//CHECK:   %BB7:
//CHECK:     %10 = BranchInst %BB4
function continue_label(cond) {
  fail:
  while (cond) { continue fail; }
}


//CHECK: function nested_label(cond)
//CHECK: frame = [cond]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %cond, [cond]
//CHECK:     %1 = BranchInst %BB1
//CHECK:   %BB2:
//CHECK:     %2 = ReturnInst undefined : undefined
//CHECK:   %BB3:
//CHECK:     %3 = BranchInst %BB4
//CHECK:   %BB5:
//CHECK:     %4 = BranchInst %BB2
//CHECK:   %BB1:
//CHECK:     %5 = LoadFrameInst [cond]
//CHECK:     %6 = CondBranchInst %5, %BB3, %BB5
//CHECK:   %BB6:
//CHECK:     %7 = LoadFrameInst [cond]
//CHECK:     %8 = CondBranchInst %7, %BB3, %BB5
//CHECK:   %BB7:
//CHECK:     %9 = BranchInst %BB6
//CHECK:   %BB8:
//CHECK:     %10 = BranchInst %BB7
//CHECK:   %BB9:
//CHECK:     %11 = BranchInst %BB10
//CHECK:   %BB11:
//CHECK:     %12 = BranchInst %BB8
//CHECK:   %BB4:
//CHECK:     %13 = LoadFrameInst [cond]
//CHECK:     %14 = CondBranchInst %13, %BB9, %BB11
//CHECK:   %BB12:
//CHECK:     %15 = LoadFrameInst [cond]
//CHECK:     %16 = CondBranchInst %15, %BB9, %BB11
//CHECK:   %BB10:
//CHECK:     %17 = BranchInst %BB12
//CHECK:   %BB13:
//CHECK:     %18 = BranchInst %BB10
function nested_label(cond) {
fail1:
  while (cond) {

fail2:
    while (cond) { continue fail2; }
  }
}



