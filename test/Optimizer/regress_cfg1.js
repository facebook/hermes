/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function f() {
  for (var j = 1; j < 1; j *= -8) {
  }
  for (var i = 1; i < 1; j += 2) {
    j * -1;
  }
}

f();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "f": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %f(): undefined
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "f": string
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "f": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %5 = ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:function f(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
