/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// Test that "arguments" in an arrow function captures the surrounding
// "let arguments" declaration.

let arguments = 10;
let foo = () => { print(arguments); }
foo();
//CHECK: 10

function outer() {
    let arguments = 20;
    return () => arguments;
}
print(outer()());
// CHECK-NEXT: 20

let x = () => {
    let arguments = 30;
    return arguments;
}
print(x());
// CHECK-NEXT: 30
