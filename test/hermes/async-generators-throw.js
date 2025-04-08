/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xasync-generators %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xasync-generators -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xasync-generators -lazy %s | %FileCheck --match-full-lines %s

async function* errorHandlingGenerator() {
    yield 1;
    throw new Error('Async generator threw!');
    yield 2;
}

// Test error propagation with the async generator
(async function testErrorPropagation() {
    try {
        for await (const value of errorHandlingGenerator()) {
            print(value);
        }
    } catch (error) {
        print('Caught error:', error.message);
    }
})();
//CHECK: Caught error: Async generator threw!
