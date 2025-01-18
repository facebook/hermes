/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheckOrRegen --match-full-lines %s
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

// Auto-generated content below. Please do not modify manually.

// CHECK:Break on 'debugger' statement in global: {{.*}}debugger-opcode-on-ret-stepping.js[2]:18:1
// CHECK-NEXT:Set breakpoint 1 at {{.*}}debugger-opcode-on-ret-stepping.js[2]:14:5
// CHECK-NEXT:Continuing execution
// CHECK-NEXT:Break on breakpoint 1 in get number: {{.*}}debugger-opcode-on-ret-stepping.js[2]:14:5
// CHECK-NEXT:Stepped to global: {{.*}}debugger-opcode-on-ret-stepping.js[2]:21:10
// CHECK-NEXT:Continuing execution
// CHECK-NEXT:1
// CHECK-NEXT:Break on breakpoint 1 in get number: {{.*}}debugger-opcode-on-ret-stepping.js[2]:14:5
// CHECK-NEXT:Stepped to global: {{.*}}debugger-opcode-on-ret-stepping.js[2]:23:10
// CHECK-NEXT:Continuing execution
// CHECK-NEXT:1
// CHECK-NEXT:Break on breakpoint 1 in get number: {{.*}}debugger-opcode-on-ret-stepping.js[2]:14:5
// CHECK-NEXT:Stepped to global: {{.*}}debugger-opcode-on-ret-stepping.js[2]:26:10
// CHECK-NEXT:Continuing execution
// CHECK-NEXT:1
// CHECK-NEXT:Break on breakpoint 1 in get number: {{.*}}debugger-opcode-on-ret-stepping.js[2]:14:5
// CHECK-NEXT:Stepped to global: {{.*}}debugger-opcode-on-ret-stepping.js[2]:27:10
// CHECK-NEXT:Continuing execution
// CHECK-NEXT:1
// CHECK-NEXT:Break on breakpoint 1 in get number: {{.*}}debugger-opcode-on-ret-stepping.js[2]:14:5
// CHECK-NEXT:Stepped to global: {{.*}}debugger-opcode-on-ret-stepping.js[2]:30:10
// CHECK-NEXT:Continuing execution
// CHECK-NEXT:1
// CHECK-NEXT:Break on breakpoint 1 in get number: {{.*}}debugger-opcode-on-ret-stepping.js[2]:14:5
// CHECK-NEXT:Stepped to global: {{.*}}debugger-opcode-on-ret-stepping.js[2]:31:10
// CHECK-NEXT:Continuing execution
// CHECK-NEXT:1
