/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s  -custom-opt="simplestackpromotion" -custom-opt="mem2reg" -custom-opt="dce" | %FileCheckOrRegen %s --match-full-lines

function test_two(x,y,z) {
  function test00() {}
  var test01 = function() {}
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "test_two": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %test_two(): any
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "test_two": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_two(x: any, y: any, z: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
