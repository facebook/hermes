// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
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

// CHECK: Break on 'debugger' statement in {{.*}}:22:1
// CHECK-NEXT: Stepped to global: {{.*}}:23:1
// CHECK-NEXT: Stepped to foo: {{.*}}:13:3
// CHECK-NEXT: first
// CHECK-NEXT: Stepped to foo: {{.*}}:14:11
// CHECK-NEXT: Stepped to bar: {{.*}}:19:3
// CHECK-NEXT: Stepped to foo: {{.*}}:14:9
// CHECK-NEXT: Stepped to foo: {{.*}}:15:3
// CHECK-NEXT: second
// CHECK-NEXT: Stepped to global: {{.*}}:23:4
// CHECK-NEXT: Stepped to global: {{.*}}:24:1
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: returned

debugger;
var x = [1,2,3];
x.forEach(function(i) {
  return 'myresult';
});

// CHECK: Break on 'debugger' statement in global: {{.*}}:40:1
// CHECK-NEXT: Stepped to global: {{.*}}:41:9
// CHECK-NEXT: Stepped to global: {{.*}}:42:1
// CHECK-NEXT: Stepped to anonymous: {{.*}}:43:3
// CHECK-NEXT: Stepped to anonymous: {{.*}}:42:23
// CHECK-NEXT: Stepped to anonymous: {{.*}}:43:3
// CHECK-NEXT: Stepped to anonymous: {{.*}}:42:23
// CHECK-NEXT: Stepped to anonymous: {{.*}}:43:3
// CHECK-NEXT: Continuing execution
