/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -fno-inline -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

'use strict';

// Multiple variable stores.
// Can't resolve the call.
function main() {
  var x;
  x = function f() {};
  if (flag) x = function g() {};
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

// CHECK:function main(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %f(): undefined
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) globalObject: object, "flag": string
// CHECK-NEXT:  %2 = CondBranchInst %1: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CreateFunctionInst (:closure) %g(): undefined
// CHECK-NEXT:  %4 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = PhiInst (:closure) %3: closure, %BB1, %0: closure, %BB0
// CHECK-NEXT:  %6 = CallInst (:any) %5: closure, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %7 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function g(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
