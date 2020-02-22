/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -allow-function-to-string -lazy %s | %FileCheck --match-full-lines %s

function nonLazy() {
  'bar';
}
print(nonLazy.toString());
// CHECK-LABEL:function nonLazy() {
// CHECK-NEXT:  'bar';
// CHECK-NEXT:}

function lazy() {
  '0123456789abcdef';
  '0123456789abcdef';
  '0123456789abcdef';
  '0123456789abcdef';
  '0123456789abcdef';
  '0123456789abcdef';
  '0123456789abcdef';
  '0123456789abcdef';
  '0123456789abcdef';
  '0123456789abcdef';
}
print(lazy.toString());
// CHECK-LABEL:function lazy() {
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:}
