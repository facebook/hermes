/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno --gen-js -O %s | %FileCheck %s --match-full-lines

function foo(x, y) {
  return true ? x + 1 : y + 1
}

// CHECK-LABEL: function foo(x, y) {
// CHECK-NEXT:   return x + 1;
// CHECK-NEXT: }

function bar(x, y) {
  return false ? x + 1 : y + 1
}

// CHECK-LABEL: function bar(x, y) {
// CHECK-NEXT:   return y + 1;
// CHECK-NEXT: }
