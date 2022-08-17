/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O -non-strict %s | %FileCheck --match-full-lines %s

x = 10;
print(x);
// CHECK: 10

try {
    print(y);
} catch (e) {
    print(e.name, e.message);
}
// CHECK-NEXT: ReferenceError {{.*}}
