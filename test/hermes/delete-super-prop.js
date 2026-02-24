/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s -test262 | %FileCheck --match-full-lines %s
// RUN: %hermes %s -lazy -test262 | %FileCheck --match-full-lines %s

print("delete super property");
// CHECK-LABEL: delete super property

try {
  var o = {
    m1() { delete super.x; }
  };
  o.m1();
} catch (e) {
  print(e);
//CHECK-NEXT: ReferenceError: Cannot delete a super property.
}
