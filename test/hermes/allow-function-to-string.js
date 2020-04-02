/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -allow-function-to-string %s | %FileCheck --match-full-lines %s

function foo() {
  'bar';
}
print(foo.toString());
// CHECK-LABEL:function foo() {
// CHECK-NEXT:  'bar';
// CHECK-NEXT:}
