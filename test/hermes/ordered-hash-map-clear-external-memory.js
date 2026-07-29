/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

function externalBytes() {
  return HermesInternal.getInstrumentedStats().js_externalBytes;
}

// The underlying implementation for Map/Set uses external memory (i.e., not
// allocated in the GC heap). After each clear() the collection is empty and
// back at its initial capacity, so its external footprint must return to the
// baseline captured before the very first refill -- regardless of how many
// clear()+refill cycles have run. The bug being fixed stranded (capacity -
// kInitialCapacity) * sizeof(uint32_t) bytes per cycle, so the empty-collection
// footprint crept upward without bound even though the live set stayed
// constant.
//
// We compare afterClear against that pre-refill baseline (NOT against
// beforeClear): a clear() that debits, say, only 90% and leaks 10% every cycle
// keeps afterClear well below beforeClear on every round, yet the leaked
// residual accumulates above the baseline and grows without bound. We flag any
// residual that exceeds 1% of the memory grown during the refill; scaling the
// tolerance to the refill size (rather than using an absolute bound) keeps the
// check immune to the small (<1KB), GC-timing-dependent bookkeeping noise from
// unrelated external memory when a Map and Set coexist, while still catching a
// 10%-per-cycle leak on the very first round.
function testClearReturnsToBaseline(collection, add) {
  var baseline = externalBytes();
  var returnsToBaselineEveryRound = true;
  for (var round = 0; round < 4; ++round) {
    for (var i = 0; i < 10000; ++i) {
      add(collection, i);
    }

    var beforeClear = externalBytes();
    collection.clear();
    var afterClear = externalBytes();

    var grown = beforeClear - baseline;
    var residual = afterClear - baseline;
    if (residual * 100 > grown)
      returnsToBaselineEveryRound = false;
  }
  print('returnsToBaselineEveryRound:', returnsToBaselineEveryRound);
}

var map = new Map();
var set = new Set();

print('Map');
// CHECK-LABEL: Map
testClearReturnsToBaseline(map, function(map, value) {
  map.set(value, value);
});
// CHECK-NEXT: returnsToBaselineEveryRound: true

print('Set');
// CHECK-LABEL: Set
testClearReturnsToBaseline(set, function(set, value) {
  set.add(value);
});
// CHECK-NEXT: returnsToBaselineEveryRound: true

print(map.size, set.size);
// CHECK-NEXT: 0 0
