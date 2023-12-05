/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// Check that the date implementation does not use fma which can produce
// non-compliant output.
print(Date.UTC(1970, 0, 213503982336, 0, 0, 0, -18446744073709552000))
// CHECK: 34447360
