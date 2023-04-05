/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

(function main() {

// Setup

var arr = [];
var i = 0;
for (i = 0; i < 1000; ++i) {
  arr.push({});
}

// Bench

var t1 = Date.now();

var N = 10000;
var sum = 0;
for (var n = 0; n < N; ++n) {
  var map = new Map();
  for (i = 0; i < 1000; ++i) {
    map.set(arr[i], i);
  }
  for (i = 0; i < 1000; ++i) {
    sum = sum + map.get(arr[i]);
  }
}
print(Date.now() - t1, "ms", N, "iterations");
print(sum);

})();
