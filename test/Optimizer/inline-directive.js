/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

(function() {
  // Marking this function as 'inline' results in the whole addition being done.
  function foo(x, y) {
    'inline'
    return x + y;
  }

  return foo(1, 2) + foo(3, 4);
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %""(): number
// CHECK-NEXT:  %1 = CallInst (:number) %0: object, %""(): number, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %2 = ReturnInst 10: number
// CHECK-NEXT:function_end

// CHECK:function ""(): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 10: number
// CHECK-NEXT:function_end
