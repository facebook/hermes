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

// Tests for the fast paths of the Array.prototype methods that access the
// indexed storage directly: shift/unshift/slice, and the ones that run a
// callback per element (forEach/map/filter). Covers the fast paths themselves
// and the conditions that must make them fall back to the generic path:
// holes, mutation from an argument coercion or from a callback mid-iteration,
// accessors, modified prototypes, and frozen/sealed arrays.

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
// Length extended past the storage (elemCount != len) must take the
// generic path.
a = [1, 2];
a.length = 5;
print(a.shift(), a.length, a);
// CHECK-NEXT: 1 4 2,,,
// Storage that does not begin at index 0 must take the generic path.
a = [];
a[3] = 'x';
print(a.shift(), a.length, 3 in a, a[2]);
// CHECK-NEXT: undefined 3 false x
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
// Heap-allocated elements (strings, boxed doubles) are moved with write
// barriers by the fast path; the contents must survive a GC.
a = ['x', 'y', 1.5, 'z'];
print(a.shift(), a);
// CHECK-NEXT: x y,1.5,z
gc();
print(a);
// CHECK-NEXT: y,1.5,z
// An own accessor clears fast index properties; the generic path calls the
// getter while moving elements down.
a = [1, 2, 3];
Object.defineProperty(
    a, 2, {get: function() { return 42; }, configurable: true});
print(a.shift(), a, a.length);
// CHECK-NEXT: 1 2,42 2
// this coercion and length access failures propagate from the slow path.
try {
  Array.prototype.shift.call(null);
} catch (e) {
  print(e instanceof TypeError);
}
// CHECK-NEXT: true
try {
  Array.prototype.shift.call({get length() { throw new Error('len boom'); }});
} catch (e) {
  print(e.message);
}
// CHECK-NEXT: len boom

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
// Shrinking keeps the storage capacity, so this unshift grows within
// capacity and takes the inline (no realloc) path.
a = [1, 2, 3];
a.length = 2;
print(a.unshift(9), a);
// CHECK-NEXT: 3 9,1,2
// Length extended past the storage (elemCount != len) must take the
// generic path; the trailing holes stay holes.
a = [1, 2];
a.length = 4;
print(a.unshift(0), a, 3 in a);
// CHECK-NEXT: 5 0,1,2,, false
a = Object.seal([1]);
try {
  a.unshift(0);
} catch (e) {
  print(e instanceof TypeError);
}
// CHECK-NEXT: true
// A hole at index 0 is moved right like an ordinary value.
a = [, 'b'];
print(a.unshift('z'), a, 1 in a);
// CHECK-NEXT: 3 z,,b false
// Heap values and boxed doubles are moved/encoded with write barriers.
a = ['c', 'd'];
print(a.unshift('a', 1.5), a);
// CHECK-NEXT: 4 a,1.5,c,d
gc();
print(a);
// CHECK-NEXT: a,1.5,c,d
// Read-only length changes the hidden class, so the fast path is skipped
// and the generic path throws when growing the array.
a = [1, 2];
Object.defineProperty(a, "length", {writable: false});
try {
  a.unshift(0);
} catch (e) {
  print(e instanceof TypeError);
}
// CHECK-NEXT: true
// Generic path on a plain object, and this coercion failure.
obj = {0: 'a', 1: 'b', length: 2};
print(Array.prototype.unshift.call(obj, 'z'),
      obj[0], obj[1], obj[2], obj.length);
