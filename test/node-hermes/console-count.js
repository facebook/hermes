/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %node-hermes %s 2>&1 | %FileCheck --match-full-lines %s
// REQUIRES: node-hermes

print('Console count functionality');
// CHECK-LABEL: Console count functionality

let user = "";

function greet() {
  console.count();
  return "hi " + user;
}

user = "bob";
greet();
// CHECK: default: 1
user = "alice";
greet();
// CHECK-NEXT: default: 2
greet();
// CHECK-NEXT: default: 3
console.count();
// CHECK-NEXT: default: 4

console.count('abc');
// CHECK-NEXT: abc: 1
console.count('xyz');
// CHECK-NEXT: xyz: 1
console.count('abc');
// CHECK-NEXT: abc: 2
console.countReset('abc');
console.count('abc');
// CHECK-NEXT: abc: 1
console.countReset('not set');
// CHECK-NEXT: Count for 'not set' does not exist
