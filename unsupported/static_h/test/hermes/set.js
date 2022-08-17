/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print("Set prototype and constructor");
//CHECK-LABEL: Set prototype and constructor

print(Set.prototype);
//CHECK-NEXT: [object Set]
print(Set.length);
//CHECK-NEXT: 0

print(Object.getOwnPropertyNames(Set.prototype).sort(function(a,b) {
  if (a < b) return -1;
  if (a === b) return 0;
  if (a > b) return 1;
}));
//CHECK-NEXT: add,clear,constructor,delete,entries,forEach,has,keys,size,values
print(Set.prototype.keys === Set.prototype.values);
//CHECK-NEXT: true
print(Set.prototype.keys.name, Set.prototype.values.name);
//CHECK-NEXT: values values

print(new Set());
//CHECK-NEXT: [object Set]
var s = new Set(new Set([1, 2]));
print(s.has(1), s.has(2), s.has(3));
// CHECK-NEXT: true true false

try {
  Set();
} catch (e) {
  print(e);
}
//CHECK-NEXT: TypeError: Constructor Set requires 'new'

try{ Set.prototype.clear(); } catch (e) { print(e); }
//CHECK-NEXT: TypeError: Non-Set object called on Set.prototype.clear

var s = new Set([1, 2, 3, 4, 5]);
s.forEach(Error);
print(s.size);
//CHECK-NEXT: 5

// Make sure we can insert internal object ID into a frozen object.
var fo = {};
Object.freeze(fo);
var fs = new Set();
fs.add(fo);

function testAdd(o1, o2, o3) {
  var s = new Set();
  print("testAdd");
//CHECK-LABEL: testAdd
  print(s.size);
//CHECK-NEXT: 0
  s.add(undefined);
  s.add("abc");
  s.add("def");
  s.add("abc");
  s.add("def");
  s.add(1/-Infinity);
  s.add(o1);
  s.add(o2);
  s.add(o3);
  var o11 = o1, o22 = o2, o33 = o3;
  s.add(o11);
  s.add(o22);
  s.add(o33);
  print(s.size);
  //CHECK-NEXT: 7
  return s;
}

function testHas(s, o1, o2, o3) {
  print("testHas");
//CHECK-LABEL: testHas
  print(s.has(0));
//CHECK-NEXT: true
  print(s.has(1));
//CHECK-NEXT: false
  print(s.has("abc"));
//CHECK-NEXT: true
  print(s.has("a"));
//CHECK-NEXT: false
  print(s.has("def"));
//CHECK-NEXT: true
  print(s.has());
//CHECK-NEXT: true
  print(s.has(null));
//CHECK-NEXT: false
  print(s.has(o1));
//CHECK-NEXT: true
  print(s.has(o2));
//CHECK-NEXT: true
  print(s.has(o3));
//CHECK-NEXT: true
  print(s.has({}));
//CHECK-NEXT: false
  var o4 = o2;
  print(s.has(o4));
//CHECK-NEXT: true
}

function testDelete(s, o1, o2, o3) {
  print("testDelete");
//CHECK-LABEL: testDelete
  print(s.delete(0));
//CHECK-NEXT: true
  print(s.size);
//CHECK-NEXT: 6
  print(s.delete(1));
//CHECK-NEXT: false
  print(s.delete("abc"));
//CHECK-NEXT: true
  print(s.size);
//CHECK-NEXT: 5
  print(s.delete("a"));
//CHECK-NEXT: false
  print(s.delete("def"));
//CHECK-NEXT: true
  print(s.size);
//CHECK-NEXT: 4
  print(s.delete());
//CHECK-NEXT: true
  print(s.size);
//CHECK-NEXT: 3
  print(s.delete(null));
//CHECK-NEXT: false
  print(s.delete(o1));
//CHECK-NEXT: true
  print(s.size);
//CHECK-NEXT: 2
  var o4 = o2;
  print(s.delete(o4));
//CHECK-NEXT: true
  print(s.size);
//CHECK-NEXT: 1
  print(s.delete(o3));
//CHECK-NEXT: true
  print(s.size);
//CHECK-NEXT: 0
  print(s.delete({}));
//CHECK-NEXT: false

  print(s.has(0));
//CHECK-NEXT: false
  print(s.has("abc"));
//CHECK-NEXT: false
  print(s.has("def"));
//CHECK-NEXT: false
  print(s.has());
//CHECK-NEXT: false
  print(s.has(o1));
//CHECK-NEXT: false
  print(s.has(o2));
//CHECK-NEXT: false
  print(s.has(o3));
//CHECK-NEXT: false
}

