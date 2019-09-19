// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hdb --lazy %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function foo() {
  print('foo called');
  /**
   * Some text to pad out the function so that it won't be eagerly compiled
   * for being too short. Lorem ipsum dolor sit amet, consectetur adipiscing
   * elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
   */

  function bar() {
    print('bar called');
    /**
     * Some text to pad out the function so that it won't be eagerly compiled
     * for being too short. Lorem ipsum dolor sit amet, consectetur adipiscing
     * elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
     */
  }

  function baz() {
    print('baz called');
    /**
     * Some text to pad out the function so that it won't be eagerly compiled
     * for being too short. Lorem ipsum dolor sit amet, consectetur adipiscing
     * elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
     */
  }

  bar();
  baz();
}

debugger;
foo();

// CHECK: Break on 'debugger' statement in global: {{.*}}:39:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:18:5
// CHECK-NEXT: Set breakpoint 2 at {{.*}}:27:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: foo called
// CHECK-NEXT: Break on breakpoint 1 in bar: {{.*}}lazy-break-nested.js[2]:18:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: bar called
// CHECK-NEXT: Break on breakpoint 2 in baz: {{.*}}lazy-break-nested.js[2]:27:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: baz called
