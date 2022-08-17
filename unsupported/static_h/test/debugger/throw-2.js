/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('throw');
// CHECK-LABEL: throw

function bar() {
  throw new Error('asdf');
}

function foo() {
  bar();
}

debugger;
try {
  foo();
} catch(e) {
  print('first');
  print('second');
}

// CHECK: Break on 'debugger' statement in global: {{.*}}:22:1
// CHECK-NEXT: Set pauseOnThrow: all errors
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on exception in bar: {{.*}}:15:3
// CHECK-NEXT: Stepped to global: {{.*}}:25:3
// CHECK-NEXT: Stepped to global: {{.*}}:26:3
// CHECK-NEXT: first
// CHECK-NEXT: Stepped to global: {{.*}}:27:3
// CHECK-NEXT: second
