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

// CHECK:function global() : closure [allCallsitesKnown]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %main() : undefined
// CHECK-NEXT:  %1 = ReturnInst %0 : closure
// CHECK-NEXT:function_end

// CHECK:function main() : undefined
// CHECK-NEXT:frame = [f : closure]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %f()
// CHECK-NEXT:  %1 = StoreFrameInst %0 : closure, [f] : closure
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f() [allCallsitesKnown]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [f@main] : closure
// CHECK-NEXT:  %1 = CallInst %0 : closure, %f(), empty, undefined : undefined
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:function_end
