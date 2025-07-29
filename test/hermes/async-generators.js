/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xasync-generators %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xasync-generators -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xasync-generators -lazy %s | %FileCheck --match-full-lines %s

async function* mixedValuesGenerator() {
    yield 1;
    yield Promise.resolve(2);
    yield new Promise((resolve) => resolve(3));
}

// Test async generator with for await...of loop
(async function testMixedValuesGenerator() {
    let sum = 0;
    for await (const value of mixedValuesGenerator()) {
        sum += value;
    }
    print(sum);
})();
//CHECK: 6
