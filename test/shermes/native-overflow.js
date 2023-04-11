/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Restrict the native stack to ensure it overflows before the register stack.
// RUN: (ulimit -s 512 && ! %shermes -fcheck-native-stack -exec %s 2>&1) | %FileCheck --match-full-lines %s
// REQUIRES: check_native_stack

function foo() {
  return foo() + 1;
}
foo();

// CHECK: Uncaught RangeError: Maximum call stack size exceeded (native stack depth)
