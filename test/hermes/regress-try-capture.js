/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

function main() {
    var x = false;
    try {
        var foo = {
            method: function() {
                x = true;
            },
        };
        foo.method();
        throw Error()
    } catch (error) {
        return x;
    }
}

print(main());
// CHECK: true
