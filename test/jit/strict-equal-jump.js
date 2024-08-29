/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -Xforce-jit -Xdump-jitcode -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function equal(x, y) {
  if (x === y) {
    return 123;
  } else {
    return 456;
  }
}

print(equal(1, 1));
// CHECK: JIT successfully compiled FunctionID 1, 'equal'
// CHECK-NEXT: 123
print(equal(1, 2));
// CHECK-NEXT: 456
print(equal('a', 'a'));
// CHECK-NEXT: 123
print(equal('a', 'b'));
// CHECK-NEXT: 456
