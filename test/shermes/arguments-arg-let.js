/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// Test that "arguments" in a paramater initializer expression is resolved
// correctly and doesn't clash with "let arguments" in the body of the function.

function foo(a=arguments[1]) {
    let arguments;
    print(a, arguments);
}
foo(undefined, 20);
//CHECK: 20 undefined
