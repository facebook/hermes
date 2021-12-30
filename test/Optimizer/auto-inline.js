/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O -dump-ir %s | %FileCheck --match-full-lines %s

function foo1(a) {
    var add = function() {
        return 100;
    }
    return add(a, 10);
}
//CHECK-LABEL:function foo1(a) : number
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = ReturnInst 100 : number
//CHECK-NEXT:function_end

function foo2(a) {
    var add = function(a, b) {
        return a + b;
    }
    return add(a, 10);
}
//CHECK-LABEL:function foo2(a) : string|number
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = BinaryOperatorInst '+', %a, 10 : number
//CHECK-NEXT:  %1 = ReturnInst %0 : string|number
//CHECK-NEXT:function_end

function foo3(a) {
    var add = function(a, b) {
        return a ? a : b;
    }
    return add(a, 10);
}
//CHECK-LABEL:function foo3(a)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CondBranchInst %a, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %1 = PhiInst %a, %BB1, 10 : number, %BB0
//CHECK-NEXT:  %2 = ReturnInst %1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %3 = BranchInst %BB2
//CHECK-NEXT:function_end

function foo4(a) {
    var add = function(a, b) {
        if (a < 0)
            return -1;
        if (a == 0)
            return b;
        return a;
    }
    return add(a, 10);
}
//CHECK-LABEL:function foo4(a)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = BinaryOperatorInst '<', %a, 0 : number
//CHECK-NEXT:  %1 = CondBranchInst %0 : boolean, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %2 = PhiInst 10 : number, %BB3, %a, %BB2, -1 : number, %BB0
//CHECK-NEXT:  %3 = ReturnInst %2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %4 = BinaryOperatorInst '==', %a, 0 : number
//CHECK-NEXT:  %5 = CondBranchInst %4 : boolean, %BB3, %BB1
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %6 = BranchInst %BB1
//CHECK-NEXT:function_end
