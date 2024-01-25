/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s
// RUN: %shermes -exec %s

// This test exercises an issue found in LowerArgumentsArray in which PHI nodes
// were not being properly updated.
function decrementArguments() {
    for (var i = 0; i < 2; i++) {
        var var1 = () => var3;
        var var3 = arguments;
    }
    return var3 - 1;
}

print(decrementArguments());
