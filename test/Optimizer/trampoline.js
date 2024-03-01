/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s     -O | %FileCheckOrRegen %s --match-full-lines

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

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "test_one": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_catch_region": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_cond_branch": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_merge_blocks": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %test_one(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "test_one": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %test_catch_region(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "test_catch_region": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %test_cond_branch(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "test_cond_branch": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %test_merge_blocks(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "test_merge_blocks": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_one(x: any, y: any, z: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = BinaryLessThanInst (:boolean) %0: any, %1: any
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst %0: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = BinaryGreaterThanInst (:boolean) %1: any, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_catch_region(x: any, y: any, z: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = CatchInst (:any)
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:       TryEndInst
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function test_cond_branch(x: any, y: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       ReturnInst %0: any
// CHECK-NEXT:function_end

// CHECK:function test_merge_blocks(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = BinaryAddInst (:string|number|bigint) %0: any, %1: any
// CHECK-NEXT:  %3 = BinaryAddInst (:string|number|bigint) %0: any, %1: any
// CHECK-NEXT:  %4 = BinaryAddInst (:string|number|bigint) %0: any, %1: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
