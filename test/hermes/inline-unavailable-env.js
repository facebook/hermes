/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// Test our ability to inline functions where their enclosing environment is not
// available at the call site.

function foo(sink){
    let x;
    let numBar = 0;
    function bar() {
        let y = ++numBar;
        // Assign x in bar, so the parent environment of the function is unavailable
        // when  we call x, and we are forced to get the environment from the
        // closure.
        x = function () { "inline"; return ++y; };
    }
    sink(bar);
    return x();
}

// Call bar when it is passed in, and then print the result.
print(foo((cb) => { cb(); cb(); cb(); }));
// CHECK: 4
