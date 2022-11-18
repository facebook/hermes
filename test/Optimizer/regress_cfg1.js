/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function f() {
  for (var j = 1; j < 1; j *= -8) {
  }
  for (var i = 1; i < 1; j += 2) {
    j * -1;
  }
}

f();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [f]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %f() : undefined
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "f" : string
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "f" : string
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:function_end

// CHECK:function f() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
