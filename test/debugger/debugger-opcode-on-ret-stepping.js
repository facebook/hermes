/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

var obj = {
  get number() {
    // We'll install the breakpoint on the Ret for the next line.
    return 1;
  }
};

debugger;

// Test doing 'step in'
print(obj.number);
// Call twice to make sure the breakpoint is restored
print(obj.number);

// Test doing 'step over'
print(obj.number);
print(obj.number);

// Test doing 'step out'
print(obj.number);
print(obj.number);

// Pause on debugger statement. Setup all the breakpoints we need.
// CHECK: Break on 'debugger' statement in global: {{.*}}:18:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:14:5
// CHECK-NEXT: Continuing execution

// Verify that reusing the Interpreter loop by delaying the restoration of
// Debugger OpCode works. Also, that the Debugger OpCode gets restored so that
// the next usage of obj.number still triggers breakpoint.
// CHECK-NEXT: Break on breakpoint 1 in get number: {{.*}}:14:5
// CHECK-NEXT: Stepped to global: {{.*}}:21:10
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: 1
// CHECK-NEXT: Break on breakpoint 1 in get number: {{.*}}:14:5
// CHECK-NEXT: Stepped to global: {{.*}}:23:10
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: 1

// Test doing 'step over' on breakpoint placed on top of Ret:
// CHECK-NEXT: Break on breakpoint 1 in get number: {{.*}}:14:5
// CHECK-NEXT: Stepped to global: {{.*}}:26:10
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: 1
// CHECK-NEXT: Break on breakpoint 1 in get number: {{.*}}:14:5
// CHECK-NEXT: Stepped to global: {{.*}}:27:10
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: 1

// Test doing 'step out' on breakpoint placed on top of Ret:
// CHECK-NEXT: Break on breakpoint 1 in get number: {{.*}}:14:5
// CHECK-NEXT: Stepped to global: {{.*}}:30:10
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: 1
// CHECK-NEXT: Break on breakpoint 1 in get number: {{.*}}:14:5
// CHECK-NEXT: Stepped to global: {{.*}}:31:10
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: 1
