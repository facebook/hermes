/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-init-heap=4M -O -Xhermes-internal-test-methods %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes -gc-init-heap=4M -Xhermes-internal-test-methods %t.hbc | %FileCheck --match-full-lines %s

"use strict";

// When a key object is only reachable through a value of a WeakMap, it should
// be removed.
// http://www.ecma-international.org/ecma-262/#sec-weakmap-objects
// CHECK-LABEL: Start
print("Start");

// During review, there was a suggestion to use the IIFE
// ("immediately-invoked function expression") idiom to make the
// subtests "hermetic."  I tried this, but it didn't work: the intent
// of the functions here is to ensure that the original variables
// referring to the unreachable keys are no longer roots.  If I make
// these functions closures inside of parent closures, the Hermes
// compiler is smart enough, when optimizing, to inline them into the
// parent, causing the keys to remain reachable, defeating the
// purposes of the tests.
//
// We could revisit this if we get better about captured variable
// reachability.

function foo0() {
  // The value *is* the key
  var x = {};
  var map = new WeakMap();
  map.set(x, x);
  return map;
}
var outerMap = foo0();
gc();
// CHECK-NEXT: 0
print(HermesInternal.getWeakSize(outerMap));

// The value contains the key
function foo1() {
  // The value contains the key
  var x = {};
  var map = new WeakMap();
  map.set(x, { "foo": x });
  return map;
}
outerMap = foo1();
gc();
// CHECK-NEXT: 0
print(HermesInternal.getWeakSize(outerMap));

// The value is a function capturing a field containing the key
function foo2() {
  var map = new WeakMap();
  var x = {};
  var g = function() { print(x); }
  map.set(x, g);
  return map;
}
outerMap = foo2();
gc();
// CHECK-NEXT: 0
print(HermesInternal.getWeakSize(outerMap));

// A chain: a reachable key leads to a value used as a different key, and so on.
// Make sure such chains are successfully followed.
var reachableKey = {};
function foo3() {
  var map = new WeakMap();
  var x0 = {};
  var x1 = {};
  var x2 = {};
  var x3 = {};
  map.set(reachableKey, x0);
  map.set(x0, x1);
  map.set(x1, x2);
  map.set(x2, x3);
  map.set(x3, {});
  return map;
}
outerMap = foo3();
gc();
// CHECK-NEXT: 5
print(HermesInternal.getWeakSize(outerMap));

// This tests that WeakMaps reachable only via other WeakMaps are discovered
// and properly scanned.
function foo4() {
  var endOfChain = {};
  var map0 = new WeakMap();
  var map1 = new WeakMap();
  map0.set(reachableKey, map1);
  // Add a key/val cycle to map0, just to make sure it's reclaimed.
  var unreachable = {};
  map0.set(unreachable, unreachable);
  var map2 = new WeakMap();
  map1.set(reachableKey, map2);
  map2.set(reachableKey, endOfChain);
  // Now use endOfChain, which should be found reachable, as a key in map0.
  map0.set(endOfChain, {});
  // map0 should have two reachable keys.
  return map0;
}
outerMap = foo4();
gc();
// CHECK-NEXT: 2
print(HermesInternal.getWeakSize(outerMap));

// If a WeakMap is a key of another WeakMap, make sure things work
// correctly, whether the key WeakMap is reachable or not.
var reachableMapKey = new WeakMap();
function foo5a() {
  var map0 = new WeakMap();
  var x = {};
  var y = {};
  map0.set(reachableMapKey, x);
  reachableMapKey.set(x, y);
  map0.set(y, {});
  // reachableMapKey is reachable ==> x is reachable ==> y is
  // reachable.  So map0 should have two elements after a GC.
  return map0;
}
outerMap = foo5a();
gc();
// CHECK-NEXT: 2
print(HermesInternal.getWeakSize(outerMap));

function foo5b() {
  var unreachableMapKey = new WeakMap();
  var map0 = new WeakMap();
  var x = {};
  var y = {};
  map0.set(unreachableMapKey, x);
  unreachableMapKey.set(x, y);
  map0.set(y, {});
  // After we return, unreachableMapKey is not reachable ==> x is
  // unreachable ==> y is unreachable.
  // So map0 should have no elements after a GC.
  return map0;
}
outerMap = foo5b();
gc();
// CHECK-NEXT: 0
print(HermesInternal.getWeakSize(outerMap));

// This tests the case where a key is reachable only as a property of another
// WeakMap.
function foo6() {
  var key0 = {};
  var key1 = {};
  var map0 = new WeakMap();
  var map1 = new WeakMap();
  map0.set(key0, key1);
  map0.set(key1, {y: 17});
  map1.set({}, {});
  // Make key0 reachable, which should make key1 reachable in map0.
  map1.prop = key0;
  // This order was important to triggering a bug in a previous implementation.
  return [map1, map0];
}
var pair6 = foo6();
gc();
// CHECK-NEXT: 2
print(HermesInternal.getWeakSize(pair6[1]));

// This test ensures that if a WeakMap's key is cleared in a YG GC, during the
// next OG GC the value is also cleared.
function foo7() {
  function createWeakMap() {
    var v1 = {};
    var wm1 = new WeakMap();
    var v2 = {};
    wm1.set(v2, v1);
    var wm2 = new WeakMap();
    v1.a = wm2;
    // v2 will become unreachable after this function exits, and thus so will
    // v1 and wm2.
    return wm1;
  }
  var wm = createWeakMap();
  var arr = [0];
  for (var i = 0; i < 57; i++) {
    wm.set(arr, 0);
    arr = [0];
    arr[3262] = 0;
  }
  gc();
  // CHECK-NEXT: 1
  print(HermesInternal.getWeakSize(wm));
  // Make sure arr is used after the call to getWeakSize to ensure optimizations
  // don't change what keys are alive.
  return arr;
}
foo7();
