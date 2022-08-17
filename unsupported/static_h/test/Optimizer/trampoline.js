/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheck %s --match-full-lines


//CHECK-LABEL:function test_one(x, y, z)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = BinaryOperatorInst '<', %x, %y
//CHECK-NEXT:  %1 = CondBranchInst %0 : boolean, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %2 = ReturnInst %x
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %3 = BinaryOperatorInst '>', %y, 0 : number
//CHECK-NEXT:  %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

// Make sure we can remove all trampolines from our code.
function test_one(x,y,z) {
  if (x < y) {
    return x;
  } else {
    if (y > 0) {}
  }
}

//CHECK-LABEL:function test_catch_region(x, y, z) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = TryStartInst %BB1, %BB2
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %1 = CatchInst
//CHECK-NEXT:    %2 = BranchInst %BB3
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %3 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %4 = BranchInst %BB4
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %5 = TryEndInst
//CHECK-NEXT:    %6 = BranchInst %BB3
//CHECK-NEXT:function_end
function test_catch_region(x,y,z) {
  try { } catch (e) { }
}


//CHECK-LABEL:function test_cond_branch(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = ReturnInst %x
//CHECK-NEXT:function_end
function test_cond_branch(x, y) {
  if (true) {
    return x;
  } else {
    return y;
  }
}

//CHECK-LABEL:function test_merge_blocks(x, y) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = BinaryOperatorInst '+', %x, %y
//CHECK-NEXT:    %1 = BinaryOperatorInst '+', %x, %y
//CHECK-NEXT:    %2 = BinaryOperatorInst '+', %x, %y
//CHECK-NEXT:    %3 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function test_merge_blocks(x, y) {
  x + y
  if (true) {}
  x + y
  if (true) {}
  x + y
}

