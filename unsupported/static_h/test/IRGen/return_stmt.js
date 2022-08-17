/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

//CHECK: function test0(x, y)
//CHECK: frame = [x, y]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %x, [x]
//CHECK:     %1 = StoreFrameInst %y, [y]
//CHECK:     %2 = LoadFrameInst [x]
//CHECK:     %3 = CondBranchInst %2, %BB1, %BB2
//CHECK:   %BB1:
//CHECK:     %4 = LoadFrameInst [x]
//CHECK:     %5 = ReturnInst %4
//CHECK:   %BB2:
//CHECK:     %6 = LoadFrameInst [y]
//CHECK:     %7 = ReturnInst %6
//CHECK:   %BB3:
//CHECK:     %8 = LoadFrameInst [y]
//CHECK:     %9 = ReturnInst %8
//CHECK:   %BB4:
//CHECK:     %10 = BranchInst %BB3
//CHECK:   %BB5:
//CHECK:     %11 = BranchInst %BB3
//CHECK:   %BB6:
//CHECK:     %12 = ReturnInst undefined : undefined

function test0(x,  y) {
  if (x) { return x; } else { return y; }
  return y;
}

//CHECK: function test1(x, y)
//CHECK: frame = [x, y]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %x, [x]
//CHECK:     %1 = StoreFrameInst %y, [y]
//CHECK:     %2 = LoadFrameInst [x]
//CHECK:     %3 = CondBranchInst %2, %BB1, %BB2
//CHECK:   %BB1:
//CHECK:     %4 = BranchInst %BB3
//CHECK:   %BB2:
//CHECK:     %5 = LoadFrameInst [y]
//CHECK:     %6 = ReturnInst %5
//CHECK:   %BB3:
//CHECK:     %7 = ReturnInst undefined : undefined

function test1(x,  y) {
  if (x) { } else { return y; }
}


//CHECK: function test2(x, y)
//CHECK: frame = [x, y]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %x, [x]
//CHECK:     %1 = StoreFrameInst %y, [y]
//CHECK:     %2 = LoadFrameInst [x]
//CHECK:     %3 = CondBranchInst %2, %BB1, %BB2
//CHECK:   %BB1:
//CHECK:     %4 = LoadFrameInst [x]
//CHECK:     %5 = ReturnInst %4
//CHECK:   %BB2:
//CHECK:     %6 = BranchInst %BB3
//CHECK:   %BB3:
//CHECK:     %7 = ReturnInst undefined : undefined
//CHECK:   %BB4:
//CHECK:     %8 = BranchInst %BB3


function test2(x,  y) {
  if (x) { return x; } else {  }
}

//CHECK: function test3(x, y)
//CHECK: frame = [x, y]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %x, [x]
//CHECK:     %1 = StoreFrameInst %y, [y]
//CHECK:     %2 = LoadFrameInst [x]
//CHECK:     %3 = ReturnInst %2
//CHECK:   %BB1:
//CHECK:     %4 = LoadFrameInst [x]
//CHECK:     %5 = CondBranchInst %4, %BB2, %BB3
//CHECK:   %BB2:
//CHECK:     %6 = LoadFrameInst [x]
//CHECK:     %7 = ReturnInst %6
//CHECK:   %BB3:
//CHECK:     %8 = LoadFrameInst [x]
//CHECK:     %9 = ReturnInst %8
//CHECK:   %BB4:
//CHECK:     %10 = ReturnInst undefined : undefined
//CHECK:   %BB5:
//CHECK:     %11 = BranchInst %BB4
//CHECK:   %BB6:
//CHECK:     %12 = BranchInst %BB4

function test3(x,  y) {
  return x;
  if (x) { return x; } else { return x; }
}

//CHECK: function test4(x, y)
//CHECK: frame = [x, y]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %x, [x]
//CHECK:     %1 = StoreFrameInst %y, [y]
//CHECK:     %2 = LoadFrameInst [x]
//CHECK:     %3 = ReturnInst %2
//CHECK:   %BB1:
//CHECK:     %4 = LoadFrameInst [x]
//CHECK:     %5 = CondBranchInst %4, %BB2, %BB3
//CHECK:   %BB2:
//CHECK:     %6 = LoadFrameInst [x]
//CHECK:     %7 = ReturnInst %6
//CHECK:   %BB3:
//CHECK:     %8 = LoadFrameInst [x]
//CHECK:     %9 = ReturnInst %8
//CHECK:   %BB4:
//CHECK:     %10 = ReturnInst undefined : undefined
//CHECK:   %BB5:
//CHECK:     %11 = BranchInst %BB4
//CHECK:   %BB6:
//CHECK:     %12 = BranchInst %BB4

function test4(x,  y) {
  return x;
  if (x) { return x; } else { return x; }
}

//CHECK: function test5()
//CHECK: %BB0:
//CHECK:   %0 = ReturnInst undefined : undefined
function test5() {
  return;
}



