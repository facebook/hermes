/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -Xhermes-internal-test-methods -gc-sanitize-handles=0.1 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes -Xhermes-internal-test-methods -gc-sanitize-handles=0.1 %t.hbc | %FileCheck --match-full-lines %s

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
// CHECK-NEXT: caught TypeError WeakMap key must be an Object or non-registered Symbol
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
// Keep b alive here so that it won't get collected when calling getWeakSize().
  print(m.get(b));
// CHECK-NEXT: 12
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

print('preserveType');
// CHECK-LABEL: preserveType
const map = new WeakMap();
const key = {};
const value = "asd";
map.set(key, value);
// Ensure the value types are preserved across a GC.
gc();
print(typeof map.get(key))
// CHECK-NEXT: string

print('WeakMap Symbol key');
// CHECK-LABEL: WeakMap Symbol key
var m = new WeakMap();
var s = Symbol("test");
m.set(s, 11);
gc();
print(m.get(s));
// CHECK-NEXT: 11
print(m.get(Symbol("test")));
// CHECK-NEXT: undefined
var s2 = Symbol.for("test");
print(m.has(s2));
// CHECK-NEXT: false
try { m.set(s2, 11) } catch (e) { print('caught', e.name, e.message); }
// CHECK-NEXT: caught TypeError WeakMap key must be an Object or non-registered Symbol
print(m.delete(s));
// CHECK-NEXT: true
print(m.has(s));
// CHECK-NEXT: false


// Ensure some reuse occurred.
print(HermesInternal.getWeakSize(m) < 10000);
// CHECK-NEXT: true

print('upsert');
// CHECK-LABEL: upsert
var a = {};
var b = {};
var c = {};
var m = new WeakMap();

// getOrInsert inserts when absent and returns the value.
print(m.getOrInsert(a, 1));
// CHECK-NEXT: 1
// getOrInsert returns the existing value without overwriting.
print(m.getOrInsert(a, 99), m.get(a));
// CHECK-NEXT: 1 1

// getOrInsertComputed computes and inserts when absent.
print(m.getOrInsertComputed(b, (k) => 2));
// CHECK-NEXT: 2
// getOrInsertComputed returns the existing value without invoking the callback.
print(m.getOrInsertComputed(b, () => { throw 'should not run'; }));
// CHECK-NEXT: 2

// The computed value overwrites anything the callback inserts for the key.
print(m.getOrInsertComputed(c, (k) => { m.set(k, 'mutated'); return 'computed'; }));
// CHECK-NEXT: computed
print(m.get(c));
// CHECK-NEXT: computed

// The computed value is stored even if the callback deletes the key it set.
var d = {};
print(m.getOrInsertComputed(d, (k) => { m.set(k, 'temp'); m.delete(k); return 'computed'; }));
// CHECK-NEXT: computed
print(m.get(d));
// CHECK-NEXT: computed

// The callback may delete a pre-existing key, and that deletion persists.
var existing = {};
var f = {};
m.set(existing, 'exists');
print(m.getOrInsertComputed(f, (k) => { m.delete(existing); return 'computed'; }));
// CHECK-NEXT: computed
print(m.get(f), m.has(existing));
// CHECK-NEXT: computed false

// When the key already exists, the callback never runs, so a callback that
// would delete the key has no effect: the existing value is returned and
// the entry is left intact.
var g = {};
m.set(g, 'original');
print(m.getOrInsertComputed(g, (k) => { m.delete(k); return 'computed'; }));
// CHECK-NEXT: original
print(m.get(g), m.has(g));
// CHECK-NEXT: original true

// Symbol keys work too.
var s = Symbol('k');
print(m.getOrInsert(s, 7), m.get(s));
// CHECK-NEXT: 7 7

// Entries survive a GC.
gc();
print(m.get(a), m.get(b), m.get(c), m.get(s));
// CHECK-NEXT: 1 2 computed 7

// Keys that cannot be held weakly throw.
try { m.getOrInsert(1, 1) } catch (e) { print('caught', e.name, e.message) }
// CHECK-NEXT: caught TypeError WeakMap key must be an Object or non-registered Symbol
try { m.getOrInsertComputed(1, () => 1) } catch (e) { print('caught', e.name, e.message) }
// CHECK-NEXT: caught TypeError WeakMap key must be an Object or non-registered Symbol

// Non-callable callback throws, even when the key is already present.
try { m.getOrInsertComputed(a, 5) } catch (e) { print('caught', e.name) }
// CHECK-NEXT: caught TypeError

// Non-WeakMap receiver throws.
try { WeakMap.prototype.getOrInsert.call([], a, 1) } catch (e) { print('caught', e.name, e.message) }
// CHECK-NEXT: caught TypeError WeakMap.prototype.getOrInsert can only be called on a WeakMap
try { WeakMap.prototype.getOrInsertComputed.call([], a, () => 1) } catch (e) { print('caught', e.name, e.message) }
// CHECK-NEXT: caught TypeError WeakMap.prototype.getOrInsertComputed can only be called on a WeakMap
