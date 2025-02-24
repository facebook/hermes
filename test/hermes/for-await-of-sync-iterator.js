/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-promise %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -Xes6-promise %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy -Xes6-promise %s | %FileCheck --match-full-lines %s

// For await of loop should work with a sync iterable
const syncIterable = [1,2,3];

(async function testCustomSyncIterable() {
    sum = 0;
    for await (const value of syncIterable) {
        sum += value;
    }
    print(sum);
})();

//CHECK: 6