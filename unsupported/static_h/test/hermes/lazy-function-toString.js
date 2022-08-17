/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict -O0 %s 2>&1 | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy -non-strict -O0 %s 2>&1 | %FileCheck --match-full-lines %s
// UNSUPPORTED: serializer

// foo is never called, but its source needs to be reserved ahead of time.
function foo(x) { "show source"; }

(function bar() {
    print(foo.toString());
    // CHECK: function foo(x) { "show source"; }
})()
