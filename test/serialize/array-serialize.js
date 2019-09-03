// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -serialize-after-init-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer
"use strict";

// Make sure that we can redefine a index property.
var a = [];
a[0] = 1;
var acc = {};
acc.get = function() {return 'hello'};
Object.defineProperty(a, 0, acc);
print(a[0]);
//CHECK: hello

print('toString');
// CHECK-LABEL: toString
print(Array(1,2,3).toString());
// CHECK-NEXT: 1,2,3
print('empty', Array().toString());
// CHECK-NEXT: empty

// Overwrite join and make sure it fails over to Object.prototype.toString().
var a = Array(1,2,3);
a.join = null;
print(a.toString());
// CHECK-NEXT: [object Array]
var ots = Object.prototype.toString;
Object.prototype.toString = null;
print(a.toString());
// CHECK-NEXT: [object Array]
Object.prototype.toString = ots;
var a = [1,2];
var b = [3,4];
var c = [5,6,b];
b.push(c);
a.push(b);
print(a.toString());
// CHECK-NEXT: 1,2,3,4,5,6,

// Test some Array functions.
var res = Array.of.call(Object, 1, 2, 3);
print(Array.isArray(res), res[0], res[1], res[2]);
// CHECK-NEXT: false 1 2 3
var fromRes = Array.from('foo');
print(fromRes);
// CHECK-NEXT: f,o,o
// user-defined iterator
var iterable = {};
iterable[Symbol.iterator] = function () {
    var step = 0;
    var iterator = {
        next: function() {
            if (step == 5) {
              return {value: undefined, done: true};
            } else {
        			return {value: step++, done: false};
            }
        }
    };
	return iterator;
}
var iterableArr = Array.from(iterable);
print(iterableArr);
// CHECK-NEXT: 0,1,2,3,4

function testThisAsConstructor(items) {
  var ctor = function() {return {hello : "hello"};};
  var res = Array.from.call(ctor, items);
  print(res.length);
  print(res.hello);
}
testThisAsConstructor([1, 2]);
// CHECK-NEXT: 2
// CHECK-NEXT: hello

var a = [1,2,3];
print(a.shift(), a, a.length);
// CHECK-NEXT: 1 2,3 2
var a = ['a','b','c','b','x'];
print(a.indexOf('b'));
// CHECK-NEXT: 1
var a = [1,2,3];
print(a.filter(function(x) {return x === 2}));
// CHECK-NEXT: 2
