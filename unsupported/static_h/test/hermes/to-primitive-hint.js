/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC %s | %FileCheck --match-full-lines %s
// Check that toPrimitive() is called with the correct hint

var x = {
    toString: function() { return "toString"; },
    valueOf: function() { return "valueOf"; }
};

print(String(x));
//CHECK: toString
print(x + "");
//CHECK-NEXT: valueOf
