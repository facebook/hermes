/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --lazy %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function foo() {
  print('foo called');

  function bar() {
    print('bar called');
  }
  function baz() {
    print('baz called');
  }

  bar();
  baz();
}

debugger;
foo();

// CHECK: Break on 'debugger' statement in global: {{.*}}:25:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:15:5
// CHECK-NEXT: Set breakpoint 2 at {{.*}}:18:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: foo called
// CHECK-NEXT: Break on breakpoint 1 in bar: {{.*}}lazy-break-nested.js[{{[0-9]+}}]:15:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: bar called
// CHECK-NEXT: Break on breakpoint 2 in baz: {{.*}}lazy-break-nested.js[{{[0-9]+}}]:18:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: baz called
