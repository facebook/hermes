/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function f1() {
  return this;
}

function f2(){
  "use strict"; // see strict mode
  return this;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [f1, f2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %f1()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "f1" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %f2()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "f2" : string
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f1()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst %this
// CHECK-NEXT:function_end

// CHECK:function f2()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst %this
// CHECK-NEXT:function_end
