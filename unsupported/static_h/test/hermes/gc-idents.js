/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O -gc-sanitize-handles=0 %s

function manyIdents(obj) {
    for(var i = 0; i < 10000; ++i) {
        delete obj["p" + i];
    }
}

manyIdents({});
print("Success!");
