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

function test() {
  fn();
}

debugger;

// Test doing 'step in'
test();
test();

// Test doing 'step over'
test();
test();

// Test doing 'step out'
test();
test();

// CHECK: Break on 'debugger' statement in global: {{.*}}:19:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:16:5
// CHECK-NEXT: Continuing execution

// Test doing 'step in'
// CHECK-NEXT: Break on breakpoint 1 in test: {{.*}}:16:5
// CHECK-NEXT: Stepped to fn: {{.*}}:12:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: fn
// CHECK-NEXT: Break on breakpoint 1 in test: {{.*}}:16:5
// CHECK-NEXT: Stepped to fn: {{.*}}:12:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: fn

// Test doing 'step over'
// CHECK-NEXT: Break on breakpoint 1 in test: {{.*}}:16:5
// CHECK-NEXT: fn
// CHECK-NEXT: Stepped to global: {{.*}}:26:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on breakpoint 1 in test: {{.*}}:16:5
// CHECK-NEXT: fn
// CHECK-NEXT: Stepped to global: {{.*}}:27:5
// CHECK-NEXT: Continuing execution

// Test doing 'step out'
// CHECK-NEXT: Break on breakpoint 1 in test: {{.*}}:16:5
// CHECK-NEXT: fn
// CHECK-NEXT: Stepped to global: {{.*}}:30:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on breakpoint 1 in test: {{.*}}:16:5
// CHECK-NEXT: fn
// CHECK-NEXT: Stepped to global: {{.*}}:31:5
// CHECK-NEXT: Continuing execution
