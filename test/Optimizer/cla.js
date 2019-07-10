// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -enable-cla -dump-ir %s -O -fno-inline | %FileCheck %s --match-full-lines

"use strict";

//CHECK-LABEL:function g16() : number
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateFunctionInst %w() : number
//CHECK-NEXT:  %1 = CreateFunctionInst %g() : closure
//CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined, %0 : closure
//CHECK-NEXT:  %3 = CallInst %0 : closure, undefined : undefined
//CHECK-NEXT:  %4 = ReturnInst 2 : number
//CHECK-NEXT:function_end
function g16() {
    var w = function () { return 1; };
    var g = function (ff) { return ff ; }
    var v2 = g(w);
    var z2 = v2() + 1;
    return z2;
}

//CHECK-LABEL:function g17(c) : number
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = CreateFunctionInst %v() : number
//CHECK-NEXT:%1 = CreateFunctionInst %"w 1#"() : number
//CHECK-NEXT:%2 = CondBranchInst %c, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:%3 = BranchInst %BB1
//CHECK-NEXT:    %BB1:
//CHECK-NEXT:%4 = PhiInst %1 : closure, %BB2, %0 : closure, %BB0
//CHECK-NEXT:%5 = CallInst %4 : closure, undefined : undefined
//CHECK-NEXT:%6 = BinaryOperatorInst '+', %5 : number, 1 : number
//CHECK-NEXT:%7 = ReturnInst %6 : number
//CHECK-NEXT:function_end
function g17(c) {
    var v = function () { return 1; };
    var w = function () { return 2; };
    var u;
    if (c) {
        u = v;
    } else {
        u = w;
    }
    var z = u() + 1;
    return z;
}

//CHECK-LABEL:function ""(a : number, b : string, c : boolean) : string
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = BinaryOperatorInst '+', 1 : number, "hello" : string
//CHECK-NEXT:%1 = BinaryOperatorInst '+', %0 : string, false : boolean
//CHECK-NEXT:%2 = ReturnInst %1 : string
//CHECK-NEXT:function_end
function g18() {
    function foo(x) {
        return x;
    }
    function bar(z) {
        return z;
    }
    function baz(y) {
        return y;
    }
    function getSectionFragment() {
        return function (a, b, c) {
            return a + b + c;
        }(foo(1), bar("hello"), baz(false));
    }
    return getSectionFragment;
}
