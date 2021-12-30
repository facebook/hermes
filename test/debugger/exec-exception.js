/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function eventuallyThrows(x) {
  if (x <= 0)
    throw new Error("I frew up.");
  eventuallyThrows(x-1);
}

function callme() {
  print("Hello");
  debugger;
  print("Goodbye");
}

callme();

// CHECK: Hello
// CHECK-NEXT: Break on 'debugger' statement in callme: {{.*}}:19:3
// CHECK-NEXT: Exception: SyntaxError: 1:6:';' expected
// CHECK-NEXT: Thrown value is: { message: 1:6:';' expected }
// CHECK-NEXT: Next
// CHECK-NEXT: Exception: Error: I frew up.
// CHECK-NEXT:   0: eventuallyThrows: {{.*}}:13:20
// CHECK-NEXT:   1: eventuallyThrows: {{.*}}:14:19
// CHECK-NEXT:   2: eventuallyThrows: {{.*}}:14:19
// CHECK-NEXT:   3: eventuallyThrows: {{.*}}:14:19
// CHECK-NEXT:   4: eventuallyThrows: {{.*}}:14:19
// CHECK-NEXT:   5: eventuallyThrows: {{.*}}:14:19
// CHECK-NEXT:   6: eval: {{.*}}:1:17
// CHECK-NEXT:   7: callme: {{.*}}:19:3
// CHECK-NEXT:   8: global: {{.*}}:23:7
// CHECK-NEXT: Thrown value is: { message: I frew up. }
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Goodbye
