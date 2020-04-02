/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-init-heap=4M -O -Xhermes-internal-test-methods %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes -gc-init-heap=4M -Xhermes-internal-test-methods %t.hbc | %FileCheck --match-full-lines %s

// This test is specific to the implementation of GenGC.
// REQUIRES: gengc

"use strict"

// CHECK-LABEL: Start
print("Start");

// This tests that WeakMap marking works even in the presence of mark stack
// overflow.  (This test failed before a bug fix.)
function foo7() {
  function makeList(n, lastVal) {
    if (n == 0) {
      return {p: lastVal};
    } else {
      return {p: {}, tail: makeList(n-1, lastVal)};
    }
  }
  var key0 = {};
  var key1 = {};
  var map = new WeakMap();
  // 1000 is the mark stack limit; exceed that.
  map.set(key0, makeList(1200, key1));
  map.set(key1, {y: 17});
  return [map, key0];
}
var pair7 = foo7();
gc();

// Make sure a mark stack overflow occurred.
var stats = HermesInternal.getInstrumentedStats();
// CHECK-NEXT: 1
print(stats.js_markStackOverflows);

// CHECK-NEXT: 2
print(HermesInternal.getWeakSize(pair7[0]));
