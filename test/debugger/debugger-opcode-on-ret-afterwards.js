/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function userBreakpointAfter(a) {
  var obj = {
    get x() {
      return 1;
    },
  };

  obj.x;
  return a; // Should also break here
}

function debuggerStmtAfter(a) {
  var obj = {
    get x() {
      return 1;
    },
  };

  obj.x;
  debugger; // Should also break here
  return a;
}

debugger;

// Test there being user breakpoint afterwards
userBreakpointAfter(1);

// Test there being 'debugger' statement afterwards
debuggerStmtAfter(1);

// Pause on debugger statement. Setup all the breakpoints we need.
// CHECK: Break on 'debugger' statement in global: {{.*}}:34:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:14:7
// CHECK-NEXT: Set breakpoint 2 at {{.*}}:19:10
// CHECK-NEXT: Set breakpoint 3 at {{.*}}:25:7
// CHECK-NEXT: Continuing execution

// Verify that user breakpoint directly after still gets triggered:
// CHECK-NEXT: Break on breakpoint 1 in get x: {{.*}}:14:7
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on breakpoint 2 in userBreakpointAfter: {{.*}}:19:10
// CHECK-NEXT: Continuing execution

// Verify that debugger statement directly after still gets triggered:
// CHECK-NEXT: Break on breakpoint 3 in get x: {{.*}}:25:7
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on 'debugger' statement in debuggerStmtAfter: {{.*}}:30:3
// CHECK-NEXT: Continuing execution
