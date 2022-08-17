/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --break-at-start %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

// Ensure the temporary breakpoints from exec don't interfere
// with the temporary breakpoints from stepping.

function foo() {
  return 1;
}

var x = 3;
debugger;
var y = foo();

// CHECK: Break on script load in global: {{.*}}:14:1
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on 'debugger' statement in global: {{.*}}:19:1
// CHECK-NEXT: Stepped to global: {{.*}}:20:9
// CHECK-NEXT: 3
// CHECK-NEXT: Stepped to foo: {{.*}}:15:3
// CHECK-NEXT: Continuing execution
