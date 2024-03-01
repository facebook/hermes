/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

(function main() {
  'use strict';

  function foo() {
    throw 123;
  }

  function bar() {
    updateEventTarget();
  }

  return function updateEventTarget(x, y) {
    foo();

    // If there are multiple blocks which contain an unreachable Phi,
    // it's possible to delete unreachable operands of the Phi in other blocks
    // prior to deleting the block containing the Phi.
    // So the Phi has to handle nullptr operands for that period.
    if (x) {
      var child = globalThis.sink();
      if (y) {
        child.prop = 3;
      }
    } else {
      globalThis.sink();
    }
  }
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %main(): functionCode
// CHECK-NEXT:  %2 = CallInst (:object) %1: object, %main(): functionCode, %0: environment, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function main(): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %updateEventTarget(): functionCode
// CHECK-NEXT:       ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:function updateEventTarget(x: any, y: any): any [noReturn]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ThrowInst 123: number
// CHECK-NEXT:function_end
