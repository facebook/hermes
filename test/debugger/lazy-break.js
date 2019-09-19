// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hdb --lazy %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function foo() {
  print('foo called');
  /* Some text to pad out the function so that it won't be eagerly compiled
   * for being too short. Lorem ipsum dolor sit amet, consectetur adipiscing
   * elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
   */
}

debugger;
foo();

// CHECK: Break on 'debugger' statement in global: {{.*}}lazy-break.js[2]:17:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:10:3
// CHECK-NEXT: 1 E {{.*}}:10:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}lazy-break.js[2]:10:3
// CHECK-NEXT: 1 E {{.*}}:10:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: foo called
