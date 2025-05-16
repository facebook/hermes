/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -fno-inline %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -fno-inline -exec %s | %FileCheck --match-full-lines %s

'use strict';

print('add property');
// CHECK-LABEL: add property

function direct(obj) {
  obj.x = 1;
  return obj;
}
print(direct({}).x);
// CHECK-NEXT: 1
print(direct({}).x);
// CHECK-NEXT: 1

function indirect1(obj) {
  obj.a = 1;
  obj.b = 1;
  obj.c = 1;
  obj.d = 1;
  obj.e = 1;
  obj.f = 1;
  obj.g = 1;
  obj.x = 15;
  return obj;
}
print(indirect1({}).x);
// CHECK-NEXT: 15
print(indirect1({}).x);
// CHECK-NEXT: 15

function indirect2(obj) {
  obj.a = 1;
  obj.b = 1;
  obj.c = 1;
  obj.d = 1;
  obj.e = 1;
  obj.f = 1;
  obj.g = 1;
  obj.h = 1;
  obj.i = 1;
  obj.x = 18;
  return obj;
}
print(indirect2({}).x);
// CHECK-NEXT: 18
print(indirect2({}).x);
// CHECK-NEXT: 18

function addX(obj) {
  obj.x = 123;
  return obj;
}

var proto = {};
print(addX({__proto__: proto}).x);
// CHECK-NEXT: 123
var proto2 = {
  get x() { return 1000; },
  set x(v) { print('setter', v); },
};
print(addX({__proto__: proto2}).x);
// CHECK-NEXT: setter 123
// CHECK-NEXT: 1000
print(proto2.x);
// CHECK-NEXT: 1000

var proto = {};
print(addX({__proto__: proto}).x);
// CHECK-NEXT: 123
Object.defineProperty(proto, 'x', {
  get() { return 2000; },
  set(v) { print('setter', v); },
});
print(addX({__proto__: proto}).x);
// CHECK-NEXT: setter 123
// CHECK-NEXT: 2000

function addIndexLike(obj) {
  obj[1] = 172;
  return obj;
}
print(addIndexLike({})[1]);
// CHECK-NEXT: 172
print(addIndexLike({})[1]);
// CHECK-NEXT: 172

var p1 = {};
var p2 = Object.create(p1);
Object.defineProperty(p2, 'x', {value: 12, writable: true, configurable: true});
Object.defineProperty(p1, 'x', {value: 10, writable: false, configurable: true});
print(addX({__proto__: p2}).x);
// CHECK-NEXT: 123
delete p2.x;
try { print(addX({__proto__: p2}).x); } catch (e) { print(e.name); }
// CHECK-NEXT: TypeError

var proto = {};
print(addX({__proto__: proto}).x);
Object.freeze(proto);
// CHECK-NEXT: 123
try { print(addX({__proto__: p2}).x); } catch (e) { print(e.name); }
// CHECK-NEXT: TypeError

var proto = Object.create(null);
print(addX({__proto__: proto}).x);
// CHECK-NEXT: 123
var p2 = {
  set x(v) { print('setter', v); }
};
Object.setPrototypeOf(proto, p2);
print(addX({__proto__: proto}).x);
// CHECK-NEXT: setter 123
// CHECK-NEXT: undefined
