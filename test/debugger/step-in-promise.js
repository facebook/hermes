/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheckOrRegen --match-full-lines %s
// REQUIRES: debugger

function test() {
  return new Promise(resolve => {
    print('inside Promise');
  });
}

debugger;

test().then(() => {
  print('done');
});

// Auto-generated content below. Please do not modify manually.

// CHECK:Break on 'debugger' statement in global: {{.*}}step-in-promise.js[3]:17:1
// CHECK-NEXT:Set breakpoint 1 at {{.*}}step-in-promise.js[3]:13:5
// CHECK-NEXT:Continuing execution
// CHECK-NEXT:Break on breakpoint 1 in anonymous: {{.*}}step-in-promise.js[3]:13:5
// CHECK-NEXT:inside Promise
// CHECK-NEXT:Stepped to test: {{.*}}step-in-promise.js[3]:12:21
// CHECK-NEXT:Stepped to global: {{.*}}step-in-promise.js[3]:19:12
// CHECK-NEXT:Continuing execution