function testClear(o1, o2, o3) {
  var s = testAdd(o1, o2, o3);

  print("testClear");
//CHECK-LABEL: testClear
  print(s.size);
//CHECK-NEXT: 7
  s.clear();
  print(s.size);
//CHECK-NEXT: 0
}

function testIteration() {
  s = new Set();
  s.add(1);
  s.add({});
  s.add(undefined);
  s.add("abc");

  print("testIteration");
//CHECK-LABEL: testIteration
  var keys = s.keys();
  print(keys.__proto__);
//CHECK-NEXT: [object Set Iterator]
  print(keys[Symbol.iterator]());
//CHECK-NEXT: [object Set Iterator]
  print(Object.getOwnPropertyNames(keys.__proto__));
//CHECK-NEXT: next
  while (true) {
    var e = keys.next();
    print(e.value);
    if (e.done) break;
  }
//CHECK-NEXT: 1
//CHECK-NEXT: [object Object]
//CHECK-NEXT: undefined
//CHECK-NEXT: abc
//CHECK-NEXT: undefined

  var values = s.values();
  while (true) {
    var e = values.next();
    print(e.value);
    if (e.done) break;
  }
//CHECK-NEXT: 1
//CHECK-NEXT: [object Object]
//CHECK-NEXT: undefined
//CHECK-NEXT: abc
//CHECK-NEXT: undefined

  var entries = s.entries();
  while (true) {
    var e = entries.next();
    print(e.value);
    if (e.done) break;
  }
//CHECK-NEXT: 1,1
//CHECK-NEXT: [object Object],[object Object]
//CHECK-NEXT: ,
//CHECK-NEXT: abc,abc
//CHECK-NEXT: undefined
}

function testForEach() {
  function callbackFn(key, value, s) {
    print(key, value, s.size);
  }
  s = new Set();
  s.add(1);
  s.add({});
  s.add(undefined);
  s.add("abc");

  print("testForEach");
//CHECK-LABEL: testForEach
  s.forEach(callbackFn);
//CHECK-NEXT: 1 1 4
//CHECK-NEXT: [object Object] [object Object] 4
//CHECK-NEXT: undefined undefined 4
//CHECK-NEXT: abc abc 4
}

function testMutatedIteration() {
  print("testMutatedIteration");
//CHECK-LABEL: testMutatedIteration
  var s = new Set();
  s.add(0);
  var keys = s.keys();
  for (var i = 0; i < 5; ++i) {
    print(keys.next().value);
    s.delete(0);
    s.add(0);
  }
//CHECK-NEXT: 0
//CHECK-NEXT: 0
//CHECK-NEXT: 0
//CHECK-NEXT: 0
//CHECK-NEXT: 0
  s = new Set();
  for (var i = 1; i < 10; ++i) s.add(i);
  s.forEach(function(k, e, ss) { print(k); ss.clear(); });
  print(s.size);
//CHECK-NEXT: 1
//CHECK-NEXT: 0
}

