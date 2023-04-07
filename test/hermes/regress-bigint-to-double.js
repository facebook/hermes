/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict -O %s | %FileCheck %s
// RUN: %hermes -non-strict -O0 %s | %FileCheck %s

// TODO: fix BigInt to number conversion (it should never result in NaN).
print(Number(0xffffffffffffffffffffffffffffffffn));
// CHECK: 3.402823669209385e+38
