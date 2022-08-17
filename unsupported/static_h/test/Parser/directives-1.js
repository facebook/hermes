/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict %s

// Make sure we ignore non-directives.
"use strict"+
1;
"use strict";

// This should be fine since the directives above are not valid.
x = 010;

// Directives cannot contain escapes.
function f1 () {
    "use\x20strict";
    // This should be fine since the directives above are not valid.
    delete y;
}

// Directives cannot contain line continuations.
function f2 () {
    "use \
strict";
    // This should be fine since the directives above are not valid.
    delete y;
}
