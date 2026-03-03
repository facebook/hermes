/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// Regression test: Array.prototype.sort passed uint64_t length to quickSort(),
// which takes uint32_t, silently truncating the sort range. No sort path can
// handle lengths exceeding UINT32_MAX, so a RangeError is thrown early.

var obj = {};
obj[0] = "b";
obj[1] = "a";
Object.defineProperty(obj, "length", {value: 4294967297});
try {
  Array.prototype.sort.call(obj);
} catch (e) {
  print(e.name);
}
// CHECK: RangeError
