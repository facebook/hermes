/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-promise %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -Xes6-promise %s | %FileCheck --match-full-lines %s

// Noted that the process of resolving Promise is asynchronous,
// and deeper promise chain will take more microtask ticks to complete.
//
// Hence, the running of tests are ordered from taking less ticks to more ticks
// to make printings conforming with the order of comments in the source code.

print('async emit parameter');
// CHECK-LABEL: async emit parameter

// This test is to make sure async function did not emit code for
// destructuring parameters twice.

// The trick is to let an iterator make side effect when visited.
var iter = {
    [Symbol.iterator]: function() {
        print('OPEN')
        return {
            next: function() {
                return { value: 42, done: false };
            }
        };
    }
};

// And examine that only one effect was made.
async function foo([x]) {
    print('START', x)
    return 5;
};

foo(iter).then(v => print(v));
// CHECK-NEXT: OPEN
// CHECK-NEXT: START 42
// CHECK-NEXT: 5
