/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -emit-binary -out=%t -O -target=HBC %s &&  %hermes -b %t | %FileCheck --match-full-lines %s

eps = HermesInternal.getEpilogues();
print(eps.length);
// CHECK:1
print(eps[0]);
// CHECK-NEXT:undefined
