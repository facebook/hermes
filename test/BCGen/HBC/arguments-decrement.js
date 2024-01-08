/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -Xdump-between-passes %s 2>&1 | %FileCheckOrRegen --match-full-lines %s --check-prefix CHKIR
// RUN: %hermes -O -dump-ra %s | %FileCheckOrRegen --match-full-lines %s --check-prefix CHKRA

// This test exercises an issue found in LowerArgumentsArray in which PHI nodes
// were not being properly updated.
function decrementArguments() {
    for (var i = 0; i < 2; i++) {
        var var1 = () => var3 = 0;
        var var3 = arguments;
    }
    return var3 - 1;
}

