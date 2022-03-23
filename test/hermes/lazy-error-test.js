/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

function foo() {
    break;
}

function bar() {
    foo();
}

print("main");
// CHECK: main

try {
    bar();
} catch(e) {
    print("caught", e);
}
// CHECK-NEXT: caught SyntaxError: 11:5:'break' not within a loop or a switch

print("end");
// CHECK-NEXT: end
