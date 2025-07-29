/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

// Custom async iterable with error throwing return()
const asyncIterableWithReturnError = {
    [Symbol.asyncIterator]: function() {
        let i = 0;
        return {
            next: function() {
                if (i < 3) {
                    return Promise.resolve({ value: i++, done: false });
                } else {
                    return Promise.resolve({ done: true });
                }
            },
            return: function() {
                print('Attempting cleanup...');
                return Promise.reject(new Error('Cleanup error'));
            }
        };
    }
};

// Test for error thrown in the loop with error in return()
(async function testErrorInReturn() {
    try {
        for await (const value of asyncIterableWithReturnError) {
            if (value === 2) {
                throw new Error('Loop error');
            }
            print(value);
        }
    } catch (e) {
        print(e.message);
    }
})();

// Expected Output:
// CHECK: 0
// CHECK-NEXT: 1
// CHECK-NEXT: Attempting cleanup...
// CHECK-NEXT: Loop error
