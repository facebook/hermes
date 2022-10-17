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

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [test_one, test_catch_region, test_cond_branch, test_merge_blocks]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %test_one#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "test_one" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %test_catch_region#0#1()#3 : undefined, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "test_catch_region" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %test_cond_branch#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "test_cond_branch" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %test_merge_blocks#0#1()#5 : undefined, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "test_merge_blocks" : string
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_one#0#1(x, y, z)#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_one#0#1()#2}
// CHECK-NEXT:  %1 = BinaryOperatorInst '<', %x, %y
// CHECK-NEXT:  %2 = CondBranchInst %1 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst %x
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = BinaryOperatorInst '>', %y, 0 : number
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_catch_region#0#1(x, y, z)#3 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_catch_region#0#1()#3}
// CHECK-NEXT:  %1 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = CatchInst
// CHECK-NEXT:  %3 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %6 = TryEndInst
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function test_cond_branch#0#1(x, y)#4
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_cond_branch#0#1()#4}
// CHECK-NEXT:  %1 = ReturnInst %x
// CHECK-NEXT:function_end

// CHECK:function test_merge_blocks#0#1(x, y)#5 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_merge_blocks#0#1()#5}
// CHECK-NEXT:  %1 = BinaryOperatorInst '+', %x, %y
// CHECK-NEXT:  %2 = BinaryOperatorInst '+', %x, %y
// CHECK-NEXT:  %3 = BinaryOperatorInst '+', %x, %y
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
