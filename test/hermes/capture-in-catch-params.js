/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

var probeParam, probeBlock;
let x = 'outside';

try {
  throw [];
} catch ([_ = probeParam = function() { return x; }]) {
  probeBlock = function() { return x; };
  let x = 'inside';
}

print(probeParam());
// CHECK: outside
print(probeBlock());
// CHECK: inside
