/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('break loop');
// CHECK-LABEL: break loop

function foo() {
  for (var i = 0; i < 3; ++i) {
    print('first');
    print('second');
  }
}

function bar() {
  debugger;
  foo();
}

bar();

// CHECK-NEXT: Break on 'debugger' statement in bar: {{.*}}:22:3
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:17:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}:17:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: second
// CHECK-NEXT: first
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}:17:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: second
// CHECK-NEXT: first
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}:17:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: second
