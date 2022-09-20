/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// Check that object literal with numeric properties works
var x = {0:10, 1:20}
print(x[0], x[1]);
//CHECK: 10 20
