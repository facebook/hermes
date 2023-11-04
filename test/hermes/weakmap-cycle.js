/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-max-heap=32M %s
// REQUIRES: !slow_debug

(function () {
    var foo = new WeakMap();

    for (var i = 0; i < 1000000; i++) {
        var x = {};
        foo.set(x, x);
    }
})();
