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
// CHECK-NEXT:       DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %main(): object
// CHECK-NEXT:       StorePropertyStrictInst %1: object, globalObject: object, "main": string
// CHECK-NEXT:       ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function main(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %retval(): undefined
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function retval(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ThrowInst 1: number
// CHECK-NEXT:function_end
