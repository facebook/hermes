/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -fno-inline -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

'use strict';

// Function is threaded through multiple variables.
// Note that the call in 'bar' is correctly resolved.
function main() {
  var x = () => 1;
  function foo() {
    var y = x;
    function bar() {
      var z = y;
      z();
    }
    return bar;
  }
  foo();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %main(): undefined
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1: object, globalObject: object, "main": string
// CHECK-NEXT:  %3 = ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function main(): undefined
// CHECK-NEXT:frame = [x: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %foo(): object
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %x(): number
// CHECK-NEXT:  %2 = StoreFrameInst %1: object, [x]: object
// CHECK-NEXT:  %3 = CallInst (:object) %0: object, %foo(): object, empty: any, undefined: undefined
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = [y: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %bar(): undefined
// CHECK-NEXT:  %1 = LoadFrameInst (:object) [x@main]: object
// CHECK-NEXT:  %2 = StoreFrameInst %1: object, [y]: object
// CHECK-NEXT:  %3 = ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:arrow x(): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:function bar(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:object) [y@foo]: object
// CHECK-NEXT:  %1 = CallInst (:any) %0: object, %x(): number, empty: any, undefined: undefined
// CHECK-NEXT:  %2 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
