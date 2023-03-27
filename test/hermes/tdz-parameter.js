/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s              | %FileCheck %s --check-prefix NO-TDZ
// RUN: %hermes -O  %s              | %FileCheck %s --check-prefix NO-TDZ
// RUN: %hermes -O0 %s -Xenable-tdz | %FileCheck %s --check-prefix NO-TDZ
// RUN: %hermes -O  %s -Xenable-tdz | %FileCheck %s --check-prefix NO-TDZ
// RUN: %hermes -O0 %s -bs          | %FileCheck %s --check-prefix TDZ
// RUN: %hermes -O  %s -bs          | %FileCheck %s --check-prefix TDZ

var print = typeof print !== "undefined" ? print : console.log;

function funcWithParamExpression(a = b, b = 10) {
    return a;
}

function test() {
    try {
        return funcWithParamExpression();
    } catch (err) {
        return `Could not call funcWithParamExpression(): ${err.stack}`;
    }
}

print(test());
// NO-TDZ: undefined
// TDZ: Could not call funcWithParamExpression(): ReferenceError:
