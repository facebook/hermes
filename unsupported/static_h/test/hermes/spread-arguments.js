/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print('args spread');
// CHECK-LABEL: args spread

function foo(x, y, z) {
  print(x, y, z);
}

foo(...[1,2,3]);
// CHECK-NEXT: 1 2 3
new foo(...[10,20,30]);
// CHECK-NEXT: 10 20 30
foo(...[1], ...[], ...[2,3]);
// CHECK-NEXT: 1 2 3
foo(...[1, 2]);
// CHECK-NEXT: 1 2 undefined

function myClass(x, y) {
  print(x, y, new.target === myClass);
}
myClass.prototype.property = 101;

myClass(...[4,5]);
// CHECK-NEXT: 4 5 false
F = new myClass(...[4,5]);
// CHECK-NEXT: 4 5 true
print(F.property)
// CHECK-NEXT: 101

function bar() {
  print(...arguments);
}

bar(1, 2, 3);
// CHECK-NEXT: 1 2 3
