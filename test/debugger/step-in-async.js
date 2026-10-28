/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheckOrRegen --match-full-lines %s
// REQUIRES: debugger

async function test() {
  print('inside async function');
}

debugger;

test().then(() => {
  print('done');
});

// Auto-generated content below. Please do not modify manually.

// CHECK:Break on 'debugger' statement in global: {{.*}}step-in-async.js[3]:15:1
// CHECK-NEXT:Set breakpoint 1 at {{.*}}step-in-async.js[3]:12:3
// CHECK-NEXT:Continuing execution
// CHECK-NEXT:Break on breakpoint 1 in ?anon_0_test: {{.*}}step-in-async.js[3]:12:3
// CHECK-NEXT:inside async function
// CHECK-NEXT:Stepped to global: {{.*}}step-in-async.js[3]:17:12
// CHECK-NEXT:Continuing execution
