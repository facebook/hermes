/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

var x = {
  10n: 'a',
  1_1n: 'b',
};

print("bigint as object key");
// CHECK-LABEL: bigint as object key

print(x[10]);
// CHECK-NEXT: a
print(x[11]);
// CHECK-NEXT: b

