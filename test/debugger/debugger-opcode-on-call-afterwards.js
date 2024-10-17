/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheckOrRegen --match-full-lines %s
// REQUIRES: debugger

function fn() {
  print('fn');
}

function userBreakpointAfter(a) {
  fn();
  return a; // Should also break here
}

function debuggerStmtAfter(a) {
  fn();
  debugger; // Should also break here
  return a;
}

debugger;

// Test there being user breakpoint afterwards
userBreakpointAfter(1);

// Test there being 'debugger' statement afterwards
debuggerStmtAfter(1);

// Pause on debugger statement. Setup all the breakpoints we need.
// CHECK: Break on 'debugger' statement in global: {{.*}}:26:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:16:5
// CHECK-NEXT: Set breakpoint 2 at {{.*}}:17:10
// CHECK-NEXT: Set breakpoint 3 at {{.*}}:21:5
// CHECK-NEXT: Continuing execution

// Verify that user breakpoint directly after still gets triggered:
// CHECK-NEXT: Break on breakpoint 1 in userBreakpointAfter: {{.*}}:16:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: fn
// CHECK-NEXT: Break on breakpoint 2 in userBreakpointAfter: {{.*}}:17:10
// CHECK-NEXT: Continuing execution

// Verify that debugger statement directly after still gets triggered:
// CHECK-NEXT: Break on breakpoint 3 in debuggerStmtAfter: {{.*}}:21:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: fn
// CHECK-NEXT: Break on 'debugger' statement in debuggerStmtAfter: {{.*}}:22:3
// CHECK-NEXT: Continuing execution
