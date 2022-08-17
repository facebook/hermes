/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// Ensure that environments work.

function makeCounter(init, step) {
    init -= step;
    return function() {
        return init += step;
    }
}

var c1 = makeCounter(10,5);
var c2 = makeCounter(1,1);
print(c1());
//CHECK:10
print(c2());
//CHECK-NEXT:1
print(c1());
//CHECK-NEXT:15
print(c2());
//CHECK-NEXT:2
print(c1());
//CHECK-NEXT:20
print(c2());
//CHECK-NEXT:3


function outer(value) {
    return (function () {
        return function () {
            return ++value;
        }
    })();
}

var f = outer(100);
print(f());
//CHECK-NEXT: 101
print(f());
//CHECK-NEXT: 102
