/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xmicrotask-queue -O -gc-init-heap=4M -gc-sanitize-handles=0 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec -O -Wx,-Xmicrotask-queue,-gc-init-heap=4M,-gc-sanitize-handles=0 %s | %FileCheck --match-full-lines %s
// REQUIRES: gc_hades

"use strict";

print("weakref deref read barrier");
// CHECK-LABEL: weakref deref read barrier
// CHECK-NEXT: alive

// We need an object to hold the reference returned by deref().
var holder = {field: null, ballast: []};

var target = {marker: "alive"};
var weak = new WeakRef(target);
var roots = [];

function numYGCollections() {
  var stats = HermesInternal.getInstrumentedStats();
  return stats.js_gcSpecific.js_numYGCollections;
}

function allocateUntilYoungCollection() {
  var start = numYGCollections();
  while (numYGCollections() === start) {
    var arr = [];
    arr[3262] = 0;
    roots.push(arr);
  }
}

function giveMarkerTimeToScanHolder() {
  var sum = 0;
  for (var i = 0; i < 10000000; ++i) {
    sum += i;
  }
  if (sum === -1) {
    print(sum);
  }
}

// Promote the WeakRef, target, and holder before the target is weak-only.
allocateUntilYoungCollection();
giveMarkerTimeToScanHolder();
// The above YG collection will start an OG collection. Start another YG
// collection to have a chance to do weak marking and complete the OG collection.
allocateUntilYoungCollection();

setTimeout(function() {
  // Each task starts after ClearKeptObjects, so the target is weak-only here.
  target = null;

  // Start OG marking after target is weak-only, then give the concurrent
  // marker time to scan holder before storing the weakly-held object into it.
  allocateUntilYoungCollection();
  giveMarkerTimeToScanHolder();

  // If holder is already scanned, without correct read barrier in `deref()`,
  // the pointed object may be swept and holder.field may point to a freed
  // cell.
  holder.field = weak.deref();
  // The GC hasn't reached the CompleteMarking phase, so this should not be
  // undefined.
  if (holder.field === undefined) {
    print("collected before deref()");
    return;
  }

  setTimeout(function() {
    // Sweep the dead objects.
    allocateUntilYoungCollection();
    // Give some time for the sweeping to complete.
    giveMarkerTimeToScanHolder();

    // If the object pointed by holder.field is freed, we will see a crash in
    // debug build since the freed cell can't be casted to JSObject.
    print(holder.field.marker);
  }, 0);
}, 0);
