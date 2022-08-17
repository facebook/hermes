/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -Xhermes-internal-test-methods %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes -Xhermes-internal-test-methods %t.hbc | %FileCheck --match-full-lines %s

print("WeakMap");
// CHECK-LABEL: WeakMap
print(WeakMap.prototype)
// CHECK-NEXT: [object WeakMap]
print(WeakMap.length)
// CHECK-NEXT: 0
print(new WeakMap());
// CHECK-NEXT: [object WeakMap]
var x = {}
var y = {}
var m = new WeakMap(new Set([[x,2], [y,4]]));
print(m.get(x), m.get(y));
// CHECK-NEXT: 2 4

print('get/has');
// CHECK-LABEL: get/has
var a = {};
var b = {x:'abc'};
var c = {y:3};
var m = new WeakMap([[a, 0x123], [b, 0xbeef]]);
print(m.get(a), m.get(b), m.get(c));
// CHECK-NEXT: 291 48879 undefined
print(m.has(a), m.has(b), m.has(c));
// CHECK-NEXT: true true false
gc();
print(m.get(a), m.get(b), m.get(c));
// CHECK-NEXT: 291 48879 undefined
gc();
print(m.get(a), m.get(b), m.get(c));
// CHECK-NEXT: 291 48879 undefined
gc();
print(m.get(a), m.get(b), m.get(c));
// CHECK-NEXT: 291 48879 undefined
print(m.get(1));
// CHECK-NEXT: undefined
print(m.has(1));
// CHECK-NEXT: false
try { WeakMap.prototype.get.call([], a) } catch(e) { print('caught', e.name, e.message) }
// CHECK-NEXT: caught TypeError WeakMap.prototype.get can only be called on a WeakMap
try { WeakMap.prototype.has.call([], a) } catch(e) { print('caught', e.name, e.message) }
// CHECK-NEXT: caught TypeError WeakMap.prototype.has can only be called on a WeakMap

print('set');
// CHECK-LABEL: set
var a = {};
var b = {x:'a'};
var m = new WeakMap();
m.set(a, 12);
print(m.get(a), m.get(b));
// CHECK-NEXT: 12 undefined
m.set(b, 88);
print(m.get(a), m.get(b));
// CHECK-NEXT: 12 88
try { m.set(1, 2) } catch(e) { print('caught', e.name, e.message) }
// CHECK-NEXT: caught TypeError WeakMap key must be an Object
try { WeakMap.prototype.set.call([], a, 3) } catch(e) { print('caught', e.name, e.message) }
// CHECK-NEXT: caught TypeError WeakMap.prototype.set can only be called on a WeakMap

print('delete');
// CHECK-LABEL: delete
var a = {};
var b = {x:1};
var m = new WeakMap();
m.set(a, 12);
print(m.get(a), m.get(b));
// CHECK-NEXT: 12 undefined
print(m.delete(a), m.delete(b));
// CHECK-NEXT: true false
print(m.delete(a), m.delete(b));
// CHECK-NEXT: false false
print(m.delete(1));
// CHECK-NEXT: false
try { WeakMap.prototype.delete.call([], a) } catch(e) { print('caught', e.name, e.message) }
// CHECK-NEXT: caught TypeError WeakMap.prototype.delete can only be called on a WeakMap
var c = {};
var d = {};
var e = {};
var f = {};
m.set(c, 1);
m.set(d, 1);
m.set(e, 1);
m.set(f, 1);
print(m.delete(e), m.delete(c));
// CHECK-NEXT: true true
m.set(c, 1);
print(m.get(c));
// CHECK-NEXT: 1
m.set(e, 2);

print('gc');
// CHECK-LABEL: gc
var a = {};
var m = new WeakMap();
m.set(a, 10);
(function() {
  var b = {};
  m.set(b, 12);
  print(HermesInternal.getWeakSize(m));
// CHECK-NEXT: 2
  for (var i = 0; i < 10000; ++i) {
    m.set({}, 12);
  }
  print(m.get(a));
// CHECK-NEXT: 10
})();
// b can be freed now.
// Run the GC twice - first to invalidate the WeakRef,
// second to delete the invalid WeakRef from the map.
print(m.get(a));
// CHECK-NEXT: 10
gc();
print(m.get(a));
// CHECK-NEXT: 10
gc();
print(m.get(a));
// CHECK-NEXT: 10

// Ensure some reuse occurred.
print(HermesInternal.getWeakSize(m) < 10000);
// CHECK-NEXT: true
