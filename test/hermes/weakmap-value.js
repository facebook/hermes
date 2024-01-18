/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -Xhermes-internal-test-methods %s | %FileCheck --match-full-lines %s

print("WeakMap value");
// CHECK-LABEL: WeakMap value
// Test that the GC correctly tracks modifications to WeakMap values that occur
// in the middle of a concurrent collection.
globalThis.sink = function () {}
var map = new WeakMap();
for (var i = 0; i < 10; ++i) {
    var keys = [];

    // Create a lot of entries in the map, and save the keys, so we can overwrite
    // them later.
    for (var j = 0; j < 1000; ++j){
        var key = {};
        map.set(key, {});
        keys.push(key);
    }

    // Save and overwrite the old value to make sure the GC is tracking it even
    // after it is overwritten.
    var oldVals = [];
    for (var k of keys) {
        oldVals.push(map.get(k));
        map.set(k, {});
    }

    // Try accessing the objects to make sure they haven't been garbage collected.
    for (var v of oldVals){
        sink(v.foo);
    }
}
