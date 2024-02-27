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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %2 = CallInst (:number) %1: object, %""(): functionCode, %0: environment, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst %2: number
// CHECK-NEXT:function_end

// CHECK:function ""(): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 10: number
// CHECK-NEXT:function_end
