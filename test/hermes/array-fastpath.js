/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s
"use strict";

// Tests for the fast paths of shift/unshift/slice/forEach/map/filter,
// including the conditions that must make them fall back to the generic
// path: holes, callback-driven mutation mid-iteration, accessors, modified
// prototypes, and frozen/sealed arrays.

print('shift');
// CHECK-LABEL: shift
var a = [1, 2, 3];
print(a.shift(), a, a.length);
// CHECK-NEXT: 1 2,3 2
a = [];
print(a.shift(), a.length);
// CHECK-NEXT: undefined 0
a = [42];
print(a.shift(), a.length);
// CHECK-NEXT: 42 0
a.push(7);
print(a);
// CHECK-NEXT: 7
a = [, 1];
print(a.shift(), a, 0 in a);
// CHECK-NEXT: undefined 1 true
a = [1, , 3];
print(a.shift(), a, 0 in a, 1 in a);
// CHECK-NEXT: 1 ,3 false true
a = [1.5, 2.5, 3.5];
print(a.shift(), a);
// CHECK-NEXT: 1.5 2.5,3.5
a = [1, 2];
Object.defineProperty(a, "length", {writable: false});
try {
  a.shift();
} catch (e) {
  print(e instanceof TypeError);
}
// CHECK-NEXT: true
a = Object.freeze([1, 2]);
try {
  a.shift();
} catch (e) {
  print(e instanceof TypeError);
}
// CHECK-NEXT: true
var obj = {0: 'a', 1: 'b', length: 2};
print(Array.prototype.shift.call(obj), obj[0], obj.length);
// CHECK-NEXT: a b 1

print('unshift');
// CHECK-LABEL: unshift
a = [3, 4];
print(a.unshift(1, 2), a);
// CHECK-NEXT: 4 1,2,3,4
a = [];
print(a.unshift(1), a);
// CHECK-NEXT: 1 1
a = [1, 2];
print(a.unshift(), a);
// CHECK-NEXT: 2 1,2
a = [1, , 3];
print(a.unshift(0), a, 2 in a, 3 in a);
// CHECK-NEXT: 4 0,1,,3 false true
a = [1, 2, 3, 4];
a.unshift(-3, -2, -1, 0);
print(a);
// CHECK-NEXT: -3,-2,-1,0,1,2,3,4
a = Object.seal([1]);
try {
  a.unshift(0);
} catch (e) {
  print(e instanceof TypeError);
}
// CHECK-NEXT: true

print('slice');
// CHECK-LABEL: slice
a = [1, 2, 3, 4, 5];
print(a.slice(1, 3), a.slice(-2), a.slice(0, -1), a.slice(3, 1).length, a);
// CHECK-NEXT: 2,3 4,5 1,2,3,4 0 1,2,3,4,5
a = [1, , 3];
var s = a.slice(0, 3);
print(s.length, 0 in s, 1 in s, 2 in s);
// CHECK-NEXT: 3 true false true
// Mutating the array from valueOf during argument coercion must not use a
// stale fast path; the captured length is used with per-index HasProperty.
a = [1, 2, 3, 4, 5];
s = a.slice({valueOf: function() { a.length = 2; return 0; }});
print(s.length, s[0], s[1], 2 in s);
// CHECK-NEXT: 5 1 2 false

print('forEach');
// CHECK-LABEL: forEach
a = [10, 20, 30];
var acc = [];
a.forEach(function(v, i) { acc.push(v, i); });
print(acc);
// CHECK-NEXT: 10,0,20,1,30,2
a = [1, , 3];
acc = [];
a.forEach(function(v, i) { acc.push(i); });
print(acc);
// CHECK-NEXT: 0,2
// Callback shrinks the array mid-iteration.
a = [1, 2, 3, 4, 5];
acc = [];
a.forEach(function(v, i) { acc.push(v); if (i === 1) a.length = 2; });
print(acc);
// CHECK-NEXT: 1,2
// Callback grows the array mid-iteration; new elements are not visited.
a = [1, 2];
acc = [];
a.forEach(function(v) { acc.push(v); a.push(99); });
print(acc);
// CHECK-NEXT: 1,2
// Callback installs an accessor mid-iteration.
a = [1, 2, 3];
acc = [];
a.forEach(function(v, i) {
  acc.push(v);
  if (i === 0) {
    Object.defineProperty(
        a, 2, {get: function() { return 42; }, configurable: true});
  }
});
print(acc);
// CHECK-NEXT: 1,2,42
// Callback swaps the prototype for a Proxy mid-iteration; the hole at index
// 1 must then be read through the Proxy.
a = [1, , 3];
acc = [];
a.forEach(function(v, i) {
  if (i === 0) {
    Object.setPrototypeOf(a, new Proxy({}, {
      has: function(t, k) { return k === '1'; },
      get: function(t, k) { return k === '1' ? 111 : undefined; },
    }));
  }
  acc.push(v);
});
print(acc);
// CHECK-NEXT: 1,111,3

print('map');
// CHECK-LABEL: map
a = [1, 2, 3];
print(a.map(function(x) { return x * 2; }));
// CHECK-NEXT: 2,4,6
a = [1, , 3];
var mp = a.map(function(x) { return x + 1; });
print(mp.length, mp[0], 1 in mp, mp[2]);
// CHECK-NEXT: 3 2 false 4
// Writes from the callback are observed by later iterations.
a = [1, 2, 3];
print(a.map(function(x, i) { if (i === 0) a[2] = 30; return x; }));
// CHECK-NEXT: 1,2,30

print('filter');
// CHECK-LABEL: filter
a = [1, 2, 3, 4, 5, 6];
print(a.filter(function(x) { return x % 2 === 0; }));
// CHECK-NEXT: 2,4,6
a = [1, , 3];
print(a.filter(function() { return true; }));
// CHECK-NEXT: 1,3
a = [1, 2, 3, 4];
print(a.filter(function(x, i) { if (i === 0) a.length = 2; return true; }));
// CHECK-NEXT: 1,2

print('prototype pollution');
// CHECK-LABEL: prototype pollution
// An element on Array.prototype forces the generic path; holes must read
// through the prototype chain.
Array.prototype[1] = 'polluted';
a = [1, , 3];
acc = [];
a.forEach(function(v) { acc.push(v); });
print(acc);
// CHECK-NEXT: 1,polluted,3
print(a.shift(), a[0]);
// CHECK-NEXT: 1 polluted
delete Array.prototype[1];
