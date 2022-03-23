/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --break-at-start %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

// Pausing on caught exceptions and evaluating code that throws a second
// exception should not affect the result of the program.
try {
  throw new Error("Foo");
} catch(e) {
  print("Message: " + e.message);
}

// CHECK: Break on script load in global: {{.*}}[{{[0-9]+}}]:13:1
// CHECK-NEXT: Set pauseOnThrow: all errors
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on exception in global: {{.*}}[{{[0-9]+}}]:14:3
// CHECK-NEXT: Exception: Error: Bar
// CHECK-NEXT:   0: eval: {{.*}}
// CHECK-NEXT:   1: global: {{.*}}/throw-throw-eval.js:14:3
// CHECK-NEXT: Thrown value is: {{.*}}
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Message: Foo
