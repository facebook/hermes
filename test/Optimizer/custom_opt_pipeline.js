/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s  -custom-opt="stackpromotion" -custom-opt="mem2reg" -custom-opt="dce" | %FileCheckOrRegen %s --match-full-lines

function test_two(x,y,z) {
  function test00() {}
  var test01 = function() {}
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [test_two]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %test_two()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "test_two" : string
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_two(x, y, z)
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
