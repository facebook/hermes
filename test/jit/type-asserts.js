/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xjit=force -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xjit=force -Xjit-crash-on-error -Xjit-emit-type-asserts %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function addNumbers(a, b) {
  var sum = 0;
  for (var i = 0; i < 100; ++i)
    sum += a * b - i;
  return sum;
}

print(addNumbers(3, 4));
// CHECK: -3750
