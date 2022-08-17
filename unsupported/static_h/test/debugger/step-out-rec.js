/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function fact(n) {
  if (n < 2) {
    return 1;
  }
  return n * fact(n - 1);
}

debugger;
fact(4);

// CHECK: Break on 'debugger' statement in global: {{.*}}:18:1
// CHECK-NEXT: Stepped to global: {{.*}}:19:1
// CHECK-NEXT: Stepped to fact: {{.*}}:12:7
// CHECK-NEXT: Stepped to global: {{.*}}:19:5
// CHECK-NEXT: Continuing execution
