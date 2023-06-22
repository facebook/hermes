/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s --match-full-lines

// Parser should reject this, but it currently doesn't.
function f1(v1 = this, \u0074his) {
  return [v1, this];
}
print(f1());
// CHECK: [object global],[object global]
print(f1(1));
// CHECK-NEXT: 1,[object global]
