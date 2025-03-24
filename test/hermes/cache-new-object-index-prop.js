/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// Test that we handle index-like properties correctly in CacheNewObject.
function MyConstructor(x, y, z) {
  this.x = x;
  this.y = y;
  this.z = z;
  this["0"] = 0;
}

(function () {
  let obj = new MyConstructor(1, 2, 3);
  print(JSON.stringify(obj));
  // CHECK: {"0":0,"x":1,"y":2,"z":3}
})();
