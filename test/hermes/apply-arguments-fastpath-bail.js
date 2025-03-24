/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s

function f(x, y, z) {
  print(y);
}
Object.prototype[1] = 20;
function h() {
  delete arguments[1];
  return f.apply(null, arguments);
}
h(1, 2, 3);

// CHECK: 20
