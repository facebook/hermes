/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print(1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1 ** 1);
// CHECK: 1
