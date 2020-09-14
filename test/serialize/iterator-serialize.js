/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=C.UTF-8 %hermes -O -serialize-after-init-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: LC_ALL=C.UTF-8 %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer
"use strict";

// Runs an iterator until it is done.
// Prints out `value` and `done` every iteration.
function runIterator(it) {
  print(it.toString());
  while (true) {
    var result = it.next();
    print(result.value, result.done);
    if (result.done) return;
  }
}

print('IteratorPrototype');
// CHECK-LABEL: IteratorPrototype
print(Object.getPrototypeOf(
  Object.getPrototypeOf([].keys())
)[Symbol.iterator].name);
// CHECK-LABEL: [Symbol.iterator]
print(Object.getPrototypeOf(
    Object.getPrototypeOf([].values())
)[Symbol.iterator].name);
// CHECK-LABEL: [Symbol.iterator]
print(Object.getPrototypeOf(
    Object.getPrototypeOf([].entries())
)[Symbol.iterator].name);
// CHECK-LABEL: [Symbol.iterator]

print('Array Iterator');
// CHECK-LABEL: Array Iterator
var a = ['a','b','c'];
runIterator(a.keys());
// CHECK-NEXT: [object Array Iterator]
// CHECK-NEXT: 0 false
// CHECK-NEXT: 1 false
// CHECK-NEXT: 2 false
// CHECK-NEXT: undefined true
runIterator(a.values());
// CHECK-NEXT: [object Array Iterator]
// CHECK-NEXT: a false
// CHECK-NEXT: b false
// CHECK-NEXT: c false
// CHECK-NEXT: undefined true
runIterator(a.entries());
// CHECK-NEXT: [object Array Iterator]
// CHECK-NEXT: 0,a false
// CHECK-NEXT: 1,b false
// CHECK-NEXT: 2,c false
// CHECK-NEXT: undefined true
var a = [];
runIterator(a.keys());
// CHECK-NEXT: [object Array Iterator]
// CHECK-NEXT: undefined true
runIterator(a.values());
// CHECK-NEXT: [object Array Iterator]
// CHECK-NEXT: undefined true
runIterator(a.entries());
// CHECK-NEXT: [object Array Iterator]
// CHECK-NEXT: undefined true

print('TypedArray Iterator');
// CHECK-LABEL: TypedArray Iterator
var a = new Uint8Array([10, 11, 12]);
runIterator(a.keys());
// CHECK-NEXT: [object Array Iterator]
// CHECK-NEXT: 0 false
// CHECK-NEXT: 1 false
// CHECK-NEXT: 2 false
// CHECK-NEXT: undefined true
runIterator(a.values());
// CHECK-NEXT: [object Array Iterator]
// CHECK-NEXT: 10 false
// CHECK-NEXT: 11 false
// CHECK-NEXT: 12 false
// CHECK-NEXT: undefined true
runIterator(a.entries());
// CHECK-NEXT: [object Array Iterator]
// CHECK-NEXT: 0,10 false
// CHECK-NEXT: 1,11 false
// CHECK-NEXT: 2,12 false
// CHECK-NEXT: undefined true
var a = new Uint8Array([]);
runIterator(a.keys());
// CHECK-NEXT: [object Array Iterator]
// CHECK-NEXT: undefined true
runIterator(a.values());
// CHECK-NEXT: [object Array Iterator]
// CHECK-NEXT: undefined true
runIterator(a.entries());
// CHECK-NEXT: [object Array Iterator]
// CHECK-NEXT: undefined true

print('String Iterator');
// CHECK-LABEL: String Iterator
var a = 'abcd';
runIterator(a[Symbol.iterator]());
// CHECK-NEXT: [object String Iterator]
// CHECK-NEXT: a false
// CHECK-NEXT: b false
// CHECK-NEXT: c false
// CHECK-NEXT: d false
// CHECK-NEXT: undefined true
var a = '';
runIterator(a[Symbol.iterator]());
// CHECK-NEXT: [object String Iterator]
// CHECK-NEXT: undefined true
var a = 'x\uD83D\uDCD3y';
runIterator(a[Symbol.iterator]());
// CHECK-NEXT: [object String Iterator]
// CHECK-NEXT: x false
// CHECK-NEXT: 📓 false
// CHECK-NEXT: y false
// CHECK-NEXT: undefined true

print('RegExp String Iterator');
// CHECK-LABEL: RegExp String Iterator
var a = 'foo bar baz';
var re = /\w+/g
runIterator(a.matchAll(re));
// CHECK-NEXT: [object RegExp String Iterator]
// CHECK-NEXT: foo false
// CHECK-NEXT: bar false
// CHECK-NEXT: baz false
// CHECK-NEXT: undefined true
var a = '';
runIterator(a.matchAll(re));
// CHECK-NEXT: [object RegExp String Iterator]
// CHECK-NEXT: undefined true
