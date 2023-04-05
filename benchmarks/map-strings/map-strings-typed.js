/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

(function main() {

const mapPrototypeGet: any = Map.prototype.get;
const mapPrototypeSet: any = Map.prototype.set;

// Setup

var arr1: string[] = [];
var arr2: string[] = [];
var i: number = 0;
for (i = 0; i < 1000; ++i) {
  // Ensure these are different strings.
  arr1.push('key' + String(i));
  arr2.push('key' + String(i));
}

// Bench

var t1 = Date.now();

var N: number = 10000;
var sum: number = 0;
for (var n: number = 0; n < N; ++n) {
  var map: any = new Map();
  for (i = 0; i < 1000; ++i) {
    $SHBuiltin.call(mapPrototypeSet, map, arr1[i], i);
  }
  for (i = 0; i < 1000; ++i) {
    sum = sum + $SHBuiltin.call(mapPrototypeGet, map, arr2[i]);
  }
}
print(Date.now() - t1, "ms", N, "iterations");
print(sum);
})();
