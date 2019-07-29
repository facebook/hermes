// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

print(`hello${1 + 1}world`);
// CHECK: hello2world
print(`world`);
// CHECK: world
print('world' + `` + 'hello');
// CHECK: worldhello
print(`${666}`);
// CHECK: 666
print(`begin${`first${`second`}firstend`}end`);
// CHECK: beginfirstsecondfirstendend
var num = 99;
print(`${111 + 222}${num > 100 ? 'big' : 'small'}`);
// CHECK: 333small
print(`first line\nsecond line`);
// CHECK: first line
// CHECK: second line
function func(x) {
  return x > 0;
}
print(`positive? ${func(10)}`);
//CHECK: positive? true
