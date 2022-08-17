/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('break');
// CHECK-LABEL: break

function foo() {
  print('first');
  print('second');
  print('third');
}

function bar() {
  debugger;
  foo();
}

bar();
// CHECK-NEXT: Break on 'debugger' statement in bar: {{.*}}:21:3
// CHECK-NEXT: Set breakpoint 1 at {{.+}}:16:3
// CHECK-NEXT: Invalid or duplicate breakpoint not set
// CHECK-NEXT: Set breakpoint 2 at {{.+}}:17:3
// CHECK-NEXT: Deleted breakpoint 2
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}:16:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: second
// CHECK-NEXT: third
