/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict -O %s | %FileCheck %s
// RUN: %hermes -non-strict -O0 %s | %FileCheck %s

const obj = {
    toString: () => {
        gc();
        return 0n;
    }
};
// trigger after obj.toString called and before addOp complete
let trigger_point = 1n + obj;
print(trigger_point);

// CHECK: 1
