/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec -Xenable-tdz -Xes6-block-scoping -O0 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec -Xenable-tdz -Xes6-block-scoping -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xenable-tdz -Xes6-block-scoping -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xenable-tdz -Xes6-block-scoping -O %s | %FileCheck --match-full-lines %s

let x;
for (let [y, z = ()=>y] in [x = ()=>y]) {
    print(y);
    print(z());
    try { x(); } catch(e) { print(e); }
}

// CHECK: 0
// CHECK-NEXT: 0
// CHECK-NEXT: ReferenceError: accessing an uninitialized variable
