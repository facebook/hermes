/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// Regression test: RegExp.prototype[Symbol.replace] truncated uint64 nCaptures
// to uint32 when passing it to ArrayStorageSmall::create(), which takes
// uint32_t capacity. A custom exec() returning an object with
// length > UINT32_MAX triggers the truncation. The fix throws a RangeError.

var rx = {
  [Symbol.replace]: RegExp.prototype[Symbol.replace],
  global: false,
  exec: function() {
    return {0: "a", length: 4294967297, index: 0, groups: undefined};
  }
};
try {
  "hello".replace(rx, "world");
} catch (e) {
  print(e.name);
}
// CHECK: RangeError
