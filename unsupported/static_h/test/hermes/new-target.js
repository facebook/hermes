/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

function foo() {
    print(typeof new.target, new.target === foo);
}

function bar() {
    return () => new.target;
}

print("start");
//CHECK: start

foo();
//CHECK-NEXT: undefined false
new foo();
//CHECK-NEXT: function true

var tmp = bar()();
print(typeof tmp, tmp === bar);
//CHECK-NEXT: undefined false
var tmp = (new bar())();
print(typeof tmp, tmp === bar);
//CHECK-NEXT: function true
