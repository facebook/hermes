/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

// Custom async iterable using Symbol.asyncIterator
const asyncIterable = {
    [Symbol.asyncIterator]: function() {
        return {
            next: function() {
                return Promise.resolve({ value:0, done: false });
            },
            return: function() {
                print('Cleanup performed');
                return Promise.resolve({ done: true });
            }
        };
    }
};

// Using break and triggering return() in for await...of loop
(async function testBreakWithReturn() {
    for await (const value of asyncIterable) {
        if (value === 0) {
            break;
        }
    }
})();

//CHECK: Cleanup performed
