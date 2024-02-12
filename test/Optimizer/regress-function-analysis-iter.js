/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

'use strict'
(function main() {
  // FunctionAnalysis was crashing because the users list was changing
  // as we iterated the users of a function to analyze callsites.
  function f1() {
    return f2(f1);
  }
  function f2(cb) {
    if (a) {
      return b ? 0 : cb();
    }
    return cb();
  }
  function f4() {
    f1();
  }
  function f5() {
    f1();
  }
  function f6() {
    f4();
  }
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %main(): functionCode
// CHECK-NEXT:  %1 = CallInst (:undefined) %0: object, %main(): functionCode, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function main(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = [f1: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %f1(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: object, [f1]: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f1(): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:object) [f1@main]: object
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) globalObject: object, "a": string
// CHECK-NEXT:       CondBranchInst %1: any, %BB3, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = PhiInst (:number) %5: number, %BB2, %11: number, %BB5
// CHECK-NEXT:       ReturnInst %3: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = CallInst (:number) %0: object, %f1(): functionCode, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "b": string
// CHECK-NEXT:       CondBranchInst %7: any, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = CallInst (:number) %0: object, %f1(): functionCode, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = PhiInst (:number) %9: number, %BB4, 0: number, %BB3
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end
