/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-max-heap=8M -gc-init-heap=8M -max-register-stack=7000000 %s | %FileCheck --match-full-lines %s

// A result too large for the heap must throw a RangeError, whether its code
// units turn out to be ASCII or not. 6M code units fit in the 8M heap as
// ASCII but not as UTF-16. A typed array supplies them without allocating
// on the heap.

print('fromCharCode');
// CHECK-LABEL: fromCharCode

var N = 6000000;
print(String.fromCharCode.apply(null, new Uint8Array(N).fill(97)).length);
// CHECK-NEXT: 6000000

try {
  String.fromCharCode.apply(null, new Uint8Array(N).fill(233));
  print('no exception');
} catch (e) {
  print(e);
}
// CHECK-NEXT: RangeError: Cannot allocate an external string primitive.
