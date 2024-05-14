/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s > %t.js && %hermes %t.js | %FileCheck --match-full-lines %s
// REQUIRES: !debug_options

// Generate a JS file containing 70,000 objects each with a single property.
// Read and sum up all these properties. This is done to exercise the
// compiler & runtime code paths for a large amount of object literals.
var code = "var sum = 0;\n";
var expectedSum = 0;
for (var i = 1; i <= 70_000; i++){
  expectedSum += i;
  code += `
var o${i} = {
  p${i}: ${i}
};
sum += o${i}.p${i};`
}
code += `
print(sum == ${expectedSum});`;
print(code);

// CHECK: true
