/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

function f() {}

var obj = {
  __proto__: f,
  b: 12
};

for (var i in obj) {
  print(i, obj[i]);
}
// CHECK: b 12
