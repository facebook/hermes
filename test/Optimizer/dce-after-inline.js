/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Demonstrate that "x" can be promoted into a stack variable and SSA after
// inlining because no references to it as a captured variable remain - they
// should all have been cleaned up by DCE.

(function () {
var x = 10;

function leaf() {
    x += x + 1;
}

function middle() {
    for(var i = 0; i < 5; ++i)
        leaf();
}

middle();
return x;

})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %""(): functionCode
// CHECK-NEXT:  %1 = CallInst (:number) %0: object, %""(): functionCode, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end

// CHECK:function ""(): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = PhiInst (:number) 10: number, %BB0, %5: number, %BB1
// CHECK-NEXT:  %3 = PhiInst (:number) 0: number, %BB0, %6: number, %BB1
// CHECK-NEXT:  %4 = FAddInst (:number) %2: number, 1: number
// CHECK-NEXT:  %5 = FAddInst (:number) %2: number, %4: number
// CHECK-NEXT:  %6 = FAddInst (:number) %3: number, 1: number
// CHECK-NEXT:  %7 = FLessThanInst (:boolean) %6: number, 5: number
// CHECK-NEXT:       CondBranchInst %7: boolean, %BB1, %BB2
// CHECK-NEXT:function_end
