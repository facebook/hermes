/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb < %s.debug %s | %FileCheck --match-full-lines %s
// RUN: %hdb --lazy < %s.debug %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('step out');
// CHECK-LABEL: step out

function bar() {
  print('bar1');
  debugger;
  print('bar2');
}

function foo() {
  print('foo1');
  bar();
  print('foo2');
}

foo();
print('finished');

// CHECK-NEXT: foo1
// CHECK-NEXT: bar1
// CHECK-NEXT: Break on 'debugger' statement in bar: {{.*}}:17:3
// CHECK-NEXT: bar2
// CHECK-NEXT: Stepped to foo: {{.*}}:24:3
// CHECK-NEXT: foo2
// CHECK-NEXT: Stepped to global: {{.*}}:27:4
// CHECK-NEXT: finished
