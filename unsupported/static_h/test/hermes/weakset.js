/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -Xhermes-internal-test-methods %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -O -emit-binary -out %t.hbc %s && %hermes -Xhermes-internal-test-methods %t.hbc | %FileCheck --match-full-lines %s

print("WeakSet");
// CHECK-LABEL: WeakSet
print(WeakSet.prototype)
// CHECK-NEXT: [object WeakSet]
print(WeakSet.length)
// CHECK-NEXT: 0
print(new WeakSet());
// CHECK-NEXT: [object WeakSet]

var x = {}
var y = {}
var s = new WeakSet(new Set([x]));
print(s.has(x), s.has(y));
// CHECK-NEXT: true false

print('has');
// CHECK-LABEL: has
var a = {};
var b = {x:'abc'};
var c = {y:3};
var m = new WeakSet([a,b]);
print(m.has(a), m.has(b), m.has(c));
// CHECK-NEXT: true true false
gc();
print(m.has(a), m.has(b), m.has(c));
// CHECK-NEXT: true true false
gc();
print(m.has(a), m.has(b), m.has(c));
// CHECK-NEXT: true true false
gc();
print(m.has(a), m.has(b), m.has(c));
// CHECK-NEXT: true true false
print(m.has(1));
// CHECK-NEXT: false
try { WeakSet.prototype.has.call([], a) } catch(e) { print('caught', e.name, e.message) }
// CHECK-NEXT: caught TypeError WeakSet.prototype.has can only be called on a WeakSet

print('add');
// CHECK-LABEL: add
var a = {};
var b = {x:'a'};
var m = new WeakSet();
m.add(a);
print(m.has(a), m.has(b));
// CHECK-NEXT: true false
m.add(b);
print(m.has(a), m.has(b));
// CHECK-NEXT: true true
try { m.add(1, 2) } catch(e) { print('caught', e.name, e.message) }
// CHECK-NEXT: caught TypeError WeakSet key must be an Object
try { WeakSet.prototype.add.call([], a, 3) } catch(e) { print('caught', e.name, e.message) }
// CHECK-NEXT: caught TypeError WeakSet.prototype.add can only be called on a WeakSet

print('delete');
// CHECK-LABEL: delete
var a = {};
var b = {x:1};
var m = new WeakSet();
m.add(a);
print(m.has(a), m.has(b));
// CHECK-NEXT: true false
print(m.delete(a), m.delete(b));
// CHECK-NEXT: true false
print(m.delete(a), m.delete(b));
// CHECK-NEXT: false false
print(m.delete(1));
// CHECK-NEXT: false
try { WeakSet.prototype.delete.call([], a) } catch(e) { print('caught', e.name, e.message) }
// CHECK-NEXT: caught TypeError WeakSet.prototype.delete can only be called on a WeakSet

print('gc');
// CHECK-LABEL: gc
var a = {};
var c = {};
var m = new WeakSet();
m.add(a);
(function() {
  var b = {};
  m.add(b);
  print(HermesInternal.getWeakSize(m));
// CHECK-NEXT: 2
})();
// b can be freed now.
// Run the GC twice - first to invalidate the WeakRef,
// second to delete the invalid WeakRef from the map.
gc();
gc();
print(m.has(a));
// CHECK-NEXT: true

print(HermesInternal.getWeakSize(m));
// CHECK-NEXT: 1
