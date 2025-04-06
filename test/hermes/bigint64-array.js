/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

// Test that TypedArray.prototype.at() returns a bigint.

print(typeof new BigUint64Array(1).at(0), typeof new BigInt64Array(1).at(0));
// CHECK: bigint bigint
