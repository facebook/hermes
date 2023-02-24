/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -fno-inline -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

'use strict';

// Single variable store.
function main(x) {
  x = function f() {};
  x();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): string [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %main(): undefined
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1: closure, globalObject: object, "main": string
// CHECK-NEXT:  %3 = ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function main(x: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %f(): undefined
// CHECK-NEXT:  %1 = CallInst (:undefined) %0: closure, %f(): undefined, empty: any, undefined: undefined
// CHECK-NEXT:  %2 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
