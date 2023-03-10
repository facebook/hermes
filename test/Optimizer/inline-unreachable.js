/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

"use strict";

function main() {

  function foo() {
    return bar() || baz();
  }

  function baz() {
    throw 1;
  }

  function bar() {
  }

  function retval() {
     foo();
  };

  return retval;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %main(): closure
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1: closure, globalObject: object, "main": string
// CHECK-NEXT:  %3 = ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function main(): closure
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %retval(): undefined
// CHECK-NEXT:  %1 = ReturnInst %0: closure
// CHECK-NEXT:function_end

// CHECK:function retval(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ThrowInst 1: number
// CHECK-NEXT:function_end
