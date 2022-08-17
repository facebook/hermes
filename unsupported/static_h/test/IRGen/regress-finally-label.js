/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s
//
// Test for a regression where we were asserting because bodies of fionally
// statements are visited more than once.

function foo(a, b) {
    try {
        if (foo())
            return;
    } finally {
        while (a < 10) {
            b[++a] = 0;
        }
    }
}