// CHECK-NEXT: 3 z a b 3
try {
  Array.prototype.unshift.call(undefined, 1);
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
// Heap-allocated elements (strings, boxed doubles) are copied with write
// barriers by the fast path.
a = ['x', 1.5, 'y'];
print(a.slice(1, 3));
// CHECK-NEXT: 1.5,y
s = a.slice(0);
gc();
print(s);
// CHECK-NEXT: x,1.5,y
// The fast path must copy the storage, not alias it.
a = [1, 2, 3];
s = a.slice(0);
a[0] = 99;
s[2] = 'i';
print(s, a);
// CHECK-NEXT: 1,2,i 99,2,3
// Length extended past the storage (elemCount != len) must take the
// generic path.
a = [1, 2];
a.length = 4;
s = a.slice(0);
print(s.length, s[0], s[1], 2 in s);
// CHECK-NEXT: 4 1 2 false
// Mutating the array from valueOf during argument coercion must not use a
// stale fast path; the captured length is used with per-index HasProperty.
a = [1, 2, 3, 4, 5];
s = a.slice({valueOf: function() { a.length = 2; return 0; }});
print(s.length, s[0], s[1], 2 in s);
// CHECK-NEXT: 5 1 2 false
// Growing the array from valueOf also fails the fast-path check; the
// captured length bounds the result.
a = [1, 2, 3];
s = a.slice({valueOf: function() { a.push(4, 5); return 0; }});
print(s.length, s);
// CHECK-NEXT: 3 1,2,3
// Installing an accessor from valueOf clears fast index properties; the
// generic path calls the getter.
a = [1, 2, 3];
s = a.slice({valueOf: function() {
  Object.defineProperty(a, 1, {get: function() { return 'got'; }});
  return 0;
}});
print(s);
// CHECK-NEXT: 1,got,3
// Mutation from valueOf that keeps the same shape still permits the fast
// path, which must observe the new values.
a = [1, 2, 3];
s = a.slice({valueOf: function() { a[1] = 'mut'; return 1; }});
print(s);
// CHECK-NEXT: mut,3

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
// Length extended past the storage (elemCount != len) must take the
// generic path; the trailing holes are not visited.
a = [1, 2];
a.length = 4;
acc = [];
a.forEach(function(v, i) { acc.push(v, i); });
print(acc);
// CHECK-NEXT: 1,0,2,1
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
// Callback throws; iteration stops and the exception propagates.
a = [1, 2, 3];
acc = [];
try {
  a.forEach(function(v, i) {
    acc.push(v);
    if (i === 1)
      throw new Error('forEach boom');
  });
} catch (e) {
  print(acc, e.message);
}
// CHECK-NEXT: 1,2 forEach boom
// Callback triggers GC mid-iteration; the fast path must re-read the
// indexed storage on every iteration rather than caching a raw pointer.
a = [1.5, 'two', 3.5];
acc = [];
a.forEach(function(v) { gc(); acc.push(v); });
print(acc);
// CHECK-NEXT: 1.5,two,3.5
// Callback deletes an element ahead; the new hole is skipped because
// nothing on the prototype chain provides it.
a = [1, 2, 3];
acc = [];
a.forEach(function(v, i) { if (i === 0) delete a[2]; acc.push(v); });
print(acc);
// CHECK-NEXT: 1,2
// Non-array this takes the generic read path.
acc = [];
Array.prototype.forEach.call({0: 'a', 2: 'c', length: 3}, function(v, i) {
  acc.push(v, i);
});
print(acc);
// CHECK-NEXT: a,0,c,2

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
// Callback shrinks the array mid-iteration; missing indices become holes
// but the result keeps the original length.
a = [1, 2, 3, 4];
mp = a.map(function(x, i) { if (i === 0) a.length = 2; return x * 10; });
print(mp.length, mp[0], mp[1], 2 in mp, 3 in mp);
// CHECK-NEXT: 4 10 20 false false
// Callback throws; the exception propagates.
a = [1, 2, 3];
try {
  a.map(function(x, i) {
    if (i === 1)
      throw new Error('map boom');
    return x;
  });
} catch (e) {
  print(e.message);
}
// CHECK-NEXT: map boom
// Callback triggers GC mid-iteration.
a = [1.5, 'two', 3.5];
print(a.map(function(v) { gc(); return v; }));
// CHECK-NEXT: 1.5,two,3.5
// Callback installs an accessor mid-iteration; the emptied indexed slot is
// then read through the property path.
a = [1, 2, 3];
mp = a.map(function(v, i) {
  if (i === 0) {
    Object.defineProperty(
        a, 2, {get: function() { return 42; }, configurable: true});
  }
  return v;
});
print(mp);
// CHECK-NEXT: 1,2,42
// Non-array this takes the generic read path.
mp = Array.prototype.map.call({0: 2, 1: 3, length: 2}, function(v) {
  return v * v;
});
print(mp);
// CHECK-NEXT: 4,9

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
// Callback throws; the exception propagates.
a = [1, 2, 3];
try {
  a.filter(function(x, i) {
    if (i === 1)
      throw new Error('filter boom');
    return true;
  });
} catch (e) {
  print(e.message);
}
// CHECK-NEXT: filter boom
// Callback triggers GC mid-iteration.
a = [1.5, 'two', 3.5];
print(a.filter(function(v) { gc(); return true; }));
// CHECK-NEXT: 1.5,two,3.5
// Callback pollutes Array.prototype mid-iteration; the hole at index 1 is
// then read through the prototype.
a = [1, , 3];
var flt = a.filter(function(v, i) {
  if (i === 0)
    Array.prototype[1] = 'p1';
  return true;
});
delete Array.prototype[1];
print(flt);
// CHECK-NEXT: 1,p1,3

print('chained');
// CHECK-LABEL: chained
// Consecutive fast-path operations must leave the array in a consistent
// state (storage bounds, length property) for the next one.
a = [1, 2, 3, 4];
print(a.shift(), a, a.length);
// CHECK-NEXT: 1 2,3,4 3
print(a.unshift(0, 1), a, a.length);
// CHECK-NEXT: 5 0,1,2,3,4 5
print(a.slice(1, 3), a.map(function(x) { return x * 2; }));
// CHECK-NEXT: 1,2 0,2,4,6,8
print(a.shift(), a.pop(), a.push(9), a);
// CHECK-NEXT: 0 4 4 1,2,3,9

print('subclass');
// CHECK-LABEL: subclass
// Array subclass instances have a different parent than Array.prototype,
// so they must take the generic path. (Hermes does not implement
// Symbol.species: slice/map/filter return plain arrays on both paths.)
class MyArr extends Array {}
var m = MyArr.of(1, 2, 3);
print(m.shift(), '' + m);
// CHECK-NEXT: 1 2,3
print(m.unshift(0), '' + m);
// CHECK-NEXT: 3 0,2,3
print(m.slice(1), m.slice(1) instanceof MyArr,
      m.map(function(x) { return x; }) instanceof MyArr);
// CHECK-NEXT: 2,3 false false
acc = [];
m.forEach(function(v) { acc.push(v); });
print(acc, m.filter(function() { return true; }) instanceof MyArr);
// CHECK-NEXT: 0,2,3 false

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
