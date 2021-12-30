/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('enable disable');
// CHECK-LABEL: enable disable

function foo() {
  print('first');
  print('second');
}

debugger;
foo();
debugger;
foo();
debugger;
foo();

// CHECK-NEXT: Break on 'debugger' statement in global: {{.*}}:19:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:15:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}:15:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: second
// CHECK-NEXT: Break on 'debugger' statement in global: {{.*}}:21:1
// CHECK-NEXT: Disabled breakpoint 1
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: second
// CHECK-NEXT: Break on 'debugger' statement in global: {{.*}}:23:1
// CHECK-NEXT: Enabled breakpoint 1
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}:15:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: second
