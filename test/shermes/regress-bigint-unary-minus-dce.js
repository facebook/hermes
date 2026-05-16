/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec -O0 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec -O %s | %FileCheck --match-full-lines %s

(function() {
  let a = BigInt.asUintN(65472, -1n);
  let b = -a;

  try {
    -b;
  } catch (e) {
    print(e);
  }
})();

// CHECK: RangeError: Maximum BigInt size exceeded
