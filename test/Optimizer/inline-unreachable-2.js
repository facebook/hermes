/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

"use strict";

(function main() {
  function f(p){
    return p;
  }
  function baz() {
    throw 1;
    return f(5);
  }
  return baz();
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %main(): undefined
// CHECK-NEXT:  %1 = CallInst (:undefined) %0: object, %main(): undefined, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function main(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ThrowInst 1: number
// CHECK-NEXT:function_end
