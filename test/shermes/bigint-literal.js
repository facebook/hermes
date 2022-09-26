/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

print('BigInt Literal ==');
// CHECK-LABEL: BigInt Literal ==

print(0n);
// CHECK-NEXT: 0
print(1n);
// CHECK-NEXT: 1
print(1234567891234678912345789123456789123456789n);
// CHECK-NEXT: 1234567891234678912345789123456789123456789
