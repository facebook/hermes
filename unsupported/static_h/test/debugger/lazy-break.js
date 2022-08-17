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

debugger;
foo();

// CHECK: Break on 'debugger' statement in global: {{.*}}lazy-break.js[{{[0-9]+}}]:15:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:12:3
// CHECK-NEXT: 1 E {{.*}}:12:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}lazy-break.js[{{[0-9]+}}]:12:3
// CHECK-NEXT: 1 E {{.*}}:12:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: foo called
