/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O -gc-sanitize-handles=0 %s | %FileCheck --match-full-lines %s
// Test objects being used as a map with a large number of random access keys.

// The number of keys in an object to be considered big for this test.
var NUM_KEYS = 10000;

function getBigObject() {
  var x = {};
  for (var i = 0; i < NUM_KEYS; i++) {
    // Make sure the integer becomes a string so that it doesn't do an array
    // optimization.
    x["" + i] = i;
  }
  return x;
}

var x = getBigObject();
sum = 0;
for (var key in x) {
  sum += x[key];
}
print(sum);
// CHECK: 49995000
