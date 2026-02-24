/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes --test262 -exec %s | %FileCheck --match-full-lines %s

function foo(a = b, b) {
    print(a);
}
try {
    foo(undefined, 20);
} catch (e) {
    print(e)
}
//CHECK: ReferenceError: accessing an uninitialized variable
