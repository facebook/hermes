/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// Regression test: Array.prototype.join truncated uint64 length to uint32 when
// allocating storage, causing an OOB write for lengths > 2^32.

try {
  Array.prototype.join.call({length: 4294967297, 1: "x"});
} catch (e) {
  print(e.name);
}
// CHECK: RangeError
