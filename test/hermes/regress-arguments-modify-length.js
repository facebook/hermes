/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// We previously had a bug where we always treated the result of
// arguments.length as a number, even if it had been overwritten. Test that this
// is no longer the case.
function foo() {
    arguments.length = undefined;
    return arguments.length + 42;
}
print(foo());
//CHECK: NaN
