/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s -Wx,-gc-sanitize-handles=0

function manyIdents(obj) {
    for(var i = 0; i < 10000; ++i) {
        delete obj["p" + i];
    }
}

manyIdents({});
print("Success!");
