/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

function className(x) {
    return Object.prototype.toString.call(x);
}

var a;

a = [];
print(className(a), a.length);
//CHECK: [object Array] 0

a = Array(5);
print(className(a), a.length);
//CHECK-NEXT: [object Array] 5

a = new Array(3);
print(className(a), a.length);
//CHECK: [object Array] 3

print(a[0], a[1], a[2], a[3], a[4]);
//CHECK-NEXT: undefined undefined undefined undefined undefined

a = Array();
print(className(a), a.length);
//CHECK: [object Array] 0

a = Array(10, 20, 30)

print(a[0], a[1], a[2], a[3], a[4]);
//CHECK-NEXT: 10 20 30 undefined undefined

for(var i = 0, e = a.length; i < e; ++i)
    ++a[i];
print(a[0], a[1], a[2], a[3], a[4]);
//CHECK-NEXT: 11 21 31 undefined undefined

a[4] = 51;
print(a[0], a[1], a[2], a[3], a[4]);
//CHECK-NEXT: 11 21 31 undefined 51

a = Array();
a[3] = 3;
print(a.length, a[0], a[1], a[2], a[3], a[4]);
//CHECK-NEXT: 4 undefined undefined undefined 3 undefined

a[2] = 2;
print(a.length, a[0], a[1], a[2], a[3], a[4]);
//CHECK-NEXT: 4 undefined undefined 2 3 undefined

a = Array();
a[4294967294] = 100;
print(a.length);
//CHECK-NEXT: 4294967295

a = [1, 2, 3, 4];
print(a[0], a[1], a[2], a[3], a.length);
//CHECK-NEXT: 1 2 3 4 4

a = [,,1,2,,];
print(a[0], a[1], a[2], a[3], a[4], a.length);
//CHECK-NEXT: undefined undefined 1 2 undefined 5

function foo() { return 1; }
a = [1, 2, foo(), foo() + 1, 3, 4,,];
print(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a.length);
//CHECK-NEXT: 1 2 1 2 3 4 undefined 7

a = [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,,5];
print(a[257], a.length);
//CHECK-NEXT: 5 258

array = new Array(4);
array[0] = 1; array[2] = 2; array[3] = 3;
var proto = {};
array.__proto__ = proto;

Object.defineProperty(proto, 1, {
    get: function get() {
        array.length = 1;
        gc();
        return "foo";
    },
});
print(Array.prototype.concat.call(array));
// CHECK-NEXT: 1,foo,,

array = new Array(4);
array[0] = 1; array[2] = 2; array[3] = 3;
var array2 = [4, 5, 6, 7, 8];
var proto = {};
array.__proto__ = proto;

Object.defineProperty(proto, 1, {
    get: function get() {
        array2.length = 0;
        return "foo";
    },
});
print(Array.prototype.concat.call(array, array2));
// CHECK-NEXT: 1,foo,2,3

a = []
a[0] = 0;
a[5] = 100;
a = Array.prototype.concat.call(a, a);
print(a.length);
// CHECK-NEXT: 12
print(Object.getOwnPropertyDescriptor(a, 0) === undefined,
      Object.getOwnPropertyDescriptor(a, 1) === undefined);
// CHECK-NEXT: false true
print(Object.getOwnPropertyDescriptor(a, 5) === undefined,
      Object.getOwnPropertyDescriptor(a, 6) === undefined);
// CHECK-NEXT: false false
print(Object.getOwnPropertyDescriptor(a, 9) === undefined,
      Object.getOwnPropertyDescriptor(a, 10) === undefined,
      Object.getOwnPropertyDescriptor(a, 11) === undefined,
      Object.getOwnPropertyDescriptor(a, 12) === undefined);
// CHECK-NEXT: true true false true


a = [1, 2, 3];
Object.defineProperty(Array.prototype, "0", {
  value: "test",
  writable: false,
  configurable: true
});
a = a.slice(0, 1);
print(a[5]);
//CHECK-NEXT: undefined


print(Array.isArray([]));
//CHECK-NEXT: true

print(Array.isArray({}));
//CHECK-NEXT: false

print(Array.isArray(Array));
//CHECK-NEXT: false


a = [];
a[4294967294] = 1;
print(a.length);
//CHECK-NEXT: 4294967295

a[1] = 1;
print(a.length);
//CHECK-NEXT: 4294967295

a[4294967295] = 1;
print(a.length);
//CHECK-NEXT: 4294967295

