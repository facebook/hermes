/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
"use strict";

print('push');
// CHECK-LABEL: push
var a = [1,2,3];
print(a.push(1834), a.toString());
// CHECK-NEXT: 4 1,2,3,1834
print(a.push('abcd', 746, 133), a.toString());
// CHECK-NEXT: 7 1,2,3,1834,abcd,746,133
print(a.push(), a.toString());
// CHECK-NEXT: 7 1,2,3,1834,abcd,746,133
print(a.pop(), a.toString());
// CHECK-NEXT: 133 1,2,3,1834,abcd,746
print(a.push('last'), a.toString());
// CHECK-NEXT: 7 1,2,3,1834,abcd,746,last
var a = [,,,'x','y','z'];
print(a)
// CHECK-NEXT: ,,,x,y,z
print(a.push(1,2,3), a);
// CHECK-NEXT: 9 ,,,x,y,z,1,2,3
var a = {3: 'x', 4: 'y', length: 5};
Array.prototype.push.call(a, 'z', 'last');
print(a.length, a[5], a[6]);
// CHECK-NEXT: 7 z last
var a = Array(0xfffffffe);
print(a.length);
// CHECK-NEXT: 4294967294
a.push('arrEnd');
print(a.length, a[4294967294]);
// CHECK-NEXT: 4294967295 arrEnd
try { a.push('a','b','c'); } catch (e) { print('caught', e.name) }
// CHECK-NEXT: caught RangeError
print(a[4294967295], a[4294967296], a[4294967297]);
// CHECK-NEXT: a b c

var fakeProto = {__proto__: Array.prototype};
fakeProto[1] = 'asdf';
Object.defineProperty(fakeProto, '1', {
  value: 'asdf',
  writable: false,
  configurable: true,
});
var a = [];
Object.setPrototypeOf(a, fakeProto);
try { a.push('x','y','z'); } catch (e) { print('caught', e.name) }
// CHECK-NEXT: caught TypeError
Object.defineProperty(fakeProto, '1', {
  get: function() { return 'asdf'; },
  set: function() { print('setter'); },
  configurable: true,
});

var a = [];
Object.setPrototypeOf(a, fakeProto);
print(a.push(1,2), a);
// CHECK-NEXT: setter
// CHECK-NEXT: 2 1,asdf

var a = [];
delete fakeProto[1];
Object.setPrototypeOf(a, fakeProto);
a[0] = 'x1';
a[10000] = 'x2';
print(a.push('last'), a[0], a[10000], a[10001]);
// CHECK-NEXT: 10002 x1 x2 last
a = null;

var a = {length: 2**53 - 1};
try { a.push(1); } catch (e) { print('caught', e.name) }
// CHECK-NEXT: caught TypeError
