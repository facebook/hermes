/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xenable-tdz -test262 -O0 -target=HBC -dump-bytecode %s | %FileCheckOrRegen --match-full-lines %s

function foo() {
    return x;
    let x;
}
