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

// Auto-generated content below. Please do not modify manually.

// CHECK:Break on 'debugger' statement in global: {{.*}}debugger-opcode-on-call-stepping.js[2]:19:1
// CHECK-NEXT:Set breakpoint 1 at {{.*}}debugger-opcode-on-call-stepping.js[2]:16:5
// CHECK-NEXT:Continuing execution
// CHECK-NEXT:Break on breakpoint 1 in test: {{.*}}debugger-opcode-on-call-stepping.js[2]:16:5
// CHECK-NEXT:Stepped to fn: {{.*}}debugger-opcode-on-call-stepping.js[2]:12:3
// CHECK-NEXT:Continuing execution
// CHECK-NEXT:fn
// CHECK-NEXT:Break on breakpoint 1 in test: {{.*}}debugger-opcode-on-call-stepping.js[2]:16:5
// CHECK-NEXT:Stepped to fn: {{.*}}debugger-opcode-on-call-stepping.js[2]:12:3
// CHECK-NEXT:Continuing execution
// CHECK-NEXT:fn
// CHECK-NEXT:Break on breakpoint 1 in test: {{.*}}debugger-opcode-on-call-stepping.js[2]:16:5
// CHECK-NEXT:fn
// CHECK-NEXT:Stepped to global: {{.*}}debugger-opcode-on-call-stepping.js[2]:26:5
// CHECK-NEXT:Continuing execution
// CHECK-NEXT:Break on breakpoint 1 in test: {{.*}}debugger-opcode-on-call-stepping.js[2]:16:5
// CHECK-NEXT:fn
// CHECK-NEXT:Stepped to global: {{.*}}debugger-opcode-on-call-stepping.js[2]:27:5
// CHECK-NEXT:Continuing execution
// CHECK-NEXT:Break on breakpoint 1 in test: {{.*}}debugger-opcode-on-call-stepping.js[2]:16:5
// CHECK-NEXT:fn
// CHECK-NEXT:Stepped to global: {{.*}}debugger-opcode-on-call-stepping.js[2]:30:5
// CHECK-NEXT:Continuing execution
// CHECK-NEXT:Break on breakpoint 1 in test: {{.*}}debugger-opcode-on-call-stepping.js[2]:16:5
// CHECK-NEXT:fn
// CHECK-NEXT:Stepped to global: {{.*}}debugger-opcode-on-call-stepping.js[2]:31:5
// CHECK-NEXT:Continuing execution
