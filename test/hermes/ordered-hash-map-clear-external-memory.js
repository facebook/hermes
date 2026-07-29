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
// allocated in the GC heap). We need to correctly debit back the freed external
// memory for each clear().
function testClearDecreasesExternalBytes(collection, add) {
  for (var round = 0; round < 4; ++round) {
    for (var i = 0; i < 10000; ++i) {
      add(collection, i);
    }

    var beforeClear = externalBytes();
    collection.clear();
    var afterClear = externalBytes();
    print(afterClear < beforeClear);
  }
}

var map = new Map();
var set = new Set();

print('Map');
// CHECK-LABEL: Map
testClearDecreasesExternalBytes(map, function(map, value) {
  map.set(value, value);
});
// CHECK-NEXT: true
// CHECK-NEXT: true
// CHECK-NEXT: true
// CHECK-NEXT: true

print('Set');
// CHECK-LABEL: Set
testClearDecreasesExternalBytes(set, function(set, value) {
  set.add(value);
});
// CHECK-NEXT: true
// CHECK-NEXT: true
// CHECK-NEXT: true
// CHECK-NEXT: true

print(map.size, set.size);
// CHECK-NEXT: 0 0
