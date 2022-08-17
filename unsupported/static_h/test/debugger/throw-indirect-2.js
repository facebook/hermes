/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// RUN: %hdb --lazy %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('throw indirect 2');
// CHECK-LABEL: throw indirect 2

function foo() {
  throw new Error('error1');
}

debugger;
try {
  foo.call();
} catch(e) {
  print('first');
  print(e.message);
  print('second');
}

try {
  throw new Error('uncaught');
} catch(e) {}

// CHECK: Break on 'debugger' statement in global: {{.*}}:19:1
// CHECK-NEXT: Set pauseOnThrow: all errors
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on exception in foo: {{.*}}:16:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: error1
// CHECK-NEXT: second
// CHECK-NEXT: Break on exception in global: {{.*}}:29:3
// CHECK-NEXT: Continuing execution
