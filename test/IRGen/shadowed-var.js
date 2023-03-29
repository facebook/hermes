/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Xenable-tdz -dump-ir %s -O0 > /dev/null

// Ensure that the var declaration and initialized shadowed by a previous
// declaration doesn't assert for a duplicate declaration.

function foo(a = 5) {
    var a = 10;
}

function bar() {
    try {
        something();
    } catch ([e = 10]) {
        var e = 10;
    }
}

