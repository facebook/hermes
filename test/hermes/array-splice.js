/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

"use strict";

print('splice');
// CHECK-LABEL: splice
var a = ['a','b','c','d'];
print(a.splice(), a);
// CHECK-NEXT:  a,b,c,d
var a = ['a','b','c','d'];
print(a.splice(1), a);
// CHECK-NEXT: b,c,d a
var a = ['a','b','c','d'];
print(a.splice(1, 2), a);
// CHECK-NEXT: b,c a,d
var a = ['a','b','c','d'];
print(a.splice(1, 2, 'x'), a);
// CHECK-NEXT: b,c a,x,d
var a = ['a','b','c','d'];
print(a.splice(1, 2, 'x', 'y', 'z'), a);
// CHECK-NEXT: b,c a,x,y,z,d
var a = ['a','b','c','d'];
print(a.splice(1, 1000, 'x', 'y', 'z'), a);
// CHECK-NEXT: b,c,d a,x,y,z
var a = ['a','b','c','d'];
print(a.splice(-1, 1, 'x', 'y', 'z'), a);
// CHECK-NEXT: d a,b,c,x,y,z
var a = ['a','b','c','d'];
print(a.splice(-1, 1, 'x', 'y', 'z'), a);
// CHECK-NEXT: d a,b,c,x,y,z
var a = ['a','b','c','d'];
print(a.splice(2, 0, 'x', 'y', 'z'), a);
// CHECK-NEXT:  a,b,x,y,z,c,d
var a = ['a','b','c','d'];
print(a.splice(2, -100, 'x', 'y', 'z'), a);
// CHECK-NEXT:  a,b,x,y,z,c,d
var a = ['a',,'b',,,'c'];
print(a.splice(0, 3, 'x', 'y'), a);
// CHECK-NEXT: a,,b x,y,,,c
var a = ['a','b','c'];
print(a.splice(0, Infinity), a);
// CHECK-NEXT: a,b,c
var a = ['a', , 'b'];
a.__proto__ = [];
Object.setPrototypeOf(a.__proto__, Array.prototype);
a.__proto__[1] = "PROTO";
print(a.splice(1, 1, 'x'), a);
// CHECK-NEXT: PROTO a,x,b
var a = ['a', , 'b'];
a.__proto__ = {};
Object.setPrototypeOf(a.__proto__, Array.prototype);
a.__proto__[1] = "PROTO";
print(a.splice(1, 1, 'x'), a);
// CHECK-NEXT: PROTO a,x,b
var a = [];
a[10] = 'a';
print(a.splice(10, 1, 'x'), a);
// CHECK-NEXT: a ,,,,,,,,,,x
var a = ['a','b','c'];
Object.defineProperty(a, 'length', { writable: false });
try { a.splice(1, 2, 'd'); } catch (e) { print(e.name); }
// CHECK-NEXT: TypeError
var x = { 0: 0, 1: 1, 2: 2, 3: 3, length: 4, };
var arr = Array.prototype.splice.call(x, 1, 2,100);
print(Array.prototype.join.call(x, ','), arr);
// CHECK-NEXT: 0,100,3 1,2
