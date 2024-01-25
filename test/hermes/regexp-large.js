/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=en_US.UTF-8 %hermes %s | %FileCheck --match-full-lines %s

// Verify that an extremely long (but flat) regexp can be parsed and match.
var longString = "x";
for (var i=0; i < 22; i++) longString += longString;
print(RegExp(longString).test(longString));
// CHECK: true

// Verify that extremely long ranges can be parsed.
print(RegExp("[" + longString + "]").exec("x"));
// CHECK: x
