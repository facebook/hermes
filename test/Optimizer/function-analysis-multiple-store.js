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

// CHECK:function global(): string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %main(): undefined
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1: object, globalObject: object, "main": string
// CHECK-NEXT:  %3 = ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function main(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %f(): undefined
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) globalObject: object, "flag": string
// CHECK-NEXT:  %2 = CondBranchInst %1: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %g(): undefined
// CHECK-NEXT:  %4 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = PhiInst (:object) %3: object, %BB1, %0: object, %BB0
// CHECK-NEXT:  %6 = CallInst (:any) %5: object, empty: any, empty: any, undefined: undefined, undefined: undefined
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
