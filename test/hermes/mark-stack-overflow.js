/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-init-heap=4M -O -Xhermes-internal-test-methods -gc-sanitize-handles=0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes -gc-init-heap=4M -Xhermes-internal-test-methods -gc-sanitize-handles=0 %t.hbc | %FileCheck --match-full-lines %s

// This test is specific to the implementation of GenGC.
// REQUIRES: gengc

"use strict"

// CHECK-LABEL: Start
print("Start");

function makeList(n, lastVal) {
  if (n == 0) {
    return {p: lastVal, tail: null};
  } else {
    // It's important to allocate the p and tail values before allocating the
    // object that will be returned, so the pointers point to lower addresses,
    // and therefore will be pushed on the mark stack.
    var pVal = {};
    var tailVal = makeList(n-1, lastVal);
    return {p: pVal, tail: tailVal};
  }
}

function findEnd(l) {
  if (l.tail == null) {
    return l.p;
  } else {
    return findEnd(l.tail);
  }
}

// The mark stack limit is 1000.
var l = makeList(1500, {v: 17});
var stats = HermesInternal.getInstrumentedStats();
// CHECK-NEXT: 0
print(stats.js_markStackOverflows);
gc();
stats = HermesInternal.getInstrumentedStats();
// CHECK-NEXT: 1
print(stats.js_markStackOverflows);

// Make sure it's cumulative.
gc();
stats = HermesInternal.getInstrumentedStats();
// CHECK-NEXT: 2
print(stats.js_markStackOverflows);

// Check that the list is still correct.
// CHECK-NEXT: 17
print(findEnd(l).v);
