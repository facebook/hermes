/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// REQUIRES: intl

// Ensures Hermes Intl's NumberFormat correctly formats BigInt inputs.
var nf = new Intl.NumberFormat("en", undefined);

print(nf.format(0n));
// CHECK: 0
print(nf.format(2n));
// CHECK-NEXT: 2
print(nf.format(-42n));
// CHECK-NEXT: -42
print(nf.format(9007199254740993n));
// CHECK-NEXT: 9,007,199,254,740,993
print(nf.format(123456789012345678901234567890n));
// CHECK-NEXT: 123,456,789,012,345,678,901,234,567,890
print(nf.format(-99999999999999999999n));
// CHECK-NEXT: -99,999,999,999,999,999,999
