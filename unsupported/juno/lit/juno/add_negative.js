/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno --gen-js -O %s | %FileCheck %s --match-full-lines
// Transform is currently disabled because it's unsafe.
// XFAIL: true

function foo() {
  x + -y;
}

// CHECK-LABEL: function foo() {
// CHECK-NEXT:   x - y;
// CHECK-NEXT: }
