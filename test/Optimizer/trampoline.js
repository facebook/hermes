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
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "test_one" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "test_catch_region" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "test_cond_branch" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "test_merge_blocks" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %test_one()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "test_one" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %test_catch_region() : undefined
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "test_catch_region" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %test_cond_branch()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "test_cond_branch" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %test_merge_blocks() : undefined
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "test_merge_blocks" : string
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_one(x, y, z)
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = LoadParamInst %y
// CHECK-NEXT:  %2 = BinaryOperatorInst '<', %0, %1
// CHECK-NEXT:  %3 = CondBranchInst %2 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst %0
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = BinaryOperatorInst '>', %1, 0 : number
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
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
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = ReturnInst %0
// CHECK-NEXT:function_end

// CHECK:function test_merge_blocks(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = LoadParamInst %y
// CHECK-NEXT:  %2 = BinaryOperatorInst '+', %0, %1
// CHECK-NEXT:  %3 = BinaryOperatorInst '+', %0, %1
// CHECK-NEXT:  %4 = BinaryOperatorInst '+', %0, %1
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
