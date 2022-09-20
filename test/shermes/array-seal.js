/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

var x = []
x[0] = 10
Object.preventExtensions(x);
print(Object.isSealed(x));
//CHECK: false
