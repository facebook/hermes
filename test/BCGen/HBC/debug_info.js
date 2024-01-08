/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-bytecode -target=HBC -g %s | %FileCheckOrRegen %s --match-full-lines
// RUN: cp %s %T/debug_info_à.js && %hermes -O0 -dump-bytecode -target=HBC -g %T/debug_info_à.js | %FileCheckOrRegen %s --match-full-lines --check-prefix=UNICODE

var v1g = "global";

function Fa() {
    var v1a = 3;
    var v2a = 5;
}


function Fb() {
    var v1b = "abc";
}

function Fc() {
    var v1c = undefined;
    function Fcc() {
        var v1cc = 42;
    }
}