function testRehash() {
  print("testRehash");
//CHECK-LABEL: testRehash

  // Simple expand
  var s = new Set();
  for (var i = 0; i < 100; ++i) s.add(i);
  for (var i = 0; i < 100; ++i) {
    if (!s.has(i)) throw new Error();
  }
  print(s.size);
//CHECK-NEXT: 100

  // Simple shrink
  for (var i = 0; i < 100; ++i) s.delete(i);
  print(s.size);
//CHECK-NEXT: 0

  // Iterating and adding
  var keys = s.keys();
  for (var i = 0; i < 100; ++i) {
    s.add(i);
  }
  for (var i = 0; i < 100; ++i) {
    if (keys.next().value != i) throw new Error();
  }

  // Iterating and deleting
  keys = s.keys();
  for (var i = 0; i < 50; ++i) s.delete(i);
  print(keys.next().value);
//CHECK-NEXT: 50
  s.clear();
  s.add(0);
  print(keys.next().value);
//CHECK-NEXT: 0

  s = new Set();
  s.add(0);
  keys = s.keys();
  keys.next(); // keys now pointing to 0.
  s.clear();
  s.add(1);
  print(keys.next().value);
//CHECK-NEXT: 1
  s.delete(1);
  s.add(2);
  print(keys.next().value);
//CHECK-NEXT: 2
}

function testZero() {
  print("testZero");
//CHECK-LABEL: testZero

  var s = new Set();
  s.add(-0);
  print(s.has(-0));
  print(s.has(+0));
  print(Object.is(-0, [...s][0]));
  print(Object.is(+0, [...s][0]));
//CHECK-NEXT: true
//CHECK-NEXT: true
//CHECK-NEXT: false
//CHECK-NEXT: true

  var s = new Set();
  s.add(+0);
  print(s.has(-0));
  print(s.has(+0));
  print(Object.is(-0, [...s][0]));
  print(Object.is(+0, [...s][0]));
//CHECK-NEXT: true
//CHECK-NEXT: true
//CHECK-NEXT: false
//CHECK-NEXT: true

  // -0 at both key and value should be normalized to +0.
  var s = new Set();
  s.add(-0);
  var [k, v] = s.entries().next().value;
  print(Object.is(-0, k));
  print(Object.is(-0, v));
//CHECK-NEXT: false
//CHECK-NEXT: false

  // delete -0 should work
  print(s.delete(-0));
//CHECK-NEXT: true
  print(s.size === 0);
//CHECK-NEXT: true
}

var s = new Set();
var o1 = {};
var o2 = {a: 1};
var o3 = {a: 1, b: 2};
var s = testAdd(o1, o2, o3);
testHas(s, o1, o2, o3);
testDelete(s, o1, o2, o3);
testClear(o1, o2, o3);
testIteration();
testForEach();
testMutatedIteration();
testRehash();
testZero();

print("constructor");
//CHECK-LABEL: constructor

var s = new Set([])
print(s.size)
//CHECK-NEXT: 0

s = new Set([,10,20,,,30,10,,]);
print(s.size);
//CHECK-NEXT: 4
var it = s.values();
var t;
t = it.next(); print(t.value, t.done);
//CHECK-NEXT: undefined false
t = it.next(); print(t.value, t.done);
//CHECK-NEXT: 10 false
t = it.next(); print(t.value, t.done);
//CHECK-NEXT: 20 false
t = it.next(); print(t.value, t.done);
//CHECK-NEXT: 30 false
t = it.next(); print(t.value, t.done);
//CHECK-NEXT: undefined true

var oldAdd = Set.prototype.add;
Set.prototype.add = 10;
try { new Set([1,2,3]) } catch (e) { print('caught', e.name) }
// CHECK-NEXT: caught TypeError
var arr = [[1,2], [3,4]];
Set.prototype.add = function(v) {
  oldAdd.call(this, v);
  arr.length = 1;
}
print(new Set(arr).size);
// CHECK-NEXT: 1
Set.prototype.add = function(v) {
  oldAdd.call(this, v);
  Set.prototype.add = 10;
}
var s = new Set([1,2,3]);
print(s.size, s.has(1));
// CHECK-NEXT: 3 true
Set.prototype.add = oldAdd;
