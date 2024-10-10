/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -Xforce-jit -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function *foo(x, y) {
  yield x;
  yield y;
}

var it = foo(1, 2);
print(it.next().value);
// CHECK: 1
print(it.next().value);
// CHECK: 2
print(it.next().done);
// CHECK: true
