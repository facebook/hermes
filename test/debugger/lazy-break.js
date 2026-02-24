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
}

function bar() {
  print('bar called');
}

debugger;
foo();
bar();

// CHECK: Break on 'debugger' statement in global: {{.*}}lazy-break.js[{{[0-9]+}}]:19:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:12:3
// CHECK-NEXT: Set breakpoint 2 at {{.*}}:16:3
// CHECK-NEXT: 1 E {{.*}}:12:3
// CHECK-NEXT: 2 E {{.*}}:16:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}lazy-break.js[{{[0-9]+}}]:12:3
// CHECK-NEXT: 1 E {{.*}}:12:3
// CHECK-NEXT: 2 E {{.*}}:16:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: foo called
// CHECK-NEXT: Break on breakpoint 2 in bar: {{.*}}lazy-break.js[{{[0-9]+}}]:16:3
// CHECK-NEXT: 1 E {{.*}}:12:3
// CHECK-NEXT: 2 E {{.*}}:16:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: bar called
