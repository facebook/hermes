/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Don't inline recursive functions.

'use strict'
(function main() {
  function f() {
    return f();
  }
});

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): closure [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %main(): undefined
// CHECK-NEXT:  %1 = ReturnInst %0: closure
// CHECK-NEXT:function_end

// CHECK:function main(): undefined
// CHECK-NEXT:frame = [f: closure]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %f(): any
// CHECK-NEXT:  %1 = StoreFrameInst %0: closure, [f]: closure
// CHECK-NEXT:  %2 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f(): any [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:closure) [f@main]: closure
// CHECK-NEXT:  %1 = CallInst (:any) %0: closure, %f(): any, empty: any, undefined: undefined
// CHECK-NEXT:  %2 = ReturnInst %1: any
// CHECK-NEXT:function_end
