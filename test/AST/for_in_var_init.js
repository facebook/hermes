/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

var global = Function("return this;")();

print("START");
//CHECK: START

function tryParse(str) {
    try {
        var result = global.eval(str);
    } catch (e) {
        print("caught:", e.message);
        return;
    }
    print("OK", result);
}

tryParse("for(var i1 = 10 in {}); i1");
//CHECK-NEXT: OK 10

tryParse("'use strict'; for(var i2 = 10 in {}); i2");
//CHECK-NEXT: caught: 1:28:for-in/for-of variable declaration may not be initialized

tryParse("for(let i3 = 10 in {}); i3");
//CHECK-NEXT: caught: 1:14:for-in/for-of variable declaration may not be initialized

tryParse("'use strict'; for(let i4 = 10 in {}); i4");
//CHECK-NEXT: caught: 1:28:for-in/for-of variable declaration may not be initialized

tryParse("for(let i5 = 10 of []); i5");
//CHECK-NEXT: caught: 1:14:for-in/for-of variable declaration may not be initialized

tryParse("var p1, [p2, p3] = [], p4;");
//CHECK-NEXT: OK undefined

tryParse("var p5, [p6, p7], p8;");
//CHECK-NEXT: caught: 1:9:destucturing declaration must be initialized

tryParse("for(var [a1] of [[1]]); a1");
//CHECK-NEXT: OK 1

tryParse("for(var [a2] = 0 of []);");
//CHECK-NEXT: caught: 1:16:destructuring declaration cannot be initialized in for-in/for-of loop

tryParse("for(var [a3] = 0 in []);");
//CHECK-NEXT: caught: 1:16:destructuring declaration cannot be initialized in for-in/for-of loop
