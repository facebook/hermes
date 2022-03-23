/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

print(`hello${1 + 1}world`);
// CHECK: hello2world
print(`world`);
// CHECK-NEXT: world
print('world' + `` + 'hello');
// CHECK-NEXT: worldhello
print(`${666}`);
// CHECK-NEXT: 666
print(`begin${`first${`second`}firstend`}end`);
// CHECK-NEXT: beginfirstsecondfirstendend
var num = 99;
print(`${111 + 222}${num > 100 ? 'big' : 'small'}`);
// CHECK-NEXT: 333small
print(`first line\nsecond line`);
// CHECK-NEXT: first line
// CHECK-NEXT: second line
function func(x) {
  return x > 0;
}
print(`positive? ${func(10)}`);
//CHECK-NEXT: positive? true

var obj = { toString() { return 'tostr'; }, valueOf(){ return 'value'; } }
print(`${obj}`)
//CHECK-NEXT: tostr
