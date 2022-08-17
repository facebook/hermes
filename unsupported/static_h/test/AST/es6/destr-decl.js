/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Werror -dump-ir %s > /dev/null

// Ensure that we are declaring the correct variable name, "c" not "b".
function foo() {
    "use strict";
    var {a, b:c} = {}
    return a + c;
}

foo();
