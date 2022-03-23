/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb < %s.debug %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('step in');
// CHECK-LABEL: step in

function foo() {
  print('first');
  var x = bar();
  print('second');
}

function bar() {
  return 'second';
}

debugger;
foo();
print('returned');

// CHECK: Break on 'debugger' statement in {{.*}}:24:1
// CHECK-NEXT: Stepped to global: {{.*}}:25:1
// CHECK-NEXT: Stepped to foo: {{.*}}:15:3
// CHECK-NEXT: first
// CHECK-NEXT: Stepped to foo: {{.*}}:16:11
// CHECK-NEXT: Stepped to bar: {{.*}}:21:3
// CHECK-NEXT: Stepped to foo: {{.*}}:16:9
// CHECK-NEXT: Stepped to foo: {{.*}}:17:3
// CHECK-NEXT: second
// CHECK-NEXT: Stepped to global: {{.*}}:25:4
// CHECK-NEXT: Stepped to global: {{.*}}:26:1
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: returned

debugger;
var x = [1,2,3];
x.forEach(function(i) {
  return 'myresult';
});

// CHECK: Break on 'debugger' statement in global: {{.*}}:42:1
// CHECK-NEXT: Stepped to global: {{.*}}:43:9
// CHECK-NEXT: Stepped to global: {{.*}}:44:1
// CHECK-NEXT: Stepped to anonymous: {{.*}}:45:3
// CHECK-NEXT: Stepped to anonymous: {{.*}}:44:11
// CHECK-NEXT: Stepped to anonymous: {{.*}}:45:3
// CHECK-NEXT: Stepped to anonymous: {{.*}}:44:11
// CHECK-NEXT: Stepped to anonymous: {{.*}}:45:3
// CHECK-NEXT: Continuing execution
