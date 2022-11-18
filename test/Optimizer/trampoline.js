/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheckOrRegen %s --match-full-lines

// Make sure we can remove all trampolines from our code.
function test_one(x,y,z) {
  if (x < y) {
    return x;
  } else {
    if (y > 0) {}
  }
}

function test_catch_region(x,y,z) {
  try { } catch (e) { }
}

function test_cond_branch(x, y) {
  if (true) {
    return x;
  } else {
    return y;
  }
}

function test_merge_blocks(x, y) {
  x + y
  if (true) {}
  x + y
  if (true) {}
  x + y
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [test_one, test_catch_region, test_cond_branch, test_merge_blocks]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %test_one()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "test_one" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %test_catch_region() : undefined
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "test_catch_region" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %test_cond_branch()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "test_cond_branch" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %test_merge_blocks() : undefined
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "test_merge_blocks" : string
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_one(x, y, z)
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BinaryOperatorInst '<', %x, %y
// CHECK-NEXT:  %1 = CondBranchInst %0 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst %x
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = BinaryOperatorInst '>', %y, 0 : number
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_catch_region(x, y, z) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = CatchInst
// CHECK-NEXT:  %2 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %5 = TryEndInst
// CHECK-NEXT:  %6 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function test_cond_branch(x, y)
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst %x
// CHECK-NEXT:function_end

// CHECK:function test_merge_blocks(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BinaryOperatorInst '+', %x, %y
// CHECK-NEXT:  %1 = BinaryOperatorInst '+', %x, %y
// CHECK-NEXT:  %2 = BinaryOperatorInst '+', %x, %y
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
