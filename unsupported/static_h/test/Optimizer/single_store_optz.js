/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O  | %FileCheck %s --match-full-lines

//CHECK-LABEL:function g12(z) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateFunctionInst %w() : number
//CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined
//CHECK-NEXT:  %2 = BinaryOperatorInst '>', %z, 0 : number
//CHECK-NEXT:  %3 = CondBranchInst %2 : boolean, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %4 = CallInst %0 : closure, undefined : undefined
//CHECK-NEXT:  %5 = BranchInst %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %6 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function g12(z) {
    var w = function () { return 1; }
    w();  // site 1
    if (z > 0) {
        w(); // site 2
    }
}

