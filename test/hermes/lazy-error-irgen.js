/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

print('irgen error');
// CHECK-LABEL: irgen error

// This function will error in IRGen.
// Make sure it can be called multiple times.
function foo() {
  return super[1];
}

try {
  foo();
} catch (e) { 
  print(1, e.name);
  // CHECK-NEXT: 1 SyntaxError
}
try {
  foo();
} catch (e) { 
  print(2, e.name);
  // CHECK-NEXT: 2 SyntaxError
}
