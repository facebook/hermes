/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-proxy %s | %FileCheck --match-full-lines %s

// Put a proxy in the prototype chain of the arguments array
({}).__proto__.__proto__ = new Proxy({},{});

function innerFunc() {
    // Trigger getArgumentsPropByValSlowPath_RJS.
    print(arguments[1]);
}

innerFunc(4, 2);
// CHECK: 2
