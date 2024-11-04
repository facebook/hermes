/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-promise %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -Xes6-promise %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy -Xes6-promise %s | %FileCheck --match-full-lines %s

// Custom async iterable using Symbol.asyncIterator
const asyncIterable = {
    [Symbol.asyncIterator]: function() {
        let i = 1;
        const max = 3;
        return {
            next: function() {
                if (i <= max) {
                    return Promise.resolve({ value: i++, done: false });
                } else {
                    return Promise.resolve({ done: true });
                }
            }
        };
    }
};

// Test for await of loop with custom async iterable
(async function testCustomAsyncIterable() {
    sum = 0;
    for await (const value of asyncIterable) {
        sum += value;
    }
    print(sum);
})();

//CHECK: 6