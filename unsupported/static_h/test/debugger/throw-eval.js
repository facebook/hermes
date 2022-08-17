/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --break-at-start %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

try {
  throw new Error("lol");
} catch(e) {
  print('caught');
}

// CHECK: Break on script load in global: {{.*}}[{{[0-9]+}}]:11:1
// CHECK-NEXT: Set pauseOnThrow: all errors
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on exception in global: {{.*}}[{{[0-9]+}}]:12:3
// CHECK-NEXT: 1
// CHECK-NEXT: Continuing execution
