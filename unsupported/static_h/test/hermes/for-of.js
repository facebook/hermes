/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

if (typeof print === "undefined")
    var print = console.log;

function forof_normal(seq, cb) {
    for(var i of seq)
        cb(i*10);
}
forof_normal([1,2,3], print);
//CHECK:10
//CHECK-NEXT:20
//CHECK-NEXT:30

function iterable_to_array(seq) {
    var i = 0, ar = [];
    for(ar[i++] of seq);
    return ar;
}

print(iterable_to_array([1,2,3]));
//CHECK-NEXT: 1,2,3

function forof_args() {
    var i = 0, ar = []
    for(ar[i++] of arguments);
    return ar;
}
print(forof_args(5,6,7));
//CHECK-NEXT: 5,6,7

function forof_proto() {
  var arr = [1,,3];
  arr.__proto__[1] = 'in proto';
  for(var i of arr) print(i)
}
forof_proto();
// CHECK-NEXT: 1
// CHECK-NEXT: in proto
// CHECK-NEXT: 3

function forof_getter() {
  var arr = [1,,3];
  Object.defineProperty(arr, '1', {
    get: function() {
      return 'in getter';
    }
  });
  for(var i of arr) print(i)
}
forof_getter();
// CHECK-NEXT: 1
// CHECK-NEXT: in getter
// CHECK-NEXT: 3

/// Since we can't rely on computer properties yet, this convenience function
/// initializes obj[@@iterator].
function iterable(obj) {
    obj[Symbol.iterator] = function() { return this; }
    return obj;
}

// Try a simple iterable.
print(iterable_to_array(iterable( {
    i : 0,
    next: function() { return ++this.i <= 3 ? { value: this.i*10 } : { done: true }; }
})));
//CHECK-NEXT: 10,20,30

/// Call target with the rest of the arguments and print the caught exception.
function catcher(target) {
    try {
        return target.apply(this, Array.prototype.slice.call(arguments, 1));
    } catch (e) {
        print("caught:", e.message);
    }
}

catcher(iterable_to_array, {});
//CHECK-NEXT: caught: iterator method is not callable

// obj[@@iterator] is not a function.
var obj = {}
obj[Symbol.iterator] = {}

catcher(iterable_to_array, obj);
//CHECK-NEXT: caught: Could not get callable method from object

// obj[@@iterator] doesn't return an object.
obj[Symbol.iterator] = function() { return 10; }

catcher(iterable_to_array, obj);
//CHECK-NEXT: caught: iterator is not an object

// next() doesn't return an object.
catcher(iterable_to_array, iterable( {
    next: function() { return 10; }
}));
//CHECK-NEXT: caught: iterator.next() did not return an object

var arr = [];
arr.__proto__ = new Proxy({}, {});
catcher(iterable_to_array, arr);
//CHECK-NEXT: caught: iterator method is not callable
