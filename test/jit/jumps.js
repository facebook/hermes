/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xjit=force -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function foo1(run, p) {
    if (p)
        print(run, "p is true");
    if (!p)
        print(run, "p is false");
}

function foo2(run, p) {
    p = +p;
    if (p)
        print(run, "p is true");
}

function foo3(run, p) {
    p = +p;
    if (!p)
        print(run, "p is false");
}

foo1(1, true);
foo1(2, false);
foo2(2, 1);
foo3(3, 0);
// CHECK: 1 p is true
// CHECK-NEXT: 2 p is false
// CHECK-NEXT: 2 p is true
// CHECK-NEXT: 3 p is false
